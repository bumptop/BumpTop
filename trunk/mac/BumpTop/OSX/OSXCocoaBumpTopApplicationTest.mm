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

#include <gtest/gtest.h>

#include "BumpTop/OSX/OSXCocoaBumpTopApplication.h"
#include "BumpTop/QStringHelpers.h"

namespace {
  class OSXCocoaBumpTopApplicationTest : public ::testing::Test {
  };

  TEST_F(OSXCocoaBumpTopApplicationTest, BumpTopApp_is_created) {
    ASSERT_NE((BumpTopApp*)NULL, BumpTopApp::singleton());
  }

  TEST_F(OSXCocoaBumpTopApplicationTest, BumpTopApp_is_OSXCocoaBumpTopApplication) {
    ASSERT_EQ("OSX", BumpTopApp::singleton()->platform());
  }

  TEST_F(OSXCocoaBumpTopApplicationTest, RenderWindow_is_hooked_up) {
    // although we're calling BumpTopApp methods, the relevant code is in
    //  OSXCocoaBumpTopApplicationTest
    ASSERT_NE((Ogre::RenderWindow*)NULL,
              BumpTopApp::singleton()->render_window());
    ASSERT_TRUE(BumpTopApp::singleton()->render_window()->isPrimary());
  }
}  // namespace
