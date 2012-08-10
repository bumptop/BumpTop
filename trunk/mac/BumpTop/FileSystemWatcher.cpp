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

#include "BumpTop/FileSystemWatcher.h"

#include <QtCore/QDir>
#include <QtCore/QFileInfo>

#include "BumpTop/for_each.h"
#if defined(OS_MACOSX)
#include "BumpTop/FSEventsWatcher.h"
#endif
#include "BumpTop/QStringHelpers.h"

// Some notes/limitations:
//
// We are listening to FSEventsWatcher because certain actions don't trigger a file modified event for kqueue (which
//  is used by QFileSystemWatcher). Ex: prompt> echo "a" >> test.txt
// However, we also listen to QFileSystemWatcher because kqueue returns events with lower latency (~<100ms vs 1500ms)
//
// If you rename two files to each others names in a short amount of time, this class might not detect any event
//
//

FileSystemWatcher::FileSystemWatcher()
#if defined(OS_MACOSX)
: qfilesystemwatcher_(NULL),
  fsevents_watcher_(NULL),
  last_snapshot_file_paths_(NULL) {
#else
: qfilesystemwatcher_(NULL) {
#endif
}

FileSystemWatcher::~FileSystemWatcher() {
  if (qfilesystemwatcher_ != NULL) {
    delete qfilesystemwatcher_;
  }
#if defined(OS_MACOSX)
  if (fsevents_watcher_ != NULL) {
    delete fsevents_watcher_;
  }
#endif
}

void FileSystemWatcher::init(const QString &path) {
  qfilesystemwatcher_ = new QFileSystemWatcher();
  assert(QObject::connect(qfilesystemwatcher_, SIGNAL(directoryChanged(const QString&)),
                          this, SLOT(onDirectoryChanged(const QString&))));
  addPath(path);
}

void FileSystemWatcher::setLastSnapshot(QStringList* snapshot_file_paths) {
  if (last_snapshot_file_paths_ != NULL) {
    delete last_snapshot_file_paths_;
  }

  last_snapshot_file_paths_ = snapshot_file_paths;
  last_snapshot_file_paths_->sort();
  onDirectoryChanged(path_);
}

void FileSystemWatcher::addPath(const QString &path) {
  path_ = path;
  qfilesystemwatcher_->addPath(path);

  boost::tuple<QStringList*, QHash<QString, ino_t>*, QHash<QString, QDateTime>*> result = getSortedFilePaths(path);
  last_snapshot_file_paths_ = result.get<0>();
  last_snapshot_file_paths_to_inodes_ = result.get<1>();
  last_snapshot_file_paths_to_last_modified_date_ = result.get<2>();
#if defined(OS_MACOSX)
  if (FSEventStreamCreate != NULL) {  // check if FSEvents is supported on this machine first
    fsevents_watcher_ = new FSEventsWatcher(path);
    assert(QObject::connect(fsevents_watcher_, SIGNAL(directoryChanged(const QString&)),
                            this, SLOT(onDirectoryChanged(const QString&))));
  }
#endif
}

void FileSystemWatcher::onDirectoryChanged(const QString& path) {
  if (path != path_)
    return;

  // go through everything that exists so far
  // sort a new list of all the directories
  // go through the new lists:
  // QHash inodes;

  boost::tuple<QStringList*, QHash<QString, ino_t>*, QHash<QString, QDateTime>*> result = getSortedFilePaths(path);
  QStringList *current_file_paths = result.get<0>();
  QHash<QString, ino_t> *current_file_paths_to_inodes = result.get<1>();
  QHash<QString, QDateTime> *current_file_paths_to_last_modified_date = result.get<2>();
  QDir dir(path);

  QHash<int, QString> inode_and_file_paths_of_added_files;
  QHash<int, QString> inode_and_file_paths_of_removed_files;

  QList<QString> file_paths_of_added_files_without_inodes;
  QList<QString> file_paths_of_removed_files_without_inodes;

  int a = 0;
  int b = 0;

  while (a < last_snapshot_file_paths_->count() || b < current_file_paths->count()) {
    if (b >= current_file_paths->count() ||
        a < last_snapshot_file_paths_->count() && current_file_paths->at(b) > last_snapshot_file_paths_->at(a)) {
      QString last_snapshot_file_path = dir.filePath(last_snapshot_file_paths_->at(a));
      if (!(*last_snapshot_file_paths_to_inodes_).contains(last_snapshot_file_path)) {
        file_paths_of_removed_files_without_inodes.append(last_snapshot_file_path);
      } else {
        ino_t inode = (*last_snapshot_file_paths_to_inodes_)[last_snapshot_file_path];

        if (inode_and_file_paths_of_added_files.contains(inode)) {
          QString current_file_path = inode_and_file_paths_of_added_files[inode];
          emit onFileRenamed(last_snapshot_file_path, current_file_path);

          if ((*last_snapshot_file_paths_to_last_modified_date_)[last_snapshot_file_path] !=
              (*current_file_paths_to_last_modified_date)[current_file_path]) {
            emit onFileModified(current_file_path);
          }

          inode_and_file_paths_of_added_files.remove(inode);
        } else {
          inode_and_file_paths_of_removed_files[inode] = last_snapshot_file_path;
        }
      }
      a++;
    } else if (a >= last_snapshot_file_paths_->count() ||
               b < current_file_paths->count() && current_file_paths->at(b) < last_snapshot_file_paths_->at(a)) {
      QString current_file_path = dir.filePath(current_file_paths->at(b));
      if (!(*current_file_paths_to_inodes).contains(current_file_path)) {
        file_paths_of_added_files_without_inodes.append(current_file_path);
      } else {
        ino_t inode = (*current_file_paths_to_inodes)[current_file_path];

        if (inode_and_file_paths_of_removed_files.contains(inode)) {
          QString last_snapshot_file_path = inode_and_file_paths_of_removed_files[inode];
          emit onFileRenamed(last_snapshot_file_path, current_file_path);

          if ((*last_snapshot_file_paths_to_last_modified_date_)[last_snapshot_file_path] !=
              (*current_file_paths_to_last_modified_date)[current_file_path]) {
            emit onFileModified(current_file_path);
          }

          inode_and_file_paths_of_removed_files.remove(inode);
        } else {
          inode_and_file_paths_of_added_files[inode] = current_file_path;
        }
      }
      b++;
    } else {  // if (last_snapshot_file_paths_->at(a) == current_file_paths->at(b)) {
      QString file_path = dir.filePath(current_file_paths->at(b));
      if ((*last_snapshot_file_paths_to_last_modified_date_)[file_path] !=
          (*current_file_paths_to_last_modified_date)[file_path]) {
        emit onFileModified(file_path);
      }

      a++;
      b++;
    }
  }

  for_each(QString added_file_path, inode_and_file_paths_of_added_files.values()) {
    emit onFileAdded(added_file_path);
  }

  for_each(QString added_filepath, file_paths_of_added_files_without_inodes) {
    emit onFileAdded(added_filepath);
  }

  for_each(QString removed_file_path, inode_and_file_paths_of_removed_files.values()) {
    emit onFileRemoved(removed_file_path);
  }

  for_each(QString removed_file_path, file_paths_of_removed_files_without_inodes) {
    emit onFileRemoved(removed_file_path);
  }

  delete last_snapshot_file_paths_;
  delete last_snapshot_file_paths_to_inodes_;
  delete last_snapshot_file_paths_to_last_modified_date_;

  last_snapshot_file_paths_ = current_file_paths;
  last_snapshot_file_paths_to_inodes_ = current_file_paths_to_inodes;
  last_snapshot_file_paths_to_last_modified_date_ = current_file_paths_to_last_modified_date;
}

boost::tuple<QStringList*,
             QHash<QString, ino_t>*,
             QHash<QString, QDateTime>*> FileSystemWatcher::getSortedFilePaths(QString directory_path) {
  QDir dir(directory_path);
  QHash<QString, ino_t> *file_paths_to_inodes = new QHash<QString, ino_t>();
  QHash<QString, QDateTime> *file_paths_to_last_modified_date = new QHash<QString, QDateTime>();

  QStringList* file_paths = new QStringList();
  if (dir.exists())
    for_each(QFileInfo file_info, dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot)) {
      QString file_path = file_info.filePath();
      file_paths->append(file_path);

      struct stat fstat_info;
      stat(utf8(file_path).c_str(), &fstat_info);

      (*file_paths_to_inodes)[file_path] = fstat_info.st_ino;  // inode
      (*file_paths_to_last_modified_date)[file_path] = file_info.lastModified();
      // TODO: what if stat() fails??
    }
  file_paths->sort();

  return boost::tuple<QStringList*, QHash<QString, ino_t>*, QHash<QString, QDateTime>*> (file_paths,
                                                                                        file_paths_to_inodes,
                                                                                        file_paths_to_last_modified_date);  // NOLINT
}

#include "moc/moc_FileSystemWatcher.cpp"
