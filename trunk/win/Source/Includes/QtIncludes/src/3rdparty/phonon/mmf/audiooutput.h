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

#ifndef PHONON_MMF_AUDIOOUTPUT_H
#define PHONON_MMF_AUDIOOUTPUT_H

#include <QHash>

#include "mmf_medianode.h"
#include <phonon/audiooutputinterface.h>

QT_BEGIN_NAMESPACE

namespace Phonon
{
namespace MMF
{
class Backend;

/**
 * @short AudioOutputInterface implementation for MMF.
 *
 * Forwards volume commands to the MediaObject instance,
 * which is provided by the backend when MediaNode objects are
 * connected.
 *
 * \section volume Volume
 *
 * Phonon's concept on volume is from 0.0 to 1.0, and from 1< it does
 * voltage multiplication. CDrmPlayerUtility goes from 1 to
 * CDrmPlayerUtility::MaxVolume(). We apply some basic math to convert
 * between the two.
 *
 * @author Frans Englich<frans.englich@nokia.com>
 */
class AudioOutput : public MediaNode
                  , public AudioOutputInterface
{
    Q_OBJECT
    Q_INTERFACES(Phonon::AudioOutputInterface)

public:
    AudioOutput(Backend *backend, QObject *parent);
    virtual qreal volume() const;
    virtual void setVolume(qreal volume);

    virtual int outputDevice() const;

    /**
     * Has no effect.
     */
    virtual bool setOutputDevice(int);

    static QHash<QByteArray, QVariant> audioOutputDescription(int index);

    enum Constants
    {
        AudioOutputDeviceID = 0
    };

protected:
    // MediaNode
    void connectMediaObject(MediaObject *mediaObject);
    void disconnectMediaObject(MediaObject *mediaObject);

Q_SIGNALS:
    void volumeChanged(qreal volume);
    void audioDeviceFailed();

private:
    qreal                           m_volume;

};
}
}

QT_END_NAMESPACE

#endif
