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

#ifndef PHONON_MMF_ABSTRACTEFFECT_H
#define PHONON_MMF_ABSTRACTEFFECT_H

#include <QScopedPointer>

#include <AudioEffectBase.h>

#include <phonon/effectinterface.h>

#include "audioplayer.h"
#include "effectparameter.h"
#include "mmf_medianode.h"

class CMdaAudioOutputStream;

QT_BEGIN_NAMESPACE

namespace Phonon
{
namespace MMF
{
class AbstractPlayer;
class AbstractMediaPlayer;

/**
 * @short Base class for all effects for MMF.
 *
 * Adhering to Phonon with MMF is cumbersome because of a number of reasons:
 *
 * - MMF has no concept of effect chaining. Simply, an effect is a applied
 *   to PlayerUtility, that's it. This means that the order of effects is
 *   undefined.
 * - We apply an effect to a PlayerUtility, and MediaObject has that one.
 *   However, effects might be created before MediaObject, but nevertheless
 *   needs to work. We solve this by that we are aware of the whole connection
 *   chain, and whenever a connection happens, we walk the chain, find effects
 *   that aren't applied, and apply it if we have a media object.
 * - There are plenty of corner cases which we don't handle and where behavior
 *   are undefined. For instance, graphs with more than one MediaObject.
 */
class AbstractAudioEffect : public MediaNode
                          , public EffectInterface
{
    Q_OBJECT
    Q_INTERFACES(Phonon::EffectInterface)
public:
    AbstractAudioEffect(QObject *parent,
                        const QList<EffectParameter> &params);

    // Phonon::EffectInterface
    virtual QList<Phonon::EffectParameter> parameters() const;
    virtual QVariant parameterValue(const Phonon::EffectParameter &param) const;
    virtual void setParameterValue(const Phonon::EffectParameter &,
                                   const QVariant &newValue);

    // Parameters which are shared by all effects
    enum CommonParameters
    {
        ParameterEnable = 0,
        ParameterBase // must be last entry in enum
    };

public Q_SLOTS:
    void abstractPlayerChanged(AbstractPlayer *player);
    void stateChanged(Phonon::State newState,
                      Phonon::State oldState);

protected:
    // MediaNode
    void connectMediaObject(MediaObject *mediaObject);
    void disconnectMediaObject(MediaObject *mediaObject);

    virtual void createEffect(AudioPlayer::NativePlayer *player) = 0;

    // Effect-specific parameter changed
    virtual int effectParameterChanged(const EffectParameter &param,
                                  const QVariant &value);

private:
    void createEffect();
    void setEnabled(bool enabled);
    const EffectParameter& internalParameter(int id) const;
    int parameterChanged(const EffectParameter &param,
            const QVariant &value);

protected:
    QScopedPointer<CAudioEffect>    m_effect;

private:
    const QList<EffectParameter>    m_params;
    AbstractMediaPlayer *           m_player;
    QHash<int, QVariant>            m_values;
};

}
}


// Macro for defining functions which depend on the native class name
// for each of the effects.  Using this reduces repetition of boilerplate
// in the implementations of the backend effect nodes.

#ifdef Q_CC_NOKIAX86
#   pragma warn_illtokenpasting off
#endif

#define PHONON_MMF_DEFINE_EFFECT_FUNCTIONS(Effect)                      \
                                                                        \
void Effect::createEffect(AudioPlayer::NativePlayer *player)            \
{                                                                       \
    C##Effect *ptr = 0;                                                 \
    QT_TRAP_THROWING(ptr = C##Effect::NewL(*player));                   \
    m_effect.reset(ptr);                                                \
}                                                                       \
                                                                        \
C##Effect* Effect::concreteEffect()                                     \
{                                                                       \
    return static_cast<C##Effect *>(m_effect.data());                   \
}

QT_END_NAMESPACE

#endif

