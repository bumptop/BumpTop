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

#ifndef Patternist_InsertionIterator_H
#define Patternist_InsertionIterator_H

#include "qabstractxmlforwarditerator_p.h"
#include "qitem_p.h"

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

namespace QPatternist
{
    /**
     * @short Conceptually inserts one QAbstractXmlForwardIterator into another, make two QAbstractXmlForwardIterator instances appear as one.
     *
     * An InsertionIterator represents a sequence that is the merge of two
     * sequences, where one of the iterators is conceptually inserted at a
     * given position. This is done while retaining the characteristic of being
     * pull-based.
     *
     * InsertionIterator contains the logic for the implementation of the <tt>fn:insert-before()</tt>
     * function, whose definition therefore specifies the detailed behaviors of the
     * InsertionIterator.
     *
     * @see <a href="http://www.w3.org/TR/xpath-functions/#func-insert-before">XQuery 1.0
     * and XPath 2.0 Functions and Operators, 15.1.7 fn:insert-before</a>
     * @author Frans Englich <frans.englich@nokia.com>
     * @ingroup Patternist_iterators
     */
    class InsertionIterator : public Item::Iterator
    {
    public:

        /**
         * Creates an InsertionIterator whose result is a merge of the
         * iterator @p insertIterator into the iterator @p target at position @p
         * position.
         *
         * @param target the iterator containing the items that the
         * item in @p insertIterator will be inserted into.
         * @param position the insertion position. Must be 1 or larger
         * @param insertIterator the iterator containing the items to insert
         * at position @p position
         */
        InsertionIterator(const Item::Iterator::Ptr &target,
                          const xsInteger position,
                          const Item::Iterator::Ptr &insertIterator);

        virtual Item next();
        virtual Item current() const;
        virtual xsInteger position() const;
        virtual xsInteger count();
        virtual Item::Iterator::Ptr copy() const;

    private:
        const Item::Iterator::Ptr m_target;
        const xsInteger m_insertPos;
        const Item::Iterator::Ptr m_inserts;
        Item m_current;
        xsInteger m_position;
        bool m_isInserting;
    };
}

QT_END_NAMESPACE

QT_END_HEADER

#endif
