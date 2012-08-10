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

#include "BumpTop/Stopwatch.h"

namespace {

  // The fixture for testing class Foo.
  class StopwatchTest : public ::testing::Test {};

  // Tests that the Foo::Bar() method does Abc.
  TEST_F(StopwatchTest, Stopwatch_measures_time_correctly_SLOW_TEST) {
    Stopwatch s;
    sleep(1);
    EXPECT_NEAR(1000, s.elapsed(), 5);
  }

  // Tests that the Foo::Bar() method does Abc.
  TEST_F(StopwatchTest, Stopwatch_measures_time_correctly_with_pause_SLOW_TEST) {
    Stopwatch s;
    sleep(1);
    EXPECT_NEAR(1000, s.elapsed(), 5);
    s.pause();
    sleep(1);
    EXPECT_NEAR(1000, s.elapsed(), 5);
    s.unpause();
    sleep(1);
    EXPECT_NEAR(2000, s.elapsed(), 5);
  }
}  // namespace

