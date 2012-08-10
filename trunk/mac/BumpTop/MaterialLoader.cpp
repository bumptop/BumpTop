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

#include "BumpTop/MaterialLoader.h"

#include <string>

#include "BumpTop/Authorization.h"
#include "BumpTop/BumpTopApp.h"
#include "BumpTop/OSX/FileIconLoader.h"
#include "BumpTop/QStringHelpers.h"

#define DEFAULT_RESOURCE_GROUP_NAME Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME

uint64_t MaterialLoader::unique_id_counter_ = 0;

SINGLETON_IMPLEMENTATION(BumpTextureManager)

void BumpTextureManager::incrementReferenceCount(QString texture_name) {
  if (texture_reference_counts_.contains(texture_name)) {
    texture_reference_counts_[texture_name] += 1;
  } else {
    texture_reference_counts_[texture_name] = 1;
  }
}

void BumpTextureManager::decrementReferenceCountAndDeleteIfZero(QString texture_name) {
  assert(texture_reference_counts_.contains(texture_name));
  texture_reference_counts_[texture_name] -= 1;
  assert(texture_reference_counts_[texture_name] >= 0);

  if (texture_reference_counts_[texture_name] == 0) {
    texture_reference_counts_.remove(texture_name);
    Ogre::TextureManager::getSingleton().remove(utf8(texture_name));
  }
}

SINGLETON_IMPLEMENTATION(BumpMaterialManager)

void BumpMaterialManager::incrementReferenceCount(QString material_name) {
  if (material_reference_counts_.contains(material_name)) {
    material_reference_counts_[material_name] += 1;
  } else {
    material_reference_counts_[material_name] = 1;
  }
}

void BumpMaterialManager::decrementReferenceCountAndDeleteIfZero(QString material_name) {
  assert(material_reference_counts_.contains(material_name));
  material_reference_counts_[material_name] -= 1;
  assert(material_reference_counts_[material_name] >= 0);

  if (material_reference_counts_[material_name] == 0) {
    material_reference_counts_.remove(material_name);
    Ogre::MaterialManager::getSingleton().remove(utf8(material_name));
  }
}

MaterialLoader::MaterialLoader()
: enable_depth_check_(true),
  manual_loader_(NULL),
  expected_number_texture_loaded_callbacks_(0),
  delete_self_on_load_complete_(false) {
}

MaterialLoader::~MaterialLoader() {
}

void MaterialLoader::initWithImageBuffer(unsigned char* image_buffer, ushort width, ushort height) {
  material_name_ = generateUniqueMaterialNameWithPrefix("MaterialFromImageBuffer");
#if TARGET_RT_BIG_ENDIAN
  Ogre::PixelFormat pixel_format = Ogre::PF_R8G8B8A8;
#else
  Ogre::PixelFormat pixel_format = Ogre::PF_A8B8G8R8;
#endif  // TARGET_RT_BIG_ENDIAN
  Ogre::Image image;
  image.loadDynamicImage(image_buffer, width, height, pixel_format);

  textures_.push_back(Ogre::TextureManager::getSingleton().loadImage(utf8(material_name_),
                                                           DEFAULT_RESOURCE_GROUP_NAME,
                                                           image));
  texture_names_.push_back(material_name_);  // the texture takes on the materials' name in the case of a single texture

  Ogre::MaterialManager::getSingleton().setDefaultTextureFiltering(Ogre::TFO_ANISOTROPIC);
  Ogre::MaterialManager::getSingleton().setDefaultAnisotropy(16);
  material_ = Ogre::MaterialManager::getSingleton().create(utf8(material_name_),
                                                           DEFAULT_RESOURCE_GROUP_NAME);
  Ogre::TextureUnitState *tus = material_->getTechnique(0)->getPass(0)->createTextureUnitState(utf8(texture_names_[0]));
  tus->setTextureAddressingMode(Ogre::TextureUnitState::TAM_CLAMP);
  material_->getTechnique(0)->getPass(0)->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
  material_->getTechnique(0)->getPass(0)->setDepthCheckEnabled(false);
}

void MaterialLoader::initWithColourValue(Ogre::ColourValue color) {
  QString material_name_ = QString("SolidColorMaterial%1,%2,%3,%4").arg(color.r)
                                                                               .arg(color.g)
                                                                               .arg(color.b)
                                                                               .arg(color.a);

  Ogre::MaterialPtr material;
  material = Ogre::MaterialManager::getSingleton().create(utf8(material_name_),
                                                          DEFAULT_RESOURCE_GROUP_NAME);
  material->getTechnique(0)->getPass(0)->setAmbient(color);
}

void MaterialLoader::initAsIconForFilePath(const QString& file_path,
                                           IconLoadMethod icon_load_method,
                                           bool is_background_loaded) {
  manual_loader_ = new FileIconLoader(file_path, icon_load_method);
  material_name_ = generateUniqueMaterialNameWithPrefix(QString("IconFor%1").arg(file_path));
  texture_names_.push_back(material_name_);
  enable_depth_check_ = false;
  init(is_background_loaded);
}

void MaterialLoader::initAsImageWithFilePath(const QString& texture_path, bool is_background_loaded) {
  QStringList texture_paths;
  texture_paths.push_back(texture_path);
  material_name_ = texture_path;
  enable_depth_check_ = false;
  initAsImageWithFilePaths(texture_paths, is_background_loaded);
}

void MaterialLoader::initAsImageAndOverlayWithFilePaths(const QString& texture_path,
                                                        const QString& overlay_path,
                                                        bool is_background_loaded) {
  QStringList texture_paths;
  texture_paths.push_back(texture_path);
  texture_paths.push_back(overlay_path);

  // we want a unique material name for this combination of texture and overlay
  material_name_ = "<OVERLAY>" + texture_paths[1] + "<ENDOVERLAY>" + texture_paths[0];

  initAsImageWithFilePaths(texture_paths, is_background_loaded);
}


void MaterialLoader::initAsImageWithFilePaths(const QStringList& texture_paths, bool is_background_loaded) {
  texture_names_ = texture_paths;

  init(is_background_loaded);
}

void MaterialLoader::backgroundLoadingComplete(Ogre::Resource *texture) {
  expected_number_texture_loaded_callbacks_--;

  if (expected_number_texture_loaded_callbacks_ == 0) {
    materialLoadingComplete();
  }
}

void MaterialLoader::materialLoadingComplete() {
  Ogre::Pass *texture_pass = material_->getTechnique(0)->getPass(0);
  texture_pass->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
  Ogre::TextureUnitState *tus = texture_pass->createTextureUnitState(textures_[0].getPointer()->getName());
  tus->setTextureAddressingMode(Ogre::TextureUnitState::TAM_CLAMP);

  if (textures_.count() == 2) {
    Ogre::TextureUnitState* overlay_tex_state = texture_pass->createTextureUnitState(textures_[1].getPointer()->getName());  // NOLINT
    overlay_tex_state->setAlphaOperation(Ogre::LBX_ADD);
    overlay_tex_state->setColourOperationEx(Ogre::LBX_MODULATE);
    overlay_tex_state->setTextureAddressingMode(Ogre::TextureUnitState::TAM_CLAMP);
  }

  if (!enable_depth_check_) {
    texture_pass->setDepthCheckEnabled(false);
  }

  BumpTopApp::singleton()->markGlobalStateAsChanged();
  if (manual_loader_ != NULL) {
    delete manual_loader_;
  }

  // Here we are delaying the emission of the the material loader's "loading complete" signal
  // for one render tick. This gives local scope references to the material loader a chance
  // to access them before said signal triggers a deletion of this object
  assert(QObject::connect(BumpTopApp::singleton(), SIGNAL(onRender()),  // NOLINT
                          this, SLOT(renderTick())));  // NOLINT
}

void MaterialLoader::backgroundPreparingComplete(Ogre::Resource *texture) {
}

void MaterialLoader::set_delete_self_on_load_complete(bool value) {
  delete_self_on_load_complete_ = value;
}

void MaterialLoader::init(bool is_background_loaded) {
  for_each(QString texture_name, texture_names_) {
    BumpTextureManager::singleton()->incrementReferenceCount(texture_name);
  }
  BumpMaterialManager::singleton()->incrementReferenceCount(material_name_);

  bool manually_loaded = manual_loader_ != NULL;
  expected_number_texture_loaded_callbacks_ = texture_names_.count();

  Ogre::MaterialManager::getSingleton().setDefaultTextureFiltering(Ogre::TFO_ANISOTROPIC);
  Ogre::MaterialManager::getSingleton().setDefaultAnisotropy(16);

  for_each(QString texture_name, texture_names_) {
    if (Ogre::TextureManager::getSingleton().resourceExists(utf8(texture_name))) {
      expected_number_texture_loaded_callbacks_--;
      textures_.push_back(Ogre::TextureManager::getSingleton().getByName(utf8(texture_name)));
    } else {
      Ogre::TexturePtr texture = Ogre::TextureManager::getSingleton().create(utf8(texture_name),
                                                                             DEFAULT_RESOURCE_GROUP_NAME,
                                                                             manually_loaded,
                                                                             manually_loaded ? manual_loader_ : NULL);
      textures_.push_back(texture);
      if (is_background_loaded) {
        texture->setBackgroundLoaded(true);
        texture->addListener(this);
        Ogre::ResourceBackgroundQueue::getSingleton().load("Texture",
                                                            utf8(texture_name),
                                                            DEFAULT_RESOURCE_GROUP_NAME,
                                                            manually_loaded,
                                                            manually_loaded ? manual_loader_ : NULL, 0, 0);
      }
    }
  }
  material_ = Ogre::MaterialManager::getSingleton().create(utf8(material_name_),
                                                           DEFAULT_RESOURCE_GROUP_NAME);
  // we won't be getting any texture loaded call-backs if we're not laoding
  // in the background
  if (!is_background_loaded) {
    expected_number_texture_loaded_callbacks_ = 0;
  }

  if (expected_number_texture_loaded_callbacks_ == 0) {
    materialLoadingComplete();
  }
}

QString MaterialLoader::generateUniqueMaterialNameWithPrefix(QString prefix) {
  unique_id_counter_++;
  return QString("%1 (%2)").arg(prefix).arg(unique_id_counter_);
}

QString MaterialLoader::name() {
  return QStringFromUtf8(material_->getName());
}

void MaterialLoader::renderTick() {
  assert(QObject::disconnect(BumpTopApp::singleton(), SIGNAL(onRender()),  // NOLINT
                             this, SLOT(renderTick())));  // NOLINT

  if (delete_self_on_load_complete_) {
    delete this;
  } else {
    // this signal will often ultimately result in the this objects deletion
    emit backgroundLoadingComplete(this);
  }
}

QSize MaterialLoader::copyImageAndChangeFormatBasedOnExtensions(QString original_path, QString new_path) {
  Ogre::Image new_texture = Ogre::Image();
  new_texture.load(utf8(original_path), DEFAULT_RESOURCE_GROUP_NAME);
  QSize size = QSize(new_texture.getWidth(), new_texture.getHeight());
  new_texture.save(utf8(new_path));
  return size;
}

QSize MaterialLoader::getImageResolution(QString path) {
  Ogre::Image new_texture = Ogre::Image();
  new_texture.load(utf8(path), DEFAULT_RESOURCE_GROUP_NAME);
  QSize size = QSize(new_texture.getWidth(), new_texture.getHeight());
  return size;
}

QSize MaterialLoader::desiredBGSizeGivenMaxDimensionsAndSourceImageSize(QSize max_resolution, QSize source_resolution) {
  bool image_scaled = false;
  Ogre::Real width_ratio = 1.0;
  Ogre::Real height_ratio = 1.0;
  ushort width = source_resolution.width();
  ushort height = source_resolution.height();
  ushort max_width = max_resolution.width();
  ushort max_height = max_resolution.height();

  if (width > max_width) {
    width_ratio = (max_width/(1.0*width));
    image_scaled = true;
  }

  if (height > max_height) {
    height_ratio = (max_height/(1.0*height));
    image_scaled = true;
  }

  if (image_scaled) {
    Ogre::Real resize_ratio = std::min(width_ratio, height_ratio);
    return QSize(floor(resize_ratio*width), floor(resize_ratio*height));
  } else {
    return source_resolution;
  }
}

QSize MaterialLoader::maxResolutionForBackground() {
  GLint max_texture_size;
  glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size);
  int max_width = std::min(max_texture_size, (GLint)round(BumpTopApp::singleton()->screen_resolution().x));
  int max_height = std::min(max_texture_size, (GLint)round(BumpTopApp::singleton()->screen_resolution().y));
  return QSize(max_width, max_height);
}

bool MaterialLoader::createImageWithResolution(QString original_path, QString new_path, QSize image_resolution) {
  Ogre::Image new_texture = Ogre::Image();
  new_texture.load(utf8(original_path), DEFAULT_RESOURCE_GROUP_NAME);
  new_texture.resize(image_resolution.width(), image_resolution.height());
  new_texture.save(utf8(new_path));
  return true;
}

#include "moc/moc_MaterialLoader.cpp"
