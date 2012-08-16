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

#ifndef Patternist_XsdIdentityConstraint_H
#define Patternist_XsdIdentityConstraint_H

#include "qnamedschemacomponent_p.h"
#include "qxsdannotated_p.h"
#include "qxsdxpathexpression_p.h"

#include <QtCore/QStringList>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

namespace QPatternist
{
    /**
     * @short Represents a XSD identity constraint object.
     *
     * This class represents the <em>identity constraint</em> object of a XML schema as described
     * <a href="http://www.w3.org/TR/xmlschema11-1/#cIdentity-constraint_Definitions">here</a>.
     *
     * It contains information from either a <em>key</em> object, a <em>keyref</em> object or an
     * <em>unique</em> object.
     *
     * @see <a href="http://www.w3.org/Submission/2004/SUBM-xmlschema-api-20040309/xml-schema-api.html#Interface-XSIdentityConstraint">XML Schema API reference</a>
     * @ingroup Patternist_schema
     * @author Tobias Koenig <tobias.koenig@nokia.com>
     */
    class XsdIdentityConstraint : public NamedSchemaComponent, public XsdAnnotated
    {
        public:
            typedef QExplicitlySharedDataPointer<XsdIdentityConstraint> Ptr;
            typedef QList<XsdIdentityConstraint::Ptr> List;

            /**
             * Describes the <a href="http://www.w3.org/TR/xmlschema11-1/#icd-identity-constraint_category">category</a> of the identity constraint.
             */
            enum Category
            {
                Key = 1,        ///< The constraint is a key constraint
                KeyReference,   ///< The constraint is a keyref constraint
                Unique          ///< The constraint is an unique constraint
            };

            /**
             * Sets the @p category of the identity constraint.
             *
             * @see Category
             */
            void setCategory(Category category);

            /**
             * Returns the category of the identity constraint.
             */
            Category category() const;

            /**
             * Sets the @p selector of the identity constraint.
             *
             * The selector is a restricted XPath 1.0 expression,
             * that selects a set of nodes.
             *
             * @see <a href="http://www.w3.org/TR/xmlschema11-1/#icd-selector"></a>
             */
            void setSelector(const XsdXPathExpression::Ptr &selector);

            /**
             * Returns the selector of the identity constraint.
             */
            XsdXPathExpression::Ptr selector() const;

            /**
             * Sets the @p fields of the identity constraint.
             *
             * Each field is a restricted XPath 1.0 expression,
             * that selects a set of nodes.
             *
             * @see <a href="http://www.w3.org/TR/xmlschema11-1/#icd-fields"></a>
             */
            void setFields(const XsdXPathExpression::List &fields);

            /**
             * Adds a new @p field to the identity constraint.
             */
            void addField(const XsdXPathExpression::Ptr &field);

            /**
             * Returns all fields of the identity constraint.
             */
            XsdXPathExpression::List fields() const;

            /**
             * Sets the referenced @p key of the identity constraint.
             *
             * The key points to a identity constraint of type Key or Unique.
             *
             * The identity constraint has only a referenced key if its
             * type is KeyReference.
             *
             * @see <a href="http://www.w3.org/TR/xmlschema11-1/#icd-referenced_key"></a>
             */
            void setReferencedKey(const XsdIdentityConstraint::Ptr &key);

            /**
             * Returns the referenced key of the identity constraint or an empty
             * pointer if its type is not KeyReference.
             */
            XsdIdentityConstraint::Ptr referencedKey() const;

        private:
            Category                   m_category;
            XsdXPathExpression::Ptr    m_selector;
            XsdXPathExpression::List   m_fields;
            XsdIdentityConstraint::Ptr m_referencedKey;
    };
}

QT_END_NAMESPACE

QT_END_HEADER

#endif
