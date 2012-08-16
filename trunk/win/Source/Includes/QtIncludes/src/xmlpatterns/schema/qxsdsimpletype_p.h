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

#ifndef Patternist_XsdSimpleType_H
#define Patternist_XsdSimpleType_H

#include "qanysimpletype_p.h"
#include "qxsdfacet_p.h"
#include "qxsduserschematype_p.h"

#include <QtCore/QSet>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

namespace QPatternist
{
    /**
     * @short Represents a XSD simpleType object.
     *
     * This class represents the <em>simpleType</em> object of a XML schema as described
     * <a href="http://www.w3.org/TR/xmlschema-2/#rf-defn">here</a>.
     *
     * It contains information from either a top-level simple type declaration (as child of a <em>schema</em> object)
     * or a local simple type declaration (as descendant of an <em>element</em> or <em>complexType</em> object).
     *
     * @see <a href="http://www.w3.org/Submission/2004/SUBM-xmlschema-api-20040309/xml-schema-api.html#Interface-XSSimpleType">XML Schema API reference</a>
     * @ingroup Patternist_schema
     * @author Tobias Koenig <tobias.koenig@nokia.com>
     */
    class XsdSimpleType : public XsdUserSchemaType<AnySimpleType>
    {
        public:
            typedef QExplicitlySharedDataPointer<XsdSimpleType> Ptr;

            /**
             * Returns the display name of the simple type.
             *
             * @param namePool The name pool the type name is stored in.
             */
            virtual QString displayName(const NamePool::Ptr &namePool) const;

            /**
             * Sets the base @p type of the simple type.
             *
             * @see <a href="http://www.w3.org/TR/xmlschema-2/#defn-basetype">Base Type Definition</a>
             */
            void setWxsSuperType(const SchemaType::Ptr &type);

            /**
             * Returns the base type of the simple type or an empty pointer if no base type is
             * set.
             */
            virtual SchemaType::Ptr wxsSuperType() const;

            /**
             * Sets the context @p component of the simple type.
             *
             * @see <a href="http://www.w3.org/TR/xmlschema11-1/#std-context">Context Definition</a>
             */
            void setContext(const NamedSchemaComponent::Ptr &component);

            /**
             * Returns the context component of the simple type.
             */
            NamedSchemaComponent::Ptr context() const;

            /**
             * Sets the primitive @p type of the simple type.
             *
             * The primitive type is only specified if the category is SimpleTypeAtomic.
             *
             * @see <a href="http://www.w3.org/TR/xmlschema-2/#defn-primitive">Primitive Type Definition</a>
             */
            void setPrimitiveType(const AnySimpleType::Ptr &type);

            /**
             * Returns the primitive type of the simple type or an empty pointer if the category is
             * not SimpleTypeAtomic.
             */
            AnySimpleType::Ptr primitiveType() const;

            /**
             * Sets the list item @p type of the simple type.
             *
             * The list item type is only specified if the category is SimpleTypeList.
             *
             * @see <a href="http://www.w3.org/TR/xmlschema-2/#defn-itemType">Item Type Definition</a>
             */
            void setItemType(const AnySimpleType::Ptr &type);

            /**
             * Returns the list item type of the simple type or an empty pointer if the category is
             * not SimpleTypeList.
             */
            AnySimpleType::Ptr itemType() const;

            /**
             * Sets the member @p types of the simple type.
             *
             * The member types are only specified if the category is SimpleTypeUnion.
             *
             * @see <a href="http://www.w3.org/TR/xmlschema-2/#defn-memberTypes">Member Types Definition</a>
             */
            void setMemberTypes(const AnySimpleType::List &types);

            /**
             * Returns the list member types of the simple type or an empty list if the category is
             * not SimpleTypeUnion.
             */
            AnySimpleType::List memberTypes() const;

            /**
             * Sets the @p facets of the simple type.
             *
             * @see <a href="http://www.w3.org/TR/xmlschema-2/#defn-facets">Facets Definition</a>
             */
            void setFacets(const XsdFacet::Hash &facets);

            /**
             * Returns the facets of the simple type.
             */
            XsdFacet::Hash facets() const;

            /**
             * Sets the @p category (variety) of the simple type.
             *
             * @see <a href="http://www.w3.org/TR/xmlschema-2/#defn-variety">Variety Definition</a>
             */
            void setCategory(TypeCategory category);

            /**
             * Returns the category (variety) of the simple type.
             */
            virtual TypeCategory category() const;

            /**
             * Sets the derivation @p method of the simple type.
             *
             * @see DerivationMethod
             */
            void setDerivationMethod(DerivationMethod method);

            /**
             * Returns the derivation method of the simple type.
             */
            virtual DerivationMethod derivationMethod() const;

            /**
             * Always returns @c true.
             */
            virtual bool isDefinedBySchema() const;

        private:
            SchemaType::Ptr           m_superType;
            NamedSchemaComponent::Ptr m_context;
            AnySimpleType::Ptr        m_primitiveType;
            AnySimpleType::Ptr        m_itemType;
            AnySimpleType::List       m_memberTypes;
            XsdFacet::Hash            m_facets;
            TypeCategory              m_typeCategory;
            DerivationMethod          m_derivationMethod;
    };
}

QT_END_NAMESPACE

QT_END_HEADER

#endif
