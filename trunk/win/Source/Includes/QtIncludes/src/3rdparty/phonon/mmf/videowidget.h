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

#ifndef PHONON_MMF_VIDEOWIDGET_H
#define PHONON_MMF_VIDEOWIDGET_H

#include "abstractvideooutput.h"
#include "mmf_medianode.h"

#include <QtGui/QWidget>
#include <phonon/videowidget.h>
#include <phonon/videowidgetinterface.h>

QT_BEGIN_NAMESPACE

namespace Phonon
{
namespace MMF
{
#ifndef PHONON_MMF_VIDEO_SURFACES
class AncestorMoveMonitor;
#endif

class VideoWidget       :   public MediaNode
                        ,   public Phonon::VideoWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(Phonon::VideoWidgetInterface)

public:
    VideoWidget(QWidget* parent);
    ~VideoWidget();

#ifndef PHONON_MMF_VIDEO_SURFACES
    void setAncestorMoveMonitor(AncestorMoveMonitor *ancestorMoveMonitor);
#endif

    // VideoWidgetInterface
    virtual Phonon::VideoWidget::AspectRatio aspectRatio() const;
    virtual void setAspectRatio(Phonon::VideoWidget::AspectRatio aspectRatio);
    virtual qreal brightness() const;
    virtual void setBrightness(qreal brightness);
    virtual Phonon::VideoWidget::ScaleMode scaleMode() const;
    virtual void setScaleMode(Phonon::VideoWidget::ScaleMode scaleMode);
    virtual qreal contrast() const;
    virtual void setContrast(qreal constrast);
    virtual qreal hue() const;
    virtual void setHue(qreal hue);
    virtual qreal saturation() const;
    virtual void setSaturation(qreal saturation);
    virtual QWidget *widget();

protected:
    // MediaNode
    void connectMediaObject(MediaObject *mediaObject);
    void disconnectMediaObject(MediaObject *mediaObject);

private:
    QScopedPointer<AbstractVideoOutput>     m_videoOutput;

    qreal                                   m_brightness;
    qreal                                   m_contrast;
    qreal                                   m_hue;
    qreal                                   m_saturation;

};
}
}

QT_END_NAMESPACE

#endif
