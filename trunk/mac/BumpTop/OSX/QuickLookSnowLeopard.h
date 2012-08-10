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

#ifndef BUMPTOP_OSX_QUICKLOOKSNOWLEOPARD_H_
#define BUMPTOP_OSX_QUICKLOOKSNOWLEOPARD_H_

#include "BumpTop/QuickLookInterface.h"

#if !defined(MAC_OS_X_VERSION_10_6)
#   define MAC_OS_X_VERSION_10_6 MAC_OS_X_VERSION_10_5 + 1
#endif
#  if (MAC_OS_X_VERSION_MAX_ALLOWED == MAC_OS_X_VERSION_10_6)

class SnowLeopardQuickLookInterface : public QuickLookInterface {
 public:
  virtual void previewPathsInSharedPreviewPanel(QHash<QString, VisualPhysicsActorId> paths);
  virtual bool sharedPreviewPanelIsOpen();
  virtual void closeSharedPreviewPanel();
  virtual void setSharedPreviewPanelPaths(QHash<QString, VisualPhysicsActorId> paths);
};

#endif  // (MAC_OS_X_VERSION_MAX_ALLOWED == MAC_OS_X_VERSION_10_6)

#endif  // BUMPTOP_OSX_QUICKLOOKSNOWLEOPARD_H_
