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

#ifndef PHONON_MMF_EFFECTPARAMETER_H
#define PHONON_MMF_EFFECTPARAMETER_H

#include <phonon/effectparameter.h>

QT_BEGIN_NAMESPACE

namespace Phonon
{
namespace MMF
{

/**
 * @short Parameter value for an audio effect
 *
 * The base class is extended in order to work around a shortcoming
 * in Phonon::EffectWidget.  This widget only displays sliders for
 * parameters with numeric values if the variant type of the parameter
 * is QReal and the range is exactly -1.0 to +1.0; otherwise, a
 * spinbox is used to set numeric parameters.  This is rather
 * inconvenient for many effects, such as the audio equalizer, for
 * which a slider is a much more natural UI control.
 *
 * For many such parameters, we therefore report the type to be QReal
 * and the range to be -1.0 to +1.0.  This class stores the actual
 * integer range for the parameter, and provides the toInternalValue
 * function for converting between the client-side floating point
 * value and the internal integer value.
 */
class EffectParameter : public Phonon::EffectParameter
{
public:
    EffectParameter();
    EffectParameter(int parameterId, const QString &name, Hints hints,
                    const QVariant &defaultValue, const QVariant &min = QVariant(),
                    const QVariant &max = QVariant(), const QVariantList &values = QVariantList(),
                    const QString &description = QString());

    void setInternalRange(qint32 min, qint32 max);
    qint32 toInternalValue(qreal external) const;

    static qreal toExternalValue(qint32 value, qint32 min, qint32 max);

private:
    bool                    m_hasInternalRange;
    QPair<qint32, qint32>   m_internalRange;

};

}
}

QT_END_NAMESPACE

#endif

