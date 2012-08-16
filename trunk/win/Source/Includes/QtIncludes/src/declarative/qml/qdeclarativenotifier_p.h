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

#ifndef QDECLARATIVENOTIFIER_P_H
#define QDECLARATIVENOTIFIER_P_H

#include "private/qdeclarativeguard_p.h"

QT_BEGIN_NAMESPACE

class QDeclarativeNotifierEndpoint;
class QDeclarativeNotifier
{
public:
    inline QDeclarativeNotifier();
    inline ~QDeclarativeNotifier();
    inline void notify();

private:
    friend class QDeclarativeNotifierEndpoint;

    static void emitNotify(QDeclarativeNotifierEndpoint *);
    QDeclarativeNotifierEndpoint *endpoints;
};

class QDeclarativeNotifierEndpoint
{
public:
    inline QDeclarativeNotifierEndpoint();
    inline QDeclarativeNotifierEndpoint(QObject *t, int m);
    inline ~QDeclarativeNotifierEndpoint();

    QObject *target;
    int targetMethod;

    inline bool isConnected();
    inline bool isConnected(QObject *source, int sourceSignal);
    inline bool isConnected(QDeclarativeNotifier *);

    void connect(QObject *source, int sourceSignal);
    inline void connect(QDeclarativeNotifier *);
    inline void disconnect();

    void copyAndClear(QDeclarativeNotifierEndpoint &other);

private:
    friend class QDeclarativeNotifier;

    struct Signal {
        QDeclarativeGuard<QObject> source;
        int sourceSignal;
    };

    struct Notifier {
        QDeclarativeNotifier *notifier;
        QDeclarativeNotifierEndpoint **disconnected;

        QDeclarativeNotifierEndpoint  *next;
        QDeclarativeNotifierEndpoint **prev;
    };

    enum { InvalidType, SignalType, NotifierType } type;
    union {
        char signalData[sizeof(Signal)];
        char notifierData[sizeof(Notifier)];
    };

    inline Notifier *toNotifier();
    inline Notifier *asNotifier();
    inline Signal *toSignal();
    inline Signal *asSignal();
};

QDeclarativeNotifier::QDeclarativeNotifier()
: endpoints(0)
{
}

QDeclarativeNotifier::~QDeclarativeNotifier()
{    
    QDeclarativeNotifierEndpoint *endpoint = endpoints;
    while (endpoint) {
        QDeclarativeNotifierEndpoint::Notifier *n = endpoint->asNotifier();
        endpoint = n->next;

        n->next = 0;
        n->prev = 0;
        n->notifier = 0;
        if (n->disconnected) *n->disconnected = 0;
        n->disconnected = 0;
    }
    endpoints = 0;
}

void QDeclarativeNotifier::notify()
{
    if (endpoints) emitNotify(endpoints);
}

QDeclarativeNotifierEndpoint::QDeclarativeNotifierEndpoint()
: target(0), targetMethod(0), type(InvalidType) 
{
}

QDeclarativeNotifierEndpoint::QDeclarativeNotifierEndpoint(QObject *t, int m)
: target(t), targetMethod(m), type(InvalidType) 
{
}

QDeclarativeNotifierEndpoint::~QDeclarativeNotifierEndpoint()
{
    disconnect();
    if (SignalType == type) {
        Signal *s = asSignal();
        s->~Signal();
    }
}

bool QDeclarativeNotifierEndpoint::isConnected()
{
    if (SignalType == type) {
        return asSignal()->source;
    } else if (NotifierType == type) {
        return asNotifier()->notifier;
    } else {
        return false;
    }
}

bool QDeclarativeNotifierEndpoint::isConnected(QObject *source, int sourceSignal)
{
    return SignalType == type && asSignal()->source == source && asSignal()->sourceSignal == sourceSignal;
}

bool QDeclarativeNotifierEndpoint::isConnected(QDeclarativeNotifier *notifier)
{
    return NotifierType == type && asNotifier()->notifier == notifier;
}

void QDeclarativeNotifierEndpoint::connect(QDeclarativeNotifier *notifier)
{
    Notifier *n = toNotifier();
    
    if (n->notifier == notifier)
        return;

    disconnect();

    n->next = notifier->endpoints;
    if (n->next) { n->next->asNotifier()->prev = &n->next; }
    notifier->endpoints = this;
    n->prev = &notifier->endpoints;
    n->notifier = notifier;
}

void QDeclarativeNotifierEndpoint::disconnect()
{
    if (type == SignalType) {
        Signal *s = (Signal *)&signalData;
        if (s->source) {
            QMetaObject::disconnectOne(s->source, s->sourceSignal, target, targetMethod);
            s->source = 0;
        }
    } else if (type == NotifierType) {
        Notifier *n = asNotifier();

        if (n->next) n->next->asNotifier()->prev = n->prev;
        if (n->prev) *n->prev = n->next;
        if (n->disconnected) *n->disconnected = 0;
        n->next = 0;
        n->prev = 0;
        n->disconnected = 0;
        n->notifier = 0;
    }
}

QDeclarativeNotifierEndpoint::Notifier *QDeclarativeNotifierEndpoint::toNotifier()
{
    if (NotifierType == type) 
        return asNotifier();

    if (SignalType == type) {
        disconnect();
        Signal *s = asSignal();
        s->~Signal();
    }

    Notifier *n = asNotifier();
    n->next = 0;
    n->prev = 0;
    n->disconnected = 0;
    n->notifier = 0;
    type = NotifierType;
    return n;
}

QDeclarativeNotifierEndpoint::Notifier *QDeclarativeNotifierEndpoint::asNotifier() 
{ 
    return (Notifier *)(&notifierData); 
}

QDeclarativeNotifierEndpoint::Signal *QDeclarativeNotifierEndpoint::toSignal()
{
    if (SignalType == type) 
        return asSignal();

    disconnect();
    Signal *s = asSignal();
    new (s) Signal;
    type = SignalType;

    return s;
}

QDeclarativeNotifierEndpoint::Signal *QDeclarativeNotifierEndpoint::asSignal() 
{ 
    return (Signal *)(&signalData); 
}

QT_END_NAMESPACE

#endif // QDECLARATIVENOTIFIER_P_H

