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

#ifndef Patternist_XsdAttributeUse_H
#define Patternist_XsdAttributeUse_H

#include "qxsdattribute_p.h"
#include "qxsdattributeterm_p.h"

#include <QtCore/QList>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

namespace QPatternist
{
    /**
     * @short Represents a XSD attribute use object.
     *
     * This class represents the <em>attribute use</em> object of a XML schema as described
     * <a href="http://www.w3.org/TR/xmlschema11-1/#cAttributeUse">here</a>.
     *
     * It contains information from a local attribute declaration (as child of <em>complexType</em>
     * or <em>attributeGroup</em> object).
     *
     * All other occurrences of the <em>attribute</em> object are represented by the XsdAttribute class.
     *
     * @see <a href="http://www.w3.org/Submission/2004/SUBM-xmlschema-api-20040309/xml-schema-api.html#Interface-XSAttributeUse">XML Schema API reference</a>
     * @ingroup Patternist_schema
     * @author Tobias Koenig <tobias.koenig@nokia.com>
     */
    class XsdAttributeUse : public XsdAttributeTerm
    {
        public:
            typedef QExplicitlySharedDataPointer<XsdAttributeUse> Ptr;
            typedef QList<XsdAttributeUse::Ptr> List;

            /**
             * Describes the value constraint of an attribute use.
             *
             * @see <a href="http://www.w3.org/TR/xmlschema11-1/#vc_au">Value Constraint Definition</a>
             */
            class ValueConstraint : public QSharedData
            {
                public:
                    typedef QExplicitlySharedDataPointer<ValueConstraint> Ptr;

                    /**
                     * Describes the <a href="http://www.w3.org/TR/xmlschema11-1/#au-value_constraint">value constraint</a> of an attribute use.
                     */
                    enum Variety
                    {
                        Default,  ///< The attribute use has a default value set.
                        Fixed     ///< The attribute use has a fixed value set.
                    };

                    /**
                     * Sets the @p variety of the attribute use value constraint.
                     */
                    void setVariety(Variety variety);

                    /**
                     * Returns the variety of the attribute use value constraint.
                     */
                    Variety variety() const;

                    /**
                     * Sets the @p value of the constraint.
                     */
                    void setValue(const QString &value);

                    /**
                     * Returns the value of the constraint.
                     */
                    QString value() const;

                    /**
                     * Sets the lexical @p form of the constraint.
                     */
                    void setLexicalForm(const QString &form);

                    /**
                     * Returns the lexical form of the constraint.
                     */
                    QString lexicalForm() const;

                    /**
                     * Creates a new value constraint from a XsdAttribute::ValueConstraint.
                     */
                    static ValueConstraint::Ptr fromAttributeValueConstraint(const XsdAttribute::ValueConstraint::Ptr &constraint);

                private:
                    Variety m_variety;
                    QString m_value;
                    QString m_lexicalForm;
            };

            /**
             * Describes the use type of the attribute use.
             */
            enum UseType
            {
                OptionalUse,        ///< The attribute can be there but doesn't need to.
                RequiredUse,        ///< The attribute must be there.
                ProhibitedUse       ///< The attribute is not allowed to be there.
            };

            /**
             * Creates a new attribute use object.
             */
            XsdAttributeUse();

            /**
             * Always returns true, used to avoid dynamic casts.
             */
            virtual bool isAttributeUse() const;

            /**
             * Sets the use @p type of the attribute use.
             *
             * @see UseType
             */
            void setUseType(UseType type);

            /**
             * Returns the use type of the attribute use.
             */
            UseType useType() const;

            /**
             * Returns whether the attribute use is required.
             *
             * @see <a href="http://www.w3.org/TR/xmlschema11-1/#au-required">Required Definition</a>
             */
            bool isRequired() const;

            /**
             * Sets the @p attribute the attribute use is referring to.
             * That is either a local definition as child of a complexType
             * or attributeGroup object, or a reference defined by the
             * 'ref' attribute.
             *
             * @see <a href="http://www.w3.org/TR/xmlschema11-1/#au-attribute_declaration">Attribute Declaration</a>
             */
            void setAttribute(const XsdAttribute::Ptr &attribute);

            /**
             * Returns the attribute the attribute use is referring to.
             */
            XsdAttribute::Ptr attribute() const;

            /**
             * Sets the value @p constraint of the attribute use.
             *
             * @see http://www.w3.org/TR/xmlschema11-1/#vc_au
             */
            void setValueConstraint(const ValueConstraint::Ptr &constraint);

            /**
             * Returns the value constraint of the attribute use.
             */
            ValueConstraint::Ptr valueConstraint() const;

        private:
            UseType              m_useType;
            XsdAttribute::Ptr    m_attribute;
            ValueConstraint::Ptr m_valueConstraint;
    };
}

QT_END_NAMESPACE

QT_END_HEADER

#endif
