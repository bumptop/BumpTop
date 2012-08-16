/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtDeclarative module of the Qt Toolkit.
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

#ifndef QDECLARATIVEINFO_H
#define QDECLARATIVEINFO_H

#include <QtCore/qdebug.h>
#include <QtCore/qurl.h>
#include <QtDeclarative/qdeclarativeerror.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Declarative)

class QDeclarativeInfoPrivate;
class Q_DECLARATIVE_EXPORT QDeclarativeInfo : public QDebug
{
public:
    QDeclarativeInfo(const QDeclarativeInfo &);
    ~QDeclarativeInfo();

    inline QDeclarativeInfo &operator<<(QChar t) { QDebug::operator<<(t); return *this; }
    inline QDeclarativeInfo &operator<<(QBool t) { QDebug::operator<<(t); return *this; }
    inline QDeclarativeInfo &operator<<(bool t) { QDebug::operator<<(t); return *this; }
    inline QDeclarativeInfo &operator<<(char t) { QDebug::operator<<(t); return *this; }
    inline QDeclarativeInfo &operator<<(signed short t) { QDebug::operator<<(t); return *this; }
    inline QDeclarativeInfo &operator<<(unsigned short t) { QDebug::operator<<(t); return *this; }
    inline QDeclarativeInfo &operator<<(signed int t) { QDebug::operator<<(t); return *this; }
    inline QDeclarativeInfo &operator<<(unsigned int t) { QDebug::operator<<(t); return *this; }
    inline QDeclarativeInfo &operator<<(signed long t) { QDebug::operator<<(t); return *this; }
    inline QDeclarativeInfo &operator<<(unsigned long t) { QDebug::operator<<(t); return *this; }
    inline QDeclarativeInfo &operator<<(qint64 t) { QDebug::operator<<(t); return *this; }
    inline QDeclarativeInfo &operator<<(quint64 t) { QDebug::operator<<(t); return *this; }
    inline QDeclarativeInfo &operator<<(float t) { QDebug::operator<<(t); return *this; }
    inline QDeclarativeInfo &operator<<(double t) { QDebug::operator<<(t); return *this; }
    inline QDeclarativeInfo &operator<<(const char* t) { QDebug::operator<<(t); return *this; }
    inline QDeclarativeInfo &operator<<(const QString & t) { QDebug::operator<<(t.toLocal8Bit().constData()); return *this; }
    inline QDeclarativeInfo &operator<<(const QStringRef & t) { return operator<<(t.toString()); }
    inline QDeclarativeInfo &operator<<(const QLatin1String &t) { QDebug::operator<<(t.latin1()); return *this; }
    inline QDeclarativeInfo &operator<<(const QByteArray & t) { QDebug::operator<<(t); return *this; }
    inline QDeclarativeInfo &operator<<(const void * t) { QDebug::operator<<(t); return *this; }
    inline QDeclarativeInfo &operator<<(QTextStreamFunction f) { QDebug::operator<<(f); return *this; }
    inline QDeclarativeInfo &operator<<(QTextStreamManipulator m) { QDebug::operator<<(m); return *this; }
#ifndef QT_NO_DEBUG_STREAM
    inline QDeclarativeInfo &operator<<(const QUrl &t) { static_cast<QDebug &>(*this) << t; return *this; }
#endif

private:
    friend Q_DECLARATIVE_EXPORT QDeclarativeInfo qmlInfo(const QObject *me);
    friend Q_DECLARATIVE_EXPORT QDeclarativeInfo qmlInfo(const QObject *me, const QDeclarativeError &error);
    friend Q_DECLARATIVE_EXPORT QDeclarativeInfo qmlInfo(const QObject *me, const QList<QDeclarativeError> &errors);

    QDeclarativeInfo(QDeclarativeInfoPrivate *);
    QDeclarativeInfoPrivate *d;
};

Q_DECLARATIVE_EXPORT QDeclarativeInfo qmlInfo(const QObject *me);
Q_DECLARATIVE_EXPORT QDeclarativeInfo qmlInfo(const QObject *me, const QDeclarativeError &error);
Q_DECLARATIVE_EXPORT QDeclarativeInfo qmlInfo(const QObject *me, const QList<QDeclarativeError> &errors);

QT_END_NAMESPACE

QT_END_HEADER

#endif // QDECLARATIVEINFO_H
