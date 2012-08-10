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

#ifndef BUMPTOP_BUMPTOPINSTANCELOCK_H_
#define BUMPTOP_BUMPTOPINSTANCELOCK_H_

#include <utility>

#include "BumpTop/FileSystemEventDispatcher.h"

struct ProcessInfo;
class QPainterMaterial;

class BumpTopInstanceLock : public FileSystemEventReceiver {
 public:
  BumpTopInstanceLock(int argc, char *argv[]);
  virtual ~BumpTopInstanceLock();
  void init();
  bool tryLock();
  void clearSandboxLock();

  virtual void fileAdded(const QString& path);
  virtual void fileRemoved(const QString& path);
  virtual void fileRenamed(const QString& old_path, const QString& new_path);
  virtual void fileModified(const QString& path);
  static bool is_running_in_sandbox();
 protected:
  std::pair<bool, ProcessInfo> isOtherInstanceOfBumpTopRunning();
  void blockUntilOtherInstanceOfBumpTopDies();
  QtLockedFile* locked_file_;
  static bool is_running_in_sandbox_;
  int argc_;
  char *argv_[];
};

class RedDot : public QObject {
  Q_OBJECT

 public:
  RedDot();
  virtual ~RedDot();

  void init();

  public slots:  // NOLINT
  void drawRedDot(QPainter* painter);
 protected:
  QPainterMaterial* red_dot_material_;
};

#endif  // BUMPTOP_BUMPTOPINSTANCELOCK_H_
