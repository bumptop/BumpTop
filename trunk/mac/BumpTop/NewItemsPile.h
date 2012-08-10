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

#ifndef BUMPTOP_NEWITEMSPILE_H_
#define BUMPTOP_NEWITEMSPILE_H_

#include <string>

#include "BumpTop/BumpButton.h"
#include "BumpTop/BumpPile.h"
#include "BumpTop/HighlightActor.h"

class NewItemsPileMarker;

class NewItemsPile : public BumpPile {
  Q_OBJECT
 public:
  explicit NewItemsPile(Ogre::SceneManager *scene_manager,
                    Physics* physics, Room* room, VisualPhysicsActorId unique_id = 0);
  virtual ~NewItemsPile();
  virtual void init();

  virtual void set_render_queue_group(uint8 queue_id);
  virtual void resetQuickView();

  virtual void breakAllItemsExceptDummy();
  virtual void breakItemsConcentrically(QList<VisualPhysicsActorId> actor_ids);
  virtual QList<Ogre::Vector3> children_offsets();
  virtual QList<Ogre::Quaternion> children_orientations();

  virtual bool nameable();
  virtual bool is_new_items_pile();
  virtual void set_selected(bool is_selected);
  virtual void draggingUpdated(MouseEvent* mouse_event);
  virtual VisualPhysicsActorId adjacentActorOfChild(VisualPhysicsActorId child_id, ArrowKey arrow_key);
  
  virtual BumpTopCommandSet* supported_context_menu_items();
  virtual bool supportsContextMenuItem(BumpTopCommand* context_menu_option);
  virtual void writeToBuffer(VisualPhysicsActorBuffer* buffer);
 public slots:
  virtual void mouseDown(MouseEvent* mouse_event);
  virtual void mouseDragged(MouseEvent* mouse_event);
  virtual void mouseUp(MouseEvent* mouse_event);
  virtual void scrollWheel(MouseEvent* mouse_event);
  virtual void globalMouseDown(MouseEvent* mouse_event);
 protected:
  virtual void removeActorFromPileHelper(VisualPhysicsActorId actor_id);
  virtual void offsetTopActor();

  virtual void stackView(bool is_first_time, Ogre::Real scale_factor, Ogre::Real animation_duration);
  Ogre::Real maximum_height();

  NewItemsPileMarker* new_items_pile_marker_;
};

class NewItemsPileMarker : public HighlightActor {
 public:
  explicit NewItemsPileMarker(Ogre::SceneManager *scene_manager, Ogre::SceneNode *parent_ogre_scene_node,
                          Ogre::Real height_of_parent);
};

#endif  // BUMPTOP_NEWITEMSPILE_H_
