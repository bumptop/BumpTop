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

#include "BumpTop/CommandButton.h"

#include "BumpTop/BumpToolbar.h"
#include "BumpTop/BumpTopApp.h"
#include "BumpTop/BumpTopCommands.h"
#include "BumpTop/BumpTopScene.h"
#include "BumpTop/Room.h"

CommandButton::CommandButton(BumpTopToolbarCommand* bump_command)
: bump_command_(bump_command) {
  assert(bump_command->is_toolbar_command());
}

CommandButton::~CommandButton() {
}

void CommandButton::init() {
  // need to load a material synchronously
  // on click, output an event
  OverlayButton::initWithMaterials(bump_command_->toolbar_button_material_active(),
                                   bump_command_->toolbar_button_material_inactive());

  Ogre::MaterialPtr material = Ogre::MaterialPtr(Ogre::MaterialManager::getSingleton().getByName(active_material_name_));  // NOLINT
  material->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureFiltering(Ogre::FO_NONE, Ogre::FO_NONE,
                                                                                     Ogre::FO_NONE);
  material = Ogre::MaterialPtr(Ogre::MaterialManager::getSingleton().getByName(inactive_material_name_));
  material->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureFiltering(Ogre::FO_NONE, Ogre::FO_NONE,
                                                                                     Ogre::FO_NONE);

  OverlayButton::set_size(Ogre::Vector2(100, 110));
}

void CommandButton::buttonAction() {
  OverlayButton::buttonAction();
  Room* room = BumpTopApp::singleton()->scene()->room();
  // we hide it first so the subsequent action brings up the toolbar if necessary
  if (bump_command_->should_dismiss_toolbar()) {
    hide();
  }
  BumpTopApp* bumptop = BumpTopApp::singleton();
  BumpEnvironment env = BumpEnvironment(bumptop->physics(), room, bumptop->ogre_scene_manager());
  bump_command_->applyToActors(env, room->selected_actors());
}
