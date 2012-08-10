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

#import "BumpTop/OSX/BumpTopOgreView.h"

#include "BumpTop/Authorization.h"
#include "BumpTop/BumpTopApp.h"
#include "BumpTop/BumpTopCommands.h"
#include "BumpTop/DropReceiver.h"
#include "BumpTop/MouseEventManager.h"
#include "BumpTop/OSX/OSXCocoaBumpTopApplication.h"
#include "BumpTop/QStringHelpers.h"
#include "BumpTop/Room.h"
#include "BumpTop/BumpTopScene.h"

@interface NSGestureEvent : NSEvent {}
- (float) magnification;
@end

@implementation BumpTopOgreView

- (id)initWithFrame:(NSRect)frame {
  self = [super initWithFrame:frame];

  // All this convoluted code about tracking areas is simply so that we get "mouseEntered" and "mouseExited" events
  //http://developer.apple.com/documentation/Cocoa/Conceptual/DisplayWebContent/Tasks/WebKitAvail.html
  // dynamically load the class, so that we can fall-back to a 10.4 (Tiger) friendly method
  Class class_NSTrackingArea = NSClassFromString(@"NSTrackingArea");
  if (class_NSTrackingArea != NULL) {
    // supported on 10.5 and later
    NSTrackingArea *tracking_area;
    tracking_area = [[class_NSTrackingArea alloc] initWithRect:NSZeroRect
                                                       options:(NSTrackingMouseEnteredAndExited |
                                                                NSTrackingActiveAlways |
                                                                NSTrackingInVisibleRect |
                                                                NSTrackingEnabledDuringMouseDrag)
                                                         owner:self
                                                      userInfo:nil];
    [self addTrackingArea:tracking_area];
  } else {
    // 10.4 fallback
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(frameDidChange:)
                                                 name:NSViewFrameDidChangeNotification
                                               object:self];
    [self updateBoundsTrackingTag];
  }

  // Make ourselves a drag destination for files
  [self registerForDraggedTypes:[NSArray arrayWithObjects:NSFilenamesPboardType, NSFilesPromisePboardType, nil]];

  last_drop_receiver_to_receive_dragging_updated_message_ = NULL;
  last_drag_operation_returned_by_drop_receiver_ = NSDragOperationNone;
  zoom_in_total_ = 0;
  zoom_out_total_ = 0;
  return self;
}

- (void)frameDidChange:(NSNotification *)notification {
  [self updateBoundsTrackingTag];
}

// TODO: Find out if all the first responder code at this page matters
// http://www.cocoadev.com/index.pl?AddTrackingRect
// http://www.cocoadev.com/index.pl?MouseEnteredAndExitedNotWorking
- (void)updateBoundsTrackingTag {
  [self removeTrackingRect:bounds_tracking_tag_];
  NSPoint mouse_location = [self convertPoint:[[self window] mouseLocationOutsideOfEventStream] fromView:nil];
  BOOL inside = ([self hitTest:mouse_location] == self);
  bounds_tracking_tag_ = [self addTrackingRect:[self visibleRect] owner:self userData:nil assumeInside:inside];
}

- (void)rightMouseDown:(NSEvent*)mouse_down_event {
  [[self nextResponder] rightMouseDown: mouse_down_event];
}

- (void)rightMouseUp:(NSEvent*)mouse_up_event {
  [[self nextResponder] rightMouseUp: mouse_up_event];
}

- (void)rightMouseDragged:(NSEvent*)mouse_dragged_event {
  [[self nextResponder] rightMouseDragged: mouse_dragged_event];
}

// This captures some pre-canned gestures
- (void)magnifyWithEvent:(NSGestureEvent*)event {
  if (ProAuthorization::singleton()->authorized()) {
    BumpTopApp* bumptop_ = BumpTopApp::singleton();
    BumpEnvironment env = BumpEnvironment(bumptop_->physics(), bumptop_->scene()->room(), bumptop_->ogre_scene_manager());

    if ([event magnification] < 0) {
      zoom_out_total_ += [event magnification];
      if (zoom_out_total_ < -ProAuthorization::singleton()->pinch_gesture_threshold()) {
        Shrink::singleton()->applyToActors(env, bumptop_->scene()->room()->selected_actors());
        zoom_out_total_ = 0;
      }
    } else if ([event magnification] > 0) {
      zoom_in_total_ += [event magnification];
      if (zoom_in_total_ > ProAuthorization::singleton()->pinch_gesture_threshold()) {
        Grow::singleton()->applyToActors(env, bumptop_->scene()->room()->selected_actors());
        zoom_in_total_ = 0;
      }
    }
  }
}
- (void)rotateWithEvent:(id)fp8 {}
- (void)swipeWithEvent:(NSGestureEvent*)event {
  if (ProAuthorization::singleton()->authorized()) {
    BumpTopApp* bumptop_ = BumpTopApp::singleton();
    BumpEnvironment env = BumpEnvironment(bumptop_->physics(), bumptop_->scene()->room(), bumptop_->ogre_scene_manager());
    float deltaY = [event deltaY];

    if (deltaY == ProAuthorization::singleton()->swipe_gesture_threshold()) {
      BreakPile::singleton()->applyToActors(env, bumptop_->scene()->room()->selected_actors());
    } else if (deltaY == -ProAuthorization::singleton()->swipe_gesture_threshold()) {
      CreatePile::singleton()->applyToActors(env, bumptop_->scene()->room()->selected_actors());
    }
  }
}
- (void)beginGestureWithEvent:(id)fp8 {}
- (void)endGestureWithEvent:(id)fp8 {}

// Drag Source methods

- (NSDragOperation)draggingSourceOperationMaskForLocal:(BOOL)isLocal {
  if (isLocal) {
    return NSDragOperationNone;
  }
  return NSDragOperationMove | NSDragOperationCopy | NSDragOperationLink | NSDragOperationDelete;
}

- (void)draggedImage:(NSImage *)image endedAt:(NSPoint)screen_point operation:(NSDragOperation)operation {
  [[self nextResponder] draggedImage:image endedAt:screen_point operation:operation];
}

// Drag Destination methods

- (NSDragOperation)draggingEntered:(id <NSDraggingInfo>)sender {
  // This happens while you're dragging, the first instance you move a mouse over (or back over, if you left) the
  // BumpTopOgreView. We need to return what drag operations are supported, given the position of the mouse cursor

  BumpTopApp *app = BumpTopApp::singleton();
  Ogre::Vector2 ogre_screen_point = [BumpTopOgreView ogreScreenPointFromNSDraggingInfo: sender];

  std::pair<DropReceiver*, NSDragOperation> DropReceiver_and_drag_operations;
  DropReceiver_and_drag_operations = app->mouse_event_manager()->draggingEntered(ogre_screen_point.x,
                                                                                 ogre_screen_point.y,
                                                                                 [sender draggingSourceOperationMask]);

  // remember what object was last dragged over, and what sort of operations it supports
  last_drop_receiver_to_receive_dragging_updated_message_ = DropReceiver_and_drag_operations.first;
  if (last_drop_receiver_to_receive_dragging_updated_message_ == NULL) {
    last_drag_operation_returned_by_drop_receiver_ = NSDragOperationNone;
  } else {
    last_drag_operation_returned_by_drop_receiver_ = DropReceiver_and_drag_operations.second;
  }
  return last_drag_operation_returned_by_drop_receiver_;
}

- (NSDragOperation)draggingUpdated:(id <NSDraggingInfo>)sender {
  // This happens while you're dragging over BumpTop, continuously called as the position of the mouse changes
  // BumpTopOgreView. We need to return what drag operations are supported, given the position of the mouse cursor
  BumpTopApp *app = BumpTopApp::singleton();
  Ogre::Vector2 ogre_screen_point = [BumpTopOgreView ogreScreenPointFromNSDraggingInfo: sender];

  std::pair<DropReceiver*, NSDragOperation> DropReceiver_and_drag_operations;
  DropReceiver_and_drag_operations = app->mouse_event_manager()->draggingUpdated(ogre_screen_point.x,
                                                                                 ogre_screen_point.y,
                                                                                 [sender draggingSourceOperationMask]);

  if (last_drop_receiver_to_receive_dragging_updated_message_ != NULL &&
      last_drop_receiver_to_receive_dragging_updated_message_ != DropReceiver_and_drag_operations.first) {
    DropReceiver_and_drag_operations.first->draggingExited();
  }
  // remember what object was last dragged over, and what sort of operations it supports
  last_drop_receiver_to_receive_dragging_updated_message_ = DropReceiver_and_drag_operations.first;
  if (last_drop_receiver_to_receive_dragging_updated_message_ == NULL) {
    last_drag_operation_returned_by_drop_receiver_ = NSDragOperationNone;
  } else {
    last_drag_operation_returned_by_drop_receiver_ = DropReceiver_and_drag_operations.second;
  }
  return last_drag_operation_returned_by_drop_receiver_;
}

+ (Ogre::Vector2)ogreScreenPointFromNSDraggingInfo:(id <NSDraggingInfo>)sender {
  NSPoint mouse_point = [sender draggingLocation];
  Ogre::Vector2 cocoa_screen_point = Ogre::Vector2(mouse_point.x, mouse_point.y);
  return OSXCocoaBumpTopApplication::singleton()->cocoaCoordinatesToOgreCoordinates(cocoa_screen_point);
}

- (void)draggingExited:(id <NSDraggingInfo>)sender {
  BumpTopApp::singleton()->mouse_event_manager()->draggingExited();
}

QStringList listReceivedFilePaths(id <NSDraggingInfo>sender, DropReceiver* drop_receiver) {
  QStringList list_of_paths;
  NSArray *types = [[sender draggingPasteboard] types];

  bool received_list_of_paths = [types containsObject:NSFilenamesPboardType];
  bool received_list_of_promised_filenames =  [types containsObject:NSFilesPromisePboardType];

  if (received_list_of_paths || received_list_of_promised_filenames) {
    NSArray *files;
    if (received_list_of_paths) {
      files = [[sender draggingPasteboard] propertyListForType:NSFilenamesPboardType];
    } else { // if (received_list_of_promised_filenames)
      NSURL *drop_location = [NSURL fileURLWithPath:NSStringFromQString(drop_receiver->target_path())];
      files = [sender namesOfPromisedFilesDroppedAtDestination:drop_location];
    }

    int number_of_files = [files count];
    for (int i = 0; i < number_of_files; i++) {
      QString file_path;
      if (received_list_of_paths) {
        file_path = QStringFromNSString([files objectAtIndex:i]);
      } else {
        file_path = drop_receiver->target_path() + "/" + QStringFromNSString([files objectAtIndex:i]);
      }

      list_of_paths.push_back(file_path);
    }
  }
  return list_of_paths;
}

- (BOOL)prepareForDragOperation:(id <NSDraggingInfo>)sender {
  // This is the first method called by Cocoa when someone releases a drag & drop on BumpTop. We need to return
  // YES or NO depending on if we'll keep going along with this
  Ogre::Vector2 ogre_screen_point = [BumpTopOgreView ogreScreenPointFromNSDraggingInfo: sender];


  if (last_drop_receiver_to_receive_dragging_updated_message_ != NULL) {
    QStringList list_of_paths = listReceivedFilePaths(sender, last_drop_receiver_to_receive_dragging_updated_message_);

    bool result;
    result = last_drop_receiver_to_receive_dragging_updated_message_->prepareForDragOperation(ogre_screen_point,
                                                                                              list_of_paths,
                                                                                              last_drag_operation_returned_by_drop_receiver_);  // NOLINT
    return result ? YES : NO;
  } else {
    return NO;
  }
}

- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender {
  // This is the second method called by Cocoa when someone releases a drag & drop on BumpTop. We channel this
  // event to the object in BumpTop that receives the drop, which moves/links/copies it.
  // We need to return YES or NO depending on if we were successful

  if (last_drop_receiver_to_receive_dragging_updated_message_ != NULL) {
    Ogre::Vector2 ogre_screen_point = [BumpTopOgreView ogreScreenPointFromNSDraggingInfo: sender];
    QStringList list_of_paths = listReceivedFilePaths(sender, last_drop_receiver_to_receive_dragging_updated_message_);
    bool result = last_drop_receiver_to_receive_dragging_updated_message_->performDragOperation(ogre_screen_point,
                                                                                                list_of_paths,
                                                                                                last_drag_operation_returned_by_drop_receiver_);  // NOLINT

    return result ? YES : NO;
  } else {
    return NO;
  }
}

- (void)concludeDragOperation:(id <NSDraggingInfo>)sender {
  // This is the last method called by Cocoa when someone releases a drag & drop on BumpTop
  // You can present some sort of confirmation here, like a sound
  if (last_drop_receiver_to_receive_dragging_updated_message_ != NULL) {
    last_drop_receiver_to_receive_dragging_updated_message_->concludeDragOperation();
  }
  last_drop_receiver_to_receive_dragging_updated_message_ = NULL;
}


- (BOOL)acceptsFirstMouse:(NSEvent *)theEvent {
  return YES;
}

@end
