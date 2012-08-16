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

#ifndef TEXTWRITER_H
#define TEXTWRITER_H

#include <qdeclarativejsglobal_p.h>

#include <QtCore/QString>
#include <QtCore/QList>
#include <QtGui/QTextCursor>

QT_BEGIN_HEADER
QT_QML_BEGIN_NAMESPACE

namespace QDeclarativeJS {

class TextWriter
{
	QString *string;
	QTextCursor *cursor;

	struct Replace {
		int pos;
		int length;
		QString replacement;
	};

	QList<Replace> replaceList;

	struct Move {
		int pos;
		int length;
		int to;
	};

	QList<Move> moveList;

	bool hasOverlap(int pos, int length);
	bool hasMoveInto(int pos, int length);

	void doReplace(const Replace &replace);
	void doMove(const Move &move);

	void write_helper();

public:
	TextWriter();

	void replace(int pos, int length, const QString &replacement);
	void move(int pos, int length, int to);

	void write(QString *s);
	void write(QTextCursor *textCursor);

};

} // end of namespace QDeclarativeJS

QT_QML_END_NAMESPACE
QT_END_HEADER

#endif // TEXTWRITER_H
