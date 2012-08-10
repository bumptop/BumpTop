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

#ifndef BUMPTOP_STICKYNOTE_H_
#define BUMPTOP_STICKYNOTE_H_

#include <string>

#include "BumpTop/BumpFlatSquare.h"
#include "BumpTop/QPainterMaterial.h"
#include "BumpTop/VisualActor.h"

class StickyNoteText;
class VisualPhysicsActorBuffer;
class StickyNote : public BumpFlatSquare {
  Q_OBJECT
 public:
  explicit StickyNote(Ogre::SceneManager *scene_manager, Physics* physics, Room *room, VisualPhysicsActorId unique_id = 0);  // NOLINT
  virtual ~StickyNote();

  virtual void init();
  virtual bool initFromBuffer(VisualPhysicsActorBuffer* buffer, bool physics_enabled = false);
  virtual void initWithPath(QString path, bool physics_enabled = true);

  std::string meshName();
  Ogre::Vector3 absoluteMeshSizeDividedBy100();
  btVector3 physicsSize();
  bool pinnable();
  void set_render_queue_group(uint8 queue_id);
  virtual void launch();
  StickyNote* constructCopyOfMyType();
  virtual void initAsVisualCopyOfActor(VisualPhysicsActor* actor);
  virtual VisualPhysicsActorType actor_type();
  virtual void save();
  virtual QString text();
  virtual BumpTopCommandSet* supported_context_menu_items();
  virtual void set_alpha(Ogre::Real alpha);
  virtual bool nameable();
  virtual void set_path(QString path);

  VisualPhysicsActor* visual_copy_of_actor_;
  virtual Ogre::AxisAlignedBox screenBoundingBox();
 public slots:  // NOLINT
  void editableTextChanged();
  void beginEditingAnimationFinished(VisualPhysicsActorAnimation* animation);
  void closeEditableStickyNote(MouseEvent* mouse_event);
  void finishEditingAnimationFinished(VisualPhysicsActorAnimation* animation);

 protected:
  void loadTextFromPath();
  static BumpTopCommandSet* sticky_note_context_menu_items_set;

  StickyNoteText* text_;
  BumpPose pose_before_editing_;
  bool is_dummy_;
};

class StickyNoteText : public VisualActor {
  Q_OBJECT
 public:
  StickyNoteText(Ogre::SceneManager *scene_manager, Ogre::SceneNode *parent_ogre_scene_node);
  virtual ~StickyNoteText();

  void init();
  void setFontSizeBasedOnAmountOfText();

  void set_text(QString text);
  QTextEdit* text_edit();
  QString text();
  void update();

 public slots:  // NOLINT
  void drawText(QPainter* painter);
 protected:

  QString text_;
  QPainterMaterial *material_;
  QTextEdit* sticky_note_text_edit_;
};

#endif  // BUMPTOP_STICKYNOTE_H_
