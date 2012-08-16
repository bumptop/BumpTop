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

#ifndef Patternist_XsdSchemaDebugger_H
#define Patternist_XsdSchemaDebugger_H

#include "qxsdschema_p.h"

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

namespace QPatternist
{
    /**
     * A helper class to print out the structure of a compiled schema.
     */
    class XsdSchemaDebugger
    {
        public:
            /**
             * Creates a new schema debugger.
             *
             * @param namePool The name pool that the schema uses.
             */
            XsdSchemaDebugger(const NamePool::Ptr &namePool);

            /**
             * Dumps the structure of the given @p particle.
             *
             * @param particle The particle to dump.
             * @param level The level of indention.
             */
            void dumpParticle(const XsdParticle::Ptr &particle, int level = 0);

            /**
             * Dumps the inheritance path of the given @p type.
             *
             * @param type The type to dump.
             * @param level The level of indention.
             */
            void dumpInheritance(const SchemaType::Ptr &type, int level = 0);

            /**
             * Dumps the structure of the given @p wildcard.
             */
            void dumpWildcard(const XsdWildcard::Ptr &wildcard);

            /**
             * Dumps the structure of the given @p type.
             */
            void dumpType(const SchemaType::Ptr &type);

            /**
             * Dumps the structure of the given @p element.
             */
            void dumpElement(const XsdElement::Ptr &element);

            /**
             * Dumps the structure of the given @p attribute.
             */
            void dumpAttribute(const XsdAttribute::Ptr &attribute);

            /**
             * Dumps the structure of the complete @p schema.
             */
            void dumpSchema(const XsdSchema::Ptr &schema);

        private:
            const NamePool::Ptr m_namePool;
    };

}

QT_END_NAMESPACE

QT_END_HEADER

#endif
