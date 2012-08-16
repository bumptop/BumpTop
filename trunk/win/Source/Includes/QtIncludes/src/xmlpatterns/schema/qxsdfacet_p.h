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

#ifndef Patternist_XsdFacet_H
#define Patternist_XsdFacet_H

#include "qitem_p.h"
#include "qnamedschemacomponent_p.h"
#include "qxsdannotated_p.h"
#include "qxsdassertion_p.h"

#include <QtCore/QList>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

namespace QPatternist
{
    /**
     * @short Represents a XSD facet object.
     *
     * This class represents one of the following XML schema objects:
     *
     *  <ul>
     *      <li><em>length</em> <a href="http://www.w3.org/TR/xmlschema-2/#rf-length">Definition</a></li>
     *      <li><em>minLength</em> <a href="http://www.w3.org/TR/xmlschema-2/#rf-minLength">Definition</a></li>
     *      <li><em>maxLength</em> <a href="http://www.w3.org/TR/xmlschema-2/#rf-maxLength">Definition</a></li>
     *      <li><em>pattern</em> <a href="http://www.w3.org/TR/xmlschema-2/#rf-pattern">Definition</a></li>
     *      <li><em>whiteSpace</em> <a href="http://www.w3.org/TR/xmlschema-2/#rf-whiteSpace">Definition</a></li>
     *      <li><em>maxInclusive</em> <a href="http://www.w3.org/TR/xmlschema-2/#rf-maxInclusive">Definition</a></li>
     *      <li><em>maxExclusive</em> <a href="http://www.w3.org/TR/xmlschema-2/#rf-maxExclusive">Definition</a></li>
     *      <li><em>minInclusive</em> <a href="http://www.w3.org/TR/xmlschema-2/#rf-minInclusive">Definition</a></li>
     *      <li><em>minExclusive</em> <a href="http://www.w3.org/TR/xmlschema-2/#rf-minExclusive">Definition</a></li>
     *      <li><em>totalDigits</em> <a href="http://www.w3.org/TR/xmlschema-2/#rf-totalDigits">Definition</a></li>
     *      <li><em>fractionDigits</em> <a href="http://www.w3.org/TR/xmlschema-2/#rf-fractionDigits">Definition</a></li>
     *      <li><em>enumeration</em> <a href="http://www.w3.org/TR/xmlschema-2/#rf-enumeration">Definition</a></li>
     *      <li><em>assertion</em> <a href="http://www.w3.org/TR/xmlschema-2/#rf-assertion">Definition</a></li>
     *  </ul>
     *
     * @see <a href="http://www.w3.org/Submission/2004/SUBM-xmlschema-api-20040309/xml-schema-api.html#Interface-XSFacet">XML Schema API reference</a>
     * @ingroup Patternist_schema
     * @author Tobias Koenig <tobias.koenig@nokia.com>
     */
    class XsdFacet : public NamedSchemaComponent, public XsdAnnotated
    {
        public:
            typedef QExplicitlySharedDataPointer<XsdFacet> Ptr;

            /**
             * Describes the type of the facet.
             */
            enum Type
            {
                None             = 0,        ///< An invalid facet.
                Length           = 1 << 0,   ///< Match the exact length (<a href="http://www.w3.org/TR/xmlschema-2/#rf-length">Length Definition</a>)
                MinimumLength    = 1 << 1,   ///< Match the minimum length (<a href="http://www.w3.org/TR/xmlschema-2/#rf-minLength">Minimum Length Definition</a>)
                MaximumLength    = 1 << 2,   ///< Match the maximum length (<a href="http://www.w3.org/TR/xmlschema-2/#rf-maxLength">Maximum Length Definition</a>)
                Pattern          = 1 << 3,   ///< Match a regular expression (<a href="http://www.w3.org/TR/xmlschema-2/#rf-pattern">Pattern Definition</a>)
                WhiteSpace       = 1 << 4,   ///< Match a whitespace rule (<a href="http://www.w3.org/TR/xmlschema-2/#rf-whiteSpace">White Space Definition</a>)
                MaximumInclusive = 1 << 5,   ///< Match a maximum inclusive (<a href="http://www.w3.org/TR/xmlschema-2/#rf-maxInclusive">Maximum Inclusive Definition</a>)
                MaximumExclusive = 1 << 6,   ///< Match a maximum exclusive (<a href="http://www.w3.org/TR/xmlschema-2/#rf-maxExclusive">Maximum Exclusive Definition</a>)
                MinimumInclusive = 1 << 7,   ///< Match a minimum inclusive (<a href="http://www.w3.org/TR/xmlschema-2/#rf-minInclusive">Minimum Inclusive Definition</a>)
                MinimumExclusive = 1 << 8,   ///< Match a minimum exclusive (<a href="http://www.w3.org/TR/xmlschema-2/#rf-minExclusive">Minimum Exclusive Definition</a>)
                TotalDigits      = 1 << 9,   ///< Match some integer digits (<a href="http://www.w3.org/TR/xmlschema-2/#rf-totalDigits">Total Digits Definition</a>)
                FractionDigits   = 1 << 10,  ///< Match some double digits (<a href="http://www.w3.org/TR/xmlschema-2/#rf-fractionDigits">Fraction Digits Definition</a>)
                Enumeration      = 1 << 11,  ///< Match an enumeration (<a href="http://www.w3.org/TR/xmlschema-2/#rf-enumeration">Enumeration Definition</a>)
                Assertion        = 1 << 12,  ///< Match an assertion (<a href="http://www.w3.org/TR/xmlschema-2/#rf-assertion">Assertion Definition</a>)
            };
            typedef QHash<XsdFacet::Type, XsdFacet::Ptr> Hash;
            typedef QHashIterator<XsdFacet::Type, XsdFacet::Ptr> HashIterator;

            /**
             * Creates a new facet object of type None.
             */
            XsdFacet();

            /**
             * Sets the @p type of the facet.
             *
             * @see Type
             */
            void setType(Type type);

            /**
             * Returns the type of the facet.
             */
            Type type() const;

            /**
             * Sets the @p value of the facet.
             *
             * Depending on the type of the facet the
             * value can be a string, interger, double etc.
             *
             * @note This method should be used for all types of facets
             *       except Pattern, Enumeration and Assertion.
             */
            void setValue(const AtomicValue::Ptr &value);

            /**
             * Returns the value of the facet or an empty pointer if facet
             * type is Pattern, Enumeration or Assertion.
             */
            AtomicValue::Ptr value() const;

            /**
             * Sets the @p value of the facet.
             *
             * @note This method should be used for if the type of the
             *       facet is Pattern or Enumeration.
             */
            void setMultiValue(const AtomicValue::List &value);

            /**
             * Returns the value of the facet or an empty pointer if facet
             * type is not of type Pattern or Enumeration.
             */
            AtomicValue::List multiValue() const;

            /**
             * Sets the @p assertions of the facet.
             *
             * @note This method should be used if the type of the
             *       facet is Assertion.
             */
            void setAssertions(const XsdAssertion::List &assertions);

            /**
             * Returns the assertions of the facet or an empty pointer if facet
             * type is not of type Assertion.
             */
            XsdAssertion::List assertions() const;

            /**
             * Sets whether the facet is @p fixed.
             *
             * All facets except pattern, enumeration and assertion can be fixed.
             */
            void setFixed(bool fixed);

            /**
             * Returns whether the facet is fixed.
             */
            bool fixed() const;

            /**
             * Returns the textual description of the facet @p type.
             */
            static QString typeName(Type type);

        private:
            Type               m_type;
            AtomicValue::Ptr   m_value;
            AtomicValue::List  m_multiValue;
            XsdAssertion::List m_assertions;
            bool               m_fixed;
    };
}

QT_END_NAMESPACE

QT_END_HEADER

#endif
