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

#ifndef Q3OBJECTDICT_H
#define Q3OBJECTDICT_H

#include <QtCore/qmetaobject.h>
#include <Qt3Support/q3asciidict.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Qt3SupportLight)

//
// The object dictionary is a collection of QMetaObjects
//

class Q3ObjectDictionary : public Q3AsciiDict<QMetaObject>
{
public:
    Q3ObjectDictionary(int size=17,bool cs=true,bool ck=true)
	: Q3AsciiDict<QMetaObject>(size,cs,ck) {}
    Q3ObjectDictionary( const Q3ObjectDictionary &dict )
	: Q3AsciiDict<QMetaObject>(dict) {}
   ~Q3ObjectDictionary() { clear(); }
    Q3ObjectDictionary &operator=(const Q3ObjectDictionary &dict)
	{ return (Q3ObjectDictionary&)Q3AsciiDict<QMetaObject>::operator=(dict);}
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // Q3OBJECTDICT_H
