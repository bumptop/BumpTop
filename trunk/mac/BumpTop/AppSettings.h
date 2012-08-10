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

#ifndef BUMPTOP_APPSETTINGS_H_
#define BUMPTOP_APPSETTINGS_H_

#include "BumpTop/protoc/AllMessages.pb.h"
#include "BumpTop/Singleton.h"

enum OSXDockPosition {
  BOTTOM_DOCK,
  LEFT_DOCK,
  RIGHT_DOCK
};

enum GlobalMaterials {
  DEFAULT_ICON,
  GRIDDED_PILE_BACKGROUND,
  TOOLBAR_TOP,
  TOOLBAR_MIDDLE,
  TOOLBAR_BOTTOM,
  HIGHLIGHT,
  TRANSPARENT_PIXEL,
  STICKY_NOTE_MATERIAL,
  STICKY_NOTE_PAD_MATERIAL,
  NO_STICKIES_LEFT,
  NEW_ITEMS_PILE_ICON
};

class AppSettings : public Settings {
  SINGLETON_HEADER(AppSettings)

 public:
  AppSettings();

  bool saveSettingsFile();
  bool loadSettingsFile();

  QString last_run_bumptop_version();
  void set_last_run_bumptop_version(QString version);

  bool has_auto_hide_dock();
  void set_has_auto_hide_dock(bool has_auto_hide_dock);
  OSXDockPosition dock_position();
  void set_dock_position(OSXDockPosition dock_position);
  void set_global_material_name(GlobalMaterials material, QString material_name);
  QString global_material_name(GlobalMaterials material);
  QStringList image_extensions();

  void updateFinderPreferences();
  bool desktop_shows_hard_disks();
  bool desktop_shows_external_hard_disks();
  bool desktop_shows_removable_media();
  bool desktop_shows_connected_servers();

 protected:
  QString last_run_bumptop_version_;
  bool has_auto_hide_dock_;
  OSXDockPosition dock_position_;
  QHash<GlobalMaterials, QString> global_material_names_;
  QStringList image_extensions_;

  bool desktop_shows_hard_disks_;
  bool desktop_shows_external_hard_disks_;
  bool desktop_shows_removable_media_;
  bool desktop_shows_connected_servers_;
};

#endif  // BUMPTOP_APPSETTINGS_H_
