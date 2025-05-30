/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtNetwork module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

//#define QNETWORKACCESSHTTPBACKEND_DEBUG

#include "qnetworkreplyhttpimpl_p.h"
#include "qnetworkaccessmanager_p.h"
#include "qnetworkaccesscache_p.h"
#include "qabstractnetworkcache.h"
#include "qnetworkrequest.h"
#include "qnetworkreply.h"
#include "qnetworkrequest_p.h"
#include "qnetworkcookie.h"
#include "qnetworkcookie_p.h"
#include "QtCore/qdatetime.h"
#include "QtCore/qelapsedtimer.h"
#include "QtNetwork/qsslconfiguration.h"
#include "qhttpthreaddelegate_p.h"
#include "qhsts_p.h"
#include "qthread.h"
#include "QtCore/qcoreapplication.h"

#include <QtCore/private/qthread_p.h>

#include "qnetworkcookiejar.h"
#include "qnetconmonitor_p.h"

#include "qnetworkreplyimpl_p.h"

#include <string.h>             // for strchr

QT_BEGIN_NAMESPACE

class QNetworkProxy;

static inline bool isSeparator(char c)
{
    static const char separators[] = "()<>@,;:\\\"/[]?={}";
    return isLWS(c) || strchr(separators, c) != nullptr;
}

// ### merge with nextField in cookiejar.cpp
static QHash<QByteArray, QByteArray> parseHttpOptionHeader(const QByteArray &header)
{
    // The HTTP header is of the form:
    // header          = #1(directives)
    // directives      = token | value-directive
    // value-directive = token "=" (token | quoted-string)
    QHash<QByteArray, QByteArray> result;

    int pos = 0;
    while (true) {
        // skip spaces
        pos = nextNonWhitespace(header, pos);
        if (pos == header.length())
            return result;      // end of parsing

        // pos points to a non-whitespace
        int comma = header.indexOf(',', pos);
        int equal = header.indexOf('=', pos);
        if (comma == pos || equal == pos)
            // huh? Broken header.
            return result;

        // The key name is delimited by either a comma, an equal sign or the end
        // of the header, whichever comes first
        int end = comma;
        if (end == -1)
            end = header.length();
        if (equal != -1 && end > equal)
            end = equal;        // equal sign comes before comma/end
        QByteArray key = QByteArray(header.constData() + pos, end - pos).trimmed().toLower();
        pos = end + 1;

        if (uint(equal) < uint(comma)) {
            // case: token "=" (token | quoted-string)
            // skip spaces
            pos = nextNonWhitespace(header, pos);
            if (pos == header.length())
                // huh? Broken header
                return result;

            QByteArray value;
            value.reserve(header.length() - pos);
            if (header.at(pos) == '"') {
                // case: quoted-string
                // quoted-string  = ( <"> *(qdtext | quoted-pair ) <"> )
                // qdtext         = <any TEXT except <">>
                // quoted-pair    = "\" CHAR
                ++pos;
                while (pos < header.length()) {
                    char c = header.at(pos);
                    if (c == '"') {
                        // end of quoted text
                        break;
                    } else if (c == '\\') {
                        ++pos;
                        if (pos >= header.length())
                            // broken header
                            return result;
                        c = header.at(pos);
                    }

                    value += c;
                    ++pos;
                }
            } else {
                // case: token
                while (pos < header.length()) {
                    char c = header.at(pos);
                    if (isSeparator(c))
                        break;
                    value += c;
                    ++pos;
                }
            }

            result.insert(key, value);

            // find the comma now:
            comma = header.indexOf(',', pos);
            if (comma == -1)
                return result;  // end of parsing
            pos = comma + 1;
        } else {
            // case: token
            // key is already set
            result.insert(key, QByteArray());
        }
    }
}

QNetworkReplyHttpImpl::QNetworkReplyHttpImpl(QNetworkAccessManager* const manager,
                                             const QNetworkRequest& request,
                                             QNetworkAccessManager::Operation& operation,
                                             QIODevice* outgoingData)
    : QNetworkReply(*new QNetworkReplyHttpImplPrivate, manager)
{
    Q_D(QNetworkReplyHttpImpl);
    Q_ASSERT(manager);
    d->manager = manager;
    d->managerPrivate = manager->d_func();
    d->request = request;
    d->originalRequest = request;
    d->operation = operation;
    d->outgoingData = outgoingData;
    d->url = request.url();
#ifndef QT_NO_SSL
    if (request.url().scheme() == QLatin1String("https"))
        d->sslConfiguration.reset(new QSslConfiguration(request.sslConfiguration()));
#endif

    QObjectPrivate::connect(this, &QNetworkReplyHttpImpl::redirectAllowed, d,
            &QNetworkReplyHttpImplPrivate::followRedirect, Qt::QueuedConnection);

    // FIXME Later maybe set to Unbuffered, especially if it is zerocopy or from cache?
    QIODevice::open(QIODevice::ReadOnly);


    // Internal code that does a HTTP reply for the synchronous Ajax
    // in Qt WebKit.
    QVariant synchronousHttpAttribute = request.attribute(
            static_cast<QNetworkRequest::Attribute>(QNetworkRequest::SynchronousRequestAttribute));
    if (synchronousHttpAttribute.isValid()) {
        d->synchronous = synchronousHttpAttribute.toBool();
        if (d->synchronous && outgoingData) {
            // The synchronous HTTP is a corner case, we will put all upload data in one big QByteArray in the outgoingDataBuffer.
            // Yes, this is not the most efficient thing to do, but on the other hand synchronous XHR needs to die anyway.
            d->outgoingDataBuffer = QSharedPointer<QRingBuffer>::create();
            qint64 previousDataSize = 0;
            do {
                previousDataSize = d->outgoingDataBuffer->size();
                d->outgoingDataBuffer->append(d->outgoingData->readAll());
            } while (d->outgoingDataBuffer->size() != previousDataSize);
            d->_q_startOperation();
            return;
        }
    }


    if (outgoingData) {
        // there is data to be uploaded, e.g. HTTP POST.

        if (!d->outgoingData->isSequential()) {
            // fixed size non-sequential (random-access)
            // just start the operation
            QMetaObject::invokeMethod(this, "_q_startOperation", Qt::QueuedConnection);
            // FIXME make direct call?
        } else {
            bool bufferingDisallowed =
                    request.attribute(QNetworkRequest::DoNotBufferUploadDataAttribute,
                                  false).toBool();

            if (bufferingDisallowed) {
                // if a valid content-length header for the request was supplied, we can disable buffering
                // if not, we will buffer anyway
                if (request.header(QNetworkRequest::ContentLengthHeader).isValid()) {
                    QMetaObject::invokeMethod(this, "_q_startOperation", Qt::QueuedConnection);
                    // FIXME make direct call?
                } else {
                    d->state = d->Buffering;
                    QMetaObject::invokeMethod(this, "_q_bufferOutgoingData", Qt::QueuedConnection);
                }
            } else {
                // _q_startOperation will be called when the buffering has finished.
                d->state = d->Buffering;
                QMetaObject::invokeMethod(this, "_q_bufferOutgoingData", Qt::QueuedConnection);
            }
        }
    } else {
        // No outgoing data (POST, ..)
        d->_q_startOperation();
    }
}

QNetworkReplyHttpImpl::~QNetworkReplyHttpImpl()
{
    // This will do nothing if the request was already finished or aborted
    emit abortHttpRequest();
}

void QNetworkReplyHttpImpl::close()
{
    Q_D(QNetworkReplyHttpImpl);

    if (d->state == QNetworkReplyPrivate::Aborted ||
        d->state == QNetworkReplyPrivate::Finished)
        return;

    // According to the documentation close only stops the download
    // by closing we can ignore the download part and continue uploading.
    QNetworkReply::close();

    // call finished which will emit signals
    // FIXME shouldn't this be emitted Queued?
    d->error(OperationCanceledError, tr("Operation canceled"));
    d->finished();
}

void QNetworkReplyHttpImpl::abort()
{
    Q_D(QNetworkReplyHttpImpl);
    // FIXME
    if (d->state == QNetworkReplyPrivate::Finished || d->state == QNetworkReplyPrivate::Aborted)
        return;

    QNetworkReply::close();

    if (d->state != QNetworkReplyPrivate::Finished) {
        // call finished which will emit signals
        // FIXME shouldn't this be emitted Queued?
        d->error(OperationCanceledError, tr("Operation canceled"));
        d->finished();
    }

    d->state = QNetworkReplyPrivate::Aborted;

    emit abortHttpRequest();
}

qint64 QNetworkReplyHttpImpl::bytesAvailable() const
{
    Q_D(const QNetworkReplyHttpImpl);

    // if we load from cache device
    if (d->cacheLoadDevice) {
        return QNetworkReply::bytesAvailable() + d->cacheLoadDevice->bytesAvailable();
    }

    // zerocopy buffer
    if (d->downloadZerocopyBuffer) {
        return QNetworkReply::bytesAvailable() + d->downloadBufferCurrentSize - d->downloadBufferReadPosition;
    }

    // normal buffer
    return QNetworkReply::bytesAvailable();
}

bool QNetworkReplyHttpImpl::isSequential () const
{
    // FIXME In the cache of a cached load or the zero-copy buffer we could actually be non-sequential.
    // FIXME however this requires us to implement stuff like seek() too.
    return true;
}

qint64 QNetworkReplyHttpImpl::size() const
{
    // FIXME At some point, this could return a proper value, e.g. if we're non-sequential.
    return QNetworkReply::size();
}

qint64 QNetworkReplyHttpImpl::readData(char* data, qint64 maxlen)
{
    Q_D(QNetworkReplyHttpImpl);

    // cacheload device
    if (d->cacheLoadDevice) {
        // FIXME bytesdownloaded, position etc?

        qint64 ret = d->cacheLoadDevice->read(data, maxlen);
        return ret;
    }

    // zerocopy buffer
    if (d->downloadZerocopyBuffer) {
        // FIXME bytesdownloaded, position etc?

        qint64 howMuch = qMin(maxlen, (d->downloadBufferCurrentSize - d->downloadBufferReadPosition));
        memcpy(data, d->downloadZerocopyBuffer + d->downloadBufferReadPosition, howMuch);
        d->downloadBufferReadPosition += howMuch;
        return howMuch;

    }

    // normal buffer
    if (d->state == d->Finished || d->state == d->Aborted)
        return -1;

    qint64 wasBuffered = d->bytesBuffered;
    d->bytesBuffered = 0;
    if (readBufferSize())
        emit readBufferFreed(wasBuffered);
    return 0;
}

void QNetworkReplyHttpImpl::setReadBufferSize(qint64 size)
{
    QNetworkReply::setReadBufferSize(size);
    emit readBufferSizeChanged(size);
    return;
}

bool QNetworkReplyHttpImpl::canReadLine () const
{
    Q_D(const QNetworkReplyHttpImpl);

    if (QNetworkReply::canReadLine())
        return true;

    if (d->cacheLoadDevice)
        return d->cacheLoadDevice->canReadLine();

    if (d->downloadZerocopyBuffer)
        return memchr(d->downloadZerocopyBuffer + d->downloadBufferReadPosition, '\n', d->downloadBufferCurrentSize - d->downloadBufferReadPosition);

    return false;
}

#ifndef QT_NO_SSL
void QNetworkReplyHttpImpl::ignoreSslErrors()
{
    Q_D(QNetworkReplyHttpImpl);
    Q_ASSERT(d->managerPrivate);

    if (d->managerPrivate->stsEnabled && d->managerPrivate->stsCache.isKnownHost(url())) {
        // We cannot ignore any Security Transport-related errors for this host.
        return;
    }

    d->pendingIgnoreAllSslErrors = true;
}

void QNetworkReplyHttpImpl::ignoreSslErrorsImplementation(const QList<QSslError> &errors)
{
    Q_D(QNetworkReplyHttpImpl);
    Q_ASSERT(d->managerPrivate);

    if (d->managerPrivate->stsEnabled && d->managerPrivate->stsCache.isKnownHost(url())) {
        // We cannot ignore any Security Transport-related errors for this host.
        return;
    }

    // the pending list is set if QNetworkReply::ignoreSslErrors(const QList<QSslError> &errors)
    // is called before QNetworkAccessManager::get() (or post(), etc.)
    d->pendingIgnoreSslErrorsList = errors;
}

void QNetworkReplyHttpImpl::setSslConfigurationImplementation(const QSslConfiguration &newconfig)
{
    // Setting a SSL configuration on a reply is not supported. The user needs to set
    // her/his QSslConfiguration on the QNetworkRequest.
    Q_UNUSED(newconfig);
}

void QNetworkReplyHttpImpl::sslConfigurationImplementation(QSslConfiguration &configuration) const
{
    Q_D(const QNetworkReplyHttpImpl);
    if (d->sslConfiguration.data())
        configuration = *d->sslConfiguration;
    else
        configuration = request().sslConfiguration();
}
#endif

QNetworkReplyHttpImplPrivate::QNetworkReplyHttpImplPrivate()
    : QNetworkReplyPrivate()
    , manager(nullptr)
    , managerPrivate(nullptr)
    , synchronous(false)
    , state(Idle)
    , statusCode(0)
    , uploadByteDevicePosition(false)
    , uploadDeviceChoking(false)
    , outgoingData(nullptr)
    , bytesUploaded(-1)
    , cacheLoadDevice(nullptr)
    , loadingFromCache(false)
    , cacheSaveDevice(nullptr)
    , cacheEnabled(false)
    , resumeOffset(0)
    , bytesDownloaded(0)
    , bytesBuffered(0)
    , transferTimeout(nullptr)
    , downloadBufferReadPosition(0)
    , downloadBufferCurrentSize(0)
    , downloadZerocopyBuffer(nullptr)
    , pendingDownloadDataEmissions(QSharedPointer<QAtomicInt>::create())
    , pendingDownloadProgressEmissions(QSharedPointer<QAtomicInt>::create())
    #ifndef QT_NO_SSL
    , pendingIgnoreAllSslErrors(false)
    #endif

{
}

QNetworkReplyHttpImplPrivate::~QNetworkReplyHttpImplPrivate()
{
}

/*
    For a given httpRequest
    1) If AlwaysNetwork, return
    2) If we have a cache entry for this url populate headers so the server can return 304
    3) Calculate if response_is_fresh and if so send the cache and set loadedFromCache to true
 */
bool QNetworkReplyHttpImplPrivate::loadFromCacheIfAllowed(QHttpNetworkRequest &httpRequest)
{
    QNetworkRequest::CacheLoadControl CacheLoadControlAttribute =
        (QNetworkRequest::CacheLoadControl)request.attribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferNetwork).toInt();
    if (CacheLoadControlAttribute == QNetworkRequest::AlwaysNetwork) {
        // If the request does not already specify preferred cache-control
        // force reload from the network and tell any caching proxy servers to reload too
        if (!request.rawHeaderList().contains("Cache-Control")) {
            httpRequest.setHeaderField("Cache-Control", "no-cache");
            httpRequest.setHeaderField("Pragma", "no-cache");
        }
        return false;
    }

    // The disk cache API does not currently support partial content retrieval.
    // That is why we don't use the disk cache for any such requests.
    if (request.hasRawHeader("Range"))
        return false;

    QAbstractNetworkCache *nc = managerPrivate->networkCache;
    if (!nc)
        return false;                 // no local cache

    QNetworkCacheMetaData metaData = nc->metaData(httpRequest.url());
    if (!metaData.isValid())
        return false;                 // not in cache

    if (!metaData.saveToDisk())
        return false;

    QNetworkHeadersPrivate cacheHeaders;
    QNetworkHeadersPrivate::RawHeadersList::ConstIterator it;
    cacheHeaders.setAllRawHeaders(metaData.rawHeaders());

    it = cacheHeaders.findRawHeader("content-length");
    if (it != cacheHeaders.rawHeaders.constEnd()) {
        std::unique_ptr<QIODevice> data(nc->data(httpRequest.url()));
        if (!data || data->size() < it->second.toLongLong())
            return false; // The data is smaller than the content-length specified
    }

    it = cacheHeaders.findRawHeader("etag");
    if (it != cacheHeaders.rawHeaders.constEnd())
        httpRequest.setHeaderField("If-None-Match", it->second);

    QDateTime lastModified = metaData.lastModified();
    if (lastModified.isValid())
        httpRequest.setHeaderField("If-Modified-Since", QNetworkHeadersPrivate::toHttpDate(lastModified));

    it = cacheHeaders.findRawHeader("Cache-Control");
    if (it != cacheHeaders.rawHeaders.constEnd()) {
        QHash<QByteArray, QByteArray> cacheControl = parseHttpOptionHeader(it->second);
        if (cacheControl.contains("must-revalidate"))
            return false;
        if (cacheControl.contains("no-cache"))
            return false;
    }

    QDateTime currentDateTime = QDateTime::currentDateTimeUtc();
    QDateTime expirationDate = metaData.expirationDate();

    bool response_is_fresh;
    if (!expirationDate.isValid()) {
        /*
         * age_value
         *      is the value of Age: header received by the cache with
         *              this response.
         * date_value
         *      is the value of the origin server's Date: header
         * request_time
         *      is the (local) time when the cache made the request
         *              that resulted in this cached response
         * response_time
         *      is the (local) time when the cache received the
         *              response
         * now
         *      is the current (local) time
         */
        qint64 age_value = 0;
        it = cacheHeaders.findRawHeader("age");
        if (it != cacheHeaders.rawHeaders.constEnd())
            age_value = it->second.toLongLong();

        QDateTime dateHeader;
        qint64 date_value = 0;
        it = cacheHeaders.findRawHeader("date");
        if (it != cacheHeaders.rawHeaders.constEnd()) {
            dateHeader = QNetworkHeadersPrivate::fromHttpDate(it->second);
            date_value = dateHeader.toSecsSinceEpoch();
        }

        qint64 now = currentDateTime.toSecsSinceEpoch();
        qint64 request_time = now;
        qint64 response_time = now;

        // Algorithm from RFC 2616 section 13.2.3
        qint64 apparent_age = qMax<qint64>(0, response_time - date_value);
        qint64 corrected_received_age = qMax(apparent_age, age_value);
        qint64 response_delay = response_time - request_time;
        qint64 corrected_initial_age = corrected_received_age + response_delay;
        qint64 resident_time = now - response_time;
        qint64 current_age   = corrected_initial_age + resident_time;

        qint64 freshness_lifetime = 0;

        // RFC 2616 13.2.4 Expiration Calculations
        if (lastModified.isValid() && dateHeader.isValid()) {
            qint64 diff = lastModified.secsTo(dateHeader);
            freshness_lifetime = diff / 10;
            if (httpRequest.headerField("Warning").isEmpty()) {
                QDateTime dt = currentDateTime.addSecs(current_age);
                if (currentDateTime.daysTo(dt) > 1)
                    httpRequest.setHeaderField("Warning", "113");
            }
        }

        // the cache-saving code below sets the freshness_lifetime with (dateHeader - last_modified) / 10
        // if "last-modified" is present, or to Expires otherwise
        response_is_fresh = (freshness_lifetime > current_age);
    } else {
        // expiration date was calculated earlier (e.g. when storing object to the cache)
        response_is_fresh = currentDateTime.secsTo(expirationDate) >= 0;
    }

    if (!response_is_fresh)
        return false;

#if defined(QNETWORKACCESSHTTPBACKEND_DEBUG)
    qDebug() << "response_is_fresh" << CacheLoadControlAttribute;
#endif
    return sendCacheContents(metaData);
}

QHttpNetworkRequest::Priority QNetworkReplyHttpImplPrivate::convert(const QNetworkRequest::Priority& prio)
{
    switch (prio) {
    case QNetworkRequest::LowPriority:
        return QHttpNetworkRequest::LowPriority;
    case QNetworkRequest::HighPriority:
        return QHttpNetworkRequest::HighPriority;
    case QNetworkRequest::NormalPriority:
    default:
        return QHttpNetworkRequest::NormalPriority;
    }
}

void QNetworkReplyHttpImplPrivate::postRequest(const QNetworkRequest &newHttpRequest)
{
    Q_Q(QNetworkReplyHttpImpl);

    QThread *thread = nullptr;
    if (synchronous) {
        // A synchronous HTTP request uses its own thread
        thread = new QThread();
        thread->setObjectName(QStringLiteral("Qt HTTP synchronous thread"));
        QObject::connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
        thread->start();
    } else {
        // We use the manager-global thread.
        // At some point we could switch to having multiple threads if it makes sense.
        thread = managerPrivate->createThread();
    }

    QUrl url = newHttpRequest.url();
    httpRequest.setUrl(url);
    httpRequest.setRedirectCount(newHttpRequest.maximumRedirectsAllowed());

    QString scheme = url.scheme();
    bool ssl = (scheme == QLatin1String("https")
                || scheme == QLatin1String("preconnect-https"));
    q->setAttribute(QNetworkRequest::ConnectionEncryptedAttribute, ssl);
    httpRequest.setSsl(ssl);

    bool preConnect = (scheme == QLatin1String("preconnect-http")
                       || scheme == QLatin1String("preconnect-https"));
    httpRequest.setPreConnect(preConnect);

#ifndef QT_NO_NETWORKPROXY
    QNetworkProxy transparentProxy, cacheProxy;

    // FIXME the proxy stuff should be done in the HTTP thread
    const auto proxies = managerPrivate->queryProxy(QNetworkProxyQuery(newHttpRequest.url()));
    for (const QNetworkProxy &p : proxies) {
        // use the first proxy that works
        // for non-encrypted connections, any transparent or HTTP proxy
        // for encrypted, only transparent proxies
        if (!ssl
            && (p.capabilities() & QNetworkProxy::CachingCapability)
            && (p.type() == QNetworkProxy::HttpProxy ||
                p.type() == QNetworkProxy::HttpCachingProxy)) {
            cacheProxy = p;
            transparentProxy = QNetworkProxy::NoProxy;
            break;
        }
        if (p.isTransparentProxy()) {
            transparentProxy = p;
            cacheProxy = QNetworkProxy::NoProxy;
            break;
        }
    }

    // check if at least one of the proxies
    if (transparentProxy.type() == QNetworkProxy::DefaultProxy &&
        cacheProxy.type() == QNetworkProxy::DefaultProxy) {
        // unsuitable proxies
        QMetaObject::invokeMethod(q, "_q_error", synchronous ? Qt::DirectConnection : Qt::QueuedConnection,
                                  Q_ARG(QNetworkReply::NetworkError, QNetworkReply::ProxyNotFoundError),
                                  Q_ARG(QString, QNetworkReplyHttpImpl::tr("No suitable proxy found")));
        QMetaObject::invokeMethod(q, "_q_finished", synchronous ? Qt::DirectConnection : Qt::QueuedConnection);
        return;
    }
#endif

    auto redirectPolicy = QNetworkRequest::NoLessSafeRedirectPolicy;
    const QVariant value = newHttpRequest.attribute(QNetworkRequest::RedirectPolicyAttribute);
    if (value.isValid())
        redirectPolicy = qvariant_cast<QNetworkRequest::RedirectPolicy>(value);

    httpRequest.setRedirectPolicy(redirectPolicy);

    httpRequest.setPriority(convert(newHttpRequest.priority()));
    loadingFromCache = false;

    switch (operation) {
    case QNetworkAccessManager::GetOperation:
        httpRequest.setOperation(QHttpNetworkRequest::Get);
        if (loadFromCacheIfAllowed(httpRequest))
            return; // no need to send the request! :)
        break;

    case QNetworkAccessManager::HeadOperation:
        httpRequest.setOperation(QHttpNetworkRequest::Head);
        if (loadFromCacheIfAllowed(httpRequest))
            return; // no need to send the request! :)
        break;

    case QNetworkAccessManager::PostOperation:
        invalidateCache();
        httpRequest.setOperation(QHttpNetworkRequest::Post);
        createUploadByteDevice();
        break;

    case QNetworkAccessManager::PutOperation:
        invalidateCache();
        httpRequest.setOperation(QHttpNetworkRequest::Put);
        createUploadByteDevice();
        break;

    case QNetworkAccessManager::DeleteOperation:
        invalidateCache();
        httpRequest.setOperation(QHttpNetworkRequest::Delete);
        break;

    case QNetworkAccessManager::CustomOperation:
        invalidateCache(); // for safety reasons, we don't know what the operation does
        httpRequest.setOperation(QHttpNetworkRequest::Custom);
        createUploadByteDevice();
        httpRequest.setCustomVerb(newHttpRequest.attribute(
                QNetworkRequest::CustomVerbAttribute).toByteArray());
        break;

    default:
        break;                  // can't happen
    }

    QList<QByteArray> headers = newHttpRequest.rawHeaderList();
    if (resumeOffset != 0) {
        const int rangeIndex = headers.indexOf("Range");
        if (rangeIndex != -1) {
            // Need to adjust resume offset for user specified range

            headers.removeAt(rangeIndex);

            // We've already verified that requestRange starts with "bytes=", see canResume.
            QByteArray requestRange = newHttpRequest.rawHeader("Range").mid(6);

            int index = requestRange.indexOf('-');

            quint64 requestStartOffset = requestRange.left(index).toULongLong();
            quint64 requestEndOffset = requestRange.mid(index + 1).toULongLong();

            // In case an end offset is not given it is skipped from the request range
            requestRange = "bytes=" + QByteArray::number(resumeOffset + requestStartOffset) +
                           '-' + (requestEndOffset ? QByteArray::number(requestEndOffset) : QByteArray());

            httpRequest.setHeaderField("Range", requestRange);
        } else {
            httpRequest.setHeaderField("Range", "bytes=" + QByteArray::number(resumeOffset) + '-');
        }
    }

    for (const QByteArray &header : qAsConst(headers))
        httpRequest.setHeaderField(header, newHttpRequest.rawHeader(header));

    if (newHttpRequest.attribute(QNetworkRequest::HttpPipeliningAllowedAttribute).toBool())
        httpRequest.setPipeliningAllowed(true);

    if (auto allowed = request.attribute(QNetworkRequest::Http2AllowedAttribute);
        allowed.isValid() && allowed.canConvert<bool>()) {
        httpRequest.setHTTP2Allowed(allowed.value<bool>());
    }

    if (request.attribute(QNetworkRequest::Http2DirectAttribute).toBool()) {
        // Intentionally mutually exclusive - cannot be both direct and 'allowed'
        httpRequest.setHTTP2Direct(true);
        httpRequest.setHTTP2Allowed(false);
    }

    if (static_cast<QNetworkRequest::LoadControl>
        (newHttpRequest.attribute(QNetworkRequest::AuthenticationReuseAttribute,
                             QNetworkRequest::Automatic).toInt()) == QNetworkRequest::Manual)
        httpRequest.setWithCredentials(false);

    if (request.attribute(QNetworkRequest::EmitAllUploadProgressSignalsAttribute).toBool())
        emitAllUploadProgressSignals = true;

    httpRequest.setMinimumArchiveBombSize(newHttpRequest.decompressedSafetyCheckThreshold());
    httpRequest.setPeerVerifyName(newHttpRequest.peerVerifyName());

    // Create the HTTP thread delegate
    QHttpThreadDelegate *delegate = new QHttpThreadDelegate;
    // Propagate Http/2 settings:
    delegate->http2Parameters = request.http2Configuration();

    // For the synchronous HTTP, this is the normal way the delegate gets deleted
    // For the asynchronous HTTP this is a safety measure, the delegate deletes itself when HTTP is finished
    QMetaObject::Connection threadFinishedConnection =
            QObject::connect(thread, SIGNAL(finished()), delegate, SLOT(deleteLater()));

    // QTBUG-88063: When 'delegate' is deleted the connection will be added to 'thread''s orphaned
    // connections list. This orphaned list will be cleaned up next time 'thread' emits a signal,
    // unfortunately that's the finished signal. It leads to a soft-leak so we do this to disconnect
    // it on deletion so that it cleans up the orphan immediately.
    QObject::connect(delegate, &QObject::destroyed, delegate, [threadFinishedConnection]() {
        if (bool(threadFinishedConnection))
            QObject::disconnect(threadFinishedConnection);
    });

    // Set the properties it needs
    delegate->httpRequest = httpRequest;
#ifndef QT_NO_NETWORKPROXY
    delegate->cacheProxy = cacheProxy;
    delegate->transparentProxy = transparentProxy;
#endif
    delegate->ssl = ssl;
#ifndef QT_NO_SSL
    if (ssl)
        delegate->incomingSslConfiguration.reset(new QSslConfiguration(newHttpRequest.sslConfiguration()));
#endif

    // Do we use synchronous HTTP?
    delegate->synchronous = synchronous;

    // The authentication manager is used to avoid the BlockingQueuedConnection communication
    // from HTTP thread to user thread in some cases.
    delegate->authenticationManager = managerPrivate->authenticationManager;

    if (!synchronous) {
        // Tell our zerocopy policy to the delegate
        QVariant downloadBufferMaximumSizeAttribute = newHttpRequest.attribute(QNetworkRequest::MaximumDownloadBufferSizeAttribute);
        if (downloadBufferMaximumSizeAttribute.isValid()) {
            delegate->downloadBufferMaximumSize = downloadBufferMaximumSizeAttribute.toLongLong();
        } else {
            // If there is no MaximumDownloadBufferSizeAttribute set (which is for the majority
            // of QNetworkRequest) then we can assume we'll do it anyway for small HTTP replies.
            // This helps with performance and memory fragmentation.
            delegate->downloadBufferMaximumSize = 128*1024;
        }


        // These atomic integers are used for signal compression
        delegate->pendingDownloadData = pendingDownloadDataEmissions;
        delegate->pendingDownloadProgress = pendingDownloadProgressEmissions;

        // Connect the signals of the delegate to us
        QObject::connect(delegate, SIGNAL(downloadData(QByteArray)),
                q, SLOT(replyDownloadData(QByteArray)),
                Qt::QueuedConnection);
        QObject::connect(delegate, SIGNAL(downloadFinished()),
                q, SLOT(replyFinished()),
                Qt::QueuedConnection);
        QObject::connect(delegate, SIGNAL(downloadMetaData(QList<QPair<QByteArray,QByteArray> >,
                                                           int, QString, bool,
                                                           QSharedPointer<char>, qint64, qint64,
                                                           bool)),
                q, SLOT(replyDownloadMetaData(QList<QPair<QByteArray,QByteArray> >,
                                              int, QString, bool,
                                              QSharedPointer<char>, qint64, qint64, bool)),
                Qt::QueuedConnection);
        QObject::connect(delegate, SIGNAL(downloadProgress(qint64,qint64)),
                q, SLOT(replyDownloadProgressSlot(qint64,qint64)),
                Qt::QueuedConnection);
        QObject::connect(delegate, SIGNAL(error(QNetworkReply::NetworkError,QString)),
                q, SLOT(httpError(QNetworkReply::NetworkError,QString)),
                Qt::QueuedConnection);
        QObject::connect(delegate, SIGNAL(redirected(QUrl,int,int)),
                q, SLOT(onRedirected(QUrl,int,int)),
                Qt::QueuedConnection);

#ifndef QT_NO_SSL
        QObject::connect(delegate, SIGNAL(sslConfigurationChanged(QSslConfiguration)),
                q, SLOT(replySslConfigurationChanged(QSslConfiguration)),
                Qt::QueuedConnection);
#endif
        // Those need to report back, therefore BlockingQueuedConnection
        QObject::connect(delegate, SIGNAL(authenticationRequired(QHttpNetworkRequest,QAuthenticator*)),
                q, SLOT(httpAuthenticationRequired(QHttpNetworkRequest,QAuthenticator*)),
                Qt::BlockingQueuedConnection);
#ifndef QT_NO_NETWORKPROXY
        QObject::connect(delegate, SIGNAL(proxyAuthenticationRequired(QNetworkProxy,QAuthenticator*)),
                 q, SLOT(proxyAuthenticationRequired(QNetworkProxy,QAuthenticator*)),
                 Qt::BlockingQueuedConnection);
#endif
#ifndef QT_NO_SSL
        QObject::connect(delegate, SIGNAL(encrypted()), q, SLOT(replyEncrypted()),
                Qt::BlockingQueuedConnection);
        QObject::connect(delegate, SIGNAL(sslErrors(QList<QSslError>,bool*,QList<QSslError>*)),
                q, SLOT(replySslErrors(QList<QSslError>,bool*,QList<QSslError>*)),
                Qt::BlockingQueuedConnection);
        QObject::connect(delegate, SIGNAL(preSharedKeyAuthenticationRequired(QSslPreSharedKeyAuthenticator*)),
                         q, SLOT(replyPreSharedKeyAuthenticationRequiredSlot(QSslPreSharedKeyAuthenticator*)),
                         Qt::BlockingQueuedConnection);
#endif
        // This signal we will use to start the request.
        QObject::connect(q, SIGNAL(startHttpRequest()), delegate, SLOT(startRequest()));
        QObject::connect(q, SIGNAL(abortHttpRequest()), delegate, SLOT(abortRequest()));

        // To throttle the connection.
        QObject::connect(q, SIGNAL(readBufferSizeChanged(qint64)), delegate, SLOT(readBufferSizeChanged(qint64)));
        QObject::connect(q, SIGNAL(readBufferFreed(qint64)), delegate, SLOT(readBufferFreed(qint64)));

        if (uploadByteDevice) {
            QNonContiguousByteDeviceThreadForwardImpl *forwardUploadDevice =
                    new QNonContiguousByteDeviceThreadForwardImpl(uploadByteDevice->atEnd(), uploadByteDevice->size());
            forwardUploadDevice->setParent(delegate); // needed to make sure it is moved on moveToThread()
            delegate->httpRequest.setUploadByteDevice(forwardUploadDevice);

            // If the device in the user thread claims it has more data, keep the flow to HTTP thread going
            QObject::connect(uploadByteDevice.data(), SIGNAL(readyRead()),
                             q, SLOT(uploadByteDeviceReadyReadSlot()),
                             Qt::QueuedConnection);

            // From user thread to http thread:
            QObject::connect(q, SIGNAL(haveUploadData(qint64,QByteArray,bool,qint64)),
                             forwardUploadDevice, SLOT(haveDataSlot(qint64,QByteArray,bool,qint64)), Qt::QueuedConnection);
            QObject::connect(uploadByteDevice.data(), SIGNAL(readyRead()),
                             forwardUploadDevice, SIGNAL(readyRead()),
                             Qt::QueuedConnection);

            // From http thread to user thread:
            QObject::connect(forwardUploadDevice, SIGNAL(wantData(qint64)),
                             q, SLOT(wantUploadDataSlot(qint64)));
            QObject::connect(forwardUploadDevice,SIGNAL(processedData(qint64,qint64)),
                             q, SLOT(sentUploadDataSlot(qint64,qint64)));
            QObject::connect(forwardUploadDevice, SIGNAL(resetData(bool*)),
                    q, SLOT(resetUploadDataSlot(bool*)),
                    Qt::BlockingQueuedConnection); // this is the only one with BlockingQueued!
        }
    } else if (synchronous) {
        QObject::connect(q, SIGNAL(startHttpRequestSynchronously()), delegate, SLOT(startRequestSynchronously()), Qt::BlockingQueuedConnection);

        if (uploadByteDevice) {
            // For the synchronous HTTP use case the use thread (this one here) is blocked
            // so we cannot use the asynchronous upload architecture.
            // We therefore won't use the QNonContiguousByteDeviceThreadForwardImpl but directly
            // use the uploadByteDevice provided to us by the QNetworkReplyImpl.
            // The code that is in start() makes sure it is safe to use from a thread
            // since it only wraps a QRingBuffer
            delegate->httpRequest.setUploadByteDevice(uploadByteDevice.data());
        }
    }


    // Move the delegate to the http thread
    delegate->moveToThread(thread);
    // This call automatically moves the uploadDevice too for the asynchronous case.

    // Prepare timers for progress notifications
    downloadProgressSignalChoke.start();
    uploadProgressSignalChoke.invalidate();

    // Send an signal to the delegate so it starts working in the other thread
    if (synchronous) {
        emit q->startHttpRequestSynchronously(); // This one is BlockingQueuedConnection, so it will return when all work is done

        if (delegate->incomingErrorCode != QNetworkReply::NoError) {
            replyDownloadMetaData
                    (delegate->incomingHeaders,
                     delegate->incomingStatusCode,
                     delegate->incomingReasonPhrase,
                     delegate->isPipeliningUsed,
                     QSharedPointer<char>(),
                     delegate->incomingContentLength,
                     delegate->removedContentLength,
                     delegate->isHttp2Used);
            replyDownloadData(delegate->synchronousDownloadData);
            httpError(delegate->incomingErrorCode, delegate->incomingErrorDetail);
        } else {
            replyDownloadMetaData
                    (delegate->incomingHeaders,
                     delegate->incomingStatusCode,
                     delegate->incomingReasonPhrase,
                     delegate->isPipeliningUsed,
                     QSharedPointer<char>(),
                     delegate->incomingContentLength,
                     delegate->removedContentLength,
                     delegate->isHttp2Used);
            replyDownloadData(delegate->synchronousDownloadData);
        }

        thread->quit();
        thread->wait(QDeadlineTimer(5000));
        if (thread->isFinished())
            delete thread;
        else
            QObject::connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

        finished();
    } else {
        emit q->startHttpRequest(); // Signal to the HTTP thread and go back to user.
    }
}

void QNetworkReplyHttpImplPrivate::invalidateCache()
{
    QAbstractNetworkCache *nc = managerPrivate->networkCache;
    if (nc)
        nc->remove(httpRequest.url());
}

void QNetworkReplyHttpImplPrivate::initCacheSaveDevice()
{
    Q_Q(QNetworkReplyHttpImpl);

    // The disk cache does not support partial content, so don't even try to
    // save any such content into the cache.
    if (q->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 206) {
        cacheEnabled = false;
        return;
    }

    // save the meta data
    QNetworkCacheMetaData metaData;
    metaData.setUrl(url);
    metaData = fetchCacheMetaData(metaData);

    // save the redirect request also in the cache
    QVariant redirectionTarget = q->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if (redirectionTarget.isValid()) {
        QNetworkCacheMetaData::AttributesMap attributes = metaData.attributes();
        attributes.insert(QNetworkRequest::RedirectionTargetAttribute, redirectionTarget);
        metaData.setAttributes(attributes);
    }

    cacheSaveDevice = managerPrivate->networkCache->prepare(metaData);

    if (cacheSaveDevice)
        q->connect(cacheSaveDevice, SIGNAL(aboutToClose()), SLOT(_q_cacheSaveDeviceAboutToClose()));

    if (!cacheSaveDevice || (cacheSaveDevice && !cacheSaveDevice->isOpen())) {
        if (Q_UNLIKELY(cacheSaveDevice && !cacheSaveDevice->isOpen()))
            qCritical("QNetworkReplyImpl: network cache returned a device that is not open -- "
                  "class %s probably needs to be fixed",
                  managerPrivate->networkCache->metaObject()->className());

        managerPrivate->networkCache->remove(url);
        cacheSaveDevice = nullptr;
        cacheEnabled = false;
    }
}

void QNetworkReplyHttpImplPrivate::replyDownloadData(QByteArray d)
{
    Q_Q(QNetworkReplyHttpImpl);

    // If we're closed just ignore this data
    if (!q->isOpen())
        return;

    if (cacheEnabled && isCachingAllowed() && !cacheSaveDevice)
        initCacheSaveDevice();

    // This is going to look a little strange. When downloading data while a
    // HTTP redirect is happening (and enabled), we write the redirect
    // response to the cache. However, we do not append it to our internal
    // buffer as that will contain the response data only for the final
    // response
    if (cacheSaveDevice)
        cacheSaveDevice->write(d);

    if (!isHttpRedirectResponse()) {
        buffer.append(d);
        bytesDownloaded += d.size();
        setupTransferTimeout();
    }
    bytesBuffered += d.size();

    int pendingSignals = pendingDownloadDataEmissions->fetchAndSubAcquire(1) - 1;
    if (pendingSignals > 0) {
        // Some more signal emissions to this slot are pending.
        // Instead of writing the downstream data, we wait
        // and do it in the next call we get
        // (signal comppression)
        return;
    }

    if (isHttpRedirectResponse())
        return;

    QVariant totalSize = cookedHeaders.value(QNetworkRequest::ContentLengthHeader);

    emit q->readyRead();
    // emit readyRead before downloadProgress incase this will cause events to be
    // processed and we get into a recursive call (as in QProgressDialog).
    if (downloadProgressSignalChoke.elapsed() >= progressSignalInterval) {
        downloadProgressSignalChoke.restart();
        emit q->downloadProgress(bytesDownloaded,
                             totalSize.isNull() ? Q_INT64_C(-1) : totalSize.toLongLong());
    }

}

void QNetworkReplyHttpImplPrivate::replyFinished()
{
    // We are already loading from cache, we still however
    // got this signal because it was posted already
    if (loadingFromCache)
        return;

    finished();
}

QNetworkAccessManager::Operation QNetworkReplyHttpImplPrivate::getRedirectOperation(QNetworkAccessManager::Operation currentOp, int httpStatus)
{
    // HTTP status code can be used to decide if we can redirect with a GET
    // operation or not. See http://www.ietf.org/rfc/rfc2616.txt [Sec 10.3] for
    // more details

    // We MUST keep using the verb that was used originally when being redirected with 307 or 308.
    if (httpStatus == 307 || httpStatus == 308)
        return currentOp;

    switch (currentOp) {
    case QNetworkAccessManager::HeadOperation:
        return QNetworkAccessManager::HeadOperation;
    default:
        break;
    }
    // Use GET for everything else.
    return QNetworkAccessManager::GetOperation;
}

bool QNetworkReplyHttpImplPrivate::isHttpRedirectResponse() const
{
    return httpRequest.isFollowRedirects() && QHttpNetworkReply::isHttpRedirect(statusCode);
}

QNetworkRequest QNetworkReplyHttpImplPrivate::createRedirectRequest(const QNetworkRequest &originalRequest,
                                                                    const QUrl &url,
                                                                    int maxRedirectsRemaining)
{
    QNetworkRequest newRequest(originalRequest);
    newRequest.setUrl(url);
    newRequest.setMaximumRedirectsAllowed(maxRedirectsRemaining);

    return newRequest;
}

void QNetworkReplyHttpImplPrivate::onRedirected(const QUrl &redirectUrl, int httpStatus, int maxRedirectsRemaining)
{
    Q_Q(QNetworkReplyHttpImpl);
    Q_ASSERT(manager);
    Q_ASSERT(managerPrivate);

    if (isFinished)
        return;

    const QString schemeBefore(url.scheme());
    if (httpRequest.isFollowRedirects()) // update the reply's url as it could've changed
        url = redirectUrl;

    if (managerPrivate->stsEnabled && managerPrivate->stsCache.isKnownHost(url)) {
        // RFC6797, 8.3:
        // The UA MUST replace the URI scheme with "https" [RFC2818],
        // and if the URI contains an explicit port component of "80",
        // then the UA MUST convert the port component to be "443", or
        // if the URI contains an explicit port component that is not
        // equal to "80", the port component value MUST be preserved;
        // otherwise, if the URI does not contain an explicit port
        // component, the UA MUST NOT add one.
        url.setScheme(QLatin1String("https"));
        if (url.port() == 80)
            url.setPort(443);
    }

    const bool isLessSafe = schemeBefore == QLatin1String("https")
                            && url.scheme() == QLatin1String("http");
    if (httpRequest.redirectPolicy() == QNetworkRequest::NoLessSafeRedirectPolicy
        && isLessSafe) {
        error(QNetworkReply::InsecureRedirectError,
              QCoreApplication::translate("QHttp", "Insecure redirect"));
        return;
    }

    redirectRequest = createRedirectRequest(originalRequest, url, maxRedirectsRemaining);
    operation = getRedirectOperation(operation, httpStatus);

    // Clear stale headers, the relevant ones get set again later
    httpRequest.clearHeaders();
    if (operation == QNetworkAccessManager::GetOperation
        || operation == QNetworkAccessManager::HeadOperation) {
        // possibly changed from not-GET/HEAD to GET/HEAD, make sure to get rid of upload device
        uploadByteDevice.reset();
        uploadByteDevicePosition = 0;
        if (outgoingData) {
            QObject::disconnect(outgoingData, SIGNAL(readyRead()), q,
                                SLOT(_q_bufferOutgoingData()));
            QObject::disconnect(outgoingData, SIGNAL(readChannelFinished()), q,
                                SLOT(_q_bufferOutgoingDataFinished()));
        }
        outgoingData = nullptr;
        outgoingDataBuffer.reset();
        // We need to explicitly unset these headers so they're not reapplied to the httpRequest
        redirectRequest.setHeader(QNetworkRequest::ContentLengthHeader, QVariant());
        redirectRequest.setHeader(QNetworkRequest::ContentTypeHeader, QVariant());
    }

    if (const QNetworkCookieJar *const cookieJar = manager->cookieJar()) {
        auto cookies = cookieJar->cookiesForUrl(url);
        if (!cookies.empty()) {
            redirectRequest.setHeader(QNetworkRequest::KnownHeaders::CookieHeader,
                                      QVariant::fromValue(cookies));
        }
    }

    if (httpRequest.redirectPolicy() != QNetworkRequest::UserVerifiedRedirectPolicy)
        followRedirect();

    emit q->redirected(url);
}

void QNetworkReplyHttpImplPrivate::followRedirect()
{
    Q_Q(QNetworkReplyHttpImpl);
    Q_ASSERT(managerPrivate);

    rawHeaders.clear();
    cookedHeaders.clear();

    if (managerPrivate->thread)
        managerPrivate->thread->disconnect();

    QMetaObject::invokeMethod(
            q, [this]() { postRequest(redirectRequest); }, Qt::QueuedConnection);
}

void QNetworkReplyHttpImplPrivate::checkForRedirect(const int statusCode)
{
    Q_Q(QNetworkReplyHttpImpl);
    switch (statusCode) {
    case 301:                   // Moved Permanently
    case 302:                   // Found
    case 303:                   // See Other
    case 307:                   // Temporary Redirect
    case 308:                   // Permanent Redirect
        // What do we do about the caching of the HTML note?
        // The response to a 303 MUST NOT be cached, while the response to
        // all of the others is cacheable if the headers indicate it to be
        QByteArray header = q->rawHeader("location");
        QUrl url = QUrl(QString::fromUtf8(header));
        if (!url.isValid())
            url = QUrl(QLatin1String(header));
        q->setAttribute(QNetworkRequest::RedirectionTargetAttribute, url);
    }
}

void QNetworkReplyHttpImplPrivate::replyDownloadMetaData(const QList<QPair<QByteArray,QByteArray> > &hm,
                                                         int sc, const QString &rp, bool pu,
                                                         QSharedPointer<char> db,
                                                         qint64 contentLength,
                                                         qint64 removedContentLength,
                                                         bool h2Used)
{
    Q_Q(QNetworkReplyHttpImpl);
    Q_UNUSED(contentLength);

    statusCode = sc;
    reasonPhrase = rp;

#ifndef QT_NO_SSL
    // We parse this header only if we're using secure transport:
    //
    // RFC6797, 8.1
    // If an HTTP response is received over insecure transport, the UA MUST
    // ignore any present STS header field(s).
    if (url.scheme() == QLatin1String("https") && managerPrivate->stsEnabled)
        managerPrivate->stsCache.updateFromHeaders(hm, url);
#endif
    // Download buffer
    if (!db.isNull()) {
        downloadBufferPointer = db;
        downloadZerocopyBuffer = downloadBufferPointer.data();
        downloadBufferCurrentSize = 0;
        q->setAttribute(QNetworkRequest::DownloadBufferAttribute, QVariant::fromValue<QSharedPointer<char> > (downloadBufferPointer));
    }

    q->setAttribute(QNetworkRequest::HttpPipeliningWasUsedAttribute, pu);
    q->setAttribute(QNetworkRequest::Http2WasUsedAttribute, h2Used);

    // reconstruct the HTTP header
    QList<QPair<QByteArray, QByteArray> > headerMap = hm;
    QList<QPair<QByteArray, QByteArray> >::ConstIterator it = headerMap.constBegin(),
                                                        end = headerMap.constEnd();
    for (; it != end; ++it) {
        QByteArray value = q->rawHeader(it->first);

        // Reset any previous "location" header set in the reply. In case of
        // redirects, we don't want to 'append' multiple location header values,
        // rather we keep only the latest one
        if (it->first.toLower() == "location")
            value.clear();

        if (!value.isEmpty()) {
            // Why are we appending values for headers which are already
            // present?
            if (it->first.compare("set-cookie", Qt::CaseInsensitive) == 0)
                value += '\n';
            else
                value += ", ";
        }
        value += it->second;
        q->setRawHeader(it->first, value);
    }

    q->setAttribute(QNetworkRequest::HttpStatusCodeAttribute, statusCode);
    q->setAttribute(QNetworkRequest::HttpReasonPhraseAttribute, reasonPhrase);
    if (removedContentLength != -1)
        q->setAttribute(QNetworkRequest::OriginalContentLengthAttribute, removedContentLength);

    // is it a redirection?
    if (!isHttpRedirectResponse())
        checkForRedirect(statusCode);

    if (statusCode >= 500 && statusCode < 600) {
        QAbstractNetworkCache *nc = managerPrivate->networkCache;
        if (nc) {
            QNetworkCacheMetaData metaData = nc->metaData(httpRequest.url());
            QNetworkHeadersPrivate cacheHeaders;
            cacheHeaders.setAllRawHeaders(metaData.rawHeaders());
            QNetworkHeadersPrivate::RawHeadersList::ConstIterator it;
            it = cacheHeaders.findRawHeader("Cache-Control");
            bool mustReValidate = false;
            if (it != cacheHeaders.rawHeaders.constEnd()) {
                QHash<QByteArray, QByteArray> cacheControl = parseHttpOptionHeader(it->second);
                if (cacheControl.contains("must-revalidate"))
                    mustReValidate = true;
            }
            if (!mustReValidate && sendCacheContents(metaData))
                return;
        }
    }

    if (statusCode == 304) {
#if defined(QNETWORKACCESSHTTPBACKEND_DEBUG)
        qDebug() << "Received a 304 from" << request.url();
#endif
        QAbstractNetworkCache *nc = managerPrivate->networkCache;
        if (nc) {
            QNetworkCacheMetaData oldMetaData = nc->metaData(httpRequest.url());
            QNetworkCacheMetaData metaData = fetchCacheMetaData(oldMetaData);
            if (oldMetaData != metaData)
                nc->updateMetaData(metaData);
            if (sendCacheContents(metaData))
                return;
        }
    }


    if (statusCode != 304 && statusCode != 303) {
        if (!isCachingEnabled())
            setCachingEnabled(true);
    }

    _q_metaDataChanged();
}

void QNetworkReplyHttpImplPrivate::replyDownloadProgressSlot(qint64 bytesReceived,  qint64 bytesTotal)
{
    Q_Q(QNetworkReplyHttpImpl);

    // If we're closed just ignore this data
    if (!q->isOpen())
        return;

    // we can be sure here that there is a download buffer

    int pendingSignals = (int)pendingDownloadProgressEmissions->fetchAndAddAcquire(-1) - 1;
    if (pendingSignals > 0) {
        // Let's ignore this signal and look at the next one coming in
        // (signal comppression)
        return;
    }

    if (!q->isOpen())
        return;

    if (cacheEnabled && isCachingAllowed() && bytesReceived == bytesTotal) {
        // Write everything in one go if we use a download buffer. might be more performant.
        initCacheSaveDevice();
        // need to check again if cache enabled and device exists
        if (cacheSaveDevice && cacheEnabled)
            cacheSaveDevice->write(downloadZerocopyBuffer, bytesTotal);
        // FIXME where is it closed?
    }

    if (isHttpRedirectResponse())
        return;

    bytesDownloaded = bytesReceived;
    setupTransferTimeout();

    downloadBufferCurrentSize = bytesReceived;

    // Only emit readyRead when actual data is there
    // emit readyRead before downloadProgress incase this will cause events to be
    // processed and we get into a recursive call (as in QProgressDialog).
    if (bytesDownloaded > 0)
        emit q->readyRead();
    if (downloadProgressSignalChoke.elapsed() >= progressSignalInterval) {
        downloadProgressSignalChoke.restart();
        emit q->downloadProgress(bytesDownloaded, bytesTotal);
    }
}

void QNetworkReplyHttpImplPrivate::httpAuthenticationRequired(const QHttpNetworkRequest &request,
                                                           QAuthenticator *auth)
{
    managerPrivate->authenticationRequired(auth, q_func(), synchronous, url, &urlForLastAuthentication, request.withCredentials());
}

#ifndef QT_NO_NETWORKPROXY
void QNetworkReplyHttpImplPrivate::proxyAuthenticationRequired(const QNetworkProxy &proxy,
                                                        QAuthenticator *authenticator)
{
    managerPrivate->proxyAuthenticationRequired(request.url(), proxy, synchronous, authenticator, &lastProxyAuthentication);
}
#endif

void QNetworkReplyHttpImplPrivate::httpError(QNetworkReply::NetworkError errorCode,
                                          const QString &errorString)
{
#if defined(QNETWORKACCESSHTTPBACKEND_DEBUG)
    qDebug() << "http error!" << errorCode << errorString;
#endif

    // FIXME?
    error(errorCode, errorString);
}

#ifndef QT_NO_SSL
void QNetworkReplyHttpImplPrivate::replyEncrypted()
{
    Q_Q(QNetworkReplyHttpImpl);
    emit q->encrypted();
}

void QNetworkReplyHttpImplPrivate::replySslErrors(
        const QList<QSslError> &list, bool *ignoreAll, QList<QSslError> *toBeIgnored)
{
    Q_Q(QNetworkReplyHttpImpl);
    emit q->sslErrors(list);
    // Check if the callback set any ignore and return this here to http thread
    if (pendingIgnoreAllSslErrors)
        *ignoreAll = true;
    if (!pendingIgnoreSslErrorsList.isEmpty())
        *toBeIgnored = pendingIgnoreSslErrorsList;
}

void QNetworkReplyHttpImplPrivate::replySslConfigurationChanged(const QSslConfiguration &newSslConfiguration)
{
    // Receiving the used SSL configuration from the HTTP thread
    if (sslConfiguration.data())
        *sslConfiguration = newSslConfiguration;
    else
        sslConfiguration.reset(new QSslConfiguration(newSslConfiguration));
}

void QNetworkReplyHttpImplPrivate::replyPreSharedKeyAuthenticationRequiredSlot(QSslPreSharedKeyAuthenticator *authenticator)
{
    Q_Q(QNetworkReplyHttpImpl);
    emit q->preSharedKeyAuthenticationRequired(authenticator);
}
#endif

// Coming from QNonContiguousByteDeviceThreadForwardImpl in HTTP thread
void QNetworkReplyHttpImplPrivate::resetUploadDataSlot(bool *r)
{
    *r = uploadByteDevice->reset();
    if (*r) {
        // reset our own position which is used for the inter-thread communication
        uploadByteDevicePosition = 0;
    }
}

// Coming from QNonContiguousByteDeviceThreadForwardImpl in HTTP thread
void QNetworkReplyHttpImplPrivate::sentUploadDataSlot(qint64 pos, qint64 amount)
{
    if (!uploadByteDevice) // uploadByteDevice is no longer available
        return;

    if (uploadByteDevicePosition + amount != pos) {
        // Sanity check, should not happen.
        error(QNetworkReply::UnknownNetworkError, QString());
        return;
    }
    uploadByteDevice->advanceReadPointer(amount);
    uploadByteDevicePosition += amount;
}

// Coming from QNonContiguousByteDeviceThreadForwardImpl in HTTP thread
void QNetworkReplyHttpImplPrivate::wantUploadDataSlot(qint64 maxSize)
{
    Q_Q(QNetworkReplyHttpImpl);

    if (!uploadByteDevice) // uploadByteDevice is no longer available
        return;

    // call readPointer
    qint64 currentUploadDataLength = 0;
    char *data = const_cast<char*>(uploadByteDevice->readPointer(maxSize, currentUploadDataLength));

    if (currentUploadDataLength == 0) {
        uploadDeviceChoking = true;
        // No bytes from upload byte device. There will be bytes later, it will emit readyRead()
        // and our uploadByteDeviceReadyReadSlot() is called.
        return;
    } else {
        uploadDeviceChoking = false;
    }

    // Let's make a copy of this data
    QByteArray dataArray(data, currentUploadDataLength);

    // Communicate back to HTTP thread
    emit q->haveUploadData(uploadByteDevicePosition, dataArray, uploadByteDevice->atEnd(), uploadByteDevice->size());
}

void QNetworkReplyHttpImplPrivate::uploadByteDeviceReadyReadSlot()
{
    // Start the flow between this thread and the HTTP thread again by triggering a upload.
    // However only do this when we were choking before, else the state in
    // QNonContiguousByteDeviceThreadForwardImpl gets messed up.
    if (uploadDeviceChoking) {
        uploadDeviceChoking = false;
        wantUploadDataSlot(1024);
    }
}


/*
    A simple web page that can be used to test us: http://www.procata.com/cachetest/
 */
bool QNetworkReplyHttpImplPrivate::sendCacheContents(const QNetworkCacheMetaData &metaData)
{
    Q_Q(QNetworkReplyHttpImpl);

    setCachingEnabled(false);
    if (!metaData.isValid())
        return false;

    QAbstractNetworkCache *nc = managerPrivate->networkCache;
    Q_ASSERT(nc);
    QIODevice *contents = nc->data(url);
    if (!contents) {
#if defined(QNETWORKACCESSHTTPBACKEND_DEBUG)
        qDebug() << "Cannot send cache, the contents are 0" << url;
#endif
        return false;
    }
    contents->setParent(q);

    QNetworkCacheMetaData::AttributesMap attributes = metaData.attributes();
    int status = attributes.value(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (status < 100)
        status = 200;           // fake it

    statusCode = status;

    q->setAttribute(QNetworkRequest::HttpStatusCodeAttribute, status);
    q->setAttribute(QNetworkRequest::HttpReasonPhraseAttribute, attributes.value(QNetworkRequest::HttpReasonPhraseAttribute));
    q->setAttribute(QNetworkRequest::SourceIsFromCacheAttribute, true);

    QNetworkCacheMetaData::RawHeaderList rawHeaders = metaData.rawHeaders();
    QNetworkCacheMetaData::RawHeaderList::ConstIterator it = rawHeaders.constBegin(),
                                                       end = rawHeaders.constEnd();
    QUrl redirectUrl;
    for ( ; it != end; ++it) {
        if (httpRequest.isFollowRedirects() &&
            !it->first.compare("location", Qt::CaseInsensitive))
            redirectUrl = QUrl::fromEncoded(it->second);
        setRawHeader(it->first, it->second);
    }

    if (!isHttpRedirectResponse())
        checkForRedirect(status);

    cacheLoadDevice = contents;
    q->connect(cacheLoadDevice, SIGNAL(readyRead()), SLOT(_q_cacheLoadReadyRead()));
    q->connect(cacheLoadDevice, SIGNAL(readChannelFinished()), SLOT(_q_cacheLoadReadyRead()));

    // This needs to be emitted in the event loop because it can be reached at
    // the direct code path of qnam.get(...) before the user has a chance
    // to connect any signals.
    QMetaObject::invokeMethod(q, "_q_metaDataChanged", Qt::QueuedConnection);
    QMetaObject::invokeMethod(q, "_q_cacheLoadReadyRead", Qt::QueuedConnection);


#if defined(QNETWORKACCESSHTTPBACKEND_DEBUG)
    qDebug() << "Successfully sent cache:" << url << contents->size() << "bytes";
#endif

    // Do redirect processing
    if (httpRequest.isFollowRedirects() && QHttpNetworkReply::isHttpRedirect(status)) {
        QMetaObject::invokeMethod(q, "onRedirected", Qt::QueuedConnection,
                                  Q_ARG(QUrl, redirectUrl),
                                  Q_ARG(int, status),
                                  Q_ARG(int, httpRequest.redirectCount() - 1));
    }

    // Set the following flag so we can ignore some signals from HTTP thread
    // that would still come
    loadingFromCache = true;
    return true;
}

QNetworkCacheMetaData QNetworkReplyHttpImplPrivate::fetchCacheMetaData(const QNetworkCacheMetaData &oldMetaData) const
{
    Q_Q(const QNetworkReplyHttpImpl);

    QNetworkCacheMetaData metaData = oldMetaData;

    QNetworkHeadersPrivate cacheHeaders;
    cacheHeaders.setAllRawHeaders(metaData.rawHeaders());
    QNetworkHeadersPrivate::RawHeadersList::ConstIterator it;

    const QList<QByteArray> newHeaders = q->rawHeaderList();
    for (QByteArray header : newHeaders) {
        QByteArray originalHeader = header;
        header = header.toLower();
        bool hop_by_hop =
            (header == "connection"
             || header == "keep-alive"
             || header == "proxy-authenticate"
             || header == "proxy-authorization"
             || header == "te"
             || header == "trailers"
             || header == "transfer-encoding"
             || header ==  "upgrade");
        if (hop_by_hop)
            continue;

        if (header == "set-cookie")
            continue;

        // for 4.6.0, we were planning to not store the date header in the
        // cached resource; through that we planned to reduce the number
        // of writes to disk when using a QNetworkDiskCache (i.e. don't
        // write to disk when only the date changes).
        // However, without the date we cannot calculate the age of the page
        // anymore.
        //if (header == "date")
            //continue;

        // Don't store Warning 1xx headers
        if (header == "warning") {
            QByteArray v = q->rawHeader(header);
            if (v.length() == 3
                && v[0] == '1'
                && v[1] >= '0' && v[1] <= '9'
                && v[2] >= '0' && v[2] <= '9')
                continue;
        }

        it = cacheHeaders.findRawHeader(header);
        if (it != cacheHeaders.rawHeaders.constEnd()) {
            // Match the behavior of Firefox and assume Cache-Control: "no-transform"
            if (header == "content-encoding"
                || header == "content-range"
                || header == "content-type")
                continue;
        }

        // IIS has been known to send "Content-Length: 0" on 304 responses, so
        // ignore this too
        if (header == "content-length" && statusCode == 304)
            continue;

#if defined(QNETWORKACCESSHTTPBACKEND_DEBUG)
        QByteArray n = q->rawHeader(header);
        QByteArray o;
        if (it != cacheHeaders.rawHeaders.constEnd())
            o = (*it).second;
        if (n != o && header != "date") {
            qDebug() << "replacing" << header;
            qDebug() << "new" << n;
            qDebug() << "old" << o;
        }
#endif
        cacheHeaders.setRawHeader(originalHeader, q->rawHeader(header));
    }
    metaData.setRawHeaders(cacheHeaders.rawHeaders);

    bool checkExpired = true;

    QHash<QByteArray, QByteArray> cacheControl;
    it = cacheHeaders.findRawHeader("Cache-Control");
    if (it != cacheHeaders.rawHeaders.constEnd()) {
        cacheControl = parseHttpOptionHeader(it->second);
        QByteArray maxAge = cacheControl.value("max-age");
        if (!maxAge.isEmpty()) {
            checkExpired = false;
            QDateTime dt = QDateTime::currentDateTimeUtc();
            dt = dt.addSecs(maxAge.toInt());
            metaData.setExpirationDate(dt);
        }
    }
    if (checkExpired) {
        it = cacheHeaders.findRawHeader("expires");
        if (it != cacheHeaders.rawHeaders.constEnd()) {
            QDateTime expiredDateTime = QNetworkHeadersPrivate::fromHttpDate(it->second);
            metaData.setExpirationDate(expiredDateTime);
        }
    }

    it = cacheHeaders.findRawHeader("last-modified");
    if (it != cacheHeaders.rawHeaders.constEnd())
        metaData.setLastModified(QNetworkHeadersPrivate::fromHttpDate(it->second));

    bool canDiskCache;
    // only cache GET replies by default, all other replies (POST, PUT, DELETE)
    //  are not cacheable by default (according to RFC 2616 section 9)
    if (httpRequest.operation() == QHttpNetworkRequest::Get) {

        canDiskCache = true;
        // HTTP/1.1. Check the Cache-Control header
        if (cacheControl.contains("no-store"))
            canDiskCache = false;

    // responses to POST might be cacheable
    } else if (httpRequest.operation() == QHttpNetworkRequest::Post) {

        canDiskCache = false;
        // some pages contain "expires:" and "cache-control: no-cache" field,
        // so we only might cache POST requests if we get "cache-control: max-age ..."
        if (cacheControl.contains("max-age"))
            canDiskCache = true;

    // responses to PUT and DELETE are not cacheable
    } else {
        canDiskCache = false;
    }

    metaData.setSaveToDisk(canDiskCache);
    QNetworkCacheMetaData::AttributesMap attributes;
    if (statusCode != 304) {
        // update the status code
        attributes.insert(QNetworkRequest::HttpStatusCodeAttribute, statusCode);
        attributes.insert(QNetworkRequest::HttpReasonPhraseAttribute, reasonPhrase);
    } else {
        // this is a redirection, keep the attributes intact
        attributes = oldMetaData.attributes();
    }
    metaData.setAttributes(attributes);
    return metaData;
}

bool QNetworkReplyHttpImplPrivate::canResume() const
{
    Q_Q(const QNetworkReplyHttpImpl);

    // Only GET operation supports resuming.
    if (operation != QNetworkAccessManager::GetOperation)
        return false;

    // Can only resume if server/resource supports Range header.
    QByteArray acceptRangesheaderName("Accept-Ranges");
    if (!q->hasRawHeader(acceptRangesheaderName) || q->rawHeader(acceptRangesheaderName) == "none")
        return false;

    // We only support resuming for byte ranges.
    if (request.hasRawHeader("Range")) {
        QByteArray range = request.rawHeader("Range");
        if (!range.startsWith("bytes="))
            return false;
    }

    // If we're using a download buffer then we don't support resuming/migration
    // right now. Too much trouble.
    if (downloadZerocopyBuffer)
        return false;

    return true;
}

void QNetworkReplyHttpImplPrivate::setResumeOffset(quint64 offset)
{
    resumeOffset = offset;
}

void QNetworkReplyHttpImplPrivate::_q_startOperation()
{
    if (state == Working) // ensure this function is only being called once
        return;

    state = Working;

    postRequest(request);

    setupTransferTimeout();
    if (synchronous) {
        state = Finished;
        q_func()->setFinished(true);
    }
}

void QNetworkReplyHttpImplPrivate::_q_cacheLoadReadyRead()
{
    Q_Q(QNetworkReplyHttpImpl);

    if (state != Working)
        return;
    if (!cacheLoadDevice || !q->isOpen() || !cacheLoadDevice->bytesAvailable())
        return;

    // FIXME Optimize to use zerocopy download buffer if it is a QBuffer.
    // Needs to be done where sendCacheContents() (?) of HTTP is emitting
    // metaDataChanged ?


    QVariant totalSize = cookedHeaders.value(QNetworkRequest::ContentLengthHeader);

    // emit readyRead before downloadProgress incase this will cause events to be
    // processed and we get into a recursive call (as in QProgressDialog).

    if (!(isHttpRedirectResponse())) {
        // This readyRead() goes to the user. The user then may or may not read() anything.
        emit q->readyRead();

        if (downloadProgressSignalChoke.elapsed() >= progressSignalInterval) {
            downloadProgressSignalChoke.restart();
            emit q->downloadProgress(bytesDownloaded,
                                     totalSize.isNull() ? Q_INT64_C(-1) : totalSize.toLongLong());
        }
    }

    // A signal we've emitted might be handled by a slot that aborts,
    // so we need to check for that and bail out if it's happened:
    if (!q->isOpen())
        return;

    // If there are still bytes available in the cacheLoadDevice then the user did not read
    // in response to the readyRead() signal. This means we have to load from the cacheLoadDevice
    // and buffer that stuff. This is needed to be able to properly emit finished() later.
    while (cacheLoadDevice->bytesAvailable() && !isHttpRedirectResponse())
        buffer.append(cacheLoadDevice->readAll());

    if (cacheLoadDevice->isSequential()) {
        // check if end and we can read the EOF -1
        char c;
        qint64 actualCount = cacheLoadDevice->read(&c, 1);
        if (actualCount < 0) {
            cacheLoadDevice->deleteLater();
            cacheLoadDevice = nullptr;
            QMetaObject::invokeMethod(q, "_q_finished", Qt::QueuedConnection);
        } else if (actualCount == 1) {
            // This is most probably not happening since most QIODevice returned something proper for bytesAvailable()
            // and had already been "emptied".
            cacheLoadDevice->ungetChar(c);
        }
    } else if ((!cacheLoadDevice->isSequential() && cacheLoadDevice->atEnd())) {
        // This codepath is in case the cache device is a QBuffer, e.g. from QNetworkDiskCache.
        cacheLoadDevice->deleteLater();
        cacheLoadDevice = nullptr;
        QMetaObject::invokeMethod(q, "_q_finished", Qt::QueuedConnection);
    }
}


void QNetworkReplyHttpImplPrivate::_q_bufferOutgoingDataFinished()
{
    Q_Q(QNetworkReplyHttpImpl);

    // make sure this is only called once, ever.
    //_q_bufferOutgoingData may call it or the readChannelFinished emission
    if (state != Buffering)
        return;

    // disconnect signals
    QObject::disconnect(outgoingData, SIGNAL(readyRead()), q, SLOT(_q_bufferOutgoingData()));
    QObject::disconnect(outgoingData, SIGNAL(readChannelFinished()), q, SLOT(_q_bufferOutgoingDataFinished()));

    // finally, start the request
    QMetaObject::invokeMethod(q, "_q_startOperation", Qt::QueuedConnection);
}

void QNetworkReplyHttpImplPrivate::_q_cacheSaveDeviceAboutToClose()
{
    // do not keep a dangling pointer to the device around (device
    // is closing because e.g. QAbstractNetworkCache::remove() was called).
    cacheSaveDevice = nullptr;
}

void QNetworkReplyHttpImplPrivate::_q_bufferOutgoingData()
{
    Q_Q(QNetworkReplyHttpImpl);

    if (!outgoingDataBuffer) {
        // first call, create our buffer
        outgoingDataBuffer = QSharedPointer<QRingBuffer>::create();

        QObject::connect(outgoingData, SIGNAL(readyRead()), q, SLOT(_q_bufferOutgoingData()));
        QObject::connect(outgoingData, SIGNAL(readChannelFinished()), q, SLOT(_q_bufferOutgoingDataFinished()));
    }

    qint64 bytesBuffered = 0;
    qint64 bytesToBuffer = 0;

    // read data into our buffer
    forever {
        bytesToBuffer = outgoingData->bytesAvailable();
        // unknown? just try 2 kB, this also ensures we always try to read the EOF
        if (bytesToBuffer <= 0)
            bytesToBuffer = 2*1024;

        char *dst = outgoingDataBuffer->reserve(bytesToBuffer);
        bytesBuffered = outgoingData->read(dst, bytesToBuffer);

        if (bytesBuffered == -1) {
            // EOF has been reached.
            outgoingDataBuffer->chop(bytesToBuffer);

            _q_bufferOutgoingDataFinished();
            break;
        } else if (bytesBuffered == 0) {
            // nothing read right now, just wait until we get called again
            outgoingDataBuffer->chop(bytesToBuffer);

            break;
        } else {
            // don't break, try to read() again
            outgoingDataBuffer->chop(bytesToBuffer - bytesBuffered);
        }
    }
}

void QNetworkReplyHttpImplPrivate::_q_transferTimedOut()
{
    Q_Q(QNetworkReplyHttpImpl);
    q->abort();
}

void QNetworkReplyHttpImplPrivate::setupTransferTimeout()
{
    Q_Q(QNetworkReplyHttpImpl);
    if (!transferTimeout) {
      transferTimeout = new QTimer(q);
      QObject::connect(transferTimeout, SIGNAL(timeout()),
                       q, SLOT(_q_transferTimedOut()),
                       Qt::QueuedConnection);
    }
    transferTimeout->stop();
    if (request.transferTimeout()) {
        transferTimeout->setSingleShot(true);
        transferTimeout->setInterval(request.transferTimeout());
        QMetaObject::invokeMethod(transferTimeout, "start",
                                  Qt::QueuedConnection);

    }
}

// need to have this function since the reply is a private member variable
// and the special backends need to access this.
void QNetworkReplyHttpImplPrivate::emitReplyUploadProgress(qint64 bytesSent, qint64 bytesTotal)
{
    Q_Q(QNetworkReplyHttpImpl);
    if (isFinished)
        return;

    setupTransferTimeout();

    if (!emitAllUploadProgressSignals) {
        //choke signal emissions, except the first and last signals which are unconditional
        if (uploadProgressSignalChoke.isValid()) {
            if (bytesSent != bytesTotal && uploadProgressSignalChoke.elapsed() < progressSignalInterval) {
                return;
            }
            uploadProgressSignalChoke.restart();
        } else {
            uploadProgressSignalChoke.start();
        }
    }
    emit q->uploadProgress(bytesSent, bytesTotal);
}

QNonContiguousByteDevice* QNetworkReplyHttpImplPrivate::createUploadByteDevice()
{
    Q_Q(QNetworkReplyHttpImpl);

    if (outgoingDataBuffer)
        uploadByteDevice = QNonContiguousByteDeviceFactory::createShared(outgoingDataBuffer);
    else if (outgoingData) {
        uploadByteDevice = QNonContiguousByteDeviceFactory::createShared(outgoingData);
    } else {
        return nullptr;
    }

    // We want signal emissions only for normal asynchronous uploads
    if (!synchronous)
        QObject::connect(uploadByteDevice.data(), SIGNAL(readProgress(qint64,qint64)),
                         q, SLOT(emitReplyUploadProgress(qint64,qint64)));

    return uploadByteDevice.data();
}

void QNetworkReplyHttpImplPrivate::_q_finished()
{
    // This gets called queued, just forward to real call then
    finished();
}

void QNetworkReplyHttpImplPrivate::finished()
{
    Q_Q(QNetworkReplyHttpImpl);
    if (transferTimeout)
      transferTimeout->stop();
    if (state == Finished || state == Aborted)
        return;

    QVariant totalSize = cookedHeaders.value(QNetworkRequest::ContentLengthHeader);

    // if we don't know the total size of or we received everything save the cache
    if (totalSize.isNull() || totalSize == -1 || bytesDownloaded == totalSize)
        completeCacheSave();

    // We check for errorCode too as in case of SSL handshake failure, we still
    // get the HTTP redirect status code (301, 303 etc)
    if (isHttpRedirectResponse() && errorCode == QNetworkReply::NoError)
        return;

    state = Finished;
    q->setFinished(true);

    if (totalSize.isNull() || totalSize == -1) {
        emit q->downloadProgress(bytesDownloaded, bytesDownloaded);
    } else {
        emit q->downloadProgress(bytesDownloaded, totalSize.toLongLong());
    }

    if (bytesUploaded == -1 && (outgoingData || outgoingDataBuffer))
        emit q->uploadProgress(0, 0);

    emit q->readChannelFinished();
    emit q->finished();
}

void QNetworkReplyHttpImplPrivate::_q_error(QNetworkReplyImpl::NetworkError code, const QString &errorMessage)
{
    this->error(code, errorMessage);
}


void QNetworkReplyHttpImplPrivate::error(QNetworkReplyImpl::NetworkError code, const QString &errorMessage)
{
    Q_Q(QNetworkReplyHttpImpl);
    // Can't set and emit multiple errors.
    if (errorCode != QNetworkReply::NoError) {
        qWarning("QNetworkReplyImplPrivate::error: Internal problem, this method must only be called once.");
        return;
    }

    errorCode = code;
    q->setErrorString(errorMessage);

    // note: might not be a good idea, since users could decide to delete us
    // which would delete the backend too...
    // maybe we should protect the backend
    emit q->errorOccurred(code);
}

void QNetworkReplyHttpImplPrivate::_q_metaDataChanged()
{
    // FIXME merge this with replyDownloadMetaData(); ?

    Q_Q(QNetworkReplyHttpImpl);
    // 1. do we have cookies?
    // 2. are we allowed to set them?
    Q_ASSERT(manager);
    const auto it = cookedHeaders.constFind(QNetworkRequest::SetCookieHeader);
    if (it != cookedHeaders.cend()
        && request.attribute(QNetworkRequest::CookieSaveControlAttribute,
                                QNetworkRequest::Automatic).toInt() == QNetworkRequest::Automatic) {
        QNetworkCookieJar *jar = manager->cookieJar();
        if (jar) {
            QList<QNetworkCookie> cookies =
                qvariant_cast<QList<QNetworkCookie> >(it.value());
            jar->setCookiesFromUrl(cookies, url);
        }
    }
    emit q->metaDataChanged();
}

void QNetworkReplyHttpImplPrivate::createCache()
{
    // check if we can save and if we're allowed to
    if (!managerPrivate->networkCache
        || !request.attribute(QNetworkRequest::CacheSaveControlAttribute, true).toBool())
        return;
    cacheEnabled = true;
}

bool QNetworkReplyHttpImplPrivate::isCachingEnabled() const
{
    return (cacheEnabled && managerPrivate->networkCache != nullptr);
}

void QNetworkReplyHttpImplPrivate::setCachingEnabled(bool enable)
{
    if (!enable && !cacheEnabled)
        return;                 // nothing to do
    if (enable && cacheEnabled)
        return;                 // nothing to do either!

    if (enable) {
        if (Q_UNLIKELY(bytesDownloaded)) {
            qDebug() << "setCachingEnabled: " << bytesDownloaded << " bytesDownloaded";
            // refuse to enable in this case
            qCritical("QNetworkReplyImpl: backend error: caching was enabled after some bytes had been written");
            return;
        }

        createCache();
    } else {
        // someone told us to turn on, then back off?
        // ok... but you should make up your mind
        qDebug("QNetworkReplyImpl: setCachingEnabled(true) called after setCachingEnabled(false)");
        managerPrivate->networkCache->remove(url);
        cacheSaveDevice = nullptr;
        cacheEnabled = false;
    }
}

bool QNetworkReplyHttpImplPrivate::isCachingAllowed() const
{
    return operation == QNetworkAccessManager::GetOperation || operation == QNetworkAccessManager::HeadOperation;
}

void QNetworkReplyHttpImplPrivate::completeCacheSave()
{
    if (cacheEnabled && errorCode != QNetworkReplyImpl::NoError) {
        managerPrivate->networkCache->remove(url);
    } else if (cacheEnabled && cacheSaveDevice) {
        managerPrivate->networkCache->insert(cacheSaveDevice);
    }
    cacheSaveDevice = nullptr;
    cacheEnabled = false;
}

QT_END_NAMESPACE
