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

#include "BumpTop/MouseEventManager.h"

#include <utility>

#include "BumpTop/BumpBoxLabel.h"
#include "BumpTop/BumpTopApp.h"
#include "BumpTop/Clickable.h"
#include "BumpTop/Cube.h"
#include "BumpTop/DropReceiver.h"
#include "BumpTop/OSX/EventModifierFlags.h"
#include "BumpTop/PolygonLevelRaySceneQuery.h"
#include "BumpTop/VisualPhysicsActorList.h"

MouseEventManager::MouseEventManager(BumpTopApp* bumptop)
: bumptop_(bumptop),
  global_capture_object_(NULL),
  is_handling_mouse_up_(false) {
  polygon_level_ray_scene_query_ = new PolygonLevelRaySceneQuery(bumptop_->ogre_scene_manager());
}

MouseEventManager::~MouseEventManager() {
}

Ogre::Ray MouseEventManager::createMouseRay(float x, float y) {
  float normalized_x = x / bumptop_->viewport()->getActualWidth();
  float normalized_y = y / bumptop_->viewport()->getActualHeight();

  return bumptop_->camera()->getCameraToViewportRay(normalized_x,
                                                    normalized_y);
}

void MouseEventManager::set_global_capture(Clickable *capture_object) {
  global_capture_object_ = capture_object;
}

Clickable *MouseEventManager::global_capture() {
  return global_capture_object_;
}

void MouseEventManager::mouseDown(float x, float y, int num_clicks, int modifier_flags) {
  routeMouseEvent(MOUSE_DOWN, x, y, num_clicks, modifier_flags);
}

void MouseEventManager::mouseMoved(float x, float y, int num_clicks, int modifier_flags) {
  routeMouseEvent(MOUSE_MOVED, x, y, num_clicks, modifier_flags);
}

void MouseEventManager::mouseDragged(float x, float y, int num_clicks, int modifier_flags) {
  routeMouseEvent(MOUSE_DRAGGED, x, y, num_clicks, modifier_flags);
}

void MouseEventManager::mouseUp(float x, float y, int num_clicks, int modifier_flags) {
  // Need to set this variable "is handling mouse up"-- otherwise, in certain situations, on a mouse up, we also
  // get a mouseexited event, and then that causes lots of weirdness.
  // how to reproduce: drag internally a file to a folder where a file with the given name already exists
  is_handling_mouse_up_ = true;
  routeMouseEvent(MOUSE_UP, x, y, num_clicks, modifier_flags);
  is_handling_mouse_up_ = false;
}

void MouseEventManager::rightMouseDown(float x, float y, int num_clicks, int modifier_flags) {
  routeMouseEvent(RIGHT_MOUSE_DOWN, x, y, num_clicks, modifier_flags);
}

void MouseEventManager::rightMouseDragged(float x, float y, int num_clicks, int modifier_flags) {
  routeMouseEvent(RIGHT_MOUSE_DRAGGED, x, y, num_clicks, modifier_flags);
}

void MouseEventManager::rightMouseUp(float x, float y, int num_clicks, int modifier_flags) {
  routeMouseEvent(RIGHT_MOUSE_UP, x, y, num_clicks, modifier_flags);
}

void MouseEventManager::scrollWheel(float x, float y, int num_clicks, int modifier_flags, float delta_y) {
  routeMouseEvent(SCROLL_WHEEL, x, y, num_clicks, modifier_flags,
                  NSDragOperationNone, VisualPhysicsActorList(), false, delta_y);
}

void MouseEventManager::mouseEntered() {
  emit onMouseEntered();
}

void MouseEventManager::mouseExited() {
  // Need to act based on this variable "is handling mouse up"-- otherwise, in certain situations, on a mouse up, we
  // also get a mouseexited event, and then that causes lots of weirdness.
  // how to reproduce: drag internally a file to a folder where a file with the given name already exists
  if (!is_handling_mouse_up_) {
    emit onMouseExited();
  }
}

void MouseEventManager::draggingExited() {
  emit onDraggingExited();
}

std::pair<bool, Ogre::Vector3> MouseEventManager::intersectWithEntityMesh(Ogre::Entity *entity,
                                                                          Ogre::Vector2 mouse_in_window_space) {
  std::pair<bool, Ogre::Real> intersection(false, 0.0);

  Ogre::Ray mouse_ray = createMouseRay(mouse_in_window_space.x, mouse_in_window_space.y);
  intersection = polygon_level_ray_scene_query_->intersectWithEntityMesh(entity, mouse_ray);

  if (intersection.first) {
    return std::pair<bool, Ogre::Vector3>(true, mouse_ray.getPoint(intersection.second));
  } else {
    return std::pair<bool, Ogre::Vector3>(false, Ogre::Vector3::ZERO);
  }
}

std::pair<DropReceiver*, NSDragOperation> MouseEventManager::draggingEntered(float x, float y,
                                                                             NSDragOperation source_drag_operations,
                                                                             VisualPhysicsActorList items_being_dropped,
                                                                             bool disable_global_mouse_capture) {
  MouseEvent mouse_event = routeMouseEvent(DRAGGING_ENTERED, x, y, 0, NO_KEY_MODIFIERS_MASK,
                                           source_drag_operations, items_being_dropped, disable_global_mouse_capture);
  return std::pair<DropReceiver*, NSDragOperation>(mouse_event.drop_receiver, mouse_event.drag_operations);
}

std::pair<DropReceiver*, NSDragOperation> MouseEventManager::draggingUpdated(float x, float y,
                                                                             NSDragOperation source_drag_operations,
                                                                             VisualPhysicsActorList items_being_dropped,
                                                                             bool disable_global_mouse_capture) {
  MouseEvent mouse_event = routeMouseEvent(DRAGGING_UPDATED, x, y, 0, NO_KEY_MODIFIERS_MASK,
                                           source_drag_operations, items_being_dropped, disable_global_mouse_capture);
  return std::pair<DropReceiver*, NSDragOperation>(mouse_event.drop_receiver, mouse_event.drag_operations);
}

MouseEvent MouseEventManager::routeMouseEventThroughOverlayHierarchy(Ogre::OverlayContainer* overlay_container,
                                                                     MouseEventType mouse_event_type, float x,
                                                                     float y, int num_clicks, int modifier_flags,
                                                                     NSDragOperation source_drag_operations,
                                                                     Ogre::Real delta_y) {
  Ogre::OverlayContainer::ChildContainerIterator child_iterator = overlay_container->getChildContainerIterator();
  while (child_iterator.hasMoreElements()) {
    Ogre::OverlayContainer* child_overlay_container = child_iterator.getNext();
    MouseEvent mouse_event = routeMouseEventThroughOverlayHierarchy(child_overlay_container, mouse_event_type,
                                                                    x - overlay_container->getLeft(),
                                                                     y-overlay_container->getTop(),
                                                                    num_clicks, modifier_flags,
                                                                    source_drag_operations, delta_y);
    if (mouse_event.handled)
      return mouse_event;
  }
  if (Ogre::any_cast<Clickable*>(&overlay_container->getUserAny()) &&
        x > overlay_container->getLeft() &&
        x < overlay_container->getLeft() + overlay_container->getWidth() &&
        y > overlay_container->getTop() &&
        y < overlay_container->getTop() + overlay_container->getHeight()) {
    Clickable* clickable = Ogre::any_cast<Clickable*>(overlay_container->getUserAny());
    MouseEvent mouse_event = MouseEvent(Ogre::Vector2(x, y), true, clickable, Ogre::Vector3::ZERO,
                                        num_clicks, modifier_flags, false, false, source_drag_operations,
                                        VisualPhysicsActorList(), delta_y);
    bubbleMouseEvent(mouse_event_type, NULL, clickable, &mouse_event);
    if (mouse_event.handled) {
      return mouse_event;
    }
  }
  MouseEvent empty_event;
  return empty_event;
}

struct LabelAndDistance {
  BumpBoxLabel* label;
  Ogre::Real distance;
};

bool LabelAndDistanceLessThan(LabelAndDistance a, LabelAndDistance b) {
  return (a.distance < b.distance);
}

MouseEvent MouseEventManager::routeMouseEvent(MouseEventType mouse_event_type, float x, float y, int num_clicks,
                                              int modifier_flags, NSDragOperation source_drag_operations,
                                              VisualPhysicsActorList items_being_dropped,
                                              bool disable_global_mouse_capture, Ogre::Real delta_y) {
  Ogre::Ray mouse_ray = createMouseRay(x, y);
  Ogre::Vector2 mouse_in_window_space = Ogre::Vector2(x, y);

  MouseEvent mouse_event = MouseEvent(mouse_in_window_space, false, NULL, Ogre::Vector3::ZERO, num_clicks,
                                      modifier_flags, false, false, source_drag_operations,
                                      items_being_dropped, delta_y);
  // emitMouseSignal(mouse_event_type, NULL, NULL, &mouse_event);

  if (!disable_global_mouse_capture) {
    if (global_capture_object_ != NULL) {
      std::pair<bool, Ogre::Real> intersection(false, 0.0);

      if (global_capture_object_->_entity() != NULL) {
        intersection = polygon_level_ray_scene_query_->intersectWithEntityMesh(global_capture_object_->_entity(),
                                                                               mouse_ray);
      }

      bool intersects_item = intersection.first;
      Ogre::Vector3 mouse_in_world_space = intersects_item ? mouse_ray.getPoint(intersection.second) : Ogre::Vector3::ZERO;  // NOLINT
      MouseEvent mouse_event = MouseEvent(mouse_in_window_space, intersects_item, global_capture_object_,
                                          mouse_in_world_space, num_clicks, modifier_flags, false, true,
                                          source_drag_operations, items_being_dropped, delta_y);
      bubbleMouseEvent(mouse_event_type, NULL, global_capture_object_, &mouse_event);
      return mouse_event;
    }
  }

  // Send mouse event to the overlays
  Ogre::OverlayManager::OverlayMapIterator overlay_iterator = Ogre::OverlayManager::getSingleton().getOverlayIterator();
  while (overlay_iterator.hasMoreElements()) {
    Ogre::Overlay* overlay = overlay_iterator.getNext();
    Ogre::Overlay::Overlay2DElementsIterator overlay_containers_iterator = overlay->get2DElementsIterator();
    while (overlay_containers_iterator.hasMoreElements()) {
      Ogre::OverlayContainer* overlay_container = overlay_containers_iterator.getNext();
      MouseEvent mouse_event = routeMouseEventThroughOverlayHierarchy(overlay_container, mouse_event_type, x, y,
                                                                      num_clicks, modifier_flags,
                                                                      source_drag_operations, delta_y);

      if (mouse_event.handled)
        return mouse_event;
    }
  }


  QList<LabelAndDistance> intersecting_labels;
  for_each(BumpBoxLabel* label, BumpBoxLabelManager::singleton()->labels()) {
    if (label->visible() &&
        x > label->position_in_pixel_coords().x &&
        x < label->position_in_pixel_coords().x + label->width_of_drawn_region() &&
        y > label->position_in_pixel_coords().y &&
        y < label->position_in_pixel_coords().y + label->height()) {
      LabelAndDistance lad;
      lad.label = label;

      std::pair<bool, Ogre::Real> intersection = Ogre::Math::intersects(mouse_ray, label->plane());
      lad.distance = intersection.second;

      intersecting_labels.push_back(lad);
    }
  }

  qSort(intersecting_labels.begin(), intersecting_labels.end(), LabelAndDistanceLessThan);

  polygon_level_ray_scene_query_->setRay(mouse_ray);
  BumpRaySceneQueryResult& ray_scene_query_result = polygon_level_ray_scene_query_->execute();


  for_each(Ogre::RaySceneQueryResultEntry ray_scene_query_result_entry, ray_scene_query_result) {
    Ogre::MovableObject* movable = ray_scene_query_result_entry.movable;

    while (intersecting_labels.size() > 0 && ray_scene_query_result_entry.distance > intersecting_labels[0].distance) {
      Clickable *clickable = intersecting_labels[0].label;
      MouseEvent mouse_event = MouseEvent(Ogre::Vector2(x, y), true, clickable, Ogre::Vector3::ZERO,
                                          num_clicks, modifier_flags, false, false, source_drag_operations,
                                          VisualPhysicsActorList(), delta_y);
      bubbleMouseEvent(mouse_event_type, NULL, clickable, &mouse_event);
      if (mouse_event.handled) {
        return mouse_event;
      }
      intersecting_labels.pop_front();
    }
    if (Ogre::any_cast<Clickable*>(&movable->getUserAny())) {
      Clickable *clickable = Ogre::any_cast<Clickable*>(movable->getUserAny());
      Ogre::Vector3 mouse_in_world_space = mouse_ray.getPoint(ray_scene_query_result_entry.distance);
      MouseEvent mouse_event = MouseEvent(mouse_in_window_space, true, clickable, mouse_in_world_space, num_clicks,
                                          modifier_flags, false, false, source_drag_operations,
                                          items_being_dropped, delta_y);
      bubbleMouseEvent(mouse_event_type, movable, clickable, &mouse_event);
      if (mouse_event.handled) {
        return mouse_event;
      }
    }
  }

  mouse_event = MouseEvent(mouse_in_window_space, false, NULL, Ogre::Vector3::ZERO, num_clicks,
                           modifier_flags, false, false, source_drag_operations,
                           items_being_dropped, delta_y);
  bubbleMouseEvent(mouse_event_type, NULL, NULL, &mouse_event);
  return mouse_event;
}

void MouseEventManager::bubbleMouseEvent(MouseEventType mouse_event_type, Ogre::MovableObject* movable,
                                         Clickable* clickable, MouseEvent* event) {
  emitMouseSignal(mouse_event_type, movable, clickable, event);
  if (clickable != NULL) {
    QStack<Clickable*> ancestor_stack;

    // "capture" phase --> send events, starting with topmost parent
    event->capture = true;

    Clickable *current_clickable = clickable->clickable_parent();
    // create a stack of all ancestors
    while (current_clickable != NULL) {
      ancestor_stack.push(current_clickable);
      current_clickable = current_clickable->clickable_parent();
    }

    // then propagate event to all ancestors, starting with the oldest
    while (!event->handled && !event->cancelled && !ancestor_stack.isEmpty()) {
      current_clickable = ancestor_stack.pop();
      if (current_clickable->capture_mouse_events()) {
        sendMouseEvent(mouse_event_type, movable, current_clickable, event);
      }
    }

    event->capture = false;

    // "target" phase --> send event to the main item himself, and then back up to the parents
    current_clickable = clickable;
    while (!event->handled && !event->cancelled && current_clickable != NULL) {
      sendMouseEvent(mouse_event_type, movable, current_clickable, event);
      if (event->handled || event->cancelled) {  // we need this check in case the event handling deleted the clickable
        break;
      }
      current_clickable = current_clickable->clickable_parent();
    }
    // reset the cancelled variable for next object in the z order
    event->cancelled = false;
  }
}

void MouseEventManager::sendMouseEvent(MouseEventType mouse_event_type, Ogre::MovableObject* movable,
                                       Clickable* clickable, MouseEvent* e) {
  switch (mouse_event_type) {
    case MOUSE_DOWN:
      clickable->mouseDown(e);
      break;
    case MOUSE_DRAGGED:
      clickable->mouseDragged(e);
      break;
    case MOUSE_UP:
      clickable->mouseUp(e);
      break;
    case MOUSE_MOVED:
      clickable->mouseMoved(e);
      break;
    case RIGHT_MOUSE_DOWN:
      clickable->rightMouseDown(e);
      break;
    case RIGHT_MOUSE_DRAGGED:
      clickable->rightMouseDragged(e);
      break;
    case RIGHT_MOUSE_UP:
      clickable->rightMouseUp(e);
      break;
    case DRAGGING_ENTERED:
      clickable->draggingEntered(e);
      break;
    case DRAGGING_UPDATED:
      clickable->draggingUpdated(e);
      break;
    case SCROLL_WHEEL:
      clickable->scrollWheel(e);
      break;
  }
}

void MouseEventManager::emitMouseSignal(MouseEventType mouse_event_type, Ogre::MovableObject* movable,
                                        Clickable* clickable, MouseEvent* e) {
  switch (mouse_event_type) {
    case MOUSE_DOWN:
      emit onMouseDown(e);
      break;
    case DRAGGING_ENTERED:
      emit onDraggingEntered();
      break;
    case DRAGGING_UPDATED:
      emit onDraggingUpdated();
      break;
  }
}

#include "BumpTop/moc/moc_MouseEventManager.cpp"
