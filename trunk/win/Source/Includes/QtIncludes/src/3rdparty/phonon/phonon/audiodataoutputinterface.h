/*  This file is part of the KDE project
    Copyright (C) 2008 Matthias Kretz <kretz@kde.org>

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

#ifndef PHONON_AUDIODATAOUTPUTINTERFACE_H
#define PHONON_AUDIODATAOUTPUTINTERFACE_H

QT_BEGIN_HEADER
QT_BEGIN_NAMESPACE

namespace Phonon
{

class AudioDataOutput;

class AudioDataOutputInterface
{
    public:
        virtual ~AudioDataOutputInterface() {}

        virtual AudioDataOutput *frontendObject() const = 0;
        virtual void setFrontendObject(AudioDataOutput *) = 0;
};

} // namespace Phonon

Q_DECLARE_INTERFACE(Phonon::AudioDataOutputInterface, "0AudioDataOutputInterface.phonon.kde.org")

QT_END_NAMESPACE
QT_END_HEADER

#endif // PHONON_AUDIODATAOUTPUTINTERFACE_H
