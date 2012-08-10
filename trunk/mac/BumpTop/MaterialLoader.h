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

#ifndef BUMPTOP_MATERIALLOADER_H_
#define BUMPTOP_MATERIALLOADER_H_

#include <string>

#include "BumpTop/Singleton.h"

class FileItem;

class BumpTextureManager {
  SINGLETON_HEADER(BumpTextureManager)
 public:
  void incrementReferenceCount(QString texture_name);
  void decrementReferenceCountAndDeleteIfZero(QString texture_name);
 protected:
  QHash<QString, int> texture_reference_counts_;
};

class BumpMaterialManager {
  SINGLETON_HEADER(BumpMaterialManager)
 public:
  void incrementReferenceCount(QString material_name);
  void decrementReferenceCountAndDeleteIfZero(QString material_name);
 protected:
  QHash<QString, int> material_reference_counts_;
};

class MaterialLoader: public QObject, public Ogre::Resource::Listener {
  Q_OBJECT

 public:

  enum IconLoadMethod {
    kStandardIcon = 1,
    kQuickLook = 2,
    kQuickLookWithStandardIconFallback = 3
  };

  explicit MaterialLoader();
  virtual ~MaterialLoader();

  // Initializers
  virtual void initWithImageBuffer(unsigned char *image_data, ushort width, ushort height);
  virtual void initAsImageWithFilePath(const QString& texture_path, bool is_background_loaded = true);
  virtual void initAsImageAndOverlayWithFilePaths(const QString& texture_path,
                                                  const QString& overlay_path,
                                                  bool is_background_loaded = true);

  virtual void initAsIconForFilePath(const QString& file_path,
                                     IconLoadMethod icon_load_method,
                                     bool is_background_loaded = true);
  virtual void initWithColourValue(Ogre::ColourValue color);

  // Getters
  virtual QString name();

  virtual void backgroundLoadingComplete(Ogre::Resource *texture);
  virtual void backgroundPreparingComplete(Ogre::Resource *texture);

  virtual void set_delete_self_on_load_complete(bool value);

  static QSize copyImageAndChangeFormatBasedOnExtensions(QString original_path, QString new_path);
  static QSize maxResolutionForBackground();
  static bool createImageWithResolution(QString original_path, QString new_path, QSize image_resolution);
  static QSize getImageResolution(QString path);
  static QSize desiredBGSizeGivenMaxDimensionsAndSourceImageSize(QSize max_resolution, QSize source_resolution);


 signals:
  void backgroundLoadingComplete(MaterialLoader *material_loader);

 public slots:  // NOLINT
  void renderTick();

 protected:
  virtual void initAsImageWithFilePaths(const QStringList& texture_paths, bool is_background_loaded);
  virtual void init(bool is_background_loaded);
  virtual void materialLoadingComplete();
  QString generateUniqueMaterialNameWithPrefix(QString prefix);

  // Class variables
  static uint64_t unique_id_counter_;
  Ogre::MaterialPtr material_;
  QList<Ogre::TexturePtr> textures_;
  QStringList texture_names_;
  QString material_name_;
  bool enable_depth_check_;
  Ogre::ManualResourceLoader *manual_loader_;
  int expected_number_texture_loaded_callbacks_;
  bool delete_self_on_load_complete_;
};

#endif  // BUMPTOP_MATERIALLOADER_H_
