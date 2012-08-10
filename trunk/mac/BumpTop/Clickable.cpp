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

#include "BumpTop/Clickable.h"

#include "BumpTop/OSX/EventModifierFlags.h"

MouseEvent::MouseEvent()
: handled(false),
#if defined(OS_MACOSX)
  drag_operations(NSDragOperationNone),
#endif
  mouse_in_window_space(Ogre::Vector2::ZERO),
  mouse_in_world_space(Ogre::Vector3::ZERO),
  num_clicks(0),
  modifier_flags(NO_KEY_MODIFIERS_MASK),
  capture(false),
  global_capture(false),
  drop_receiver(NULL),
  cancelled(false),
  delta_y(0) {
}

MouseEvent::MouseEvent(Ogre::Vector2 mouse_in_window_space, bool intersects_item, Clickable* item,
                       Ogre::Vector3 mouse_in_world_space, int num_clicks, int modifier_flags, bool capture,
                       bool global_capture, NSDragOperation source_drag_operations,
                       VisualPhysicsActorList items_being_dropped, Ogre::Real delta_y)
: handled(false),
#if defined(OS_MACOSX)
  drag_operations(source_drag_operations),
#endif
  intersects_item(intersects_item),
  item(item),
  mouse_in_window_space(mouse_in_window_space),
  mouse_in_world_space(mouse_in_world_space),
  num_clicks(num_clicks),
  modifier_flags(modifier_flags),
  capture(capture),
  global_capture(global_capture),
  drop_receiver(NULL),
  cancelled(false),
  delta_y(delta_y),
  items_being_dropped(items_being_dropped) {
}

Clickable::Clickable()
: clickable_parent_(NULL) {
}

Clickable::~Clickable() {
}

Clickable* Clickable::clickable_parent() {
  return clickable_parent_;
}

bool Clickable::capture_mouse_events() {
  return false;
}

void Clickable::mouseDown(MouseEvent* mouse_event) {
  emit onMouseDown(mouse_event);
}

void Clickable::mouseUp(MouseEvent* mouse_event) {
  emit onMouseUp(mouse_event);
}

void Clickable::mouseDragged(MouseEvent* mouse_event) {
  emit onMouseDragged(mouse_event);
}

void Clickable::mouseMoved(MouseEvent* mouse_event) {
  emit onMouseMoved(mouse_event);
}

void Clickable::rightMouseDown(MouseEvent* mouse_event) {
  emit onRightMouseDown(mouse_event);
}

void Clickable::rightMouseUp(MouseEvent* mouse_event) {
  emit onRightMouseUp(mouse_event);
}

void Clickable::rightMouseDragged(MouseEvent* mouse_event) {
  emit onRightMouseDragged(mouse_event);
}

void Clickable::draggingEntered(MouseEvent* mouse_event) {
  emit onDraggingEntered(mouse_event);
}

void Clickable::draggingUpdated(MouseEvent* mouse_event) {
  emit onDraggingUpdated(mouse_event);
}

void Clickable::scrollWheel(MouseEvent* mouse_event) {
  emit onScrollWheel(mouse_event);
}

ClickableOverlay::ClickableOverlay()
: Clickable() {
}

ClickableOverlay::~ClickableOverlay() {
}

Ogre::Entity* ClickableOverlay::_entity() {
  return NULL;
}

#include "BumpTop/moc/moc_Clickable.cpp"
