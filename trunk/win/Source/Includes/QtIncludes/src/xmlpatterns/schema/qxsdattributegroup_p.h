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

#ifndef Patternist_XsdAttributeGroup_H
#define Patternist_XsdAttributeGroup_H

#include "qxsdannotated_p.h"
#include "qxsdattributeuse_p.h"
#include "qxsdwildcard_p.h"

#include <QtCore/QList>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

namespace QPatternist
{
    /**
     * @short Represents the XSD attributeGroup object.
     *
     * This class represents the <em>attributeGroup</em> object of a XML schema as described
     * <a href="http://www.w3.org/TR/xmlschema11-1/#cAttribute_Group_Definitions">here</a>.
     *
     * @see <a href="http://www.w3.org/Submission/2004/SUBM-xmlschema-api-20040309/xml-schema-api.html#Interface-XSAttributeGroup">XML Schema API reference</a>
     * @ingroup Patternist_schema
     * @author Tobias Koenig <tobias.koenig@nokia.com>
     */
    class XsdAttributeGroup : public NamedSchemaComponent, public XsdAnnotated
    {
        public:
            typedef QExplicitlySharedDataPointer<XsdAttributeGroup> Ptr;
            typedef QList<XsdAttributeGroup::Ptr> List;

            /**
             * Sets the list of attribute @p uses that are defined in the attribute group.
             *
             * @see <a href="http://www.w3.org/TR/xmlschema11-1/#agd-attribute_uses">Attribute Uses</a>
             */
            void setAttributeUses(const XsdAttributeUse::List &uses);

            /**
             * Adds a new attribute @p use to the attribute group.
             */
            void addAttributeUse(const XsdAttributeUse::Ptr &use);

            /**
             * Returns the list of all attribute uses of the attribute group.
             */
            XsdAttributeUse::List attributeUses() const;

            /**
             * Sets the attribute @p wildcard of the attribute group.
             *
             * @see <a href="http://www.w3.org/TR/xmlschema11-1/#agd-attribute_wildcard">Attribute Wildcard</a>
             */
            void setWildcard(const XsdWildcard::Ptr &wildcard);

            /**
             * Returns the attribute wildcard of the attribute group.
             */
            XsdWildcard::Ptr wildcard() const;

        private:
            XsdAttributeUse::List m_attributeUses;
            XsdWildcard::Ptr      m_wildcard;
    };
}

QT_END_NAMESPACE

QT_END_HEADER

#endif
