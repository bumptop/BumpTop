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

#ifndef PHONON_MMF_MEDIANODE_H
#define PHONON_MMF_MEDIANODE_H

#include <QObject>
#include <phonon/effectinterface.h>
#include "audioplayer.h"

QT_BEGIN_NAMESPACE

/**
 * @file mmf_medianode.h mmf_medianode.cpp
 *
 * This file starts with mmf_ in order to avoid clash with Phonon's
 * medianode.h. The GStreamer backend has a file named medianode.h, but it
 * isn't compiled with ABLD build system, which have problems with separating
 * user and system include paths.
 */

namespace Phonon
{
namespace MMF
{
class MediaObject;

/**
 * @short Base class for all nodes in the MMF backend.
 *
 * MediaNode is the base class for all nodes created by the MMF
 * backend.
 *
 * These nodes may be one of the following types:
 *
 * - MediaObject
 *      This represents the source of media data.  It encapsulates the
 *      appropriate MMF client API for playing audio or video.
 * - AudioOutput
 *      This represents the audio output device.  Since the MMF client API
 *      does not expose the output device directly, this backend node
 *      simply forwards volume control commands to the MediaObject.
 * - VideoWidget
 *      A native widget on which video will be rendered.
 * - An audio effect, derived form AbstractAudioEffect
 *
 * Because the MMF API does not support the concept of a media filter graph,
 * this class must ensure the following:
 *
 * - Each media graph contains at most one MediaObject instance.
 * - Every non-MediaObject node holds a reference to the MediaObject.  This
 * allows commands to be sent through the graph to the encapsulated MMF client
 * API.
 */
class MediaNode : public QObject
{
    Q_OBJECT
public:
    MediaNode(QObject *parent);
    ~MediaNode();

    bool connectOutput(MediaNode *output);
    bool disconnectOutput(MediaNode *output);

    virtual void connectMediaObject(MediaObject *mediaObject) = 0;
    virtual void disconnectMediaObject(MediaObject *mediaObject) = 0;

private:
    bool isMediaObject() const;

    void updateMediaObject();
    void setMediaObject(MediaObject *mediaObject);

    typedef QList<const MediaNode *> NodeList;
    void visit(QList<MediaNode *>& visited, MediaObject*& mediaObject);

private:
    MediaObject *       m_mediaObject;

    // All nodes except MediaObject may have an input
    MediaNode *         m_input;

    // Only MediaObject can have more than one output
    QList<MediaNode *>  m_outputs;
};

}
}

QT_END_NAMESPACE

#endif

