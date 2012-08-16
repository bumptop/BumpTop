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

#ifndef PHONON_MMF_VIDEOPLAYER_DSA_H
#define PHONON_MMF_VIDEOPLAYER_DSA_H

#include "abstractvideoplayer.h"

QT_BEGIN_NAMESPACE

namespace Phonon
{
namespace MMF
{

/**
 * @short Wrapper over the MMF video player utility (DSA version)
 *
 * This implementation is used on devices with the legacy graphics
 * subsystem, which does not support surfaces.  On such devices,
 * video rendering is done via Direct Screen Access (DSA), whereby
 * the video decoder writes directly to the framebuffer.  To ensure
 * that the window server and video decoder do not try to draw to
 * the same screen region at the same time, the video subsystem
 * first requests permission to perform DSA.  If the window server
 * needs to draw to this screen region (for example to display a
 * message dialog), it first notifies the video subsystem that it
 * must stop rendering to this region.
 *
 * @see SurfaceVideoPlayer
 */
class DsaVideoPlayer
    :   public AbstractVideoPlayer
{
    Q_OBJECT

public:
    // Factory function
    static DsaVideoPlayer* create(MediaObject *parent = 0,
                                  const AbstractPlayer *player = 0);
    ~DsaVideoPlayer();

public Q_SLOTS:
    void videoWindowScreenRectChanged();
    void suspendDirectScreenAccess();
    void resumeDirectScreenAccess();

private:
    DsaVideoPlayer(MediaObject *parent, const AbstractPlayer *player);

    // AbstractVideoPlayer
    void createPlayer();
    void initVideoOutput();
    void prepareCompleted();
    void handleVideoWindowChanged();
    void handleParametersChanged(VideoParameters parameters);

    void startDirectScreenAccess();
    bool stopDirectScreenAccess();

private:
    bool                m_dsaActive;
    bool                m_dsaWasActive;

    // Absolute screen rectangle on which video is displayed
    TRect               m_videoScreenRect;

};

}
}

QT_END_NAMESPACE

#endif // !PHONON_MMF_VIDEOPLAYER_DSA_H


