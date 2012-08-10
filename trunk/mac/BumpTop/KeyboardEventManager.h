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

#ifndef BUMPTOP_KEYBOARDEVENTMANAGER_H_
#define BUMPTOP_KEYBOARDEVENTMANAGER_H_

#include "BumpTop/BumpTopCommands.h"

enum ArrowKey {
  // using values defined in NSEvent.h
  ARROW_UP = NSUpArrowFunctionKey,
  ARROW_DOWN = NSDownArrowFunctionKey,
  ARROW_LEFT = NSLeftArrowFunctionKey,
  ARROW_RIGHT = NSRightArrowFunctionKey
};

struct KeyboardEvent {
  KeyboardEvent();
  KeyboardEvent(QString characters, int modifier_flags);
  QString characters;
  int modifier_flags;
};

class BumpTopApp;

class KeyboardEventManager : public QObject {
  Q_OBJECT
 public:
  explicit KeyboardEventManager(BumpTopApp* bumptop);
  ~KeyboardEventManager();

  void goToMenu(NSString* menu_item);
  void keyDown(KeyboardEvent keyboard_event);
  void flagsChanged(KeyboardEvent keyboard_event);
  bool command_key_down();
  bool option_key_down();

 private:
  BumpTopApp* bumptop_;
  bool command_key_down_;
  bool option_key_down_;
};
#endif  // BUMPTOP_KEYBOARDEVENTMANAGER_H_

