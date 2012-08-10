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

#ifndef BUMPTOP_QPAINTEROVERLAY_H_
#define BUMPTOP_QPAINTEROVERLAY_H_

#include <Ogre.h>
#include <QtCore/QObject>
#include <string>

#include "BumpTop/Clickable.h"
#include "BumpTop/AlphaElementAnimation.h"

class QPainterMaterial;

class QPainterOverlay : public ClickableOverlay, public AlphaElement {
  Q_OBJECT

 public:
  explicit QPainterOverlay();

  virtual ~QPainterOverlay();
  virtual void init();
  virtual void initMaterial(int material_width, int material_height);
  virtual void initOverlay(std::string overlay_name);
  virtual void fade(int milliseconds);
  virtual void endFade();
  virtual void setAlpha(Ogre::Real alpha);
  virtual void set_position(Ogre::Vector2 position);
  virtual size_t width_of_drawn_region();
  virtual size_t height_of_drawn_region();
  virtual size_t width();
  virtual size_t height();
  virtual Ogre::AxisAlignedBox boundingBox();

 public slots:  // NOLINT
  virtual void draw(QPainter* painter);
  void fadeCompleted();

 signals:
  void onFadeComplete();

 protected:
  QPainterMaterial *material_;
  Ogre::Overlay *overlay_;
  std::string overlay_name_;
  AlphaElementAnimation overlay_alpha_animation_;
  Ogre::PanelOverlayElement* panel_;
  Ogre::Vector2 position_;
};

#endif  // BUMPTOP_QPAINTEROVERLAY_H_
