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

#ifndef BUMPTOP_OVERLAYBUTTON_H_
#define BUMPTOP_OVERLAYBUTTON_H_

#include <string>

#include "BumpTop/Clickable.h"

class BumpToolbar;
class BumpTopToolbarCommand;
class Room;

class OverlayButton : public ClickableOverlay {
  Q_OBJECT
 public:
  explicit OverlayButton();
  ~OverlayButton();

  virtual void initWithMaterials(QString active_material_name, QString inactive_material_name);
  virtual void initWithImages(QString active_image, QString inactive_image);
  virtual Ogre::OverlayContainer* overlay_container();
  virtual void set_parent(BumpToolbar* parent);

  virtual void mouseDown(MouseEvent* mouse_event);
  virtual void mouseDragged(MouseEvent* mouse_event);
  virtual void mouseUp(MouseEvent* mouse_event);

  virtual void set_position(Ogre::Vector2 position);
  virtual void set_size(Ogre::Vector2 size);
  virtual void setAlpha(Ogre::Real alpha);

 signals:
  void performAction();

 protected:
  virtual void buttonAction();
  virtual void hide();
  virtual void show();

  Ogre::Real left();
  Ogre::Real top();
  bool hidden();

  BumpTopToolbarCommand* bump_command_;
  BumpToolbar* parent_;
  Ogre::OverlayContainer* panel_;
  std::string inactive_material_name_;
  std::string active_material_name_;
  Room* room_;
};

#endif  // BUMPTOP_OVERLAYBUTTON_H_
