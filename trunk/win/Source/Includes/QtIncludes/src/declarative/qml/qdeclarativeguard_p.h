/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtCore module of the Qt Toolkit.
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

#ifndef QDECLARATIVEGUARD_P_H
#define QDECLARATIVEGUARD_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qapplication_*.cpp, qwidget*.cpp and qfiledialog.cpp.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qglobal.h>
#include <QtCore/qvariant.h>

QT_BEGIN_NAMESPACE

class QObject;
template<class T>
class QDeclarativeGuard
{
    QObject *o;
    QDeclarativeGuard<QObject> *next;
    QDeclarativeGuard<QObject> **prev;
    friend class QDeclarativeData;
public:
    inline QDeclarativeGuard();
    inline QDeclarativeGuard(T *);
    inline QDeclarativeGuard(const QDeclarativeGuard<T> &);
    inline virtual ~QDeclarativeGuard();

    inline QDeclarativeGuard<T> &operator=(const QDeclarativeGuard<T> &o);
    inline QDeclarativeGuard<T> &operator=(T *);
    
    inline bool isNull() const
        { return !o; }

    inline T* operator->() const
        { return static_cast<T*>(const_cast<QObject*>(o)); }
    inline T& operator*() const
        { return *static_cast<T*>(const_cast<QObject*>(o)); }
    inline operator T*() const
        { return static_cast<T*>(const_cast<QObject*>(o)); }
    inline T* data() const
        { return static_cast<T*>(const_cast<QObject*>(o)); }

protected:
    virtual void objectDestroyed(T *) {}

private:
    inline void addGuard();
    inline void remGuard();
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QDeclarativeGuard<QObject>)

#include "private/qdeclarativedata_p.h"

QT_BEGIN_NAMESPACE

template<class T>
QDeclarativeGuard<T>::QDeclarativeGuard()
: o(0), next(0), prev(0)
{
}

template<class T>
QDeclarativeGuard<T>::QDeclarativeGuard(T *g)
: o(g), next(0), prev(0)
{
    if (o) addGuard();
}

template<class T>
QDeclarativeGuard<T>::QDeclarativeGuard(const QDeclarativeGuard<T> &g)
: o(g.o), next(0), prev(0)
{
    if (o) addGuard();
}

template<class T>
QDeclarativeGuard<T>::~QDeclarativeGuard()
{
    if (prev) remGuard();
    o = 0;
}

template<class T>
QDeclarativeGuard<T> &QDeclarativeGuard<T>::operator=(const QDeclarativeGuard<T> &g)
{
    if (g.o != o) {
        if (prev) remGuard();
        o = g.o;
        if (o) addGuard();
    }
    return *this;
}

template<class T>
QDeclarativeGuard<T> &QDeclarativeGuard<T>::operator=(T *g)
{
    if (g != o) {
        if (prev) remGuard();
        o = g;
        if (o) addGuard();
    }
    return *this;
}

QT_END_NAMESPACE

#endif // QDECLARATIVEGUARD_P_H
