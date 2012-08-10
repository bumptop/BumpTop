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

#ifndef BUMPTOP_STICKYNOTEPAD_H_
#define BUMPTOP_STICKYNOTEPAD_H_

#include "BumpTop/BumpFlatSquare.h"

class StickyNotePad : public BumpFlatSquare {
 public:
  StickyNotePad(Ogre::SceneManager *scene_manager, Physics* physics,
                Room *room, VisualPhysicsActorId unique_id = 0);
  virtual ~StickyNotePad();

  void init();
  virtual bool initFromBuffer(VisualPhysicsActorBuffer* buffer, bool physics_enabled);
  virtual void launch();
  virtual bool nameable();
  virtual void stickyNoteCountChanged();
  virtual VisualPhysicsActorType actor_type();
  virtual BumpTopCommandSet* supported_context_menu_items();

 protected:
  static BumpTopCommandSet* sticky_note_pad_context_menu_items_set;
};

#include "BumpTop/Singleton.h"
class StickyNoteCounter {
  SINGLETON_HEADER(StickyNoteCounter)
 public:
  explicit StickyNoteCounter();
  void incrementStickyNoteCount();
  void decrementStickyNoteCount();
  int count();
  void set_sticky_note_pad(StickyNotePad* pad);
 protected:
  StickyNotePad* sticky_note_pad_;
  int sticky_note_count_;
};

#endif  // BUMPTOP_STICKYNOTEPAD_H_
