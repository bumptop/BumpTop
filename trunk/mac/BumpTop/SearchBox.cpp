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

#include "BumpTop/SearchBox.h"

#include <QtCore/QList>
#include <string>
#include <utility>

#include "BumpTop/AnimationManager.h"
#include "BumpTop/AppSettings.h"
#include "BumpTop/Authorization.h"
#include "BumpTop/BumpTopApp.h"
#include "BumpTop/BumpBox.h"
#include "BumpTop/BumpPile.h"
#include "BumpTop/FileItem.h"
#include "BumpTop/MaterialBlendAnimation.h"
#include "BumpTop/OgreHelpers.h"
#include "BumpTop/QStringHelpers.h"
#include "BumpTop/Room.h"
#include "BumpTop/RoomSurface.h"
#include "BumpTop/RoomItemPoseConstraints.h"
#include "BumpTop/SearchBoxOverlay.h"
#include "BumpTop/Timer.h"
#include "BumpTop/VisualPhysicsActor.h"
#include "BumpTop/VisualPhysicsActorAnimation.h"

SearchBox::SearchBox(Room* room)
: room_(room),
  overlay_(NULL),
  user_search_string_(""),
  is_open_(false) {
}

SearchBox::~SearchBox() {
  if (overlay_ != NULL) {
    delete overlay_;
  }
  overlay_ = NULL;
}

void SearchBox::init() {
  overlay_ = new SearchBoxOverlay("");
  overlay_->init();
  assert(QObject::connect(&timer_, SIGNAL(onTick(Timer*)), // NOLINT
                          this, SLOT(startFadingAnimation()))); // NOLINT

  assert(QObject::connect(overlay_, SIGNAL(onFadeComplete()),  // NOLINT
                          this, SLOT(hideOverlayAfterFading())));  // NOLINT
}

void SearchBox::startSearch(QString key) {
  if (is_open_) {
    if (key == "\n") {
    } else if (key == "\033") {  // \033 is esc key
      user_search_string_ = "";
      deactivateSearchBox();
    } else if (key == "\177") {  // ~\177 is backspace
      deleteLastKeyFromUserSearchString();
    } else if (key >= "\40" && key <= "\176") {  // between \40 and \176 are all number/letter/common symbol
      addToUserSearchString(key);
    }
  } else {
    if (key >= "\40" && key<= "\176") {  // between \40 and \176 are all number/letter/common symbol
      activateSearchBox();
      addToUserSearchString(key);
    }
  }
  BumpTopApp::singleton()->markGlobalStateAsChanged();
}

void SearchBox::findMatch(VisualPhysicsActorList room_actors) {
  std::pair<int, VisualPhysicsActorList > matches = matchingActors(room_actors);
  int number_of_matches = matches.first;
  VisualPhysicsActorList matching_actors = matches.second;

  for_each(VisualPhysicsActor* actor, room_actors) {
    if (!matching_actors.contains(actor)) {
      actor->set_selected(false);
    }
  }

  QHash<VisualPhysicsActorId, BumpPose> desired_poses;

  for_each(VisualPhysicsActor* actor, matching_actors) {
    if (!actor->room_surface()->is_pinnable_receiver()) {
      Ogre::Vector3 desired_position = actor->position();
      if (ProAuthorization::singleton()->find_as_you_type_y_displacement() + actor->size().y/2 > desired_position.y) {
        desired_position.y = ProAuthorization::singleton()->find_as_you_type_y_displacement() + actor->size().y/2;
      }
      BumpPose desired_pose = BumpPose(desired_position, actor->orientation());
      desired_poses.insert(actor->unique_id(), desired_pose);
    }
    actor->set_selected(ProAuthorization::singleton()->find_as_you_type_selected_value());
  }

  // TODO: this _could_ get a bit slow with lots of items
  QHash<VisualPhysicsActorId, BumpPose> constrained_poses = getActorPosesConstrainedToRoom(desired_poses, room_);
  constrained_poses = getActorPosesConstrainedToNoIntersections(constrained_poses, room_);

  for_each(VisualPhysicsActorId actor_id, constrained_poses.keys()) {
    VisualPhysicsActor* actor = room_->actor_with_unique_id(actor_id);
    if (!actor->is_new_items_pile()) {
      AnimationManager::singleton()->endAnimationsForActor(actor, AnimationManager::STOP_AT_CURRENT_STATE);
      VisualPhysicsActorAnimation* actor_animation;
      actor_animation = new VisualPhysicsActorAnimation(actor, 300,
                                                        constrained_poses[actor->unique_id()].position,
                                                        constrained_poses[actor->unique_id()].orientation);
      actor_animation->start();
    }
  }

  QString material_name = AppSettings::singleton()->global_material_name(HIGHLIGHT);
  AnimationManager::singleton()->endAnimationsForMaterial(material_name);
  Ogre::MaterialPtr highlight_material = Ogre::MaterialPtr(Ogre::MaterialManager::getSingleton().getByName(utf8(material_name)));  // NOLINT
  highlight_material->setAmbient(ProAuthorization::singleton()->find_as_you_type_r_tint(),
                                 ProAuthorization::singleton()->find_as_you_type_g_tint(),
                                 ProAuthorization::singleton()->find_as_you_type_b_tint());

  MaterialBlendAnimation* material_animation;
  material_animation = new MaterialBlendAnimation(material_name,
                                                  1, 1, 1, 3000);
  material_animation->start();

  overlay_text_.setNum(number_of_matches);
  overlay_text_ += QString(" file%1 found containing: \"").arg(number_of_matches > 1 ? "s" : "") +
                   user_search_string_ + "\"";
}

std::pair<int, VisualPhysicsActorList > SearchBox::matchingActors(VisualPhysicsActorList room_actors) {
  int number_of_matches = 0;
  VisualPhysicsActorList matching_actors;
  for_each(VisualPhysicsActor* actor, room_actors) {
    int matches_found_in_object = numberOfMatchesFoundInActor(actor);
    if (matches_found_in_object > 0) {
      matching_actors.append(actor);
    }
    number_of_matches += matches_found_in_object;
  }
  return std::pair<int, VisualPhysicsActorList >(number_of_matches, matching_actors);
}

int SearchBox::numberOfMatchesFoundInActor(VisualPhysicsActor* actor) {
  int number_of_matches = 0;
  if (QFileInfo(actor->path()).fileName().contains(user_search_string_, Qt::CaseInsensitive)) {
    number_of_matches++;
  } else if (user_search_string_ != "" && actor->display_name().contains(user_search_string_, Qt::CaseInsensitive)) {
    number_of_matches++;
  }
  for_each(QString path, actor->pathsOfDescendants()) {
    if (QFileInfo(path).fileName().contains(user_search_string_, Qt::CaseInsensitive)) {
      number_of_matches++;
    }
  }
  return number_of_matches;
}

void SearchBox::updateSearchBox() {
  if (user_search_string_ != "") {
    overlay_->set_search_box_text(overlay_text_);
    overlay_->updateSearchBoxOverlay();
    updateSearchBoxPosition();
    timer_.start(1000);
  }
}

void SearchBox::updateSearchBoxPosition() {
  if (overlay_ != NULL) {
    Ogre::Vector3 center = room_->center_of_floor();
    int overlay_x;
    int overlay_z;
    overlay_x = center.x - overlay_->width_of_search_box()/2;
    if (AppSettings::singleton()->has_auto_hide_dock()) {
      overlay_z = center.z * 1.62;
    } else {
      overlay_z = center.z * 1.58;
    }
    overlay_->set_position(Ogre::Vector2(overlay_x, overlay_z));
  }
}

void SearchBox::addToUserSearchString(QString key) {
  user_search_string_ += key;
  findMatch(room_->room_actor_list());
  updateSearchBox();
}

void SearchBox::deleteLastKeyFromUserSearchString() {
  user_search_string_.remove(user_search_string_.length() - 1, 1);
  if (user_search_string_ == "") {
    for_each(VisualPhysicsActor* actor, room_->room_actor_list())
    actor->set_selected(false);
    deactivateSearchBox();
  } else {
    findMatch(room_->room_actor_list());
    updateSearchBox();
  }
}

void SearchBox::startFadingAnimation() {
  if (is_open_) {
    user_search_string_ = "";
    is_open_ = false;
    emit onSearchBoxDismissed();
    overlay_->fade(1000);
  }
}

void SearchBox::activateSearchBox() {
  is_open_ = true;
  emit onSearchBoxActivated();
  overlay_->endFade();
  overlay_->showSearchBoxOverlay();
}

void SearchBox::deactivateSearchBox() {
  is_open_ = false;
  emit onSearchBoxDismissed();
  overlay_->hideSearchBoxOverlay();
}

bool SearchBox::is_open() {
  return is_open_;
}

void SearchBox::hideOverlayAfterFading() {
  overlay_->hideSearchBoxOverlay();
}

#include "moc/moc_SearchBox.cpp"

