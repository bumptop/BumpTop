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

#include <OSX/OgreOSXCocoaView.h>

#include "FullScreenWindow.h"
#include "BumpTop/BumpTopApp.h"
#include "BumpTop/OSX/OgreController.h"

@implementation FullScreenWindow

// Overrides the NSWindow's behaviour of restricting the window's frame and size to sit above the dock
- (NSRect)constrainFrameRect:(NSRect)frameRect toScreen:(NSScreen *)screen {
  return frameRect;
}

- (BOOL) canBecomeKeyWindow {
  return YES;
}

// This catches key combinations eg. command+key, shift+key
- (BOOL)performKeyEquivalent:(NSEvent *)theEvent {
  [[self firstResponder] keyDown:theEvent];
  return YES;
}

@end
