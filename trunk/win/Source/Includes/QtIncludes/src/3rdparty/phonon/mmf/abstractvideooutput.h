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

#ifndef PHONON_MMF_ABSTRACTVIDEOOUTPUT_H
#define PHONON_MMF_ABSTRACTVIDEOOUTPUT_H

#include <QtGui/QWidget>
#include <QVector>
#include <QRect>
#include "defs.h"

#include <phonon/abstractvideooutput.h>
#include <phonon/videowidget.h>

#include <e32std.h>
class RWindowBase;

QT_BEGIN_NAMESPACE

namespace Phonon
{
namespace MMF
{

/**
 * @short ABC for widget on which video is displayed
 *
 * @see DsaVideoOutput, SurfaceVideoOutput
 */
class AbstractVideoOutput
    :   public QWidget
{
    Q_OBJECT

public:
    ~AbstractVideoOutput();

    // Set size of video frame.  Called by VideoPlayer.
    void setVideoSize(const QSize &size);

    RWindowBase* videoWindow() const;
    QSize videoWindowSize() const;

    Phonon::VideoWidget::AspectRatio aspectRatio() const;
    void setAspectRatio(Phonon::VideoWidget::AspectRatio aspectRatio);

    Phonon::VideoWidget::ScaleMode scaleMode() const;
    void setScaleMode(Phonon::VideoWidget::ScaleMode scaleMode);

    // Debugging output
    void dump() const;

Q_SIGNALS:
    void videoWindowChanged();
    void aspectRatioChanged();
    void scaleModeChanged();

protected:
    AbstractVideoOutput(QWidget *parent);

private:
    // QWidget
    QSize sizeHint() const;

private:
    // Dimensions of the video clip
    QSize                   m_videoFrameSize;

    Phonon::VideoWidget::AspectRatio        m_aspectRatio;
    Phonon::VideoWidget::ScaleMode          m_scaleMode;

};
}
}

QT_END_NAMESPACE

#endif
