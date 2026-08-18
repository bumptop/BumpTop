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

#include "BumpTop/Processes.h"

#include <signal.h>

#include "BumpTop/QStringHelpers.h"

// Ported from the Carbon Process Manager (GetNextProcess et al., removed from
// modern macOS) to NSWorkspace/NSRunningApplication.

QList<ProcessInfo> listAllProcesses() {
  QList<ProcessInfo> process_list;
  pid_t my_pid = [[NSRunningApplication currentApplication] processIdentifier];
  for (NSRunningApplication* app in [[NSWorkspace sharedWorkspace] runningApplications]) {
    if ([app processIdentifier] == my_pid)
      continue;
    ProcessInfo process_info;
    process_info.name = QStringFromNSString([app localizedName]);
    process_info.psn = [app processIdentifier];
    process_list.push_back(process_info);
  }
  return process_list;
}

pid_t getFinder() {
  NSArray* finders = [NSRunningApplication runningApplicationsWithBundleIdentifier:@"com.apple.finder"];
  if ([finders count] > 0)
    return [(NSRunningApplication*)[finders objectAtIndex:0] processIdentifier];
  return 0;
}

void setFinderToFront() {
  NSArray* finders = [NSRunningApplication runningApplicationsWithBundleIdentifier:@"com.apple.finder"];
  if ([finders count] > 0)
    [(NSRunningApplication*)[finders objectAtIndex:0] activateWithOptions:0];
}

bool isProcessRunning(pid_t psn) {
  if (psn <= 0)
    return false;
  return kill(psn, 0) == 0;
}
