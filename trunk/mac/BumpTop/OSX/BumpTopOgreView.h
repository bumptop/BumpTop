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

class DropReceiver;

@interface BumpTopOgreView : OgreView {
  NSTrackingRectTag bounds_tracking_tag_;
  DropReceiver* last_drop_receiver_to_receive_dragging_updated_message_;
  NSDragOperation last_drag_operation_returned_by_drop_receiver_;
  float zoom_in_total_, zoom_out_total_;
}
- (id)initWithFrame:(NSRect)frame;
- (void)frameDidChange:(NSNotification *)notification;
- (void)updateBoundsTrackingTag;
- (void)draggedImage:(NSImage *)image endedAt:(NSPoint)screen_point operation:(NSDragOperation)operation;
+ (Ogre::Vector2) ogreScreenPointFromNSDraggingInfo:(id <NSDraggingInfo>)sender;

@end
