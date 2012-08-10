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

#ifndef BUMPTOP_FILESYSTEMEVENTDISPATCHER_H_
#define BUMPTOP_FILESYSTEMEVENTDISPATCHER_H_

#include "BumpTop/Singleton.h"

class FileSystemWatcher;

class FileSystemEventReceiver : public QObject {
  Q_OBJECT

 public slots:  // NOLINT
  virtual void fileAdded(const QString& path) = 0;
  virtual void fileRemoved(const QString& path) = 0;
  virtual void fileRenamed(const QString& old_path, const QString& new_path) = 0;
  virtual void fileModified(const QString& path) = 0;
};


class FileSystemEventDispatcher : public QObject {
  Q_OBJECT
  SINGLETON_HEADER(FileSystemEventDispatcher)

 public:
  ~FileSystemEventDispatcher();

  bool generateFileSystemEventsSinceLastSnapshot(const QString& path, QStringList* subpaths_in_last_snapshot);
  bool addPathToWatch(const QString& path, FileSystemEventReceiver* receiver, bool emit_events_for_children);
  bool removePathToWatch(const QString& path, FileSystemEventReceiver* receiver);
  void removePathToWatchOnFileDeletion(const QString& path);
 public slots:  // NOLINT
  virtual void fileAdded(const QString& path);
  virtual void fileRemoved(const QString& path);
  virtual void fileModified(const QString& path);
  virtual void fileRenamed(const QString& old_path, const QString& new_path);

 protected:
  explicit FileSystemEventDispatcher();
  QString getPathWithoutTrailingSlash(QString path);

  QHash<QString, FileSystemWatcher*> file_system_watchers_;
  QHash<QString, FileSystemEventReceiver*> file_paths_being_watched_;
};

#endif  // BUMPTOP_FILESYSTEMEVENTDISPATCHER_H_
