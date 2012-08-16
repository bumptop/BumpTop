/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtSCriptTools module of the Qt Toolkit.
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

#ifndef QSCRIPTBREAKPOINTDATA_P_H
#define QSCRIPTBREAKPOINTDATA_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qobjectdefs.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qmap.h>

QT_BEGIN_NAMESPACE

class QDataStream;
class QVariant;

class QScriptBreakpointDataPrivate;
class Q_AUTOTEST_EXPORT QScriptBreakpointData
{
public:
    friend Q_AUTOTEST_EXPORT QDataStream &operator<<(QDataStream &, const QScriptBreakpointData &);
    friend Q_AUTOTEST_EXPORT QDataStream &operator>>(QDataStream &, QScriptBreakpointData &);

    QScriptBreakpointData();
    QScriptBreakpointData(qint64 scriptId, int lineNumber);
    QScriptBreakpointData(const QString &fileName, int lineNumber);
    QScriptBreakpointData(const QScriptBreakpointData &other);
    ~QScriptBreakpointData();
    QScriptBreakpointData &operator=(const QScriptBreakpointData &other);

    bool isValid() const;

    // location
    qint64 scriptId() const;
    void setScriptId(qint64 id);

    QString fileName() const;
    void setFileName(const QString &fileName);

    int lineNumber() const;
    void setLineNumber(int lineNumber);

    // data
    bool isEnabled() const;
    void setEnabled(bool enabled);

    bool isSingleShot() const;
    void setSingleShot(bool singleShot);

    int ignoreCount() const;
    void setIgnoreCount(int count);

    QString condition() const;
    void setCondition(const QString &condition);

    QVariant data() const;
    void setData(const QVariant &data);

    bool hit();

    // statistics
    int hitCount() const;


    bool operator==(const QScriptBreakpointData &other) const;
    bool operator!=(const QScriptBreakpointData &other) const;

private:
    QScopedPointer<QScriptBreakpointDataPrivate> d_ptr;

    Q_DECLARE_PRIVATE(QScriptBreakpointData)
};

typedef QMap<int, QScriptBreakpointData> QScriptBreakpointMap;

Q_AUTOTEST_EXPORT QDataStream &operator<<(QDataStream &, const QScriptBreakpointData &);
Q_AUTOTEST_EXPORT QDataStream &operator>>(QDataStream &, QScriptBreakpointData &);

QT_END_NAMESPACE

#endif
