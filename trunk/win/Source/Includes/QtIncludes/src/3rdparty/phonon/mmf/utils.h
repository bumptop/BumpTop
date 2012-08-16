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

#ifndef PHONON_MMF_UTILS_H
#define PHONON_MMF_UTILS_H

#include <private/qcore_symbian_p.h>
#include <e32debug.h>   // for RDebug
#include <QtCore/QCoreApplication> // for Q_DECLARE_TR_FUNCTIONS
#include <QColor>

#include "defs.h"

QT_BEGIN_NAMESPACE

namespace Phonon
{
namespace MMF
{
/**
 * Panic codes for fatal errors
 */
enum PanicCode {
    InvalidStatePanic               = 1,
    InvalidMediaTypePanic           = 2,
    InvalidBackendInterfaceClass    = 3,
    AudioUtilityUrlNotSupported     = 4
};

class Utils
{
    Q_DECLARE_TR_FUNCTIONS(Phonon::MMF)

public:
/**
 * Raise a fatal exception
 */
static void panic(PanicCode code);

/**
 * Determines whether the provided MIME type is an audio or video
 * type.  If it is neither, the function returns MediaTypeUnknown.
 */
static MediaType mimeTypeToMediaType(const TDesC& mimeType);

/**
 * Translates a Symbian error code into a user-readable string.
 */
static QString symbianErrorToString(int errorCode);

#ifndef QT_NO_DEBUG
/**
 * Retrieve color of specified pixel from the screen.
 */
static QColor getScreenPixel(const QPoint& pos);

/**
 * Samples a small number of pixels from the screen, and dumps their
 * colors to the debug log.
 */
static void dumpScreenPixelSample();
#endif
};

/**
 * Available trace categories;
 */
enum TTraceCategory {
    /**
     * Backend
     */
    EBackend             = 0x00000001,

    /**
     * Functions which map directly to the public Phonon audio API
     */
    EAudioApi            = 0x00000010,

    /**
     * Internal functions in the audio implementation
     */
    EAudioInternal       = 0x00000020,

    /**
     * Functions which map directly to the public Phonon video API
     */
    EVideoApi            = 0x00010000,

    /**
     * Internal functions in the video implementation
     */
    EVideoInternal       = 0x00020000
};

/**
 * Mask indicating which trace categories are enabled
 *
 * Note that, at the moment, this is a compiled static constant.  For
 * runtime control over enabled trace categories, this could be replaced
 * by a per-thread singleton object which owns the trace mask, and which
 * exposes an API allowing it to be modified.
 */
static const TUint KTraceMask = 0xffffffff;

/**
 * Data structure used by tracing macros
 */
class TTraceContext
{
public:
    TTraceContext(const TText* aFunction, const TUint aAddr,
                  const TUint aCategory = 0)
            :    iFunction(aFunction),
            iAddr(aAddr),
            iCategory(aCategory) { }

    /**
     * Check whether iCategory appears in the trace mask
     */
    TBool Enabled() const {
        return (iCategory == 0) or(iCategory & KTraceMask);
    }

    const TText*    iFunction;    // Name of function
    const TUint        iAddr;        // 'this' pointer
    const TUint        iCategory;
};

// Macros used internally by the trace system
#define _TRACE_PRINT RDebug::Print
#define _TRACE_TEXT(x) (TPtrC((const TText *)(x)))
#define _TRACE_MODULE Phonon::MMF

// Macros available for use by implementation code
#ifndef QT_NO_DEBUG
#define TRACE_CONTEXT(_fn, _cat) const ::Phonon::MMF::TTraceContext _tc((TText*)L ## #_fn, (TUint)this, _cat);
#define TRACE_ENTRY_0() { if (_tc.Enabled()) _TRACE_PRINT(_TRACE_TEXT(L ## "+ Phonon::MMF::%s [0x%08x]"), _tc.iFunction, _tc.iAddr); }
#define TRACE_ENTRY(string, args...) { if (_tc.Enabled()) _TRACE_PRINT(_TRACE_TEXT(L ## "+ Phonon::MMF::%s [0x%08x] " L ## string), _tc.iFunction, _tc.iAddr, args); }
#define TRACE_EXIT_0() { if (_tc.Enabled()) _TRACE_PRINT(_TRACE_TEXT(L ## "- Phonon::MMF::%s [0x%08x]"), _tc.iFunction, _tc.iAddr); }
#define TRACE_EXIT(string, args...) { if (_tc.Enabled()) _TRACE_PRINT(_TRACE_TEXT(L ## "- Phonon::MMF::%s [0x%08x] " L ## string), _tc.iFunction, _tc.iAddr, args); }
#define TRACE_RETURN(string, result) { if (_tc.Enabled()) _TRACE_PRINT(_TRACE_TEXT(L ## "r Phonon::MMF::%s [0x%08x] " L ## string), _tc.iFunction, _tc.iAddr, result); } return result;
#define TRACE_PANIC(code) { _TRACE_PRINT(_TRACE_TEXT(L ## "! Phonon::MMF::%s [0x%08x] panic %d"), _tc.iFunction, _tc.iAddr, code); } Utils::panic(code);
#define TRACE_0(string) { if (_tc.Enabled()) _TRACE_PRINT(_TRACE_TEXT(L ## "  Phonon::MMF::%s [0x%08x] " L ## string), _tc.iFunction, _tc.iAddr); }
#define TRACE(string, args...) { if (_tc.Enabled()) _TRACE_PRINT(_TRACE_TEXT(L ## "  Phonon::MMF::%s [0x%08x] " L ## string), _tc.iFunction, _tc.iAddr, args); }
#else
#define TRACE_CONTEXT(_fn, _cat)
#define TRACE_ENTRY_0()
#define TRACE_ENTRY(string, args...)
#define TRACE_EXIT_0()
#define TRACE_EXIT(string, args...)
#define TRACE_RETURN(string, result) return result;
#define TRACE_PANIC(code) Utils::panic(code);
#define TRACE_0(string)
#define TRACE(string, args...)
#endif
}
}

QT_END_NAMESPACE

#endif
