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

#ifndef BUMPTOP_OSX_OSXCOCOABUMPTOPAPPLICATION_H_
#define BUMPTOP_OSX_OSXCOCOABUMPTOPAPPLICATION_H_

#include <Ogre.h>
#include <OSX/OgreOSXCocoaView.h>

#include "BumpTop/BumpTopApp.h"

@class FullScreenWindow;
@class OgreController;
class OSXCocoaBumpTopApplication : public BumpTopApp {
public:
  explicit OSXCocoaBumpTopApplication(OgreView* ogre_view, OgreController* ogre_controller);
  virtual ~OSXCocoaBumpTopApplication();

  static OSXCocoaBumpTopApplication* singleton();

  virtual void init();

  virtual QString platform();
  virtual OgreView* ogre_view();
  virtual void processOneEvent(int milliseconds = -1);
  virtual QString bumptopVersion();

  virtual void pushGLContextAndSwitchToOgreGLContext();
  virtual void popGLContext();

  virtual Ogre::Vector2 taskbar_item_location();
  virtual Ogre::Vector2 screen_resolution();
  virtual Ogre::Vector2 window_size();
  virtual void terminate_application();
  virtual FullScreenWindow* window();
  virtual void set_window(FullScreenWindow* window);
  virtual void speedUpRenderTimer();
  virtual void set_ogre_controller(OgreController* ogre_controller);

  virtual Ogre::Vector2 mouse_location();
  Ogre::Vector2 cocoaCoordinatesToOgreCoordinates(Ogre::Vector2 cocoa_coordinates);
  static bool isRunningLeopardOrEarlier();
protected:
  virtual void initOgreCore();

  OgreController* ogre_controller_;
  OgreView *ogre_view_;
  FullScreenWindow* window_;
  NSOpenGLContext *ogre_gl_context_;
  NSOpenGLContext *pushed_gl_context_;
};

#endif  // BUMPTOP_OSX_OSXCOCOABUMPTOPAPPLICATION_H_
