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

#ifndef BUMPTOP_SEARCHBOXOVERLAY_H_
#define BUMPTOP_SEARCHBOXOVERLAY_H_

#include <Ogre.h>
#include <QtCore/QObject>
#include <QtCore/QRect>
#include <QtCore/QString>

#include "BumpTop/QPainterOverlay.h"
#include "BumpTop/AlphaElementAnimation.h"

class QPainterMaterial;

class SearchBoxOverlay : public QPainterOverlay {
  Q_OBJECT
 public:
  explicit SearchBoxOverlay(QString display_text);
  ~SearchBoxOverlay();
  virtual void init();

  // subset of the QT image that was actually drawn on
  size_t width_of_search_box();
  size_t height_of_search_box();

  void updateSearchBoxOverlay();
  void hideSearchBoxOverlay();
  void showSearchBoxOverlay();
  Ogre::AxisAlignedBox boundingBox();
  void set_search_box_text(QString search_box_text);

 public slots: // NOLINT
  virtual void draw(QPainter* painter);

 protected:
  void updateDrawnRegion();

  QFont search_box_font_;
  QString search_box_text_;
  QRect search_box_text_rect_;
  QRect search_box_highlight_around_text_rect_;
  static QImage dummy_image;
  AlphaElementAnimation overlay_alpha_animation_;
};

#endif  // BUMPTOP_SEARCHBOXOVERLAY_H_
