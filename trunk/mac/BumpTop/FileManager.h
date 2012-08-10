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

#ifndef BUMPTOP_FILEMANAGER_H_
#define BUMPTOP_FILEMANAGER_H_

#include "BumpTop/Singleton.h"
#include "BumpTop/VisualPhysicsActor.h"

enum BehaviorOnConflict {
  kAutomaticallyRenameOnConflict,
  kPromptUserOnConflict
};

enum FileKind {
  VOLUME,
  APPLICATION,
  ALIAS,
  REGULAR_FILE
};

class FileManager {
  SINGLETON_HEADER(FileManager)
 public:
  static void runAppleScriptThroughNSTask(NSString* script);
  static void getAndSetLabelColourThroughNSTask(uint64_t actor_id);
  static void setFinderLabelColourThroughNSTask(QString filename, BumpBoxLabelColour label_colour);
  static BumpBoxLabelColour bumpBoxLabelColourFromFinderLabelColour(int colour);
  static int finderLabelColourFromBumpBoxLabelColour(BumpBoxLabelColour colour);
  static QString getPathsInAppleScriptFormat(QStringList files_paths);
  static bool moveFile(QString file_path, QString new_file_parent_dir, BehaviorOnConflict);
  static bool linkFile(QString file_path, QString new_file_parent_dir);
  static void copyFile(QString file_path, QString new_file_parent_dir, BehaviorOnConflict);
  static void moveFileToTrash(QStringList paths_of_files_to_move_to_trash);
  static void ejectDrive(QString file_path);
  static void duplicateFile(QStringList paths_of_files_to_duplicate);
  static void makeAliasOfFile(QString file_path);
  static void emptyTrash();

  static QString getResourcePath();
  static QString getApplicationDataPath();
  static QString getBackgroundCachePath();
  static QString getApplicationsPath();
  static QString getDesktopPath();
  static QString getUserPath();
  static QString pathForResource(QString resource_name);
  static QString getParentPath(QString path);
  static FileKind getFileKind(QString path);
  static bool isStartupDrive(QString path);
  static QString getStartupDriveName();
  static bool arePathsOnSameVolume(QString path1, QString path2);
  static bool isVolume(QString path);
  static bool isEjectableDrive(QString path);
  static bool isConnectedServer(QString path);
  static void addEjectableDrive(QString path);
  static void addConnectedServer(QString path);
  static void removeConnectedServer(QString path);

  static QStringList pending_alias_names;
#if defined(OS_WIN)
#else
  static QString getDeletedFilesPath();
  static bool launchPath(QString path);
  static bool launchPath(QString path, QString app);
  static bool chooseApplicationAndLaunchPaths(QStringList paths);
  static bool renameFile(QString original_path, QString new_name);
#endif
 protected:
  QString startup_drive_name_;
  QList<QString> ejectable_drives_;
  QList<QString> connected_servers_;
};

#endif  // BUMPTOP_FILEMANAGER_H_
