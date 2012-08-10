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

#import "BumpTop/OSX/SimulateMouseEvent.h"
#import <Foundation/Foundation.h>

#define PID 58680// The process ID of the process that events should be forwarded to (get from Quartz Debug)
#define WID 30// The window ID of the window of the process that events are being forwarded to (get from Quartz Debug)

@implementation SimulateMouseEvent
+ (void)mouseDown: (NSEvent *)event {[self forwardEvent: event];}
+ (void)rightMouseDown: (NSEvent *)event {[self forwardEvent: event];}
+ (void)otherMouseDown: (NSEvent *)event {[self forwardEvent: event];}
+ (void)mouseUp: (NSEvent *)event {[self forwardEvent: event];}
+ (void)rightMouseUp: (NSEvent *)event {[self forwardEvent: event];}
+ (void)otherMouseUp: (NSEvent *)event {[self forwardEvent: event];}
+ (void)mouseMoved: (NSEvent *)event {[self forwardEvent: event];}
+ (void)mouseDragged: (NSEvent *)event {[self forwardEvent: event];}
+ (void)rightMouseDragged: (NSEvent *)event {[self forwardEvent: event];}
+ (void)otherMouseDragged: (NSEvent *)event {[self forwardEvent: event];}

+ (void)forwardEvent: (NSEvent *)event {
  ProcessSerialNumber psn;
  CGEventRef CGEvent;
  NSEvent *customEvent;
  NSPoint point = [event locationInWindow];

  NSLog(@"Received event:");
  NSLog([event description]);

  customEvent = [NSEvent mouseEventWithType: [event type]
                                   location: point
                              modifierFlags: [event modifierFlags]
                                  timestamp: [event timestamp]
                               windowNumber: WID
                                    context: [event context]
                                eventNumber: [event eventNumber]
                                 clickCount: [event clickCount]
                                   pressure: [event pressure]];
  NSLog(@"Sending event:");
  NSLog([customEvent description]);

  CGEvent = [customEvent CGEvent];
  NSAssert(GetProcessForPID(PID, &psn) == noErr, @"GetProcessForPID failed!");
  CGEventPostToPSN(&psn, CGEvent);
}
@end
