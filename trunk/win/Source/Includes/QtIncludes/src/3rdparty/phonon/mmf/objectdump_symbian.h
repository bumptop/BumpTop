/*  This file is part of the KDE project.

Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).

This library is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 or 3 of the License.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this library.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef OBJECTDUMP_SYMBIAN_H
#define OBJECTDUMP_SYMBIAN_H

#include "objectdump.h"

QT_BEGIN_NAMESPACE

namespace ObjectDump
{
namespace Symbian
{

/**
 * Annotator which returns Symbian-specific widget information
 */
class QAnnotatorWidget : public QAnnotator
{
    Q_OBJECT
public:
    QList<QByteArray> annotation(const QObject& object);
};

/**
 * Annotator which returns control information
 */
class QAnnotatorControl : public QAnnotator
{
    Q_OBJECT
public:
    QList<QByteArray> annotation(const QObject& object);
};

/**
 * Annotator which returns window information
 */
class QAnnotatorWindow : public QAnnotator
{
    Q_OBJECT
public:
    QList<QByteArray> annotation(const QObject& object);
};

} // namespace Symbian
} // namespace ObjectDump

QT_END_NAMESPACE

#endif
