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

#ifndef Patternist_CastingPlatform_H
#define Patternist_CastingPlatform_H

#include "qatomiccasterlocator_p.h"
#include "qatomiccaster_p.h"
#include "qatomicstring_p.h"
#include "qatomictype_p.h"
#include "qbuiltintypes_p.h"
#include "qcommonsequencetypes_p.h"
#include "qpatternistlocale_p.h"
#include "qqnamevalue_p.h"
#include "qschematypefactory_p.h"
#include "qstaticcontext_p.h"
#include "qvalidationerror_p.h"

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

namespace QPatternist
{
    /**
     * @short Provides casting functionality for classes, such as CastAs or NumberFN, which
     * needs to perform casting.
     *
     * Classes which need to perform casting can simply from this class and gain
     * access to casting functinality wrapped in a convenient way. At the center of this
     * class is the cast() function, which is used at runtime to perform the actual cast.
     *
     * The actual circumstances where casting is used, such as in the 'castable as'
     * expression or the <tt>fn:number()</tt> function, often have other things to handle as well,
     * error handling and cardinality checks for example. This class handles only casting
     * and leaves the other case-specific details to the sub-class such that this class only
     * do one thing well.
     *
     * This template class takes two parameters:
     * - TSubClass This should be the class inheriting from CastingPlatform.
     * - issueError if true, errors are issued via ReportContext, otherwise
     *   ValidationError instances are returned appropriately.
     *
     * The class inheriting CastingPlatform must implement the following function:
     * @code
     * ItemType::Ptr targetType() const
     * @endcode
     *
     * that returns the type that should be cast to. The type must be an AtomicType.
     * Typically, it is appropriate to declare this function @c inline.
     *
     * A sub-class calls prepareCasting() at compile time(such that CastingPlatform can attempt
     * to lookup the proper AtomicCaster) and then it simply uses the cast() function at runtime. The
     * function targetType() must be implemented such that CastingPlatform knows
     * what type it shall cast to.
     *
     * @see ValueFactory
     * @author Frans Englich <frans.englich@nokia.com>
     * @ingroup Patternist_expressions
     */
    template<typename TSubClass, const bool issueError>
    class CastingPlatform
    {
    protected:
        /**
         * @note issueCastError() depends on the default value.
         */
        inline CastingPlatform(const ReportContext::ErrorCode code = ReportContext::FORG0001) : m_errorCode(code)
        {
        }

        /**
         * Attempts to cast @p sourceValue to targetType(), and returns
         * the created value. Remember that prepareCasting() should have been
         * called at compile time, otherwise this function will be slow.
         *
         * Error reporting is done in two ways. If a cast fails because
         * of an error in lexical representation a ValidationError is returned.
         * If the cause of failure is that the casting combination is invalid(such as
         * when attempting to cast @c xs:date to @c xs:integer), a ValidationError
         * is returned if @c false was passed in the template instantiation,
         * an error is issued via @p context.
         *
         * @param sourceValue the value to cast. Must be non @c null.
         * @param context the usual ReportContext, used for error reporting.
         * @returns the new value which was the result of the cast. If the
         * cast failed, an ValidationError is returned.
         */
        Item cast(const Item &sourceValue,
                  const ReportContext::Ptr &context) const;

        /**
         * This function should be called at compiled time, it attempts to determine
         * what AtomicCaster that should be used when casting from @p sourceType to
         * targetType(). If that is not possible, because the @p sourceType is
         * @c xs:anyAtomicType for instance, the AtomicCaster lookup will done at
         * runtime on a case-per-case basis.
         *
         * @returns @c true if the requested casting combination is valid or might be valid.
         * If it is guranteed to be invalid, @c false is returned.
         */
        bool prepareCasting(const ReportContext::Ptr &context,
                            const ItemType::Ptr &sourceType);

        /**
         * Checks that the targetType() is a valid target type for <tt>castable as</tt>
         * and <tt>cast as</tt>. For example, that it is not abstract. If the type is
         * invalid, an error is raised via the @p context. Note that it is assumed the type
         * is atomic.
         */
        void checkTargetType(const ReportContext::Ptr &context) const;

    private:
        inline Item castWithCaster(const Item &sourceValue,
                                   const AtomicCaster::Ptr &caster,
                                   const ReportContext::Ptr &context) const;

        /**
         * Locates the caster for casting values of type @p sourceType to targetType(), if
         * possible.
         *
         * @p castImpossible is not initialized. Initialize it to @c false.
         */
        static AtomicCaster::Ptr locateCaster(const ItemType::Ptr &sourceType,
                                              const ReportContext::Ptr &context,
                                              bool &castImpossible,
                                              const SourceLocationReflection *const location,
                                              const ItemType::Ptr &targetType);
    private:
        inline Item castWithCaster(const Item &sourceValue,
                                   const AtomicCaster::Ptr &caster,
                                   const DynamicContext::Ptr &context) const;


        inline ItemType::Ptr targetType() const
        {
            Q_ASSERT(static_cast<const TSubClass *>(this)->targetType());
            return static_cast<const TSubClass *>(this)->targetType();
        }

        void issueCastError(const Item &validationError,
                            const Item &sourceValue,
                            const ReportContext::Ptr &context) const;

        Q_DISABLE_COPY(CastingPlatform)
        AtomicCaster::Ptr m_caster;
        const ReportContext::ErrorCode m_errorCode;
    };

#include "qcastingplatform.cpp"

}

QT_END_NAMESPACE

QT_END_HEADER

#endif
