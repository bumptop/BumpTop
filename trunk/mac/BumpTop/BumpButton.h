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

#ifndef BUMPTOP_BUMPBUTTON_H_
#define BUMPTOP_BUMPBUTTON_H_

#include <string>

#include "BumpTop/VisualPhysicsActor.h"

class BumpButton : public VisualPhysicsActor {
  Q_OBJECT

 public:
  BumpButton(Ogre::SceneManager *scene_manager, Physics *physics,
                     Ogre::SceneNode *parent_ogre_scene_node, VisualPhysicsActorId unique_id = 0);
  void initWithImages(QString non_active_image_path, QString active_image_path);
  void setImages(QString non_active_image_path, QString active_image_path);
  void makePhysicsActor(bool physics_enabled);

  void set_disabled(bool disabled);
  bool disabled();
 public slots:  // NOLINT
  virtual void mouseDown(MouseEvent* mouse_event);
  virtual void mouseDragged(MouseEvent* mouse_event);
  virtual void mouseUp(MouseEvent* mouse_event);
  void emitOnRenderTickWhileDepressed();
 signals:  // NOLINT
  void onClicked();
  void onRenderTickWhileDepressed();
 protected:
  virtual void init();
  virtual std::string meshName();
  virtual Ogre::Vector3 absoluteMeshSizeDividedBy100();
  virtual btVector3 physicsSize();
  btScalar mass();
  VisualPhysicsActorType actor_type();
  bool is_mouse_clicked_over_button_;
  QString icon_material_name_;
  QString active_icon_material_name_;
  bool disabled_;
};

#endif  // BUMPTOP_BUMPBUTTON_H_
