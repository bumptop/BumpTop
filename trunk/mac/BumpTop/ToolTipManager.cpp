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

#include "BumpTop/ToolTipManager.h"

#include "BumpTop/AppSettings.h"
#include "BumpTop/BumpPile.h"
#include "BumpTop/BumpTopApp.h"
#include "BumpTop/Authorization.h"
#include "BumpTop/ToolTipOverlay.h"

SINGLETON_IMPLEMENTATION(ToolTipManager)

ToolTipManager::ToolTipManager()
: taskbar_tooltip_(NULL),
  gridded_pile_tooltip_(NULL),
  pile_flip_tooltip_(NULL),
  name_pile_tooltip_(NULL),
  taskbar_tooltip_will_be_deleted_(false),
  gridded_pile_tooltip_will_be_deleted_(false),
  pile_flip_tooltip_will_be_deleted_(false),
  name_pile_tooltip_will_be_deleted_(false) {
}

void ToolTipManager::deleteTooltip(ToolTipOverlay* tooltip) {
  if (taskbar_tooltip_ == tooltip) {
    delete taskbar_tooltip_;
    taskbar_tooltip_ = NULL;
  } else if (gridded_pile_tooltip_ == tooltip) {
    delete gridded_pile_tooltip_;
    gridded_pile_tooltip_ = NULL;
  } else if (pile_flip_tooltip_ == tooltip) {
    delete pile_flip_tooltip_;
    pile_flip_tooltip_ = NULL;
  } else if (name_pile_tooltip_ == tooltip) {
    delete name_pile_tooltip_;
    name_pile_tooltip_ = NULL;
  }
}

void ToolTipManager::showTaskbarTooltip() {
  if (AppSettings::singleton()->show_taskbar_tooltip() && !taskbar_tooltip_will_be_deleted_) {
    if (taskbar_tooltip_ == NULL) {
      QStringList str_list;
      str_list.push_back("This is the BumpTop");
      str_list.push_back("menu: click it to modify");
      str_list.push_back("preferences");
      taskbar_tooltip_ = new ToolTipOverlay();
      taskbar_tooltip_->initWithTextLines(str_list, TOOLTIP_UP);
      // give 10 pixels of padding for the case of entering "show desktop" mode; otherwise, you wouldn't see
      // what we're pointing to in that case
      taskbar_tooltip_->setCenter(Ogre::Vector2(BumpTopApp::singleton()->taskbar_item_location().x, 10) +
                                  Ogre::Vector2(0, taskbar_tooltip_->size().y/2.0));
      taskbar_tooltip_->fadeIn(0);
    } else {
      taskbar_tooltip_->fadeIn(0);
    }
  }
}

void ToolTipManager::hideTaskbarTooltip() {
  if (taskbar_tooltip_ != NULL) {
    AppSettings::singleton()->set_show_taskbar_tooltip(false);
    AppSettings::singleton()->saveSettingsFile();
    taskbar_tooltip_will_be_deleted_ = true;
    taskbar_tooltip_->fadeOutAndDeleteSelfOnCompletion();
  }
}

void ToolTipManager::showGriddedPileTooltip(BumpPile* pile) {
  if (AppSettings::singleton()->show_gridded_pile_tooltip() && !gridded_pile_tooltip_will_be_deleted_) {
    Ogre::AxisAlignedBox bounding_box = pile->screenBoundingBox();
    Ogre::Vector2 pos = Ogre::Vector2(bounding_box.getMinimum().x, bounding_box.getCenter().y);
    if (gridded_pile_tooltip_ == NULL) {
      QStringList str_list;
      str_list.push_back("Try double-clicking this");
      str_list.push_back("pile");
      gridded_pile_tooltip_ = new ToolTipOverlay();
      gridded_pile_tooltip_->initWithTextLines(str_list, TOOLTIP_RIGHT);
      gridded_pile_tooltip_->setCenter(pos + Ogre::Vector2(-gridded_pile_tooltip_->size().x/2.0, 0));
      gridded_pile_tooltip_->fadeIn(0);
      gridded_pile_tooltip_->followActor(pile);
    } else {
      gridded_pile_tooltip_->followActor(pile);
      gridded_pile_tooltip_->setCenter(pos + Ogre::Vector2(-gridded_pile_tooltip_->size().x/2.0, 0));
      gridded_pile_tooltip_->fadeIn(0);
    }
  }
}

void ToolTipManager::hideGriddedPileTooltip() {
  if (gridded_pile_tooltip_ != NULL) {
    AppSettings::singleton()->set_show_gridded_pile_tooltip(false);
    AppSettings::singleton()->saveSettingsFile();
    gridded_pile_tooltip_will_be_deleted_ = true;
    gridded_pile_tooltip_->fadeOutAndDeleteSelfOnCompletion();
  }
}

void ToolTipManager::showPileFlipTooltip(BumpPile* pile) {
  if (AppSettings::singleton()->show_pile_flip_tooltip()
      &&!AppSettings::singleton()->show_gridded_pile_tooltip()
      && ProAuthorization::singleton()->authorized()
      && !pile->is_new_items_pile()
      && !pile_flip_tooltip_will_be_deleted_) {
    Ogre::AxisAlignedBox bounding_box = pile->screenBoundingBox();
    Ogre::Vector2 pos = Ogre::Vector2(bounding_box.getMinimum().x, bounding_box.getCenter().y);
    if (pile_flip_tooltip_ == NULL) {
      QStringList str_list;
      str_list.push_back("Try scrolling or swiping");
      str_list.push_back("up and down while");
      str_list.push_back("hovering above a pile");
      pile_flip_tooltip_ = new ToolTipOverlay();
      pile_flip_tooltip_->initWithTextLines(str_list, TOOLTIP_RIGHT);
      pile_flip_tooltip_->setCenter(pos + Ogre::Vector2(-pile_flip_tooltip_->size().x/2.0, 0));
      pile_flip_tooltip_->fadeIn(0);
      pile_flip_tooltip_->followActor(pile);
    } else {
      pile_flip_tooltip_->followActor(pile);
      pile_flip_tooltip_->setCenter(pos + Ogre::Vector2(-pile_flip_tooltip_->size().x/2.0, 0));
      pile_flip_tooltip_->fadeIn(0);
    }
  }
}

void ToolTipManager::hidePileFlipTooltip() {
  if (pile_flip_tooltip_ != NULL) {
    AppSettings::singleton()->set_show_pile_flip_tooltip(false);
    AppSettings::singleton()->saveSettingsFile();
    pile_flip_tooltip_will_be_deleted_ = true;
    pile_flip_tooltip_->fadeOutAndDeleteSelfOnCompletion();
  }
}

void ToolTipManager::showNamePileTooltip(BumpPile* pile) {
  if (AppSettings::singleton()->show_name_pile_tooltip()
      && !AppSettings::singleton()->show_gridded_pile_tooltip()
      && !(AppSettings::singleton()->show_pile_flip_tooltip() && ProAuthorization::singleton()->authorized())
      && !name_pile_tooltip_will_be_deleted_
      && !pile->is_new_items_pile()) {
    Ogre::AxisAlignedBox bounding_box = pile->screenBoundingBox();
    Ogre::Vector2 pos = Ogre::Vector2(bounding_box.getMinimum().x, bounding_box.getCenter().y);
    if (name_pile_tooltip_ == NULL) {
      QStringList str_list;
      str_list.push_back("To name a pile, select");
      str_list.push_back("it and press Enter");
      name_pile_tooltip_ = new ToolTipOverlay();
      name_pile_tooltip_->initWithTextLines(str_list, TOOLTIP_RIGHT);
      name_pile_tooltip_->setCenter(pos + Ogre::Vector2(-name_pile_tooltip_->size().x/2.0, 0));
      name_pile_tooltip_->fadeIn(0);
      name_pile_tooltip_->followActor(pile);
    } else {
      name_pile_tooltip_->followActor(pile);
      name_pile_tooltip_->setCenter(pos + Ogre::Vector2(-name_pile_tooltip_->size().x/2.0, 0));
      name_pile_tooltip_->fadeIn(0);
    }
  }
}

void ToolTipManager::hideNamePileTooltip() {
  if (name_pile_tooltip_ != NULL) {
    AppSettings::singleton()->set_show_name_pile_tooltip(false);
    AppSettings::singleton()->saveSettingsFile();
    name_pile_tooltip_will_be_deleted_ = true;
    name_pile_tooltip_->fadeOutAndDeleteSelfOnCompletion();
  }
}
