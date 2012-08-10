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

const int kLeftMargin = 30;
const int kTopMargin = 22;

#include "BumpTop/ToolTipOverlay.h"

#include "BumpTop/AlphaElementAnimation.h"
#include "BumpTop/AnimationManager.h"
#include "BumpTop/AppSettings.h"
#include "BumpTop/BumpTopApp.h"
#include "BumpTop/Clickable.h"
#include "BumpTop/FileManager.h"
#include "BumpTop/MaterialLoader.h"
#include "BumpTop/OgreHelpers.h"
#include "BumpTop/OSX/CoreTextHelper.h"
#include "BumpTop/OverlayButton.h"
#include "BumpTop/QPainterMaterial.h"
#include "BumpTop/QStringHelpers.h"
#include "BumpTop/ToolTipManager.h"
#include "BumpTop/VisualPhysicsActor.h"

ToolTipOverlay::ToolTipOverlay()
: text_material_(NULL),
  delete_self_on_next_fadeout_complete_(false) {
}

ToolTipOverlay::~ToolTipOverlay() {
  Ogre::String address = addressToString(this);

  BumpMaterialManager::singleton()->decrementReferenceCountAndDeleteIfZero(QString(background_material_name_));
  BumpTextureManager::singleton()->decrementReferenceCountAndDeleteIfZero(QString(background_material_name_));
  BumpMaterialManager::singleton()->decrementReferenceCountAndDeleteIfZero(QString(background_material_name_));
  BumpTextureManager::singleton()->decrementReferenceCountAndDeleteIfZero(QString(background_material_name_));

  Ogre::OverlayManager::getSingleton().destroyOverlayElement("ToolTip" + address);
  Ogre::OverlayManager::getSingleton().destroyOverlayElement("ToolTipBackground" + address);
  Ogre::OverlayManager::getSingleton().destroyOverlayElement("ToolTipText" + address);
  Ogre::OverlayManager::getSingleton().destroy("ToolTipOverlay" + address);
}

void ToolTipOverlay::initWithTextLines(QStringList text_lines, ToolTipPointingDirection dir) {
  Ogre::String address = addressToString(this);
  Ogre::OverlayElement* tool_tip_panel;
  tool_tip_panel = Ogre::OverlayManager::getSingleton().createOverlayElement("Panel", "ToolTip" + address);
  panel_ = static_cast<Ogre::OverlayContainer*>(tool_tip_panel);

  panel_->setMetricsMode(Ogre::GMM_PIXELS);
  panel_->setPosition(0, 0);
  panel_->setUserAny(Ogre::Any(static_cast<Clickable*>(this)));

  MaterialLoader* material_loader = new MaterialLoader();
  material_loader->initAsImageWithFilePath(FileManager::pathForResource(backgroundForDirection(dir)), false);
  background_material_name_ = material_loader->name();
  BumpMaterialManager::singleton()->incrementReferenceCount(background_material_name_);
  BumpTextureManager::singleton()->incrementReferenceCount(background_material_name_);
  delete material_loader;

  Ogre::OverlayElement* background;
  background = Ogre::OverlayManager::getSingleton().createOverlayElement("Panel", "ToolTipBackground" + address);
  background_ = static_cast<Ogre::OverlayContainer*>(background);
  background_->setMaterialName(utf8(background_material_name_));
  background_->setMetricsMode(Ogre::GMM_PIXELS);

  font_ = QFont("Lucida Grande");
  font_.setBold(true);
  font_.setPointSize(13);
  text_lines_list_ = text_lines;
  createTipText(text_lines);

  Ogre::OverlayElement* text;
  text = Ogre::OverlayManager::getSingleton().createOverlayElement("Panel", "ToolTipText" + address);
  text_lines_ = static_cast<Ogre::OverlayContainer*>(text);
  text_lines_material_name_ = text_material_->name();
  BumpMaterialManager::singleton()->incrementReferenceCount(text_lines_material_name_);
  BumpTextureManager::singleton()->incrementReferenceCount(text_lines_material_name_);
  text_lines_->setMaterialName(utf8(text_lines_material_name_));
  text_lines_->setMetricsMode(Ogre::GMM_PIXELS);
  text_lines_->setDimensions(text_size_.width(), text_size_.height());

  panel_->addChildImpl(background_);
  panel_->addChildImpl(text_lines_);

  setSize(Ogre::Vector2(220, 94));
  setCenter(Ogre::Vector2(500, 500));

  overlay_ = Ogre::OverlayManager::getSingleton().create("ToolTipOverlay" + address);
  overlay_->add2D(panel_);
  setAlpha(0.0);

  overlay_->show();
}

QString ToolTipOverlay::backgroundForDirection(ToolTipPointingDirection dir) {
  if (dir == TOOLTIP_UP) {
    return QString("tooltip-up.png");
  } else if (dir == TOOLTIP_DOWN) {
    return QString("tooltip-down.png");
  } else if (dir == TOOLTIP_LEFT) {
    return QString("tooltip-left.png");
  } else {
    return QString("tooltip-right.png");
  }
}

void ToolTipOverlay::followActor(VisualPhysicsActor* actor) {
  // before we connect to some actor, we need to first disconnect from
  // any actor previously connected to. Caveat, this also disconnects any
  // animation callbacks, but that is not a problem here. (A bit sloppy).
  QObject::disconnect(this);

  actor_to_follow_ = actor;
  assert(QObject::connect(actor, SIGNAL(onPoseChanged(VisualPhysicsActorId)),  // NOLINT
                          this, SLOT(actorToFollowPoseChanged(VisualPhysicsActorId))));  // NOLINT

  assert(QObject::connect(actor, SIGNAL(onRemoved(VisualPhysicsActorId)),  // NOLINT
                          this, SLOT(actorToFollowRemoved(VisualPhysicsActorId))));  // NOLINT
}

void ToolTipOverlay::actorToFollowPoseChanged(VisualPhysicsActorId actor_id) {
  if (actor_to_follow_ != NULL) {
    Ogre::AxisAlignedBox bounding_box = actor_to_follow_->screenBoundingBox();
    Ogre::Vector2 pos = Ogre::Vector2(bounding_box.getMinimum().x, bounding_box.getCenter().y);
    setCenter(pos + Ogre::Vector2(-size().x/2.0, 0));
  }
}

void ToolTipOverlay::actorToFollowRemoved(VisualPhysicsActorId actor_id) {
  actor_to_follow_ = NULL;
  fadeOut();
}

void ToolTipOverlay::createTipText(QStringList text_lines) {
  initTextMaterial();
}

void ToolTipOverlay::fadeOut(Ogre::Real final_alpha) {
  if (!delete_self_on_next_fadeout_complete_) {
    faded_ = true;
    AnimationManager::singleton()->endAnimationsForAlphaElement(this, AnimationManager::STOP_AT_CURRENT_STATE);
    AlphaElementAnimation* animation = new AlphaElementAnimation(this, final_alpha, 500);
    assert(QObject::connect(animation, SIGNAL(onAnimationComplete()),  // NOLINT
                            this, SLOT(fadeOutComplete())));  // NOLINT
    animation->start();
  }
}

void ToolTipOverlay::fadeOutComplete() {
  overlay_->hide();
  if (delete_self_on_next_fadeout_complete_) {
    ToolTipManager::singleton()->deleteTooltip(this);
  }
}

void ToolTipOverlay::fadeOutAndDeleteSelfOnCompletion() {
  if (!delete_self_on_next_fadeout_complete_) {
    fadeOut();
    delete_self_on_next_fadeout_complete_ = true;
  }
}

void ToolTipOverlay::fadeIn(int delay) {
  faded_ = false;
  overlay_->show();
  AnimationManager::singleton()->endAnimationsForAlphaElement(this, AnimationManager::STOP_AT_CURRENT_STATE);
  AlphaElementAnimation* animation = new AlphaElementAnimation(this, 1.0, 500);
  animation->start();
}

void ToolTipOverlay::initTextMaterial() {
  if (text_material_ != NULL) {
    delete text_material_;
    text_material_ = NULL;
  }

  QFontMetrics metrics(font_);
  int max_width = 0;
  int y_pos = 0;
  for_each(QString line, text_lines_list_) {
    QRect textRect = metrics.boundingRect(line);
    if (textRect.size().width() > max_width)
      max_width = textRect.size().width();

    y_pos += textRect.size().height();
  }

  text_material_ = new QPainterMaterial();
  text_material_->initWithSize(max_width, y_pos);

  text_size_ = QSize(text_material_->width(), text_material_->height());

  assert(QObject::connect(text_material_, SIGNAL(draw(QPainter*)),  // NOLINT
                          this, SLOT(drawText(QPainter*))));  // NOLINT
  text_material_->update();
}

void ToolTipOverlay::drawText(QPainter* painter) {
  QFontMetrics metrics(font_);
  painter->setRenderHint(QPainter::Antialiasing, true);
  painter->setRenderHint(QPainter::TextAntialiasing, true);
  painter->setFont(font_);
  painter->setPen(QPen(Qt::white));

  int y_pos = 0;
  for_each(QString line, text_lines_list_) {
    QRect textRect = metrics.boundingRect(line);
    drawNativeText(painter, 0, y_pos, line, Qt::gray);
    y_pos += textRect.size().height();
  }
}

Ogre::Vector2 ToolTipOverlay::size() {
  return Ogre::Vector2(panel_->getWidth(), panel_->getHeight());
}

void ToolTipOverlay::setSize(Ogre::Vector2 size) {
  panel_->setDimensions(size.x, size.y);
  background_->setDimensions(size.x, size.y);
  background_->_notifyZOrder(0);
}

void ToolTipOverlay::setCenter(Ogre::Vector2 position) {
  panel_->setPosition(position.x - panel_->getWidth()/2.0, position.y - panel_->getHeight()/2.0);
  int y_offset = text_lines_list_.count() == 2 ? 8 : 0;
  text_lines_->setPosition(kLeftMargin, kTopMargin + y_offset);
}

void ToolTipOverlay::setAlpha(Ogre::Real alpha) {
  AlphaElement::setAlpha(alpha);
  Ogre::MaterialPtr material = Ogre::MaterialPtr(Ogre::MaterialManager::getSingleton().getByName(utf8(text_lines_material_name_)));  // NOLINT
  material->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setAlphaOperation(Ogre::LBX_MODULATE,
                                                                                   Ogre::LBS_TEXTURE,
                                                                                   Ogre::LBS_MANUAL,
                                                                                   0.0,
                                                                                   alpha);
  material = Ogre::MaterialPtr(Ogre::MaterialManager::getSingleton().getByName(utf8(background_material_name_)));
  material->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setAlphaOperation(Ogre::LBX_MODULATE,
                                                                                   Ogre::LBS_TEXTURE,
                                                                                   Ogre::LBS_MANUAL,
                                                                                   0.0,
                                                                                   alpha);
  BumpTopApp::singleton()->markGlobalStateAsChanged();
}

#include "BumpTop/moc/moc_ToolTipOverlay.cpp"

