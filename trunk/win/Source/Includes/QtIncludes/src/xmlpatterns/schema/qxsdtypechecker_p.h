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

#ifndef Patternist_XsdTypeChecker_H
#define Patternist_XsdTypeChecker_H

#include <QtXmlPatterns/QSourceLocation>

#include "qschematype_p.h"
#include "qsourcelocationreflection_p.h"
#include "qxsdschemacontext_p.h"

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class QXmlQuery;

namespace QPatternist
{
    /**
     * @short An implementation of SourceLocationReflection that takes a QSourceLocation.
     *
     * This is a convenience class which provides a QSourceLocation with a SourceLocationReflection
     * interface.
     */
    class XsdSchemaSourceLocationReflection : public SourceLocationReflection
    {
        public:
            XsdSchemaSourceLocationReflection(const QSourceLocation &location);

            virtual const SourceLocationReflection *actualReflection() const;
            virtual QSourceLocation sourceLocation() const;

        private:
            const QSourceLocation m_sourceLocation;
    };

    /**
     * @short The class that provides methods for checking a string against a type.
     *
     * The class provides functionality for type-aware string handling.
     */
    class XsdTypeChecker
    {
        public:
            /**
             * Creates a new type checker.
             *
             * @param context The schema context that is used for error reporting.
             * @param namespaceBindings The namespace bindings that shall be used to check against xs:QName based types.
             * @param location The source location that is used for error reporting.
             */
            XsdTypeChecker(const XsdSchemaContext::Ptr &context, const QVector<QXmlName> &namespaceBindings, const QSourceLocation &location);

            /**
             * Destroys the type checker.
             */
            ~XsdTypeChecker();

            /**
             * Returns all facets for the given @p type.
             *
             * The list of facets is created by following the type hierarchy from xs:anyType down to the given type
             * and merging the facets in each step.
             */
            static XsdFacet::Hash mergedFacetsForType(const SchemaType::Ptr &type, const XsdSchemaContext::Ptr &context);

            /**
             * Returns the normalized value for the given @p value.
             *
             * The normalized value is the original value with all the white space facets
             * applied on it.
             *
             * @param value The original value.
             * @param facets The hash of all facets of the values type.
             */
            static QString normalizedValue(const QString &value, const XsdFacet::Hash &facets);

            /**
             * Checks whether the @p normalizedString is valid according the given @p type.
             *
             * @param normalizedString The string in normalized form (whitespace facets applied).
             * @param type The type the string shall be tested against.
             * @param errorMsg Contains the error message if the normalizedString does not match the type.
             * @param boundType The type the data was bound to during validation.
             *
             * @note The @p boundType only differs from @p type if the type is derived from an based union value.
             */
            bool isValidString(const QString &normalizedString, const AnySimpleType::Ptr &type, QString &errorMsg, AnySimpleType::Ptr *boundType = 0) const;

            /**
             * Returns whether the given @p value and @p otherValue are of @p type and are equal.
             */
            bool valuesAreEqual(const QString &value, const QString &otherValue, const AnySimpleType::Ptr &type) const;

        private:
            Q_DISABLE_COPY(XsdTypeChecker)

            /**
             * Checks the given value against the facets of @p type.
             */
            bool checkConstrainingFacets(const AtomicValue::Ptr &value, const QString &lexicalValue, const AnySimpleType::Ptr &type, QString &errorMsg) const;
            bool checkConstrainingFacetsString(const QString &value, const XsdFacet::Hash &facets, const AnySimpleType::Ptr &type, QString &errorMsg) const;
            bool checkConstrainingFacetsSignedInteger(long long value, const QString &lexicalValue, const XsdFacet::Hash &facets, QString &errorMsg) const;
            bool checkConstrainingFacetsUnsignedInteger(unsigned long long value, const QString &lexicalValue, const XsdFacet::Hash &facets, QString &errorMsg) const;
            bool checkConstrainingFacetsDouble(double value, const QString &lexicalValue, const XsdFacet::Hash &facets, QString &errorMsg) const;
            bool checkConstrainingFacetsDecimal(const AtomicValue::Ptr &value, const QString &lexicalValue, const XsdFacet::Hash &facets, QString &errorMsg) const;
            bool checkConstrainingFacetsDateTime(const QDateTime &value, const QString &lexicalValue, const XsdFacet::Hash &facets, const AnySimpleType::Ptr &type, QString &errorMsg) const;
            bool checkConstrainingFacetsDuration(const AtomicValue::Ptr &value, const QString &lexicalValue, const XsdFacet::Hash &facets, QString &errorMsg) const;
            bool checkConstrainingFacetsBoolean(bool value, const QString &lexicalValue, const XsdFacet::Hash &facets, QString &errorMsg) const;
            bool checkConstrainingFacetsBinary(const QByteArray &value, const XsdFacet::Hash &facets, const AnySimpleType::Ptr &type, QString &errorMsg) const;
            bool checkConstrainingFacetsQName(const QXmlName&, const QString &lexicalValue, const XsdFacet::Hash &facets, QString &errorMsg) const;
            bool checkConstrainingFacetsNotation(const QXmlName &value, const XsdFacet::Hash &facets, QString &errorMsg) const;
            bool checkConstrainingFacetsList(const QStringList &values, const QString &lexicalValue, const AnySimpleType::Ptr &itemType, const XsdFacet::Hash &facets, QString &errorMsg) const;
            bool checkConstrainingFacetsUnion(const QString &value, const QString &lexicalValue, const XsdSimpleType::Ptr &simpleType, const XsdFacet::Hash &facets, QString &errorMsg) const;

            /**
             * Creates an atomic value of @p type from the given string @p value.
             */
            AtomicValue::Ptr fromLexical(const QString &value, const SchemaType::Ptr &type, const ReportContext::Ptr &context, const SourceLocationReflection *const reflection) const;

            /**
             * Converts a qualified name into a QXmlName according to the namespace
             * mappings of the current node.
             */
            QXmlName convertToQName(const QString &name) const;

            XsdSchemaContext::Ptr      m_context;
            XsdSchema::Ptr             m_schema;
            const NamePool::Ptr        m_namePool;
            QVector<QXmlName>          m_namespaceBindings;
            SourceLocationReflection*  m_reflection;
    };
}

QT_END_NAMESPACE

QT_END_HEADER

#endif
