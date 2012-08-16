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

#ifndef Patternist_AccelTreeResourceLoader_H
#define Patternist_AccelTreeResourceLoader_H

#include <QtCore/QEventLoop>
#include <QtNetwork/QNetworkReply>

#include "qabstractxmlreceiver.h"
#include "qacceltree_p.h"
#include "qacceltreebuilder_p.h"
#include "qdeviceresourceloader_p.h"
#include "qnamepool_p.h"
#include "qnetworkaccessdelegator_p.h"
#include "qreportcontext_p.h"

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class QIODevice;

namespace QPatternist
{
    /**
     * @short An helper class which enables QNetworkAccessManager
     * to be used in a blocking manner.
     *
     * @see AccelTreeResourceLoader::load()
     * @author Frans Englich <frans.englich@nokia.com>
     */
    class NetworkLoop : public QEventLoop
    {
        Q_OBJECT
    public:
        NetworkLoop() : m_hasReceivedError(false)
        {
        }

    public Q_SLOTS:
        void error(QNetworkReply::NetworkError code)
        {
            Q_UNUSED(code);
            m_hasReceivedError = true;
            exit(1);
        }

        void finished()
        {
            if(m_hasReceivedError)
                exit(1);
            else
                exit(0);
        }
    private:
        bool m_hasReceivedError;
    };

    /**
     * @short Handles requests for documents, and instantiates
     * them as AccelTree instances.
     *
     * @author Frans Englich <frans.englich@nokia.com>
     */
    class Q_AUTOTEST_EXPORT AccelTreeResourceLoader : public DeviceResourceLoader
    {
    public:
        /**
         * Describes the behaviour of the resource loader in case of an
         * error.
         */
        enum ErrorHandling
        {
            FailOnError,        ///< The resource loader will report the error via the report context.
            ContinueOnError     ///< The resource loader will report no error and return an empty QNetworkReply.
        };

        /**
         * AccelTreeResourceLoader does not own @p context.
         */
        AccelTreeResourceLoader(const NamePool::Ptr &np,
                                const NetworkAccessDelegator::Ptr &networkDelegator, AccelTreeBuilder<true>::Features = AccelTreeBuilder<true>::NoneFeature);

        virtual Item openDocument(const QUrl &uri,
                                  const ReportContext::Ptr &context);
        virtual Item openDocument(QIODevice *source, const QUrl &documentUri,
                                  const ReportContext::Ptr &context);
        virtual SequenceType::Ptr announceDocument(const QUrl &uri, const Usage usageHint);
        virtual bool isDocumentAvailable(const QUrl &uri);

        virtual bool isUnparsedTextAvailable(const QUrl &uri,
                                             const QString &encoding);

        virtual Item openUnparsedText(const QUrl &uri,
                                      const QString &encoding,
                                      const ReportContext::Ptr &context,
                                      const SourceLocationReflection *const where);

        /**
         * @short Helper function that do NetworkAccessDelegator::get(), but
         * does it blocked.
         *
         * The returned QNetworkReply has emitted QNetworkReply::finished().
         *
         * The caller owns the return QIODevice instance.
         *
         * @p context may be @c null or valid. If @c null, no error reporting
         * is done and @c null is returned.
         *
         * @see NetworkAccessDelegator
         */
        static QNetworkReply *load(const QUrl &uri,
                                   QNetworkAccessManager *const networkManager,
                                   const ReportContext::Ptr &context, ErrorHandling handling = FailOnError);

        /**
         * @overload
         */
        static QNetworkReply *load(const QUrl &uri,
                                   const NetworkAccessDelegator::Ptr &networkDelegator,
                                   const ReportContext::Ptr &context, ErrorHandling handling = FailOnError);

        /**
         * @short Returns the URIs this AccelTreeResourceLoader has loaded
         * which are for devices through variable bindings.
         */
        virtual QSet<QUrl> deviceURIs() const;

        virtual void clear(const QUrl &uri);

    private:
        static bool streamToReceiver(QIODevice *const dev,
                                     AccelTreeBuilder<true> *const receiver,
                                     const NamePool::Ptr &np,
                                     const ReportContext::Ptr &context,
                                     const QUrl &uri);
        bool retrieveDocument(const QUrl &uri,
                              const ReportContext::Ptr &context);
        bool retrieveDocument(QIODevice *source, const QUrl &documentUri,
                              const ReportContext::Ptr &context);
        /**
         * If @p context is @c null, no error reporting should be done.
         */
        bool retrieveUnparsedText(const QUrl &uri,
                                  const QString &encoding,
                                  const ReportContext::Ptr &context,
                                  const SourceLocationReflection *const where);

        QHash<QUrl, AccelTree::Ptr>             m_loadedDocuments;
        const NamePool::Ptr                     m_namePool;
        const NetworkAccessDelegator::Ptr       m_networkAccessDelegator;
        QHash<QPair<QUrl, QString>, QString>    m_unparsedTexts;
        AccelTreeBuilder<true>::Features        m_features;
    };
}

QT_END_NAMESPACE

QT_END_HEADER

#endif
