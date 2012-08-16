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

#ifndef PHONON_MMF_MEDIAOBJECT_H
#define PHONON_MMF_MEDIAOBJECT_H

#include <phonon/mediasource.h>
#include <phonon/mediaobjectinterface.h>
#include <QScopedPointer>
#include <QTimer>
#include <QString>

// For recognizer
#include <apgcli.h>

#include "abstractplayer.h"
#include "mmf_medianode.h"
#include "defs.h"

QT_BEGIN_NAMESPACE

class QResource;

namespace Phonon
{
namespace MMF
{
class AbstractPlayer;
class AbstractVideoOutput;

/**
 * @short Facade class which wraps MMF client utility instance
 */
class MediaObject : public MediaNode
                  , public MediaObjectInterface
{
    Q_OBJECT
    Q_INTERFACES(Phonon::MediaObjectInterface)

public:
    MediaObject(QObject *parent);
    virtual ~MediaObject();

    // MediaObjectInterface
    virtual void play();
    virtual void pause();
    virtual void stop();
    virtual void seek(qint64 milliseconds);
    virtual qint32 tickInterval() const;
    virtual void setTickInterval(qint32 interval);
    virtual bool hasVideo() const;
    virtual bool isSeekable() const;
    virtual qint64 currentTime() const;
    virtual Phonon::State state() const;
    virtual QString errorString() const;
    virtual Phonon::ErrorType errorType() const;
    virtual qint64 totalTime() const;
    virtual MediaSource source() const;
    virtual void setSource(const MediaSource &);
    virtual void setNextSource(const MediaSource &source);
    virtual qint32 prefinishMark() const;
    virtual void setPrefinishMark(qint32);
    virtual qint32 transitionTime() const;
    virtual void setTransitionTime(qint32);

    // MediaNode
    void connectMediaObject(MediaObject *mediaObject);
    void disconnectMediaObject(MediaObject *mediaObject);

    /**
     * This class owns the AbstractPlayer, and will delete it upon
     * destruction.
     */
    AbstractPlayer *abstractPlayer() const;

    void setVideoOutput(AbstractVideoOutput* videoOutput);

    int openFileHandle(const QString &fileName);
    RFile* file() const;
    QResource* resource() const;
    int currentIAP() const;

public Q_SLOTS:
    void volumeChanged(qreal volume);
    void switchToNextSource();

Q_SIGNALS:
    void abstractPlayerChanged(AbstractPlayer *player);
    void totalTimeChanged(qint64 length);
    void hasVideoChanged(bool hasVideo);
    void seekableChanged(bool seekable);
    void bufferStatus(int);
    void aboutToFinish();
    void prefinishMarkReached(qint32 remaining);
    // TODO: emit metaDataChanged from MediaObject
    void metaDataChanged(const QMultiMap<QString, QString>& metaData);
    void currentSourceChanged(const MediaSource& source);
    void stateChanged(Phonon::State newState,
                      Phonon::State oldState);
    void finished();
    void tick(qint64 time);

protected:
    bool eventFilter(QObject *watched, QEvent *event);

private Q_SLOTS:
    void handlePrefinishMarkReached(qint32);

private:
    void switchToSource(const MediaSource &source);
    void createPlayer(const MediaSource &source);
    bool openRecognizer();
    void setIAPIdFromNameL(const QString& iapString);

    // Audio / video media type recognition
    MediaType fileMediaType(const QString& fileName);
    MediaType bufferMediaType(const uchar *data, qint64 size);
    // TODO: urlMediaType function

    static qint64 toMilliSeconds(const TTimeIntervalMicroSeconds &);

private:

    // Audio / video media type recognition
    bool                                m_recognizerOpened;
    RApaLsSession                       m_recognizer;
    RFs                                 m_fileServer;

    MediaSource                         m_source;
    MediaSource                         m_nextSource;
    bool                                m_nextSourceSet;

    RFile*                              m_file;
    QResource*                          m_resource;

    QScopedPointer<AbstractPlayer>      m_player;
    int                                 m_iap;

};
}
}

QT_END_NAMESPACE

#endif
