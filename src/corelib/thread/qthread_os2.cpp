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

#include "qplatformdefs.h"
#include "qthread.h"
#include "qthread_p.h"
#include "qmutex.h"

#include <private/qcoreapplication_p.h>
#include <private/qeventdispatcher_os2_p.h>
#include <private/qthreadstorage_p.h>

#include <chrono>

QT_BEGIN_NAMESPACE

#if QT_CONFIG(thread)

static void qt_watch_adopted_thread(const int adoptedTID, QThread *qthread);
static void qt_adopted_thread_watcher_function(void *);

// Note: we use DosAllocThreadLocalMemory rather than C++ thread_local since we need an explicit
// way to clean up adopted threads anyway (see qt_watch_adopted_thread below).
static QThreadData **qt_tls = nullptr;

static void qt_create_tls()
{
    if (qt_tls != nullptr)
        return;
    Q_CONSTINIT static QBasicMutex mutex;
    QMutexLocker locker(&mutex);
    // Re-check under the lock: another thread may have allocated it meanwhile.
    if (qt_tls != nullptr)
        return;
    APIRET arc = DosAllocThreadLocalMemory((sizeof(QThreadData *) + 3) / 4,
                                           (PULONG *)&qt_tls);
    if (arc != NO_ERROR)
        qWarning("qt_create_tls: DosAllocThreadLocalMemory returned %lu", arc);
}

static void qt_free_tls()
{
    if (qt_tls != nullptr) {
        DosFreeThreadLocalMemory((PULONG)qt_tls);
        qt_tls = nullptr;
    }
}

Q_DESTRUCTOR_FUNCTION(qt_free_tls)

static QThreadData *get_thread_data() noexcept
{
    return qt_tls ? *qt_tls : nullptr;
}

static void set_thread_data(QThreadData *data) noexcept
{
    if (data) {
        // Make sure the QThreadStorage destructor registry is initialized before the first
        // QThreadData is stored, for a consistent destruction order (see qthreadstorage.cpp).
        Q_CONSTINIT static bool storageInitialized = false;
        if (!storageInitialized) {
            QThreadStoragePrivate::init();
            storageInitialized = true;
        }
        qt_create_tls();
    }
    if (qt_tls)
        *qt_tls = data;
}

/*
    QThreadData
*/

void QThreadData::clearCurrentThreadData()
{
    set_thread_data(nullptr);
}

QThreadData *QThreadData::currentThreadData() noexcept
{
    return get_thread_data();
}

QThreadData *QThreadData::createCurrentThreadData()
{
    Q_ASSERT(!currentThreadData());

    QThreadData *data = new QThreadData();

    // This needs to be called prior to new QAdoptedThread() to avoid
    // recursion (see qobject.cpp).
    set_thread_data(data);

    QT_TRY {
        // Note: QAdoptedThread's constructor sets isAdopted, threadId and, for the first
        // adopted thread, makes it the main thread.
        data->thread.storeRelease(new QAdoptedThread(data));
    } QT_CATCH(...) {
        set_thread_data(nullptr);
        data->deref();
        QT_RETHROW;
    }

    // Non-main adopted threads need to be watched to release their QThreadData when they end
    // (there are no TLS destructors for DosAllocThreadLocalMemory).
    if (data->threadId.loadRelaxed() != QCoreApplicationPrivate::theMainThreadId.loadAcquire())
        qt_watch_adopted_thread(_gettid(), data->thread.loadRelaxed());

    return data;
}

void QAdoptedThread::init()
{
    d_func()->tid = _gettid();
}

static QList<int> qt_adopted_thread_tids;
static QList<QThread *> qt_adopted_qthreads;
Q_CONSTINIT static QBasicMutex qt_adopted_thread_watcher_mutex;
Q_CONSTINIT static int qt_adopted_thread_watcher_tid = 0;

/*!
    \internal
    Adds an adopted thread to the list of threads that Qt watches to make sure
    the thread data is properly cleaned up. This function starts the watcher
    thread if necessary.
*/
static void qt_watch_adopted_thread(const int adoptedTID, QThread *qthread)
{
    QMutexLocker lock(&qt_adopted_thread_watcher_mutex);

    if (_gettid() == qt_adopted_thread_watcher_tid)
        return;

    qt_adopted_thread_tids.append(adoptedTID);
    qt_adopted_qthreads.append(qthread);

    // Start watcher thread if it is not already running.
    if (qt_adopted_thread_watcher_tid == 0) {
        int tid = _beginthread(qt_adopted_thread_watcher_function, nullptr, 0, nullptr);
        if (tid == -1)
            qErrnoWarning(errno, "qt_watch_adopted_thread: _beginthread failed");
        else
            qt_adopted_thread_watcher_tid = tid;
    }
}

/*!
    \internal
    This function loops and waits for native adopted threads to finish.
    When this happens it derefs the QThreadData for the adopted thread
    to make sure it gets cleaned up properly.
*/
static void qt_adopted_thread_watcher_function(void *)
{
    QMutexLocker lock(&qt_adopted_thread_watcher_mutex);

    forever {
        if (qt_adopted_thread_tids.isEmpty()) {
            // our service is no longer necessary
            qt_adopted_thread_watcher_tid = 0;
            break;
        }

        lock.unlock();

        // Wait for any thread of this process
        APIRET arc;
        TID tid = 0;
        qDosNI(arc = DosWaitThread(&tid, DCWW_WAIT));

        lock.relock();

        if (arc == NO_ERROR) {
            qsizetype i = qt_adopted_thread_tids.indexOf(static_cast<int>(tid));
            if (i >= 0) {
                QThreadData *data = QThreadData::get2(qt_adopted_qthreads.at(i));

                lock.unlock();

                if (data->isAdopted) {
                    QThread *thread = data->thread.loadAcquire();
                    Q_ASSERT(thread);
                    QThreadPrivate *thread_p = static_cast<QThreadPrivate *>(QObjectPrivate::get(thread));
                    Q_ASSERT(thread_p->threadState != QThreadPrivate::Finished);
                    thread_p->finish();
                }
                data->deref();

                lock.relock();

                qt_adopted_thread_tids.remove(i);
                qt_adopted_qthreads.remove(i);
            }
        } else {
            // Should never fail.
            qWarning("qt_adopted_thread_watcher_function: DosWaitThread returned %lu", arc);
            break;
        }
    }

    lock.unlock();

    // Release the data of this thread in case some Qt function adopted it.
    if (QThreadData *threadData = get_thread_data()) {
        set_thread_data(nullptr);
        threadData->deref();
    }
}

/**************************************************************************
 ** QThreadPrivate
 *************************************************************************/

#endif // QT_CONFIG(thread)

QAbstractEventDispatcher *QThreadPrivate::createEventDispatcher(QThreadData *data)
{
    Q_UNUSED(data);
    return new QEventDispatcherOS2;
}

#if QT_CONFIG(thread)

// static
void QThreadPrivate::start(void *arg) noexcept
{
    QThread *thr = reinterpret_cast<QThread *>(arg);
    QThreadData *data = QThreadData::get2(thr);

    set_thread_data(data);
    data->threadId.storeRelaxed(QThread::currentThreadId());

    // If a QThread is restarted, reuse the QBindingStatus, too
    data->reuseBindingStatusForNewNativeThread();

    QThread::setTerminationEnabled(false);

    {
        QMutexLocker locker(&thr->d_func()->mutex);
        data->quitNow = thr->d_func()->exited;
        // OS/2 has no notion of thread names, drop the one captured in start().
        thr->d_func()->objectName.clear();
    }

    data->ensureEventDispatcher();
    data->eventDispatcher.loadRelaxed()->startingUp();

    emit thr->started(QThread::QPrivateSignal());
    QThread::setTerminationEnabled(true);
    thr->run();

    thr->d_func()->finish();

    // Don't leave a dangling pointer in this TID's TLS slot (it may be reused by a new thread).
    set_thread_data(nullptr);
}

void QThreadPrivate::setQualityOfServiceLevel(QThread::QualityOfService qosLevel)
{
    // Not supported on OS/2.
    serviceLevel = qosLevel;
}

/*
    For regularly terminating threads, this will be called and executed by the thread as the
    last code before the thread exits. However, this function will also be called by
    QThread::terminate (as well as wait() and setTerminationEnabled) to give Qt a chance to update
    the terminated thread's state and process pending DeleteLater events for objects that live in
    the terminated thread. And for adopted threads, this method is called by the thread watcher.
*/
void QThreadPrivate::finish(bool lockAnyway) noexcept
{
    QThreadPrivate *d = this;
    QThread *thr = q_func();

    QMutexLocker locker(lockAnyway ? &d->mutex : nullptr);
    d->threadState = QThreadPrivate::Finishing;
    d->priority = QThread::InheritPriority;
    if (lockAnyway)
        locker.unlock();
    emit thr->finished(QThread::QPrivateSignal());
    QCoreApplicationPrivate::sendPostedEvents(nullptr, QEvent::DeferredDelete, d->data);
    QThreadStoragePrivate::finish(&d->data->tls);
    if (lockAnyway)
        locker.relock();

    QAbstractEventDispatcher *eventDispatcher = d->data->eventDispatcher.loadRelaxed();
    if (eventDispatcher) {
        d->data->eventDispatcher = nullptr;
        if (lockAnyway)
            locker.unlock();
        eventDispatcher->closingDown();
        delete eventDispatcher;
        if (lockAnyway)
            locker.relock();
    }

    d->threadState = QThreadPrivate::Finished;
    d->interruptionRequested.store(false, std::memory_order_relaxed);

    // Keep the TID while someone is waiting for it (see #wait).
    if (!d->waiters)
        d->tid = 0;
}

/**************************************************************************
 ** QThread
 *************************************************************************/

Qt::HANDLE QThread::currentThreadIdImpl() noexcept
{
    return reinterpret_cast<Qt::HANDLE>(quintptr(_gettid()));
}

int QThread::idealThreadCount() noexcept
{
    ULONG cpuCnt = 1;
    APIRET rc = DosQuerySysInfo(QSV_NUMPROCESSORS, QSV_NUMPROCESSORS,
                                &cpuCnt, sizeof(cpuCnt));
    if (rc != NO_ERROR || cpuCnt == 0)
        cpuCnt = 1;
    return int(cpuCnt);
}

void QThread::yieldCurrentThread()
{
    DosSleep(0);
}

#endif // QT_CONFIG(thread)

void QThread::sleep(std::chrono::nanoseconds nsecs)
{
    using namespace std::chrono;
    // Round up: DosSleep has millisecond granularity (0 merely yields).
    DosSleep(ULONG(ceil<milliseconds>(nsecs).count()));
}

void QThread::sleep(unsigned long secs)
{
    DosSleep(secs * 1000);
}

void QThread::msleep(unsigned long msecs)
{
    DosSleep(msecs);
}

void QThread::usleep(unsigned long usecs)
{
    sleep(std::chrono::microseconds(usecs));
}

#if QT_CONFIG(thread)

// Returns the current priority of the given thread of this process in the
// tib2_ulpri format (class in bits 8-15, level 0-31 in bits 0-7) or false if
// the thread doesn't exist (anymore) or the query fails.
static bool qt_query_thread_priority(TID tid, ULONG *priority)
{
    PTIB ptib;
    PPIB ppib;
    DosGetInfoBlocks(&ptib, &ppib);

    // Fast path for the current thread.
    if (ptib->tib_ptib2->tib2_ultid == tid) {
        *priority = ptib->tib_ptib2->tib2_ulpri;
        return true;
    }

    // QS_PROCESS returns the process record followed by its thread records.
    enum { SysStateSize = 64 * 1024 };
    QByteArray buf(SysStateSize, Qt::Uninitialized);
    APIRET arc = DosQuerySysState(QS_PROCESS, 0, ppib->pib_ulpid, 0, buf.data(), SysStateSize);
    if (arc != NO_ERROR) {
        qWarning("qt_query_thread_priority: DosQuerySysState returned %lu", arc);
        return false;
    }

    const QSPTRREC *pPtrRec = reinterpret_cast<const QSPTRREC *>(buf.constData());
    const QSPREC *pProcRec = pPtrRec->pProcRec;
    if (!pProcRec || pProcRec->RecType != QS_PROCESS || !pProcRec->pThrdRec)
        return false;

    for (USHORT i = 0; i < pProcRec->cTCB; ++i) {
        const QSTREC &rec = pProcRec->pThrdRec[i];
        if (rec.tid == tid) {
            *priority = rec.priority;
            return true;
        }
    }

    return false;
}

// NOTE: Caller must hold the mutex.
void QThreadPrivate::setPriority(QThread::Priority threadPriority)
{
    priority = threadPriority;

    // Target priority class and absolute level (0-31) within this class. Note that the
    // IdleTime class has three Qt priorities so they use distinct levels to stay ordered.
    ULONG prioClass = 0;
    LONG prioLevel = 0;
    switch (threadPriority) {
    case QThread::IdlePriority:
        prioClass = PRTYC_IDLETIME;
        prioLevel = 0;
        break;
    case QThread::LowestPriority:
        prioClass = PRTYC_IDLETIME;
        prioLevel = PRTYD_MAXIMUM / 2;
        break;
    case QThread::LowPriority:
        prioClass = PRTYC_IDLETIME;
        prioLevel = PRTYD_MAXIMUM;
        break;
    case QThread::NormalPriority:
        prioClass = PRTYC_REGULAR;
        prioLevel = 0;
        break;
    case QThread::HighPriority:
        prioClass = PRTYC_REGULAR;
        prioLevel = PRTYD_MAXIMUM;
        break;
    case QThread::HighestPriority:
        prioClass = PRTYC_TIMECRITICAL;
        prioLevel = 0;
        break;
    case QThread::TimeCriticalPriority:
        prioClass = PRTYC_TIMECRITICAL;
        prioLevel = PRTYD_MAXIMUM;
        break;
    case QThread::InheritPriority:
    default:
        // Threads created with _beginthread already inherit the priority of the creator.
        return;
    }

    // NOTE: The delta argument of DosSetPriority is relative to the current level of the
    // thread, so passing the target level directly only works if the thread is at level 0
    // (e.g. switching from Low to Lowest would otherwise be a no-op). Query the current
    // level and compute the delta. Since it's not obvious whether changing the class resets
    // the level, do the class change first and query the level again afterwards.

    APIRET rc;
    ULONG cur = 0;
    if (!qt_query_thread_priority(tid, &cur)) {
        // Can't query (e.g. the thread has already ended), use the old approach which is
        // correct at least for threads that are at level 0.
        rc = DosSetPriority(PRTYS_THREAD, prioClass, prioLevel, tid);
        if (rc != NO_ERROR && rc != ERROR_INVALID_THREADID)
            qWarning("QThreadPrivate::setPriority: DosSetPriority returned %lu", rc);
        return;
    }

    if (((cur >> 8) & 0xFF) != prioClass) {
        rc = DosSetPriority(PRTYS_THREAD, prioClass, 0, tid);
        if (rc != NO_ERROR) {
            if (rc != ERROR_INVALID_THREADID)
                qWarning("QThreadPrivate::setPriority: DosSetPriority returned %lu", rc);
            return;
        }
        if (!qt_query_thread_priority(tid, &cur))
            return; // the thread is gone
    }

    const LONG delta = prioLevel - LONG(cur & 0xFF);
    if (delta != 0) {
        rc = DosSetPriority(PRTYS_THREAD, PRTYC_NOCHANGE, delta, tid);
        if (rc != NO_ERROR && rc != ERROR_INVALID_THREADID)
            qWarning("QThreadPrivate::setPriority: DosSetPriority returned %lu", rc);
    }
}

void QThread::start(Priority priority)
{
    Q_D(QThread);
    QMutexLocker locker(&d->mutex);

    if (d->threadState == QThreadPrivate::Finishing) {
        locker.unlock();
        wait();
        locker.relock();
    }

    if (d->threadState == QThreadPrivate::Running)
        return;

    // avoid interacting with the binding system
    d->objectName = d->extraData ? d->extraData->objectName.valueBypassingBindings()
                                 : QString();
    d->threadState = QThreadPrivate::Running;
    d->exited = false;
    d->returnCode = 0;
    d->interruptionRequested.store(false, std::memory_order_relaxed);

    // Note: the new thread blocks on d->mutex (held here) before doing anything that depends
    // on d->tid, so it's safe to assign it after _beginthread returns.
    int tid = _beginthread(QThreadPrivate::start, nullptr, d->stackSize, this);
    if (tid == -1) {
        qErrnoWarning(errno, "QThread::start: Failed to create thread");
        d->tid = 0;
        d->threadState = QThreadPrivate::NotStarted;
        return;
    }
    d->tid = TID(tid);

    d->setPriority(priority);
}

void QThread::terminate()
{
    Q_D(QThread);
    QMutexLocker locker(&d->mutex);
    if (d->threadState != QThreadPrivate::Running)
        return;
    if (d->tid == TID(_gettid())) {
        qWarning("QThread::terminate: thread cannot terminate itself");
        return;
    }
    if (!d->terminationEnabled) {
        d->terminatePending = true;
        return;
    }
    APIRET rc = DosKillThread(d->tid);
    if (rc != NO_ERROR)
        qWarning("QThread::terminate: DosKillThread returned %lu", rc);
    d->finish(false);
}

// NOTE: Called by QThread::wait with the mutex locked and the thread neither NotStarted nor
// Finished.
bool QThreadPrivate::wait(QMutexLocker<QMutex> &locker, QDeadlineTimer deadline)
{
    Q_ASSERT(threadState != QThreadPrivate::Finished);
    Q_ASSERT(locker.isLocked());
    QThreadPrivate *d = this;

    // Never pass 0 to DosWaitThread as it would wait for *any* thread.
    TID waitTid = d->tid;
    if (waitTid == 0)
        return true;

    ++d->waiters;
    locker.mutex()->unlock();

    APIRET rc = NO_ERROR;
    if (deadline.isForever()) {
        qDosNI(rc = DosWaitThread(&waitTid, DCWW_WAIT));
    } else {
        // OS/2 can't wait for a thread with a timeout, so poll at short intervals. Note that
        // reading threadState without the lock is only an optimization (it's re-checked below).
        constexpr qint64 PollIntervalMs = 10;
        forever {
            qDosNI(rc = DosWaitThread(&waitTid, DCWW_NOWAIT));
            if (rc != ERROR_THREAD_NOT_TERMINATED)
                break;
            if (d->threadState == QThreadPrivate::Finished) {
                rc = NO_ERROR;
                break;
            }
            const qint64 remaining = deadline.remainingTime();
            if (remaining <= 0)
                break;
            DosSleep(ULONG(qMin(remaining, PollIntervalMs)));
        }
    }

    // ERROR_INVALID_THREADID means the thread is already gone (e.g. reaped by the adopted
    // thread watcher's DosWaitThread call).
    bool ret = rc == NO_ERROR || rc == ERROR_INVALID_THREADID;
    if (!ret && rc != ERROR_THREAD_NOT_TERMINATED)
        qWarning("QThread::wait: DosWaitThread returned %lu", rc);

    locker.mutex()->lock();
    --d->waiters;

    if (ret && d->threadState < QThreadPrivate::Finished) {
        // thread was terminated by someone else
        d->finish(false);
    }

    if (d->threadState == QThreadPrivate::Finished && !d->waiters)
        d->tid = 0;

    return ret;
}

void QThread::setTerminationEnabled(bool enabled)
{
    QThread *thr = currentThread();
    Q_ASSERT_X(thr != nullptr, "QThread::setTerminationEnabled()",
               "Current thread was not started with QThread.");
    QThreadPrivate *d = thr->d_func();
    QMutexLocker locker(&d->mutex);
    d->terminationEnabled = enabled;
    if (enabled && d->terminatePending) {
        d->finish(false);
        locker.unlock(); // don't leave the mutex locked!
        _endthread();
    }
}

#endif // QT_CONFIG(thread)

QT_END_NAMESPACE
