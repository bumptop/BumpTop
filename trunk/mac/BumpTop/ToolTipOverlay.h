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

#ifndef BUMPTOP_TOOLTIPOVERLAY_H_
#define BUMPTOP_TOOLTIPOVERLAY_H_

#include "BumpTop/AlphaElementAnimation.h"
#include "BumpTop/VisualPhysicsActorId.h"
#include "BumpTop/Clickable.h"

enum ToolTipPointingDirection {
  TOOLTIP_UP,
  TOOLTIP_DOWN,
  TOOLTIP_LEFT,
  TOOLTIP_RIGHT
};

class QPainterMaterial;
class VisualPhysicsActor;

class ToolTipOverlay : public ClickableOverlay, public AlphaElement {
  Q_OBJECT
 public:
  explicit ToolTipOverlay();
  virtual ~ToolTipOverlay();
  void initWithTextLines(QStringList text_lines, ToolTipPointingDirection dir);
  void createTipText(QStringList text_lines);
  void setSize(Ogre::Vector2 size);
  virtual void setAlpha(Ogre::Real alpha);
  void setCenter(Ogre::Vector2 position);
  void fadeOut(Ogre::Real final_alpha = 0.0);
  void fadeIn(int delay);
  Ogre::Vector2 size();
  void followActor(VisualPhysicsActor* actor);
  void fadeOutAndDeleteSelfOnCompletion();

 public slots:  // NOLINT
  virtual void drawText(QPainter* painter);
  virtual void fadeOutComplete();
  virtual void actorToFollowPoseChanged(VisualPhysicsActorId actor_id);
  virtual void actorToFollowRemoved(VisualPhysicsActorId actor_id);

 protected:
  void initTextMaterial();
  QString backgroundForDirection(ToolTipPointingDirection dir);

  bool faded_;
  QStringList text_lines_list_;
  QPainterMaterial *text_material_;
  Ogre::Overlay* overlay_;
  QString background_material_name_;
  QString text_lines_material_name_;
  Ogre::OverlayContainer* panel_;
  Ogre::OverlayContainer* background_;
  Ogre::OverlayContainer* text_lines_;
  VisualPhysicsActor* actor_to_follow_;
  QSize text_size_;
  QFont font_;
  bool delete_self_on_next_fadeout_complete_;
};

#endif  // BUMPTOP_TOOLTIPOVERLAY_H_
