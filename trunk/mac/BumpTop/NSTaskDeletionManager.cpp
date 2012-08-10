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

#include "BumpTop/NSTaskDeletionManager.h"

#include "BumpTop/BumpTopApp.h"

SINGLETON_IMPLEMENTATION(NSTaskDeletionManager)

NSTaskDeletionManager::~NSTaskDeletionManager() {
}

void NSTaskDeletionManager::add_task_to_delete(NSTask* task) {
  if (!tasks_to_delete_.contains(task)) {
    // guard to take only unique tasks to avoid getting the same task more than once
    tasks_to_delete_.append(task);
    // hook up to the rendertick and releases the tasks on next render tick
    assert(QObject::connect(BumpTopApp::singleton(), SIGNAL(onRender()),  // NOLINT
                            this, SLOT(delete_tasks())));  // NOLINT
  }
}

void NSTaskDeletionManager::delete_tasks() {
  // release all completed tasks from last render tick
  for_each(NSTask* task, tasks_to_delete_) {
    [task release];
  }
  // reset tasks_to_delete_ to an empty list
  tasks_to_delete_.clear();

  BumpTopApp::singleton()->disconnect(this);
}

#include "moc/moc_NSTaskDeletionManager.cpp"
