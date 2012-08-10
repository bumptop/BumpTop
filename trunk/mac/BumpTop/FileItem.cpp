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

#include "BumpTop/FileItem.h"

#include "BumpTop/BumpTopApp.h"
#include "BumpTop/FileManager.h"
#include "BumpTop/OSX/QuickLookSnowLeopard.h"

FileItem::FileItem(QString path) {
  path_ = path;
  if (path_ != "" && path_ != ".") {
    FileSystemEventDispatcher::singleton()->addPathToWatch(path, this, false);
  }
}

FileItem::~FileItem() {
  if (!path_.isNull() && path_ != "")
    FileSystemEventDispatcher::singleton()->removePathToWatch(path_, this);
}

const QString& FileItem::path() {
  return path_;
}

void FileItem::launch() {
#if defined(OS_WIN)
#else
  FileManager::launchPath(path_);
#endif
}

void FileItem::fileAdded(const QString& path) {
  // this should never get called
}

void FileItem::rename(const QString& new_name) {
  FileManager::renameFile(path_, new_name);
}

void FileItem::fileRemoved(const QString& path) {
  if (path == path_) {
    emit onFileRemoved(path);
  }
}

void FileItem::fileRenamed(const QString& old_path, const QString& new_path) {
  if (old_path == path_) {
    path_ = new_path;
    emit onFileRenamed(old_path, new_path);
  }
}

void FileItem::fileModified(const QString& path) {
  if (path == path_) {
    emit onFileModified(path);
  }
}

#include "moc/moc_FileItem.cpp"
