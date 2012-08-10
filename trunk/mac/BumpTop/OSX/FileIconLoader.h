//
//  Copyright 2012 Google Inc. All Rights Reserved.
//  
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  
//      http://www.apache.org/licenses/LICENSE-2.0
//  
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//

#ifndef BUMPTOP_OSX_FILEICON_H_
#define BUMPTOP_OSX_FILEICON_H_

#include "BumpTop/MaterialLoader.h"

class BitmapImage;
class FileItem;

// Basic image wrapper
struct BitmapImage {
  unsigned char *image_data;
  unsigned int width;
  unsigned int height;
};

BitmapImage iconBitmapForPath(QString path, int icon_size, MaterialLoader::IconLoadMethod icon_load_method);

class FileIconLoader : public Ogre::ManualResourceLoader {
 public:
  FileIconLoader(const QString& file_path, MaterialLoader::IconLoadMethod icon_load_method);
  virtual ~FileIconLoader();

  virtual void loadResource (Ogre::Resource *resource);
 protected:
  Ogre::ConstImagePtrList image_list_;
  Ogre::String mName;
  QString file_path_;
  BitmapImage icon_;
  MaterialLoader::IconLoadMethod icon_load_method_;
};

CGImageRef iconForContextMenu(QString path);

#endif  // BUMPTOP_OSX_FILEICON_H_
