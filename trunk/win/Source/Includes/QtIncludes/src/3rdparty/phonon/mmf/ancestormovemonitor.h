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

#ifndef PHONON_MMF_ANCESTORMOVEMONITOR_H
#define PHONON_MMF_ANCESTORMOVEMONITOR_H

#include <QObject>
#include <QHash>
#include <QList>

QT_BEGIN_NAMESPACE

namespace Phonon
{
namespace MMF
{
class DsaVideoOutput;

class AncestorMoveMonitor : public QObject
{
    Q_OBJECT

public:
    explicit AncestorMoveMonitor(QObject *parent);
    ~AncestorMoveMonitor();

    /**
     * Register target widget for notification.
     *
     * The widget receives an ancestorMoveEvent callback when a move event
     * is delivered to any of its ancestors:
     *
     * If the target is already registered, this function causes its
     * ancestor list to be updated - therefore it should be called when
     * the target receives a ParentChange event.
     */
    void registerTarget(DsaVideoOutput *target);

    /**
     * Remove target from the monitor.
     *
     * The target will no longer receive notification when move events are
     * delivered to its ancestors.
     */
    void unRegisterTarget(DsaVideoOutput *target);

protected:
    /**
     * Function which receives events from the global event filter.
     */
    bool eventFilter(QObject *watched, QEvent *event);

    void dump();

private:
    /**
     * List of registered target widgets which descend from a given
     * ancestor.
     *
     * Note that the members of the list should be non-redundant; this
     * invariant is checked in debug builds.  Semantically, the value is
     * therefore a set, however we use QList rather than QSet for
     * efficiency of iteration.
     */
    typedef QList<DsaVideoOutput *> TargetList;

    /**
     * Map from widget on which the move event occurs, to widgets which
     * descend from it and therefore need to be notified.
     */
    typedef QHash<QObject *, TargetList> Hash;
    Hash m_hash;

};
}
}

QT_END_NAMESPACE

#endif // !PHONON_MMF_ANCESTORMOVEMONITOR_H
