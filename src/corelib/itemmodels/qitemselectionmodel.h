/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
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

#ifndef QITEMSELECTIONMODEL_H
#define QITEMSELECTIONMODEL_H

#include <QtCore/qglobal.h>

#include <QtCore/qabstractitemmodel.h>
#include <QtCore/qlist.h>
#include <QtCore/qset.h>

QT_REQUIRE_CONFIG(itemmodel);

QT_BEGIN_NAMESPACE

class Q_CORE_EXPORT QItemSelectionRange
{

public:
    QItemSelectionRange() = default;
    QItemSelectionRange(const QModelIndex &topL, const QModelIndex &bottomR) : tl(topL), br(bottomR) {}
    explicit QItemSelectionRange(const QModelIndex &index) : tl(index), br(tl) {}

    void swap(QItemSelectionRange &other) noexcept
    {
        tl.swap(other.tl);
        br.swap(other.br);
    }

    inline int top() const { return tl.row(); }
    inline int left() const { return tl.column(); }
    inline int bottom() const { return br.row(); }
    inline int right() const { return br.column(); }
    inline int width() const { return br.column() - tl.column() + 1; }
    inline int height() const { return br.row() - tl.row() + 1; }

    inline const QPersistentModelIndex &topLeft() const { return tl; }
    inline const QPersistentModelIndex &bottomRight() const { return br; }
    inline QModelIndex parent() const { return tl.parent(); }
    inline const QAbstractItemModel *model() const { return tl.model(); }

    inline bool contains(const QModelIndex &index) const
    {
        return (parent() == index.parent()
                && tl.row() <= index.row() && tl.column() <= index.column()
                && br.row() >= index.row() && br.column() >= index.column());
    }

    inline bool contains(int row, int column, const QModelIndex &parentIndex) const
    {
        return (parent() == parentIndex
                && tl.row() <= row && tl.column() <= column
                && br.row() >= row && br.column() >= column);
    }

    bool intersects(const QItemSelectionRange &other) const;
    QItemSelectionRange intersected(const QItemSelectionRange &other) const;


    inline bool operator==(const QItemSelectionRange &other) const
        { return (tl == other.tl && br == other.br); }
    inline bool operator!=(const QItemSelectionRange &other) const
        { return !operator==(other); }

    inline bool isValid() const
    {
        return (tl.isValid() && br.isValid() && tl.parent() == br.parent()
                && top() <= bottom() && left() <= right());
    }

    bool isEmpty() const;

    QModelIndexList indexes() const;

private:
    QPersistentModelIndex tl, br;
};
Q_DECLARE_TYPEINFO(QItemSelectionRange, Q_RELOCATABLE_TYPE);

class QItemSelection;
class QItemSelectionModelPrivate;

class Q_CORE_EXPORT QItemSelectionModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel *model READ model WRITE setModel NOTIFY modelChanged
               BINDABLE bindableModel)
    Q_PROPERTY(bool hasSelection READ hasSelection NOTIFY selectionChanged STORED false
               DESIGNABLE false)
    Q_PROPERTY(QModelIndex currentIndex READ currentIndex NOTIFY currentChanged STORED false
               DESIGNABLE false)
    Q_PROPERTY(QItemSelection selection READ selection NOTIFY selectionChanged STORED false
               DESIGNABLE false)
    Q_PROPERTY(QModelIndexList selectedIndexes READ selectedIndexes NOTIFY selectionChanged
               STORED false DESIGNABLE false)

    Q_DECLARE_PRIVATE(QItemSelectionModel)

public:

    enum SelectionFlag {
        NoUpdate       = 0x0000,
        Clear          = 0x0001,
        Select         = 0x0002,
        Deselect       = 0x0004,
        Toggle         = 0x0008,
        Current        = 0x0010,
        Rows           = 0x0020,
        Columns        = 0x0040,
        SelectCurrent  = Select | Current,
        ToggleCurrent  = Toggle | Current,
        ClearAndSelect = Clear | Select
    };

    Q_DECLARE_FLAGS(SelectionFlags, SelectionFlag)
    Q_FLAG(SelectionFlags)

    explicit QItemSelectionModel(QAbstractItemModel *model = nullptr);
    explicit QItemSelectionModel(QAbstractItemModel *model, QObject *parent);
    virtual ~QItemSelectionModel();

    QModelIndex currentIndex() const;

    Q_INVOKABLE bool isSelected(const QModelIndex &index) const;
    Q_INVOKABLE bool isRowSelected(int row, const QModelIndex &parent = QModelIndex()) const;
    Q_INVOKABLE bool isColumnSelected(int column, const QModelIndex &parent = QModelIndex()) const;

    Q_INVOKABLE bool rowIntersectsSelection(int row, const QModelIndex &parent = QModelIndex()) const;
    Q_INVOKABLE bool columnIntersectsSelection(int column, const QModelIndex &parent = QModelIndex()) const;

    bool hasSelection() const;

    QModelIndexList selectedIndexes() const;
    Q_INVOKABLE QModelIndexList selectedRows(int column = 0) const;
    Q_INVOKABLE QModelIndexList selectedColumns(int row = 0) const;
    const QItemSelection selection() const;

    const QAbstractItemModel *model() const;
    QAbstractItemModel *model();
    QBindable<QAbstractItemModel *> bindableModel();

    void setModel(QAbstractItemModel *model);

public Q_SLOTS:
    virtual void setCurrentIndex(const QModelIndex &index, QItemSelectionModel::SelectionFlags command);
    virtual void select(const QModelIndex &index, QItemSelectionModel::SelectionFlags command);
    virtual void select(const QItemSelection &selection, QItemSelectionModel::SelectionFlags command);
    virtual void clear();
    virtual void reset();

    void clearSelection();
    virtual void clearCurrentIndex();

Q_SIGNALS:
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void currentChanged(const QModelIndex &current, const QModelIndex &previous);
    void currentRowChanged(const QModelIndex &current, const QModelIndex &previous);
    void currentColumnChanged(const QModelIndex &current, const QModelIndex &previous);
    void modelChanged(QAbstractItemModel *model);

protected:
    QItemSelectionModel(QItemSelectionModelPrivate &dd, QAbstractItemModel *model);
    void emitSelectionChanged(const QItemSelection &newSelection, const QItemSelection &oldSelection);

private:
    Q_DISABLE_COPY(QItemSelectionModel)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QItemSelectionModel::SelectionFlags)

// We export each out-of-line method individually to prevent MSVC from
// exporting the whole QList class.
class QItemSelection : public QList<QItemSelectionRange>
{
public:
    using QList<QItemSelectionRange>::QList;
    Q_CORE_EXPORT QItemSelection(const QModelIndex &topLeft, const QModelIndex &bottomRight);

    // reusing QList::swap() here is OK!

    Q_CORE_EXPORT void select(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    Q_CORE_EXPORT bool contains(const QModelIndex &index) const;
    Q_CORE_EXPORT QModelIndexList indexes() const;
    Q_CORE_EXPORT void merge(const QItemSelection &other, QItemSelectionModel::SelectionFlags command);
    Q_CORE_EXPORT static void split(const QItemSelectionRange &range,
                      const QItemSelectionRange &other,
                      QItemSelection *result);
};
Q_DECLARE_SHARED(QItemSelection)

#ifndef QT_NO_DEBUG_STREAM
Q_CORE_EXPORT QDebug operator<<(QDebug, const QItemSelectionRange &);
#endif

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QItemSelectionRange)
Q_DECLARE_METATYPE(QItemSelection)

#endif // QITEMSELECTIONMODEL_H
