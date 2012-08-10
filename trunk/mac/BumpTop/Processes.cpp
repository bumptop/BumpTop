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

#include "BumpTop/QStringHelpers.h"

QList<ProcessInfo> listAllProcesses() {
  ProcessSerialNumber bumptop_PSN;
  ProcessSerialNumber next_process_PSN = {kNoProcess, kNoProcess};
  OSStatus status;
  QList<ProcessInfo> process_list;
  ProcessInfoRec process_info_record;

  GetCurrentProcess(&bumptop_PSN);
  do {
    status = GetNextProcess(&next_process_PSN);
    if (status == noErr) {
      Boolean process_is_me;
      SameProcess(&bumptop_PSN, &next_process_PSN, &process_is_me);
      if (!process_is_me) {
        process_info_record.processInfoLength = sizeof(ProcessInfoRec);
        process_info_record.processName = new unsigned char[33];
        memset(process_info_record.processName, '\0', 33);
        process_info_record.processAppSpec = NULL;
        if (GetProcessInformation(&next_process_PSN, &process_info_record) == noErr) {
          ProcessInfo process_info;
          process_info.name = QStringFromStringPtr(process_info_record.processName);
          delete[] process_info_record.processName;
          process_info.psn = next_process_PSN;
          process_list.push_back(process_info);
        }
      }
    }
  } while (status == noErr);

  return process_list;
}

ProcessSerialNumber getFinder() {
  QList<ProcessInfo> processes = listAllProcesses();

  for_each(ProcessInfo process_info, processes) {
    if (process_info.name == QString("Finder")) {
      return process_info.psn;
    }
  }
}

void setFinderToFront() {
  ProcessSerialNumber finder_psn = getFinder();
  SetFrontProcessWithOptions(&finder_psn, kSetFrontProcessFrontWindowOnly);
}

bool isProcessRunning(ProcessSerialNumber psn) {
  ProcessInfoRec process_info_record = {0};
  return GetProcessInformation(&psn, &process_info_record) == noErr;
}
