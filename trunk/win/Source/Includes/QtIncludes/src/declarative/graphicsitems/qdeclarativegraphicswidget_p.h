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

#ifndef QDECLARATIVEGRAPHICSWIDGET_P_H
#define QDECLARATIVEGRAPHICSWIDGET_P_H

#include <QObject>
#include <QtDeclarative/qdeclarativecomponent.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Declarative)

class QDeclarativeAnchorLine;
class QDeclarativeAnchors;
class QGraphicsObject;
class QDeclarativeGraphicsWidgetPrivate;

// ### TODO can the extension object be the anchor directly? We save one allocation -> awesome.
class QDeclarativeGraphicsWidget : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QDeclarativeAnchors * anchors READ anchors DESIGNABLE false CONSTANT FINAL)
    Q_PROPERTY(QDeclarativeAnchorLine left READ left CONSTANT FINAL)
    Q_PROPERTY(QDeclarativeAnchorLine right READ right CONSTANT FINAL)
    Q_PROPERTY(QDeclarativeAnchorLine horizontalCenter READ horizontalCenter CONSTANT FINAL)
    Q_PROPERTY(QDeclarativeAnchorLine top READ top CONSTANT FINAL)
    Q_PROPERTY(QDeclarativeAnchorLine bottom READ bottom CONSTANT FINAL)
    Q_PROPERTY(QDeclarativeAnchorLine verticalCenter READ verticalCenter CONSTANT FINAL)
    // ### TODO : QGraphicsWidget don't have a baseline concept yet.
    //Q_PROPERTY(QDeclarativeAnchorLine baseline READ baseline CONSTANT FINAL)
public:
    QDeclarativeGraphicsWidget(QObject *parent = 0);
    ~QDeclarativeGraphicsWidget();
    QDeclarativeAnchors *anchors();
    QDeclarativeAnchorLine left() const;
    QDeclarativeAnchorLine right() const;
    QDeclarativeAnchorLine horizontalCenter() const;
    QDeclarativeAnchorLine top() const;
    QDeclarativeAnchorLine bottom() const;
    QDeclarativeAnchorLine verticalCenter() const;
    Q_DISABLE_COPY(QDeclarativeGraphicsWidget)
    Q_DECLARE_PRIVATE(QDeclarativeGraphicsWidget)
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // QDECLARATIVEGRAPHICSWIDGET_P_H
