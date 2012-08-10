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

#ifndef BUMPTOP_OSX_CONTEXTMENU_H_
#define BUMPTOP_OSX_CONTEXTMENU_H_

#include "BumpTop/BumpTopCommands.h"

void launchContextMenu(const BumpEnvironment& env,
                       const VisualPhysicsActorList& actors, Ogre::Vector2 mouse_in_window_space);
void launchContextMenu(const BumpEnvironment& env, const VisualPhysicsActorList& actors,
                       BumpTopCommandSet* context_menu_items, Ogre::Vector2 mouse_in_window_space);

// unimplemented, todo soon
void contextMenu_Show_Package_Contents(const QString& file_path);
void contextMenu_New_Folder(const QString& file_path);
void contextMenu_New_Burn_Folder(const QString& file_path);

// unimplemented, hard
// ** "Open With"
void contextMenu_Show_Original(const QString& file_path);
void contextMenu_Quick_Look(const QString& file_path);
void contextMenu_Compress(const QString& file_path);
void contextMenu_Copy(const QString& file_path);
void contextMenu_Paste_Item(const QString& file_path);
// ** label colors

#endif  // BUMPTOP_OSX_CONTEXTMENU_H_
