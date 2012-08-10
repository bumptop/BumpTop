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

#include "BumpTop/SearchBoxOverlay.h"

#include <QtGui/QPainter>

#include "BumpTop/Authorization.h"
#include "BumpTop/BumpTopApp.h"
#include "BumpTop/OgreHelpers.h"
#include "BumpTop/AlphaElementAnimation.h"
#include "BumpTop/QPainterMaterial.h"
#include "BumpTop/QStringHelpers.h"

const int kSearchBoxTextMarginHorizontal = -1;
const int kSearchBoxTextMarginRightAdditionalFudgeFactor = -20;
const int kSearchBoxTextMarginVertical = 2;
const int kSearchBoxPadding = 1;

QImage SearchBoxOverlay::dummy_image = QImage(1, 1, QImage::Format_ARGB32);

SearchBoxOverlay::SearchBoxOverlay(QString display_text)
: QPainterOverlay(),
  search_box_text_(QString("000  Files are Found Containing:  abcdefghijklmnopqrstuvwxyz")) {
  assert(display_text != " ");
  }

SearchBoxOverlay::~SearchBoxOverlay() {
  Ogre::OverlayManager::getSingleton().destroyOverlayElement("SearchBoxOverlayPanel" + addressToString(this));
}

void SearchBoxOverlay::init() {
  QPainterOverlay::init();

  // First, just find out how big the search box is
  search_box_font_ = QFont("Lucida Grande");
  search_box_font_.setBold(true);
  search_box_font_.setPointSize(30);

  updateDrawnRegion();

  // Create the search box material
  QPainterOverlay::initMaterial(ceil(static_cast<float>(kSearchBoxPadding +
                                        search_box_highlight_around_text_rect_.width())),
                ceil(static_cast<float>(kSearchBoxPadding +
                                        search_box_highlight_around_text_rect_.height())));
  Ogre::String address = addressToString(this);
  panel_ = static_cast<Ogre::PanelOverlayElement*> (Ogre::OverlayManager::getSingleton().createOverlayElement("Panel",
                                                                                                              "SearchBoxOverlayPanel" +  // NOLINT
                                                                                                              address));  // NOLINT
  panel_->setMetricsMode(Ogre::GMM_PIXELS);
  panel_->setPosition(0, 0);
  panel_->setDimensions(width(), height());
  panel_->setMaterialName(utf8(material_->name()));

  QPainterOverlay::initOverlay("SearchOverlay" + address);
  overlay_->add2D(panel_);
  overlay_->hide();
}

size_t SearchBoxOverlay::width_of_search_box() {
  return search_box_highlight_around_text_rect_.width();
}
size_t SearchBoxOverlay::height_of_search_box() {
  return search_box_highlight_around_text_rect_.height();
}

Ogre::AxisAlignedBox SearchBoxOverlay::boundingBox() {
  return Ogre::AxisAlignedBox(position_.x - width()/2.0, position_.y - height()/2.0, -1,
                              position_.x + width()/2.0, position_.y + height()/2.0, 1);
}

void SearchBoxOverlay::draw(QPainter* painter) {
  painter->setRenderHint(QPainter::Antialiasing, true);
  painter->setRenderHint(QPainter::TextAntialiasing, true);

  // draw the rounded rect around the text
  painter->translate(kSearchBoxPadding, kSearchBoxPadding);
  painter->setPen(Qt::NoPen);

  painter->setBrush(QBrush(QColor(80, 120, 250, 180)));
  painter->drawRoundedRect(search_box_highlight_around_text_rect_,
                           search_box_highlight_around_text_rect_.height()/4,
                           search_box_highlight_around_text_rect_.height()/4);

  painter->translate(search_box_highlight_around_text_rect_.height()/4 + kSearchBoxTextMarginHorizontal,
                     kSearchBoxTextMarginVertical);

  // draw the search box text
  painter->setFont(search_box_font_);
  painter->setPen(QPen(Qt::white));
  painter->drawText(search_box_text_rect_, search_box_text_);
}

void SearchBoxOverlay::updateSearchBoxOverlay() {
  updateDrawnRegion();
  material_->update();
}

void SearchBoxOverlay::set_search_box_text(QString search_box_text) {
  if (search_box_text != search_box_text_) {
    search_box_text_ = search_box_text;
  }
}

void SearchBoxOverlay::updateDrawnRegion() {
  QPainter temp_painter;
  temp_painter.begin(&dummy_image);
  temp_painter.setFont(search_box_font_);
  search_box_text_rect_ = temp_painter.boundingRect(0, 0, 0, 0, Qt::AlignLeft | Qt::TextSingleLine, search_box_text_);
  temp_painter.end();

  search_box_highlight_around_text_rect_ = search_box_text_rect_;
  search_box_highlight_around_text_rect_.setHeight(search_box_text_rect_.height() + 2*kSearchBoxTextMarginVertical);
  search_box_highlight_around_text_rect_.setWidth(search_box_text_rect_.width() +
                                                  2*(search_box_highlight_around_text_rect_.height()/2
                                                     + kSearchBoxTextMarginHorizontal) +
                                                  kSearchBoxTextMarginRightAdditionalFudgeFactor);
}

void SearchBoxOverlay::showSearchBoxOverlay() {
  setAlpha(ProAuthorization::singleton()->find_as_you_type_visible_alpha_value());
  overlay_->show();
}

void SearchBoxOverlay::hideSearchBoxOverlay() {
  overlay_->hide();
}

#include "moc/moc_SearchBoxOverlay.cpp"
