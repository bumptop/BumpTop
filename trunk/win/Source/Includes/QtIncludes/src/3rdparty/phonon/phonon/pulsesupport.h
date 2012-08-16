/*  This file is part of the KDE project
    Copyright (C) 2009 Colin Guthrie <cguthrie@mandriva.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), Nokia Corporation 
    (or its successors, if any) and the KDE Free Qt Foundation, which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public 
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef PHONON_PULSESUPPORT_H
#define PHONON_PULSESUPPORT_H

#include "phonon_export.h"
#include "phononnamespace.h"
#include "objectdescription.h"

#include <QtCore/QtGlobal>
#include <QtCore/QSet>

QT_BEGIN_HEADER
QT_BEGIN_NAMESPACE

namespace Phonon
{
    class PHONON_EXPORT PulseSupport : public QObject
    {
        Q_OBJECT
        public:
            static PulseSupport* getInstance();
            static void shutdown();

            bool isActive();
            void enable(bool enabled = true);

            QList<int> objectDescriptionIndexes(ObjectDescriptionType type) const;
            QHash<QByteArray, QVariant> objectDescriptionProperties(ObjectDescriptionType type, int index) const;
            QList<int> objectIndexesByCategory(ObjectDescriptionType type, Category category) const;

            void setOutputDevicePriorityForCategory(Category category, QList<int> order);
            void setCaptureDevicePriorityForCategory(Category category, QList<int> order);

            void setStreamPropList(Category category, QString streamUuid);
            void emitObjectDescriptionChanged(ObjectDescriptionType);
            void emitUsingDevice(QString streamUuid, int device);

            bool setOutputDevice(QString streamUuid, int device);
            bool setCaptureDevice(QString streamUuid, int device);
            void clearStreamCache(QString streamUuid);

        signals:
            void objectDescriptionChanged(ObjectDescriptionType);
            void usingDevice(QString streamUuid, int device);

        private:
            PulseSupport();
            ~PulseSupport();

            bool mEnabled;
    };
} // namespace Phonon

QT_END_NAMESPACE
QT_END_HEADER

#endif // PHONON_PULSESUPPORT_H
