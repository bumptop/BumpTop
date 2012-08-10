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

#include "BumpTop/RoomSurface.h"

#include <string>
#include <utility>

#include "BumpTop/AppSettings.h"
#include "BumpTop/BumpTopApp.h"
#include "BumpTop/CameraAnimation.h"
#include "BumpTop/FileManager.h"
#include "BumpTop/Lasso.h"
#include "BumpTop/MouseEventManager.h"
#include "BumpTop/OSX/ContextMenu.h"
#include "BumpTop/OSX/EventModifierFlags.h"
#include "BumpTop/QStringHelpers.h"
#include "BumpTop/BumpTopScene.h"
#include "BumpTop/BumpToolbar.h"
#include "BumpTop/StaticBox.h"
#include "BumpTop/VisualActor.h"
#include "BumpTop/VisualPhysicsActorList.h"

#ifndef BUMPTOP_TEST
const Ogre::Real RoomSurface::kSurfaceThickness = 50.0;
#else
const Ogre::Real RoomSurface::kSurfaceThickness = 1.0;
#endif

BumpTopCommandSet* RoomSurface::context_menu_items_set = MakeQSet(7,  // count, must keep this updated
                                                                   NewFolder::singleton(),
                                                                   GetInfo::singleton(),
                                                                   ChangeBackground::singleton(),
                                                                   Undo::singleton(),
                                                                   Redo::singleton(),
                                                                   PileByTypeForAllActors::singleton(),
                                                                   Paste::singleton());


BumpTopCommandSet* RoomSurface::supported_context_menu_items() {
  return context_menu_items_set;
}

bool RoomSurface::supportsContextMenuItem(BumpTopCommand* context_menu_item) {
  return context_menu_items_set->contains(context_menu_item);
}

RoomSurface::RoomSurface(Ogre::SceneManager *scene_manager, Physics* physics, Ogre::SceneNode *parent_ogre_scene_node,
                         const QString& path, PinnableReceiver is_pinnable_receiver)
: StaticBox(scene_manager, physics, parent_ogre_scene_node),
  lasso_(NULL),
  is_pinnable_receiver_(is_pinnable_receiver == PINNABLE_RECEIVER),
  app_(BumpTopApp::singleton()),
  room_(NULL),
  path_(path),
  create_visual_actor_(true) {
}

void RoomSurface::init() {
  StaticBox::init();
}

void RoomSurface::init(Ogre::Vector3 normal, Ogre::Real x_size, Ogre::Real z_size,
                       CreateVisualActor create_visual_actor) {
  create_visual_actor_ = create_visual_actor == CREATE_VISUAL_ACTOR;

  init();

  normal.normalise();
  normal_ = normal;

  // ***Important***
  // The surface is constructed from a "flat" rectangle sitting in the xz-plane, hence
  // we define its size by the x- and z- sizes. Once the surface has been rotated to have the
  // passed-in normal the x_size and z_size will not actually correspond to the surfaces'
  // x and z dimensions in world space.
  StaticBox::set_size(Ogre::Vector3(x_size, kSurfaceThickness, z_size));

  // Rotate the box to correspond to the passed-in normal
  set_orientation(surfaceOrientationForNormal(normal));
}

void RoomSurface::set_size(Ogre::Real local_x_size, Ogre::Real local_z_size) {
  StaticBox::set_size(Ogre::Vector3(local_x_size, kSurfaceThickness, local_z_size));
}

void RoomSurface::set_thickness(Ogre::Real thickness) {
  Ogre::Vector3 current_size = scale();
  StaticBox::set_size(Ogre::Vector3(current_size.x, thickness, current_size.z));
}

std::pair<Ogre::Matrix4, Ogre::Plane> RoomSurface::getWorldTransformAndPlane(Ogre::Real distance_above) {
  //assert(ogre_scene_node()->_getNumWorldTransforms() == 1);
  Ogre::Matrix4 world_transform = ogre_scene_node()->_getFullTransform();

  Ogre::Vector3 scale = ogre_scene_node()->getScale();

  // Create a plane transformed into world space
  // add 50 to the z-height because the half-extent of the box is 50 in object space
  // additionally, add distance_above to the height in a way so that *after* scaling, the plane will be
  //      "distance_above" units above the top the wall surface
  Ogre::Plane plane = Ogre::Plane(Ogre::Vector3::UNIT_Y, 50.0 + distance_above / scale.y);
  plane = world_transform * plane;

  return std::pair<Ogre::Matrix4, Ogre::Plane>(world_transform, plane);
}

Ogre::Real RoomSurface::distanceAbove(Ogre::Vector3 position) {
  std::pair<Ogre::Matrix4, Ogre::Plane> world_transform_and_plane = getWorldTransformAndPlane(0.0);
  const Ogre::Plane plane = world_transform_and_plane.second;
  return plane.getDistance(position);
}

std::pair<bool, Ogre::Vector3> RoomSurface::mouseIntersectionAbove(Ogre::Vector2 mouse_in_window_space,
                                                                   Ogre::Real distance_above,
                                                                   MouseIntersectionBehavior mouse_intersection_behavior) {  // NOLINT
  Ogre::Ray mouse_ray = BumpTopApp::singleton()->mouse_event_manager()->createMouseRay(mouse_in_window_space.x,
                                                                                               mouse_in_window_space.y);
  Ogre::Vector3 scale = ogre_scene_node()->getScale();

  std::pair<Ogre::Matrix4, Ogre::Plane> world_transform_and_plane = getWorldTransformAndPlane(distance_above);
  const Ogre::Matrix4& world_transform = world_transform_and_plane.first;
  const Ogre::Plane plane = world_transform_and_plane.second;
  std::pair<bool, Ogre::Real> intersect_result = Ogre::Math::intersects(mouse_ray, plane);

  Ogre::Vector3 intersect_point = mouse_ray.getPoint(intersect_result.second);

  if (mouse_intersection_behavior == IGNORE_SURFACE_BOUNDS) {
    return std::pair<bool, Ogre::Vector3>(intersect_result.first, intersect_point);
  } else {  // if (mouse_intersection_behavior == ENFORCE_SURFACE_BOUNDS)
    if (intersect_result.first && world_transform.isAffine()) {
      Ogre::Matrix4 world_transform_inverse = world_transform.inverseAffine();
      Ogre::Vector3 intersect_point = mouse_ray.getPoint(intersect_result.second);

      // we'll keep the scaled size of the object, but otherwise transform this to object space
      Ogre::Vector3 intersect_point_scaled_object_space = world_transform_inverse *
                                                          intersect_point *
                                                          ogre_scene_node()->getScale();

      if (intersect_point_scaled_object_space.x <= size().x/2 && intersect_point_scaled_object_space.x >= -size().x/2 &&
          intersect_point_scaled_object_space.z <= size().z/2 && intersect_point_scaled_object_space.z >= -size().z/2) {
        return std::pair<bool, Ogre::Vector3>(intersect_result.first, intersect_point);
      }
    }
  }
  return std::pair<bool, Ogre::Vector3>(false, Ogre::Vector3::ZERO);
}

std::pair<bool, Ogre::Vector3> RoomSurface::mouseIntersection(Ogre::Vector2 mouse_in_window_space,
                                                 MouseIntersectionBehavior mouse_intersection_behavior) {  // NOLINT
  return mouseIntersectionAbove(mouse_in_window_space, 0.0, mouse_intersection_behavior);
}

void RoomSurface::set_room(Room* room) {
  room_ = room;
}

Room* RoomSurface::room() {
  return room_;
}

std::string RoomSurface::meshName() {
  return create_visual_actor_ ? StaticBox::meshName() : "";
}

Ogre::Vector3 RoomSurface::normal() {
  return normal_;
}

bool RoomSurface::is_pinnable_receiver() {
  return is_pinnable_receiver_;
}

void RoomSurface::set_room_surface_type(RoomSurfaceType room_surface_type) {
  room_surface_type_ = room_surface_type;
}

RoomSurfaceType RoomSurface::room_surface_type() {
  return room_surface_type_;
}

void RoomSurface::setCameraForSurface() {
  Ogre::Radian fov_y = app_->camera()->getFOVy();
  Ogre::Radian fov_x = fov_y*(BumpTopApp::singleton()->window_size().x/BumpTopApp::singleton()->window_size().y);

  // Calculates the camera height which will encapsulate the z extent of the floor, and then shifts it up
  // according to the constant "upward_shift_factor" for the angled view
  Ogre::Real distance_above_surface = 1.2*size().x/2.0/Ogre::Math::Tan(fov_x/2.0);

  // for the floor we want the view to be completely flush with the boundaries of the screen
  if (normal() == Ogre::Vector3::UNIT_Y) {
    distance_above_surface = kSurfaceThickness/2.0 + (room_->floor_depth())/2.0/Ogre::Math::Tan(fov_y/2.0);
  }
  Ogre::Real new_near_clipping_distance = (1.0/3.0)*distance_above_surface;
  Ogre::Vector3 camera_position = position() + normal()*distance_above_surface;

  Ogre::Vector3 current_position = app_->camera()->getPosition();
  Ogre::Quaternion current_orientation = app_->camera()->getOrientation();
  // determine the final orientation by making the camera look-at the desired point
  // and making sure it's "up" is as desired
  app_->camera()->setPosition(camera_position);
  app_->camera()->lookAt(position());
  Ogre::Vector3 desiredUp = normal() == Ogre::Vector3::UNIT_Y ? -Ogre::Vector3::UNIT_Z : Ogre::Vector3::UNIT_Y;
  Ogre::Quaternion rotation = app_->camera()->getRealUp().getRotationTo(desiredUp);
  app_->camera()->rotate(rotation);
  // store the finaly position and orientation to be animated to
  Ogre::Quaternion final_orientation = app_->camera()->getOrientation();
  // reset the camera to it's position in preparation for animation
  app_->camera()->setPosition(current_position);
  app_->camera()->setOrientation(current_orientation);

  CameraAnimation* camera_animation = new CameraAnimation(app_->camera(), 500, camera_position,
                                                          final_orientation, new_near_clipping_distance);
  camera_animation->start();

  app_->scene()->set_surface_that_camera_is_zoomed_to(room_surface_type());
}

bool RoomSurface::isWall() {
  return true;
}

void RoomSurface::mouseDown(MouseEvent* mouse_event) {
  StaticBox::mouseDown(mouse_event);
  bool command_key_down = mouse_event->modifier_flags & COMMAND_KEY_MASK;
  bool shift_key_down = mouse_event->modifier_flags & SHIFT_KEY_MASK;
  VisualPhysicsActorList selected_actors;

  if (!(room_surface_type() == BACK_SAFETY_WALL || room_surface_type() == FRONT_SAFETY_WALL ||
        room_surface_type() == LEFT_SAFETY_WALL || room_surface_type() == RIGHT_SAFETY_WALL)) {
    if (mouse_event->num_clicks == 2 && app_->scene()->surface_that_camera_is_zoomed_to() != room_surface_type() && room_surface_type() != FLOOR) {  // NOLINT
      setCameraForSurface();
    } else if (mouse_event->num_clicks == 2 && app_->scene()->surface_that_camera_is_zoomed_to() == NONE && room_surface_type() == FLOOR) {  // NOLINT
      setCameraForSurface();
    } else if (mouse_event->num_clicks == 2) {
      room_->setCameraForRoom();
    }
  }

  // Create a lasso if there isn't one
  if (lasso_ != NULL) {
    lasso_->endFade();
    deleteLasso();
  }
  if (room_ != NULL) {
    room_->set_lasso_selection_active(true);
    selected_actors = room_->selected_actors();
    if (!(command_key_down || shift_key_down)) {
      room_->deselectActors();
      selected_actors.clear();
    }
  }

  lasso_ = new Lasso(app_, room_, !AppSettings::singleton()->use_lasso_setting(), selected_actors);
  lasso_->init();
  app_->mouse_event_manager()->set_global_capture(visual_actor_);
  lasso_->mouseDown(mouse_event);
  is_lasso_available_for_fade_ = true;
}

void RoomSurface::mouseDragged(MouseEvent* mouse_event) {
  StaticBox::mouseDragged(mouse_event);

  // If there is a lasso, pass on the mouse event
  if (lasso_ != NULL) {
    lasso_->mouseDragged(mouse_event);
  }
}

void RoomSurface::mouseUp(MouseEvent* mouse_event) {
  StaticBox::mouseUp(mouse_event);
  if (lasso_ != NULL && is_lasso_available_for_fade_) {
    lasso_->fade(300);
    assert(QObject::connect(lasso_, SIGNAL(onFadeComplete()),  // NOLINT
                            this, SLOT(deleteLasso())));  // NOLINT
    BumpTopApp::singleton()->markGlobalStateAsChanged();
    room_->updateBumpToolbar();
    app_->mouse_event_manager()->set_global_capture(NULL);
    is_lasso_available_for_fade_ = false;
    room_->set_lasso_selection_active(false);
    room_->bump_toolbar()->lassoComplete(room_->selected_actors());
    mouse_event->handled = true;
  } else {
    mouse_event->handled = false;
  }
}

void RoomSurface::rightMouseDown(MouseEvent* mouse_event) {
  StaticBox::rightMouseDown(mouse_event);
  if (lasso_ != NULL) {
    lasso_->endFade();
    deleteLasso();
    BumpTopApp::singleton()->markGlobalStateAsChanged();
    app_->mouse_event_manager()->set_global_capture(NULL);
  }

  if (room_ != NULL && !(mouse_event->modifier_flags & COMMAND_KEY_MASK))
    room_->deselectActors();

  BumpEnvironment env(physics_, room_, scene_manager_);
  VisualPhysicsActorList list_with_just_me;
  list_with_just_me.append(this);
  mouse_event->handled = true;
  launchContextMenu(env, list_with_just_me, mouse_event->mouse_in_window_space);
}

Ogre::Quaternion RoomSurface::surfaceOrientationForNormal(Ogre::Vector3 normal) {
  Ogre::Real pi_by_two = Ogre::Math::PI/2.0;
  Ogre::Matrix3 mat;

  if (normal == Ogre::Vector3::UNIT_Y) {
    mat.FromEulerAnglesXYZ(Ogre::Radian(0), Ogre::Radian(0), Ogre::Radian(0));
  } else if (normal == Ogre::Vector3::UNIT_X) {
    mat.FromEulerAnglesXYZ(Ogre::Radian(pi_by_two), Ogre::Radian(0), Ogre::Radian(-pi_by_two));
  } else if (normal == -Ogre::Vector3::UNIT_X) {
    mat.FromEulerAnglesXYZ(Ogre::Radian(pi_by_two), Ogre::Radian(0), Ogre::Radian(pi_by_two));
  } else if (normal == Ogre::Vector3::UNIT_Z) {
    mat.FromEulerAnglesXYZ(Ogre::Radian(pi_by_two), Ogre::Radian(0), Ogre::Radian(0));
  } else if (normal == -Ogre::Vector3::UNIT_Z) {
    // the last component should ideally be  Ogre::Radian(2*pi_by_two), however the corresponding
    // physics constraint has a problem with that.
    mat.FromEulerAnglesXYZ(Ogre::Radian(pi_by_two), Ogre::Radian(2*pi_by_two), Ogre::Radian(0));
  }
  return Ogre::Quaternion(mat);
}

void RoomSurface::deleteLasso() {
  if (lasso_ != NULL) {
    delete lasso_;
    lasso_ = NULL;
  }
}

const QString& RoomSurface::path() {
  return path_;
}

#include "moc/moc_RoomSurface.cpp"
