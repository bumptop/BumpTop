/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

/****************************************************************************
**
** Copyright (c) 2007-2008, Apple, Inc.
**
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**
**   * Redistributions of source code must retain the above copyright notice,
**     this list of conditions and the following disclaimer.
**
**   * Redistributions in binary form must reproduce the above copyright notice,
**     this list of conditions and the following disclaimer in the documentation
**     and/or other materials provided with the distribution.
**
**   * Neither the name of Apple, Inc. nor the names of its contributors
**     may be used to endorse or promote products derived from this software
**     without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
** CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
** EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
** PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
** PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
** LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
** NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
** SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
****************************************************************************/

#include <qglobal.h>
#ifdef QT_MAC_USE_COCOA
#include <private/qcocoaapplication_mac_p.h>
#include <private/qcocoaapplicationdelegate_mac_p.h>
#include <private/qt_cocoa_helpers_mac_p.h>

QT_USE_NAMESPACE

@implementation NSApplication (QT_MANGLE_NAMESPACE(QApplicationIntegration))

- (void)QT_MANGLE_NAMESPACE(qt_setDockMenu):(NSMenu *)newMenu
{
    [[QT_MANGLE_NAMESPACE(QCocoaApplicationDelegate) sharedDelegate] setDockMenu:newMenu];
}

- (QApplicationPrivate *)QT_MANGLE_NAMESPACE(qt_qappPrivate)
{
    return [[QT_MANGLE_NAMESPACE(QCocoaApplicationDelegate) sharedDelegate] qAppPrivate];
}

- (QT_MANGLE_NAMESPACE(QCocoaMenuLoader) *)QT_MANGLE_NAMESPACE(qt_qcocoamenuLoader)
{
    return [[QT_MANGLE_NAMESPACE(QCocoaApplicationDelegate) sharedDelegate] menuLoader];
}

- (int)QT_MANGLE_NAMESPACE(qt_validModesForFontPanel):(NSFontPanel *)fontPanel
{
    Q_UNUSED(fontPanel);
    // only display those things that QFont can handle
    return NSFontPanelFaceModeMask
            | NSFontPanelSizeModeMask
            | NSFontPanelCollectionModeMask
            | NSFontPanelUnderlineEffectModeMask
            | NSFontPanelStrikethroughEffectModeMask;
}

- (void)qt_sendPostedMessage:(NSEvent *)event
{
    // WARNING: data1 and data2 is truncated to from 64-bit to 32-bit on OS 10.5! 
    // That is why we need to split the address in two parts:
    quint64 lower = [event data1];
    quint64 upper = [event data2];
    QCocoaPostMessageArgs *args = reinterpret_cast<QCocoaPostMessageArgs *>(lower | (upper << 32));
    [args->target performSelector:args->selector];
    delete args;
}

- (BOOL)qt_sendEvent:(NSEvent *)event
{
    if ([event type] == NSApplicationDefined) {
        switch ([event subtype]) {
            case QtCocoaEventSubTypePostMessage:
                [NSApp qt_sendPostedMessage:event];
                return true;
            default:
                break;
        }
    }
    return false;
}

@end

@implementation QNSApplication

// WARNING: If Qt did not create NSApplication (this can e.g.
// happend if Qt is used as a plugin from a 3rd-party cocoa
// application), QNSApplication::sendEvent will never be called.
// SO DO NOT RELY ON THIS FUNCTION BEING AVAILABLE. 
// Plugin developers that _do_ control the NSApplication sub-class
// implementation of the 3rd-party application can call qt_sendEvent
// from the sub-class event handler (like we do here) to work around
// any issues.
- (void)sendEvent:(NSEvent *)event
{
    if (![self qt_sendEvent:event])
        [super sendEvent:event];
}

@end

#endif
