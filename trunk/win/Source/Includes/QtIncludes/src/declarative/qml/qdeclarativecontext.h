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

#ifndef QDECLARATIVECONTEXT_H
#define QDECLARATIVECONTEXT_H

#include <QtCore/qurl.h>
#include <QtCore/qobject.h>
#include <QtScript/qscriptvalue.h>
#include <QtCore/qmetatype.h>
#include <QtCore/qvariant.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Declarative)

class QString;
class QDeclarativeEngine;
class QDeclarativeRefCount;
class QDeclarativeContextPrivate;
class QDeclarativeCompositeTypeData;
class QDeclarativeContextData;

class Q_DECLARATIVE_EXPORT QDeclarativeContext : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QDeclarativeContext)

public:
    QDeclarativeContext(QDeclarativeEngine *parent, QObject *objParent=0);
    QDeclarativeContext(QDeclarativeContext *parent, QObject *objParent=0);
    virtual ~QDeclarativeContext();

    bool isValid() const;

    QDeclarativeEngine *engine() const;
    QDeclarativeContext *parentContext() const;

    QObject *contextObject() const;
    void setContextObject(QObject *);

    QVariant contextProperty(const QString &) const;
    void setContextProperty(const QString &, QObject *);
    void setContextProperty(const QString &, const QVariant &);

    QUrl resolvedUrl(const QUrl &);

    void setBaseUrl(const QUrl &);
    QUrl baseUrl() const;

private:
    friend class QDeclarativeVME;
    friend class QDeclarativeEngine;
    friend class QDeclarativeEnginePrivate;
    friend class QDeclarativeExpression;
    friend class QDeclarativeExpressionPrivate;
    friend class QDeclarativeContextScriptClass;
    friend class QDeclarativeObjectScriptClass;
    friend class QDeclarativeComponent;
    friend class QDeclarativeComponentPrivate;
    friend class QDeclarativeScriptPrivate;
    friend class QDeclarativeBoundSignalProxy;
    friend class QDeclarativeContextData;
    QDeclarativeContext(QDeclarativeContextData *);
    QDeclarativeContext(QDeclarativeEngine *, bool);
    Q_DISABLE_COPY(QDeclarativeContext)
};
QT_END_NAMESPACE

Q_DECLARE_METATYPE(QList<QObject*>)

QT_END_HEADER

#endif // QDECLARATIVECONTEXT_H
