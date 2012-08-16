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

#ifndef Patternist_UnlimitedContainer_H
#define Patternist_UnlimitedContainer_H

#include <QList>
#include "qexpression_p.h"
#include "qgenericsequencetype_p.h"

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

namespace QPatternist
{
    /**
     * @short Base class for expressions that has any amount of operands.
     *
     * @author Frans Englich <frans.englich@nokia.com>
     * @ingroup Patternist_expressions
     */
    class UnlimitedContainer : public Expression
    {
    public:
        /**
         * Creates an UnlimitedContainer containing the operands @p operands. @p operands
         * may be empty.
         */
        UnlimitedContainer(const Expression::List &operands = Expression::List());

        virtual void setOperands(const Expression::List &list);

        virtual Expression::List operands() const;

        /**
         * @note This function cannot be called before setOperands is called.
         */
        virtual bool compressOperands(const StaticContext::Ptr &);

        /**
         * Tells how operandsUnionType() should compute the cardinality of
         * its children.
         *
         * This type is public because of a bug in the HP-UX aCC compiler.
         */
        enum CardinalityComputation
        {
            ProductOfCardinality,
            UnionOfCardinality
        };

    protected:
        /**
         * Computes and returns the union type of all the Expression instances
         * in this Expression's operands.
         *
         * This implementation is placed inside because CardinalityComputation
         * can't be referenced from the outside(in conforming compilers).
         */
        template<CardinalityComputation suppliedCard>
        inline
        SequenceType::Ptr operandsUnionType() const
        {
            Q_ASSERT(suppliedCard ==  ProductOfCardinality || suppliedCard == UnionOfCardinality);
            const Expression::List::const_iterator end(m_operands.constEnd());
            Expression::List::const_iterator it(m_operands.constBegin());

            /* Load the first one, and jump over it in the loop. */
            SequenceType::Ptr t(m_operands.first()->staticType());
            ItemType::Ptr type(t->itemType());
            Cardinality card(t->cardinality());
            ++it;

            for(; it != end; ++it)
            {
                t = (*it)->staticType();
                type |= t->itemType();

                /* Since this function is a template function, it doesn't
                 * hurt performance that this test is inside the loop. */
                if(suppliedCard == ProductOfCardinality)
                    card += t->cardinality();
                else
                    card |= t->cardinality();
            }

            return makeGenericSequenceType(type, card);
        }

        Expression::List m_operands;
    };


}

QT_END_NAMESPACE

QT_END_HEADER

#endif
