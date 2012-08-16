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

#ifndef QDECLARATIVEPROPERTY_P_H
#define QDECLARATIVEPROPERTY_P_H

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

#include "qdeclarativeproperty.h"

#include <private/qobject_p.h>
#include <private/qdeclarativeglobal_p.h>
#include <private/qdeclarativepropertycache_p.h>
#include <private/qdeclarativeguard_p.h>

QT_BEGIN_NAMESPACE

class QDeclarativeContext;
class QDeclarativeEnginePrivate;
class QDeclarativeExpression;
class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativePropertyPrivate : public QDeclarativeRefCount
{
public:
    enum WriteFlag { BypassInterceptor = 0x01, DontRemoveBinding = 0x02, RemoveBindingOnAliasWrite = 0x04 };
    Q_DECLARE_FLAGS(WriteFlags, WriteFlag)

    QDeclarativePropertyPrivate()
        : context(0), engine(0), object(0), isNameCached(false) {}

    QDeclarativeContextData *context;
    QDeclarativeEngine *engine;
    QDeclarativeGuard<QObject> object;

    bool isNameCached:1;
    QDeclarativePropertyCache::Data core;
    QString nameCache;

    // Describes the "virtual" value-type sub-property.  
    QDeclarativePropertyCache::ValueTypeData valueType;

    void initProperty(QObject *obj, const QString &name);
    void initDefault(QObject *obj);

    bool isValueType() const;
    int propertyType() const;
    QDeclarativeProperty::Type type() const;
    QDeclarativeProperty::PropertyTypeCategory propertyTypeCategory() const;

    QVariant readValueProperty();
    bool writeValueProperty(const QVariant &, WriteFlags);

    static const QMetaObject *rawMetaObjectForType(QDeclarativeEnginePrivate *, int);
    static bool writeEnumProperty(const QMetaProperty &prop, int idx, QObject *object, 
                                  const QVariant &value, int flags);
    static bool write(QObject *, const QDeclarativePropertyCache::Data &, const QVariant &, 
                      QDeclarativeContextData *, WriteFlags flags = 0);
    static void findAliasTarget(QObject *, int, QObject **, int *);
    static QDeclarativeAbstractBinding *setBinding(QObject *, int coreIndex, int valueTypeIndex /* -1 */,
                                                   QDeclarativeAbstractBinding *,
                                                   WriteFlags flags = DontRemoveBinding);
    static QDeclarativeAbstractBinding *setBindingNoEnable(QObject *, int coreIndex, int valueTypeIndex /* -1 */,
                                                           QDeclarativeAbstractBinding *);
    static QDeclarativeAbstractBinding *binding(QObject *, int coreIndex, int valueTypeIndex /* -1 */);

    static QByteArray saveValueType(const QMetaObject *, int, 
                                    const QMetaObject *, int);
    static QByteArray saveProperty(const QMetaObject *, int);

    static QDeclarativeProperty restore(const QByteArray &, QObject *, QDeclarativeContextData *);
    static QDeclarativeProperty restore(const QDeclarativePropertyCache::Data &,
                                        const QDeclarativePropertyCache::ValueTypeData &,
                                        QObject *,
                                        QDeclarativeContextData *);

    static bool equal(const QMetaObject *, const QMetaObject *);
    static bool canConvert(const QMetaObject *from, const QMetaObject *to);

    // "Public" (to QML) methods
    static QDeclarativeAbstractBinding *binding(const QDeclarativeProperty &that);
    static QDeclarativeAbstractBinding *setBinding(const QDeclarativeProperty &that,
                                                   QDeclarativeAbstractBinding *,
                                                   WriteFlags flags = DontRemoveBinding);
    static QDeclarativeExpression *signalExpression(const QDeclarativeProperty &that);
    static QDeclarativeExpression *setSignalExpression(const QDeclarativeProperty &that, 
                                                       QDeclarativeExpression *) ;
    static bool write(const QDeclarativeProperty &that, const QVariant &, WriteFlags);
    static int valueTypeCoreIndex(const QDeclarativeProperty &that);
    static int bindingIndex(const QDeclarativeProperty &that);
    static QMetaMethod findSignalByName(const QMetaObject *mo, const QByteArray &);
    static bool connect(const QObject *sender, int signal_index,
                        const QObject *receiver, int method_index,
                        int type = 0, int *types = 0);
    static const QMetaObject *metaObjectForProperty(const QMetaObject *, int);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QDeclarativePropertyPrivate::WriteFlags)

QT_END_NAMESPACE

#endif // QDECLARATIVEPROPERTY_P_H
