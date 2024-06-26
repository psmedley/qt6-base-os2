/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Copyright (C) 2015 ownCloud Inc
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

#include <QtNetwork/private/qtlsbackend_p.h>

#include <QtNetwork/qsslcertificate.h>

#include <QtCore/qloggingcategory.h>
#include <QtCore/qglobal.h>
#include <QtCore/qdebug.h>


#ifdef Q_OS_MACOS

#include <QtCore/private/qcore_mac_p.h>

#include <CoreFoundation/CFArray.h>
#include <Security/Security.h>

#endif // Q_OS_MACOS

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcX509, "qt.mac.shared.x509");

#ifdef Q_OS_MACOS
namespace {

bool hasTrustedSslServerPolicy(SecPolicyRef policy, CFDictionaryRef props) {
    QCFType<CFDictionaryRef> policyProps = SecPolicyCopyProperties(policy);
    // only accept certificates with policies for SSL server validation for now
    if (CFEqual(CFDictionaryGetValue(policyProps, kSecPolicyOid), kSecPolicyAppleSSL)) {
        CFBooleanRef policyClient;
        if (CFDictionaryGetValueIfPresent(policyProps, kSecPolicyClient, reinterpret_cast<const void**>(&policyClient)) &&
            CFEqual(policyClient, kCFBooleanTrue)) {
            return false; // no client certs
        }
        if (!CFDictionaryContainsKey(props, kSecTrustSettingsResult)) {
            // as per the docs, no trust settings result implies full trust
            return true;
        }
        CFNumberRef number = static_cast<CFNumberRef>(CFDictionaryGetValue(props, kSecTrustSettingsResult));
        SecTrustSettingsResult settingsResult;
        CFNumberGetValue(number, kCFNumberSInt32Type, &settingsResult);
        switch (settingsResult) {
        case kSecTrustSettingsResultTrustRoot:
        case kSecTrustSettingsResultTrustAsRoot:
            return true;
        default:
            return false;
        }
    }
    return false;
}

bool isCaCertificateTrusted(SecCertificateRef cfCert, int domain)
{
    QCFType<CFArrayRef> cfTrustSettings;
    OSStatus status = SecTrustSettingsCopyTrustSettings(cfCert, SecTrustSettingsDomain(domain), &cfTrustSettings);
    if (status == noErr) {
        CFIndex size = CFArrayGetCount(cfTrustSettings);
        // if empty, trust for everything (as per the Security Framework documentation)
        if (size == 0) {
            return true;
        } else {
            for (CFIndex i = 0; i < size; ++i) {
                CFDictionaryRef props = static_cast<CFDictionaryRef>(CFArrayGetValueAtIndex(cfTrustSettings, i));
                if (CFDictionaryContainsKey(props, kSecTrustSettingsPolicy)) {
                    if (hasTrustedSslServerPolicy((SecPolicyRef)CFDictionaryGetValue(props, kSecTrustSettingsPolicy), props))
                        return true;
                }
            }
        }
    }

    return false;
}

bool canDERBeParsed(CFDataRef derData, const QSslCertificate &qtCert)
{
    // We are observing certificates, that while accepted when we copy them
    // from the keychain(s), later give us 'Failed to create SslCertificate
    // from QSslCertificate'. It's interesting to know at what step the failure
    // occurred. Let's check it and skip it below if it's not valid.

    auto checkDer = [](CFDataRef derData, const char *source)
    {
        Q_ASSERT(source);
        Q_ASSERT(derData);

        const auto cfLength = CFDataGetLength(derData);
        if (cfLength <= 0) {
            qCWarning(lcX509) << source << "returned faulty DER data with invalid length.";
            return false;
        }

        QCFType<SecCertificateRef> secRef = SecCertificateCreateWithData(nullptr, derData);
        if (!secRef) {
            qCWarning(lcX509) << source << "returned faulty DER data which cannot be parsed back.";
            return false;
        }
        return true;
    };

    if (!checkDer(derData, "SecCertificateCopyData")) {
        qCDebug(lcX509) << "Faulty QSslCertificate is:" << qtCert;// Just in case we managed to parse something.
        return false;
    }

    // Generic parser failed?
    if (qtCert.isNull()) {
        qCWarning(lcX509, "QSslCertificate failed to parse DER");
        return false;
    }

    const QCFType<CFDataRef> qtDerData = qtCert.toDer().toCFData();
    if (!checkDer(qtDerData, "QSslCertificate")) {
        qCWarning(lcX509) << "Faulty QSslCertificate is:" << qtCert;
        return false;
    }

    return true;
}

} // unnamed namespace
#endif // Q_OS_MACOS

namespace QTlsPrivate {
QList<QSslCertificate> systemCaCertificates()
{
    QList<QSslCertificate> systemCerts;
    // SecTrustSettingsCopyCertificates is not defined on iOS.
#ifdef Q_OS_MACOS
    // iterate through all enum members, order:
    // kSecTrustSettingsDomainUser, kSecTrustSettingsDomainAdmin, kSecTrustSettingsDomainSystem
    for (int dom = kSecTrustSettingsDomainUser; dom <= int(kSecTrustSettingsDomainSystem); dom++) {
        QCFType<CFArrayRef> cfCerts;
        OSStatus status = SecTrustSettingsCopyCertificates(SecTrustSettingsDomain(dom), &cfCerts);
        if (status == noErr) {
            const CFIndex size = CFArrayGetCount(cfCerts);
            for (CFIndex i = 0; i < size; ++i) {
                SecCertificateRef cfCert = (SecCertificateRef)CFArrayGetValueAtIndex(cfCerts, i);
                QCFType<CFDataRef> derData = SecCertificateCopyData(cfCert);
                if (isCaCertificateTrusted(cfCert, dom)) {
                    if (derData) {
                        const auto newCert = QSslCertificate(QByteArray::fromCFData(derData), QSsl::Der);
                        if (!canDERBeParsed(derData, newCert)) {
                            // Last attempt to get some information about the certificate:
                            CFShow(cfCert);
                            continue;
                        }
                        systemCerts << newCert;
                    } else {
                        // "Returns NULL if the data passed in the certificate parameter
                        // is not a valid certificate object."
                        qCWarning(lcX509, "SecCertificateCopyData returned invalid DER data (nullptr).");
                    }
                }
            }
        }
    }
#endif
    return systemCerts;
}
} // namespace QTlsPrivate

QT_END_NAMESPACE
