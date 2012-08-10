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

#include "BumpTop/QPainterOverlay.h"

#include <QtGui/QPainter>
#include <string>

#include "BumpTop/AnimationManager.h"
#include "BumpTop/QPainterMaterial.h"

QPainterOverlay::QPainterOverlay()
: ClickableOverlay(),
  material_(NULL) {
}

QPainterOverlay::~QPainterOverlay() {
  if (material_ != NULL) {
    delete material_;
    Ogre::OverlayManager::getSingleton().destroy(overlay_name_);
  }
}

void QPainterOverlay::init() {
}

void QPainterOverlay::initMaterial(int material_width, int material_height) {
  if (material_ != NULL) {
    delete material_;
    material_ = NULL;
  }

  material_ = new QPainterMaterial();
  material_->initWithSize(material_width, material_height);
  assert(QObject::connect(material_, SIGNAL(draw(QPainter*)),  // NOLINT
                          this, SLOT(draw(QPainter*))));  // NOLINT
  material_->update();
  setAlpha(1);
}

void QPainterOverlay::initOverlay(std::string overlay_name) {
  overlay_name_ = overlay_name;
  overlay_ = Ogre::OverlayManager::getSingleton().create(overlay_name_);
}

Ogre::AxisAlignedBox QPainterOverlay::boundingBox() {
  return Ogre::AxisAlignedBox(position_.x - width()/2.0, position_.y - height()/2.0, -1,
                              position_.x + width()/2.0, position_.y + height()/2.0, 1);
}

void QPainterOverlay::draw(QPainter* painter) {
}

void QPainterOverlay::fade(int milliseconds) {
  AlphaElementAnimation* overlay_animation;
  overlay_animation = new AlphaElementAnimation(this, 0, milliseconds);
  overlay_animation->start();
  assert(QObject::connect(overlay_animation, SIGNAL(onAnimationComplete()),  // NOLINT
                          this, SLOT(fadeCompleted())));  // NOLINT
}

void QPainterOverlay::endFade() {
  AnimationManager::singleton()->endAnimationsForAlphaElement(this);
}

void QPainterOverlay::fadeCompleted() {
  emit onFadeComplete();
}

void QPainterOverlay::setAlpha(Ogre::Real alpha) {
  AlphaElement::setAlpha(alpha);
  material_->setAlpha(alpha);
}

void QPainterOverlay::set_position(Ogre::Vector2 position) {
  position_ = position;
  panel_->setPosition(position_.x, position_.y);
}

size_t QPainterOverlay::width_of_drawn_region() {
  return material_->width_of_drawn_region();
}

size_t QPainterOverlay::height_of_drawn_region() {
  return material_->height_of_drawn_region();
}

size_t QPainterOverlay::width() {
  return material_->width();
}

size_t QPainterOverlay::height() {
  return material_->height();
}

#include "BumpTop/moc/moc_QPainterOverlay.cpp"

