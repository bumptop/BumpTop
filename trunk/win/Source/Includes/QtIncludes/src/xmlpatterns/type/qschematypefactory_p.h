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

#ifndef Patternist_SchemaTypeFactory_H
#define Patternist_SchemaTypeFactory_H

#include <QSharedData>

#include "qreportcontext_p.h"
#include "qitemtype_p.h"
#include "qschematype_p.h"

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

namespace QPatternist
{
    /**
     * @short A factory creating schema types.
     *
     * @ingroup Patternist_types
     * @author Frans Englich <frans.englich@nokia.com>
     */
    class SchemaTypeFactory : public QSharedData
    {
    public:
        typedef QExplicitlySharedDataPointer<SchemaTypeFactory> Ptr;

        SchemaTypeFactory();
        virtual ~SchemaTypeFactory();

        /**
         * @returns a schema type for name @p name. If no schema type exists for @p name, @c null
         * is returned
         */
        virtual SchemaType::Ptr createSchemaType(const QXmlName name) const = 0;

        /**
         * @returns a dictionary containing the types this factory serves. The key
         * is the type's QName in Clark name syntax.
         */
        virtual SchemaType::Hash types() const = 0;

    private:
        Q_DISABLE_COPY(SchemaTypeFactory)
    };
}

QT_END_NAMESPACE

QT_END_HEADER

#endif
