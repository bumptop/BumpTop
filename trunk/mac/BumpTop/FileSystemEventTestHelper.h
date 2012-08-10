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

#include <gtest/gtest.h>

#include "BumpTop/FileSystemEventDispatcher.h"

#ifndef BUMPTOP_FILESYSTEMEVENTTESTHELPER_H_
#define BUMPTOP_FILESYSTEMEVENTTESTHELPER_H_

class BumpTopApp;

class FileSystemEventTest : public FileSystemEventReceiver, public ::testing::Test {
  Q_OBJECT

 protected:
  QDir test_dir_;
  QStringList events_;
  BumpTopApp *bumptop_;

  void resetHandlerFlags();

  void clearTestDirectory();

  virtual void SetUp();
  virtual void TearDown();
  virtual void createFileWithName(QString filename);
  virtual void modifyFileWithName(QString filename);
  virtual void renameFile(QString filename, QString new_filename);
  virtual void removeFile(QString filename);

  QString runEventLoopUntilReceiveEventOfType(QString event_type);
  int numEventsOfType(QString event_type);
 public slots:  // NOLINT
  void fileAdded(const QString& path);
  void fileRenamed(const QString& old_path, const QString& new_path);
  void fileRemoved(const QString& path);
  void fileModified(const QString& path);
};


#endif  // BUMPTOP_FILESYSTEMEVENTTESTHELPER_H_
