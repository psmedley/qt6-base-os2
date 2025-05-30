/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Copyright (C) 2013 Olivier Goffart <ogoffart@woboq.com>
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

#ifndef QOBJECT_P_H
#define QOBJECT_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qapplication_*.cpp, qwidget*.cpp and qfiledialog.cpp.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/private/qglobal_p.h>
#include "QtCore/qcoreevent.h"
#include "QtCore/qlist.h"
#include "QtCore/qobject.h"
#include "QtCore/qpointer.h"
#include "QtCore/qsharedpointer.h"
#include "QtCore/qvariant.h"
#include "QtCore/qproperty.h"
#include "QtCore/private/qproperty_p.h"

QT_BEGIN_NAMESPACE

class QVariant;
class QThreadData;
class QObjectConnectionListVector;
namespace QtSharedPointer { struct ExternalRefCountData; }

/* for Qt Test */
struct QSignalSpyCallbackSet
{
    typedef void (*BeginCallback)(QObject *caller, int signal_or_method_index, void **argv);
    typedef void (*EndCallback)(QObject *caller, int signal_or_method_index);
    BeginCallback signal_begin_callback,
                    slot_begin_callback;
    EndCallback signal_end_callback,
                slot_end_callback;
};
void Q_CORE_EXPORT qt_register_signal_spy_callbacks(QSignalSpyCallbackSet *callback_set);

extern Q_CORE_EXPORT QBasicAtomicPointer<QSignalSpyCallbackSet> qt_signal_spy_callback_set;

enum { QObjectPrivateVersion = QT_VERSION };

class Q_CORE_EXPORT QAbstractDeclarativeData
{
public:
    static void (*destroyed)(QAbstractDeclarativeData *, QObject *);
    static void (*signalEmitted)(QAbstractDeclarativeData *, QObject *, int, void **);
    static int  (*receivers)(QAbstractDeclarativeData *, const QObject *, int);
    static bool (*isSignalConnected)(QAbstractDeclarativeData *, const QObject *, int);
    static void (*setWidgetParent)(QObject *, QObject *); // Used by the QML engine to specify parents for widgets. Set by QtWidgets.
};

class Q_CORE_EXPORT QObjectPrivate : public QObjectData
{
    Q_DECLARE_PUBLIC(QObject)

public:
    struct ExtraData
    {
        ExtraData(QObjectPrivate *ptr) : parent(ptr) { }

        inline void setObjectNameForwarder(const QString &name)
        {
            parent->q_func()->setObjectName(name);
        }

        inline void nameChangedForwarder(const QString &name)
        {
            Q_EMIT parent->q_func()->objectNameChanged(name, QObject::QPrivateSignal());
        }

        QList<QByteArray> propertyNames;
        QList<QVariant> propertyValues;
        QList<int> runningTimers;
        QList<QPointer<QObject>> eventFilters;
        Q_OBJECT_COMPAT_PROPERTY(QObjectPrivate::ExtraData, QString, objectName,
                                 &QObjectPrivate::ExtraData::setObjectNameForwarder,
                                 &QObjectPrivate::ExtraData::nameChangedForwarder)
        QObjectPrivate *parent;
    };

    typedef void (*StaticMetaCallFunction)(QObject *, QMetaObject::Call, int, void **);
    struct Connection;
    struct SignalVector;

    struct ConnectionOrSignalVector {
        union {
            // linked list of orphaned connections that need cleaning up
            ConnectionOrSignalVector *nextInOrphanList;
            // linked list of connections connected to slots in this object
            Connection *next;
        };

        static SignalVector *asSignalVector(ConnectionOrSignalVector *c)
        {
            if (reinterpret_cast<quintptr>(c) & 1)
                return reinterpret_cast<SignalVector *>(reinterpret_cast<quintptr>(c) & ~quintptr(1u));
            return nullptr;
        }
        static Connection *fromSignalVector(SignalVector *v) {
            return reinterpret_cast<Connection *>(reinterpret_cast<quintptr>(v) | quintptr(1u));
        }
    };

    struct Connection : public ConnectionOrSignalVector
    {
        // linked list of connections connected to slots in this object, next is in base class
        Connection **prev;
        // linked list of connections connected to signals in this object
        QAtomicPointer<Connection> nextConnectionList;
        Connection *prevConnectionList;

        QObject *sender;
        QAtomicPointer<QObject> receiver;
        QAtomicPointer<QThreadData> receiverThreadData;
        union {
            StaticMetaCallFunction callFunction;
            QtPrivate::QSlotObjectBase *slotObj;
        };
        QAtomicPointer<const int> argumentTypes;
        QAtomicInt ref_;
        uint id = 0;
        ushort method_offset;
        ushort method_relative;
        signed int signal_index : 27; // In signal range (see QObjectPrivate::signalIndex())
        ushort connectionType : 2; // 0 == auto, 1 == direct, 2 == queued, 3 == blocking
        ushort isSlotObject : 1;
        ushort ownArgumentTypes : 1;
        ushort isSingleShot : 1;
        Connection() : ref_(2), ownArgumentTypes(true) {
            //ref_ is 2 for the use in the internal lists, and for the use in QMetaObject::Connection
        }
        ~Connection();
        int method() const { Q_ASSERT(!isSlotObject); return method_offset + method_relative; }
        void ref() { ref_.ref(); }
        void freeSlotObject()
        {
            if (isSlotObject) {
                slotObj->destroyIfLastRef();
                isSlotObject = false;
            }
        }
        void deref()
        {
            if (!ref_.deref()) {
                Q_ASSERT(!receiver.loadRelaxed());
                Q_ASSERT(!isSlotObject);
                delete this;
            }
        }
    };
    // ConnectionList is a singly-linked list
    struct ConnectionList {
        QAtomicPointer<Connection> first;
        QAtomicPointer<Connection> last;
    };

    struct Sender
    {
        Sender(QObject *receiver, QObject *sender, int signal)
            : receiver(receiver), sender(sender), signal(signal)
        {
            if (receiver) {
                ConnectionData *cd = receiver->d_func()->connections.loadRelaxed();
                previous = cd->currentSender;
                cd->currentSender = this;
            }
        }
        ~Sender()
        {
            if (receiver)
                receiver->d_func()->connections.loadRelaxed()->currentSender = previous;
        }
        void receiverDeleted()
        {
            Sender *s = this;
            while (s) {
                s->receiver = nullptr;
                s = s->previous;
            }
        }
        Sender *previous;
        QObject *receiver;
        QObject *sender;
        int signal;
    };

    struct SignalVector : public ConnectionOrSignalVector {
        quintptr allocated;
        // ConnectionList signals[]
        ConnectionList &at(int i)
        {
            return reinterpret_cast<ConnectionList *>(this + 1)[i + 1];
        }
        const ConnectionList &at(int i) const
        {
            return reinterpret_cast<const ConnectionList *>(this + 1)[i + 1];
        }
        int count() const { return static_cast<int>(allocated); }
    };

    /*
        This contains the all connections from and to an object.

        The signalVector contains the lists of connections for a given signal. The index in the vector correspond
        to the signal index. The signal index is the one returned by QObjectPrivate::signalIndex (not
        QMetaObject::indexOfSignal). allsignals contains a list of special connections that will get invoked on
        any signal emission. This is done by connecting to signal index -1.

        This vector is protected by the object mutex (signalSlotLock())

        Each Connection is also part of a 'senders' linked list. This one contains all connections connected
        to a slot in this object. The mutex of the receiver must be locked when touching the pointers of this
        linked list.
    */
    struct ConnectionData {
        // the id below is used to avoid activating new connections. When the object gets
        // deleted it's set to 0, so that signal emission stops
        QAtomicInteger<uint> currentConnectionId;
        QAtomicInt ref;
        QAtomicPointer<SignalVector> signalVector;
        Connection *senders = nullptr;
        Sender *currentSender = nullptr;   // object currently activating the object
        QAtomicPointer<Connection> orphaned;

        ~ConnectionData()
        {
            Q_ASSERT(ref.loadRelaxed() == 0);
            auto *c = orphaned.fetchAndStoreRelaxed(nullptr);
            if (c)
                deleteOrphaned(c);
            SignalVector *v = signalVector.loadRelaxed();
            if (v) {
                v->~SignalVector();
                free(v);
            }
        }

        // must be called on the senders connection data
        // assumes the senders and receivers lock are held
        void removeConnection(Connection *c);
        enum LockPolicy {
            NeedToLock,
            // Beware that we need to temporarily release the lock
            // and thus calling code must carefully consider whether
            // invariants still hold.
            AlreadyLockedAndTemporarilyReleasingLock
        };
        void cleanOrphanedConnections(QObject *sender, LockPolicy lockPolicy = NeedToLock)
        {
            if (orphaned.loadRelaxed() && ref.loadAcquire() == 1)
                cleanOrphanedConnectionsImpl(sender, lockPolicy);
        }
        void cleanOrphanedConnectionsImpl(QObject *sender, LockPolicy lockPolicy);

        ConnectionList &connectionsForSignal(int signal)
        {
            return signalVector.loadRelaxed()->at(signal);
        }

        void resizeSignalVector(uint size)
        {
            SignalVector *vector = this->signalVector.loadRelaxed();
            if (vector && vector->allocated > size)
                return;
            size = (size + 7) & ~7;
            void *ptr = malloc(sizeof(SignalVector) + (size + 1) * sizeof(ConnectionList));
            auto newVector = new (ptr) SignalVector;

            int start = -1;
            if (vector) {
                // not (yet) existing trait:
                //static_assert(std::is_relocatable_v<SignalVector>);
                //static_assert(std::is_relocatable_v<ConnectionList>);
                memcpy(newVector, vector, sizeof(SignalVector) + (vector->allocated + 1) * sizeof(ConnectionList));
                start = vector->count();
            }
            for (int i = start; i < int(size); ++i)
                new (&newVector->at(i)) ConnectionList();
            newVector->next = nullptr;
            newVector->allocated = size;

            signalVector.storeRelaxed(newVector);
            if (vector) {
                Connection *o = nullptr;
                /* No ABA issue here: When adding a node, we only care about the list head, it doesn't
                 * matter if the tail changes.
                 */
                do {
                    o = orphaned.loadRelaxed();
                    vector->nextInOrphanList = o;
                } while (!orphaned.testAndSetRelease(o, ConnectionOrSignalVector::fromSignalVector(vector)));

            }
        }
        int signalVectorCount() const
        {
            return signalVector.loadAcquire() ? signalVector.loadRelaxed()->count() : -1;
        }

        static void deleteOrphaned(ConnectionOrSignalVector *c);
    };

    QObjectPrivate(int version = QObjectPrivateVersion);
    virtual ~QObjectPrivate();
    void deleteChildren();
    // used to clear binding storage early in ~QObject
    void clearBindingStorage();

    inline void checkForIncompatibleLibraryVersion(int version) const;

    void setParent_helper(QObject *);
    void moveToThread_helper();
    void setThreadData_helper(QThreadData *currentData, QThreadData *targetData, QBindingStatus *status);
    void _q_reregisterTimers(void *pointer);

    bool isSender(const QObject *receiver, const char *signal) const;
    QObjectList receiverList(const char *signal) const;
    QObjectList senderList() const;

    void addConnection(int signal, Connection *c);

    static QObjectPrivate *get(QObject *o) { return o->d_func(); }
    static const QObjectPrivate *get(const QObject *o) { return o->d_func(); }

    int signalIndex(const char *signalName, const QMetaObject **meta = nullptr) const;
    bool isSignalConnected(uint signalIdx, bool checkDeclarative = true) const;
    bool maybeSignalConnected(uint signalIndex) const;
    inline bool isDeclarativeSignalConnected(uint signalIdx) const;

    // To allow abitrary objects to call connectNotify()/disconnectNotify() without making
    // the API public in QObject. This is used by QQmlNotifierEndpoint.
    inline void connectNotify(const QMetaMethod &signal);
    inline void disconnectNotify(const QMetaMethod &signal);

    void reinitBindingStorageAfterThreadMove();

    template <typename Func1, typename Func2>
    static inline QMetaObject::Connection connect(const typename QtPrivate::FunctionPointer<Func1>::Object *sender, Func1 signal,
                                                  const typename QtPrivate::FunctionPointer<Func2>::Object *receiverPrivate, Func2 slot,
                                                  Qt::ConnectionType type = Qt::AutoConnection);

    template <typename Func1, typename Func2>
    static inline bool disconnect(const typename QtPrivate::FunctionPointer<Func1>::Object *sender, Func1 signal,
                                  const typename QtPrivate::FunctionPointer<Func2>::Object *receiverPrivate, Func2 slot);

    static QMetaObject::Connection connectImpl(const QObject *sender, int signal_index,
                                               const QObject *receiver, void **slot,
                                               QtPrivate::QSlotObjectBase *slotObj, int type,
                                               const int *types, const QMetaObject *senderMetaObject);
    static QMetaObject::Connection connect(const QObject *sender, int signal_index, QtPrivate::QSlotObjectBase *slotObj, Qt::ConnectionType type);
    static QMetaObject::Connection connect(const QObject *sender, int signal_index,
                                           const QObject *receiver,
                                           QtPrivate::QSlotObjectBase *slotObj,
                                           Qt::ConnectionType type);
    static bool disconnect(const QObject *sender, int signal_index, void **slot);
    static bool disconnect(const QObject *sender, int signal_index, const QObject *receiver,
                           void **slot);
    static bool disconnect(Connection *c);

    void ensureConnectionData()
    {
        if (connections.loadRelaxed())
            return;
        ConnectionData *cd = new ConnectionData;
        cd->ref.ref();
        connections.storeRelaxed(cd);
    }

public:
    mutable ExtraData *extraData; // extra data set by the user
    // This atomic requires acquire/release semantics in a few places,
    // e.g. QObject::moveToThread must synchronize with QCoreApplication::postEvent,
    // because postEvent is thread-safe.
    // However, most of the code paths involving QObject are only reentrant and
    // not thread-safe, so synchronization should not be necessary there.
    QAtomicPointer<QThreadData> threadData; // id of the thread that owns the object

    using ConnectionDataPointer = QExplicitlySharedDataPointer<ConnectionData>;
    QAtomicPointer<ConnectionData> connections;

    union {
        QObject *currentChildBeingDeleted; // should only be used when QObjectData::isDeletingChildren is set
        QAbstractDeclarativeData *declarativeData; //extra data used by the declarative module
    };

    // these objects are all used to indicate that a QObject was deleted
    // plus QPointer, which keeps a separate list
    QAtomicPointer<QtSharedPointer::ExternalRefCountData> sharedRefcount;
};

Q_DECLARE_TYPEINFO(QObjectPrivate::ConnectionList, Q_RELOCATABLE_TYPE);

/*
    Catch mixing of incompatible library versions.

    Should be called from the constructor of every non-final subclass
    of QObjectPrivate, to ensure we catch incompatibilities between
    the intermediate base and subclasses thereof.
*/
inline void QObjectPrivate::checkForIncompatibleLibraryVersion(int version) const
{
#if defined(QT_BUILD_INTERNAL)
    // Don't check the version parameter in internal builds.
    // This allows incompatible versions to be loaded, possibly for testing.
    Q_UNUSED(version);
#else
    if (Q_UNLIKELY(version != QObjectPrivateVersion)) {
        qFatal("Cannot mix incompatible Qt library (%d.%d.%d) with this library (%d.%d.%d)",
                (version >> 16) & 0xff, (version >> 8) & 0xff, version & 0xff,
                (QObjectPrivateVersion >> 16) & 0xff, (QObjectPrivateVersion >> 8) & 0xff, QObjectPrivateVersion & 0xff);
    }
#endif
}

inline bool QObjectPrivate::isDeclarativeSignalConnected(uint signal_index) const
{
    return !isDeletingChildren && declarativeData && QAbstractDeclarativeData::isSignalConnected
            && QAbstractDeclarativeData::isSignalConnected(declarativeData, q_func(), signal_index);
}

inline void QObjectPrivate::connectNotify(const QMetaMethod &signal)
{
    q_ptr->connectNotify(signal);
}

inline void QObjectPrivate::disconnectNotify(const QMetaMethod &signal)
{
    q_ptr->disconnectNotify(signal);
}

namespace QtPrivate {
template<typename Func, typename Args, typename R> class QPrivateSlotObject : public QSlotObjectBase
{
    typedef QtPrivate::FunctionPointer<Func> FuncType;
    Func function;
    static void impl(int which, QSlotObjectBase *this_, QObject *r, void **a, bool *ret)
    {
        switch (which) {
            case Destroy:
                delete static_cast<QPrivateSlotObject*>(this_);
                break;
            case Call:
                FuncType::template call<Args, R>(static_cast<QPrivateSlotObject*>(this_)->function,
                                                 static_cast<typename FuncType::Object *>(QObjectPrivate::get(r)), a);
                break;
            case Compare:
                *ret = *reinterpret_cast<Func *>(a) == static_cast<QPrivateSlotObject*>(this_)->function;
                break;
            case NumOperations: ;
        }
    }
public:
    explicit QPrivateSlotObject(Func f) : QSlotObjectBase(&impl), function(f) {}
};
} //namespace QtPrivate

template <typename Func1, typename Func2>
inline QMetaObject::Connection QObjectPrivate::connect(const typename QtPrivate::FunctionPointer<Func1>::Object *sender, Func1 signal,
                                                       const typename QtPrivate::FunctionPointer<Func2>::Object *receiverPrivate, Func2 slot,
                                                       Qt::ConnectionType type)
{
    typedef QtPrivate::FunctionPointer<Func1> SignalType;
    typedef QtPrivate::FunctionPointer<Func2> SlotType;
    static_assert(QtPrivate::HasQ_OBJECT_Macro<typename SignalType::Object>::Value,
                      "No Q_OBJECT in the class with the signal");

    //compilation error if the arguments does not match.
    static_assert(int(SignalType::ArgumentCount) >= int(SlotType::ArgumentCount),
                      "The slot requires more arguments than the signal provides.");
    static_assert((QtPrivate::CheckCompatibleArguments<typename SignalType::Arguments, typename SlotType::Arguments>::value),
                      "Signal and slot arguments are not compatible.");
    static_assert((QtPrivate::AreArgumentsCompatible<typename SlotType::ReturnType, typename SignalType::ReturnType>::value),
                      "Return type of the slot is not compatible with the return type of the signal.");

    const int *types = nullptr;
    if (type == Qt::QueuedConnection || type == Qt::BlockingQueuedConnection)
        types = QtPrivate::ConnectionTypes<typename SignalType::Arguments>::types();

    return QObject::connectImpl(sender, reinterpret_cast<void **>(&signal),
        receiverPrivate->q_ptr, reinterpret_cast<void **>(&slot),
        new QtPrivate::QPrivateSlotObject<Func2, typename QtPrivate::List_Left<typename SignalType::Arguments, SlotType::ArgumentCount>::Value,
                                        typename SignalType::ReturnType>(slot),
        type, types, &SignalType::Object::staticMetaObject);
}

template <typename Func1, typename Func2>
bool QObjectPrivate::disconnect(const typename QtPrivate::FunctionPointer< Func1 >::Object* sender, Func1 signal,
                                const typename QtPrivate::FunctionPointer< Func2 >::Object* receiverPrivate, Func2 slot)
{
    typedef QtPrivate::FunctionPointer<Func1> SignalType;
    typedef QtPrivate::FunctionPointer<Func2> SlotType;
    static_assert(QtPrivate::HasQ_OBJECT_Macro<typename SignalType::Object>::Value,
                      "No Q_OBJECT in the class with the signal");
    //compilation error if the arguments does not match.
    static_assert((QtPrivate::CheckCompatibleArguments<typename SignalType::Arguments, typename SlotType::Arguments>::value),
                      "Signal and slot arguments are not compatible.");
    return QObject::disconnectImpl(sender, reinterpret_cast<void **>(&signal),
                          receiverPrivate->q_ptr, reinterpret_cast<void **>(&slot),
                          &SignalType::Object::staticMetaObject);
}

Q_DECLARE_TYPEINFO(QObjectPrivate::Connection, Q_RELOCATABLE_TYPE);
Q_DECLARE_TYPEINFO(QObjectPrivate::Sender, Q_RELOCATABLE_TYPE);

class QSemaphore;
class Q_CORE_EXPORT QAbstractMetaCallEvent : public QEvent
{
public:
    QAbstractMetaCallEvent(const QObject *sender, int signalId, QSemaphore *semaphore = nullptr)
        : QEvent(MetaCall), signalId_(signalId), sender_(sender)
#if QT_CONFIG(thread)
        , semaphore_(semaphore)
#endif
    { Q_UNUSED(semaphore); }
    ~QAbstractMetaCallEvent();

    virtual void placeMetaCall(QObject *object) = 0;

    inline const QObject *sender() const { return sender_; }
    inline int signalId() const { return signalId_; }

private:
    int signalId_;
    const QObject *sender_;
#if QT_CONFIG(thread)
    QSemaphore *semaphore_;
#endif
};

class Q_CORE_EXPORT QMetaCallEvent : public QAbstractMetaCallEvent
{
public:
    // blocking queued with semaphore - args always owned by caller
    QMetaCallEvent(ushort method_offset, ushort method_relative,
                   QObjectPrivate::StaticMetaCallFunction callFunction,
                   const QObject *sender, int signalId,
                   void **args, QSemaphore *semaphore);
    QMetaCallEvent(QtPrivate::QSlotObjectBase *slotObj,
                   const QObject *sender, int signalId,
                   void **args, QSemaphore *semaphore);

    // queued - args allocated by event, copied by caller
    QMetaCallEvent(ushort method_offset, ushort method_relative,
                   QObjectPrivate::StaticMetaCallFunction callFunction,
                   const QObject *sender, int signalId,
                   int nargs);
    QMetaCallEvent(QtPrivate::QSlotObjectBase *slotObj,
                   const QObject *sender, int signalId,
                   int nargs);

    ~QMetaCallEvent() override;

    inline int id() const { return d.method_offset_ + d.method_relative_; }
    inline const void * const* args() const { return d.args_; }
    inline void ** args() { return d.args_; }
    inline const QMetaType *types() const { return reinterpret_cast<QMetaType *>(d.args_ + d.nargs_); }
    inline QMetaType *types() { return reinterpret_cast<QMetaType *>(d.args_ + d.nargs_); }

    virtual void placeMetaCall(QObject *object) override;

private:
    inline void allocArgs();

    struct Data {
        QtPrivate::QSlotObjectBase *slotObj_;
        void **args_;
        QObjectPrivate::StaticMetaCallFunction callFunction_;
        int nargs_;
        ushort method_offset_;
        ushort method_relative_;
    } d;
    // preallocate enough space for three arguments
    alignas(void *) char prealloc_[3 * sizeof(void *) + 3 * sizeof(QMetaType)];
};

class QBoolBlocker
{
    Q_DISABLE_COPY_MOVE(QBoolBlocker)
public:
    explicit inline QBoolBlocker(bool &b, bool value = true) : block(b), reset(b) { block = value; }
    inline ~QBoolBlocker() { block = reset; }

private:
    bool &block;
    bool reset;
};

void Q_CORE_EXPORT qDeleteInEventHandler(QObject *o);

struct QAbstractDynamicMetaObject;
struct Q_CORE_EXPORT QDynamicMetaObjectData
{
    virtual ~QDynamicMetaObjectData();
    virtual void objectDestroyed(QObject *) { delete this; }

    virtual QAbstractDynamicMetaObject *toDynamicMetaObject(QObject *) = 0;
    virtual int metaCall(QObject *, QMetaObject::Call, int _id, void **) = 0;
};

struct Q_CORE_EXPORT QAbstractDynamicMetaObject : public QDynamicMetaObjectData, public QMetaObject
{
    ~QAbstractDynamicMetaObject();

    QAbstractDynamicMetaObject *toDynamicMetaObject(QObject *) override { return this; }
    virtual int createProperty(const char *, const char *) { return -1; }
    int metaCall(QObject *, QMetaObject::Call c, int _id, void **a) override
    { return metaCall(c, _id, a); }
    virtual int metaCall(QMetaObject::Call, int _id, void **) { return _id; } // Compat overload
};

inline const QBindingStorage *qGetBindingStorage(const QObjectPrivate *o)
{
    return &o->bindingStorage;
}
inline QBindingStorage *qGetBindingStorage(QObjectPrivate *o)
{
    return &o->bindingStorage;
}
inline const QBindingStorage *qGetBindingStorage(const QObjectPrivate::ExtraData *ed)
{
    return &ed->parent->bindingStorage;
}
inline QBindingStorage *qGetBindingStorage(QObjectPrivate::ExtraData *ed)
{
    return &ed->parent->bindingStorage;
}

QT_END_NAMESPACE

#endif // QOBJECT_P_H
