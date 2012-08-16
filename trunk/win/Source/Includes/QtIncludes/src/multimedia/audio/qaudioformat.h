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


#ifndef QAUDIOFORMAT_H
#define QAUDIOFORMAT_H

#include <QtCore/qobject.h>
#include <QtCore/qglobal.h>
#include <QtCore/qshareddata.h>


QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Multimedia)


class QAudioFormatPrivate;

class Q_MULTIMEDIA_EXPORT QAudioFormat
{
public:
    enum SampleType { Unknown, SignedInt, UnSignedInt, Float };
    enum Endian { BigEndian = QSysInfo::BigEndian, LittleEndian = QSysInfo::LittleEndian };

    QAudioFormat();
    QAudioFormat(const QAudioFormat &other);
    ~QAudioFormat();

    QAudioFormat& operator=(const QAudioFormat &other);
    bool operator==(const QAudioFormat &other) const;
    bool operator!=(const QAudioFormat &other) const;

    bool isValid() const;

    void setFrequency(int frequency);
    int frequency() const;
    void setSampleRate(int sampleRate);
    int sampleRate() const;

    void setChannels(int channels);
    int channels() const;
    void setChannelCount(int channelCount);
    int channelCount() const;

    void setSampleSize(int sampleSize);
    int sampleSize() const;

    void setCodec(const QString &codec);
    QString codec() const;

    void setByteOrder(QAudioFormat::Endian byteOrder);
    QAudioFormat::Endian byteOrder() const;

    void setSampleType(QAudioFormat::SampleType sampleType);
    QAudioFormat::SampleType sampleType() const;

private:
    QSharedDataPointer<QAudioFormatPrivate> d;
};


QT_END_NAMESPACE

QT_END_HEADER

#endif  // QAUDIOFORMAT_H
