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

#include "BumpTop/BumpToolbar.h"

#include "BumpTop/AlphaElementAnimation.h"
#include "BumpTop/AnimationManager.h"
#include "BumpTop/AppSettings.h"
#include "BumpTop/BumpTopApp.h"
#include "BumpTop/BumpTopCommands.h"
#include "BumpTop/CommandButton.h"
#include "BumpTop/FileManager.h"
#include "BumpTop/MaterialLoader.h"
#include "BumpTop/MouseEventManager.h"
#include "BumpTop/OgreHelpers.h"
#include "BumpTop/OSX/OgreController.h"
#include "BumpTop/QStringHelpers.h"
#include "BumpTop/Room.h"
#include "BumpTop/BumpTopScene.h"
#include "BumpTop/VisualPhysicsActor.h"

Ogre::Real kToolbarItemWidth = 80;
Ogre::Real kToolbarItemHeight = 80;
Ogre::Real kToolbarVerticalPadding = 10;
Ogre::Real kToolbarHorizontalPadding = 5;

Ogre::Real kToolbarItemSpacing = 0;

Ogre::Real kMarginBetweenItemsAndToolbar = 15;
Ogre::Real kToolbarRegionPadding = 75;

int kLeftOrRight = 1;  // -1 for left, 1 for right

BumpToolbar::BumpToolbar(Room* room)
: room_(room),
  use_large_bounding_box_after_lasso_(false) {
}

BumpToolbar::~BumpToolbar() {
}

void BumpToolbar::init() {
  // need to load a material synchronously
  // on click, output an event

  Ogre::String address = addressToString(this);
  Ogre::OverlayElement* toolbar_panel;
  toolbar_panel = Ogre::OverlayManager::getSingleton().createOverlayElement("Panel",
                                                                            "BumpToolbarPanel" + address);  // NOLINT
  panel_ = static_cast<Ogre::OverlayContainer*>(toolbar_panel);

  panel_->setMetricsMode(Ogre::GMM_PIXELS);
  panel_->setPosition(0, 0);
  panel_->setUserAny(Ogre::Any(static_cast<Clickable*>(this)));

  Ogre::OverlayElement* top;
  top = Ogre::OverlayManager::getSingleton().createOverlayElement("Panel",
                                                                  "BumpToolbarPanelTop" + address);  // NOLINT
  top_ = static_cast<Ogre::OverlayContainer*>(top);
  top_material_name_ = utf8(AppSettings::singleton()->global_material_name(TOOLBAR_TOP));
  top_->setMaterialName(top_material_name_);
  top_->setMetricsMode(Ogre::GMM_PIXELS);

  Ogre::OverlayElement* middle;
  middle = Ogre::OverlayManager::getSingleton().createOverlayElement("Panel",
                                                                  "BumpToolbarPanelMiddle" + address);  // NOLINT
  middle_ = static_cast<Ogre::OverlayContainer*>(middle);
  middle_material_name_ = utf8(AppSettings::singleton()->global_material_name(TOOLBAR_MIDDLE));
  middle_->setMaterialName(middle_material_name_);
  middle_->setMetricsMode(Ogre::GMM_PIXELS);

  Ogre::OverlayElement* bottom;
  bottom = Ogre::OverlayManager::getSingleton().createOverlayElement("Panel",
                                                                  "BumpToolbarPanelBottom" + address);  // NOLINT
  bottom_ = static_cast<Ogre::OverlayContainer*>(bottom);
  bottom_material_name_ = utf8(AppSettings::singleton()->global_material_name(TOOLBAR_BOTTOM));
  bottom_->setMaterialName(bottom_material_name_);
  bottom_->setMetricsMode(Ogre::GMM_PIXELS);

  panel_->addChildImpl(top_);
  panel_->addChildImpl(middle_);
  panel_->addChildImpl(bottom_);

  setSize(Ogre::Vector2(100, 400));

  Ogre::MaterialPtr material = Ogre::MaterialPtr(Ogre::MaterialManager::getSingleton().getByName(top_material_name_));
  material->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureFiltering(Ogre::FO_NONE,
                                                                                     Ogre::FO_NONE, Ogre::FO_NONE);
  material = Ogre::MaterialPtr(Ogre::MaterialManager::getSingleton().getByName(middle_material_name_));
  material->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureFiltering(Ogre::FO_NONE,
                                                                                     Ogre::FO_NONE, Ogre::FO_NONE);
  material = Ogre::MaterialPtr(Ogre::MaterialManager::getSingleton().getByName(bottom_material_name_));
  material->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureFiltering(Ogre::FO_NONE,
                                                                                     Ogre::FO_NONE, Ogre::FO_NONE);

  overlay_ = Ogre::OverlayManager::getSingleton().create("BumpToolbarOverlay" + address);
  overlay_->add2D(panel_);
  setAlpha(0);
  faded_ = true;
  overlay_->show();

  assert(QObject::connect(BumpTopApp::singleton(), SIGNAL(onRender()),  // NOLINT
                          this, SLOT(renderTick())));  // NOLINT

  fade_in_timer_ = new Timer();
  assert(QObject::connect(fade_in_timer_, SIGNAL(onTick(Timer*)),  // NOLINT
                          this, SLOT(fadeInTimerComplete(Timer*))));  // NOLINT
}

void BumpToolbar::setSize(Ogre::Vector2 size) {
  panel_->setDimensions(size.x, size.y);
  top_->setDimensions(size.x, 16);
  top_->setPosition(0, 0);
  middle_->setPosition(0, 16);
  middle_->setDimensions(size.x, size.y - 32);
  bottom_->setPosition(0, size.y - 16);
  bottom_->setDimensions(size.x, 16);
  top_->_notifyZOrder(1);
  middle_->_notifyZOrder(0);
  bottom_->_notifyZOrder(2);
}

bool BumpToolbar::set_center(Ogre::Vector2 position, bool fail_on_x_coord_off_screen) {
  Ogre::Vector2 window_size = BumpTopApp::singleton()->window_size();

  // we make sure the toolbar stays vertically on the the screen
  if (position.y + panel_->getHeight()/2.0 > window_size.y) {
    position.y = position.y - (position.y + panel_->getHeight()/2.0 - window_size.y);
  }
  if (position.y - panel_->getHeight()/2.0 < MENU_BAR_HEIGHT) {
    position.y = position.y - (position.y - panel_->getHeight()/2.0 - MENU_BAR_HEIGHT);
  }

  // for horizontal positioning, we return false if the toolbar doesn't fit
  // since for horiz
  if (position.x + panel_->getWidth()/2.0 > window_size.x) {
    if (fail_on_x_coord_off_screen)
      return false;
    else
      position.x = position.x - (position.x + panel_->getWidth()/2.0 - window_size.x);
  }
  if (position.x - panel_->getWidth()/2.0 < 0) {
    if (fail_on_x_coord_off_screen)
      return false;
    else
      position.x = position.x - (position.x - panel_->getHeight()/2.0);
  }

  center_ = position;
  panel_->setPosition(position.x - panel_->getWidth()/2.0, position.y - panel_->getHeight()/2.0);
  return true;
}

Ogre::Vector2 BumpToolbar::center() {
  return center_;
}

void BumpToolbar::fadeOut(Ogre::Real final_alpha) {
    faded_ = true;
    use_large_bounding_box_after_lasso_ = false;
    fade_in_timer_->stop();
    AnimationManager::singleton()->endAnimationsForAlphaElement(this, AnimationManager::STOP_AT_CURRENT_STATE);
    AlphaElementAnimation* animation = new AlphaElementAnimation(this, final_alpha, 250);
    assert(QObject::connect(animation, SIGNAL(onAnimationComplete()),  // NOLINT
                            this, SLOT(fadeOutComplete())));  // NOLINT

    animation->start();
}

void BumpToolbar::fadeOutComplete() {
  overlay_->hide();
}

void BumpToolbar::fadeIn(int delay) {
  faded_ = false;
  fade_in_timer_->start(delay);
}

void BumpToolbar::fadeInTimerComplete(Timer* timer) {
  AnimationManager::singleton()->endAnimationsForAlphaElement(this, AnimationManager::STOP_AT_CURRENT_STATE);
  overlay_->show();
  AlphaElementAnimation* animation = new AlphaElementAnimation(this, 1.0, 250);
  animation->start();
}

#include "BumpTop/DebugAssert.h"
void BumpToolbar::lassoComplete(VisualPhysicsActorList actors) {
  updateForActorList(actors);

  Ogre::Vector2 mouse_position = BumpTopApp::singleton()->mouse_location();
  Ogre::Vector3 mouse_position_3 = Ogre::Vector3(mouse_position.x, mouse_position.y, 0);

  after_lasso_bounding_box_ = addPointToBoundingBox(actors_and_toolbar_screen_bounding_box_, mouse_position_3);

  if (mouse_position.x == after_lasso_bounding_box_.getMaximum().x) {
    after_lasso_bounding_box_.setMaximumX(mouse_position.x + kToolbarRegionPadding);
  } else if (mouse_position.x == after_lasso_bounding_box_.getMinimum().x) {
    after_lasso_bounding_box_.setMinimumX(mouse_position.x - kToolbarRegionPadding);
  }

  if (mouse_position.y == after_lasso_bounding_box_.getMaximum().y) {
    after_lasso_bounding_box_.setMaximumY(mouse_position.y + kToolbarRegionPadding);
  } else if (mouse_position.y == after_lasso_bounding_box_.getMinimum().y) {
    after_lasso_bounding_box_.setMinimumY(mouse_position.y - kToolbarRegionPadding);
  }

  use_large_bounding_box_after_lasso_ = true;

  show(0);
}

void BumpToolbar::show(int delay) {
  if (hidden_) {
    fadeIn(delay);
  }
  hidden_ = false;
}

void BumpToolbar::hide() {
  if (!hidden_)
    fadeOut();
  hidden_ = true;
}

bool BumpToolbar::visible() {
  return !hidden_;
}

void BumpToolbar::setPositionForActorList(VisualPhysicsActorList actors) {
  Ogre::AxisAlignedBox bounding_box;
  bool first_pass = true;
  for_each(VisualPhysicsActor* actor, actors) {
    if (first_pass) {
      bounding_box = worldBoundingBoxToScreenBoundingBox(actor->destinationWorldBoundingBox());
    } else {
      bounding_box = getUninionOfBoundingBoxes(bounding_box, worldBoundingBoxToScreenBoundingBox(actor->destinationWorldBoundingBox()));  // NOLINT
    }
    first_pass = false;
  }

  Ogre::Vector2 upper_right = Ogre::Vector2(bounding_box.getMaximum().x, bounding_box.getMaximum().y);
  Ogre::Vector2 lower_left = Ogre::Vector2(bounding_box.getMinimum().x, bounding_box.getMinimum().y);

  Ogre::Vector2 screen_position;
  if (kLeftOrRight == 1) {  // 1 is right
    screen_position = Ogre::Vector2(upper_right.x, (lower_left.y + upper_right.y)/2.0);
  } else {
    screen_position = Ogre::Vector2(lower_left.x, (lower_left.y + upper_right.y)/2.0);
  }

  actors_bounding_box_ = bounding_box;
  if (!set_center(screen_position +
                  kLeftOrRight*Ogre::Vector2(panel_->getWidth()/2.0 + kMarginBetweenItemsAndToolbar, 0))) {
    if (!set_center(screen_position + kLeftOrRight*Ogre::Vector2(panel_->getWidth()/2.0, 0))) {
      Ogre::Real x_pos = screen_position.x;
      bool conflict = true;
      while (x_pos > 0 && conflict) {
        Ogre::AxisAlignedBox bounding_box(x_pos - panel_->getWidth()/2.0,
                                          screen_position.y - panel_->getHeight()/2.0, -1.0,
                                          x_pos + panel_->getWidth()/2.0,
                                          screen_position.y + panel_->getHeight()/2.0, 1.0);
        conflict = false;
        for_each(VisualPhysicsActor* actor, actors) {
          Ogre::AxisAlignedBox actor_bounding_box = worldBoundingBoxToScreenBoundingBox(actor->destinationWorldBoundingBox());  // NOLINT
          if (actor_bounding_box.intersects(bounding_box)) {
            Ogre::AxisAlignedBox intersection = actor_bounding_box.intersection(bounding_box);
            x_pos = x_pos - kLeftOrRight*1.05*intersection.getSize().x;
            conflict = true;
            break;
          }
        }
      }
      screen_position.x = x_pos;
      set_center(screen_position, false);
    }
  }
}

bool toolbarItemsLessThan(BumpTopCommand* item1, BumpTopCommand* item2) {
  if (item1->number_of_separators_above_me() == item2->number_of_separators_above_me()) {
    return item1->position_within_my_category() < item2->position_within_my_category();
  }
  return item1->number_of_separators_above_me() < item2->number_of_separators_above_me();
}

void BumpToolbar::updateForActorList(VisualPhysicsActorList actors) {
  BumpTopCommandSet supported_toolbar_items;
  bool first_actor = true;
  for_each(VisualPhysicsActor* actor, actors) {
    if (first_actor) {
      supported_toolbar_items = BumpTopCommandSet(*actor->supported_context_menu_items());  // make a copy
      first_actor = false;
    } else {
      supported_toolbar_items = supported_toolbar_items.intersect(*actor->supported_context_menu_items());
    }
  }

  BumpTopApp* bumptop = BumpTopApp::singleton();
  BumpEnvironment env = BumpEnvironment(bumptop->physics(),
                                        room_,
                                        bumptop->ogre_scene_manager());

  BumpTopCommandSet items_to_remove;
  for_each(BumpTopCommand* option, supported_toolbar_items) {
    if (!option->canBeAppliedToActors(env, actors) || !option->is_toolbar_command()) {
      items_to_remove.insert(option);
    }
  }
  supported_toolbar_items.subtract(items_to_remove);

  int num_items = supported_toolbar_items.count();
  Ogre::Real panel_width = kToolbarItemWidth + 2*kToolbarHorizontalPadding;
  Ogre::Real panel_height = kToolbarItemHeight*num_items + kToolbarItemSpacing*(num_items - 1) + 2*kToolbarVerticalPadding;  // NOLINT

  // First we clear the panel of all children
  clearCommandButtons();
  QList<BumpTopCommand*> ordered_toolbar_items = supported_toolbar_items.values();

  qSort(ordered_toolbar_items.begin(),
        ordered_toolbar_items.end(),
        toolbarItemsLessThan);

  setSize(Ogre::Vector2(panel_width, panel_height));
  setPositionForActorList(actors);

  // Increase the size of the bounding box to include the toolbar
  actors_and_toolbar_screen_bounding_box_ = actors_bounding_box_;
  Ogre::Real toolbar_max_x = center().x + panel_width/2.0;
  Ogre::Real toolbar_min_x = center().x - panel_width/2.0;
  Ogre::Real toolbar_max_y = center().y + panel_height/2.0;
  Ogre::Real toolbar_min_y = center().y - panel_height/2.0;

  if (actors_and_toolbar_screen_bounding_box_.getMaximum().x < toolbar_max_x + kToolbarRegionPadding)
    actors_and_toolbar_screen_bounding_box_.setMaximumX(toolbar_max_x + kToolbarRegionPadding);
  if (actors_and_toolbar_screen_bounding_box_.getMinimum().x > toolbar_min_x - kToolbarRegionPadding)
    actors_and_toolbar_screen_bounding_box_.setMinimumX(toolbar_min_x - kToolbarRegionPadding);
  if (actors_and_toolbar_screen_bounding_box_.getMaximum().y < toolbar_max_y + kToolbarRegionPadding)
    actors_and_toolbar_screen_bounding_box_.setMaximumY(toolbar_max_y + kToolbarRegionPadding);
  if (actors_and_toolbar_screen_bounding_box_.getMinimum().y > toolbar_min_y - kToolbarRegionPadding)
    actors_and_toolbar_screen_bounding_box_.setMinimumY(toolbar_min_y - kToolbarRegionPadding);


  // Next we populate the panel with the new children
  int count = 0;
  for_each(BumpTopCommand* option, ordered_toolbar_items) {
    command_buttons_.push_back(option->command_button());
    option->command_button()->setAlpha(AlphaElement::alpha());
    panel_->addChildImpl(option->command_button()->overlay_container());
    Ogre::Vector2 position = Ogre::Vector2(kToolbarHorizontalPadding,
                                           kToolbarVerticalPadding + count*kToolbarItemSpacing + count*kToolbarItemHeight);  // NOLINT
    option->command_button()->set_parent(this);
    option->command_button()->set_size(Ogre::Vector2(kToolbarItemWidth, kToolbarItemHeight));
    option->command_button()->set_position(position);
    count++;
  }
}

void BumpToolbar::clearCommandButtons() {
  for_each(CommandButton* command_button, command_buttons_) {
    panel_->removeChild(command_button->overlay_container()->getName());
  }
  command_buttons_.clear();
}

void BumpToolbar::setAlpha(Ogre::Real alpha) {
  AlphaElement::setAlpha(alpha);
  for_each(CommandButton* command_button, command_buttons_) {
    command_button->setAlpha(alpha);
  }
  Ogre::MaterialPtr material = Ogre::MaterialPtr(Ogre::MaterialManager::getSingleton().getByName(top_material_name_));
  material->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setAlphaOperation(Ogre::LBX_MODULATE,
                                                                                   Ogre::LBS_TEXTURE,
                                                                                   Ogre::LBS_MANUAL,
                                                                                   0.0,
                                                                                   alpha);
  material = Ogre::MaterialPtr(Ogre::MaterialManager::getSingleton().getByName(middle_material_name_));
  material->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setAlphaOperation(Ogre::LBX_MODULATE,
                                                                                   Ogre::LBS_TEXTURE,
                                                                                   Ogre::LBS_MANUAL,
                                                                                   0.0,
                                                                                   alpha);
  material = Ogre::MaterialPtr(Ogre::MaterialManager::getSingleton().getByName(bottom_material_name_));
  material->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setAlphaOperation(Ogre::LBX_MODULATE,
                                                                                   Ogre::LBS_TEXTURE,
                                                                                   Ogre::LBS_MANUAL,
                                                                                   0.0,
                                                                                   alpha);

  BumpTopApp::singleton()->markGlobalStateAsChanged();
}

Ogre::OverlayContainer* BumpToolbar::overlay_container() {
  return panel_;
}

void BumpToolbar::mouseDown(MouseEvent* mouse_event) {
}

void BumpToolbar::mouseDragged(MouseEvent* mouse_event) {
}

void BumpToolbar::mouseUp(MouseEvent* mouse_event) {
}

bool BumpToolbar::faded() {
  return faded_;
}

void BumpToolbar::renderTick() {
  Ogre::Vector2 mouse_position = BumpTopApp::singleton()->mouse_location();
  Ogre::Vector3 mouse_position_3 = Ogre::Vector3(mouse_position.x, mouse_position.y, 0);

  if (!hidden_) {
    bool should_fade_out = false;
    if (use_large_bounding_box_after_lasso_) {
      should_fade_out = !after_lasso_bounding_box_.contains(mouse_position_3);
      if (alpha() == 1)
        use_large_bounding_box_after_lasso_ = !actors_and_toolbar_screen_bounding_box_.contains(mouse_position_3);
    } else {
      should_fade_out = (!faded_ && (((alpha() == 1) && !actors_and_toolbar_screen_bounding_box_.contains(mouse_position_3)) ||  // NOLINT
                                     ((alpha() < 1) && !actors_bounding_box_.contains(mouse_position_3))));
    }

    if (command_buttons_.count() == 0) {
      if (!faded_) {
        fadeOut();
      }
    } else if (should_fade_out) {
      fadeOut();
    } else if (BumpTopApp::singleton()->context_menu_open()) {
      fadeOut();
    } else if (faded_ && actors_bounding_box_.contains(mouse_position_3)) {
      fadeIn(250);
    }
  }
}

#include "BumpTop/moc/moc_BumpToolbar.cpp"
