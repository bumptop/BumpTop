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

#ifndef QAUDIODEVICEINFO_SYMBIAN_P_H
#define QAUDIODEVICEINFO_SYMBIAN_P_H

#include <QtCore/QMap>
#include <QtMultimedia/qaudioengine.h>
#include <sounddevice.h>

QT_BEGIN_NAMESPACE

namespace SymbianAudio {
class DevSoundWrapper;
}

class QAudioDeviceInfoInternal
    :   public QAbstractAudioDeviceInfo
{
    Q_OBJECT

public:
    QAudioDeviceInfoInternal(QByteArray device, QAudio::Mode mode);
    ~QAudioDeviceInfoInternal();

    // QAbstractAudioDeviceInfo
    QAudioFormat preferredFormat() const;
    bool isFormatSupported(const QAudioFormat &format) const;
    QAudioFormat nearestFormat(const QAudioFormat &format) const;
    QString deviceName() const;
    QStringList codecList();
    QList<int> frequencyList();
    QList<int> channelsList();
    QList<int> sampleSizeList();
    QList<QAudioFormat::Endian> byteOrderList();
    QList<QAudioFormat::SampleType> sampleTypeList();
    static QByteArray defaultInputDevice();
    static QByteArray defaultOutputDevice();
    static QList<QByteArray> availableDevices(QAudio::Mode);

private slots:
    void devsoundInitializeComplete(int err);

private:
    void getSupportedFormats() const;

private:
    mutable bool m_initializing;
    int m_intializationResult;

    QString m_deviceName;
    QAudio::Mode m_mode;

    struct Capabilities
    {
        QList<int> m_frequencies;
        QList<int> m_channels;
        QList<int> m_sampleSizes;
        QList<QAudioFormat::Endian> m_byteOrders;
        QList<QAudioFormat::SampleType> m_sampleTypes;
    };

    // Mutable to allow lazy initialization when called from const-qualified
    // public functions (isFormatSupported, nearestFormat)
    mutable bool m_updated;
    mutable QMap<QString, Capabilities> m_capabilities;
    mutable Capabilities m_unionCapabilities;
};

QT_END_NAMESPACE

#endif
