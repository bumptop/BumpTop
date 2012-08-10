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

#include "BumpTop/FSEventsWatcher.h"

#include "BumpTop/QStringHelpers.h"

static void FSEventsCallback(ConstFSEventStreamRef stream_ref,
                             void *client_callback_info,
                             size_t num_events,
                             void *event_paths,
                             const FSEventStreamEventFlags event_flags[],
                             const FSEventStreamEventId event_ids[]) {
  FSEventsWatcher* fsevents_watcher = static_cast<FSEventsWatcher*>(client_callback_info);
  fsevents_watcher->_directoryChanged(QStringFromUtf8(static_cast<char**>(event_paths)[0]));
}

FSEventsWatcher::FSEventsWatcher(QString path) {
  CFStringRef path_to_watch = CFStringCreateWithCString(kCFAllocatorDefault,
                                                        utf8(path).c_str(),
                                                        kCFStringEncodingUTF8);
  CFArrayRef paths_to_watch = CFArrayCreate(NULL, (const void **)&path_to_watch, 1, NULL);

  context_ = new FSEventStreamContext;
  context_->version = 0;
  context_->info = static_cast<void*>(this);
  context_->retain = NULL;
  context_->release = NULL;
  context_->copyDescription = NULL;

  stream_ = FSEventStreamCreate(kCFAllocatorDefault,
                                &FSEventsCallback,
                                context_,
                                paths_to_watch,
                                kFSEventStreamEventIdSinceNow, /* Or a previous event ID */
                                1.0, /* latency in seconds */
                                kFSEventStreamCreateFlagNone);

  FSEventStreamScheduleWithRunLoop(stream_, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
  FSEventStreamStart(stream_);
}

FSEventsWatcher::~FSEventsWatcher() {
  FSEventStreamStop(stream_);
  FSEventStreamInvalidate(stream_); /* will remove from runloop */
  FSEventStreamRelease(stream_);
  delete context_;
}

void FSEventsWatcher::_directoryChanged(QString path) {
  emit directoryChanged(path);
}

#include "moc/moc_FSEventsWatcher.cpp"
