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

#ifndef Patternist_XsdSchema_H
#define Patternist_XsdSchema_H

#include "qschematype_p.h"
#include "qxsdannotated_p.h"
#include "qxsdattribute_p.h"
#include "qxsdattributegroup_p.h"
#include "qxsdcomplextype_p.h"
#include "qxsdelement_p.h"
#include "qxsdidentityconstraint_p.h"
#include "qxsdmodelgroup_p.h"
#include "qxsdnotation_p.h"
#include "qxsdsimpletype_p.h"

#include <QtCore/QHash>
#include <QtCore/QReadWriteLock>

/**
 * @defgroup Patternist_schema XML Schema Processing
 */

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

namespace QPatternist
{
    /**
     * @short Represents a XSD schema object.
     *
     * The class provides access to all components of a parsed XSD.
     *
     * @note In the documentation of this class objects, which are direct
     *       children of the <em>schema</em> object, are called top-level objects.
     *
     * @see <a href="http://www.w3.org/Submission/2004/SUBM-xmlschema-api-20040309/xml-schema-api.html#Interface-XSModel">XML Schema API reference</a>
     * @ingroup Patternist_schema
     * @author Tobias Koenig <tobias.koenig@nokia.com>
     */
    class XsdSchema : public QSharedData, public XsdAnnotated
    {
        public:
            typedef QExplicitlySharedDataPointer<XsdSchema> Ptr;
            typedef QList<XsdSchema::Ptr> List;

            /**
             * Creates a new schema object.
             *
             * @param namePool The namepool that should be used for names of
             *                 all schema components.
             */
            XsdSchema(const NamePool::Ptr &namePool);

            /**
             * Destroys the schema object.
             */
            ~XsdSchema();

            /**
             * Returns the namepool that is used for names of
             * all schema components.
             */
            NamePool::Ptr namePool() const;

            /**
             * Sets the @p targetNamespace of the schema.
             */
            void setTargetNamespace(const QString &targetNamespace);

            /**
             * Returns the target namespace of the schema.
             */
            QString targetNamespace() const;

            /**
             * Adds a new top-level @p element to the schema.
             *
             * @param element The new element.
             * @see <a href="http://www.w3.org/TR/xmlschema11-1/#declare-element">Element Declaration</a>
             */
            void addElement(const XsdElement::Ptr &element);

            /**
             * Returns the top-level element of the schema with
             * the given @p name or an empty pointer if none exist.
             */
            XsdElement::Ptr element(const QXmlName &name) const;

            /**
             * Returns the list of all top-level elements.
             */
            XsdElement::List elements() const;

            /**
             * Adds a new top-level @p attribute to the schema.
             *
             * @param attribute The new attribute.
             * @see <a href="http://www.w3.org/TR/xmlschema11-1/#declare-attribute">Attribute Declaration</a>
             */
            void addAttribute(const XsdAttribute::Ptr &attribute);

            /**
             * Returns the top-level attribute of the schema with
             * the given @p name or an empty pointer if none exist.
             */
            XsdAttribute::Ptr attribute(const QXmlName &name) const;

            /**
             * Returns the list of all top-level attributes.
             */
            XsdAttribute::List attributes() const;

            /**
             * Adds a new top-level @p type to the schema.
             * That can be a simple or a complex type.
             *
             * @param type The new type.
             * @see <a href="http://www.w3.org/TR/xmlschema11-1/#declare-datatype">Simple Type Declaration</a>
             * @see <a href="http://www.w3.org/TR/xmlschema11-1/#declare-type">Complex Type Declaration</a>
             */
            void addType(const SchemaType::Ptr &type);

            /**
             * Returns the top-level type of the schema with
             * the given @p name or an empty pointer if none exist.
             */
            SchemaType::Ptr type(const QXmlName &name) const;

            /**
             * Returns the list of all top-level types.
             */
            SchemaType::List types() const;

            /**
             * Returns the list of all top-level simple types.
             */
            XsdSimpleType::List simpleTypes() const;

            /**
             * Returns the list of all top-level complex types.
             */
            XsdComplexType::List complexTypes() const;

            /**
             * Adds an anonymous @p type to the schema.
             * Anonymous types have no name and are declared
             * locally inside an element object.
             *
             * @param type The new anonymous type.
             */
            void addAnonymousType(const SchemaType::Ptr &type);

            /**
             * Returns the list of all anonymous types.
             */
            SchemaType::List anonymousTypes() const;

            /**
             * Adds a new top-level attribute @p group to the schema.
             *
             * @param group The new attribute group.
             * @see <a href="http://www.w3.org/TR/xmlschema11-1/#declare-attributeGroup">Attribute Group Declaration</a>
             */
            void addAttributeGroup(const XsdAttributeGroup::Ptr &group);

            /**
             * Returns the top-level attribute group of the schema with
             * the given @p name or an empty pointer if none exist.
             */
            XsdAttributeGroup::Ptr attributeGroup(const QXmlName name) const;

            /**
             * Returns the list of all top-level attribute groups.
             */
            XsdAttributeGroup::List attributeGroups() const;

            /**
             * Adds a new top-level element @p group to the schema.
             *
             * @param group The new element group.
             * @see <a href="http://www.w3.org/TR/xmlschema11-1/#declare-namedModelGroup">Element Group Declaration</a>
             */
            void addElementGroup(const XsdModelGroup::Ptr &group);

            /**
             * Returns the top-level element group of the schema with
             * the given @p name or an empty pointer if none exist.
             */
            XsdModelGroup::Ptr elementGroup(const QXmlName &name) const;

            /**
             * Returns the list of all top-level element groups.
             */
            XsdModelGroup::List elementGroups() const;

            /**
             * Adds a new top-level @p notation to the schema.
             *
             * @param notation The new notation.
             * @see <a href="http://www.w3.org/TR/xmlschema11-1/#declare-notation">Notation Declaration</a>
             */
            void addNotation(const XsdNotation::Ptr &notation);

            /**
             * Returns the top-level notation of the schema with
             * the given @p name or an empty pointer if none exist.
             */
            XsdNotation::Ptr notation(const QXmlName &name) const;

            /**
             * Returns the list of all top-level notations.
             */
            XsdNotation::List notations() const;

            /**
             * Adds a new identity @p constraint to the schema.
             */
            void addIdentityConstraint(const XsdIdentityConstraint::Ptr &constraint);

            /**
             * Returns the identity constraint with the given @p name
             * or an empty pointer if none exist.
             */
            XsdIdentityConstraint::Ptr identityConstraint(const QXmlName &name) const;

            /**
             * Returns the list of all identity constraints in this schema.
             */
            XsdIdentityConstraint::List identityConstraints() const;

        private:
            NamePool::Ptr                               m_namePool;
            QString                                     m_targetNamespace;
            QHash<QXmlName, XsdElement::Ptr>            m_elements;
            QHash<QXmlName, XsdAttribute::Ptr>          m_attributes;
            QHash<QXmlName, SchemaType::Ptr>            m_types;
            QHash<QXmlName, SchemaType::Ptr>            m_anonymousTypes;
            QHash<QXmlName, XsdAttributeGroup::Ptr>     m_attributeGroups;
            QHash<QXmlName, XsdModelGroup::Ptr>         m_elementGroups;
            QHash<QXmlName, XsdNotation::Ptr>           m_notations;
            QHash<QXmlName, XsdIdentityConstraint::Ptr> m_identityConstraints;
            mutable QReadWriteLock                      m_lock;
    };
}

QT_END_NAMESPACE

QT_END_HEADER

#endif
