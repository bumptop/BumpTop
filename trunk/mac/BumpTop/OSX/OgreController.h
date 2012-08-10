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

#define MENU_BAR_HEIGHT 22

class OSXCocoaBumpTopApplication;
class OSXCocoaDragAndDrop;
@class FullScreenWindow;
@class AboutWindowController;
@class BumpTopOgreView;

@interface OgreController : NSResponder {
  IBOutlet BumpTopOgreView *ogre_view_;
  IBOutlet FullScreenWindow *fullScreenWindow;
  IBOutlet FullScreenWindow *splashScreenWindow;
  IBOutlet NSWindowController *fullscreen_window_controller;
  IBOutlet NSWindowController *splashscreen_window_controller;
  IBOutlet AboutWindowController *about_window_controller_;
  IBOutlet NSMenu* status_bar_menu_;
  OSXCocoaBumpTopApplication *bumptop_app_;
  NSRect last_screen_rect_;
  NSRect last_window_rect_;
  NSStatusItem* bumptop_status_item_;

  int render_tick_count_;
  NSTimer* render_timer_;
  bool running_in_idle_mode_;
}
// Methods
- (void)setupWindowsForLaunch;
- (void)manageSplashScreenAndFadeIn;
- (void)updateWindowSizeAndPlacement;
- (void)checkAndCorrectWindowSizeAndPlacementIfChanged;
- (void)makeOgreViewDesktop;
- (void)addToOgreViewResponderChain;
- (void)initRenderTimer;
- (void)slowDownRenderTimer;
- (void)speedUpRenderTimer;
- (void)renderTick;
- (void)setupTaskBarItem;
- (void)scrollWheel:(NSEvent *)scroll_wheel_event;

- (IBAction)showAboutWindow:(id)sender;

- (Ogre::Vector2)getOgreMouseCoordinatesFromNSEvent: (NSEvent*)event;
- (Ogre::Vector2)taskbarItemLocation;

// Getters
- (BumpTopOgreView*) ogre_view;
@end
