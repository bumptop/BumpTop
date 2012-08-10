//
//  Copyright 2012 Google Inc. All Rights Reserved.
//  
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  
//      http://www.apache.org/licenses/LICENSE-2.0
//  
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//

#import "BumpTop/OSX/OgreController.h"

#include "BumpTop/VisualPhysicsActorList.h"

#ifdef BUMPTOP_TEST
#include <gtest/gtest.h>
#endif

#import <CMCrashReporter/CMCrashReporter.h>
#ifndef BUMPTOP_TEST
#import <Sparkle/Sparkle.h>
#endif

#import "BumpTop/OSX/AboutWindowController.h"
#import "BumpTop/OSX/BumpTopOgreView.h"
#import "BumpTop/OSX/ModifyLoginItems.h"

#include "BumpTop/AppSettings.h"
#include "BumpTop/Authorization.h"
#include "BumpTop/BumpTopInstanceLock.h"
#include "BumpTop/BumpTopScene.h"
#include "BumpTop/FileManager.h"
#include "BumpTop/OSX/EventModifierFlags.h"
#include "BumpTop/KeyboardEventManager.h"
#include "BumpTop/MouseEventManager.h"
#include "BumpTop/OSX/NSTaskManager.h"
#include "BumpTop/OSX/OSXCocoaBumpTopApplication.h"
#include "BumpTop/OSX/OSXCocoaDragAndDrop.h"
#include "BumpTop/PersistenceManager.h"
#include "BumpTop/QStringHelpers.h"
#include "BumpTop/ToolTipManager.h"
#import "BumpTop/OSX/FullScreenWindow.h"

#import "ThirdParty/PFMoveApplication/PFMoveApplication.h"

const int kSplashScreenWidth = 500;
const int kSplashScreenHeight = 300;
const int kDisplayDelay = 2;
const int kFadeInLength = 10;
const bool kShowSplashScreen = false;

@interface NSStatusItem (global_frame)
- (NSRect)global_frame;
@end

@implementation NSStatusItem (global_frame)
- (NSRect)global_frame {
  return [_fWindow frame];
}
@end

@implementation OgreController

- (void)applicationDidFinishLaunching:(NSNotification *)notification {

#ifndef BUMPTOP_TEST
  bool settings_file_exists = AppSettings::singleton()->loadSettingsFile();
  if (!settings_file_exists) {
    [[SUUpdater sharedUpdater] setSendsSystemProfile:YES];
    [[SUUpdater sharedUpdater] setAutomaticallyDownloadsUpdates:YES];
    AppSettings::singleton()->saveSettingsFile();
  }
#endif

  render_tick_count_ = 0;

#ifndef BUMPTOP_TEST
  if (!BumpTopInstanceLock::is_running_in_sandbox()) {
    LoginItems::addOrRepairLoginItemIfRequired();
  }
#endif  // BUMPTOP_TEST

#ifdef BUMPTOP_TEST
  [fullscreen_window_controller showWindow:self];
#else
  [self setupWindowsForLaunch];
  [self setupTaskBarItem];
#endif

  bumptop_app_ = new OSXCocoaBumpTopApplication(ogre_view_, self);
  bumptop_app_->init();
  bumptop_app_->set_window(fullScreenWindow);

  [self addToOgreViewResponderChain];
  [fullScreenWindow makeFirstResponder:self];
  [fullScreenWindow setAcceptsMouseMovedEvents:YES];
  [self initRenderTimer];

#ifdef BUMPTOP_TEST
  exit(RUN_ALL_TESTS());
#endif

  BumpTopScene *scene = new BumpTopScene(bumptop_app_);
  // IMPORTANT we set the scene before init'ing, because in the init sequence some actors
  //    access the scene through BumpTopApp
  bumptop_app_->set_scene(scene);
  scene->init();
}

- (void)setupWindowsForLaunch {
  // If there is a splash screen, we want to center it;
  NSRect screen_rect = [[[NSScreen screens] objectAtIndex:0] frame];
  screen_rect.origin.x = screen_rect.origin.x + ((screen_rect.size.width - kSplashScreenWidth)/2);
  screen_rect.origin.y = screen_rect.origin.y + ((screen_rect.size.height - kSplashScreenHeight)/2);
  screen_rect.size.width = kSplashScreenWidth;
  screen_rect.size.height = kSplashScreenHeight;

  [fullScreenWindow setAlphaValue:0];

  [fullScreenWindow setFrame:screen_rect display:YES];
  [fullScreenWindow setContentSize:screen_rect.size];

  if (kShowSplashScreen) {
    [splashScreenWindow setFrame:screen_rect display:YES];
    [splashScreenWindow setContentSize:screen_rect.size];
  }

  [fullscreen_window_controller showWindow:self];

  if (kShowSplashScreen) {
    [splashscreen_window_controller showWindow:self];
  }
}

- (IBAction)showAboutWindow:(id)sender {
  [about_window_controller_ setBumpTopVersion:NSStringFromQString(BumpTopApp::singleton()->bumptopVersion())];
  [about_window_controller_ showWindow:self];
}

- (void)mouseEntered:(NSEvent*)mouse_entered_event {
  bumptop_app_->mouse_event_manager()->mouseEntered();
}

- (void)mouseExited:(NSEvent*)mouse_exited_event {
  bumptop_app_->mouse_event_manager()->mouseExited();
}

- (void)mouseDown:(NSEvent*)mouse_down_event {
  OSXCocoaDragAndDrop* osx_cocoa_drag_and_drop = static_cast<OSXCocoaDragAndDrop*>(bumptop_app_->drag_and_drop());  // NOLINT
  osx_cocoa_drag_and_drop->set_last_mouse_down_event(mouse_down_event);

  Ogre::Vector2 point = [self getOgreMouseCoordinatesFromNSEvent: mouse_down_event];
  if ([mouse_down_event modifierFlags] & CONTROL_KEY_MASK) {
    bumptop_app_->rightMouseDown(point.x, point.y,
                                         [mouse_down_event clickCount], [mouse_down_event modifierFlags]);
  } else {
    bumptop_app_->mouseDown(point.x, point.y,
                                    [mouse_down_event clickCount], [mouse_down_event modifierFlags]);
  }
}

- (void)mouseUp:(NSEvent*)mouse_up_event {
  Ogre::Vector2 point = [self getOgreMouseCoordinatesFromNSEvent: mouse_up_event];
  bumptop_app_->mouseUp(point.x, point.y, [mouse_up_event clickCount], [mouse_up_event modifierFlags]);
}

- (void)mouseMoved:(NSEvent*)mouse_moved_event {
  Ogre::Vector2 point = [self getOgreMouseCoordinatesFromNSEvent: mouse_moved_event];
  bumptop_app_->mouseMoved(point.x, point.y, [mouse_moved_event clickCount], [mouse_moved_event modifierFlags]);
}

- (void)mouseDragged:(NSEvent*)mouse_dragged_event {
  Ogre::Vector2 point = [self getOgreMouseCoordinatesFromNSEvent: mouse_dragged_event];
  bumptop_app_->mouseDragged(point.x, point.y, [mouse_dragged_event clickCount], [mouse_dragged_event modifierFlags]);
}

- (void)rightMouseDown:(NSEvent*)mouse_down_event {
  Ogre::Vector2 point = [self getOgreMouseCoordinatesFromNSEvent: mouse_down_event];
  bumptop_app_->rightMouseDown(point.x, point.y, [mouse_down_event clickCount], [mouse_down_event modifierFlags]);
}

- (void)rightMouseUp:(NSEvent*)mouse_up_event {
  Ogre::Vector2 point = [self getOgreMouseCoordinatesFromNSEvent: mouse_up_event];
  bumptop_app_->rightMouseUp(point.x, point.y, [mouse_up_event clickCount], [mouse_up_event modifierFlags]);
}

- (void)rightMouseDragged:(NSEvent*)mouse_dragged_event {
  Ogre::Vector2 point = [self getOgreMouseCoordinatesFromNSEvent: mouse_dragged_event];
  bumptop_app_->rightMouseDragged(point.x, point.y,
                                          [mouse_dragged_event clickCount], [mouse_dragged_event modifierFlags]);
}

- (void)scrollWheel:(NSEvent *)scroll_wheel_event {
  Ogre::Vector2 point = [self getOgreMouseCoordinatesFromNSEvent:scroll_wheel_event];
  bumptop_app_->scrollWheel(point.x, point.y, 0,
                            NO_KEY_MODIFIERS_MASK, scroll_wheel_event.deltaY);
}

- (void)flagsChanged:(NSEvent*) flags_changed_event {
  int flags = [flags_changed_event modifierFlags];

  KeyboardEvent keyboard_event = KeyboardEvent("", flags);
  bumptop_app_->keyboard_event_manager()->flagsChanged(keyboard_event);
}

- (void)keyDown:(NSEvent *)key_down_event {
  int flags = [key_down_event modifierFlags];
  NSString *characters = [key_down_event charactersIgnoringModifiers];

  KeyboardEvent keyboard_event = KeyboardEvent(QStringFromNSString(characters), flags);
  bumptop_app_->keyboard_event_manager()->keyDown(keyboard_event);
}

- (BOOL)prepareForDragOperation:(id <NSDraggingInfo>)sender {
  bumptop_app_->mouseUp(0.0, 0.0, 1, 0);
  return YES;
}

- (void)draggedImage:(NSImage *)image endedAt:(NSPoint)screen_point operation:(NSDragOperation)operation {
  Ogre::Vector2 cocoa_screen_point = Ogre::Vector2(screen_point.x, screen_point.y);
  Ogre::Vector2 ogre_screen_point = bumptop_app_->cocoaCoordinatesToOgreCoordinates(cocoa_screen_point);

  // We pass a draggingEntered event if nothing's going to happen, just to prevent the item from snapping back
  if (operation == NSDragOperationNone) {
    BumpTopApp::singleton()->mouse_event_manager()->draggingEntered(ogre_screen_point.x,
                                                                            ogre_screen_point.y, operation);
  }

  int num_clicks = 1;
  int modifier_flags = 0;
  // TODO: route this more directly to the item being dragged
  bumptop_app_->mouseUp(ogre_screen_point.x, ogre_screen_point.y, num_clicks, modifier_flags);
  if (operation & NSDragOperationDelete) {
    bumptop_app_->drag_and_drop()->dragFinishedInTrash();
  }
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender {
  bumptop_app_->applicationWillTerminate();
  return NSTerminateNow;
}

- (void)updateWindowSizeAndPlacement {
  NSRect screen_rect = [[[NSScreen screens] objectAtIndex:0] frame];
  screen_rect.size.height = screen_rect.size.height - MENU_BAR_HEIGHT;
  // When you disconnect an external display, you get a 1x1 sized screen for
  // a second or two, in this case we want to ignore
  if (!(screen_rect.size.width <= 50 && screen_rect.size.height <= 50)) {
    [fullScreenWindow setFrame:screen_rect display:YES];
    [fullScreenWindow setContentSize:screen_rect.size];
    if (bumptop_app_ != NULL) {
      bumptop_app_->camera()->setAspectRatio(Ogre::Real(bumptop_app_->viewport()->getActualWidth()) /
                                                   Ogre::Real(bumptop_app_->viewport()->getActualHeight()));

      bumptop_app_->windowRectChanged();
    }
  }
}

- (void) checkAndCorrectWindowSizeAndPlacementIfChanged {
  if (bumptop_app_->should_force_bumptop_window_to_front())
    [fullScreenWindow orderFrontRegardless];

  NSRect screen_rect = [[[NSScreen screens] objectAtIndex:0] frame];
  NSRect window_rect = [fullScreenWindow frame];
  if (!NSEqualRects(screen_rect, last_screen_rect_) || !NSEqualRects(window_rect, last_window_rect_)) {
    [self updateWindowSizeAndPlacement];
    last_screen_rect_ = screen_rect;
    last_window_rect_ = window_rect;
  }
}

- (void)makeOgreViewDesktop {
  [self updateWindowSizeAndPlacement];

  [fullScreenWindow setLevel:kCGDesktopIconWindowLevel];
  [fullScreenWindow orderFrontRegardless];
}

- (void) setupTaskBarItem {
  [status_bar_menu_ setDelegate:(id<NSMenuDelegate>)self];
  bumptop_status_item_ = [[NSStatusBar systemStatusBar] statusItemWithLength:31];
  [bumptop_status_item_ retain];
  [bumptop_status_item_ setImage:[NSImage imageNamed:@"statusBarImage.png"]];
  [bumptop_status_item_ setEnabled:YES];
  [bumptop_status_item_ setHighlightMode:YES];
  [bumptop_status_item_ setMenu:status_bar_menu_];
}

- (void) menuWillOpen:(NSMenu*)menu {
  ToolTipManager::singleton()->hideTaskbarTooltip();
}

- (Ogre::Vector2) taskbarItemLocation {
  NSRect frame = [bumptop_status_item_ global_frame];
  NSPoint pt = NSMakePoint(NSMidX(frame), NSMinY(frame));
  return Ogre::Vector2(pt.x, pt.y);
}

- (void)addToOgreViewResponderChain {
  [self setNextResponder: [ogre_view_ nextResponder]];
  [ogre_view_ setNextResponder: self];
}

- (Ogre::Vector2)getOgreMouseCoordinatesFromNSEvent: (NSEvent*)event {
  NSPoint point = [event locationInWindow];
  return bumptop_app_->cocoaCoordinatesToOgreCoordinates(Ogre::Vector2(point.x, point.y));
}

- (void)slowDownRenderTimer {
  if (!running_in_idle_mode_) {
    running_in_idle_mode_ = true;
    [render_timer_ invalidate];
    render_timer_ = [NSTimer scheduledTimerWithTimeInterval:0.05
                                                     target:self
                                                   selector:@selector(renderTick)
                                                   userInfo:NULL
                                                    repeats:YES];
    [[NSRunLoop currentRunLoop] addTimer:render_timer_ forMode:NSEventTrackingRunLoopMode];
  }
}

- (void)speedUpRenderTimer {
  if (running_in_idle_mode_) {
    [render_timer_ invalidate];
    [self initRenderTimer];
  }
}

- (void)initRenderTimer {
  running_in_idle_mode_ = false;
  render_timer_ = [NSTimer scheduledTimerWithTimeInterval:0.005
                                                   target:self
                                                 selector:@selector(renderTick)
                                                 userInfo:NULL
                                                  repeats:YES];
  [[NSRunLoop currentRunLoop] addTimer:render_timer_ forMode:NSEventTrackingRunLoopMode];
}

- (void)renderTick {
  bumptop_app_->renderTick();
#ifndef BUMPTOP_TEST
  render_tick_count_++;
  [self manageSplashScreenAndFadeIn];
  if (render_tick_count_ > kDisplayDelay)
    [self checkAndCorrectWindowSizeAndPlacementIfChanged];
  if (!running_in_idle_mode_ && bumptop_app_->isInIdleMode()) {
    [self slowDownRenderTimer];
  } else if (running_in_idle_mode_ && !bumptop_app_->isInIdleMode()) {
    [self speedUpRenderTimer];
  }
#endif
}

-(void) manageSplashScreenAndFadeIn {
  if (render_tick_count_ == kDisplayDelay) {
    [self makeOgreViewDesktop];
    if (kShowSplashScreen)
      [splashscreen_window_controller close];
  }
  if (render_tick_count_ > kDisplayDelay && render_tick_count_ <= kFadeInLength + kDisplayDelay) {
    float window_alpha = (render_tick_count_-kDisplayDelay)/(kFadeInLength*1.0);
    [fullScreenWindow setAlphaValue:window_alpha];
    if (window_alpha == 1) {
      ToolTipManager::singleton()->showTaskbarTooltip();
    }
  }
}

// Getters
- (BumpTopOgreView*) ogre_view {
  return ogre_view_;
}

- (void)applicationWillFinishLaunching:(NSNotification *)aNotification
{
  ProAuthorization::singleton()->init();
  ProAuthorization::singleton()->add_sticky_note_sizes(200);  // first note
  ProAuthorization::singleton()->add_sticky_note_sizes(200);  // second note
  ProAuthorization::singleton()->add_sticky_note_sizes(0);  // third note

  // TODO:  make sure to do this after botched attempt to load file
  //if (ProAuthorization::singleton()->loadLicenseFile()) {
    // second time run, and all runs after that
    BoolAndQString success_and_error_message = ProAuthorization::singleton()->decipherAndLoadConfigFile();
    // don't do anything if there's an error
  //}

#ifndef BUMPTOP_TEST
  // commented out code to generate crashes (to test the code below)
  // int *i = 0;
  // int j = *i;
  NSTask *task = [[NSTask alloc] init];
  QString working_directory = QStringFromNSString([[NSBundle mainBundle] bundlePath]);
  [task setCurrentDirectoryPath:NSStringFromQString(FileManager::getParentPath(working_directory))];
  [task setLaunchPath:@"BumpTop.app/Contents/SharedSupport/CMCrashReporterModule.app/Contents/MacOS/CMCrashReporterModule"];
  /*[[NSNotificationCenter defaultCenter] addObserver:[NSTaskManager singleton]
                                           selector:@selector(taskExited:)
                                               name:NSTaskDidTerminateNotification
                                             object:task];*/
  //[task launch];

  PFMoveToApplicationsFolderIfNecessary();
#endif
}

@end
