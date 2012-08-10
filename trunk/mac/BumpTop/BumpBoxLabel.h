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

#ifndef BUMPTOP_BUMPBOXLABEL_H_
#define BUMPTOP_BUMPBOXLABEL_H_

#include <Ogre.h>
#include <QtCore/QObject>
#include <QtCore/QRect>
#include <QtCore/QString>

#include "BumpTop/BumpBox.h"
#include "BumpTop/Clickable.h"
#include "BumpTop/Singleton.h"
#include "BumpTop/VisualActor.h"

class BumpBoxLabel;
class QPainterMaterial;
class VisualActor;
class VisualPhysicsActor;

class BumpBoxLabelManager {
  SINGLETON_HEADER(BumpBoxLabelManager)
 public:
  void addLabel(BumpBoxLabel* label);
  void removeLabel(BumpBoxLabel* label);
  const QSet<BumpBoxLabel*>& labels();
 protected:
  QSet<BumpBoxLabel*> labels_;
};

class BumpBoxLabel : public Clickable, public Ogre::Renderable::Visitor, public RenderOrder {
  Q_OBJECT
 public:
  enum TextAlignment {
    AlignLeft,
    AlignCenter,
    AlignRight
  };
  explicit BumpBoxLabel(QString label, VisualPhysicsActor* associated_actor);
  ~BumpBoxLabel();
  virtual void init(Ogre::Real size_factor = 1);

  virtual void initMaterial(int material_width, int material_height);
  virtual void set_selected(bool is_selected);
  virtual bool selected();

  virtual void set_position_in_pixel_coords(Ogre::Vector2 position);
  virtual Ogre::Vector2 position_in_pixel_coords();
  virtual Ogre::Vector3 position();
  virtual Ogre::Plane plane();

  virtual BumpBoxLabelColour label_colour();
  virtual void set_label_colour(BumpBoxLabelColour label_colour);

  virtual size_t width_of_drawn_region();
  virtual size_t width();
  virtual size_t height();

  Ogre::Renderable* renderable();
  Ogre::MovableObject* movable_object();
  virtual void set_render_queue_group(uint8 queue_id);
  virtual void visit(Ogre::Renderable *renderable, ushort lod_index, bool is_debug, Ogre::Any *any);
  virtual void set_visible(bool visible);
  virtual bool visible();
  virtual Ogre::Entity* _entity();
  virtual Ogre::Real boundingWidth();

  QString text_;
 public slots: // NOLINT
  virtual void draw(QPainter* painter);
  virtual void updateScale();
 protected:
  virtual QImage createBlurredText();
  QSize getTextBounds(QStringList *linesOut, QList<QSize> *lineSizesOut, int leading, int max_width);

  QFont font_;

  QSize text_size_;
  QStringList text_lines_;
  QList<QSize> text_line_sizes_;
  QRect highlight_around_text_rect_;
  static QImage dummy_image;
  bool is_selected_;
  bool visible_;
  QPainterMaterial *material_;
  Ogre::SceneNode *node_;
  Ogre::ManualObject *manual_;
  Ogre::Renderable *renderable_;
  TextAlignment text_alignment_;
  VisualPhysicsActor* associated_actor_;

  BumpBoxLabelColour label_colour_;

  bool truncated_;
  bool truncate_to_single_line_;
};

#endif  // BUMPTOP_BUMPBOXLABEL_H_
