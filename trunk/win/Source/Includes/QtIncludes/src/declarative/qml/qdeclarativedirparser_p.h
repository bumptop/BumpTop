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

#ifndef QDECLARATIVEDIRPARSER_P_H
#define QDECLARATIVEDIRPARSER_P_H

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

#include <QtCore/QUrl>
#include <QtCore/QHash>

QT_BEGIN_NAMESPACE

class QDeclarativeError;
class QDeclarativeDirParser
{
    Q_DISABLE_COPY(QDeclarativeDirParser)

public:
    QDeclarativeDirParser();
    ~QDeclarativeDirParser();

    QUrl url() const;
    void setUrl(const QUrl &url);

    QString source() const;
    void setSource(const QString &source);

    bool isParsed() const;
    bool parse();

    bool hasError() const;
    QList<QDeclarativeError> errors() const;

    struct Plugin
    {
        Plugin() {}

        Plugin(const QString &name, const QString &path)
            : name(name), path(path) {}

        QString name;
        QString path;
    };

    struct Component
    {
        Component()
            : majorVersion(0), minorVersion(0), internal(false) {}

        Component(const QString &typeName, const QString &fileName, int majorVersion, int minorVersion)
            : typeName(typeName), fileName(fileName), majorVersion(majorVersion), minorVersion(minorVersion),
            internal(false) {}

        QString typeName;
        QString fileName;
        int majorVersion;
        int minorVersion;
        bool internal;
    };

    QList<Component> components() const;
    QList<Plugin> plugins() const;

private:
    void reportError(int line, int column, const QString &message);

private:
    QList<QDeclarativeError> _errors;
    QUrl _url;
    QString _source;
    QList<Component> _components;
    QList<Plugin> _plugins;
    unsigned _isParsed: 1;
};

typedef QList<QDeclarativeDirParser::Component> QDeclarativeDirComponents;


QT_END_NAMESPACE

#endif // QDECLARATIVEDIRPARSER_P_H
