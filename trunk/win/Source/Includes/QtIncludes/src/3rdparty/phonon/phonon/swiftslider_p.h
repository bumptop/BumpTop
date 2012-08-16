/*  This file is part of the KDE project
    Copyright (C) 2006-2008 Ricardo Villalba <rvm@escomposlinux.org>

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

#ifndef SWIFTSLIDER_H
#define SWIFTSLIDER_H

#include <QtGui/QSlider>

QT_BEGIN_NAMESPACE

#if !defined(QT_NO_PHONON_SEEKSLIDER) && !defined(QT_NO_PHONON_VOLUMESLIDER)

namespace Phonon
{

/** \class SwiftSlider swiftslider_p.h phonon/SwiftSlider
 * \short Modified QSlider that allows sudden/quick moves instead of stepped moves ("Click'n'Go" QSlider)
 *
 * This is an internal class used by SeekSlider and VolumeSlider.
 *
 * Ricardo Villalba, the original author of MySlider.cpp (from the SMPlayer project)
 * gave his permission for the inclusion of this code inside Phonon by
 * switching MySlider.cpp to the LGPLv2.1+ license.
 * See http://smplayer.svn.sourceforge.net/viewvc/smplayer/smplayer/trunk/src/myslider.cpp?revision=2406&view=markup
 *
 * The original discussion about a "Click'n'Go QSlider": http://lists.trolltech.com/qt-interest/2006-11/msg00363.html
 *
 * \ingroup PhononWidgets
 */
class SwiftSlider : public QSlider
{
	Q_OBJECT
public:
	SwiftSlider(Qt::Orientation orientation, QWidget * parent);
	~SwiftSlider();

private:
	void mousePressEvent(QMouseEvent *event);
	inline int pick(const QPoint &pt) const;
	int pixelPosToRangeValue(int pos) const;
};

} // namespace Phonon

#endif //QT_NO_PHONON_VOLUMESLIDER && QT_NO_PHONON_VOLUMESLIDER

QT_END_NAMESPACE

#endif //SWIFTSLIDER_H
