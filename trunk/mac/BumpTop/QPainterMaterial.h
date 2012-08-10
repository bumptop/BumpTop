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

#ifndef BUMPTOP_QPAINTERMATERIAL_H_
#define BUMPTOP_QPAINTERMATERIAL_H_

#include <Ogre.h>
#include <QtCore/QObject>
#include <QtGui/QPainter>

class QPainterMaterial: public QObject {
  Q_OBJECT

 public:
  QPainterMaterial();
  virtual ~QPainterMaterial();

  void initWithSize(size_t width, size_t height, int num_mip_maps = 0, bool use_texture_filtering = false);
  void update();

  size_t width_of_drawn_region();
  size_t height_of_drawn_region();
  size_t width();
  size_t height();

  QString name();
  void setAlpha(Ogre::Real alpha);

 signals:
  void draw(QPainter* qpainter);
 protected:
  Ogre::MaterialPtr material_;
  Ogre::TexturePtr texture_;
  uchar *image_buffer_;
  QImage *qimage_;
  QPainter *qpainter_;
  Ogre::PixelBox *pixelbox_;
  size_t width_of_drawn_region_, height_of_drawn_region_;
  int num_mip_maps_;
};

#endif  // BUMPTOP_QPAINTERMATERIAL_H_
