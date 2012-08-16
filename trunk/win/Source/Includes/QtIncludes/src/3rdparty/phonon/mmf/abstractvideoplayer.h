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

#ifndef PHONON_MMF_ABSTRACTVIDEOPLAYER_H
#define PHONON_MMF_ABSTRACTVIDEOPLAYER_H

#include <videoplayer.h> // from epoc32/include

#include <QSize>

#include "abstractmediaplayer.h"
#include "abstractvideooutput.h"
#include "defs.h"

QT_BEGIN_NAMESPACE

namespace Phonon
{
namespace MMF
{
/**
 * @short ABC for classes which wrap the MMF video player utility
 *
 * On devices which use the legacy graphics subsystem which does not
 * support surfaces, video rendering is done via Direct Screen Access using
 * the CVideoPlayerUtility API.  On devices with a graphics subsystem which
 * does support surfaces, video rendering is done using the
 * CVideoPlayerUtility2 API.  Because CVideoPlayerUtility2 inherits from
 * CVideoPlayerUtility, AbstractVideoPlayer holds a pointer to the latter.
 *
 * @see DsaVideoPlayer, SurfaceVideoPlayer
 */
class AbstractVideoPlayer
    :   public AbstractMediaPlayer
    ,   public MVideoPlayerUtilityObserver
    ,   public MVideoLoadingObserver
{
    Q_OBJECT

public:
    ~AbstractVideoPlayer();

    typedef CVideoPlayerUtility NativePlayer;
    NativePlayer *nativePlayer() const;

    // AbstractPlayer
    virtual void doPlay();
    virtual void doPause();
    virtual void doStop();
    virtual void doSeek(qint64 milliseconds);
    virtual int setDeviceVolume(int mmfVolume);
    virtual int openFile(const QString &fileName);
    virtual int openFile(RFile &file);
    virtual int openUrl(const QString &url, int iap);
    virtual int openDescriptor(const TDesC8 &des);
    virtual int bufferStatus() const;
    virtual void doClose();

    // MediaObjectInterface
    virtual bool hasVideo() const;
    virtual qint64 totalTime() const;

    // AbstractPlayer
    virtual void videoOutputChanged();

    // AbstractMediaPlayer
    virtual qint64 getCurrentTime() const;
    virtual int numberOfMetaDataEntries() const;
    virtual QPair<QString, QString> metaDataEntry(int index) const;

public Q_SLOTS:
    void videoWindowChanged();
    void aspectRatioChanged();
    void scaleModeChanged();

protected:
    AbstractVideoPlayer(MediaObject *parent, const AbstractPlayer *player);
    void construct();
    virtual void initVideoOutput();
    void updateScaleFactors(const QSize &windowSize, bool apply = true);

    // Called when a video parameter changes.  If the underlying native API is
    // ready to handle the change, it is propagated immediately, otherwise the
    // change is recorded in m_pendingChanged and handled later by
    // handlePendingParametersChanged().
    void parametersChanged(VideoParameters parameter);

    // Implementation must initialize the m_player pointer.
    virtual void createPlayer() = 0;

    // Called from the MvpuoPrepareComplete callback.  Allows derived class to
    // calculate clipping rectangles and scale factors prior to starting
    // playback.
    virtual void prepareCompleted() = 0;

    // Called when native video window handle changes.  Derived class may defer
    // propagation of this change to the MMF video player utility by calling
    // parametersChanged().
    virtual void handleVideoWindowChanged() = 0;

    // Called when the derived class must handle changes which have been made
    // to video parameters such as window handle, screen rectangle and scale
    // factors.  Guaranteed to be called only when the underlying MMF video
    // player object is ready to handle changes to these parameters.
    virtual void handleParametersChanged(VideoParameters parameters) = 0;

private:
    void getVideoClipParametersL(TInt aError);

    // Called when native player API enters a state in which it is able to
    // handle pending changes such as new video window handle, updated scale
    // factors etc.
    void handlePendingParametersChanged();

private:
    // MVideoPlayerUtilityObserver
    virtual void MvpuoOpenComplete(TInt aError);
    virtual void MvpuoPrepareComplete(TInt aError);
    virtual void MvpuoFrameReady(CFbsBitmap &aFrame, TInt aError);
    virtual void MvpuoPlayComplete(TInt aError);
    virtual void MvpuoEvent(const TMMFEvent &aEvent);

    // MVideoLoadingObserver
    virtual void MvloLoadingStarted();
    virtual void MvloLoadingComplete();

protected:
    QScopedPointer<NativePlayer>        m_player;

    // Not owned
    RWsSession&                         m_wsSession;
    CWsScreenDevice&                    m_screenDevice;
    RWindowBase*                        m_window;

    // Scaling factors for video display, expressed as percentages
    TReal32                             m_scaleWidth;
    TReal32                             m_scaleHeight;

    // Dimensions of the video clip
    QSize                               m_videoFrameSize;

private:
    // Bitmask of parameters which have changed while the MMF video player
    // object is not yet ready to receive these changes.
    // See handlePendingParametersChanged().
    VideoParameters                     m_pendingChanges;

    // Duration of the video clip
    qint64                              m_totalTime;

};

}
}

QT_END_NAMESPACE

#endif
