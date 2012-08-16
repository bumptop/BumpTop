/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef QDECLARATIVEDOM_P_P_H
#define QDECLARATIVEDOM_P_P_H

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

#include "private/qdeclarativeparser_p.h"

#include <QtCore/QtGlobal>

QT_BEGIN_NAMESPACE

class QDeclarativeDomDocumentPrivate : public QSharedData
{
public:
    QDeclarativeDomDocumentPrivate();
    QDeclarativeDomDocumentPrivate(const QDeclarativeDomDocumentPrivate &o)
    : QSharedData(o) { qFatal("Not impl"); }
    ~QDeclarativeDomDocumentPrivate();

    QList<QDeclarativeError> errors;
    QList<QDeclarativeDomImport> imports;
    QDeclarativeParser::Object *root;
    QList<int> automaticSemicolonOffsets;
};

class QDeclarativeDomObjectPrivate : public QSharedData
{
public:
    QDeclarativeDomObjectPrivate();
    QDeclarativeDomObjectPrivate(const QDeclarativeDomObjectPrivate &o)
    : QSharedData(o) { qFatal("Not impl"); }
    ~QDeclarativeDomObjectPrivate();

    typedef QList<QPair<QDeclarativeParser::Property *, QByteArray> > Properties;
    Properties properties() const;
    Properties properties(QDeclarativeParser::Property *) const;

    QDeclarativeParser::Object *object;
};

class QDeclarativeDomPropertyPrivate : public QSharedData
{
public:
    QDeclarativeDomPropertyPrivate();
    QDeclarativeDomPropertyPrivate(const QDeclarativeDomPropertyPrivate &o)
    : QSharedData(o) { qFatal("Not impl"); }
    ~QDeclarativeDomPropertyPrivate();

    QByteArray propertyName;
    QDeclarativeParser::Property *property;
};

class QDeclarativeDomDynamicPropertyPrivate : public QSharedData
{
public:
    QDeclarativeDomDynamicPropertyPrivate();
    QDeclarativeDomDynamicPropertyPrivate(const QDeclarativeDomDynamicPropertyPrivate &o)
    : QSharedData(o) { qFatal("Not impl"); }
    ~QDeclarativeDomDynamicPropertyPrivate();

    bool valid;
    QDeclarativeParser::Object::DynamicProperty property;
};

class QDeclarativeDomValuePrivate : public QSharedData
{
public:
    QDeclarativeDomValuePrivate();
    QDeclarativeDomValuePrivate(const QDeclarativeDomValuePrivate &o)
    : QSharedData(o) { qFatal("Not impl"); }
    ~QDeclarativeDomValuePrivate();

    QDeclarativeParser::Property *property;
    QDeclarativeParser::Value *value;
};

class QDeclarativeDomBasicValuePrivate : public QSharedData
{
public:
    QDeclarativeDomBasicValuePrivate();
    QDeclarativeDomBasicValuePrivate(const QDeclarativeDomBasicValuePrivate &o) 
    : QSharedData(o) { qFatal("Not impl"); }
    ~QDeclarativeDomBasicValuePrivate();

    QDeclarativeParser::Value *value;
};

class QDeclarativeDomImportPrivate : public QSharedData
{
public:
    QDeclarativeDomImportPrivate();
    QDeclarativeDomImportPrivate(const QDeclarativeDomImportPrivate &o) 
    : QSharedData(o) { qFatal("Not impl"); }
    ~QDeclarativeDomImportPrivate();

    enum Type { Library, File };

    Type type;
    QString uri;
    QString version;
    QString qualifier;
};

QT_END_NAMESPACE

#endif // QDECLARATIVEDOM_P_P_H

