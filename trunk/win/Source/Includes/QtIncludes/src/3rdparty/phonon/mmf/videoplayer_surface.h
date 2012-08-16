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

#ifndef PHONON_MMF_VIDEOPLAYER_SURFACE_H
#define PHONON_MMF_VIDEOPLAYER_SURFACE_H

#include "abstractvideoplayer.h"

class RWindow;

QT_BEGIN_NAMESPACE

namespace Phonon
{
namespace MMF
{

/**
 * @short Wrapper over the MMF video player utility (surface version)
 *
 * This implementation is used on devices with a graphics subsystem which
 * supports surfaces.
 *
 * @see DsaVideoPlayer
 */
class SurfaceVideoPlayer
    :   public AbstractVideoPlayer
{
    Q_OBJECT

public:
    // Factory function
    static SurfaceVideoPlayer* create(MediaObject *parent = 0,
                                  const AbstractPlayer *player = 0);
    ~SurfaceVideoPlayer();

public Q_SLOTS:
    void videoWindowSizeChanged();

private:
    SurfaceVideoPlayer(MediaObject *parent, const AbstractPlayer *player);

    // AbstractVideoPlayer
    void createPlayer();
    void initVideoOutput();
    void prepareCompleted();
    void handleVideoWindowChanged();
    void handleParametersChanged(VideoParameters parameters);

    void addDisplayWindow(const TRect &rect);
    void removeDisplayWindow();

private:
    // Window handle which has been passed to the MMF via
    // CVideoPlayerUtility2::SetDisplayWindowL
    RWindow*    m_displayWindow;

};

}
}

QT_END_NAMESPACE

#endif // !PHONON_MMF_VIDEOPLAYER_SURFACE_H

