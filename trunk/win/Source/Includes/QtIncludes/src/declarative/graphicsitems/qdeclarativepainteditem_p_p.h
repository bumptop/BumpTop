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

#ifndef QDECLARATIVEIMAGEITEM_P_H
#define QDECLARATIVEIMAGEITEM_P_H

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

QT_BEGIN_NAMESPACE

class QDeclarativePaintedItemPrivate : public QDeclarativeItemPrivate
{
    Q_DECLARE_PUBLIC(QDeclarativePaintedItem)

public:
    QDeclarativePaintedItemPrivate()
      : max_imagecache_size(100000), contentsScale(1.0), fillColor(Qt::transparent), cachefrozen(false), smoothCache(true)
    {
    }

    struct ImageCacheItem {
        ImageCacheItem() : age(0) {}
        ~ImageCacheItem() { }
        int age;
        QRect area;
        QRect dirty; // one dirty area (allows optimization of common cases)
        QPixmap image;
    };

    QList<ImageCacheItem*> imagecache;

    int max_imagecache_size;
    QSize contentsSize;
    qreal contentsScale;
    QColor fillColor;
    bool cachefrozen;
    bool smoothCache;
};

QT_END_NAMESPACE
#endif
