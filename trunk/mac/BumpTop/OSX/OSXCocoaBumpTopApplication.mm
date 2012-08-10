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

#include "BumpTop/OSX/OSXCocoaBumpTopApplication.h"

#import <AppKit/NSWorkspace.h>
#import "BumpTop/OSX/FullScreenWindow.h"
#import "BumpTop/OSX/OgreController.h"
#include "BumpTop/OSX/OSXCocoaDragAndDrop.h"
#include "BumpTop/QStringHelpers.h"

OSXCocoaBumpTopApplication::OSXCocoaBumpTopApplication(OgreView* ogre_view, OgreController* ogre_controller)
: BumpTopApp(),
  ogre_view_(ogre_view),
  ogre_controller_(ogre_controller) {
  drag_and_drop_ = new OSXCocoaDragAndDrop(ogre_view);
}

OSXCocoaBumpTopApplication::~OSXCocoaBumpTopApplication() {
}

OSXCocoaBumpTopApplication* OSXCocoaBumpTopApplication::singleton() {
  return static_cast<OSXCocoaBumpTopApplication*>(BumpTopApp::singleton());
}

void OSXCocoaBumpTopApplication::init() {
  BumpTopApp::init();
  ogre_gl_context_ = [NSOpenGLContext currentContext];
}

void OSXCocoaBumpTopApplication::pushGLContextAndSwitchToOgreGLContext() {
  pushed_gl_context_ = [NSOpenGLContext currentContext];
  if (pushed_gl_context_ != ogre_gl_context_) {
    [ogre_gl_context_ makeCurrentContext];
  }
}

void OSXCocoaBumpTopApplication::popGLContext() {
  if (pushed_gl_context_ != ogre_gl_context_) {
    [pushed_gl_context_ makeCurrentContext];
  }
}

QString OSXCocoaBumpTopApplication::platform() {
  return "OSX";
}

OgreView* OSXCocoaBumpTopApplication::ogre_view() {
  return ogre_view_;
}

// Taken from:
// http://cocoawithlove.com/2009/01/demystifying-nsapplication-by.html
// if milliseconds is -1, then block indefinitely
void OSXCocoaBumpTopApplication::processOneEvent(int milliseconds) {
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
  NSApplication *application = [NSApplication sharedApplication];

  NSDate* until_date;

  if (milliseconds < 0) {
    until_date = [NSDate distantFuture];
  } else if (milliseconds == 0) {
    until_date = nil;
  } else {
    until_date = [NSDate date];
    [until_date addTimeInterval: milliseconds / 1000.0];
  }

  NSEvent *event = [application nextEventMatchingMask:NSAnyEventMask
                                            untilDate:until_date
                                               inMode:NSDefaultRunLoopMode
                                              dequeue:YES];
  if (event != nil) {
    [application sendEvent:event];
    [application updateWindows];
  }

  [pool release];
}

void OSXCocoaBumpTopApplication::initOgreCore() {
  // Initialise, we do not want an auto created window, as that will create a carbon window
  Ogre::Root::getSingleton().initialise(false);

  // Build the param list for a embeded cocoa window...
  Ogre::NameValuePairList misc;
  misc["macAPI"] = "cocoa";
  misc["externalWindowHandle"] = Ogre::StringConverter::toString((size_t)ogre_view_);

  // Create the window and load the params
  Ogre::Root::getSingleton().createRenderWindow("", 0, 0, false, &misc);

  render_window_ = [ogre_view_ ogreWindow];
}

Ogre::Vector2 OSXCocoaBumpTopApplication::screen_resolution() {
   NSRect screen_rect = [[NSScreen mainScreen] frame];
   Ogre::Vector2 screen_resolution = Ogre::Vector2(screen_rect.size.width, screen_rect.size.height);
   return screen_resolution;
}

Ogre::Vector2 OSXCocoaBumpTopApplication::taskbar_item_location() {
  return cocoaCoordinatesToOgreCoordinates([ogre_controller_ taskbarItemLocation]);
}

Ogre::Vector2 OSXCocoaBumpTopApplication::window_size() {
  return Ogre::Vector2(render_window_->getWidth(), render_window_->getHeight());
}

FullScreenWindow* OSXCocoaBumpTopApplication::window() {
  return window_;
}

void OSXCocoaBumpTopApplication::set_window(FullScreenWindow* window) {
  window_ = window;
}

void OSXCocoaBumpTopApplication::terminate_application() {
  [[NSApplication sharedApplication] terminate:nil];
}

void OSXCocoaBumpTopApplication::set_ogre_controller(OgreController* ogre_controller) {
  ogre_controller_ = ogre_controller;
}

void OSXCocoaBumpTopApplication::speedUpRenderTimer() {
  [ogre_controller_ speedUpRenderTimer];
}

QString OSXCocoaBumpTopApplication::bumptopVersion() {
  CFBundleRef bundle = ::CFBundleGetMainBundle();
  CFStringRef version_string = (CFStringRef)::CFBundleGetValueForInfoDictionaryKey(bundle,kCFBundleVersionKey);
  return QStringFromNSString((NSString*)version_string);
}

Ogre::Vector2 OSXCocoaBumpTopApplication::mouse_location() {
  NSPoint mouse_location = [[ogre_view_ window] mouseLocationOutsideOfEventStream];
  return cocoaCoordinatesToOgreCoordinates(Ogre::Vector2(mouse_location.x, mouse_location.y));
}

Ogre::Vector2 OSXCocoaBumpTopApplication::cocoaCoordinatesToOgreCoordinates(Ogre::Vector2 cocoa_coordinates) {
  cocoa_coordinates.y = [ogre_view_ bounds].size.height - cocoa_coordinates.y;
  return cocoa_coordinates;
}

bool OSXCocoaBumpTopApplication::isRunningLeopardOrEarlier() {
  return floor(NSAppKitVersionNumber) <= 949;  // 949 == NSAppKitVersionNumber10_5 // NOLINT
}
