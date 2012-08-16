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

#ifndef Patternist_XsdValidatedXmlNodeModel_H
#define Patternist_XsdValidatedXmlNodeModel_H

#include "qabstractxmlnodemodel.h"

#include "qabstractxmlforwarditerator_p.h"
#include "qitem_p.h"
#include "qschematype_p.h"
#include "qxsdelement_p.h"

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

namespace QPatternist
{
    /**
     * @short A delegate class that wraps around a QAbstractXmlNodeModel and provides
     *        additional validation specific information.
     *
     * This class represents the input XML document enriched with additional type
     * information that has been assigned during validation.
     *
     * @ingroup Patternist_schema
     * @author Tobias Koenig <tobias.koenig@nokia.com>
     */
    class XsdValidatedXmlNodeModel : public QAbstractXmlNodeModel
    {
        public:
            typedef QExplicitlySharedDataPointer<XsdValidatedXmlNodeModel> Ptr;
            typedef QList<Ptr> List;

            /**
             * Creates a new validated xml node model.
             */
            XsdValidatedXmlNodeModel(const QAbstractXmlNodeModel *model);

            /**
             * Destroys the validated xml node model.
             */
            virtual ~XsdValidatedXmlNodeModel();

            virtual QUrl baseUri(const QXmlNodeModelIndex &ni) const;
            virtual QUrl documentUri(const QXmlNodeModelIndex &ni) const;
            virtual QXmlNodeModelIndex::NodeKind kind(const QXmlNodeModelIndex &ni) const;
            virtual QXmlNodeModelIndex::DocumentOrder compareOrder(const QXmlNodeModelIndex &ni1, const QXmlNodeModelIndex &ni2) const;
            virtual QXmlNodeModelIndex root(const QXmlNodeModelIndex &n) const;
            virtual QXmlName name(const QXmlNodeModelIndex &ni) const;
            virtual QString stringValue(const QXmlNodeModelIndex &n) const;
            virtual QVariant typedValue(const QXmlNodeModelIndex &n) const;
            virtual QExplicitlySharedDataPointer<QAbstractXmlForwardIterator<QXmlNodeModelIndex> > iterate(const QXmlNodeModelIndex &ni, QXmlNodeModelIndex::Axis axis) const;
            virtual QPatternist::ItemIteratorPtr sequencedTypedValue(const QXmlNodeModelIndex &ni) const;
            virtual QPatternist::ItemTypePtr type(const QXmlNodeModelIndex &ni) const;
            virtual QXmlName::NamespaceCode namespaceForPrefix(const QXmlNodeModelIndex &ni, const QXmlName::PrefixCode prefix) const;
            virtual bool isDeepEqual(const QXmlNodeModelIndex &ni1, const QXmlNodeModelIndex &ni2) const;
            virtual void sendNamespaces(const QXmlNodeModelIndex &n, QAbstractXmlReceiver *const receiver) const;
            virtual QVector<QXmlName> namespaceBindings(const QXmlNodeModelIndex &n) const;
            virtual QXmlNodeModelIndex elementById(const QXmlName &NCName) const;
            virtual QVector<QXmlNodeModelIndex> nodesByIdref(const QXmlName &NCName) const;
            virtual void copyNodeTo(const QXmlNodeModelIndex &node, QAbstractXmlReceiver *const receiver, const NodeCopySettings &) const;

            /**
             * Sets the @p element that is assigned to the xml node at @p index.
             */
            void setAssignedElement(const QXmlNodeModelIndex &index, const XsdElement::Ptr &element);

            /**
             * Returns the element that is assigned to the xml node at @p index.
             */
            XsdElement::Ptr assignedElement(const QXmlNodeModelIndex &index) const;

            /**
             * Sets the @p attribute that is assigned to the xml node at @p index.
             */
            void setAssignedAttribute(const QXmlNodeModelIndex &index, const XsdAttribute::Ptr &attribute);

            /**
             * Returns the attribute that is assigned to the xml node at @p index.
             */
            XsdAttribute::Ptr assignedAttribute(const QXmlNodeModelIndex &index) const;

            /**
             * Sets the @p type that is assigned to the xml node at @p index.
             *
             * @note The type can be a different than the type of the element or
             *       attribute that is assigned to the index, since the instance
             *       document can overwrite it by xsi:type.
             */
            void setAssignedType(const QXmlNodeModelIndex &index, const SchemaType::Ptr &type);

            /**
             * Returns the type that is assigned to the xml node at @p index.
             */
            SchemaType::Ptr assignedType(const QXmlNodeModelIndex &index) const;

            /**
             * Adds the attribute or element @p binding with the given @p id.
             */
            void addIdIdRefBinding(const QString &id, const NamedSchemaComponent::Ptr &binding);

            /**
             * Returns a list of all binding ids.
             */
            QStringList idIdRefBindingIds() const;

            /**
             * Returns the set of bindings with the given @p id.
             */
            QSet<NamedSchemaComponent::Ptr> idIdRefBindings(const QString &id) const;

        protected:
            virtual QXmlNodeModelIndex nextFromSimpleAxis(SimpleAxis axis, const QXmlNodeModelIndex &origin) const;
            virtual QVector<QXmlNodeModelIndex> attributes(const QXmlNodeModelIndex &element) const;

        private:
            QExplicitlySharedDataPointer<const QAbstractXmlNodeModel> m_internalModel;
            QHash<QXmlNodeModelIndex, XsdElement::Ptr>                m_assignedElements;
            QHash<QXmlNodeModelIndex, XsdAttribute::Ptr>              m_assignedAttributes;
            QHash<QXmlNodeModelIndex, SchemaType::Ptr>                m_assignedTypes;
            QHash<QString, QSet<NamedSchemaComponent::Ptr> >          m_idIdRefBindings;
    };
}

QT_END_NAMESPACE

QT_END_HEADER

#endif
