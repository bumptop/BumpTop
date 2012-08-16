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

#ifndef PHONON_MMF_ABSTRACTPLAYER_H
#define PHONON_MMF_ABSTRACTPLAYER_H

#include <phonon/phononnamespace.h>
#include <phonon/mediasource.h>

#include <QObject>

#include "abstractvideooutput.h"

QT_BEGIN_NAMESPACE

namespace Phonon
{
namespace MMF
{

/**
 * @short Interface which abstracts from MediaObject the current
 * media type
 *
 * This may be:
 *  -   Nothing, in which case this interface is implemented by
 *      DummyPlayer
 *  -   Audio, in which case the implementation is AudioPlayer
 *  -   Video, in which case the implementation is VideoPlayer
 */
class AbstractPlayer : public QObject
{
    // Required although this class has no signals or slots
    // Without this, qobject_cast will fail
    Q_OBJECT

public:
    AbstractPlayer(const AbstractPlayer *player);

    virtual void open() = 0;
    virtual void close() = 0;

    // MediaObjectInterface (implemented)
    qint32 tickInterval() const;
    void setTickInterval(qint32);
    void setTransitionTime(qint32);
    qint32 transitionTime() const;
    void setPrefinishMark(qint32);
    qint32 prefinishMark() const;

    // MediaObjectInterface (abstract)
    virtual void play() = 0;
    virtual void pause() = 0;
    virtual void stop() = 0;
    virtual void seek(qint64 milliseconds) = 0;
    virtual bool hasVideo() const = 0;
    virtual bool isSeekable() const = 0;
    virtual qint64 currentTime() const = 0;
    virtual Phonon::ErrorType errorType() const;
    virtual QString errorString() const;
    virtual qint64 totalTime() const = 0;

    virtual void volumeChanged(qreal volume);

    void setVideoOutput(AbstractVideoOutput *videoOutput);

    /**
     * Records error message and changes state to ErrorState
     */
    void setError(const QString &errorMessage);

    /**
     * Records error message and changes state to ErrorState
     *
     * Appends a human-readable version of symbianErrorCode to the error message,
     * e.g.
     * @code
     *      setError("Opening file failed", KErrPermissionDenied)
     * @endcode
     * results in the following error message:
     *      "Opening file failed: permission denied"
     */
    void setError(const QString &errorMessage, int symbianErrorCode);

    Phonon::State state() const;

Q_SIGNALS:
    void totalTimeChanged(qint64 length);
    void finished();
    void tick(qint64 time);
    void bufferStatus(int percentFilled);
    void stateChanged(Phonon::State newState,
                      Phonon::State oldState);
    void metaDataChanged(const QMultiMap<QString, QString>& metaData);
    void aboutToFinish();
    void prefinishMarkReached(qint32 remaining);

protected:
    /**
     * Defined private state enumeration in order to add GroundState
     */
    enum PrivateState {
        LoadingState    = Phonon::LoadingState,
        StoppedState    = Phonon::StoppedState,
        PlayingState    = Phonon::PlayingState,
        BufferingState  = Phonon::BufferingState,
        PausedState     = Phonon::PausedState,
        ErrorState      = Phonon::ErrorState,
        GroundState
    };

    /**
     * Converts PrivateState into the corresponding Phonon::State
     */
    Phonon::State phononState() const;

    /**
     * Converts PrivateState into the corresponding Phonon::State
     */
    virtual Phonon::State phononState(PrivateState state) const;

    virtual void videoOutputChanged();

    PrivateState privateState() const;

    /**
     * Changes state and emits stateChanged()
     */
    virtual void changeState(PrivateState newState);

    /**
     * Modifies m_state directly. Typically you want to call changeState(),
     * which performs the business logic.
     */
    void setState(PrivateState newState);

private:
    virtual void doSetTickInterval(qint32 interval) = 0;

protected:
    // Not owned
    AbstractVideoOutput*        m_videoOutput;

    qreal                       m_volume;

private:
    PrivateState                m_state;
    Phonon::ErrorType           m_error;
    QString                     m_errorString;
    qint32                      m_tickInterval;
    qint32                      m_transitionTime;
    qint32                      m_prefinishMark;

};
}
}

QT_END_NAMESPACE

#endif

