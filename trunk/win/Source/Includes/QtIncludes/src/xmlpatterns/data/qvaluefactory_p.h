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

#ifndef Patternist_ValueFactory_H
#define Patternist_ValueFactory_H

#include "qitem_p.h"
#include "qreportcontext_p.h"
#include "qschematype_p.h"

QT_BEGIN_HEADER
QT_BEGIN_NAMESPACE

namespace QPatternist
{
    /**
     * @short Provides fromLexical(), which allows instantiation of atomic
     * values from arbitrary types.
     *
     * This class wraps the helper class CastingPlatform with a more specific,
     * high-level API.
     *
     * @see CastingPlatform
     * @author Frans Englich <frans.englich@nokia.com>
     * @ingroup Patternist_schema
     */
    class ValueFactory
    {
    public:
        /**
         * @short Returns an AtomicValue of type @p type from the lexical space
         * @p lexicalValue, and raise an error through @p context if that's
         * impossible.
         *
         * ValueFactory does not take ownership of @p sourceLocationReflection.
         */
        static AtomicValue::Ptr fromLexical(const QString &lexicalValue,
                                            const SchemaType::Ptr &type,
                                            const ReportContext::Ptr &context,
                                            const SourceLocationReflection *const sourceLocationReflection);

    private:
        Q_DISABLE_COPY(ValueFactory)
    };
}

QT_END_NAMESPACE
QT_END_HEADER

#endif
