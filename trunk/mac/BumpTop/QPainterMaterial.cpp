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

#include "BumpTop/QPainterMaterial.h"

#include "BumpTop/Authorization.h"
#include "BumpTop/BumpTopApp.h"
#include "BumpTop/Math.h"
#include "BumpTop/OgreHelpers.h"
#include "BumpTop/QStringHelpers.h"

QPainterMaterial::QPainterMaterial() {
}

QPainterMaterial::~QPainterMaterial() {
  delete qpainter_;
  delete qimage_;
  delete pixelbox_;
  delete[] image_buffer_;

  Ogre::MaterialManager::getSingleton().remove(material_->getName());
  Ogre::TextureManager::getSingleton().remove(texture_->getName());
}

void QPainterMaterial::initWithSize(size_t width, size_t height, int num_mip_maps, bool use_texture_filtering) {
  width_of_drawn_region_ = width;
  height_of_drawn_region_ = height;
  num_mip_maps_ = num_mip_maps;

  width = Math::nearestPowerOf2(width);
  height = Math::nearestPowerOf2(height);

  texture_ = Ogre::TextureManager::getSingleton().createManual(addressToString(this),
                                                               Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                                               Ogre::TEX_TYPE_2D,
                                                               width,
                                                               height,
                                                               num_mip_maps_,  // number of mipmaps
                                                               Ogre::PF_A8R8G8B8,
                                                               Ogre::TU_DYNAMIC);

  material_ = Ogre::MaterialManager::getSingleton().create(addressToString(this),
                                                           Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

  Ogre::Pass *texture_pass = material_->getTechnique(0)->getPass(0);

  texture_pass->createTextureUnitState(addressToString(this));

  texture_pass->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
  if (use_texture_filtering) {
    texture_pass->getTextureUnitState(0)->setTextureFiltering(Ogre::TFO_ANISOTROPIC);
  } else {
    texture_pass->getTextureUnitState(0)->setTextureFiltering(Ogre::FO_NONE, Ogre::FO_NONE, Ogre::FO_NONE);
  }
  texture_pass->setDepthCheckEnabled(false);

  image_buffer_ = new uchar[4*width*height];
  qimage_ = new QImage(image_buffer_, width, height, 4*width,
                       QImage::Format_ARGB32);

  qpainter_ = new QPainter();

  pixelbox_ = new Ogre::PixelBox(width, height, 1, Ogre::PF_A8R8G8B8, image_buffer_);
}

void QPainterMaterial::update() {
  qimage_->fill(0);
  qpainter_->begin(qimage_);
  emit draw(qpainter_);
  qpainter_->end();
  BumpTopApp::singleton()->pushGLContextAndSwitchToOgreGLContext();
  for (int i = 0; i < texture_->getNumMipmaps() + 1; i++) {
    texture_->getBuffer(0, i)->blitFromMemory(*pixelbox_);
  }
  BumpTopApp::singleton()->popGLContext();

  BumpTopApp::singleton()->markGlobalStateAsChanged();
}

size_t QPainterMaterial::width_of_drawn_region() {
  return width_of_drawn_region_;
}

size_t QPainterMaterial::height_of_drawn_region() {
  return height_of_drawn_region_;
}

size_t QPainterMaterial::width() {
  return qimage_->width();
}
size_t QPainterMaterial::height() {
  return qimage_->height();
}

QString QPainterMaterial::name() {
  return QStringFromUtf8(material_->getName());
}

void QPainterMaterial::setAlpha(Ogre::Real alpha) {
  material_->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setAlphaOperation(Ogre::LBX_MODULATE,
                                                                                    Ogre::LBS_TEXTURE,
                                                                                    Ogre::LBS_MANUAL,
                                                                                    0.0,
                                                                                    alpha);
  update();
}

#include "moc/moc_QPainterMaterial.cpp"
