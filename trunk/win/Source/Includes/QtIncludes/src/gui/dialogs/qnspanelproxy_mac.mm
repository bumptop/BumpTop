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

#include <qdialogbuttonbox.h>
#if defined(Q_WS_MAC)
#include <private/qt_mac_p.h>
#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>
#import <objc/objc-class.h>

QT_BEGIN_NAMESPACE
static QWidget *currentWindow = 0;
QT_END_NAMESPACE

QT_USE_NAMESPACE

@class QT_MANGLE_NAMESPACE(QNSPanelProxy);

@interface QT_MANGLE_NAMESPACE(QNSPanelProxy) : NSWindow {
}
- (id)initWithContentRect:(NSRect)contentRect styleMask:(NSUInteger)windowStyle
    backing:(NSBackingStoreType)bufferingType defer:(BOOL)deferCreation;
- (id)initWithContentRect:(NSRect)contentRect styleMask:(NSUInteger)windowStyle
    backing:(NSBackingStoreType)bufferingType defer:(BOOL)deferCreation screen:(NSScreen *)screen;
- (id)qt_fakeInitWithContentRect:(NSRect)contentRect styleMask:(NSUInteger)windowStyle
    backing:(NSBackingStoreType)bufferingType defer:(BOOL)deferCreation;
- (id)qt_fakeInitWithContentRect:(NSRect)contentRect styleMask:(NSUInteger)windowStyle
    backing:(NSBackingStoreType)bufferingType defer:(BOOL)deferCreation screen:(NSScreen *)screen;
@end

@implementation QT_MANGLE_NAMESPACE(QNSPanelProxy)
- (id)initWithContentRect:(NSRect)contentRect styleMask:(NSUInteger)windowStyle
      backing:(NSBackingStoreType)bufferingType defer:(BOOL)deferCreation
{
    // remove evil flag
    windowStyle &= ~NSUtilityWindowMask;
	self = [self qt_fakeInitWithContentRect:contentRect styleMask:windowStyle
	                                backing:bufferingType defer:deferCreation];
    return self;
}

- (id)initWithContentRect:(NSRect)contentRect styleMask:(NSUInteger)windowStyle
      backing:(NSBackingStoreType)bufferingType defer:(BOOL)deferCreation screen:(NSScreen *)screen
{
    // remove evil flag
    windowStyle &= ~NSUtilityWindowMask;
	return [self qt_fakeInitWithContentRect:contentRect styleMask:windowStyle
	                                backing:bufferingType defer:deferCreation screen:screen];
}

- (id)qt_fakeInitWithContentRect:(NSRect)contentRect styleMask:(NSUInteger)windowStyle
    backing:(NSBackingStoreType)bufferingType defer:(BOOL)deferCreation
{
    Q_UNUSED(contentRect);
    Q_UNUSED(windowStyle);
    Q_UNUSED(bufferingType);
    Q_UNUSED(deferCreation);
    return nil;
}

- (id)qt_fakeInitWithContentRect:(NSRect)contentRect styleMask:(NSUInteger)windowStyle
    backing:(NSBackingStoreType)bufferingType defer:(BOOL)deferCreation screen:(NSScreen *)screen
{
    Q_UNUSED(contentRect);
    Q_UNUSED(windowStyle);
    Q_UNUSED(bufferingType);
    Q_UNUSED(deferCreation);
    Q_UNUSED(screen);
    return nil;
}
@end

@class QT_MANGLE_NAMESPACE(QNSWindowProxy);

@interface QT_MANGLE_NAMESPACE(QNSWindowProxy) : NSWindow {
}
- (void)setTitle:(NSString *)title;
- (void)qt_fakeSetTitle:(NSString *)title;
@end

@implementation QT_MANGLE_NAMESPACE(QNSWindowProxy)
- (void)setTitle:(NSString *)title
{
    QCFString cftitle(currentWindow->windowTitle());

    // evil reverse engineering
    if ([title isEqualToString:@"Print"]
            || [title isEqualToString:@"Page Setup"]
            || [[self className] isEqualToString:@"PMPrintingWindow"])
        title = (NSString *)(static_cast<CFStringRef>(cftitle));
    return [self qt_fakeSetTitle:title];
}

- (void)qt_fakeSetTitle:(NSString *)title
{
    Q_UNUSED(title);
}
@end

QT_BEGIN_NAMESPACE

void macStartIntercept(SEL originalSel, SEL fakeSel, Class baseClass, Class proxyClass)
{
#ifndef QT_MAC_USE_COCOA
    if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_5)
#endif
    {
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5
        // The following code replaces the _implementation_ for the selector we want to hack
        // (originalSel) with the implementation found in proxyClass. Then it creates
        // a new 'backup' method inside baseClass containing the old, original,
        // implementation (fakeSel). You can let the proxy implementation of originalSel
        // call fakeSel if needed (similar approach to calling a super class implementation).
        // fakeSel must also be implemented in proxyClass, as the signature is used
        // as template for the method one we add into baseClass.
        // NB: You will typically never create any instances of proxyClass; we use it
        // only for stealing its contents and put it into baseClass. 
        Method originalMethod = class_getInstanceMethod(baseClass, originalSel);
        Method newMethod = class_getInstanceMethod(proxyClass, originalSel);
        Method fakeMethod = class_getInstanceMethod(proxyClass, fakeSel);

        IMP originalImp = method_setImplementation(originalMethod, method_getImplementation(newMethod));
        class_addMethod(baseClass, fakeSel, originalImp, method_getTypeEncoding(fakeMethod));
#endif
    }
}

void macStopIntercept(SEL originalSel, SEL fakeSel, Class baseClass, Class /* proxyClass */)
{
#ifndef QT_MAC_USE_COCOA
    if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_5)
#endif
    {
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5
        Method originalMethod = class_getInstanceMethod(baseClass, originalSel);
        Method fakeMethodInBaseClass = class_getInstanceMethod(baseClass, fakeSel);
        method_setImplementation(originalMethod, method_getImplementation(fakeMethodInBaseClass));
#endif
    }
}

/*
    Intercept the NSColorPanel constructor if the shared
    color panel doesn't exist yet. What's going on here is
    quite wacky, because we want to override the NSPanel
    constructor and at the same time call the old NSPanel
    constructor. So what we do is we effectively rename the
    old NSPanel constructor qt_fakeInitWithContentRect:...
    and have the new one call the old one.
*/
void macStartInterceptNSPanelCtor()
{
    macStartIntercept(@selector(initWithContentRect:styleMask:backing:defer:),
                      @selector(qt_fakeInitWithContentRect:styleMask:backing:defer:),
                      [NSPanel class], [QT_MANGLE_NAMESPACE(QNSPanelProxy) class]);
    macStartIntercept(@selector(initWithContentRect:styleMask:backing:defer:screen:),
                      @selector(qt_fakeInitWithContentRect:styleMask:backing:defer:screen:),
                      [NSPanel class], [QT_MANGLE_NAMESPACE(QNSPanelProxy) class]);
}

/*
    Restore things as they were.
*/
void macStopInterceptNSPanelCtor()
{
    macStopIntercept(@selector(initWithContentRect:styleMask:backing:defer:screen:),
                     @selector(qt_fakeInitWithContentRect:styleMask:backing:defer:screen:),
                     [NSPanel class], [QT_MANGLE_NAMESPACE(QNSPanelProxy) class]);
    macStopIntercept(@selector(initWithContentRect:styleMask:backing:defer:),
                     @selector(qt_fakeInitWithContentRect:styleMask:backing:defer:),
                     [NSPanel class], [QT_MANGLE_NAMESPACE(QNSPanelProxy) class]);
}

/*
    Intercept the NSPrintPanel and NSPageLayout setTitle: calls. The
    hack is similar as for NSColorPanel above.
*/
void macStartInterceptWindowTitle(QWidget *window)
{
    currentWindow = window;
    macStartIntercept(@selector(setTitle:), @selector(qt_fakeSetTitle:),
                      [NSWindow class], [QT_MANGLE_NAMESPACE(QNSWindowProxy) class]);
}

/*
    Restore things as they were.
*/
void macStopInterceptWindowTitle()
{
    currentWindow = 0;
    macStopIntercept(@selector(setTitle:), @selector(qt_fakeSetTitle:),
                     [NSWindow class], [QT_MANGLE_NAMESPACE(QNSWindowProxy) class]);
}

/*
    Doesn't really belong in here.
*/
NSButton *macCreateButton(const char *text, NSView *superview)
{
    static const NSRect buttonFrameRect = { { 0.0, 0.0 }, { 0.0, 0.0 } };

    NSButton *button = [[NSButton alloc] initWithFrame:buttonFrameRect];
    [button setButtonType:NSMomentaryLightButton];
    [button setBezelStyle:NSRoundedBezelStyle];
    [button setTitle:(NSString*)(CFStringRef)QCFString(QDialogButtonBox::tr(text)
                                                       .remove(QLatin1Char('&')))];
    [[button cell] setFont:[NSFont systemFontOfSize:
            [NSFont systemFontSizeForControlSize:NSRegularControlSize]]];
    [superview addSubview:button];
    return button;
}

QT_END_NAMESPACE

#endif
