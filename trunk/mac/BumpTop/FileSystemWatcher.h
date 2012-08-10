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

#ifndef BUMPTOP_FILESYSTEMWATCHER_H_
#define BUMPTOP_FILESYSTEMWATCHER_H_

#include <sys/stat.h>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QHash>
#include <QtCore/QDateTime>
#include <QtCore/QFileSystemWatcher>

#if defined(OS_MACOSX)
class FSEventsWatcher;
#endif

class FileSystemWatcher : public QObject {
  Q_OBJECT

 public:
  FileSystemWatcher();
  ~FileSystemWatcher();

  void init(const QString& path);
  void setLastSnapshot(QStringList* old_paths);
 signals:
  void onFileAdded(const QString& path);
  void onFileRemoved(const QString& path);
  void onFileRenamed(const QString& old_path, const QString& new_path);
  void onFileModified(const QString& path);
 public slots: // NOLINT
  void onDirectoryChanged(const QString& path);
 protected:
  void addPath(const QString& path);

  QString path_;
  QFileSystemWatcher *qfilesystemwatcher_;
  QStringList *last_snapshot_file_paths_;
  QHash<QString, ino_t> *last_snapshot_file_paths_to_inodes_;
  QHash<QString, QDateTime> *last_snapshot_file_paths_to_last_modified_date_;
#if defined(OS_MACOSX)
  FSEventsWatcher *fsevents_watcher_;
#endif

  boost::tuple<QStringList*,
               QHash<QString, ino_t>*,
               QHash<QString, QDateTime>*> getSortedFilePaths(QString directory_path);  // NOLINT
};

#endif  // BUMPTOP_FILESYSTEMWATCHER_H_
