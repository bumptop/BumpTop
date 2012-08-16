/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#ifndef QXMLSCHEMA_P_H
#define QXMLSCHEMA_P_H

#include "qabstractmessagehandler.h"
#include "qabstracturiresolver.h"
#include "qautoptr_p.h"
#include "qcoloringmessagehandler_p.h"
#include "qreferencecountedvalue_p.h"

#include "qxsdschemacontext_p.h"
#include "qxsdschemaparser_p.h"
#include "qxsdschemaparsercontext_p.h"

#include <QtCore/QSharedData>
#include <QtNetwork/QNetworkAccessManager>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class QXmlSchemaPrivate : public QSharedData
{
    public:
        QXmlSchemaPrivate(const QXmlNamePool &namePool);
        QXmlSchemaPrivate(const QPatternist::XsdSchemaContext::Ptr &schemaContext);
        QXmlSchemaPrivate(const QXmlSchemaPrivate &other);

        void load(const QUrl &source, const QString &targetNamespace);
        void load(QIODevice *source, const QUrl &documentUri, const QString &targetNamespace);
        void load(const QByteArray &data, const QUrl &documentUri, const QString &targetNamespace);
        bool isValid() const;
        QXmlNamePool namePool() const;
        QUrl documentUri() const;
        void setMessageHandler(QAbstractMessageHandler *handler);
        QAbstractMessageHandler *messageHandler() const;
        void setUriResolver(const QAbstractUriResolver *resolver);
        const QAbstractUriResolver *uriResolver() const;
        void setNetworkAccessManager(QNetworkAccessManager *networkmanager);
        QNetworkAccessManager *networkAccessManager() const;

        QXmlNamePool                                                     m_namePool;
        QAbstractMessageHandler*                                         m_userMessageHandler;
        const QAbstractUriResolver*                                      m_uriResolver;
        QNetworkAccessManager*                                           m_userNetworkAccessManager;
        QPatternist::ReferenceCountedValue<QAbstractMessageHandler>::Ptr m_messageHandler;
        QPatternist::ReferenceCountedValue<QNetworkAccessManager>::Ptr   m_networkAccessManager;

        QPatternist::XsdSchemaContext::Ptr                               m_schemaContext;
        QPatternist::XsdSchemaParserContext::Ptr                         m_schemaParserContext;
        bool                                                             m_schemaIsValid;
        QUrl                                                             m_documentUri;
};

QT_END_NAMESPACE

QT_END_HEADER

#endif
