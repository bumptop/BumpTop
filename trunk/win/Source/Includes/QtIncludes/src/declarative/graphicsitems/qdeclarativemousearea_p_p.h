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

#ifndef QDECLARATIVEMOUSEREGION_P_H
#define QDECLARATIVEMOUSEREGION_P_H

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

#include "private/qdeclarativeitem_p.h"

#include <qdatetime.h>
#include <qbasictimer.h>
#include <qgraphicssceneevent.h>

QT_BEGIN_NAMESPACE

class QDeclarativeMouseAreaPrivate : public QDeclarativeItemPrivate
{
    Q_DECLARE_PUBLIC(QDeclarativeMouseArea)

public:
    QDeclarativeMouseAreaPrivate()
      : absorb(true), hovered(false), pressed(false), longPress(false),
      moved(false), stealMouse(false), doubleClick(false), preventStealing(false), drag(0)
    {
    }

    ~QDeclarativeMouseAreaPrivate();

    void init()
    {
        Q_Q(QDeclarativeMouseArea);
        q->setAcceptedMouseButtons(Qt::LeftButton);
        q->setFiltersChildEvents(true);
    }

    void saveEvent(QGraphicsSceneMouseEvent *event) {
        lastPos = event->pos();
        lastScenePos = event->scenePos();
        lastButton = event->button();
        lastButtons = event->buttons();
        lastModifiers = event->modifiers();
    }

    bool isPressAndHoldConnected() {
        Q_Q(QDeclarativeMouseArea);
        static int idx = QObjectPrivate::get(q)->signalIndex("pressAndHold(QDeclarativeMouseEvent*)");
        return QObjectPrivate::get(q)->isSignalConnected(idx);
    }

    bool isDoubleClickConnected() {
        Q_Q(QDeclarativeMouseArea);
        static int idx = QObjectPrivate::get(q)->signalIndex("doubleClicked(QDeclarativeMouseEvent*)");
        return QObjectPrivate::get(q)->isSignalConnected(idx);
    }

    bool absorb : 1;
    bool hovered : 1;
    bool pressed : 1;
    bool longPress : 1;
    bool moved : 1;
    bool dragX : 1;
    bool dragY : 1;
    bool stealMouse : 1;
    bool doubleClick : 1;
    bool preventStealing : 1;
    QDeclarativeDrag *drag;
    QPointF startScene;
    qreal startX;
    qreal startY;
    QPointF lastPos;
    QDeclarativeNullableValue<QPointF> lastScenePos;
    Qt::MouseButton lastButton;
    Qt::MouseButtons lastButtons;
    Qt::KeyboardModifiers lastModifiers;
    QBasicTimer pressAndHoldTimer;
};

QT_END_NAMESPACE

#endif // QDECLARATIVEMOUSEREGION_P_H
