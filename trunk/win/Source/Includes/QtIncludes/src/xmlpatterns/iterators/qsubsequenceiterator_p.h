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

#ifndef Patternist_SubsequenceIterator_H
#define Patternist_SubsequenceIterator_H

#include "qitem_p.h"

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

namespace QPatternist
{
    /**
     * @short Picks out a slice from another QAbstractXmlForwardIterator, specified by a start and end position.
     *
     * SubsequenceIterator allows a "slice", a subsequence, from an QAbstractXmlForwardIterator to
     * be extracted. The SubsequenceIterator's constructor takes a source QAbstractXmlForwardIterator,
     * a start position, and the length of the subsequence to be extracted.
     *
     * SubsequenceIterator contains the central business logic to implement
     * the <tt>fn:subsequence()</tt> function. The detailed behavior, such as how it behaves
     * if the source QAbstractXmlForwardIterator is empty or if the specified subsequence stretches
     * beyond the source QAbstractXmlForwardIterator, is therefore consistent with the definition of
     * the <tt>fn:subsequence()</tt> function.
     *
     * @see <a href="http://www.w3.org/TR/xpath-functions/#func-subsequence">XQuery 1.0
     * and XPath 2.0 Functions and Operators, 15.1.10 fn:subsequence</a>
     * @author Frans Englich <frans.englich@nokia.com>
     * @ingroup Patternist_iterators
     */
    class SubsequenceIterator : public Item::Iterator
    {
    public:
        /**
         * Creates a SubsequenceIterator that extracts a subsequence from the sequence
         * in @p iterator, as specified by the @p start position and @p length parameter.
         *
         * @param iterator the iterator which the subsequence should
         * be extracted from
         * @param start the start position of extraction. Must be 1 or larger.
         * @param length the length of the subsequence to extract. If it is
         * -1, to the end is returned. The value must be -1 or 1 or larger.
         */
        SubsequenceIterator(const Item::Iterator::Ptr &iterator,
                            const xsInteger start,
                            const xsInteger length);

        virtual Item next();
        virtual Item current() const;
        virtual xsInteger position() const;
        virtual Item::Iterator::Ptr copy() const;

    private:
        xsInteger m_position;
        Item m_current;
        const Item::Iterator::Ptr m_it;
        xsInteger m_counter;
        const xsInteger m_start;
        const xsInteger m_len;
        const xsInteger m_stop;
    };
}

QT_END_NAMESPACE

QT_END_HEADER

#endif
