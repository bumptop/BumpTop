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

#ifndef BUMPTOP_EVENTMODIFIERFLAGS_H_
#define BUMPTOP_EVENTMODIFIERFLAGS_H_

enum EventModifierMasks{
  NO_KEY_MODIFIERS_MASK = 0,
  ALPHA_SHIFT_KEY_MASK  = NSAlphaShiftKeyMask,
  SHIFT_KEY_MASK        = NSShiftKeyMask,
  CONTROL_KEY_MASK      = NSControlKeyMask,
  ALT_OPTION_KEY_MASK    = NSAlternateKeyMask,
  COMMAND_KEY_MASK      = NSCommandKeyMask,
  NUMERIC_KEYPAD_MASK   = NSNumericPadKeyMask,
  HELP_KEY_MASK         = NSHelpKeyMask,
  FUNCTION_KEY_MASK     = NSFunctionKeyMask,
  DEVICE_INDEPENDENT_MODIFIER_FLAGS_MASK = NSDeviceIndependentModifierFlagsMask
};

#endif  // BUMPTOP_EVENTMODIFIERFLAGS_H_