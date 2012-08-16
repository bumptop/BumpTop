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

#ifndef Patternist_RemovalIterator_H
#define Patternist_RemovalIterator_H

#include "qitem_p.h"

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

namespace QPatternist
{

    /**
     * @short Removes one items at a specified position from an input QAbstractXmlForwardIterator.
     *
     * RemoveIterator removes an item from a sequence at a certain position,
     * while retaining the pull-based characteristic of being an QAbstractXmlForwardIterator itself. The
     * RemovalIterator's constructor is passed an QAbstractXmlForwardIterator, the QAbstractXmlForwardIterator to
     * remove from, and the position of the item to remove. When calling the RemovalIterator's
     * functions, it acts as an ordinary QAbstractXmlForwardIterator, taking into account that
     * one item is removed from the source QAbstractXmlForwardIterator.
     *
     * The RemovalIterator class contains the central business logic for implementing the
     * <tt>fn:remove()</tt> function, whose definition therefore specifies the detailed behaviors
     * of RemovalIterator.
     *
     * @see <a href="http://www.w3.org/TR/xpath-functions/#func-remove">XQuery 1.0
     * and XPath 2.0 Functions and Operators, 15.1.8 fn:remove</a>
     * @author Frans Englich <frans.englich@nokia.com>
     * @ingroup Patternist_iterators
     */
    class RemovalIterator : public Item::Iterator
    {
    public:

        /**
         * Creates an RemovalIterator.
         *
         * @param target the QAbstractXmlForwardIterator containing the sequence of items
         * which the item at position @p position should be removed from.
         * @param position the position of the item to remove. Must be
         * 1 or larger.
         */
        RemovalIterator(const Item::Iterator::Ptr &target,
                        const xsInteger position);

        virtual Item next();
        virtual Item current() const;
        virtual xsInteger position() const;

        /**
         * The QAbstractXmlForwardIterator's count is computed by subtracting one from the source
         * QAbstractXmlForwardIterator's count.
         */
        virtual xsInteger count();

        virtual Item::Iterator::Ptr copy() const;

    private:
        const Item::Iterator::Ptr m_target;
        const xsInteger m_removalPos;
        Item m_current;
        xsInteger m_position;
    };
}

QT_END_NAMESPACE

QT_END_HEADER

#endif
