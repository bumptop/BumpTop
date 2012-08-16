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

#ifndef PHONON_MMF_ABSTRACTMEDIAPLAYER_H
#define PHONON_MMF_ABSTRACTMEDIAPLAYER_H

#include <QTimer>
#include <QScopedPointer>
#include <e32std.h>
#include "abstractplayer.h"
#ifdef PHONON_MMF_PROGRESSIVE_DOWNLOAD
#   include "download.h"
#endif

class RFile;

QT_BEGIN_NAMESPACE

namespace Phonon
{
namespace MMF
{
class AudioOutput;
class MediaObject;

/**
 * Interface via which MMF client APIs for both audio and video can be
 * accessed.
 */
class AbstractMediaPlayer : public AbstractPlayer
{
    Q_OBJECT

protected:
    AbstractMediaPlayer(MediaObject *parent, const AbstractPlayer *player);

public:
    virtual void open();
    virtual void close();

    // MediaObjectInterface
    virtual void play();
    virtual void pause();
    virtual void stop();
    virtual void seek(qint64 milliseconds);
    virtual bool isSeekable() const;
    virtual qint64 currentTime() const;
    virtual void volumeChanged(qreal volume);

protected:
    // AbstractPlayer
    virtual void doSetTickInterval(qint32 interval);
    virtual Phonon::State phononState(PrivateState state) const;
    virtual void changeState(PrivateState newState);

    virtual void doPlay() = 0;
    virtual void doPause() = 0;
    virtual void doStop() = 0;
    virtual void doSeek(qint64 pos) = 0;
    virtual int setDeviceVolume(int mmfVolume) = 0;
    virtual int openFile(const QString &fileName) = 0;
    virtual int openFile(RFile& file) = 0;
    virtual int openUrl(const QString& url, int iap) = 0;
    virtual int openDescriptor(const TDesC8 &des) = 0;
    virtual int bufferStatus() const = 0;
    virtual void doClose() = 0;

    void updateMetaData();
    virtual qint64 getCurrentTime() const = 0;
    virtual int numberOfMetaDataEntries() const = 0;
    virtual QPair<QString, QString> metaDataEntry(int index) const = 0;

protected:
    void bufferingStarted();
    void bufferingComplete();
    void maxVolumeChanged(int maxVolume);
    void loadingComplete(int error);
    void playbackComplete(int error);

    static qint64 toMilliSeconds(const TTimeIntervalMicroSeconds &);

    bool isProgressiveDownload() const;
    bool progressiveDownloadStalled() const;

private:
    void startPositionTimer();
    void stopPositionTimer();
    void startBufferStatusTimer();
    void stopBufferStatusTimer();
    void stopTimers();
    void doVolumeChanged();
    void emitMarksIfReached(qint64 position);
    void resetMarksIfRewound();
    void startPlayback();
    void setProgressiveDownloadStalled();

    enum Pending {
        NothingPending,
        PausePending,
        PlayPending
    };

    void setPending(Pending pending);

private Q_SLOTS:
    void positionTick();
    void bufferStatusTick();
#ifdef PHONON_MMF_PROGRESSIVE_DOWNLOAD
    void downloadLengthChanged(qint64);
    void downloadStateChanged(Download::State);
#endif

private:
    MediaObject *const          m_parent;

    Pending                     m_pending;

    QScopedPointer<QTimer>      m_positionTimer;
    qint64                      m_position;

    QScopedPointer<QTimer>      m_bufferStatusTimer;
    PrivateState                m_stateBeforeBuffering;

    int                         m_mmfMaxVolume;

    bool                        m_prefinishMarkSent;
    bool                        m_aboutToFinishSent;

    // Used for playback of resource files
    TPtrC8                      m_buffer;

#ifdef PHONON_MMF_PROGRESSIVE_DOWNLOAD
    Download                    *m_download;
    bool                        m_downloadStalled;
#endif

    QMultiMap<QString, QString> m_metaData;

};
}
}

QT_END_NAMESPACE

#endif

