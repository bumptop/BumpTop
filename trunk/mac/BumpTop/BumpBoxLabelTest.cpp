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

#include "BumpTop/BumpBoxLabel.h"

namespace {

  // The fixture for testing class Foo.
  class BumpBoxLabelTest : public ::testing::Test {
  protected:
    // You can remove any or all of the following functions if its body
    // is empty.

    BumpBoxLabelTest() {
      // You can do set-up work for each test here.
    }

    virtual ~BumpBoxLabelTest() {
      // You can do clean-up work that doesn't throw exceptions here.
    }

    // If the constructor and destructor are not enough for setting up
    // and cleaning up each test, you can define the following methods:

    virtual void SetUp() {
      // Code here will be called immediately after the constructor (right
      // before each test).
    }

    virtual void TearDown() {
      // Code here will be called immediately after each test (right
      // before the destructor).
    }

    // Objects declared here can be used by all tests in the test case for Foo.
  };

  TEST_F(BumpBoxLabelTest, TestSizingOfText) {
    BumpBoxLabel *label = new BumpBoxLabel("test string");
    label->init();
    EXPECT_EQ(128, label->width());
    EXPECT_EQ(16, label->height());
  }

}  // namespace
