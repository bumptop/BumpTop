/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtMultimedia module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifndef QAUDIO_SYMBIAN_P_H
#define QAUDIO_SYMBIAN_P_H

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtMultimedia/qaudioformat.h>
#include <QtMultimedia/qaudio.h>
#include <sounddevice.h>

QT_BEGIN_NAMESPACE

namespace SymbianAudio {

/**
 * Default values used by audio input and output classes, when underlying
 * DevSound instance has not yet been created.
 */

const int DefaultBufferSize = 4096; // bytes
const int DefaultNotifyInterval = 1000; // ms

/**
 * Enumeration used to track state of internal DevSound instances.
 * Values are translated to the corresponding QAudio::State values by
 * SymbianAudio::Utils::stateNativeToQt.
 */
enum State {
        ClosedState
    ,   InitializingState
    ,   ActiveState
    ,   IdleState
    // QAudio is suspended; DevSound is paused
    ,   SuspendedPausedState
    // QAudio is suspended; DevSound is stopped
    ,   SuspendedStoppedState
};

/**
 * Wrapper around DevSound instance
 */
class DevSoundWrapper
    :   public QObject
    ,   public MDevSoundObserver
{
    Q_OBJECT

public:
    DevSoundWrapper(QAudio::Mode mode, QObject *parent = 0);
    ~DevSoundWrapper();

public:
    // List of supported codecs; can be called once object is constructed
    const QList<QString>& supportedCodecs() const;

    // Asynchronous initialization function; emits devsoundInitializeComplete
    void initialize(const QString& codec);

    // Capabilities, for selected codec.  Can be called once initialize has returned
    // successfully.
    const QList<int>& supportedFrequencies() const;
    const QList<int>& supportedChannels() const;
    const QList<int>& supportedSampleSizes() const;
    const QList<QAudioFormat::Endian>& supportedByteOrders() const;
    const QList<QAudioFormat::SampleType>& supportedSampleTypes() const;

    bool isFormatSupported(const QAudioFormat &format) const;

    int samplesProcessed() const;
    bool setFormat(const QAudioFormat &format);
    bool start();

    // If DevSound implementation supports pause, calls pause and returns true.
    // Otherwise calls stop and returns false.  In this case, all DevSound buffers
    // currently held by the backend must be discarded.
    bool pause();

    void resume();

    void stop();
    void bufferProcessed();

public:
    // MDevSoundObserver
    void InitializeComplete(TInt aError);
    void ToneFinished(TInt aError);
    void BufferToBeFilled(CMMFBuffer *aBuffer);
    void PlayError(TInt aError);
    void BufferToBeEmptied(CMMFBuffer *aBuffer);
    void RecordError(TInt aError);
    void ConvertError(TInt aError);
    void DeviceMessage(TUid aMessageType, const TDesC8 &aMsg);

signals:
    void initializeComplete(int error);
    void bufferToBeProcessed(CMMFBuffer *buffer);
    void processingError(int error);

private:
    void getSupportedCodecs();
    void populateCapabilities();
    bool isResumeSupported() const;

private:
    const QAudio::Mode              m_mode;
    TMMFState                       m_nativeMode;

    enum State {
        StateIdle,
        StateInitializing,
        StateInitialized
    }                               m_state;

    CMMFDevSound*                   m_devsound;
    TFourCC                         m_fourcc;

    QList<QString>                  m_supportedCodecs;
    QList<int>                      m_supportedFrequencies;
    QList<int>                      m_supportedChannels;
    QList<int>                      m_supportedSampleSizes;
    QList<QAudioFormat::Endian>     m_supportedByteOrders;
    QList<QAudioFormat::SampleType> m_supportedSampleTypes;

};


namespace Utils {

/**
 * Convert internal states to QAudio states.
 */
QAudio::State stateNativeToQt(State nativeState);

/**
 * Convert data length to number of samples.
 */
qint64 bytesToSamples(const QAudioFormat &format, qint64 length);

/**
 * Convert number of samples to data length.
 */
qint64 samplesToBytes(const QAudioFormat &format, qint64 samples);

} // namespace Utils
} // namespace SymbianAudio

QT_END_NAMESPACE

#endif
