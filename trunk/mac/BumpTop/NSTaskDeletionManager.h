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

#ifndef BUMPTOP_NSTASKDELETIONMANAGER_H_
#define BUMPTOP_NSTASKDELETIONMANAGER_H_

#include "BumpTop/Singleton.h"

class NSTaskDeletionManager : public QObject {
  Q_OBJECT
  SINGLETON_HEADER(NSTaskDeletionManager)

 public:
  virtual ~NSTaskDeletionManager();
  void add_task_to_delete(NSTask* task);

 public slots:  // NOLINT
  virtual void delete_tasks();

 protected:
  QList<NSTask*> tasks_to_delete_;
};

#endif  // BUMPTOP_NSTASKDELETIONMANAGER_H_
