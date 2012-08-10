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

#include <string>

#include "BumpTop/BumpTopApp.h"
#include "BumpTop/Clickable.h"
#include "BumpTop/BumpButton.h"
#include "BumpTop/FileManager.h"
#include "BumpTop/MaterialLoader.h"
#include "BumpTop/MouseEventManager.h"
#include "BumpTop/QStringHelpers.h"
#include "BumpTop/Shape.h"
#include "BumpTop/VisualActor.h"

BumpButton::BumpButton(Ogre::SceneManager *scene_manager, Physics *physics,
                         Ogre::SceneNode *parent_ogre_scene_node, VisualPhysicsActorId unique_id)
: VisualPhysicsActor(scene_manager, physics, parent_ogre_scene_node, unique_id),
  is_mouse_clicked_over_button_(false),
  disabled_(false) {
}

void BumpButton::initWithImages(QString non_active_image_path, QString active_image_path) {
  VisualPhysicsActor::init();

  setImages(non_active_image_path, active_image_path);
}

void BumpButton::setImages(QString non_active_image_path, QString active_image_path) {
  MaterialLoader icon_material_loader;
  icon_material_loader.initAsImageWithFilePath(non_active_image_path, false);
  icon_material_name_ = icon_material_loader.name();

  MaterialLoader active_icon_material_loader;
  active_icon_material_loader.initAsImageWithFilePath(active_image_path, false);
  active_icon_material_name_ = active_icon_material_loader.name();

  set_material_name(icon_material_name_);
}

void BumpButton::makePhysicsActor(bool physics_enabled) {
  // no physics actor -- don't do anything!
}

void BumpButton::set_disabled(bool disabled) {
  disabled_ = disabled;
  if (disabled_) {
    visual_actor_->set_material_name(active_icon_material_name_);
    if (BumpTopApp::singleton()->mouse_event_manager()->global_capture() == this) {
      BumpTopApp::singleton()->mouse_event_manager()->set_global_capture(NULL);
    }
    BumpTopApp::singleton()->disconnect(this);
    is_mouse_clicked_over_button_ = false;
  } else {
    visual_actor_->set_material_name(icon_material_name_);
  }
}

bool BumpButton::disabled() {
  return disabled_;
}

void BumpButton::mouseDown(MouseEvent* mouse_event) {
  if (!disabled()) {
    set_material_name(active_icon_material_name_);
    is_mouse_clicked_over_button_ = true;
    BumpTopApp::singleton()->mouse_event_manager()->set_global_capture(this);
    mouse_event->handled = true;
    assert(QObject::connect(BumpTopApp::singleton(), SIGNAL(onRender()),  // NOLINT
                            this, SLOT(emitOnRenderTickWhileDepressed())));  // NOLINT
  }
}

void BumpButton::emitOnRenderTickWhileDepressed() {
  emit onRenderTickWhileDepressed();
}

void BumpButton::mouseDragged(MouseEvent* mouse_event) {
  if (!disabled()) {
    if (is_mouse_clicked_over_button_ && !mouse_event->intersects_item) {
      set_material_name(icon_material_name_);
      is_mouse_clicked_over_button_ = false;
    } else if (!is_mouse_clicked_over_button_ && mouse_event->intersects_item) {
      set_material_name(active_icon_material_name_);
      is_mouse_clicked_over_button_ = true;
    }
    mouse_event->handled = true;
  }
}

void BumpButton::mouseUp(MouseEvent* mouse_event) {
  if (!disabled()) {
    BumpTopApp::singleton()->disconnect(this);
    set_material_name(icon_material_name_);
    is_mouse_clicked_over_button_ = false;
    BumpTopApp::singleton()->mouse_event_manager()->set_global_capture(NULL);
    mouse_event->handled = true;

    if (mouse_event->intersects_item)
      emit onClicked();  // this emit probably deletes self, so don't access any members after this line
  }
}

void BumpButton::init() {
  VisualPhysicsActor::init();
}

std::string BumpButton::meshName() {
  return Shape::singleton()->flat_square();
}

Ogre::Vector3 BumpButton::absoluteMeshSizeDividedBy100() {
  return Ogre::Vector3(1.0, 0, 1.0);
}

btVector3 BumpButton::physicsSize() {
  return btVector3(1.0, 0, 1.0);
}

btScalar BumpButton::mass() {
  return 1.0;
}

VisualPhysicsActorType BumpButton::actor_type() {
  return NULL_ACTOR_TYPE;
}

#include "moc/moc_BumpButton.cpp"
