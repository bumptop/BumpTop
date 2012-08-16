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

#ifndef QDECLARATIVEDATA_P_H
#define QDECLARATIVEDATA_P_H

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

#include <QtScript/qscriptvalue.h>
#include <private/qobject_p.h>
#include "private/qdeclarativeguard_p.h"

QT_BEGIN_NAMESPACE

class QDeclarativeCompiledData;
class QDeclarativeAbstractBinding;
class QDeclarativeContext;
class QDeclarativePropertyCache;
class QDeclarativeContextData;
class QDeclarativeNotifier;
class QDeclarativeDataExtended;
// This class is structured in such a way, that simply zero'ing it is the
// default state for elemental object allocations.  This is crucial in the
// workings of the QDeclarativeInstruction::CreateSimpleObject instruction.
// Don't change anything here without first considering that case!
class Q_AUTOTEST_EXPORT QDeclarativeData : public QAbstractDeclarativeData
{
public:
    QDeclarativeData()
        : ownMemory(true), ownContext(false), indestructible(true), explicitIndestructibleSet(false), 
          context(0), outerContext(0), bindings(0), nextContextObject(0), prevContextObject(0), bindingBitsSize(0), 
          bindingBits(0), lineNumber(0), columnNumber(0), deferredComponent(0), deferredIdx(0), 
          scriptValue(0), objectDataRefCount(0), propertyCache(0), guards(0), extendedData(0) {
          init(); 
      }

    static inline void init() {
        QAbstractDeclarativeData::destroyed = destroyed;
        QAbstractDeclarativeData::parentChanged = parentChanged;
        QAbstractDeclarativeData::objectNameChanged = objectNameChanged;
    }

    static void destroyed(QAbstractDeclarativeData *, QObject *);
    static void parentChanged(QAbstractDeclarativeData *, QObject *, QObject *);
    static void objectNameChanged(QAbstractDeclarativeData *, QObject *);

    void destroyed(QObject *);
    void parentChanged(QObject *, QObject *);
    void objectNameChanged(QObject *);

    void setImplicitDestructible() {
        if (!explicitIndestructibleSet) indestructible = false;
    }

    quint32 ownMemory:1;
    quint32 ownContext:1;
    quint32 indestructible:1;
    quint32 explicitIndestructibleSet:1;
    quint32 dummy:28;

    // The context that created the C++ object
    QDeclarativeContextData *context; 
    // The outermost context in which this object lives
    QDeclarativeContextData *outerContext;

    QDeclarativeAbstractBinding *bindings;

    // Linked list for QDeclarativeContext::contextObjects
    QDeclarativeData *nextContextObject;
    QDeclarativeData**prevContextObject;

    int bindingBitsSize;
    quint32 *bindingBits; 
    bool hasBindingBit(int) const;
    void clearBindingBit(int);
    void setBindingBit(QObject *obj, int);

    ushort lineNumber;
    ushort columnNumber;

    QDeclarativeCompiledData *deferredComponent; // Can't this be found from the context?
    unsigned int deferredIdx;

    // ### Can we make this QScriptValuePrivate so we incur no additional allocation
    // cost?
    QScriptValue *scriptValue;
    quint32 objectDataRefCount;
    QDeclarativePropertyCache *propertyCache;

    QDeclarativeGuard<QObject> *guards;

    static QDeclarativeData *get(const QObject *object, bool create = false) {
        QObjectPrivate *priv = QObjectPrivate::get(const_cast<QObject *>(object));
        if (priv->wasDeleted) {
            Q_ASSERT(!create);
            return 0;
        } else if (priv->declarativeData) {
            return static_cast<QDeclarativeData *>(priv->declarativeData);
        } else if (create) {
            priv->declarativeData = new QDeclarativeData;
            return static_cast<QDeclarativeData *>(priv->declarativeData);
        } else {
            return 0;
        }
    }

    bool hasExtendedData() const { return extendedData != 0; }
    QDeclarativeNotifier *objectNameNotifier() const;
    QHash<int, QObject *> *attachedProperties() const;

private:
    // For objectNameNotifier and attachedProperties
    mutable QDeclarativeDataExtended *extendedData;
};

template<class T>
void QDeclarativeGuard<T>::addGuard()
{
    Q_ASSERT(!prev);

    if (QObjectPrivate::get(o)->wasDeleted) 
        return;

    QDeclarativeData *data = QDeclarativeData::get(o, true);
    next = data->guards;
    if (next) reinterpret_cast<QDeclarativeGuard<T> *>(next)->prev = &next;
    data->guards = reinterpret_cast<QDeclarativeGuard<QObject> *>(this);
    prev = &data->guards;
}

template<class T>
void QDeclarativeGuard<T>::remGuard()
{
    Q_ASSERT(prev);

    if (next) reinterpret_cast<QDeclarativeGuard<T> *>(next)->prev = prev;
    *prev = next;
    next = 0;
    prev = 0;
}

QT_END_NAMESPACE

#endif // QDECLARATIVEDATA_P_H
