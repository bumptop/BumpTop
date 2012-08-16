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

#ifndef Patternist_NamespaceSupport_H
#define Patternist_NamespaceSupport_H

#include "qnamepool_p.h"

#include <QtCore/QExplicitlySharedDataPointer>
#include <QtCore/QHash>
#include <QtCore/QSet>
#include <QtCore/QStack>
#include <QtCore/QXmlStreamNamespaceDeclarations>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

namespace QPatternist
{
    /**
     * @short A helper class for handling nested namespace declarations.
     *
     * This class is mostly an adaption of QXmlNamespaceSupport to the NamePool
     * mechanism used in XmlPatterns.
     *
     * @ingroup Patternist_schema
     * @author Tobias Koenig <tobias.koenig@nokia.com>
     */
    class NamespaceSupport
    {
        public:
            /**
             * Describes whether the name to process is an attribute or element.
             */
            enum NameType
            {
                AttributeName,  ///< An attribute name to process.
                ElementName     ///< An element name to process.
            };

            /**
             * Creates an empty namespace support object.
             */
            NamespaceSupport();

            /**
             * Creates a new namespace support object.
             *
             * @param namePool The name pool where all processed names are stored in.
             */
            NamespaceSupport(const NamePool::Ptr &namePool);

            /**
             * Adds a new prefix-to-namespace binding.
             *
             * @param prefixCode The name pool code for the prefix.
             * @param namespaceCode The name pool code for the namespace.
             */
            void setPrefix(const QXmlName::PrefixCode prefixCode, const QXmlName::NamespaceCode namespaceCode);

            /**
             * Adds the prefix-to-namespace bindings from @p declarations to
             * the namespace support.
             */
            void setPrefixes(const QXmlStreamNamespaceDeclarations &declarations);

            /**
             * Sets the name pool code of the target namespace of the schema the
             * namespace support works on.
             */
            void setTargetNamespace(const QXmlName::NamespaceCode code);

            /**
             * Returns the prefix code for the given namespace @p code.
             */
            QXmlName::PrefixCode prefix(const QXmlName::NamespaceCode code) const;

            /**
             * Returns the namespace code for the given prefix @p code.
             */
            QXmlName::NamespaceCode uri(const QXmlName::PrefixCode code) const;

            /**
             * Converts the given @p qualifiedName to a resolved QXmlName @p name according
             * to the current namespace mapping.
             *
             * @param qualifiedName The full qualified name.
             * @param type The type of name processing.
             * @param name The resolved QXmlName.
             *
             * @returns @c true if the name could be processed correctly or @c false if the
             *          namespace prefix is unknown.
             */
            bool processName(const QString &qualifiedName, NameType type, QXmlName &name) const;

            /**
             * Pushes the current namespace mapping onto the stack.
             */
            void pushContext();

            /**
             * Pops the current namespace mapping from the stack.
             */
            void popContext();

            /**
             * Returns the list of namespace bindings.
             */
            QList<QXmlName> namespaceBindings() const;

        private:
            typedef QHash<QXmlName::PrefixCode, QXmlName::NamespaceCode> NamespaceHash;

            NamePool::Ptr         m_namePool;
            QStack<NamespaceHash> m_nsStack;
            NamespaceHash         m_ns;
    };
}

QT_END_NAMESPACE

QT_END_HEADER

#endif
