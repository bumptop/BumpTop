/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtCore module of the Qt Toolkit.
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

#ifndef QCORE_SYMBIAN_P_H
#define QCORE_SYMBIAN_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <e32std.h>
#include <QtCore/qglobal.h>
#include <qstring.h>
#include <qrect.h>
#include <qhash.h>
#include <f32file.h>

#define QT_LSTRING2(x) L##x
#define QT_LSTRING(x) QT_LSTRING2(x)

#if defined(QT_LIBINFIX)
#  define QT_LIBINFIX_UNICODE QT_LSTRING(QT_LIBINFIX)
#else
#  define QT_LIBINFIX_UNICODE L""
#endif

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

Q_CORE_EXPORT HBufC* qt_QString2HBufC(const QString& aString);

Q_CORE_EXPORT QString qt_TDesC2QString(const TDesC& aDescriptor);
inline QString qt_TDes2QString(const TDes& aDescriptor) { return qt_TDesC2QString(aDescriptor); }

static inline QSize qt_TSize2QSize(const TSize& ts)
{
    return QSize(ts.iWidth, ts.iHeight);
}

static inline TSize qt_QSize2TSize(const QSize& qs)
{
    return TSize(qs.width(), qs.height());
}

static inline QRect qt_TRect2QRect(const TRect& tr)
{
    return QRect(tr.iTl.iX, tr.iTl.iY, tr.Width(), tr.Height());
}

static inline TRect qt_QRect2TRect(const QRect& qr)
{
    return TRect(TPoint(qr.left(), qr.top()), TSize(qr.width(), qr.height()));
}

// Returned TPtrC is valid as long as the given parameter is valid and unmodified
static inline TPtrC qt_QString2TPtrC( const QString& string )
{
    return TPtrC16(static_cast<const TUint16*>(string.utf16()), string.length());
}

/*!
    \internal
    This class is a wrapper around the Symbian HBufC descriptor class.
    It makes sure that the heap allocated HBufC class is freed when it is
    destroyed.
*/
class Q_CORE_EXPORT QHBufC
{
public:
    QHBufC();
    QHBufC(const QHBufC &src);
    QHBufC(HBufC *src);
    QHBufC(const QString &src);
    ~QHBufC();

    inline operator HBufC *() { return m_hBufC; }
    inline operator const HBufC *() const { return m_hBufC; }
    inline HBufC *data() { return m_hBufC; }
    inline const HBufC *data() const { return m_hBufC; }
    inline HBufC & operator*() { return *m_hBufC; }
    inline const HBufC & operator*() const { return *m_hBufC; }
    inline HBufC * operator->() { return m_hBufC; }
    inline const HBufC * operator->() const { return m_hBufC; }

    inline bool operator==(const QHBufC &param) const { return data() == param.data(); }
    inline bool operator!=(const QHBufC &param) const { return data() != param.data(); }

private:
    HBufC *m_hBufC;
};

inline uint qHash(TUid uid)
{
    return qHash(uid.iUid);
}

// S60 version specific function ordinals that can be resolved
enum S60PluginFuncOrdinals
{
    S60Plugin_TimeFormatL            = 1,
    S60Plugin_GetTimeFormatSpec      = 2,
    S60Plugin_GetLongDateFormatSpec  = 3,
    S60Plugin_GetShortDateFormatSpec = 4,
    S60Plugin_LocalizedDirectoryName = 5,
    S60Plugin_GetSystemDrive         = 6
};

Q_CORE_EXPORT TLibraryFunction qt_resolveS60PluginFunc(int ordinal);

Q_CORE_EXPORT RFs& qt_s60GetRFs();

// Defined in qlocale_symbian.cpp.
Q_CORE_EXPORT QByteArray qt_symbianLocaleName(int code);

QT_END_NAMESPACE

QT_END_HEADER

#endif //QCORE_SYMBIAN_P_H
