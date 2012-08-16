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

#ifndef PHONON_MMF_DEFS_H
#define PHONON_MMF_DEFS_H

#include <QtGlobal>

QT_BEGIN_NAMESPACE

namespace Phonon
{
namespace MMF
{
static const qint32 DefaultTickInterval = 10;
static const qreal  InitialVolume = 0.5;

enum MediaType {
    MediaTypeUnknown,
    MediaTypeAudio,
    MediaTypeVideo
};

enum VideoParameter {
    WindowHandle        = 0x1,
    WindowScreenRect    = 0x2,
    ScaleFactors        = 0x4
};
Q_DECLARE_FLAGS(VideoParameters, VideoParameter)
Q_DECLARE_OPERATORS_FOR_FLAGS(VideoParameters)

}
}

QT_END_NAMESPACE

#endif // PHONON_MMF_DEFS_H
