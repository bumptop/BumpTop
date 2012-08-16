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

#ifndef QDECLARATIVEENGINEDEBUG_P_H
#define QDECLARATIVEENGINEDEBUG_P_H

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

#include <private/qdeclarativedebugservice_p.h>

#include <QtCore/qurl.h>
#include <QtCore/qvariant.h>
#include <QWeakPointer>

QT_BEGIN_NAMESPACE

class QDeclarativeEngine;
class QDeclarativeContext;
class QDeclarativeWatcher;
class QDataStream;
class QDeclarativeState;

class QDeclarativeEngineDebugServer : public QDeclarativeDebugService
{
    Q_OBJECT
public:
    QDeclarativeEngineDebugServer(QObject * = 0);

    struct QDeclarativeObjectData {
        QUrl url;
        int lineNumber;
        int columnNumber;
        QString idString;
        QString objectName;
        QString objectType;
        int objectId;
        int contextId;
    };

    struct QDeclarativeObjectProperty {
        enum Type { Unknown, Basic, Object, List, SignalProperty };
        Type type;
        QString name;
        QVariant value;
        QString valueTypeName;
        QString binding;
        bool hasNotifySignal;
    };

    void addEngine(QDeclarativeEngine *);
    void remEngine(QDeclarativeEngine *);
    void objectCreated(QDeclarativeEngine *, QObject *);

    static QDeclarativeEngineDebugServer *instance();

protected:
    virtual void messageReceived(const QByteArray &);

private Q_SLOTS:
    void propertyChanged(int id, int objectId, const QMetaProperty &property, const QVariant &value);

private:
    void prepareDeferredObjects(QObject *);
    void buildObjectList(QDataStream &, QDeclarativeContext *);
    void buildObjectDump(QDataStream &, QObject *, bool, bool);
    void buildStatesList(QDeclarativeContext *, bool);
    void buildStatesList(QObject *obj);
    QDeclarativeObjectData objectData(QObject *);
    QDeclarativeObjectProperty propertyData(QObject *, int);
    QVariant valueContents(const QVariant &defaultValue) const;
    void setBinding(int objectId, const QString &propertyName, const QVariant &expression, bool isLiteralValue);
    void resetBinding(int objectId, const QString &propertyName);
    void setMethodBody(int objectId, const QString &method, const QString &body);

    QList<QDeclarativeEngine *> m_engines;
    QDeclarativeWatcher *m_watch;
    QList<QWeakPointer<QDeclarativeState> > m_allStates;
};
Q_DECLARATIVE_PRIVATE_EXPORT QDataStream &operator<<(QDataStream &, const QDeclarativeEngineDebugServer::QDeclarativeObjectData &);
Q_DECLARATIVE_PRIVATE_EXPORT QDataStream &operator>>(QDataStream &, QDeclarativeEngineDebugServer::QDeclarativeObjectData &);
Q_DECLARATIVE_PRIVATE_EXPORT QDataStream &operator<<(QDataStream &, const QDeclarativeEngineDebugServer::QDeclarativeObjectProperty &);
Q_DECLARATIVE_PRIVATE_EXPORT QDataStream &operator>>(QDataStream &, QDeclarativeEngineDebugServer::QDeclarativeObjectProperty &);

QT_END_NAMESPACE

#endif // QDECLARATIVEENGINEDEBUG_P_H

