/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtSvg module of the Qt Toolkit.
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

#ifndef QSVGGRAPHICS_P_H
#define QSVGGRAPHICS_P_H

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

#include "qsvgnode_p.h"

#ifndef QT_NO_SVG

#include "QtGui/qpainterpath.h"
#include "QtGui/qimage.h"
#include "QtGui/qtextlayout.h"
#include "QtGui/qtextoption.h"
#include "QtCore/qstack.h"

QT_BEGIN_NAMESPACE

class QTextCharFormat;

class QSvgAnimation : public QSvgNode
{
public:
    virtual void draw(QPainter *p, QSvgExtraStates &states);
    virtual Type type() const;
};

class QSvgArc : public QSvgNode
{
public:
    QSvgArc(QSvgNode *parent, const QPainterPath &path);
    virtual void draw(QPainter *p, QSvgExtraStates &states);
    virtual Type type() const;
    virtual QRectF bounds(QPainter *p, QSvgExtraStates &states) const;
private:
    QPainterPath m_path;
};

class QSvgEllipse : public QSvgNode
{
public:
    QSvgEllipse(QSvgNode *parent, const QRectF &rect);
    virtual void draw(QPainter *p, QSvgExtraStates &states);
    virtual Type type() const;
    virtual QRectF bounds(QPainter *p, QSvgExtraStates &states) const;
private:
    QRectF m_bounds;
};

class QSvgCircle : public QSvgEllipse
{
public:
    QSvgCircle(QSvgNode *parent, const QRectF &rect) : QSvgEllipse(parent, rect) { }
    virtual Type type() const;
};

class QSvgImage : public QSvgNode
{
public:
    QSvgImage(QSvgNode *parent, const QImage &image,
              const QRect &bounds);
    virtual void draw(QPainter *p, QSvgExtraStates &states);
    virtual Type type() const;
    virtual QRectF bounds(QPainter *p, QSvgExtraStates &states) const;
private:
    QImage m_image;
    QRect  m_bounds;
};

class QSvgLine : public QSvgNode
{
public:
    QSvgLine(QSvgNode *parent, const QLineF &line);
    virtual void draw(QPainter *p, QSvgExtraStates &states);
    virtual Type type() const;
    virtual QRectF bounds(QPainter *p, QSvgExtraStates &states) const;
private:
    QLineF m_line;
};

class QSvgPath : public QSvgNode
{
public:
    QSvgPath(QSvgNode *parent, const QPainterPath &qpath);
    virtual void draw(QPainter *p, QSvgExtraStates &states);
    virtual Type type() const;
    virtual QRectF bounds(QPainter *p, QSvgExtraStates &states) const;

    QPainterPath *qpath() {
        return &m_path;
    }
private:
    QPainterPath m_path;
};

class QSvgPolygon : public QSvgNode
{
public:
    QSvgPolygon(QSvgNode *parent, const QPolygonF &poly);
    virtual void draw(QPainter *p, QSvgExtraStates &states);
    virtual Type type() const;
    virtual QRectF bounds(QPainter *p, QSvgExtraStates &states) const;
private:
    QPolygonF m_poly;
};

class QSvgPolyline : public QSvgNode
{
public:
    QSvgPolyline(QSvgNode *parent, const QPolygonF &poly);
    virtual void draw(QPainter *p, QSvgExtraStates &states);
    virtual Type type() const;
    virtual QRectF bounds(QPainter *p, QSvgExtraStates &states) const;
private:
    QPolygonF m_poly;
};

class QSvgRect : public QSvgNode
{
public:
    QSvgRect(QSvgNode *paren, const QRectF &rect, int rx=0, int ry=0);
    virtual Type type() const;
    virtual void draw(QPainter *p, QSvgExtraStates &states);
    virtual QRectF bounds(QPainter *p, QSvgExtraStates &states) const;
private:
    QRectF m_rect;
    int m_rx, m_ry;
};

class  QSvgTspan;

class  QSvgText : public QSvgNode
{
public:
    enum WhitespaceMode
    {
        Default,
        Preserve
    };

    QSvgText(QSvgNode *parent, const QPointF &coord);
    ~QSvgText();
    void setTextArea(const QSizeF &size);

    virtual void draw(QPainter *p, QSvgExtraStates &states);
    virtual Type type() const;

    void addTspan(QSvgTspan *tspan) {m_tspans.append(tspan);}
    void addText(const QString &text);
    void addLineBreak() {m_tspans.append(LINEBREAK);}
    void setWhitespaceMode(WhitespaceMode mode) {m_mode = mode;}

    //virtual QRectF bounds(QPainter *p, QSvgExtraStates &states) const;
private:
    static QSvgTspan * const LINEBREAK;

    QPointF m_coord;

    // 'm_tspans' is also used to store characters outside tspans and line breaks.
    // If a 'm_tspan' item is null, it indicates a line break.
    QVector<QSvgTspan *> m_tspans;

    Type m_type;
    QSizeF m_size;
    WhitespaceMode m_mode;
};

class  QSvgTspan : public QSvgNode
{
public:
    // tspans are also used to store normal text, so the 'isProperTspan' is used to separate text from tspan.
    QSvgTspan(QSvgNode *parent, bool isProperTspan = true)
        : QSvgNode(parent), m_mode(QSvgText::Default), m_isTspan(isProperTspan)
    {
    }
    ~QSvgTspan() { };
    virtual Type type() const {return TSPAN;}
    virtual void draw(QPainter *, QSvgExtraStates &) {Q_ASSERT(!"Tspans should be drawn through QSvgText::draw().");}
    void addText(const QString &text) {m_text += text;}
    const QString &text() const {return m_text;}
    bool isTspan() const {return m_isTspan;}
    void setWhitespaceMode(QSvgText::WhitespaceMode mode) {m_mode = mode;}
    QSvgText::WhitespaceMode whitespaceMode() const {return m_mode;}
private:
    QString m_text;
    QSvgText::WhitespaceMode m_mode;
    bool m_isTspan;
};

class QSvgUse : public QSvgNode
{
public:
    QSvgUse(const QPointF &start, QSvgNode *parent, QSvgNode *link);
    virtual void draw(QPainter *p, QSvgExtraStates &states);
    virtual Type type() const;
    virtual QRectF bounds(QPainter *p, QSvgExtraStates &states) const;

private:
    QSvgNode *m_link;
    QPointF   m_start;
};

class QSvgVideo : public QSvgNode
{
public:
    virtual void draw(QPainter *p, QSvgExtraStates &states);
    virtual Type type() const;
};

QT_END_NAMESPACE

#endif // QT_NO_SVG
#endif // QSVGGRAPHICS_P_H
