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

#ifndef BUMPTOP_OSX_OSXCOCOADRAGANDDROP_H_
#define BUMPTOP_OSX_OSXCOCOADRAGANDDROP_H_

#include "BumpTop/DragAndDrop.h"

#include "BumpTop/MaterialLoader.h"
#import <QuickLook/QuickLook.h>

class OSXCocoaDragAndDrop : public DragAndDrop {
 public:
  OSXCocoaDragAndDrop(NSView* view);
  virtual ~OSXCocoaDragAndDrop();
  virtual void initiateDrag(QStringList paths);
  virtual void dragFinishedInTrash();
  void set_last_mouse_down_event(NSEvent* mouse_down_event);
 protected:
  void writeToPasteboard(NSPasteboard *pb, QStringList paths);

  NSImage *iconImageForPath(QString path, int icon_size, MaterialLoader::IconLoadMethod icon_load_method);
  NSEvent *last_mouse_down_event_;
  NSView *view_;
  QStringList dragged_file_paths_;
};

#endif  // BUMPTOP_OSX_OSXCOCOADRAGANDDROP_H_