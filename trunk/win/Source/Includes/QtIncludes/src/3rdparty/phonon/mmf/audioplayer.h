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

#ifndef PHONON_MMF_AUDIOPLAYER_H
#define PHONON_MMF_AUDIOPLAYER_H

#include "abstractmediaplayer.h"

class CDrmPlayerUtility;
class TTimeIntervalMicroSeconds;

#ifdef QT_PHONON_MMF_AUDIO_DRM
#include <drmaudiosampleplayer.h>
typedef MDrmAudioPlayerCallback NativePlayerObserver;
#else
#include <mdaaudiosampleplayer.h>
typedef MMdaAudioPlayerCallback NativePlayerObserver;
#endif

QT_BEGIN_NAMESPACE

namespace Phonon
{
namespace MMF
{
/**
 * @short Wrapper over MMF audio client utility
 */
class AudioPlayer   :   public AbstractMediaPlayer
                    ,   public NativePlayerObserver
                    ,   public MAudioLoadingObserver
{
    Q_OBJECT

public:
    AudioPlayer(MediaObject *parent = 0, const AbstractPlayer *player = 0);
    virtual ~AudioPlayer();

#ifdef QT_PHONON_MMF_AUDIO_DRM
typedef CDrmPlayerUtility NativePlayer;
#else
typedef CMdaAudioPlayerUtility NativePlayer;
#endif

    NativePlayer *nativePlayer() const;

    // AbstractMediaPlayer
    virtual void doPlay();
    virtual void doPause();
    virtual void doStop();
    virtual void doSeek(qint64 milliseconds);
    virtual int setDeviceVolume(int mmfVolume);
    virtual int openFile(const QString &fileName);
    virtual int openFile(RFile& file);
    virtual int openUrl(const QString& url, int iap);
    virtual int openDescriptor(const TDesC8 &des);
    virtual int bufferStatus() const;
    virtual void doClose();

    // MediaObjectInterface
    virtual bool hasVideo() const;
    virtual qint64 totalTime() const;

    // AbstractMediaPlayer
    virtual qint64 getCurrentTime() const;
    virtual int numberOfMetaDataEntries() const;
    virtual QPair<QString, QString> metaDataEntry(int index) const;

    /**
     * This class owns the pointer.
     */
    NativePlayer *player() const;

private:
    void construct();

private:
#ifdef QT_PHONON_MMF_AUDIO_DRM
    // MDrmAudioPlayerCallback
    virtual void MdapcInitComplete(TInt aError,
                                   const TTimeIntervalMicroSeconds &aDuration);
    virtual void MdapcPlayComplete(TInt aError);
#else
    // MMdaAudioPlayerCallback
    virtual void MapcInitComplete(TInt aError,
                                  const TTimeIntervalMicroSeconds &aDuration);
    virtual void MapcPlayComplete(TInt aError);
#endif

    // MAudioLoadingObserver
    virtual void MaloLoadingStarted();
    virtual void MaloLoadingComplete();

private:
    /**
     * Using CPlayerType typedef in order to be able to easily switch between
     * CMdaAudioPlayerUtility and CDrmPlayerUtility
     */
    QScopedPointer<NativePlayer> m_player;

    qint64                      m_totalTime;

};
}
}

QT_END_NAMESPACE

#endif
