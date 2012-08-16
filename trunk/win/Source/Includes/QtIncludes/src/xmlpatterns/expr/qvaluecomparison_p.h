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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#ifndef Patternist_ValueComparison_H
#define Patternist_ValueComparison_H

#include "qatomiccomparator_p.h"
#include "qpaircontainer_p.h"
#include "qcomparisonplatform_p.h"

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

namespace QPatternist
{

    /**
     * @short Implements XPath 2.0 value comparions, such as the <tt>eq</tt> operator.
     *
     * ComparisonPlatform is inherited with @c protected scope because ComparisonPlatform
     * must access members of ValueComparison.
     *
     * @see <a href="http://www.w3.org/TR/xpath20/#id-value-comparisons">XML Path Language
     * (XPath) 2.0, 3.5.1 Value Comparisons</a>
     * @author Frans Englich <frans.englich@nokia.com>
     * @ingroup Patternist_expressions
     */
    class ValueComparison : public PairContainer,
                            public ComparisonPlatform<ValueComparison, true>
    {
    public:
        ValueComparison(const Expression::Ptr &op1,
                        const AtomicComparator::Operator op,
                        const Expression::Ptr &op2);

        virtual Item evaluateSingleton(const DynamicContext::Ptr &) const;

        virtual Expression::Ptr typeCheck(const StaticContext::Ptr &context,
                                          const SequenceType::Ptr &reqType);

        /**
         * @returns always CommonSequenceTypes::ExactlyOneBoolean
         */
        virtual SequenceType::Ptr staticType() const;

        virtual SequenceType::List expectedOperandTypes() const;

        /**
         * @returns IDValueComparison
         */
        virtual ID id() const;

        virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;
        virtual QList<QExplicitlySharedDataPointer<OptimizationPass> > optimizationPasses() const;

        /**
         * Overridden to optimize case-insensitive compares.
         */
        virtual Expression::Ptr compress(const StaticContext::Ptr &context);

        /**
         * @returns the operator that this ValueComparison is using.
         */
        inline AtomicComparator::Operator operatorID() const
        {
            return m_operator;
        }

        /**
         * It is considered that the string value from @p op1 will be compared against @p op2. This
         * function determines whether the user intends the comparison to be case insensitive. If
         * that is the case @c true is returned, and the operands are re-written appropriately.
         *
         * This is a helper function for Expression classes that compares strings.
         *
         * @see ComparisonPlatform::useCaseInsensitiveComparator()
         */
        static bool isCaseInsensitiveCompare(Expression::Ptr &op1, Expression::Ptr &op2);

    private:
        const AtomicComparator::Operator m_operator;
    };
}

QT_END_NAMESPACE

QT_END_HEADER

#endif
