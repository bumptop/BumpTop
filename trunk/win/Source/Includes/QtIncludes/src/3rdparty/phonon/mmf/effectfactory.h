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

#ifndef PHONON_MMF_EFFECTFACTORY_H
#define PHONON_MMF_EFFECTFACTORY_H

#include "abstractaudioeffect.h"
#include "effectparameter.h"

QT_BEGIN_NAMESPACE

namespace Phonon
{
namespace MMF
{

/**
 * @short Contains utility functions related to effects.
 */
class EffectFactory : public QObject
{
    Q_OBJECT

public:
    EffectFactory(QObject *parent);
    ~EffectFactory();

    enum Type
    {
        TypeAudioEqualizer = 0
    ,   TypeBassBoost
    ,   TypeDistanceAttenuation
    ,   TypeEnvironmentalReverb
    ,   TypeListenerOrientation
    ,   TypeLoudness
    ,   TypeSourceOrientation
    ,   TypeStereoWidening
    };

    /**
     * @short Creates an audio effect of type @p type.
     */
    AbstractAudioEffect *createAudioEffect(Type type, QObject *parent);

    /**
     * @short Return the properties for effect @p type.
     *
     * This handles the effects for
     * BackendInterface::objectDescriptionProperties().
     */
    QHash<QByteArray, QVariant> audioEffectDescriptions(Type type);

    /**
     * @short Returns the indexes for the supported effects.
     *
     * This handles the effects for
     * BackendInterface::objectDescriptionIndexes().
     */
    QList<int> effectIndexes();

private:
    void initialize();

    struct EffectData
    {
        bool                            m_supported;
        QHash<QByteArray, QVariant>     m_descriptions;
        QList<EffectParameter>          m_parameters;
    };

    template<typename BackendNode> EffectData getData();
    const EffectData& data(Type type) const;

private:
    bool                                m_initialized;
    QHash<Type, EffectData>             m_effectData;

};

}
}

QT_END_NAMESPACE

#endif

