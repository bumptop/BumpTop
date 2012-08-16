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

#ifndef PHONON_MMF_VIDEOOUTPUT_DSA_H
#define PHONON_MMF_VIDEOOUTPUT_DSA_H

#include <QRect>

#include "phonon/mmf/abstractvideooutput.h"

QT_BEGIN_NAMESPACE

class QResizeEvent;
class QMoveEvent;

namespace Phonon
{
namespace MMF
{
class AncestorMoveMonitor;

/**
 * @short Widget on which video is displayed by DSA rendering
 *
 * This implementation is used on devices with the legacy graphics
 * subsystem, which does not support surfaces.  On such devices,
 * video rendering is done via Direct Screen Access (DSA), whereby
 * the video decoder writes directly to the framebuffer.  To ensure
 * that the window server and video decoder do not try to draw to
 * the same screen region at the same time, the video subsystem
 * first requests permission to perform DSA.  If the window server
 * needs to draw to this screen region (for example to display a
 * message dialog), it first notifies the video subsystem that it
 * must stop rendering to this region.
 *
 * @see SurfaceVideoOutput
 */
class DsaVideoOutput
    :   public AbstractVideoOutput
{
    Q_OBJECT

public:
    DsaVideoOutput(QWidget *parent);
    ~DsaVideoOutput();

    void setAncestorMoveMonitor(AncestorMoveMonitor *monitor);

    // Get absolute screen rectangle for video window
    const QRect& videoWindowScreenRect() const;

    // Called by AncestorMoveMonitor
    void ancestorMoved();

public Q_SLOTS:
    // Callbacks received from Symbian QtGui implementation, when it
    // begins / ends blitting the video widget's backing store to the
    // window server.
    void beginNativePaintEvent(const QRect & /*controlRect*/);
    void endNativePaintEvent(const QRect & /*controlRect*/);

Q_SIGNALS:
    void videoWindowScreenRectChanged();

    // Emitted when the Symbian QtGui implementation begins / ends
    // blitting the video widget's backing store to the window server.
    void beginVideoWindowNativePaint();
    void endVideoWindowNativePaint();

private:
    void getVideoWindowScreenRect();
    void registerForAncestorMoved();

    // QWidget
    void resizeEvent(QResizeEvent *event);
    void moveEvent(QMoveEvent *event);
    bool event(QEvent *event);

private:
    // Not owned
    AncestorMoveMonitor*    m_ancestorMoveMonitor;

    // Absolute screen rectangle on which video is displayed
    QRect                   m_videoWindowScreenRect;

};

}
}

QT_END_NAMESPACE

#endif // !PHONON_MMF_VIDEOOUTPUT_DSA_H

