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

#ifndef QDECLARATIVELISTMODELWORKERAGENT_P_H
#define QDECLARATIVELISTMODELWORKERAGENT_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qdeclarative.h"

#include <QtScript/qscriptvalue.h>
#include <QtGui/qevent.h>
#include <QMutex>
#include <QWaitCondition>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Declarative)

class QDeclarativeListModel;
class FlatListScriptClass;

class QDeclarativeListModelWorkerAgent : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int count READ count)

public:
    QDeclarativeListModelWorkerAgent(QDeclarativeListModel *);
    ~QDeclarativeListModelWorkerAgent();

    void setScriptEngine(QScriptEngine *eng);
    QScriptEngine *scriptEngine() const;

    void addref();
    void release();

    int count() const;

    Q_INVOKABLE void clear();
    Q_INVOKABLE void remove(int index);
    Q_INVOKABLE void append(const QScriptValue &);
    Q_INVOKABLE void insert(int index, const QScriptValue&);
    Q_INVOKABLE QScriptValue get(int index) const;
    Q_INVOKABLE void set(int index, const QScriptValue &);
    Q_INVOKABLE void setProperty(int index, const QString& property, const QVariant& value);
    Q_INVOKABLE void move(int from, int to, int count);
    Q_INVOKABLE void sync();

    struct VariantRef
    {
        VariantRef() : a(0) {}
        VariantRef(const VariantRef &r) : a(r.a) { if (a) a->addref(); }
        VariantRef(QDeclarativeListModelWorkerAgent *_a) : a(_a) { if (a) a->addref(); }
        ~VariantRef() { if (a) a->release(); }

        VariantRef &operator=(const VariantRef &o) { 
            if (o.a) o.a->addref(); 
            if (a) a->release(); a = o.a; 
            return *this; 
        }

        QDeclarativeListModelWorkerAgent *a;
    };
protected:
    virtual bool event(QEvent *);

private:
    friend class QDeclarativeWorkerScriptEnginePrivate;
    friend class FlatListScriptClass;
    QScriptEngine *m_engine;

    struct Change {
        enum { Inserted, Removed, Moved, Changed } type;
        int index; // Inserted/Removed/Moved/Changed
        int count; // Inserted/Removed/Moved/Changed
        int to;    // Moved
        QList<int> roles;
    };

    struct Data {
        QList<Change> changes;

        void clearChange();
        void insertChange(int index, int count);
        void removeChange(int index, int count);
        void moveChange(int index, int count, int to);
        void changedChange(int index, int count, const QList<int> &roles);
    };
    Data data;

    struct Sync : public QEvent {
        Sync() : QEvent(QEvent::User) {}
        Data data;
        QDeclarativeListModel *list;
    };

    void changedData(int index, int count, const QList<int> &roles);

    QAtomicInt m_ref;
    QDeclarativeListModel *m_orig;
    QDeclarativeListModel *m_copy;
    QMutex mutex;
    QWaitCondition syncDone;
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QDeclarativeListModelWorkerAgent::VariantRef)

QT_END_HEADER

#endif // QDECLARATIVEWORKERSCRIPT_P_H

