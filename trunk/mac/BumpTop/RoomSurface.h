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

#ifndef BUMPTOP_ROOMSURFACE_H_
#define BUMPTOP_ROOMSURFACE_H_

#include <string>
#include <utility>

#include "BumpTop/StaticBox.h"
#include "BumpTop/Room.h"

class Physics;
class BumpTopApp;
class Lasso;

enum MouseIntersectionBehavior {
  ENFORCE_SURFACE_BOUNDS,
  IGNORE_SURFACE_BOUNDS
};

enum PinnableReceiver {
  NOT_PINNABLE_RECEIVER,
  PINNABLE_RECEIVER
};

enum CreateVisualActor {
  NO_VISUAL_ACTOR,
  CREATE_VISUAL_ACTOR
};

class RoomSurface : public StaticBox {
  Q_OBJECT
 public:
  RoomSurface(Ogre::SceneManager *scene_manager, Physics* physics,
              Ogre::SceneNode *parent_ogre_scene_node, const QString& path, PinnableReceiver is_pinnable_receiver);

  void init();
  virtual void init(Ogre::Vector3 normal, Ogre::Real x_size, Ogre::Real z_size,
                    CreateVisualActor create_visual_actor = CREATE_VISUAL_ACTOR);
  virtual void set_size(Ogre::Real local_x_size, Ogre::Real local_z_size);
  virtual void set_thickness(Ogre::Real thickness);

  Ogre::Real distanceAbove(Ogre::Vector3 position);
  std::pair<bool, Ogre::Vector3> mouseIntersectionAbove(Ogre::Vector2 mouse_in_window_space,
                                                        Ogre::Real distance_above,
                                                        MouseIntersectionBehavior mouse_intersection_behavior = ENFORCE_SURFACE_BOUNDS);  // NOLINT
  std::pair<bool, Ogre::Vector3> mouseIntersection(Ogre::Vector2 mouse_in_window_space,
                                                   MouseIntersectionBehavior mouse_intersection_behavior = ENFORCE_SURFACE_BOUNDS);  // NOLINT

  Ogre::Quaternion surfaceOrientationForNormal(Ogre::Vector3 normal);

  void set_room_surface_type(RoomSurfaceType type);
  RoomSurfaceType room_surface_type();

  virtual bool is_pinnable_receiver();
  virtual Ogre::Vector3 normal();
  virtual Room* room();
  virtual void set_room(Room* room);
  virtual void setCameraForSurface();

  virtual bool isWall();

  virtual const QString& path();

  static const Ogre::Real kSurfaceThickness;

  BumpTopCommandSet* supported_context_menu_items();
  bool supportsContextMenuItem(BumpTopCommand* context_menu_option);


 public slots:  // NOLINT
  virtual void mouseDown(MouseEvent* mouse_event);
  virtual void mouseDragged(MouseEvent* mouse_event);
  virtual void mouseUp(MouseEvent* mouse_event);
  virtual void deleteLasso();
  virtual void rightMouseDown(MouseEvent* mouse_event);

 protected:
  std::string meshName();
  std::pair<Ogre::Matrix4, Ogre::Plane> getWorldTransformAndPlane(Ogre::Real distance_above);

  static BumpTopCommandSet* context_menu_items_set;

  RoomSurfaceType room_surface_type_;
  bool is_pinnable_receiver_;
  Ogre::Vector3 normal_;
  BumpTopApp* app_;
  Room* room_;
  Lasso* lasso_;
  QString path_;
  bool create_visual_actor_;
  bool is_lasso_available_for_fade_;
};

#endif  // BUMPTOP_ROOMSURFACE_H_
