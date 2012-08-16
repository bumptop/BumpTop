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
 NB: This is not a header file, dispite the file name suffix. This file is
 included directly into the source code of qcocoawindow_mac.mm and
 qcocoapanel_mac.mm to avoid manually doing copy and paste of the exact
 same code needed at both places. This solution makes it more difficult
 to e.g fix a bug in qcocoawindow_mac.mm, but forget to do the same in
 qcocoapanel_mac.mm.
 The reason we need to do copy and paste in the first place, rather than
 resolve to method overriding, is that QCocoaPanel needs to inherit from
 NSPanel, while QCocoaWindow needs to inherit NSWindow rather than NSPanel).
****************************************************************************/

// WARNING: Don't include any header files from within this file. Put them
// directly into qcocoawindow_mac_p.h and qcocoapanel_mac_p.h

QT_BEGIN_NAMESPACE
extern Qt::MouseButton cocoaButton2QtButton(NSInteger buttonNum); // qcocoaview.mm
extern QPointer<QWidget> qt_button_down; //qapplication_mac.cpp
extern const QStringList& qEnabledDraggedTypes(); // qmime_mac.cpp
extern void qt_event_request_window_change(QWidget *); // qapplication_mac.mm
extern void qt_mac_send_posted_gl_updates(QWidget *widget); // qapplication_mac.mm

Q_GLOBAL_STATIC(QPointer<QWidget>, currentDragTarget);

QT_END_NAMESPACE

- (id)initWithContentRect:(NSRect)contentRect
    styleMask:(NSUInteger)windowStyle
    backing:(NSBackingStoreType)bufferingType
    defer:(BOOL)deferCreation
{
    self = [super initWithContentRect:contentRect styleMask:windowStyle
        backing:bufferingType defer:deferCreation];
    if (self) {
        currentCustomDragTypes = 0;
    }
    return self;
}

- (void)dealloc
{
    delete currentCustomDragTypes;
    [super dealloc];
}

- (BOOL)canBecomeKeyWindow
{
    QWidget *widget = [self QT_MANGLE_NAMESPACE(qt_qwidget)];
    if (!widget)
        return NO; // This should happen only for qt_root_win
    if (QApplicationPrivate::isBlockedByModal(widget))
        return NO;

    bool isToolTip = (widget->windowType() == Qt::ToolTip);
    bool isPopup = (widget->windowType() == Qt::Popup);
    return !(isPopup || isToolTip);
}

- (BOOL)canBecomeMainWindow
{
    QWidget *widget = [self QT_MANGLE_NAMESPACE(qt_qwidget)];
    if (!widget)
        return NO; // This should happen only for qt_root_win
    if ([self isSheet])
        return NO;

    bool isToolTip = (widget->windowType() == Qt::ToolTip);
    bool isPopup = (widget->windowType() == Qt::Popup);
    bool isTool = (widget->windowType() == Qt::Tool);
    return !(isPopup || isToolTip || isTool);
}

- (void)becomeMainWindow
{
    [super becomeMainWindow];
    // Cocoa sometimes tell a hidden window to become the
    // main window (and as such, show it). This can e.g
    // happend when the application gets activated. If
    // this is the case, we tell it to hide again:
    if (![self isVisible])
        [self orderOut:self];
}

- (void)toggleToolbarShown:(id)sender
{
    macSendToolbarChangeEvent([self QT_MANGLE_NAMESPACE(qt_qwidget)]);
    [super toggleToolbarShown:sender];
}

- (void)flagsChanged:(NSEvent *)theEvent
{
    qt_dispatchModifiersChanged(theEvent, [self QT_MANGLE_NAMESPACE(qt_qwidget)]);
    [super flagsChanged:theEvent];
}


- (void)tabletProximity:(NSEvent *)tabletEvent
{
    qt_dispatchTabletProximityEvent(tabletEvent);
}

- (void)qtDispatcherToQAction:(id)sender
{
    // If this window is modal, the menu bar will be modally shaddowed.
    // In that case, since the window will be in the first responder chain,
    // we can still catch the trigger here and forward it to the menu bar.
    // This is needed as a single modal dialog on Qt should be able to access
    // the application menu (e.g. quit).
    [[NSApp QT_MANGLE_NAMESPACE(qt_qcocoamenuLoader)] qtDispatcherToQAction:sender];
}

- (void)terminate:(id)sender
{
    // This function is called from the quit item in the menubar when this window
    // is in the first responder chain (see also qtDispatcherToQAction above)
    [NSApp terminate:sender];
}

- (void)setLevel:(NSInteger)windowLevel
{
    // Cocoa will upon activating/deactivating applications level modal
    // windows up and down, regardsless of any explicit set window level.
    // To ensure that modal stays-on-top dialogs actually stays on top after
    // the application is activated (and therefore stacks in front of
    // other stays-on-top windows), we need to add this little special-case override:
    QWidget *widget = [[QT_MANGLE_NAMESPACE(QCocoaWindowDelegate) sharedDelegate] qt_qwidgetForWindow:self];
    if (widget && widget->isModal() && (widget->windowFlags() & Qt::WindowStaysOnTopHint))
        [super setLevel:NSPopUpMenuWindowLevel];
    else
        [super setLevel:windowLevel];
}

- (void)sendEvent:(NSEvent *)event
{
    if ([event type] == NSApplicationDefined) {
        switch ([event subtype]) {
            case QtCocoaEventSubTypePostMessage:
                [NSApp qt_sendPostedMessage:event];
                return;
            default:
                break;
        }
        return;
    }

    QWidget *widget = [[QT_MANGLE_NAMESPACE(QCocoaWindowDelegate) sharedDelegate] qt_qwidgetForWindow:self];
    // Cocoa can hold onto the window after we've disavowed its knowledge. So,
    // if we get sent an event afterwards just have it go through the super's
    // version and don't do any stuff with Qt.
    if (!widget) {
        [super sendEvent:event];
        return;
    }

    [self retain];
    QT_MANGLE_NAMESPACE(QCocoaView) *view = static_cast<QT_MANGLE_NAMESPACE(QCocoaView) *>(qt_mac_nativeview_for(widget));
    Qt::MouseButton mouseButton = cocoaButton2QtButton([event buttonNumber]);

    bool handled = false;
    // sometimes need to redirect mouse events to the popup.
    QWidget *popup = qAppInstance()->activePopupWidget();
    if (popup && popup != widget) {
        switch([event type])
        {
        case NSLeftMouseDown:
            if (!qt_button_down)
                qt_button_down = widget;
            handled = qt_mac_handleMouseEvent(view, event, QEvent::MouseButtonPress, mouseButton);
            // Don't call super here. This prevents us from getting the mouseUp event,
            // which we need to send even if the mouseDown event was not accepted.
            // (this is standard Qt behavior.)
            break;
        case NSRightMouseDown:
        case NSOtherMouseDown:
            if (!qt_button_down)
                qt_button_down = widget;
            handled = qt_mac_handleMouseEvent(view, event, QEvent::MouseButtonPress, mouseButton);
            break;
        case NSLeftMouseUp:
        case NSRightMouseUp:
        case NSOtherMouseUp:
            handled = qt_mac_handleMouseEvent(view, event, QEvent::MouseButtonRelease, mouseButton);
            qt_button_down = 0;
            break;
        case NSMouseMoved:
            handled = qt_mac_handleMouseEvent(view, event, QEvent::MouseMove, Qt::NoButton);
            break;
        case NSLeftMouseDragged:
        case NSRightMouseDragged:
        case NSOtherMouseDragged:
            [QT_MANGLE_NAMESPACE(QCocoaView) currentMouseEvent]->view = view;
            [QT_MANGLE_NAMESPACE(QCocoaView) currentMouseEvent]->theEvent = event;
            handled = qt_mac_handleMouseEvent(view, event, QEvent::MouseMove, mouseButton);
            break;
        default:
            [super sendEvent:event];
            break;
        }
    } else {
        [super sendEvent:event];
    }

    if (!handled)
        qt_mac_dispatchNCMouseMessage(self, event, [self QT_MANGLE_NAMESPACE(qt_qwidget)], leftButtonIsRightButton);

    [self release];
}

- (void)setInitialFirstResponder:(NSView *)view
{
    // This method is called the first time the window is placed on screen and
    // is the earliest point in time we can connect OpenGL contexts to NSViews.
    QWidget *qwidget = [[QT_MANGLE_NAMESPACE(QCocoaWindowDelegate) sharedDelegate] qt_qwidgetForWindow:self];
    if (qwidget) {
        qt_event_request_window_change(qwidget);
        qt_mac_send_posted_gl_updates(qwidget);
    }

    [super setInitialFirstResponder:view];
}

- (BOOL)makeFirstResponder:(NSResponder *)responder
{
    // For some reason Cocoa wants to flip the first responder
    // when Qt doesn't want to, sorry, but "No" :-)
    if (responder == nil && qApp->focusWidget())
        return NO;
    return [super makeFirstResponder:responder];
}

+ (Class)frameViewClassForStyleMask:(NSUInteger)styleMask
{
    if (styleMask & QtMacCustomizeWindow)
        return [QT_MANGLE_NAMESPACE(QCocoaWindowCustomThemeFrame) class];
    return [super frameViewClassForStyleMask:styleMask];
}

-(void)registerDragTypes
{
    // Calling registerForDraggedTypes below is slow, so only do
    // it once for each window, or when the custom types change.
    QMacCocoaAutoReleasePool pool;
    const QStringList& customTypes = qEnabledDraggedTypes();
    if (currentCustomDragTypes == 0 || *currentCustomDragTypes != customTypes) {
        if (currentCustomDragTypes == 0)
            currentCustomDragTypes = new QStringList();
        *currentCustomDragTypes = customTypes;
        const NSString* mimeTypeGeneric = @"com.trolltech.qt.MimeTypeName";
        NSMutableArray *supportedTypes = [NSMutableArray arrayWithObjects:NSColorPboardType,
                       NSFilenamesPboardType, NSStringPboardType,
                       NSFilenamesPboardType, NSPostScriptPboardType, NSTIFFPboardType,
                       NSRTFPboardType, NSTabularTextPboardType, NSFontPboardType,
                       NSRulerPboardType, NSFileContentsPboardType, NSColorPboardType,
                       NSRTFDPboardType, NSHTMLPboardType, NSPICTPboardType,
                       NSURLPboardType, NSPDFPboardType, NSVCardPboardType,
                       NSFilesPromisePboardType, NSInkTextPboardType,
                       NSMultipleTextSelectionPboardType, mimeTypeGeneric, nil];
        // Add custom types supported by the application.
        for (int i = 0; i < customTypes.size(); i++) {
           [supportedTypes addObject:reinterpret_cast<const NSString *>(QCFString::toCFStringRef(customTypes[i]))];
        }
        [self registerForDraggedTypes:supportedTypes];
    }
}

- (QWidget *)dragTargetHitTest:(id <NSDraggingInfo>)sender
{
    // Do a hittest to find the NSView under the
    // mouse, and return the corresponding QWidget:
    NSPoint windowPoint = [sender draggingLocation];
    NSView *candidateView = [[self contentView] hitTest:windowPoint];
    if (![candidateView isKindOfClass:[QT_MANGLE_NAMESPACE(QCocoaView) class]])
        return 0;
    return [static_cast<QT_MANGLE_NAMESPACE(QCocoaView) *>(candidateView) qt_qwidget];
}

- (NSDragOperation)draggingEntered:(id <NSDraggingInfo>)sender
{
    // The user dragged something into the window. Send a draggingEntered message
    // to the QWidget under the mouse. As the drag moves over the window, and over
    // different widgets, we will handle enter and leave events from within
    // draggingUpdated below. The reason why we handle this ourselves rather than
    // subscribing for drag events directly in QCocoaView is that calling
    // registerForDraggedTypes on the views will severly degrade initialization time
    // for an application that uses a lot of drag subscribing widgets.

    QWidget *target = [self dragTargetHitTest:sender];
    if (!target)
        return NSDragOperationNone;
    if (target->testAttribute(Qt::WA_DropSiteRegistered) == false)
        return NSDragOperationNone;

    *currentDragTarget() = target;
    return [reinterpret_cast<NSView *>((*currentDragTarget())->winId()) draggingEntered:sender];
 }

- (NSDragOperation)draggingUpdated:(id < NSDraggingInfo >)sender
{
    QWidget *target = [self dragTargetHitTest:sender];
    if (!target)
        return NSDragOperationNone;

    if (target == *currentDragTarget()) {
        // The drag continues to move over the widget that we have sendt
        // a draggingEntered message to. So just update the view:
        return [reinterpret_cast<NSView *>((*currentDragTarget())->winId()) draggingUpdated:sender];
    } else {
        // The widget under the mouse has changed.
        // So we need to fake enter/leave events:
        if (*currentDragTarget())
            [reinterpret_cast<NSView *>((*currentDragTarget())->winId()) draggingExited:sender];
        if (target->testAttribute(Qt::WA_DropSiteRegistered) == false) {
            *currentDragTarget() = 0;
            return NSDragOperationNone;
        }
        *currentDragTarget() = target;
        return [reinterpret_cast<NSView *>((*currentDragTarget())->winId()) draggingEntered:sender];
    }
}

- (void)draggingExited:(id < NSDraggingInfo >)sender
{
    QWidget *target = [self dragTargetHitTest:sender];
    if (!target)
        return;

    if (*currentDragTarget()) {
        [reinterpret_cast<NSView *>((*currentDragTarget())->winId()) draggingExited:sender];
        *currentDragTarget() = 0;
    }
}

- (BOOL)performDragOperation:(id < NSDraggingInfo >)sender
{
    QWidget *target = [self dragTargetHitTest:sender];
    if (!target)
        return NO;

    BOOL dropResult = NO;
    if (*currentDragTarget()) {
        dropResult = [reinterpret_cast<NSView *>((*currentDragTarget())->winId()) performDragOperation:sender];
        *currentDragTarget() = 0;
    }
    return dropResult;
}

- (void)displayIfNeeded
{

    QWidget *qwidget = [[QT_MANGLE_NAMESPACE(QCocoaWindowDelegate) sharedDelegate] qt_qwidgetForWindow:self];
    if (qwidget == 0) {
        [super displayIfNeeded];
        return;
    }

    if (QApplicationPrivate::graphicsSystem() != 0) {
        if (QWidgetBackingStore *bs = qt_widget_private(qwidget)->maybeBackingStore())
            bs->sync(qwidget, qwidget->rect());
    }
    [super displayIfNeeded];
}

// This is a hack and it should be removed once we find the real cause for
// the painting problems.
// We have a static variable that signals if we have been called before or not.
static bool firstDrawingInvocation = true;

// The method below exists only as a workaround to draw/not draw the baseline
// in the title bar. This is to support unifiedToolbar look.

// This method is very special. To begin with, it is a
// method that will get called only if we enable documentMode.
// Furthermore, it won't get called as a normal method, we swap
// this method with the normal implementation of drawRect in
// _NSThemeFrame. When this method is active, its mission is to
// first call the original drawRect implementation so the widget
// gets proper painting. After that, it needs to detect if there
// is a toolbar or not, in order to decide how to handle the unified
// look. The distinction is important since the presence and
// visibility of a toolbar change the way we enter into unified mode.
// When there is a toolbar and that toolbar is visible, the problem
// is as simple as to tell the toolbar not to draw its baseline.
// However when there is not toolbar or the toolbar is not visible,
// we need to draw a line on top of the baseline, because the baseline
// in that case will belong to the title. For this case we need to draw
// a line on top of the baseline.
// As usual, there is a special case. When we first are called, we might
// need to repaint ourselves one more time. We only need that if we
// didn't get the activation, i.e. when we are launched via the command
// line. And this only if the toolbar is visible from the beginning,
// so we have a special flag that signals if we need to repaint or not.
- (void)drawRectSpecial:(NSRect)rect
{
    // Call the original drawing method.
    [self drawRectOriginal:rect];
    NSWindow *window = [self window];
    NSToolbar *toolbar = [window toolbar];
    if(!toolbar) {
        // There is no toolbar, we have to draw a line on top of the line drawn by Cocoa.
        macDrawRectOnTop((void *)window);
    } else {
        if([toolbar isVisible]) {
            // We tell Cocoa to avoid drawing the line at the end.
            if(firstDrawingInvocation) {
                firstDrawingInvocation = false;
                macSyncDrawingOnFirstInvocation((void *)window);
            } else
                [toolbar setShowsBaselineSeparator:NO];
        } else {
            // There is a toolbar but it is not visible so
            // we have to draw a line on top of the line drawn by Cocoa.
            macDrawRectOnTop((void *)window);
        }
    }
}
