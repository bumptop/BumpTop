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

#ifndef QDECLARATIVEINTEGERCACHE_P_H
#define QDECLARATIVEINTEGERCACHE_P_H

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

#include "private/qdeclarativerefcount_p.h"
#include "private/qdeclarativecleanup_p.h"

#include <QtCore/qhash.h>

#include <private/qscriptdeclarativeclass_p.h>

QT_BEGIN_NAMESPACE

class QDeclarativeType;
class QDeclarativeEngine;
class QDeclarativeIntegerCache : public QDeclarativeRefCount, public QDeclarativeCleanup
{
public:
    QDeclarativeIntegerCache(QDeclarativeEngine *);
    virtual ~QDeclarativeIntegerCache();

    inline int count() const;
    void add(const QString &, int);
    int value(const QString &);
    QString findId(int value) const;
    inline int value(const QScriptDeclarativeClass::Identifier &id) const;

protected:
    virtual void clear();

private:
    struct Data : public QScriptDeclarativeClass::PersistentIdentifier {
        Data(const QScriptDeclarativeClass::PersistentIdentifier &i, int v) 
        : QScriptDeclarativeClass::PersistentIdentifier(i), value(v) {}

        int value;
    };

    typedef QHash<QString, Data *> StringCache;
    typedef QHash<QScriptDeclarativeClass::Identifier, Data *> IdentifierCache;

    StringCache stringCache;
    IdentifierCache identifierCache;
    QDeclarativeEngine *engine;
};

int QDeclarativeIntegerCache::value(const QScriptDeclarativeClass::Identifier &id) const
{
    Data *d = identifierCache.value(id);
    return d?d->value:-1;
}

int QDeclarativeIntegerCache::count() const 
{
    return stringCache.count();
}

QT_END_NAMESPACE

#endif // QDECLARATIVEINTEGERCACHE_P_H

