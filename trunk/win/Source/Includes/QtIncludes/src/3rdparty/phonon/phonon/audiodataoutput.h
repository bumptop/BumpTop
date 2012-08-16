/*  This file is part of the KDE project
    Copyright (C) 2005-2006 Matthias Kretz <kretz@kde.org>

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
#ifndef Phonon_AUDIODATAOUTPUT_H
#define Phonon_AUDIODATAOUTPUT_H

#include "phonon_export.h"
#include "abstractaudiooutput.h"
#include "phonondefs.h"

QT_BEGIN_HEADER
QT_BEGIN_NAMESPACE

#ifndef DOXYGEN_SHOULD_SKIP_THIS
template<typename T> class QVector;
template<typename Key, typename T> class QMap;
#endif

namespace Phonon
{
    class AudioDataOutputPrivate;

    /**
     * \short This class gives you the audio data (for visualizations).
     *
     * This class implements a special AbstractAudioOutput that gives your
     * application the audio data. Don't expect realtime performance. But
     * the latencies should be low enough to use the audio data for
     * visualizations. You can also use the audio data for further processing
     * (e.g. encoding and saving to a file).
     *
     * \author Matthias Kretz <kretz@kde.org>
     */
    class PHONON_EXPORT AudioDataOutput : public AbstractAudioOutput
    {
        Q_OBJECT
        K_DECLARE_PRIVATE(AudioDataOutput)
        Q_ENUMS(Channel)
        Q_PROPERTY(int dataSize READ dataSize WRITE setDataSize)
        PHONON_HEIR(AudioDataOutput)
        public:
            /**
             * Specifies the channel the audio data belongs to.
             */
            enum Channel
            {
                LeftChannel,
                RightChannel,
                CenterChannel,
                LeftSurroundChannel,
                RightSurroundChannel,
                SubwooferChannel
            };

            /**
             * Returns the currently used number of samples passed through
             * the signal.
             *
             * \see setDataSize
             */
            int dataSize() const;

            /**
             * Returns the sample rate in Hz. Common sample rates are 44100 Hz
             * and 48000 Hz. AudioDataOutput will not do any sample rate
             * conversion for you. If you need to convert the sample rate you
             * might want to take a look at libsamplerate. For visualizations it
             * is often enough to do simple interpolation or even drop/duplicate
             * samples.
             *
             * \return The sample rate as reported by the backend. If the
             * backend is unavailable -1 is returned.
             */
            int sampleRate() const;

        public Q_SLOTS:
            /**
             * Sets the number of samples to be passed in one signal emission.
             *
             * Defaults to 512 samples per emitted signal.
             *
             * \param size the number of samples
             */
            void setDataSize(int size);

        Q_SIGNALS:
            /**
             * Emitted whenever another dataSize number of samples are ready.
             *
             * \param data A mapping of Channel to a vector holding the audio data.
             */
            void dataReady(const QMap<Phonon::AudioDataOutput::Channel, QVector<qint16> > &data);


            /**
             * This signal is emitted before the last dataReady signal of a
             * media is emitted.
             *
             * If, for example, the playback of a media file has finished and the
             * last audio data of that file is going to be passed with the next
             * dataReady signal, and only the 28 first samples of the data
             * vector are from that media file endOfMedia will be emitted right
             * before dataReady with \p remainingSamples = 28.
             *
             * \param remainingSamples The number of samples in the next
             * dataReady vector that belong to the media that was playing to
             * this point.
             */
            void endOfMedia(int remainingSamples);
    };
} // namespace Phonon

QT_END_NAMESPACE
QT_END_HEADER

// vim: sw=4 ts=4 tw=80
#endif // Phonon_AUDIODATAOUTPUT_H
