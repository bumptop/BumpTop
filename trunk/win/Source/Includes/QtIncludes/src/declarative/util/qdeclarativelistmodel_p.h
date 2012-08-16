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

#ifndef QDECLARATIVELISTMODEL_H
#define QDECLARATIVELISTMODEL_H

#include <qdeclarative.h>
#include <private/qdeclarativecustomparser_p.h>

#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QVariant>
#include <private/qlistmodelinterface_p.h>
#include <QtScript/qscriptvalue.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Declarative)

class FlatListModel;
class NestedListModel;
class QDeclarativeListModelWorkerAgent;
struct ModelNode;
class FlatListScriptClass;
class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativeListModel : public QListModelInterface
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    QDeclarativeListModel(QObject *parent=0);
    ~QDeclarativeListModel();

    virtual QList<int> roles() const;
    virtual QString toString(int role) const;
    virtual int count() const;
    virtual QVariant data(int index, int role) const;

    Q_INVOKABLE void clear();
    Q_INVOKABLE void remove(int index);
    Q_INVOKABLE void append(const QScriptValue&);
    Q_INVOKABLE void insert(int index, const QScriptValue&);
    Q_INVOKABLE QScriptValue get(int index) const;
    Q_INVOKABLE void set(int index, const QScriptValue&);
    Q_INVOKABLE void setProperty(int index, const QString& property, const QVariant& value);
    Q_INVOKABLE void move(int from, int to, int count);
    Q_INVOKABLE void sync();

    QDeclarativeListModelWorkerAgent *agent();

Q_SIGNALS:
    void countChanged();

private:
    friend class QDeclarativeListModelParser;
    friend class QDeclarativeListModelWorkerAgent;
    friend class FlatListModel;
    friend class FlatListScriptClass;
    friend struct ModelNode;

    // Constructs a flat list model for a worker agent
    QDeclarativeListModel(const QDeclarativeListModel *orig, QDeclarativeListModelWorkerAgent *parent);

    void set(int index, const QScriptValue&, QList<int> *roles);
    void setProperty(int index, const QString& property, const QVariant& value, QList<int> *roles);

    bool flatten();
    bool inWorkerThread() const;

    inline bool canMove(int from, int to, int n) const { return !(from+n > count() || to+n > count() || from < 0 || to < 0 || n < 0); }

    QDeclarativeListModelWorkerAgent *m_agent;
    NestedListModel *m_nested;
    FlatListModel *m_flat;
};

// ### FIXME
class QDeclarativeListElement : public QObject
{
Q_OBJECT
};

class QDeclarativeListModelParser : public QDeclarativeCustomParser
{
public:
    QByteArray compile(const QList<QDeclarativeCustomParserProperty> &);
    void setCustomData(QObject *, const QByteArray &);

private:
    struct ListInstruction
    {
        enum { Push, Pop, Value, Set } type;
        int dataIdx;
    };
    struct ListModelData
    {
        int dataOffset;
        int instrCount;
        ListInstruction *instructions() const;
    };
    bool compileProperty(const QDeclarativeCustomParserProperty &prop, QList<ListInstruction> &instr, QByteArray &data);

    bool definesEmptyList(const QString &);

    QByteArray listElementTypeName;
};


QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeListModel)
QML_DECLARE_TYPE(QDeclarativeListElement)

QT_END_HEADER

#endif // QDECLARATIVELISTMODEL_H
