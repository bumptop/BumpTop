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

#include "BumpTop/FileSystemEventDispatcher.h"

#include "BumpTop/FileManager.h"
#include "BumpTop/FileSystemWatcher.h"

#include "BumpTop/QStringHelpers.h"


SINGLETON_IMPLEMENTATION(FileSystemEventDispatcher)

FileSystemEventDispatcher::FileSystemEventDispatcher() {
}

FileSystemEventDispatcher::~FileSystemEventDispatcher() {
}

bool FileSystemEventDispatcher::addPathToWatch(const QString& raw_path, FileSystemEventReceiver* receiver,
                                               bool emit_events_for_children) {
  // test what happens when you add/remove a slash at the end
  QFileInfo path_info = QFileInfo(raw_path);
  if (!path_info.exists() || raw_path == "" || raw_path == ".") {
    return false;
  }

  // canonical file path for an alias will get path of file it is
  // referencing to, so here we want to use absolute path instead.
  QString path = getPathWithoutTrailingSlash(path_info.absoluteFilePath());
  QString parent_path = FileManager::getParentPath(path);
  if (path_info.isFile() && !file_system_watchers_.contains(parent_path)) {
    return false;
  }

  if (file_paths_being_watched_.contains(path)) {
    return false;
  }

  if (path_info.isDir() && emit_events_for_children) {
    if (file_system_watchers_.contains(path)) {
      return false;
    }
    FileSystemWatcher* new_file_system_watcher = new FileSystemWatcher();
    new_file_system_watcher->init(path);
    file_system_watchers_[path] = new_file_system_watcher;
    assert(QObject::connect(new_file_system_watcher, SIGNAL(onFileAdded(const QString&)),
                            this, SLOT(fileAdded(const QString&))));
    assert(QObject::connect(new_file_system_watcher, SIGNAL(onFileRemoved(const QString&)),
                            this, SLOT(fileRemoved(const QString&))));
    assert(QObject::connect(new_file_system_watcher, SIGNAL(onFileModified(const QString&)),
                            this, SLOT(fileModified(const QString&))));
    assert(QObject::connect(new_file_system_watcher, SIGNAL(onFileRenamed(const QString&, const QString&)),
                            this, SLOT(fileRenamed(const QString&, const QString&))));
  }

  file_paths_being_watched_[path] = receiver;
  return true;
}

bool FileSystemEventDispatcher::removePathToWatch(const QString& raw_path, FileSystemEventReceiver* receiver) {
  QFileInfo path_info = QFileInfo(raw_path);
  if (!path_info.exists())
    return false;
  QString path = getPathWithoutTrailingSlash(path_info.absoluteFilePath());

  if (!file_paths_being_watched_.contains(path) ||
      file_paths_being_watched_[path] != receiver) {
    return false;
  }

  file_paths_being_watched_.remove(path);

  if (QFileInfo(path).isDir()) {
    if (file_system_watchers_.contains(path)) {
      file_system_watchers_.remove(path);
    }
  }
  return true;
}

void FileSystemEventDispatcher::removePathToWatchOnFileDeletion(const QString& path) {
  if (!file_paths_being_watched_.contains(path)) {
    return;
  }
  file_paths_being_watched_.remove(path);

  if (QFileInfo(path).isDir()) {
    if (file_system_watchers_.contains(path)) {
      file_system_watchers_.remove(path);
    }
  }
}

bool FileSystemEventDispatcher::generateFileSystemEventsSinceLastSnapshot(const QString& raw_path,
                                                                          QStringList* subpaths_in_last_snapshot) {
  QFileInfo path_info = QFileInfo(raw_path);
  if (!path_info.exists()) {
    return false;
  }
  QString path = getPathWithoutTrailingSlash(path_info.absoluteFilePath());
  if (!file_system_watchers_.contains(path)) {
    return false;
  }

  file_system_watchers_[path]->setLastSnapshot(subpaths_in_last_snapshot);
  return true;
}

void FileSystemEventDispatcher::fileAdded(const QString& path) {
  QString parent_path = FileManager::getParentPath(path);
  if (file_paths_being_watched_.contains(parent_path)) {
    file_paths_being_watched_[parent_path]->fileAdded(path);
  }
}

void FileSystemEventDispatcher::fileRemoved(const QString& path) {
  if (file_paths_being_watched_.contains(path)) {
    file_paths_being_watched_[path]->fileRemoved(path);
    file_paths_being_watched_.remove(path);
  }

  QString parent_path = FileManager::getParentPath(path);
  if (file_paths_being_watched_.contains(parent_path)) {
    file_paths_being_watched_[parent_path]->fileRemoved(path);
  }
}

void FileSystemEventDispatcher::fileModified(const QString& path) {
  if (file_paths_being_watched_.contains(path)) {
    file_paths_being_watched_[path]->fileModified(path);
  }

  QString parent_path = FileManager::getParentPath(path);
  if (file_paths_being_watched_.contains(parent_path)) {
    file_paths_being_watched_[parent_path]->fileModified(path);
  }
}

void FileSystemEventDispatcher::fileRenamed(const QString& old_path, const QString& new_path) {
  if (file_paths_being_watched_.contains(old_path)) {
    file_paths_being_watched_[old_path]->fileRenamed(old_path, new_path);
    file_paths_being_watched_[new_path] = file_paths_being_watched_[old_path];
    file_paths_being_watched_.remove(old_path);
  }

  QString parent_path = FileManager::getParentPath(old_path);
  if (file_paths_being_watched_.contains(parent_path)) {
    file_paths_being_watched_[parent_path]->fileRenamed(old_path, new_path);
  }
}

QString FileSystemEventDispatcher::getPathWithoutTrailingSlash(QString path) {
  if (path.endsWith("/")) {
    return getPathWithoutTrailingSlash(path.left(path.count() - 1));
  } else {
    return path;
  }
}

#include "moc/moc_FileSystemEventDispatcher.cpp"
