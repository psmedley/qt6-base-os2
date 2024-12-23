/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtDBus module of the Qt Toolkit.
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

#include "qdbusmetatype.h"
#include "qdbusmetatype_p.h"

#include <string.h>
#include "qdbus_symbols_p.h"

#include <qbytearray.h>
#include <qglobal.h>
#include <qlist.h>
#include <qreadwritelock.h>
#include <qdatetime.h>
#include <qrect.h>
#include <qsize.h>
#include <qpoint.h>
#include <qline.h>

#include "qdbusargument_p.h"
#include "qdbusutil_p.h"
#include "qdbusunixfiledescriptor.h"
#ifndef QT_BOOTSTRAPPED
#include "qdbusmessage.h"
#endif

#ifndef QT_NO_DBUS

#ifndef DBUS_TYPE_UNIX_FD
# define DBUS_TYPE_UNIX_FD int('h')
# define DBUS_TYPE_UNIX_FD_AS_STRING "h"
#endif

QT_BEGIN_NAMESPACE

class QDBusCustomTypeInfo
{
public:
    QDBusCustomTypeInfo() : signature(), marshall(nullptr), demarshall(nullptr)
    { }

    // Suggestion:
    // change 'signature' to char* and make QDBusCustomTypeInfo a Movable type
    QByteArray signature;
    QDBusMetaType::MarshallFunction marshall;
    QDBusMetaType::DemarshallFunction demarshall;
};

void QDBusMetaTypeId::init()
{
    static QBasicAtomicInt initialized = Q_BASIC_ATOMIC_INITIALIZER(false);

    // reentrancy is not a problem since everything else is locked on their own
    // set the guard variable at the end
    if (!initialized.loadRelaxed()) {
        // register our types with Qt Core (calling qMetaTypeId<T>() does this implicitly)
        (void)message();
        (void)argument();
        (void)variant();
        (void)objectpath();
        (void)signature();
        (void)error();
        (void)unixfd();

#ifndef QDBUS_NO_SPECIALTYPES
        // and register Qt Core's with us
        qDBusRegisterMetaType<QDate>();
        qDBusRegisterMetaType<QTime>();
        qDBusRegisterMetaType<QDateTime>();
        qDBusRegisterMetaType<QRect>();
        qDBusRegisterMetaType<QRectF>();
        qDBusRegisterMetaType<QSize>();
        qDBusRegisterMetaType<QSizeF>();
        qDBusRegisterMetaType<QPoint>();
        qDBusRegisterMetaType<QPointF>();
        qDBusRegisterMetaType<QLine>();
        qDBusRegisterMetaType<QLineF>();
        qDBusRegisterMetaType<QVariantList>();
        qDBusRegisterMetaType<QVariantMap>();
        qDBusRegisterMetaType<QVariantHash>();
        qDBusRegisterMetaType<QDBusObjectPath>();

        qDBusRegisterMetaType<QList<bool> >();
        qDBusRegisterMetaType<QList<short> >();
        qDBusRegisterMetaType<QList<ushort> >();
        qDBusRegisterMetaType<QList<int> >();
        qDBusRegisterMetaType<QList<uint> >();
        qDBusRegisterMetaType<QList<qlonglong> >();
        qDBusRegisterMetaType<QList<qulonglong> >();
        qDBusRegisterMetaType<QList<double> >();
        qDBusRegisterMetaType<QList<QDBusObjectPath> >();
        qDBusRegisterMetaType<QList<QDBusSignature> >();
        qDBusRegisterMetaType<QList<QDBusUnixFileDescriptor> >();
#endif

        initialized.storeRelaxed(true);
    }
}

using QDBusCustomTypeHash = QHash<int, QDBusCustomTypeInfo>;
Q_GLOBAL_STATIC(QDBusCustomTypeHash, customTypes)
Q_GLOBAL_STATIC(QReadWriteLock, customTypesLock)

/*!
    \class QDBusMetaType
    \inmodule QtDBus
    \brief Meta-type registration system for the Qt D-Bus module.
    \internal

    The QDBusMetaType class allows you to register class types for
    marshalling and demarshalling over D-Bus. D-Bus supports a very
    limited set of primitive types, but allows one to extend the type
    system by creating compound types, such as arrays (lists) and
    structs. In order to use them with Qt D-Bus, those types must be
    registered.

    See \l {qdbustypesystem.html}{Qt D-Bus Type System} for more
    information on the type system and how to register additional
    types.

    \sa {qdbustypesystem.html}{Qt D-Bus Type System},
    qDBusRegisterMetaType(), QMetaType, QVariant, QDBusArgument
*/

/*!
    \fn int qDBusRegisterMetaType()
    \relates QDBusArgument
    \threadsafe
    \since 4.2

    Registers \c{T} with the
    \l {qdbustypesystem.html}{Qt D-Bus Type System} and the Qt \l
    {QMetaType}{meta-type system}, if it's not already registered.

    To register a type, it must be declared as a meta-type with the
    Q_DECLARE_METATYPE() macro, and then registered as in the
    following example:

    \snippet code/src_qdbus_qdbusmetatype.cpp 0-0
    \codeline
    \snippet code/src_qdbus_qdbusmetatype.cpp 0-1

    If \c{T} isn't one of
    Qt's \l{container classes}, the \c{operator<<} and
    \c{operator>>} streaming operators between \c{T} and QDBusArgument
    must be already declared. See the \l {qdbustypesystem.html}{Qt D-Bus
    Type System} page for more information on how to declare such
    types.

    This function returns the Qt meta type id for the type (the same
    value that is returned from qRegisterMetaType()).

    \note The feature that a \c{T} inheriting a streamable type (including
    the containers QList, QHash or QMap) can be streamed without providing
    custom \c{operator<<} and \c{operator>>} is deprecated as of Qt 5.7,
    because it ignores everything in \c{T} except the base class. There is
    no diagnostic. You should always provide these operators for all types
    you wish to stream and not rely on Qt-provided stream operators for
    base classes.

    \sa {qdbustypesystem.html}{Qt D-Bus Type System}, qRegisterMetaType(), QMetaType
*/

/*!
    \typedef QDBusMetaType::MarshallFunction
    \internal
*/

/*!
    \typedef QDBusMetaType::DemarshallFunction
    \internal
*/

/*!
    \internal
    Registers the marshalling and demarshalling functions for meta
    type \a metaType.
*/
void QDBusMetaType::registerMarshallOperators(QMetaType metaType, MarshallFunction mf,
                                              DemarshallFunction df)
{
    int id = metaType.id();
    auto *ct = customTypes();
    if (id < 0 || !mf || !df || !ct)
        return;                 // error!

    QWriteLocker locker(customTypesLock());
    QDBusCustomTypeInfo &info = (*ct)[id];
    info.marshall = mf;
    info.demarshall = df;
}

/*!
    \internal
    Executes the marshalling of type \a metaType (whose data is contained in
    \a data) to the D-Bus marshalling argument \a arg. Returns \c true if
    the marshalling succeeded, or false if an error occurred.
*/
bool QDBusMetaType::marshall(QDBusArgument &arg, QMetaType metaType, const void *data)
{
    int id = metaType.id();
    QDBusMetaTypeId::init();

    MarshallFunction mf;
    {
        QReadLocker locker(customTypesLock());
        auto *ct = customTypes();
        auto it = ct->constFind(id);
        if (it == ct->cend())
            return false;       // non-existent

        const QDBusCustomTypeInfo &info = *it;
        if (!info.marshall) {
            mf = nullptr;       // make gcc happy
            return false;
        } else
            mf = info.marshall;
    }

    mf(arg, data);
    return true;
}

/*!
    \internal
    Executes the demarshalling of type \a metaType (whose data will be placed in
    \a data) from the D-Bus marshalling argument \a arg. Returns \c true if
    the demarshalling succeeded, or false if an error occurred.
*/
bool QDBusMetaType::demarshall(const QDBusArgument &arg, QMetaType metaType, void *data)
{
    int id = metaType.id();
    QDBusMetaTypeId::init();

    DemarshallFunction df;
    {
        QReadLocker locker(customTypesLock());
        auto *ct = customTypes();
        auto it = ct->constFind(id);
        if (it == ct->cend())
            return false;       // non-existent

        const QDBusCustomTypeInfo &info = *it;
        if (!info.demarshall) {
            df = nullptr;       // make gcc happy
            return false;
        } else
            df = info.demarshall;
    }
#ifndef QT_BOOTSTRAPPED
    QDBusArgument copy = arg;
    df(copy, data);
#else
    Q_UNUSED(arg);
    Q_UNUSED(data);
    Q_UNUSED(df);
#endif
    return true;
}

/*!
    \fn QDBusMetaType::signatureToType(const char *signature)
    \internal

    Returns the Qt meta type id for the given D-Bus signature for exactly one full type, given
    by \a signature.

    Note: this function only handles the basic D-Bus types.

    \sa QDBusUtil::isValidSingleSignature(), typeToSignature(),
        QVariant::metaType()
*/
QMetaType QDBusMetaType::signatureToMetaType(const char *signature)
{
    if (!signature)
        return QMetaType(QMetaType::UnknownType);

    QDBusMetaTypeId::init();
    switch (signature[0])
    {
    case DBUS_TYPE_BOOLEAN:
        return QMetaType(QMetaType::Bool);

    case DBUS_TYPE_BYTE:
        return QMetaType(QMetaType::UChar);

    case DBUS_TYPE_INT16:
        return QMetaType(QMetaType::Short);

    case DBUS_TYPE_UINT16:
        return QMetaType(QMetaType::UShort);

    case DBUS_TYPE_INT32:
        return QMetaType(QMetaType::Int);

    case DBUS_TYPE_UINT32:
        return QMetaType(QMetaType::UInt);

    case DBUS_TYPE_INT64:
        return QMetaType(QMetaType::LongLong);

    case DBUS_TYPE_UINT64:
        return QMetaType(QMetaType::ULongLong);

    case DBUS_TYPE_DOUBLE:
        return QMetaType(QMetaType::Double);

    case DBUS_TYPE_STRING:
        return QMetaType(QMetaType::QString);

    case DBUS_TYPE_OBJECT_PATH:
        return QDBusMetaTypeId::objectpath();

    case DBUS_TYPE_SIGNATURE:
        return QDBusMetaTypeId::signature();

    case DBUS_TYPE_UNIX_FD:
        return QDBusMetaTypeId::unixfd();

    case DBUS_TYPE_VARIANT:
        return QDBusMetaTypeId::variant();

    case DBUS_TYPE_ARRAY:       // special case
        switch (signature[1]) {
        case DBUS_TYPE_BYTE:
            return QMetaType(QMetaType::QByteArray);

        case DBUS_TYPE_STRING:
            return QMetaType(QMetaType::QStringList);

        case DBUS_TYPE_VARIANT:
            return QMetaType(QMetaType::QVariantList);

        case DBUS_TYPE_OBJECT_PATH:
            return QMetaType::fromType<QList<QDBusObjectPath> >();

        case DBUS_TYPE_SIGNATURE:
            return QMetaType::fromType<QList<QDBusSignature> >();

        }
        Q_FALLTHROUGH();
    default:
        return QMetaType(QMetaType::UnknownType);
    }
}

/*!
    \fn QDBusMetaType::typeToSignature(int type)
    \internal

    Returns the D-Bus signature equivalent to the supplied meta type id \a type.

    More types can be registered with the qDBusRegisterMetaType() function.

    \sa QDBusUtil::isValidSingleSignature(), signatureToType(),
        QVariant::metaType()
*/
const char *QDBusMetaType::typeToSignature(QMetaType type)
{
    // check if it's a static type
    switch (type.id())
    {
    case QMetaType::UChar:
        return DBUS_TYPE_BYTE_AS_STRING;

    case QMetaType::Bool:
        return DBUS_TYPE_BOOLEAN_AS_STRING;

    case QMetaType::Short:
        return DBUS_TYPE_INT16_AS_STRING;

    case QMetaType::UShort:
        return DBUS_TYPE_UINT16_AS_STRING;

    case QMetaType::Int:
        return DBUS_TYPE_INT32_AS_STRING;

    case QMetaType::UInt:
        return DBUS_TYPE_UINT32_AS_STRING;

    case QMetaType::LongLong:
        return DBUS_TYPE_INT64_AS_STRING;

    case QMetaType::ULongLong:
        return DBUS_TYPE_UINT64_AS_STRING;

    case QMetaType::Double:
        return DBUS_TYPE_DOUBLE_AS_STRING;

    case QMetaType::QString:
        return DBUS_TYPE_STRING_AS_STRING;

    case QMetaType::QStringList:
        return DBUS_TYPE_ARRAY_AS_STRING
            DBUS_TYPE_STRING_AS_STRING; // as

    case QMetaType::QByteArray:
        return DBUS_TYPE_ARRAY_AS_STRING
            DBUS_TYPE_BYTE_AS_STRING; // ay
    }

    QDBusMetaTypeId::init();
    if (type == QDBusMetaTypeId::variant())
        return DBUS_TYPE_VARIANT_AS_STRING;
    else if (type == QDBusMetaTypeId::objectpath())
        return DBUS_TYPE_OBJECT_PATH_AS_STRING;
    else if (type == QDBusMetaTypeId::signature())
        return DBUS_TYPE_SIGNATURE_AS_STRING;
    else if (type == QDBusMetaTypeId::unixfd())
        return DBUS_TYPE_UNIX_FD_AS_STRING;

    // try the database
    auto *ct = customTypes();
    {
        QReadLocker locker(customTypesLock());
        auto it = ct->constFind(type.id());
        if (it == ct->end())
            return nullptr;

        const QDBusCustomTypeInfo &info = *it;

        if (!info.signature.isNull())
            return info.signature;

        if (!info.marshall)
            return nullptr;           // type not registered with us
    }

    // call to user code to construct the signature type
    QDBusCustomTypeInfo *info;
    {
        // createSignature will never return a null QByteArray
        // if there was an error, it'll return ""
        QByteArray signature = QDBusArgumentPrivate::createSignature(type.id());

        // re-acquire lock
        QWriteLocker locker(customTypesLock());
        info = &(*ct)[type.id()];
        info->signature = signature;
    }
    return info->signature;
}

QT_END_NAMESPACE

#endif // QT_NO_DBUS
