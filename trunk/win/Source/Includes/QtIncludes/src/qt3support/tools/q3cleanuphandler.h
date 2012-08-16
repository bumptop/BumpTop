/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt3Support module of the Qt Toolkit.
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

#ifndef Q3CLEANUPHANDLER_H
#define Q3CLEANUPHANDLER_H

#include <QtCore/qlist.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Qt3SupportLight)

template<class T>
class Q3CleanupHandler
{
    QListData p;
public:
    inline Q3CleanupHandler()
    { p.d = 0; }
    ~Q3CleanupHandler()
    {
        if (p.d) {
            for (int i = 0; i < p.size(); ++i) {
                T** t = static_cast<T**>(*p.at(i));
                delete *t;
                *t = 0;
            }
            qFree(p.d);
            p.d = 0;
        }
    }

    T* add(T **object)
    {
        if (!p.d) {
            p.d = &QListData::shared_null;
            p.d->ref.ref();
            p.detach();
        }
        *p.prepend() =  object;
        return *object;
    }
    void remove(T **object)
    {
        if (p.d)
            for (int i = 0; i < p.size(); ++i)
                if (*p.at(i) == object)
                    p.remove(i--);
    }
};

template<class T>
class Q3SingleCleanupHandler
{
    T **object;
public:
    inline Q3SingleCleanupHandler()
    : object(0) {}
    inline ~Q3SingleCleanupHandler()
    { if (object) { delete *object; *object = 0; } }
    inline T* set(T **o)
    { object = o; return *object; }
    inline void reset() { object = 0; }
};

QT_END_NAMESPACE

QT_END_HEADER

#endif //Q3CLEANUPHANDLER_H
