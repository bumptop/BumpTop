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

#ifndef QDECLARATIVEOBJECTSCRIPTCLASS_P_H
#define QDECLARATIVEOBJECTSCRIPTCLASS_P_H

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

#include "private/qdeclarativepropertycache_p.h"
#include "private/qdeclarativetypenamecache_p.h"

#include <private/qscriptdeclarativeclass_p.h>
#include <QtScript/qscriptengine.h>

QT_BEGIN_NAMESPACE

class QDeclarativeEngine;
class QScriptContext;
class QScriptEngine;
class QDeclarativeContextData;
class MethodData;

class Q_AUTOTEST_EXPORT QDeclarativeObjectMethodScriptClass : public QScriptDeclarativeClass
{
public:
    QDeclarativeObjectMethodScriptClass(QDeclarativeEngine *);
    ~QDeclarativeObjectMethodScriptClass();

    QScriptValue newMethod(QObject *, const QDeclarativePropertyCache::Data *);

protected:
    virtual Value call(Object *, QScriptContext *);
    virtual QScriptClass::QueryFlags queryProperty(Object *, const Identifier &, QScriptClass::QueryFlags flags);
    virtual Value property(Object *, const Identifier &);

private:
    int enumType(const QMetaObject *, const QString &);

    Value callPrecise(QObject *, const QDeclarativePropertyCache::Data &, QScriptContext *);
    Value callOverloaded(MethodData *, QScriptContext *);
    Value callMethod(QObject *, int index, int returnType, int argCount, int *argTypes, QScriptContext *ctxt);

    int matchScore(const QScriptValue &, int, const QByteArray &);
    QDeclarativePropertyCache::Data *relatedMethod(QObject *, QDeclarativePropertyCache::Data *current, 
                                                   QDeclarativePropertyCache::Data &dummy);

    PersistentIdentifier m_connectId;
    PersistentIdentifier m_disconnectId;
    QScriptValue m_connect;
    QScriptValue m_disconnect;

    static QScriptValue connect(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue disconnect(QScriptContext *context, QScriptEngine *engine);

    QDeclarativeEngine *engine;
};

class Q_AUTOTEST_EXPORT QDeclarativeObjectScriptClass : public QScriptDeclarativeClass
{
public:
    QDeclarativeObjectScriptClass(QDeclarativeEngine *);
    ~QDeclarativeObjectScriptClass();

    QScriptValue newQObject(QObject *, int type = QMetaType::QObjectStar);

    QObject *toQObject(const QScriptValue &) const;
    int objectType(const QScriptValue &) const;

    enum QueryHint {
        ImplicitObject = 0x01,
        SkipAttachedProperties = 0x02
    };
    Q_DECLARE_FLAGS(QueryHints, QueryHint)

    QScriptClass::QueryFlags queryProperty(QObject *, const Identifier &, 
                                           QScriptClass::QueryFlags flags, 
                                           QDeclarativeContextData *evalContext,
                                           QueryHints hints = 0);

    Value property(QObject *, const Identifier &);

    void setProperty(QObject *, const Identifier &name, const QScriptValue &,
                     QScriptContext *context, QDeclarativeContextData *evalContext = 0);
    virtual QStringList propertyNames(Object *);
    virtual bool compare(Object *, Object *);

protected:
    virtual QScriptClass::QueryFlags queryProperty(Object *, const Identifier &, 
                                                   QScriptClass::QueryFlags flags);

    virtual Value property(Object *, const Identifier &);
    virtual void setProperty(Object *, const Identifier &name, const QScriptValue &);
    virtual bool isQObject() const;
    virtual QObject *toQObject(Object *, bool *ok = 0);

private:
    friend class QDeclarativeObjectMethodScriptClass;
    QDeclarativeObjectMethodScriptClass methods;

    QDeclarativeTypeNameCache::Data *lastTNData;
    QDeclarativePropertyCache::Data *lastData;
    QDeclarativePropertyCache::Data local;

    PersistentIdentifier m_destroyId;
    PersistentIdentifier m_toStringId;
    QScriptValue m_destroy;
    QScriptValue m_toString;

    static QScriptValue tostring(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue destroy(QScriptContext *context, QScriptEngine *engine);

    QDeclarativeEngine *engine;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(QDeclarativeObjectScriptClass::QueryHints);

QT_END_NAMESPACE

#endif // QDECLARATIVEOBJECTSCRIPTCLASS_P_H

