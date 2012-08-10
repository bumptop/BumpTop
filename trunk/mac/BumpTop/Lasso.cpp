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

#include "BumpTop/Lasso.h"

#include <algorithm>
#include <vector>

#include "BumpTop/BumpBox.h"
#include "BumpTop/BumpTopApp.h"
#include "BumpTop/Math.h"
#include "BumpTop/MouseEventManager.h"
#include "BumpTop/OgreHelpers.h"
#include "BumpTop/QPainterMaterial.h"
#include "BumpTop/QStringHelpers.h"
#include "BumpTop/Room.h"
#include "BumpTop/VisualPhysicsActorList.h"

Lasso::Lasso(BumpTopApp* app, Room* room, bool is_rectangle, VisualPhysicsActorList previously_selected_actors)
: QPainterOverlay(),
  app_(app),
  room_(room),
  is_rectangle_(is_rectangle),
  previously_selected_actors_(previously_selected_actors) {
}

Lasso::Lasso() {
}

void Lasso::init() {
  QPainterOverlay::init();
  panel_ = static_cast<Ogre::PanelOverlayElement*>(Ogre::OverlayManager::getSingleton().createOverlayElement("Panel",  // NOLINT
                                                                                                             "LassoOverlayPanel" +  // NOLINT
                                                                                                             addressToString(this)));  // NOLINT
  panel_->setMetricsMode(Ogre::GMM_PIXELS);
  QPainterOverlay::initOverlay("LassoOverlay" + addressToString(this));
  overlay_->add2D(panel_);
  // Show the overlay
  overlay_->show();
  updateLassoMaterial();
}

Lasso::~Lasso() {
  if (material_ != NULL) {
    material_->disconnect(this);
    Ogre::OverlayManager::getSingleton().destroyOverlayElement("LassoOverlayPanel" + addressToString(this));
  }
}

bool Lasso::isPointInPolygon(Ogre::Vector2 point) {
  int n_vertices = lasso_polygon_.size();
  bool in_polygon = false;
  double slope, intersect_y;
  Ogre::Vector2 v0, v1;

  // A polygon is defined by 3 or more vertices
  if (n_vertices < 3) {
    return false;
  }

  int i = 0;

  // For each pair of adjacent vertices (j,i) in the polygon
  for (int j = 0; j < n_vertices; j++) {
    i = (j + 1) % n_vertices;

    v0 = lasso_polygon_[j];
    v1 = lasso_polygon_[i];

    // This block checks that the point lies below the line segment joingin v0 and v1
    // (in the sense of lower y value)
    if ((point.x >= v1.x && point.x < v0.x) ||
        (point.x >= v0.x && point.x < v1.x)) {
      // Point lies between the two vertices in the x dimension
      // Check now to see if the point lies below the line

      // Simplified algebra that finds the y-value of the line connecting v0 and v1
      // at the x-value of the point in interest
      slope = (v1.y - v0.y)/(v1.x - v0.x);
      intersect_y = slope*(point.x - v0.x) + v0.y;
      if (point.y < intersect_y) {
        in_polygon = !in_polygon;
      }
    }
  }
  return in_polygon;
}

void Lasso::addVertexToLasso(Ogre::Vector2 vertex) {
  lasso_polygon_.push_back(vertex);
  lasso_polygon_qpoints_.push_back(QPointF(vertex.x, vertex.y));

  if (lasso_polygon_.size() == 1) {
    lasso_bounding_box_ = Ogre::AxisAlignedBox(vertex.x, vertex.y, -1, vertex.x, vertex.y, 1);
  } else if (lasso_polygon_.size() > 1) {
  // Maintain the lasso's bounding box by updating its extremities if vertex being added extends the box
    lasso_bounding_box_ = addPointToBoundingBox(lasso_bounding_box_, Ogre::Vector3(vertex.x, vertex.y, 0));
  }
}

void Lasso::resizeRectangle(Ogre::Vector2 vertex) {
  lasso_polygon_[2] = vertex;
  lasso_polygon_[1] = Ogre::Vector2(lasso_polygon_[2].x,lasso_polygon_[0].y);
  lasso_polygon_[3] = Ogre::Vector2(lasso_polygon_[0].x,lasso_polygon_[2].y);
  
  rectangle_ = QRectF(QPointF(MIN(lasso_polygon_[0].x,lasso_polygon_[2].x),
                              MIN(lasso_polygon_[0].y,lasso_polygon_[2].y)),
                      QPointF(MAX(lasso_polygon_[0].x,lasso_polygon_[2].x),
                              MAX(lasso_polygon_[0].y,lasso_polygon_[2].y)));
  
  lasso_bounding_box_ = addPointToBoundingBox(lasso_bounding_box_, Ogre::Vector3(vertex.x, vertex.y, 0));
}

void Lasso::mouseDown(MouseEvent* mouse_event) {
  if (is_rectangle_) {
    addVertexToLasso(mouse_event->mouse_in_window_space);
    addVertexToLasso(mouse_event->mouse_in_window_space);
    addVertexToLasso(mouse_event->mouse_in_window_space);
    addVertexToLasso(mouse_event->mouse_in_window_space);
  } else {
    addVertexToLasso(mouse_event->mouse_in_window_space);
  }
  updateLassoMaterialSizeAndPosition(true);
  mouse_event->handled = true;
}

void Lasso::updateLassoMaterial() {
  Ogre::Vector3 lasso_size_ = lasso_bounding_box_.getSize();
  lasso_size_.x = std::max(lasso_size_.x, (Ogre::Real)256);
  lasso_size_.y = std::max(lasso_size_.y, (Ogre::Real)256);

  QPainterOverlay::initMaterial(lasso_size_.x, lasso_size_.y);
}

void Lasso::updateLassoMaterialSizeAndPosition(bool force_texture_creation) {
  // offsets are for the thickness of the lines at the border of the lasso
  Ogre::Vector3 lasso_min = lasso_bounding_box_.getMinimum();
  lasso_material_left_offset_ = std::min(lasso_min.x, (Ogre::Real)4);
  lasso_material_top_offset_ = std::min(lasso_min.y, (Ogre::Real)4);

  Ogre::Vector3 lasso_size_ = lasso_bounding_box_.getSize();
  int lasso_material_right_offset = 4;  // std::min(monitor_size - lasso_size.x, 4);
  int lasso_material_bottom_offset = 4;  // std::min(monitor_size - lasso_size.y, 4);

  lasso_size_.x += lasso_material_left_offset_ + lasso_material_right_offset;
  lasso_size_.y += lasso_material_top_offset_ + lasso_material_bottom_offset;

  panel_ = static_cast<Ogre::PanelOverlayElement*>(Ogre::OverlayManager::getSingleton().getOverlayElement("LassoOverlayPanel" +  // NOLINT
                                                                                                          addressToString(this)));  // NOLINT

  if (force_texture_creation ||
      lasso_size_.x > material_->width() ||
      lasso_size_.y > material_->height()) {
    updateLassoMaterial();
    panel_->setDimensions(material_->width(), material_->height());
    // Ogre 1.6.3 has an optimization that if the material name is the same, it doesn't update it; in this case,
    // we might have a new material created but with exactly the same name, and therefore have a problem
    // updating. Instead, first set the material to something else (empty string) and then back to the
    // real material name
    panel_->setMaterialName("");
    panel_->setMaterialName(utf8(material_->name()));
  }

  QPainterOverlay::set_position(Ogre::Vector2(lasso_min.x - lasso_material_left_offset_, lasso_min.y - lasso_material_top_offset_));  // NOLINT
}

void Lasso::mouseDragged(MouseEvent* mouse_event) {
  if (is_rectangle_) {
    resizeRectangle(mouse_event->mouse_in_window_space);
  } else {
    addVertexToLasso(mouse_event->mouse_in_window_space);
  }
  updateLassoMaterialSizeAndPosition(false);
  findBoxesInPolygon();
  material_->update();
  mouse_event->handled = true;
}

void Lasso::draw(QPainter* painter) {
  QBrush fill_brush(QColor(190, 190, 240, 70));
  painter->setBrush(fill_brush);

  QPen pen;  // creates a default pen

  pen.setStyle(Qt::SolidLine);
  pen.setWidth(1);
  pen.setBrush(QBrush(QColor(255, 255, 255, 100)));
  pen.setCapStyle(Qt::RoundCap);
  pen.setJoinStyle(Qt::RoundJoin);

  painter->setPen(pen);

  painter->setRenderHint(QPainter::Antialiasing, true);

  Ogre::Vector3 lasso_min_ = lasso_bounding_box_.getMinimum();
  painter->translate(-lasso_min_.x + lasso_material_left_offset_,
                     -lasso_min_.y + lasso_material_top_offset_);
  if (is_rectangle_) {
    painter->drawRect(rectangle_);
  } else {
    painter->drawPolygon(&lasso_polygon_qpoints_[0], lasso_polygon_qpoints_.size());
  }
}

// http://local.wasp.uwa.edu.au/~pbourke/geometry/lineline2d/
bool doLineSegmentsCross(Ogre::Vector2 p1, Ogre::Vector2 p2,
                         Ogre::Vector2 p3, Ogre::Vector2 p4) {
  Ogre::Real u_a = (p4.x - p3.x)*(p1.y - p3.y) - (p4.y - p3.y)*(p1.x - p3.x);
  Ogre::Real u_a_den = (p4.y - p3.y)*(p2.x - p1.x) - (p4.x - p3.x)*(p2.y - p1.y);
  if (u_a_den == 0)
    return false;
  u_a /= u_a_den;
  if (u_a < 0 || u_a > 1)
    return false;

  Ogre::Real u_b = (p2.x - p1.x)*(p1.y - p3.y) - (p2.y - p1.y)*(p1.x - p3.x);
  Ogre::Real u_b_den = (p4.y - p3.y)*(p2.x - p1.x) - (p4.x - p3.x)*(p2.y - p1.y);
  u_b /= u_b_den;
  if (u_b < 0 || u_b > 1)
    return false;

  return true;
}

bool Lasso::doesLineSegmentCrossLastPolygonSegment(Ogre::Vector2 p1, Ogre::Vector2 p2) {
  int n_vertices = lasso_polygon_.size();
  if (n_vertices >= 2) {
    return doLineSegmentsCross(p1, p2, lasso_polygon_[n_vertices - 1], lasso_polygon_[n_vertices - 2]);
  }
  return false;
}

void Lasso::findBoxesInPolygon() {
  for_each(VisualPhysicsActor* actor, room_->room_actor_list()) {
    Ogre::Vector2 screen_position = actor->getScreenPosition();
    // Check if 8 corners are in bounding box (low cost)
    //  and if a corner's in the bounding box check if it's in the lasso's polygon
    std::vector<Ogre::Vector2> corners = actor->getCornersInScreenSpace();
    std::swap(corners[4], corners[6]);
    bool selected = actors_intersecting_polygon_border_.contains(actor);
    if (!selected) {
      for_each(Ogre::Vector2 corner, corners) {
        if (lasso_bounding_box_.contains(Ogre::Vector3(corner.x, corner.y, 0)) &&
            isPointInPolygon(corner)) {
          selected = true;
          break;
        }
      }
    }
    if (!selected) {
      /* corners look like this: (after swapping 4 and 6 earlier)
         1-----2
        /|    /|
       / |   / |
      5-----6  |
      |  0--|--3
      | /   | /
      |/    |/
      4-----7
      */
      for (int i = 0; i < 4; i++) {
        if (selected)
          break;
        for (int j = 0; j < 3; j++) {
          Ogre::Vector2 corner;
          Ogre::Vector2 next_corner;
          switch (j) {
            case 0:
              corner = corners[i];
              next_corner = corners[(i + 1) % 4];
              break;
            case 1:
              corner = corners[i];
              next_corner = corners[i + 4];
              break;
            case 2:
              corner = corners[i + 4];
              next_corner = corners[(i + 1) % 4 + 4];
          }

          Ogre::AxisAlignedBox line_segment_box(std::min(corner.x, next_corner.x),
                                                std::min(corner.y, next_corner.y),
                                                -1,
                                                std::max(corner.x, next_corner.x),
                                                std::max(corner.y, next_corner.y),
                                                1);
          if (lasso_bounding_box_.intersects(line_segment_box) &&
              doesLineSegmentCrossLastPolygonSegment(corner, next_corner)) {
            actors_intersecting_polygon_border_.insert(actor);
            selected = true;
            break;
          }
        }
      }
    }
    actor->set_selected(selected != previously_selected_actors_.contains(actor));
  }
}

#include "moc/moc_Lasso.cpp"
