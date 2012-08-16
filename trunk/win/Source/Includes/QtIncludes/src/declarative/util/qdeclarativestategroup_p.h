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

#ifndef QDECLARATIVESTATEGROUP_H
#define QDECLARATIVESTATEGROUP_H

#include "private/qdeclarativestate_p.h"

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Declarative)

class QDeclarativeStateGroupPrivate;
class Q_DECLARATIVE_EXPORT QDeclarativeStateGroup : public QObject, public QDeclarativeParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QDeclarativeParserStatus)
    Q_DECLARE_PRIVATE(QDeclarativeStateGroup)

    Q_PROPERTY(QString state READ state WRITE setState NOTIFY stateChanged)
    Q_PROPERTY(QDeclarativeListProperty<QDeclarativeState> states READ statesProperty DESIGNABLE false)
    Q_PROPERTY(QDeclarativeListProperty<QDeclarativeTransition> transitions READ transitionsProperty DESIGNABLE false)

public:
    QDeclarativeStateGroup(QObject * = 0);
    virtual ~QDeclarativeStateGroup();

    QString state() const;
    void setState(const QString &);

    QDeclarativeListProperty<QDeclarativeState> statesProperty();
    QList<QDeclarativeState *> states() const;

    QDeclarativeListProperty<QDeclarativeTransition> transitionsProperty();

    QDeclarativeState *findState(const QString &name) const;

    virtual void classBegin();
    virtual void componentComplete();
Q_SIGNALS:
    void stateChanged(const QString &);

private:
    friend class QDeclarativeState;
    bool updateAutoState();
    void removeState(QDeclarativeState *state);
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeStateGroup)

QT_END_HEADER

#endif // QDECLARATIVESTATEGROUP_H
