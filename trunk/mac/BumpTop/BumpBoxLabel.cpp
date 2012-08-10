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
const int kRoundedRectCorner = 9;
const int kTextMarginHorizontal = 1;
const int kTextMarginVertical = 0;
const int kShadowOffsetVertical = 1;
const int kShadowOffsetHorizontal = 0;
const int kExtraSizeForShadowRect = 4;


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
  // First, just find out how big the label is
  font_ = QFont("Lucida Grande");
  font_.setBold(true);
  font_.setPointSize(13);

  text_size_ = getTextBounds(&text_lines_, &text_line_sizes_, 0, kInitialLabelMaxWidth * size_factor);

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
  Ogre::Vector2 adjusted_position = position - Ogre::Vector2(width_of_drawn_region()/2, 0);
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

  return Blitz::blur(unblurred_text, 2);
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
    // Create and draw the blurred shadow text; we draw it thrice to increase the strength
    QImage blurred_text = createBlurredText();

    for (int i = 0; i < 3; i++) {
      painter->drawImage(kShadowOffsetHorizontal - kExtraSizeForShadowRect/2.0,
                         kShadowOffsetVertical - kExtraSizeForShadowRect/2.0,
                         blurred_text);
    }

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
    QSize tmpSize;
    int maxLineWidth = 0;

    // we know that the line does not fit on a single line of the specified
    // preferred width
    if (truncated_) {
      QString line;

      // if the text is within the max bounds then just return it
      // Note: we ignore the line height
      if (width <= maxWidth) {
        linesOut->append(srcText);
        lineSizesOut->append(QSize(textRect.size().width(), height));
        return textRect.size();
      }

      QTextBoundaryFinder boundaries(QTextBoundaryFinder::Word, srcText);
      int prevBoundary = -1;
      int nextBoundary = boundaries.toNextBoundary();
      while (-1 < nextBoundary && metrics.boundingRect(srcText.mid(0, nextBoundary)).width() < maxWidth) {
        prevBoundary = nextBoundary;
        nextBoundary = boundaries.toNextBoundary();
      }

      if (prevBoundary < 0)  {
        int currentChar = 0;
        while (currentChar < srcText.size() &&
            metrics.boundingRect(srcText.mid(0, currentChar + 1)).width() < maxWidth) {
          currentChar++;
        }
        line = srcText.mid(0, currentChar);

        // add first line
        tmpSize = metrics.boundingRect(line).size();
        int maxLineWidth = tmpSize.width();
        linesOut->append(line);
        lineSizesOut->append(tmpSize);

        // break up the second line and add it
        line = metrics.elidedText(srcText.mid(currentChar), Qt::ElideMiddle, maxWidth).trimmed();
        tmpSize = metrics.boundingRect(line).size();
        maxLineWidth = std::max(tmpSize.width(), maxLineWidth);
        linesOut->append(line);
        lineSizesOut->append(tmpSize);
        return QSize(maxLineWidth, linesOut->size() * lineSpacing);
      } else {
        line = srcText.mid(0, prevBoundary).trimmed();
        tmpSize = metrics.boundingRect(line).size();
        linesOut->append(line);
        lineSizesOut->append(tmpSize);
        if (tmpSize.width() > maxLineWidth) {
          maxLineWidth = tmpSize.width();
        }

        line = metrics.elidedText(srcText.mid(prevBoundary), Qt::ElideMiddle, maxWidth).trimmed();
        tmpSize = metrics.boundingRect(line).size();
        linesOut->append(line);
        lineSizesOut->append(tmpSize);
        if (tmpSize.width() > maxLineWidth) {
          maxLineWidth = tmpSize.width();
        }

        return QSize(maxLineWidth, linesOut->size() * lineSpacing);
      }
    } else {
      QString line;
      QTextBoundaryFinder boundaries(QTextBoundaryFinder::Word, srcText);
      int lastBoundary = 0;
      int prevBoundary = 0;
      int nextBoundary = std::min((unsigned int) srcText.indexOf("\n", prevBoundary),
                                  (unsigned int) boundaries.toNextBoundary());
      int quarterMaxWidth = maxWidth / 4;

      while (lastBoundary < srcText.size()) {
        if (nextBoundary >= 0) {
          line = srcText.mid(lastBoundary, nextBoundary - lastBoundary).trimmed();
          if ((metrics.width(line) > maxWidth) || (line.size() > 1 && line.endsWith("\n"))) {
            line = srcText.mid(lastBoundary, prevBoundary - lastBoundary).trimmed();
            if (prevBoundary > lastBoundary && (metrics.width(line) > quarterMaxWidth)) {
              // the next boundary is OK
              line = srcText.mid(lastBoundary, prevBoundary - lastBoundary).trimmed();
              tmpSize = metrics.boundingRect(line).size();
              linesOut->append(line);
              lineSizesOut->append(tmpSize);
              if (tmpSize.width() > maxLineWidth) {
                maxLineWidth = tmpSize.width();
              }
              lastBoundary = prevBoundary;
            } else {  // prevBoundary <= lastBoundary
              // the next boundary is beyond the max width
              int tmpLen = nextBoundary - lastBoundary;
              while (metrics.width(srcText.mid(lastBoundary, tmpLen)) > maxWidth && (tmpLen > 0)) {
                --tmpLen;
              }

              line = srcText.mid(lastBoundary, tmpLen).trimmed();
              tmpSize = metrics.boundingRect(line).size();
              linesOut->append(line);
              lineSizesOut->append(tmpSize);
              if (tmpSize.width() > maxLineWidth) {
                maxLineWidth = tmpSize.width();
              }
              lastBoundary += tmpLen;
              // prevBoundary = lastBoundary;
            }
          } else {
            // move to the next boundary
            prevBoundary = nextBoundary;
            nextBoundary = std::min((unsigned int) srcText.indexOf("\n", prevBoundary + 1),
                                    (unsigned int) boundaries.toNextBoundary());
          }
        } else {
          line = srcText.mid(lastBoundary).trimmed();
          tmpSize = metrics.boundingRect(line).size();
          linesOut->append(line);
          lineSizesOut->append(tmpSize);
          if (tmpSize.width() > maxLineWidth) {
            maxLineWidth = tmpSize.width();
          }
          break;
        }
      }

      return QSize(maxLineWidth, linesOut->size() * lineSpacing);
    }
  }
  return QSize();
}

Ogre::Real BumpBoxLabel::boundingWidth() {
  return text_size_.width();
}

#include "moc/moc_BumpBoxLabel.cpp"

