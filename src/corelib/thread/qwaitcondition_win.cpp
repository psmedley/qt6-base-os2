/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
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

#include "qwaitcondition.h"
#include "qdeadlinetimer.h"
#include "qnamespace.h"
#include "qmutex.h"
#include "qreadwritelock.h"
#include "qlist.h"
#include "qalgorithms.h"

#ifdef Q_OS_OS2
#include "qmutex_p.h"
#include "qt_os2.h"
#else
#define Q_MUTEX_T void *
#include <private/qmutex_p.h>
#include <private/qreadwritelock_p.h>
#include <qt_windows.h>
#endif

QT_BEGIN_NAMESPACE

//***********************************************************************
// QWaitConditionPrivate
// **********************************************************************

class QWaitConditionEvent
{
public:
    inline QWaitConditionEvent() : priority(0), wokenUp(false)
    {
#ifdef Q_OS_OS2
        DosCreateEventSem(NULL, &event, 0 /* manual reset */, FALSE);
#else
        event = CreateEvent(NULL, TRUE, FALSE, NULL);
#endif
    }
    inline ~QWaitConditionEvent()
    {
#ifdef Q_OS_OS2
        DosCloseEventSem(event);
#else
        CloseHandle(event);
#endif
    }
    int priority;
    bool wokenUp;
#ifdef Q_OS_OS2
    HEV event;
#else
    HANDLE event;
#endif
};

typedef QList<QWaitConditionEvent *> EventQueue;

class QWaitConditionPrivate
{
public:
    QMutex mtx;
    EventQueue queue;
    EventQueue freeQueue;

    QWaitConditionEvent *pre();
    bool wait(QWaitConditionEvent *wce, unsigned long time);
    void post(QWaitConditionEvent *wce, bool ret);
};

QWaitConditionEvent *QWaitConditionPrivate::pre()
{
    mtx.lock();
    QWaitConditionEvent *wce =
            freeQueue.isEmpty() ? new QWaitConditionEvent : freeQueue.takeFirst();
#ifdef Q_OS_OS2
    PTIB ptib;
    DosGetInfoBlocks(&ptib, NULL);
    wce->priority = ptib->tib_ptib2->tib2_ulpri;
#else
    wce->priority = GetThreadPriority(GetCurrentThread());
#endif
    wce->wokenUp = false;

    // insert 'wce' into the queue (sorted by priority)
    int index = 0;
    for (; index < queue.size(); ++index) {
        QWaitConditionEvent *current = queue.at(index);
        if (current->priority < wce->priority)
            break;
    }
    queue.insert(index, wce);
    mtx.unlock();

    return wce;
}

bool QWaitConditionPrivate::wait(QWaitConditionEvent *wce, unsigned long time)
{
    // wait for the event
#ifdef Q_OS_OS2
    APIRET arc;
    qDosNI(arc = DosWaitEventSem(wce->event, time));
    bool ret = !arc;
#else
    bool ret = false;
    switch (WaitForSingleObjectEx(wce->event, time, FALSE)) {
    default:
        break;

    case WAIT_OBJECT_0:
        ret = true;
        break;
    }
#endif
    return ret;
}

void QWaitConditionPrivate::post(QWaitConditionEvent *wce, bool ret)
{
    mtx.lock();

    // remove 'wce' from the queue
    queue.removeAll(wce);
#ifdef Q_OS_OS2
    ULONG cnt;
    DosResetEventSem(wce->event, &cnt);
#else
    ResetEvent(wce->event);
#endif
    freeQueue.append(wce);

    // wakeups delivered after the timeout should be forwarded to the next waiter
    if (!ret && wce->wokenUp && !queue.isEmpty()) {
        QWaitConditionEvent *other = queue.constFirst();
#ifdef Q_OS_OS2
        DosPostEventSem(other->event);
#else
        SetEvent(other->event);
#endif
        other->wokenUp = true;
    }

    mtx.unlock();
}

//***********************************************************************
// QWaitCondition implementation
//***********************************************************************

QWaitCondition::QWaitCondition()
{
    d = new QWaitConditionPrivate;
}

QWaitCondition::~QWaitCondition()
{
    if (!d->queue.isEmpty()) {
        qWarning("QWaitCondition: Destroyed while threads are still waiting");
        qDeleteAll(d->queue);
    }

    qDeleteAll(d->freeQueue);
    delete d;
}

bool QWaitCondition::wait(QMutex *mutex, unsigned long time)
{
    if (!mutex)
        return false;

    QWaitConditionEvent *wce = d->pre();
    mutex->unlock();

    bool returnValue = d->wait(wce, time);

    mutex->lock();
    d->post(wce, returnValue);

    return returnValue;
}

bool QWaitCondition::wait(QMutex *mutex, QDeadlineTimer deadline)
{
    return wait(mutex, deadline.remainingTime());
}

bool QWaitCondition::wait(QReadWriteLock *readWriteLock, unsigned long time)
{
    if (!readWriteLock)
        return false;
    auto previousState = readWriteLock->stateForWaitCondition();
    if (previousState == QReadWriteLock::Unlocked)
        return false;
    if (previousState == QReadWriteLock::RecursivelyLocked) {
        qWarning("QWaitCondition: cannot wait on QReadWriteLocks with recursive lockForWrite()");
        return false;
    }

    QWaitConditionEvent *wce = d->pre();
    readWriteLock->unlock();

    bool returnValue = d->wait(wce, time);

    if (previousState == QReadWriteLock::LockedForWrite)
        readWriteLock->lockForWrite();
    else
        readWriteLock->lockForRead();
    d->post(wce, returnValue);

    return returnValue;
}

bool QWaitCondition::wait(QReadWriteLock *readWriteLock, QDeadlineTimer deadline)
{
    return wait(readWriteLock, deadline.remainingTime());
}

void QWaitCondition::wakeOne()
{
    // wake up the first waiting thread in the queue
    QMutexLocker locker(&d->mtx);
    for (QWaitConditionEvent *current : qAsConst(d->queue)) {
        if (current->wokenUp)
            continue;
#ifdef Q_OS_OS2
        DosPostEventSem(current->event);
#else
        SetEvent(current->event);
#endif
        current->wokenUp = true;
        break;
    }
}

void QWaitCondition::wakeAll()
{
    // wake up the all threads in the queue
    QMutexLocker locker(&d->mtx);
    for (QWaitConditionEvent *current : qAsConst(d->queue)) {
#ifdef Q_OS_OS2
        DosPostEventSem(current->event);
#else
        SetEvent(current->event);
#endif
        current->wokenUp = true;
    }
}

QT_END_NAMESPACE
