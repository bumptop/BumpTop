//
//  Copyright 2012 Google Inc. All Rights Reserved.
//  
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  
//      http://www.apache.org/licenses/LICENSE-2.0
//  
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//

#ifdef BUMPTOP_TEST
#include <gtest/gtest.h>
#else
#include "BumpTop/BumpTopInstanceLock.h"
#endif
#include "BumpTop/FileManager.h"

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
#ifdef BUMPTOP_TEST
  ::testing::InitGoogleTest(&argc, argv);
#else
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
  BumpTopInstanceLock desktop_lock(argc, argv);
  desktop_lock.init();
  if (!desktop_lock.tryLock()) {
    exit(0);
  }
  NSString *now = [[[NSDate date] description] stringByReplacingOccurrencesOfString:@" " withString: @"#"];
  NSString *stdout_path = [NSString stringWithFormat:@"%s/stdout-%@.txt", FileManager::getResourcePath().toUtf8().data(), now];
  freopen([stdout_path cStringUsingEncoding:NSUTF8StringEncoding], "w", stdout);

  NSString *stderr_path = [NSString stringWithFormat:@"%s/stderr-%@.txt", FileManager::getResourcePath().toUtf8().data(), now];
  freopen([stderr_path cStringUsingEncoding:NSUTF8StringEncoding], "w", stderr);
  [pool release];
#endif
  return NSApplicationMain(argc,  (const char **) argv);
}
