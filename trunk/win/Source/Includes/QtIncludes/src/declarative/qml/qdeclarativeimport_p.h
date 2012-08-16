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

#ifndef QDECLARATIVEIMPORT_P_H
#define QDECLARATIVEIMPORT_P_H

#include <QtCore/qurl.h>
#include <QtCore/qcoreapplication.h>
#include <QtCore/qset.h>
#include <private/qdeclarativedirparser_p.h>
#include <private/qdeclarativescriptparser_p.h>
#include <private/qdeclarativemetatype_p.h>

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

QT_BEGIN_NAMESPACE

class QDeclarativeTypeNameCache;
class QDeclarativeEngine;
class QDir;
class QDeclarativeImportedNamespace;
class QDeclarativeImportsPrivate;
class QDeclarativeImportDatabase;

class QDeclarativeImports
{
public:
    QDeclarativeImports();
    QDeclarativeImports(const QDeclarativeImports &);
    ~QDeclarativeImports();
    QDeclarativeImports &operator=(const QDeclarativeImports &);

    void setBaseUrl(const QUrl &url);
    QUrl baseUrl() const;

    bool resolveType(const QByteArray& type,
                     QDeclarativeType** type_return, QUrl* url_return,
                     int *version_major, int *version_minor,
                     QDeclarativeImportedNamespace** ns_return,
                     QString *errorString = 0) const;
    bool resolveType(QDeclarativeImportedNamespace*, 
                     const QByteArray& type,
                     QDeclarativeType** type_return, QUrl* url_return,
                     int *version_major, int *version_minor) const;

    bool addImport(QDeclarativeImportDatabase *, 
                   const QString& uri, const QString& prefix, int vmaj, int vmin, 
                   QDeclarativeScriptParser::Import::Type importType,
                   const QDeclarativeDirComponents &qmldircomponentsnetwork, 
                   QString *errorString);

    void populateCache(QDeclarativeTypeNameCache *cache, QDeclarativeEngine *) const;

private:
    friend class QDeclarativeImportDatabase;
    QDeclarativeImportsPrivate *d;
};

class QDeclarativeImportDatabase
{
    Q_DECLARE_TR_FUNCTIONS(QDeclarativeImportDatabase)
public:
    QDeclarativeImportDatabase(QDeclarativeEngine *);
    ~QDeclarativeImportDatabase();

    bool importPlugin(const QString &filePath, const QString &uri, QString *errorString);

    QStringList importPathList() const;
    void setImportPathList(const QStringList &paths);
    void addImportPath(const QString& dir);

    QStringList pluginPathList() const;
    void setPluginPathList(const QStringList &paths);
    void addPluginPath(const QString& path);

private:
    friend class QDeclarativeImportsPrivate;
    QString resolvePlugin(const QDir &qmldirPath, const QString &qmldirPluginPath, 
                          const QString &baseName, const QStringList &suffixes,
                          const QString &prefix = QString());
    QString resolvePlugin(const QDir &qmldirPath, const QString &qmldirPluginPath, 
                          const QString &baseName);


    QStringList filePluginPath;
    QStringList fileImportPath;

    QSet<QString> initializedPlugins;
    QDeclarativeEngine *engine;
};

QT_END_NAMESPACE

#endif // QDECLARATIVEIMPORT_P_H

