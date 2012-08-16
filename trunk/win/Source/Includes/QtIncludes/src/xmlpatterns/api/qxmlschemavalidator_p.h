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

#ifndef QXMLSCHEMAVALIDATOR_P_H
#define QXMLSCHEMAVALIDATOR_P_H

#include "qabstractmessagehandler.h"
#include "qabstracturiresolver.h"
#include "qautoptr_p.h"
#include "qcoloringmessagehandler_p.h"
#include "qxmlschema.h"
#include "qxmlschema_p.h"

#include "qxsdschemacontext_p.h"
#include "qxsdschema_p.h"

#include <QtNetwork/QNetworkAccessManager>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class QXmlSchemaValidatorPrivate
{
public:
    QXmlSchemaValidatorPrivate(const QXmlSchema &schema)
        : m_namePool(schema.namePool())
        , m_userMessageHandler(0)
        , m_uriResolver(0)
        , m_userNetworkAccessManager(0)
    {
        setSchema(schema);

        const QXmlSchemaPrivate *p = schema.d;

        // initialize the environment properties with the ones from the schema

        if (p->m_userNetworkAccessManager) // schema has user defined network access manager
            m_userNetworkAccessManager = p->m_userNetworkAccessManager;
        else
            m_networkAccessManager = p->m_networkAccessManager;

        if (p->m_userMessageHandler) // schema has user defined message handler
            m_userMessageHandler = p->m_userMessageHandler;
        else
            m_messageHandler = p->m_messageHandler;

        m_uriResolver = p->m_uriResolver;
    }

    void setSchema(const QXmlSchema &schema)
    {
        // use same name pool as the schema
        m_namePool = schema.namePool();
        m_schema = schema.d->m_schemaParserContext->schema();
        m_schemaDocumentUri = schema.documentUri();

        // create a new schema context
        m_context = QPatternist::XsdSchemaContext::Ptr(new QPatternist::XsdSchemaContext(m_namePool.d));
        m_context->m_schemaTypeFactory = schema.d->m_schemaContext->m_schemaTypeFactory;
        m_context->m_builtinTypesFacetList = schema.d->m_schemaContext->m_builtinTypesFacetList;

        m_originalSchema = schema;
    }

    QXmlNamePool                                                     m_namePool;
    QAbstractMessageHandler*                                         m_userMessageHandler;
    const QAbstractUriResolver*                                      m_uriResolver;
    QNetworkAccessManager*                                           m_userNetworkAccessManager;
    QPatternist::ReferenceCountedValue<QAbstractMessageHandler>::Ptr m_messageHandler;
    QPatternist::ReferenceCountedValue<QNetworkAccessManager>::Ptr   m_networkAccessManager;

    QXmlSchema                                                       m_originalSchema;
    QPatternist::XsdSchemaContext::Ptr                               m_context;
    QPatternist::XsdSchema::Ptr                                      m_schema;
    QUrl                                                             m_schemaDocumentUri;
};

QT_END_NAMESPACE

QT_END_HEADER

#endif
