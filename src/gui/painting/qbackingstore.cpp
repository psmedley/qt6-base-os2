/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtGui module of the Qt Toolkit.
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

#include <qbackingstore.h>
#include <qwindow.h>
#include <qpixmap.h>
#include <qpa/qplatformbackingstore.h>
#include <qpa/qplatformintegration.h>
#include <qscreen.h>
#include <qdebug.h>
#include <qscopedpointer.h>

#include <private/qguiapplication_p.h>
#include <private/qwindow_p.h>

#include <private/qhighdpiscaling_p.h>

QT_BEGIN_NAMESPACE

class QBackingStorePrivate
{
public:
    QBackingStorePrivate(QWindow *w)
        : window(w)
    {
    }

    QWindow *window;
    QPlatformBackingStore *platformBackingStore = nullptr;
    QScopedPointer<QImage> highDpiBackingstore;
    QRegion staticContents;
    QSize size;
};

/*!
    \class QBackingStore
    \since 5.0
    \inmodule QtGui

    \brief The QBackingStore class provides a drawing area for QWindow.

    QBackingStore enables the use of QPainter to paint on a QWindow with type
    RasterSurface. The other way of rendering to a QWindow is through the use
    of OpenGL with QOpenGLContext.

    A QBackingStore contains a buffered representation of the window contents,
    and thus supports partial updates by using QPainter to only update a sub
    region of the window contents.

    QBackingStore might be used by an application that wants to use QPainter
    without OpenGL acceleration and without the extra overhead of using the
    QWidget or QGraphicsView UI stacks. For an example of how to use
    QBackingStore see the \l{Raster Window Example}.
*/

/*!
    Constructs an empty surface for the given top-level \a window.
*/
QBackingStore::QBackingStore(QWindow *window)
    : d_ptr(new QBackingStorePrivate(window))
{
    if (window->handle()) {
        // Create platform backingstore up front if we have a platform window,
        // otherwise delay the creation until absolutely necessary.
        handle();
    }
}

/*!
    Destroys this surface.
*/
QBackingStore::~QBackingStore()
{
    delete d_ptr->platformBackingStore;
}

/*!
    Returns a pointer to the top-level window associated with this
    surface.
*/
QWindow* QBackingStore::window() const
{
    return d_ptr->window;
}

/*!
    Begins painting on the backing store surface in the given \a region.

    You should call this function before using the paintDevice() to
    paint.

    \sa endPaint(), paintDevice()
*/

void QBackingStore::beginPaint(const QRegion &region)
{
    if (d_ptr->highDpiBackingstore &&
        d_ptr->highDpiBackingstore->devicePixelRatio() != d_ptr->window->devicePixelRatio())
        resize(size());

    QPlatformBackingStore *platformBackingStore = handle();
    platformBackingStore->beginPaint(QHighDpi::toNativeLocalRegion(region, d_ptr->window));

    // When QtGui is applying a high-dpi scale factor the backing store
    // creates a "large" backing store image. This image needs to be
    // painted on as a high-dpi image, which is done by setting
    // devicePixelRatio. Do this on a separate image instance that shares
    // the image data to avoid having the new devicePixelRatio be propagated
    // back to the platform plugin.
    QPaintDevice *device = platformBackingStore->paintDevice();
    if (QHighDpiScaling::isActive() && device->devType() == QInternal::Image) {
        QImage *source = static_cast<QImage *>(device);
        const bool needsNewImage = d_ptr->highDpiBackingstore.isNull()
            || source->data_ptr() != d_ptr->highDpiBackingstore->data_ptr()
            || source->size() != d_ptr->highDpiBackingstore->size()
            || source->devicePixelRatio() != d_ptr->highDpiBackingstore->devicePixelRatio();
        if (needsNewImage) {
            d_ptr->highDpiBackingstore.reset(
                new QImage(source->bits(), source->width(), source->height(), source->bytesPerLine(), source->format()));

            qreal targetDevicePixelRatio = d_ptr->window->devicePixelRatio();
            d_ptr->highDpiBackingstore->setDevicePixelRatio(targetDevicePixelRatio);
        }
    }
}

/*!
    Returns the paint device for this surface.

    \warning The device is only valid between calls to beginPaint() and
    endPaint(). You should not cache the returned value.
*/
QPaintDevice *QBackingStore::paintDevice()
{
    QPaintDevice *device = handle()->paintDevice();

    if (QHighDpiScaling::isActive() && device->devType() == QInternal::Image)
        return d_ptr->highDpiBackingstore.data();

    return device;
}

/*!
    Ends painting.

    You should call this function after painting with the paintDevice()
    has ended.

    \sa beginPaint(), paintDevice()
*/
void QBackingStore::endPaint()
{
    if (paintDevice()->paintingActive())
        qWarning("QBackingStore::endPaint() called with active painter; did you forget to destroy it or call QPainter::end() on it?");

    handle()->endPaint();
}

/*!
    Flushes the given \a region from the specified \a window onto the
    screen.

    The \a window must either be the top level window represented by
    this backingstore, or a non-transient child of that window. Passing
    \nullptr falls back to using the backingstore's top level window.

    If the \a window is a child window, the \a region should be in child window
    coordinates, and the \a offset should be the child window's offset in relation
    to the backingstore's top level window.

    You should call this function after ending painting with endPaint().
*/
void QBackingStore::flush(const QRegion &region, QWindow *window, const QPoint &offset)
{
    QWindow *topLevelWindow = this->window();

    if (!window)
        window = topLevelWindow;
    if (!window->handle()) {
        qWarning() << "QBackingStore::flush() called for "
            << window << " which does not have a handle.";
        return;
    }

    if (!QPlatformSurface::isRasterSurface(window)) {
        qWarning() << "Attempted flush to non-raster surface" << window << "of type" << window->surfaceType()
            << (window->inherits("QWidgetWindow") ? "(consider using Qt::WA_PaintOnScreen to exclude "
                                                   "from backingstore sync)" : "");
        return;
    }

    Q_ASSERT(window == topLevelWindow || topLevelWindow->isAncestorOf(window, QWindow::ExcludeTransients));

    QRegion nativeRegion = QHighDpi::toNativeLocalRegion(region, window);
    QPoint nativeOffset;
    if (!offset.isNull()) {
        nativeOffset = QHighDpi::toNativeLocalPosition(offset, window);
        // Under fractional DPR, rounding of region and offset may accumulate to an off-by-one
        QPoint topLeft = region.boundingRect().topLeft() + offset;
        QPoint nativeTopLeft = QHighDpi::toNativeLocalPosition(topLeft, window);
        QPoint diff = nativeTopLeft - (nativeRegion.boundingRect().topLeft() + nativeOffset);
        Q_ASSERT(qMax(qAbs(diff.x()), qAbs(diff.y())) <= 1);
        nativeRegion.translate(diff);
    }
    handle()->flush(window, nativeRegion, nativeOffset);
}

/*!
    Sets the size of the window surface to \a size.

    \sa size()
*/
void QBackingStore::resize(const QSize &size)
{
    d_ptr->size = size;
    handle()->resize(QHighDpi::toNativePixels(size, d_ptr->window),
                     QHighDpi::toNativePixels(d_ptr->staticContents, d_ptr->window));
}

/*!
    Returns the current size of the window surface.
*/
QSize QBackingStore::size() const
{
    return d_ptr->size;
}

/*!
    Scrolls the given \a area \a dx pixels to the right and \a dy
    downward; both \a dx and \a dy may be negative.

    Returns \c true if the area was scrolled successfully; false otherwise.
*/
bool QBackingStore::scroll(const QRegion &area, int dx, int dy)
{
    // Disable scrolling for non-integer scroll deltas. For this case
    // the existing rendered pixels can't be re-used, and we return
    // false to signal that a repaint is needed.
    const qreal nativeDx = QHighDpi::toNativePixels(qreal(dx), d_ptr->window);
    const qreal nativeDy = QHighDpi::toNativePixels(qreal(dy), d_ptr->window);
    if (qFloor(nativeDx) != nativeDx || qFloor(nativeDy) != nativeDy)
        return false;

    return handle()->scroll(QHighDpi::toNativeLocalRegion(area, d_ptr->window),
                                               nativeDx, nativeDy);
}

/*!
    Set \a region as the static contents of this window.
*/
void QBackingStore::setStaticContents(const QRegion &region)
{
    d_ptr->staticContents = region;
}

/*!
    Returns a QRegion representing the area of the window that
    has static contents.
*/
QRegion QBackingStore::staticContents() const
{
    return d_ptr->staticContents;
}

/*!
    Returns a boolean indicating if this window has static contents or not.
*/
bool QBackingStore::hasStaticContents() const
{
    return !d_ptr->staticContents.isEmpty();
}

void Q_GUI_EXPORT qt_scrollRectInImage(QImage &img, const QRect &rect, const QPoint &offset)
{
    // make sure we don't detach
    uchar *mem = const_cast<uchar*>(img.constBits());

    qsizetype lineskip = img.bytesPerLine();
    int depth = img.depth() >> 3;

    const QRect imageRect(0, 0, img.width(), img.height());
    const QRect sourceRect = rect.intersected(imageRect).intersected(imageRect.translated(-offset));
    if (sourceRect.isEmpty())
        return;

    const QRect destRect = sourceRect.translated(offset);
    Q_ASSERT_X(imageRect.contains(destRect), "qt_scrollRectInImage",
        "The sourceRect should already account for clipping, both pre and post scroll");

    const uchar *src;
    uchar *dest;

    if (sourceRect.top() < destRect.top()) {
        src = mem + sourceRect.bottom() * lineskip + sourceRect.left() * depth;
        dest = mem + (destRect.top() + sourceRect.height() - 1) * lineskip + destRect.left() * depth;
        lineskip = -lineskip;
    } else {
        src = mem + sourceRect.top() * lineskip + sourceRect.left() * depth;
        dest = mem + destRect.top() * lineskip + destRect.left() * depth;
    }

    const int w = sourceRect.width();
    int h = sourceRect.height();
    const int bytes = w * depth;

    // overlapping segments?
    if (offset.y() == 0 && qAbs(offset.x()) < w) {
        do {
            ::memmove(dest, src, bytes);
            dest += lineskip;
            src += lineskip;
        } while (--h);
    } else {
        do {
            ::memcpy(dest, src, bytes);
            dest += lineskip;
            src += lineskip;
        } while (--h);
    }
}

/*!
    Returns a pointer to the QPlatformBackingStore implementation
*/
QPlatformBackingStore *QBackingStore::handle() const
{
    if (!d_ptr->platformBackingStore) {
        d_ptr->platformBackingStore = QGuiApplicationPrivate::platformIntegration()->createPlatformBackingStore(d_ptr->window);
        d_ptr->platformBackingStore->setBackingStore(const_cast<QBackingStore*>(this));
    }
    return d_ptr->platformBackingStore;
}

QT_END_NAMESPACE
