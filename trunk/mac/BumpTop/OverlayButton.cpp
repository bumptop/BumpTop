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

#include "BumpTop/OverlayButton.h"

#include "BumpTop/BumpToolbar.h"
#include "BumpTop/BumpTopApp.h"
#include "BumpTop/BumpTopCommands.h"
#include "BumpTop/BumpTopScene.h"
#include "BumpTop/FileManager.h"
#include "BumpTop/MaterialLoader.h"
#include "BumpTop/MouseEventManager.h"
#include "BumpTop/OgreHelpers.h"
#include "BumpTop/QStringHelpers.h"
#include "BumpTop/Room.h"

OverlayButton::OverlayButton() {
}

OverlayButton::~OverlayButton() {
}

void OverlayButton::initWithMaterials(QString active_material_name, QString inactive_material_name) {
  // need to load a material synchronously
  // on click, output an event
  inactive_material_name_ = utf8(inactive_material_name);
  active_material_name_ = utf8(active_material_name);

  Ogre::String address = addressToString(this);
  Ogre::OverlayElement* overlay_element;
  overlay_element = Ogre::OverlayManager::getSingleton().createOverlayElement("Panel",
                                                                              "OverlayButton" + address);  // NOLINT
  panel_ = static_cast<Ogre::OverlayContainer*>(overlay_element);

  panel_->setMetricsMode(Ogre::GMM_PIXELS);
  panel_->setPosition(0, 0);
  panel_->setDimensions(100, 110);
  panel_->setMaterialName(inactive_material_name_);

  panel_->setUserAny(Ogre::Any(static_cast<Clickable*>(this)));
}

void OverlayButton::initWithImages(QString active_image, QString inactive_image) {
  MaterialLoader icon_material_loader;
  icon_material_loader.initAsImageWithFilePath(inactive_image, false);

  MaterialLoader active_icon_material_loader;
  active_icon_material_loader.initAsImageWithFilePath(active_image, false);

  initWithMaterials(active_icon_material_loader.name(), icon_material_loader.name());
}


Ogre::OverlayContainer* OverlayButton::overlay_container() {
  return panel_;
}

void OverlayButton::set_parent(BumpToolbar* parent) {
  parent_ = parent;
}

void OverlayButton::set_position(Ogre::Vector2 position) {
  panel_->setPosition(position.x, position.y);
}

void OverlayButton::set_size(Ogre::Vector2 size) {
  panel_->setDimensions(size.x, size.y);
}

bool OverlayButton::hidden() {
  if (parent_ != NULL)
    return !parent_->visible();
  else
    return !panel_->isVisible();
}

void OverlayButton::mouseDown(MouseEvent* mouse_event) {
  if (hidden() || (parent_ != NULL && parent_->faded()))
    return;
  Clickable::mouseDown(mouse_event);
  mouse_event->handled = true;

  panel_->setMaterialName(active_material_name_);
  BumpTopApp::singleton()->markGlobalStateAsChanged();
  BumpTopApp::singleton()->mouse_event_manager()->set_global_capture(this);
}

void OverlayButton::mouseDragged(MouseEvent* mouse_event) {
  if (hidden() || (parent_ != NULL && parent_->faded()))
    return;

  if (mouse_event->mouse_in_window_space.x > left() &&
      mouse_event->mouse_in_window_space.x < left() + panel_->getWidth() &&
      mouse_event->mouse_in_window_space.y > top() &&
      mouse_event->mouse_in_window_space.y < top() + panel_->getHeight())
    panel_->setMaterialName(active_material_name_);
  else
    panel_->setMaterialName(inactive_material_name_);
  BumpTopApp::singleton()->markGlobalStateAsChanged();
}

Ogre::Real OverlayButton::left() {
  if (parent_ != NULL)
    return parent_->overlay_container()->getLeft() + panel_->getLeft();
  else
    return panel_->getLeft();
}

Ogre::Real OverlayButton::top() {
  if (parent_ != NULL)
    return parent_->overlay_container()->getTop() + panel_->getTop();
  else
    return panel_->getTop();
}

void OverlayButton::hide() {
  if (parent_ != NULL)
    parent_->hide();
  else
    panel_->hide();
}

void OverlayButton::show() {
  if (parent_ != NULL)
    parent_->show();
  else
    panel_->show();
}

void OverlayButton::setAlpha(Ogre::Real alpha) {
  Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().getByName(inactive_material_name_);
  material->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setAlphaOperation(Ogre::LBX_MODULATE,
                                                                                   Ogre::LBS_TEXTURE,
                                                                                   Ogre::LBS_MANUAL,
                                                                                   0.0,
                                                                                   alpha);
  material = Ogre::MaterialPtr(Ogre::MaterialManager::getSingleton().getByName(active_material_name_));
  material->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setAlphaOperation(Ogre::LBX_MODULATE,
                                                                                   Ogre::LBS_TEXTURE,
                                                                                   Ogre::LBS_MANUAL,
                                                                                   0.0,
                                                                                   alpha);
}

void OverlayButton::mouseUp(MouseEvent* mouse_event) {
  BumpTopApp* bumptop =  BumpTopApp::singleton();
  if (hidden() || (parent_ != NULL && parent_->faded())) {
    if (bumptop->mouse_event_manager()->global_capture() == this)
      bumptop->mouse_event_manager()->set_global_capture(NULL);
    return;
  }

  Clickable::mouseUp(mouse_event);
  mouse_event->handled = true;

  bumptop->mouse_event_manager()->set_global_capture(NULL);

  panel_->setMaterialName(inactive_material_name_);
  BumpTopApp::singleton()->markGlobalStateAsChanged();
  if (mouse_event->mouse_in_window_space.x > left() &&
      mouse_event->mouse_in_window_space.x < left() + panel_->getWidth() &&
      mouse_event->mouse_in_window_space.y > top() &&
      mouse_event->mouse_in_window_space.y < top() + panel_->getHeight()) {
    buttonAction();
  }
}

void OverlayButton::buttonAction() {
  emit performAction();
}

#include "BumpTop/moc/moc_OverlayButton.cpp"

