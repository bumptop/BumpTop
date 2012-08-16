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

#ifndef QDECLARATIVEGRAPHICSLAYOUTITEM_H
#define QDECLARATIVEGRAPHICSLAYOUTITEM_H
#include "qdeclarativeitem.h"

#include <QGraphicsLayoutItem>
#include <QSizeF>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Declarative)

class QDeclarativeLayoutItem : public QDeclarativeItem, public QGraphicsLayoutItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsLayoutItem)
    Q_PROPERTY(QSizeF maximumSize READ maximumSize WRITE setMaximumSize NOTIFY maximumSizeChanged)
    Q_PROPERTY(QSizeF minimumSize READ minimumSize WRITE setMinimumSize NOTIFY minimumSizeChanged)
    Q_PROPERTY(QSizeF preferredSize READ preferredSize WRITE setPreferredSize NOTIFY preferredSizeChanged)
public:
    QDeclarativeLayoutItem(QDeclarativeItem* parent=0);

    QSizeF maximumSize() const { return m_maximumSize; }
    void setMaximumSize(const QSizeF &s) { if(s==m_maximumSize) return; m_maximumSize = s; emit maximumSizeChanged(); }

    QSizeF minimumSize() const { return m_minimumSize; }
    void setMinimumSize(const QSizeF &s) { if(s==m_minimumSize) return; m_minimumSize = s; emit minimumSizeChanged(); }

    QSizeF preferredSize() const { return m_preferredSize; }
    void setPreferredSize(const QSizeF &s) { if(s==m_preferredSize) return; m_preferredSize = s; emit preferredSizeChanged(); }

    virtual void setGeometry(const QRectF & rect);
protected:
    virtual QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;

Q_SIGNALS:
    void maximumSizeChanged();
    void minimumSizeChanged();
    void preferredSizeChanged();

private:
    QSizeF m_maximumSize;
    QSizeF m_minimumSize;
    QSizeF m_preferredSize;
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeLayoutItem)

QT_END_HEADER
#endif
