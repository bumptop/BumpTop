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

#ifndef QDECLARATIVEPROPERTY_H
#define QDECLARATIVEPROPERTY_H

#include <QtCore/qmetaobject.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Declarative)

class QObject;
class QVariant;
class QDeclarativeContext;
class QDeclarativeEngine;

class QDeclarativePropertyPrivate;
class Q_DECLARATIVE_EXPORT QDeclarativeProperty
{
public:
    enum PropertyTypeCategory {
        InvalidCategory,
        List,
        Object,
        Normal
    };

    enum Type { 
        Invalid,
        Property,
        SignalProperty
    };

    QDeclarativeProperty();
    ~QDeclarativeProperty();

    QDeclarativeProperty(QObject *);
    QDeclarativeProperty(QObject *, QDeclarativeContext *);
    QDeclarativeProperty(QObject *, QDeclarativeEngine *);

    QDeclarativeProperty(QObject *, const QString &);
    QDeclarativeProperty(QObject *, const QString &, QDeclarativeContext *);
    QDeclarativeProperty(QObject *, const QString &, QDeclarativeEngine *);

    QDeclarativeProperty(const QDeclarativeProperty &);
    QDeclarativeProperty &operator=(const QDeclarativeProperty &);

    bool operator==(const QDeclarativeProperty &) const;

    Type type() const;
    bool isValid() const;
    bool isProperty() const;
    bool isSignalProperty() const;

    int propertyType() const;
    PropertyTypeCategory propertyTypeCategory() const;
    const char *propertyTypeName() const;

    QString name() const;

    QVariant read() const;
    static QVariant read(QObject *, const QString &);
    static QVariant read(QObject *, const QString &, QDeclarativeContext *);
    static QVariant read(QObject *, const QString &, QDeclarativeEngine *);

    bool write(const QVariant &) const;
    static bool write(QObject *, const QString &, const QVariant &);
    static bool write(QObject *, const QString &, const QVariant &, QDeclarativeContext *);
    static bool write(QObject *, const QString &, const QVariant &, QDeclarativeEngine *);

    bool reset() const;

    bool hasNotifySignal() const;
    bool needsNotifySignal() const;
    bool connectNotifySignal(QObject *dest, const char *slot) const;
    bool connectNotifySignal(QObject *dest, int method) const;

    bool isWritable() const;
    bool isDesignable() const;
    bool isResettable() const;
    QObject *object() const;

    int index() const;
    QMetaProperty property() const;
    QMetaMethod method() const;

private:
    friend class QDeclarativePropertyPrivate;
    QDeclarativePropertyPrivate *d;
};
typedef QList<QDeclarativeProperty> QDeclarativeProperties;

inline uint qHash (const QDeclarativeProperty &key)
{
    return qHash(key.object()) + qHash(key.name());
}

QT_END_NAMESPACE

QT_END_HEADER

#endif // QDECLARATIVEPROPERTY_H
