/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** Copyright (C) 2018 bww bitwise works GmbH. OS/2 parts.
**
** This file is part of the QtCore module of the Qt Toolkit.
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

//#define QSHAREDMEMORY_DEBUG

#include "qsharedmemory.h"
#include "qsharedmemory_p.h"
#include "qsystemsemaphore.h"
#include "qfile.h"
#include <qdebug.h>
#include "qt_os2.h"

QT_BEGIN_NAMESPACE

#if QT_CONFIG(sharedmemory)

void QSharedMemoryPrivate::setOS2ErrorString(APIRET arc, QLatin1String function)
{
    if (arc == 0)
        return;

    switch (arc) {
    case ERROR_ALREADY_EXISTS:
        error = QSharedMemory::AlreadyExists;
        errorString = QSharedMemory::tr("%1: already exists").arg(function);
    break;
    case ERROR_FILE_NOT_FOUND:
        error = QSharedMemory::NotFound;
        errorString = QSharedMemory::tr("%1: doesn't exist").arg(function);
        break;
    case ERROR_CROSSES_OBJECT_BOUNDARY:
        error = QSharedMemory::InvalidSize;
        errorString = QSharedMemory::tr("%1: invalid size").arg(function);
        break;
    case ERROR_TOO_MANY_HANDLES:
    case ERROR_NOT_ENOUGH_MEMORY:
        error = QSharedMemory::OutOfResources;
        errorString = QSharedMemory::tr("%1: out of resources").arg(function);
        break;
    case ERROR_ACCESS_DENIED:
        error = QSharedMemory::PermissionDenied;
        errorString = QSharedMemory::tr("%1: permission denied").arg(function);
        break;
    case ERROR_INVALID_NAME:
        error = QSharedMemory::KeyError;
        errorString = QSharedMemory::tr("%1: invalid key name").arg(function);
        break;
    default:
        errorString = QSharedMemory::tr("%1: unknown error %2").arg(function).arg(arc);
        error = QSharedMemory::UnknownError;
    }

#if defined QSHAREDMEMORY_DEBUG
    qDebug() << errorString << "arc" << arc << "key" << key << "fileName" << self->nativeKey;
#endif
}

bool QSharedMemoryOS2::handle(QSharedMemoryPrivate *self)
{
    // don't allow making handles on empty keys
    if (self->nativeKey.isEmpty()) {
        self->setError(QSharedMemory::KeyError,
                       QSharedMemory::tr("%1: key is empty").arg("QSharedMemory::handle"));
        return false;
    }

    return true;
}

bool QSharedMemoryOS2::cleanHandle(QSharedMemoryPrivate *self)
{
    if (self->memory != nullptr) {
        APIRET arc = DosFreeMem(self->memory);
        self->memory = nullptr;
        self->size = 0;
        if (arc != NO_ERROR) {
            self->setOS2ErrorString(arc, QLatin1String("QSharedMemory::cleanHandle"));
            return false;
        }
    }
    return true;
}

bool QSharedMemoryOS2::create(QSharedMemoryPrivate *self, qsizetype size)
{
#if defined QSHAREDMEMORY_DEBUG
    qDebug() << "size" << size << "nativeKey" << self->nativeKey.nativeKey().toLatin1();
#endif

    ULONG flags = PAG_READ | PAG_WRITE | PAG_COMMIT;
    QString pszName = QLatin1String("\\SHAREMEM\\") + self->nativeKey.nativeKey().toLatin1();
    APIRET arc = DosAllocSharedMem(&self->memory, pszName.toLatin1(), (int)size, flags | OBJ_ANY);
    if (arc != NO_ERROR)
        arc = DosAllocSharedMem(&self->memory, pszName.toLatin1(), (int)size, flags);

    if (arc != NO_ERROR) {
        self->setOS2ErrorString(arc, QLatin1String("QSharedMemory::create"));
        return false;
    }

#if defined QSHAREDMEMORY_DEBUG
    qDebug() << "memory" << memory;
#endif

    self->size = (int)size;
    return true;
}

bool QSharedMemoryOS2::attach(QSharedMemoryPrivate *self, QSharedMemory::AccessMode mode)
{
#if defined QSHAREDMEMORY_DEBUG
    qDebug() << "mode" << mode << "nativeKey" << self->nativeKey << "memory" << self->memory;
#endif

    ULONG flags = PAG_READ;
    if (mode == QSharedMemory::ReadWrite)
        flags |= PAG_WRITE;

    APIRET arc;
    QString pszName = QLatin1String("\\SHAREMEM\\") + self->nativeKey.nativeKey().toLatin1();
    if (self->memory) {
        // We've just created the region, only set the desired access flags
        arc = DosSetMem(self->memory, self->size, flags);
    } else {
        // Try to open the shared memory region
        arc = DosGetNamedSharedMem(&self->memory, pszName.toLatin1(), flags);
        if (arc == NO_ERROR) {
            // Detect the region size
            ULONG sz = ~0, dummy;
            arc = DosQueryMem(self->memory, &sz, &dummy);
            if (arc == NO_ERROR)
                self->size = sz;
        }
    }

    if (arc != NO_ERROR) {
        self->setOS2ErrorString(arc, QLatin1String("QSharedMemory::attach"));
        cleanHandle(self);
        return false;
    }

    return true;
}

bool QSharedMemoryOS2::detach(QSharedMemoryPrivate *self)
{
    // Nothing to do here other than free memory
    return cleanHandle(self);
}

#endif // QT_CONFIG(sharedmemory)


QT_END_NAMESPACE
