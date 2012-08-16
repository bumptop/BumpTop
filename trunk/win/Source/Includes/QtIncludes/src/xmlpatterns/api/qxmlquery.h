/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtXmlPatterns module of the Qt Toolkit.
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

#ifndef QXMLQUERY_H
#define QXMLQUERY_H

#include <QtCore/QUrl>
#include <QtXmlPatterns/QAbstractXmlNodeModel>
#include <QtXmlPatterns/QAbstractXmlReceiver>
#include <QtXmlPatterns/QXmlNamePool>

QT_BEGIN_HEADER
QT_BEGIN_NAMESPACE

QT_MODULE(XmlPatterns)

class QAbstractMessageHandler;
class QAbstractUriResolver;
class QIODevice;
class QNetworkAccessManager;
class QXmlName;
class QXmlNodeIndex;
class QXmlQueryPrivate;
class QXmlResultItems;
class QXmlSerializer;

/* The members in the namespace QPatternistSDK are internal, not part of the public API, and
 * unsupported. Using them leads to undefined behavior. */
namespace QPatternistSDK
{
    class TestCase;
}

namespace QPatternist
{
    class XsdSchemaParser;
    class XsdValidatingInstanceReader;
    class VariableLoader;
}

class Q_XMLPATTERNS_EXPORT QXmlQuery
{
public:
    enum QueryLanguage
    {
        XQuery10                                = 1,
        XSLT20                                  = 2,
        XmlSchema11IdentityConstraintSelector   = 1024,
        XmlSchema11IdentityConstraintField      = 2048,
        XPath20                                 = 4096
    };

    QXmlQuery();
    QXmlQuery(const QXmlQuery &other);
    QXmlQuery(const QXmlNamePool &np);
    QXmlQuery(QueryLanguage queryLanguage,
              const QXmlNamePool &np = QXmlNamePool());
    ~QXmlQuery();
    QXmlQuery &operator=(const QXmlQuery &other);

    void setMessageHandler(QAbstractMessageHandler *messageHandler);
    QAbstractMessageHandler *messageHandler() const;

    void setQuery(const QString &sourceCode, const QUrl &documentURI = QUrl());
    void setQuery(QIODevice *sourceCode, const QUrl &documentURI = QUrl());
    void setQuery(const QUrl &queryURI, const QUrl &baseURI = QUrl());

    QXmlNamePool namePool() const;

    void bindVariable(const QXmlName &name, const QXmlItem &value);
    void bindVariable(const QString &localName, const QXmlItem &value);

    void bindVariable(const QXmlName &name, QIODevice *);
    void bindVariable(const QString &localName, QIODevice *);
    void bindVariable(const QXmlName &name, const QXmlQuery &query);
    void bindVariable(const QString &localName, const QXmlQuery &query);

    bool isValid() const;

    void evaluateTo(QXmlResultItems *result) const;
    bool evaluateTo(QAbstractXmlReceiver *callback) const;
    bool evaluateTo(QStringList *target) const;
    bool evaluateTo(QIODevice *target) const;
    bool evaluateTo(QString *output) const;

    void setUriResolver(const QAbstractUriResolver *resolver);
    const QAbstractUriResolver *uriResolver() const;

    void setFocus(const QXmlItem &item);
    bool setFocus(const QUrl &documentURI);
    bool setFocus(QIODevice *document);
    bool setFocus(const QString &focus);

    void setInitialTemplateName(const QXmlName &name);
    void setInitialTemplateName(const QString &name);
    QXmlName initialTemplateName() const;

    void setNetworkAccessManager(QNetworkAccessManager *newManager);
    QNetworkAccessManager *networkAccessManager() const;

    QueryLanguage queryLanguage() const;
private:
    friend class QXmlName;
    friend class QXmlSerializer;
    friend class QPatternistSDK::TestCase;
    friend class QPatternist::XsdSchemaParser;
    friend class QPatternist::XsdValidatingInstanceReader;
    friend class QPatternist::VariableLoader;
    template<typename TInputType> friend bool setFocusHelper(QXmlQuery *const queryInstance,
                                                             const TInputType &focusValue);
    QXmlQueryPrivate *d;
};

QT_END_NAMESPACE
QT_END_HEADER

#endif
