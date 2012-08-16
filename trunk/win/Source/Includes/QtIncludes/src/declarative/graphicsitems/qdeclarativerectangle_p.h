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

#ifndef QDECLARATIVERECT_H
#define QDECLARATIVERECT_H

#include "qdeclarativeitem.h"

#include <QtGui/qbrush.h>

#include <private/qdeclarativeglobal_p.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Declarative)
class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativePen : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int width READ width WRITE setWidth NOTIFY penChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY penChanged)
public:
    QDeclarativePen(QObject *parent=0)
        : QObject(parent), _width(1), _color("#000000"), _valid(false)
    {}

    int width() const { return _width; }
    void setWidth(int w);

    QColor color() const { return _color; }
    void setColor(const QColor &c);

    bool isValid() { return _valid; }

Q_SIGNALS:
    void penChanged();

private:
    int _width;
    QColor _color;
    bool _valid;
};

class Q_AUTOTEST_EXPORT QDeclarativeGradientStop : public QObject
{
    Q_OBJECT

    Q_PROPERTY(qreal position READ position WRITE setPosition)
    Q_PROPERTY(QColor color READ color WRITE setColor)

public:
    QDeclarativeGradientStop(QObject *parent=0) : QObject(parent) {}

    qreal position() const { return m_position; }
    void setPosition(qreal position) { m_position = position; updateGradient(); }

    QColor color() const { return m_color; }
    void setColor(const QColor &color) { m_color = color; updateGradient(); }

private:
    void updateGradient();

private:
    qreal m_position;
    QColor m_color;
};

class Q_AUTOTEST_EXPORT QDeclarativeGradient : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QDeclarativeListProperty<QDeclarativeGradientStop> stops READ stops)
    Q_CLASSINFO("DefaultProperty", "stops")

public:
    QDeclarativeGradient(QObject *parent=0) : QObject(parent), m_gradient(0) {}
    ~QDeclarativeGradient() { delete m_gradient; }

    QDeclarativeListProperty<QDeclarativeGradientStop> stops() { return QDeclarativeListProperty<QDeclarativeGradientStop>(this, m_stops); }

    const QGradient *gradient() const;

Q_SIGNALS:
    void updated();

private:
    void doUpdate();

private:
    QList<QDeclarativeGradientStop *> m_stops;
    mutable QGradient *m_gradient;
    friend class QDeclarativeGradientStop;
};

class QDeclarativeRectanglePrivate;
class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativeRectangle : public QDeclarativeItem
{
    Q_OBJECT

    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(QDeclarativeGradient *gradient READ gradient WRITE setGradient)
    Q_PROPERTY(QDeclarativePen * border READ border CONSTANT)
    Q_PROPERTY(qreal radius READ radius WRITE setRadius NOTIFY radiusChanged)
public:
    QDeclarativeRectangle(QDeclarativeItem *parent=0);

    QColor color() const;
    void setColor(const QColor &);

    QDeclarativePen *border();

    QDeclarativeGradient *gradient() const;
    void setGradient(QDeclarativeGradient *gradient);

    qreal radius() const;
    void setRadius(qreal radius);

    QRectF boundingRect() const;

    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *);

Q_SIGNALS:
    void colorChanged();
    void radiusChanged();

private Q_SLOTS:
    void doUpdate();

private:
    void generateRoundedRect();
    void generateBorderedRect();
    void drawRect(QPainter &painter);

private:
    Q_DISABLE_COPY(QDeclarativeRectangle)
    Q_DECLARE_PRIVATE_D(QGraphicsItem::d_ptr.data(), QDeclarativeRectangle)
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativePen)
QML_DECLARE_TYPE(QDeclarativeGradientStop)
QML_DECLARE_TYPE(QDeclarativeGradient)
QML_DECLARE_TYPE(QDeclarativeRectangle)

QT_END_HEADER

#endif // QDECLARATIVERECT_H
