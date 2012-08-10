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

#include "BumpTop/StickyNotePad.h"
#include "BumpTop/AppSettings.h"
#include "BumpTop/Authorization.h"
#include "BumpTop/BumpTopApp.h"
#include "BumpTop/BumpTopScene.h"
#include "BumpTop/FileManager.h"
#include "BumpTop/MaterialLoader.h"
#include "BumpTop/ProtocolBufferHelpers.h"
#include "BumpTop/Room.h"
#include "BumpTop/RoomItemPoseConstraints.h"
#include "BumpTop/RoomSurface.h"

#include "BumpTop/StickyNote.h"

BumpTopCommandSet* StickyNotePad::sticky_note_pad_context_menu_items_set = MakeQSet(5,  // count, must keep this updated
                                                                                    CreatePile::singleton(),
                                                                                    Open::singleton(),
                                                                                    Grow::singleton(),
                                                                                    Shrink::singleton(),
                                                                                    GridView::singleton());

StickyNotePad::StickyNotePad(Ogre::SceneManager *scene_manager, Physics* physics,
                             Room *room, VisualPhysicsActorId unique_id)
: BumpFlatSquare(scene_manager, physics, room, unique_id) {
  StickyNoteCounter::singleton()->set_sticky_note_pad(this);
}

BumpTopCommandSet* StickyNotePad::supported_context_menu_items() {
  return sticky_note_pad_context_menu_items_set;
}

void StickyNotePad::init() {
  BumpFlatSquare::init();

  if (StickyNoteCounter::singleton()->count() >= 2 && !ProAuthorization::singleton()->authorized()) {
    set_material_name(AppSettings::singleton()->global_material_name(NO_STICKIES_LEFT));
  } else {
    set_material_name(AppSettings::singleton()->global_material_name(STICKY_NOTE_PAD_MATERIAL));
  }

  room_->addActor(this);
}

VisualPhysicsActorType StickyNotePad::actor_type() {
  return STICKY_NOTE_PAD;
}

StickyNotePad::~StickyNotePad() {
}

bool StickyNotePad::initFromBuffer(VisualPhysicsActorBuffer* buffer, bool physics_enabled) {
  StickyNotePad::init();

  set_position(Vector3BufferToVector3(buffer->position()));
  set_orientation(QuaternionBufferToQuaternion(buffer->orientation()));
  set_size(Vector3BufferToVector3(buffer->size()));

  if (buffer->has_room_surface()) {
    RoomSurface* surface = room_->getSurface((RoomSurfaceType)buffer->room_surface());
    set_room_surface(surface);
    if (surface->is_pinnable_receiver())
      pinToSurface(surface);
  }
  return true;
}

void StickyNotePad::launch() {
  if (StickyNoteCounter::singleton()->count() >= 2 && !ProAuthorization::singleton()->authorized()) {
    return;
  }

  StickyNote* sticky_note = new StickyNote(BumpTopApp::singleton()->ogre_scene_manager(),
                                           BumpTopApp::singleton()->physics(), room_);
  sticky_note->init();

  Ogre::Real size = ProAuthorization::singleton()->sticky_note_sizes(std::min(2, StickyNoteCounter::singleton()->count() - 1));  // NOLINT

  sticky_note->set_size(Ogre::Vector3(size, size, size));
  switch (BumpTopApp::singleton()->scene()->surface_that_camera_is_zoomed_to()) {
    case FLOOR:
      sticky_note->set_position(Ogre::Vector3((room_->min_x()+room_->max_x())/2.0, 0,
                                              (room_->min_z()+room_->max_z())/2.0));
      break;
    case FRONT_WALL:
      sticky_note->set_position(Ogre::Vector3((room_->min_x()+room_->max_x())/2.0, 0,
                                              room_->max_z()));
      break;
    case BACK_WALL:
      sticky_note->set_position(Ogre::Vector3((room_->min_x()+room_->max_x())/2.0, 0,
                                              room_->min_z()));
      break;
    case LEFT_WALL:
      sticky_note->set_position(Ogre::Vector3(room_->min_x(), 0,
                                              (room_->min_z()+room_->max_z())/2.0));
      break;
    case RIGHT_WALL:
      sticky_note->set_position(Ogre::Vector3(room_->max_x(), 0,
                                              (room_->min_z()+room_->max_z())/2.0));
      break;
    default:
      sticky_note->set_position(Ogre::Vector3((room_->min_x()+room_->max_x())/2.0, 0,
                                              (room_->min_z()+room_->max_z())/2.0));
      break;
  }
  sticky_note->set_pose(getActorPoseConstrainedToRoomAndNoIntersections(sticky_note, room_));
  room_->addActor(sticky_note);
  sticky_note->launch();
}

void StickyNotePad::stickyNoteCountChanged() {
  if (StickyNoteCounter::singleton()->count() == 2 && !ProAuthorization::singleton()->authorized()) {
    set_material_name(AppSettings::singleton()->global_material_name(NO_STICKIES_LEFT));
  } else if (StickyNoteCounter::singleton()->count() == 1 && !ProAuthorization::singleton()->authorized()) {
    set_material_name(AppSettings::singleton()->global_material_name(STICKY_NOTE_PAD_MATERIAL));
  }
}

bool StickyNotePad::nameable() {
  return false;
}

SINGLETON_IMPLEMENTATION(StickyNoteCounter)

StickyNoteCounter::StickyNoteCounter()
: sticky_note_pad_(NULL),
  sticky_note_count_(0) {
}

void StickyNoteCounter::incrementStickyNoteCount() {
  sticky_note_count_++;
  if (sticky_note_pad_ != NULL)
    sticky_note_pad_->stickyNoteCountChanged();
}

void StickyNoteCounter::decrementStickyNoteCount() {
  sticky_note_count_--;
  if (sticky_note_pad_ != NULL)
    sticky_note_pad_->stickyNoteCountChanged();
}

int StickyNoteCounter::count() {
  return sticky_note_count_;
}

void StickyNoteCounter::set_sticky_note_pad(StickyNotePad* pad) {
  sticky_note_pad_ = pad;
}

