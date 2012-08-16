/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtDeclarative module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QDECLARATIVEVISUALDATAMODEL_H
#define QDECLARATIVEVISUALDATAMODEL_H

#include <qdeclarative.h>

#include <QtCore/qobject.h>
#include <QtCore/qabstractitemmodel.h>

QT_BEGIN_HEADER

Q_DECLARE_METATYPE(QModelIndex)

QT_BEGIN_NAMESPACE

QT_MODULE(Declarative)

class QDeclarativeItem;
class QDeclarativeComponent;
class QDeclarativePackage;
class QDeclarativeVisualDataModelPrivate;

class Q_AUTOTEST_EXPORT QDeclarativeVisualModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    virtual ~QDeclarativeVisualModel() {}

    enum ReleaseFlag { Referenced = 0x01, Destroyed = 0x02 };
    Q_DECLARE_FLAGS(ReleaseFlags, ReleaseFlag)

    virtual int count() const = 0;
    virtual bool isValid() const = 0;
    virtual QDeclarativeItem *item(int index, bool complete=true) = 0;
    virtual ReleaseFlags release(QDeclarativeItem *item) = 0;
    virtual bool completePending() const = 0;
    virtual void completeItem() = 0;
    virtual QString stringValue(int, const QString &) = 0;
    virtual void setWatchedRoles(QList<QByteArray> roles) = 0;

    virtual int indexOf(QDeclarativeItem *item, QObject *objectContext) const = 0;

Q_SIGNALS:
    void countChanged();
    void itemsInserted(int index, int count);
    void itemsRemoved(int index, int count);
    void itemsMoved(int from, int to, int count);
    void itemsChanged(int index, int count);
    void modelReset();
    void createdItem(int index, QDeclarativeItem *item);
    void destroyingItem(QDeclarativeItem *item);

protected:
    QDeclarativeVisualModel(QObjectPrivate &dd, QObject *parent = 0)
        : QObject(dd, parent) {}

private:
    Q_DISABLE_COPY(QDeclarativeVisualModel)
};

class QDeclarativeVisualItemModelAttached;
class QDeclarativeVisualItemModelPrivate;
class Q_AUTOTEST_EXPORT QDeclarativeVisualItemModel : public QDeclarativeVisualModel
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QDeclarativeVisualItemModel)

    Q_PROPERTY(QDeclarativeListProperty<QDeclarativeItem> children READ children NOTIFY childrenChanged DESIGNABLE false)
    Q_CLASSINFO("DefaultProperty", "children")

public:
    QDeclarativeVisualItemModel(QObject *parent=0);
    virtual ~QDeclarativeVisualItemModel() {}

    virtual int count() const;
    virtual bool isValid() const;
    virtual QDeclarativeItem *item(int index, bool complete=true);
    virtual ReleaseFlags release(QDeclarativeItem *item);
    virtual bool completePending() const;
    virtual void completeItem();
    virtual QString stringValue(int index, const QString &role);
    virtual void setWatchedRoles(QList<QByteArray>) {}

    virtual int indexOf(QDeclarativeItem *item, QObject *objectContext) const;

    QDeclarativeListProperty<QDeclarativeItem> children();

    static QDeclarativeVisualItemModelAttached *qmlAttachedProperties(QObject *obj);

Q_SIGNALS:
    void childrenChanged();

private:
    Q_DISABLE_COPY(QDeclarativeVisualItemModel)
};


class Q_AUTOTEST_EXPORT QDeclarativeVisualDataModel : public QDeclarativeVisualModel
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QDeclarativeVisualDataModel)

    Q_PROPERTY(QVariant model READ model WRITE setModel)
    Q_PROPERTY(QDeclarativeComponent *delegate READ delegate WRITE setDelegate)
    Q_PROPERTY(QString part READ part WRITE setPart)
    Q_PROPERTY(QObject *parts READ parts CONSTANT)
    Q_PROPERTY(QVariant rootIndex READ rootIndex WRITE setRootIndex NOTIFY rootIndexChanged)
    Q_CLASSINFO("DefaultProperty", "delegate")
public:
    QDeclarativeVisualDataModel();
    QDeclarativeVisualDataModel(QDeclarativeContext *, QObject *parent=0);
    virtual ~QDeclarativeVisualDataModel();

    QVariant model() const;
    void setModel(const QVariant &);

    QDeclarativeComponent *delegate() const;
    void setDelegate(QDeclarativeComponent *);

    QVariant rootIndex() const;
    void setRootIndex(const QVariant &root);

    Q_INVOKABLE QVariant modelIndex(int idx) const;
    Q_INVOKABLE QVariant parentModelIndex() const;

    QString part() const;
    void setPart(const QString &);

    int count() const;
    bool isValid() const { return delegate() != 0; }
    QDeclarativeItem *item(int index, bool complete=true);
    QDeclarativeItem *item(int index, const QByteArray &, bool complete=true);
    ReleaseFlags release(QDeclarativeItem *item);
    bool completePending() const;
    void completeItem();
    virtual QString stringValue(int index, const QString &role);
    virtual void setWatchedRoles(QList<QByteArray> roles);

    int indexOf(QDeclarativeItem *item, QObject *objectContext) const;

    QObject *parts();

Q_SIGNALS:
    void createdPackage(int index, QDeclarativePackage *package);
    void destroyingPackage(QDeclarativePackage *package);
    void rootIndexChanged();

private Q_SLOTS:
    void _q_itemsChanged(int, int, const QList<int> &);
    void _q_itemsInserted(int index, int count);
    void _q_itemsRemoved(int index, int count);
    void _q_itemsMoved(int from, int to, int count);
    void _q_rowsInserted(const QModelIndex &,int,int);
    void _q_rowsRemoved(const QModelIndex &,int,int);
    void _q_rowsMoved(const QModelIndex &, int, int, const QModelIndex &, int);
    void _q_dataChanged(const QModelIndex&,const QModelIndex&);
    void _q_layoutChanged();
    void _q_modelReset();
    void _q_createdPackage(int index, QDeclarativePackage *package);
    void _q_destroyingPackage(QDeclarativePackage *package);

private:
    Q_DISABLE_COPY(QDeclarativeVisualDataModel)
};

class QDeclarativeVisualItemModelAttached : public QObject
{
    Q_OBJECT

public:
    QDeclarativeVisualItemModelAttached(QObject *parent)
        : QObject(parent), m_index(0) {}
    ~QDeclarativeVisualItemModelAttached() {
        attachedProperties.remove(parent());
    }

    Q_PROPERTY(int index READ index NOTIFY indexChanged)
    int index() const { return m_index; }
    void setIndex(int idx) {
        if (m_index != idx) {
            m_index = idx;
            emit indexChanged();
        }
    }

    static QDeclarativeVisualItemModelAttached *properties(QObject *obj) {
        QDeclarativeVisualItemModelAttached *rv = attachedProperties.value(obj);
        if (!rv) {
            rv = new QDeclarativeVisualItemModelAttached(obj);
            attachedProperties.insert(obj, rv);
        }
        return rv;
    }

Q_SIGNALS:
    void indexChanged();

public:
    int m_index;

    static QHash<QObject*, QDeclarativeVisualItemModelAttached*> attachedProperties;
};


QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeVisualModel)
QML_DECLARE_TYPE(QDeclarativeVisualItemModel)
QML_DECLARE_TYPEINFO(QDeclarativeVisualItemModel, QML_HAS_ATTACHED_PROPERTIES)
QML_DECLARE_TYPE(QDeclarativeVisualDataModel)

QT_END_HEADER

#endif // QDECLARATIVEVISUALDATAMODEL_H
