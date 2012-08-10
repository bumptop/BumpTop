/*
 *  Copyright 2012 Google Inc. All Rights Reserved.
 *  
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  
 *      http://www.apache.org/licenses/LICENSE-2.0
 *  
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef BUMPTOP_WINDOWS_WINDOWSBUMPTOPAPPLICATION_H_
#define BUMPTOP_WINDOWS_WINDOWSBUMPTOPAPPLICATION_H_

#include <Ogre.h>

#include "BumpTop/BumpTopApplication.h"

class WindowsBumpTopApplication : public BumpTopApplication {
public:
  explicit WindowsBumpTopApplication(HWND window_handle);
  virtual ~WindowsBumpTopApplication();

  virtual QString platform();
  virtual void processOneEvent(int milliseconds = -1);
  //virtual void set_window(FullScreenWindow* window);

  virtual Ogre::Vector2 screen_resolution();
  virtual Ogre::Vector2 window_size();

  virtual QString getResourcePath();
  virtual QString getApplicationDataPath();
  virtual QString getDesktopPath();

  virtual Ogre::Vector2 mouse_location();
  //Ogre::Vector2 cocoaCoordinatesToOgreCoordinates(Ogre::Vector2 cocoa_coordinates);
protected:
  virtual void initOgreCore();

  HWND window_handle_;
  //OgreView *ogre_view_;
  //FullScreenWindow *window_;
};

#endif  // BUMPTOP_WINDOWS_WINDOWSBUMPTOPAPPLICATION_H_
