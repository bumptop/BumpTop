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

#include "BumpTop/BumpBoxLabel.h"

#include <QtCore/QTextBoundaryFinder>
#include <QtGui/QFontDatabase>

#include "BumpTop/ArrayOnStack.h"
#include "BumpTop/Authorization.h"
#include "BumpTop/BumpTopApp.h"
#include "BumpTop/DebugAssert.h"
#include "BumpTop/Math.h"
#include "BumpTop/QPainterMaterial.h"
#include "BumpTop/QStringHelpers.h"
#include "BumpTop/OgreHelpers.h"
#include "BumpTop/OSX/CoreTextHelper.h"
#include "BumpTop/Room.h"
#include "BumpTop/VisualActor.h"
#include "BumpTop/VisualPhysicsActor.h"
#include "BumpTop/VisualPhysicsActorList.h"

#include "ThirdParty/BlitzBlur.h"

const int kInitialLabelMaxWidth = 20 + kInitialActorSize;
// Label textures are shown 1:1 in device pixels, so these point-based
// metrics are scaled by the device scale the first time a label is built.
int kRoundedRectCorner = 9;
int kTextMarginHorizontal = 1;
int kTextMarginVertical = 0;
int kShadowOffsetVertical = 1;
int kShadowOffsetHorizontal = 0;
int kExtraSizeForShadowRect = 4;
int kShadowBlurRadius = 2;

static void scaleLabelMetricsForDevice() {
  static bool label_metrics_scaled = false;
  if (label_metrics_scaled)
    return;
  Ogre::Real device_scale = BumpTopApp::singleton()->device_scale();
  kRoundedRectCorner = qRound(kRoundedRectCorner * device_scale);
  kTextMarginHorizontal = qRound(kTextMarginHorizontal * device_scale);
  kTextMarginVertical = qRound(kTextMarginVertical * device_scale);
  kShadowOffsetVertical = qRound(kShadowOffsetVertical * device_scale);
  kShadowOffsetHorizontal = qRound(kShadowOffsetHorizontal * device_scale);
  kExtraSizeForShadowRect = qRound(kExtraSizeForShadowRect * device_scale);
  kShadowBlurRadius = qRound(kShadowBlurRadius * device_scale);
  label_metrics_scaled = true;
}


SINGLETON_IMPLEMENTATION(BumpBoxLabelManager)

void BumpBoxLabelManager::addLabel(BumpBoxLabel* label) {
  labels_.insert(label);
}

void BumpBoxLabelManager::removeLabel(BumpBoxLabel* label) {
  labels_.remove(label);
}

const QSet<BumpBoxLabel*>& BumpBoxLabelManager::labels() {
  return labels_;
}

BumpBoxLabel::BumpBoxLabel(QString label, VisualPhysicsActor* associated_actor)
: is_selected_(false),
  text_(label),
  node_(NULL),
  material_(NULL),
  truncated_(true),
  truncate_to_single_line_(false),
  text_alignment_(AlignCenter),
  renderable_(NULL),
  visible_(true),
  associated_actor_(associated_actor),
  label_colour_(COLOURLESS) {
  BumpBoxLabelManager::singleton()->addLabel(this);
  assert(label != "");
}

BumpBoxLabel::~BumpBoxLabel() {
  BumpBoxLabelManager::singleton()->removeLabel(this);

  if (manual_ != NULL) {
    manual_->detachFromParent();
    BumpTopApp::singleton()->ogre_scene_manager()->destroyManualObject(manual_);
  }

  node_->getParent()->removeChild(node_);
  BumpTopApp::singleton()->ogre_scene_manager()->destroySceneNode(node_);
}

Ogre::Vector3 BumpBoxLabel::position() {
  return associated_actor_->world_position();
}

Ogre::Plane BumpBoxLabel::plane() {
  return associated_actor_->plane();
}

BumpBoxLabelColour BumpBoxLabel::label_colour() {
  return label_colour_;
}

void BumpBoxLabel::set_label_colour(BumpBoxLabelColour label_colour) {
  label_colour_ = label_colour;
}

void BumpBoxLabel::init(Ogre::Real size_factor) {
  // Labels render into textures shown 1:1 in device pixels; scale the type up
  // on Retina displays so it keeps its visual point size.
  scaleLabelMetricsForDevice();
  Ogre::Real device_scale = BumpTopApp::singleton()->device_scale();
  // First, just find out how big the label is
  // Lucida Grande was the system font when this was written; use the current
  // system font (SF), which is also what Finder draws desktop labels with.
  font_ = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
  // Finder's desktop labels are semibold system font (verified by pixel diff
  // against Finder's rendering; full bold measures visibly heavier).
  font_.setWeight(QFont::DemiBold);
  int label_point_size = 13;
  if (getenv("BUMPTOP_LABEL_SIZE") != NULL)
    label_point_size = atoi(getenv("BUMPTOP_LABEL_SIZE"));
  font_.setPointSize(qRound(label_point_size * device_scale));

  text_size_ = getTextBounds(&text_lines_, &text_line_sizes_, 0,
                             kInitialLabelMaxWidth * size_factor * device_scale);

  QFontMetrics metrics(font_);
  QRect text_rect = metrics.boundingRect(text_);


  highlight_around_text_rect_.setX(0);
  highlight_around_text_rect_.setY(0);
  highlight_around_text_rect_.setHeight(text_size_.height() + 2*kTextMarginVertical);
  highlight_around_text_rect_.setWidth(text_size_.width() + 2*kRoundedRectCorner);

  // Create the label material
  initMaterial(ceil(static_cast<float>(highlight_around_text_rect_.width())),
               ceil(static_cast<float>(highlight_around_text_rect_.height())));
  // Create a manual object for 2D
  manual_ = BumpTopApp::singleton()->ogre_scene_manager()->createManualObject("ManualObject" + addressToString(this));

  manual_->begin(utf8(material_->name()), Ogre::RenderOperation::OT_TRIANGLE_FAN);
  manual_->position(0, -40.0, 0);
  manual_->textureCoord(0, 0, 0);
  manual_->position(0, 0, 0);
  manual_->textureCoord(0, -1, 0);
  manual_->position(40.0, 0, 0);
  manual_->textureCoord(1, -1, 0);
  manual_->position(40.0, -40.0, 0);
  manual_->textureCoord(1, 0, 0);
  manual_->index(3);
  manual_->index(2);
  manual_->index(1);
  manual_->index(0);
  manual_->end();

  // just so that we know about our renderable
  manual_->visitRenderables(this);

  // Use infinite AAB to always stay visible
  manual_->setBoundingBox(Ogre::AxisAlignedBox::BOX_INFINITE);

  manual_->setUseIdentityView(true);
  manual_->setUseIdentityProjection(true);

  // Attach to scene node
  node_ = BumpTopApp::singleton()->ogre_scene_manager()->getRootSceneNode()->createChildSceneNode();
  node_->setInheritOrientation(false);
  node_->setInheritScale(false);

  assert(QObject::connect(BumpTopApp::singleton(), SIGNAL(onWindowRectChanged()),
                          this, SLOT(updateScale())));
  updateScale();

  node_->attachObject(manual_);
}

void BumpBoxLabel::updateScale() {
  Ogre::Vector2 window_size = BumpTopApp::singleton()->window_size();
  node_->setScale(Ogre::Vector3(material_->width()/(window_size.x*20), material_->height()/(window_size.y*20), 1));
}

void BumpBoxLabel::set_visible(bool visible) {
  visible_ = visible;
  node_->setVisible(visible);
}

bool BumpBoxLabel::visible() {
  return visible_;
}

Ogre::Entity* BumpBoxLabel::_entity() {
  return NULL;
}

void BumpBoxLabel::set_position_in_pixel_coords(Ogre::Vector2 position) {
  // Nudge the label down so the icon-to-text gap matches Finder's
  // (14.5pt, measured pixel-wise in the parity test).
  Ogre::Real label_gap_adjust = -5.5 * BumpTopApp::singleton()->device_scale();
  Ogre::Vector2 adjusted_position = position - Ogre::Vector2(width_of_drawn_region()/2, label_gap_adjust);
  Ogre::Vector2 normalized_position = screenPositionToNormalizedScreenPosition(adjusted_position);
  node_->setPosition(Ogre::Vector3(normalized_position.x, normalized_position.y, 0));
}

Ogre::Vector2 BumpBoxLabel::position_in_pixel_coords() {
  Ogre::Vector3 position = node_->getPosition();
  return normalizedScreenPositionToScreenPosition(Ogre::Vector2(position.x, position.y));
}

size_t BumpBoxLabel::width_of_drawn_region() {
  return material_->width_of_drawn_region();
}

size_t BumpBoxLabel::width() {
  return material_->width();
}

size_t BumpBoxLabel::height() {
  return material_->height();
}

Ogre::Renderable* BumpBoxLabel::renderable() {
  return renderable_;
}

Ogre::MovableObject* BumpBoxLabel::movable_object() {
  return manual_;
}

void BumpBoxLabel::set_render_queue_group(uint8 queue_id) {
  manual_->setRenderQueueGroup(queue_id);
}

void BumpBoxLabel::visit(Ogre::Renderable *renderable, ushort lod_index, bool is_debug, Ogre::Any *any) {
  renderable_ = renderable;
}

void BumpBoxLabel::initMaterial(int material_width, int material_height) {
  if (material_ != NULL) {
    delete material_;
    material_ = NULL;
  }

  material_ = new QPainterMaterial();
  material_->initWithSize(material_width, material_height);
  assert(QObject::connect(material_, SIGNAL(draw(QPainter*)),  // NOLINT
                          this, SLOT(draw(QPainter*))));  // NOLINT
  material_->update();
}

QImage BumpBoxLabel::createBlurredText() {
  QImage unblurred_text = QImage(text_size_.width() + kExtraSizeForShadowRect,
                                 text_size_.height() + kExtraSizeForShadowRect,
                                 QImage::Format_ARGB32);
  QPainter painter;

  unblurred_text.fill(0);
  painter.begin(&unblurred_text);
  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.setRenderHint(QPainter::TextAntialiasing, true);
  painter.setPen(QPen(QColor(0, 0, 0, 255)));
  painter.translate(kExtraSizeForShadowRect/2, kExtraSizeForShadowRect/2);
  painter.setFont(font_);
  int x = 0;
  int y = 0;
  for (int i = 0; i < text_lines_.size(); ++i) {
    // center the text horizontally
    switch (text_alignment_) {
      case AlignLeft:
        x = 0;
        break;
      case AlignCenter:
        x = (text_size_.width() - text_line_sizes_[i].width()) / 2;
        break;
      case AlignRight:
        x = text_size_.width() - text_line_sizes_[i].width();
        break;
      default: assert(false); break;
    }

    drawNativeText(&painter, x, y, text_lines_[i], Qt::black);
    y += text_line_sizes_[i].height();
  }
  painter.end();

  return Blitz::blur(unblurred_text, kShadowBlurRadius);
}


void BumpBoxLabel::draw(QPainter* painter) {
  painter->setRenderHint(QPainter::Antialiasing, true);
  painter->setRenderHint(QPainter::TextAntialiasing, true);
  painter->setPen(Qt::NoPen);

  QColor colour = Qt::transparent;
  QColor text_background_color = Qt::black;
  // set label colour
  switch (label_colour()) {
    case RED:
      colour = QColor(250,60,50,250);
      break;
    case ORANGE:
      colour = QColor(220,130,70,250);
      break;
    case YELLOW:
      colour = QColor(220,190,70,250);
      break;
    case GREEN:
      colour = QColor(80,190,40,250);
      break;
    case BLUE:
      colour = QColor(140, 180, 250, 250);
      break;
    case PURPLE:
      colour = QColor(140,70,170,250);
      break;
    case GREY:
      colour = QColor(170,170,170,250);
      break;
    default:
      break;
  }
  QBrush brush;
  if (label_colour() == COLOURLESS) {
    if (is_selected_) {
      // Colourless selection labels are blue
      brush = QBrush(QColor(80, 120, 250, 180));
      text_background_color = Qt::white;
    } else {
      brush = QBrush(Qt::transparent);
    }
  } else {
    QLinearGradient gradient(QPointF(0, 20), QPointF(0, -40));
    gradient.setColorAt(0, colour);
    gradient.setColorAt(1, Qt::white);
    brush = QBrush(gradient);
  }
  painter->setBrush(brush);
  painter->drawRoundedRect(highlight_around_text_rect_, kRoundedRectCorner, kRoundedRectCorner);


  painter->translate(kRoundedRectCorner + kTextMarginHorizontal, kTextMarginVertical);
  if (!is_selected_
      && label_colour() == COLOURLESS) {
    // if it's not selected we draw the blurred shadow
    // Single pass at reduced opacity: pixel-diffed against Finder's subtle
    // label shadow (the original stamped it three times at full strength).
    QImage blurred_text = createBlurredText();

    qreal previous_opacity = painter->opacity();
    painter->setOpacity(0.6 * previous_opacity);
    painter->drawImage(kShadowOffsetHorizontal - kExtraSizeForShadowRect/2.0,
                       kShadowOffsetVertical - kExtraSizeForShadowRect/2.0,
                       blurred_text);
    painter->setOpacity(previous_opacity);

    text_background_color = Qt::gray;
  }

  // draw the label text
  if (label_colour() == COLOURLESS) {
    painter->setPen(QPen(Qt::white));
  } else {
    painter->setPen(QPen(Qt::black));
  }
  painter->setFont(font_);

  int x = 0;
  int y = 0;
  for (int i = 0; i < text_lines_.size(); ++i) {
    // center the text horizontally
    switch (text_alignment_) {
      case AlignLeft:
        x = 0;
        break;
      case AlignCenter:
        x = (text_size_.width() - text_line_sizes_[i].width()) / 2;
        break;
      case AlignRight:
        x = text_size_.width() - text_line_sizes_[i].width();
        break;
      default: assert(false); break;
    }

    drawNativeText(painter, x, y, text_lines_[i], text_background_color);
    y += text_line_sizes_[i].height();
  }
}

void BumpBoxLabel::set_selected(bool is_selected) {
  if (is_selected != is_selected_) {
    is_selected_ = is_selected;
  }
  material_->update();
}

bool BumpBoxLabel::selected() {
  return is_selected_;
}

QSize BumpBoxLabel::getTextBounds(QStringList *linesOut, QList<QSize> *lineSizesOut, int leading, int max_width) {
  if (getenv("BUMPTOP_DEBUG_LABELS") != NULL) {
    fprintf(stderr, "[label-in] '%s' max_width=%d truncated=%d single=%d\n",
            utf8(text_).c_str(), max_width, truncated_, truncate_to_single_line_);
  }
  if (text_.isEmpty()) {
    return QSize();
  }

  // clear the lines out
  linesOut->clear();

  QString srcText = text_;

  // get the text metrics and determine how to split the lines
  QFontMetrics metrics(font_);
  QRect textRect = metrics.boundingRect(srcText);
  textRect.translate(-textRect.left(), -textRect.top());
  int lineSpacing = std::max(0, metrics.lineSpacing() - 1);
  int height = textRect.height();
  int width = textRect.width();
  int maxWidth = std::max(metrics.averageCharWidth(), max_width);

  // ensure that the area is a minimum size
  if (maxWidth < (2 * metrics.averageCharWidth())) {
    return QSize(max_width, 0);
  }

  if (truncate_to_single_line_ && truncated_) {
    // get the elided text for the single line
    QString line = metrics.elidedText(srcText, Qt::ElideMiddle, maxWidth).trimmed();
    QSize tmpSize = metrics.boundingRect(line).size();
    linesOut->    append(line);
    lineSizesOut->append(tmpSize);
    return QSize(tmpSize.width(), height);
  } else {  // !truncate_to_single_line_
    // Wrap like Finder's desktop labels: a single line when it fits,
    // otherwise exactly two lines chosen to be BALANCED (Finder minimizes the
    // longer line rather than filling the first line greedily), with the
    // second line middle-elided when the tail cannot fit.
    if (width <= maxWidth) {
      linesOut->append(srcText);
      lineSizesOut->append(QSize(textRect.size().width(), height));
      return textRect.size();
    }

    QTextBoundaryFinder boundaries(QTextBoundaryFinder::Line, srcText);
    QString best_line1;
    QString best_line2;
    int best_score = -1;
    int boundary = boundaries.toNextBoundary();
    while (boundary > 0 && boundary < srcText.size()) {
      QString line1 = srcText.left(boundary).trimmed();
      int width1 = metrics.horizontalAdvance(line1);
      if (width1 > maxWidth)
        break;  // later break positions only make line 1 wider
      QString line2 = metrics.elidedText(srcText.mid(boundary).trimmed(),
                                         Qt::ElideMiddle, maxWidth).trimmed();
      int width2 = metrics.horizontalAdvance(line2);
      int score = std::max(width1, width2);
      if (best_score < 0 || score < best_score) {
        best_score = score;
        best_line1 = line1;
        best_line2 = line2;
      }
      boundary = boundaries.toNextBoundary();
    }

    if (best_score < 0) {
      // No break opportunity fits (one enormous word): hard-split by
      // characters and elide the rest.
      int current_char = 0;
      while (current_char < srcText.size() &&
             metrics.horizontalAdvance(srcText.mid(0, current_char + 1)) < maxWidth) {
        current_char++;
      }
      best_line1 = srcText.mid(0, current_char);
      best_line2 = metrics.elidedText(srcText.mid(current_char), Qt::ElideMiddle, maxWidth).trimmed();
    }

    int maxLineWidth = 0;
    QStringList balanced_lines;  // named: BOOST_FOREACH dangles on temporaries
    balanced_lines << best_line1 << best_line2;
    for_each(QString line, balanced_lines) {
      QSize tmpSize = metrics.boundingRect(line).size();
      linesOut->append(line);
      lineSizesOut->append(tmpSize);
      maxLineWidth = std::max(maxLineWidth, tmpSize.width());
    }
    if (getenv("BUMPTOP_DEBUG_LABELS") != NULL) {
      fprintf(stderr, "[label] '%s' -> '%s' (%d) / '%s' (%d) max=%d spacing=%d\n",
              utf8(text_).c_str(), utf8(best_line1).c_str(),
              metrics.horizontalAdvance(best_line1),
              utf8(best_line2).c_str(), metrics.horizontalAdvance(best_line2),
              max_width, lineSpacing);
    }
    return QSize(maxLineWidth, linesOut->size() * lineSpacing);
  }
  return QSize();
}

Ogre::Real BumpBoxLabel::boundingWidth() {
  return text_size_.width();
}


