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

#ifndef QDECLARATIVEJSENGINE_P_H
#define QDECLARATIVEJSENGINE_P_H

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

#include "private/qdeclarativejsglobal_p.h"
#include "private/qdeclarativejsastfwd_p.h"

#include <QString>
#include <QSet>

QT_QML_BEGIN_NAMESPACE

namespace QDeclarativeJS {
class QML_PARSER_EXPORT NameId
{
    QString _text;

public:
    NameId(const QChar *u, int s)
        : _text(u, s)
    { }

    const QString asString() const
    { return _text; }

    bool operator == (const NameId &other) const
    { return _text == other._text; }

    bool operator != (const NameId &other) const
    { return _text != other._text; }

    bool operator < (const NameId &other) const
    { return _text < other._text; }
};

uint qHash(const QDeclarativeJS::NameId &id);

} // end of namespace QDeclarativeJS

#if defined(Q_CC_MSVC) && _MSC_VER <= 1300
//this ensures that code outside QDeclarativeJS can use the hash function
//it also a workaround for some compilers
inline uint qHash(const QDeclarativeJS::NameId &nameId) { return QDeclarativeJS::qHash(nameId); }
#endif

namespace QDeclarativeJS {

class Lexer;
class NodePool;

namespace Ecma {

class QML_PARSER_EXPORT RegExp
{
public:
    enum RegExpFlag {
        Global     = 0x01,
        IgnoreCase = 0x02,
        Multiline  = 0x04
    };

public:
    static int flagFromChar(const QChar &);
    static QString flagsToString(int flags);
};

} // end of namespace Ecma

class QML_PARSER_EXPORT DiagnosticMessage
{
public:
    enum Kind { Warning, Error };

    DiagnosticMessage()
        : kind(Error) {}

    DiagnosticMessage(Kind kind, const AST::SourceLocation &loc, const QString &message)
        : kind(kind), loc(loc), message(message) {}

    bool isWarning() const
    { return kind == Warning; }

    bool isError() const
    { return kind == Error; }

    Kind kind;
    AST::SourceLocation loc;
    QString message;
};

class QML_PARSER_EXPORT Engine
{
    Lexer *_lexer;
    NodePool *_nodePool;
    QSet<NameId> _literals;
    QList<QDeclarativeJS::AST::SourceLocation> _comments;

public:
    Engine();
    ~Engine();

    QSet<NameId> literals() const;

    void addComment(int pos, int len, int line, int col);
    QList<QDeclarativeJS::AST::SourceLocation> comments() const;

    NameId *intern(const QChar *u, int s);

    static QString toString(NameId *id);

    Lexer *lexer() const;
    void setLexer(Lexer *lexer);

    NodePool *nodePool() const;
    void setNodePool(NodePool *nodePool);
};

} // end of namespace QDeclarativeJS

QT_QML_END_NAMESPACE

#endif // QDECLARATIVEJSENGINE_P_H
