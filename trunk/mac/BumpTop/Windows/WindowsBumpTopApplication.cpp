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

#include "BumpTop/Windows/WindowsBumpTopApplication.h"

WindowsBumpTopApplication::WindowsBumpTopApplication(HWND window_handle)
: window_handle_(window_handle) {
}

WindowsBumpTopApplication::~WindowsBumpTopApplication() {
}

QString WindowsBumpTopApplication::platform() {
  return "Windows";
}

void WindowsBumpTopApplication::processOneEvent(int milliseconds) {
}

void WindowsBumpTopApplication::initOgreCore() {
  // Initialise, we do not want an auto created window, as that will create a carbon window
  Ogre::Root::getSingleton().initialise(false);

  // Build the param list for a custom window...
  Ogre::NameValuePairList misc;
  misc["externalWindowHandle"] = Ogre::StringConverter::toString((size_t)window_handle_);

  // Create the window and load the params
  render_window_ = Ogre::Root::getSingleton().createRenderWindow("", 0, 0, false, &misc);
  assert(render_window_ != NULL);
}

Ogre::Vector2 WindowsBumpTopApplication::screen_resolution() {
  Ogre::Vector2 screen_resolution = Ogre::Vector2(0, 0);
  return screen_resolution;
}

/*void WindowsBumpTopApplication::set_window(FullScreenWindow* window) {
  window_ = window;
}*/

Ogre::Vector2 WindowsBumpTopApplication::window_size() {
  return Ogre::Vector2(render_window_->getWidth(), render_window_->getHeight());
}

QString WindowsBumpTopApplication::getResourcePath() {
  return ".";
}

QString WindowsBumpTopApplication::getApplicationDataPath() {
  return ".";
}
QString WindowsBumpTopApplication::getDesktopPath() {
  return ".";
}

Ogre::Vector2 WindowsBumpTopApplication::mouse_location() {
  return Ogre::Vector2(0, 0);
}
