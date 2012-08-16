/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the tools applications of the Qt Toolkit.
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

#ifndef QT3TO4_H
#define QT3TO4_H

#include <QString>
#include <QList>
#include <QPair>

QT_BEGIN_NAMESPACE

class Porting
{
public:
    typedef QPair<QString, QString> Rule;
    typedef QList<Rule> RuleList;

public:
    Porting();

    inline RuleList renamedHeaders() const
    { return m_renamedHeaders; }

    inline RuleList renamedClasses() const
    { return m_renamedClasses; }

    QString renameHeader(const QString &headerName) const;
    QString renameClass(const QString &className) const;
    QString renameEnumerator(const QString &enumName) const;

protected:
    static void readXML(RuleList *renamedHeaders, RuleList *renamedClasses, RuleList *renamedEnums);
    static int findRule(const RuleList &rules, const QString &q3);

private:
    RuleList m_renamedHeaders;
    RuleList m_renamedClasses;
    RuleList m_renamedEnums;
};

QT_END_NAMESPACE

#endif // QT3TO4_H
