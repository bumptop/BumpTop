/*
 *  Modern-macOS port prefix header, force-included into every TU.
 *  Replaces OSX/PrecompiledHeaders.pch (which force-defined __OBJC__ for
 *  Ogre 1.7 and pulled in Carbon; neither applies to the Ogre 14 port).
 */

#ifndef BUMPTOP_PORT_PREFIX_H_
#define BUMPTOP_PORT_PREFIX_H_

#ifdef __OBJC__
#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>
#endif

#ifdef __cplusplus
#include <mach/mach.h>
#include <mach/mach_time.h>

#include "BumpTop/boost_includes.h"
#include "BumpTop/bullet_includes.h"
#include "BumpTop/ogre_includes.h"

// Ogre 1.7 exposed these integer typedefs at global scope; the code relies on
// that, so re-export them from the Ogre namespace.
using Ogre::uint8;
using Ogre::uint16;
using Ogre::uint32;
using Ogre::uint64;
#include "BumpTop/protobuf_includes.h"
#include "BumpTop/qtcore_includes.h"
#include "BumpTop/qtgui_includes.h"

#include "BumpTop/for_each.h"

// Qt 4's qSort was removed in Qt 6; provide equivalents.
#include <algorithm>
template <typename Container>
inline void qSort(Container& c) { std::sort(c.begin(), c.end()); }
template <typename Iterator>
inline void qSort(Iterator b, Iterator e) { std::sort(b, e); }
template <typename Iterator, typename LessThan>
inline void qSort(Iterator b, Iterator e, LessThan lt) { std::sort(b, e, lt); }
#endif

#endif  // BUMPTOP_PORT_PREFIX_H_
