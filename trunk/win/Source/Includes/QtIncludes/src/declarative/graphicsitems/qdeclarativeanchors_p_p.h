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

#ifndef QDECLARATIVEANCHORS_P_H
#define QDECLARATIVEANCHORS_P_H

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

#include "private/qdeclarativeanchors_p.h"
#include "private/qdeclarativeitemchangelistener_p.h"
#include <private/qobject_p.h>

QT_BEGIN_NAMESPACE

class QDeclarativeAnchorLine
{
public:
    QDeclarativeAnchorLine() : item(0), anchorLine(Invalid) {}

    enum AnchorLine {
        Invalid = 0x0,
        Left = 0x01,
        Right = 0x02,
        Top = 0x04,
        Bottom = 0x08,
        HCenter = 0x10,
        VCenter = 0x20,
        Baseline = 0x40,
        Horizontal_Mask = Left | Right | HCenter,
        Vertical_Mask = Top | Bottom | VCenter | Baseline
    };

    QGraphicsObject *item;
    AnchorLine anchorLine;
};

inline bool operator==(const QDeclarativeAnchorLine& a, const QDeclarativeAnchorLine& b)
{
    return a.item == b.item && a.anchorLine == b.anchorLine;
}

class QDeclarativeAnchorsPrivate : public QObjectPrivate, public QDeclarativeItemChangeListener
{
    Q_DECLARE_PUBLIC(QDeclarativeAnchors)
public:
    QDeclarativeAnchorsPrivate(QGraphicsObject *i)
      : componentComplete(true), updatingMe(false), updatingHorizontalAnchor(0),
        updatingVerticalAnchor(0), updatingFill(0), updatingCenterIn(0), item(i), usedAnchors(0), fill(0),
        centerIn(0), leftMargin(0), rightMargin(0), topMargin(0), bottomMargin(0),
        margins(0), vCenterOffset(0), hCenterOffset(0), baselineOffset(0)
    {
    }

    void clearItem(QGraphicsObject *);

    void addDepend(QGraphicsObject *);
    void remDepend(QGraphicsObject *);
    bool isItemComplete() const;

    bool componentComplete:1;
    bool updatingMe:1;
    uint updatingHorizontalAnchor:2;
    uint updatingVerticalAnchor:2;
    uint updatingFill:2;
    uint updatingCenterIn:2;

    void setItemHeight(qreal);
    void setItemWidth(qreal);
    void setItemX(qreal);
    void setItemY(qreal);
    void setItemPos(const QPointF &);
    void setItemSize(const QSizeF &);

    void updateOnComplete();
    void updateMe();

    // QDeclarativeItemGeometryListener interface
    void itemGeometryChanged(QDeclarativeItem *, const QRectF &, const QRectF &);
    void _q_widgetDestroyed(QObject *);
    void _q_widgetGeometryChanged();
    QDeclarativeAnchorsPrivate *anchorPrivate() { return this; }

    bool checkHValid() const;
    bool checkVValid() const;
    bool checkHAnchorValid(QDeclarativeAnchorLine anchor) const;
    bool checkVAnchorValid(QDeclarativeAnchorLine anchor) const;
    bool calcStretch(const QDeclarativeAnchorLine &edge1, const QDeclarativeAnchorLine &edge2, qreal offset1, qreal offset2, QDeclarativeAnchorLine::AnchorLine line, qreal &stretch);

    bool isMirrored() const;
    void updateHorizontalAnchors();
    void updateVerticalAnchors();
    void fillChanged();
    void centerInChanged();

    QGraphicsObject *item;
    QDeclarativeAnchors::Anchors usedAnchors;

    QGraphicsObject *fill;
    QGraphicsObject *centerIn;

    QDeclarativeAnchorLine left;
    QDeclarativeAnchorLine right;
    QDeclarativeAnchorLine top;
    QDeclarativeAnchorLine bottom;
    QDeclarativeAnchorLine vCenter;
    QDeclarativeAnchorLine hCenter;
    QDeclarativeAnchorLine baseline;

    qreal leftMargin;
    qreal rightMargin;
    qreal topMargin;
    qreal bottomMargin;
    qreal margins;
    qreal vCenterOffset;
    qreal hCenterOffset;
    qreal baselineOffset;
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QDeclarativeAnchorLine)

#endif
