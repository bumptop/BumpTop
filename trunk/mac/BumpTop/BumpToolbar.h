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

#ifndef BUMPTOP_BUMPTOOLBAR_H_
#define BUMPTOP_BUMPTOOLBAR_H_

#include <string>

#include "BumpTop/AlphaElementAnimation.h"
#include "BumpTop/Clickable.h"

class CommandButton;
class Room;
class Timer;

class BumpToolbar : public ClickableOverlay, public AlphaElement {
  Q_OBJECT
 public:
  explicit BumpToolbar(Room* room);
  ~BumpToolbar();

  void init();

  virtual void mouseDown(MouseEvent* mouse_event);
  virtual void mouseDragged(MouseEvent* mouse_event);
  virtual void mouseUp(MouseEvent* mouse_event);

  virtual void updateForActorList(VisualPhysicsActorList actors);
  virtual void setPositionForActorList(VisualPhysicsActorList actors);

  virtual bool set_center(Ogre::Vector2 position, bool fail_on_x_coord_off_screen = true);
  virtual void setSize(Ogre::Vector2 size);

  virtual Ogre::OverlayContainer* overlay_container();
  virtual void setAlpha(Ogre::Real alpha);

  virtual void show(int delay = 250);
  virtual void hide();
  virtual void fadeOut(Ogre::Real final_alpha = 0.0);
  virtual void fadeIn(int delay = 250);
  virtual bool faded();

  virtual void lassoComplete(VisualPhysicsActorList actors);

 public slots:  // NOLINT
  virtual void fadeOutComplete();
  virtual void renderTick();
  virtual void fadeInTimerComplete(Timer* timer);

  virtual bool visible();
 protected:
  virtual Ogre::Vector2 center();
  virtual void clearCommandButtons();

  QList<CommandButton*> command_buttons_;
  std::string top_material_name_, middle_material_name_, bottom_material_name_;
  bool hidden_;
  bool faded_;

  Room* room_;
  Ogre::Vector2 center_;
  Ogre::Vector2 mouse_offset_;
  Timer* fade_in_timer_;

  Ogre::AxisAlignedBox actors_bounding_box_;
  Ogre::AxisAlignedBox actors_and_toolbar_screen_bounding_box_;
  Ogre::AxisAlignedBox after_lasso_bounding_box_;
  bool use_large_bounding_box_after_lasso_;

  Ogre::OverlayContainer* panel_;
  Ogre::OverlayContainer* top_;
  Ogre::OverlayContainer* middle_;
  Ogre::OverlayContainer* bottom_;

  Ogre::Overlay* overlay_;
};

#endif  // BUMPTOP_BUMPTOOLBAR_H_
