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

#ifndef BUMPTOP_LASSO_H_
#define BUMPTOP_LASSO_H_

#include <Ogre.h>
#include <QtGui/QPainter>
#include <vector>

#include "BumpTop/QPainterOverlay.h"

class BumpTopApp;
class Physics;
class QPainterMaterial;
class Room;
class VisualPhysicsActor;

class Lasso : public QPainterOverlay {
  Q_OBJECT

 public:
  explicit Lasso(BumpTopApp* app, Room* room, bool is_rectangle, VisualPhysicsActorList previously_selected_actors);
  explicit Lasso();
  virtual ~Lasso();

  virtual void init();
  bool isPointInPolygon(Ogre::Vector2 point);
  bool doesLineSegmentCrossLastPolygonSegment(Ogre::Vector2 p1, Ogre::Vector2 p2);
  void findBoxesInPolygon();
  void addVertexToLasso(Ogre::Vector2 vertex);
  void resizeRectangle(Ogre::Vector2 vertex);

  // Application variables
  BumpTopApp* app_;
  Room* room_;

  // Mouse event handlers
  void mouseDown(MouseEvent* mouse_event);
  void mouseDragged(MouseEvent* mouse_event);

 public slots:  // NOLINT
  virtual void draw(QPainter* painter);

 protected:
  // Polygon variables
  std::vector<Ogre::Vector2> lasso_polygon_;
  std::vector<QPointF> lasso_polygon_qpoints_;
  void updateLassoMaterial();

  Ogre::AxisAlignedBox lasso_bounding_box_;

  int lasso_material_left_offset_;
  int lasso_material_top_offset_;

  void updateLassoMaterialSizeAndPosition(bool force_texture_creation = false);
  QSet<VisualPhysicsActor*> actors_intersecting_polygon_border_;

  bool is_rectangle_;
  QRectF rectangle_;

  VisualPhysicsActorList previously_selected_actors_;
};

#endif  // BUMPTOP_LASSO_H_
