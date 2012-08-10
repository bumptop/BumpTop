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

#include "BumpTop/Lasso.h"

namespace {
  class LassoTest : public ::testing::Test {
  protected:
    Lasso* lasso;

    virtual void SetUp() {
      lasso = new Lasso();
    }

    virtual void TearDown() {
      delete lasso;
    }
  };

  // See Documentation/Polygon_Test_Cases_1.pdf and Documentation/Polygon_Test_Cases_2.pdf for pictures describing
  // these test cases.
  TEST_F(LassoTest, Polygon_Test_Case_A) {
    lasso->addVertexToLasso(Ogre::Vector2(0, 0));
    lasso->addVertexToLasso(Ogre::Vector2(0, 3));
    lasso->addVertexToLasso(Ogre::Vector2(1, 3));
    lasso->addVertexToLasso(Ogre::Vector2(1, 2));
    lasso->addVertexToLasso(Ogre::Vector2(2, 2));
    lasso->addVertexToLasso(Ogre::Vector2(2, 0));
    bool is_inside = lasso->isPointInPolygon(Ogre::Vector2(1, 1));

    ASSERT_EQ(is_inside, true);
  }

  TEST_F(LassoTest, Polygon_Test_Case_B) {
    lasso->addVertexToLasso(Ogre::Vector2(0, 0));
    lasso->addVertexToLasso(Ogre::Vector2(0, 2));
    lasso->addVertexToLasso(Ogre::Vector2(1, 3));
    lasso->addVertexToLasso(Ogre::Vector2(2, 3));
    lasso->addVertexToLasso(Ogre::Vector2(2, 2));
    lasso->addVertexToLasso(Ogre::Vector2(2, 0));
    bool is_inside = lasso->isPointInPolygon(Ogre::Vector2(1, 1));
    ASSERT_EQ(is_inside, true);
  }

  TEST_F(LassoTest, Polygon_Test_Case_C) {
    lasso->addVertexToLasso(Ogre::Vector2(0, 0));
    lasso->addVertexToLasso(Ogre::Vector2(0, 2));
    lasso->addVertexToLasso(Ogre::Vector2(1, 2));
    lasso->addVertexToLasso(Ogre::Vector2(2, 2));
    lasso->addVertexToLasso(Ogre::Vector2(2, 0));
    bool is_inside = lasso->isPointInPolygon(Ogre::Vector2(1, 1));
    ASSERT_EQ(is_inside, true);
  }

  TEST_F(LassoTest, Polygon_Test_Case_D) {
    lasso->addVertexToLasso(Ogre::Vector2(1, 1));
    lasso->addVertexToLasso(Ogre::Vector2(1, 2));
    lasso->addVertexToLasso(Ogre::Vector2(2, 2));
    lasso->addVertexToLasso(Ogre::Vector2(2, 1));
    bool is_inside = lasso->isPointInPolygon(Ogre::Vector2(1, 0));
    ASSERT_EQ(is_inside, false);
  }

  TEST_F(LassoTest, Polygon_Test_Case_E) {
    lasso->addVertexToLasso(Ogre::Vector2(0, 1));
    lasso->addVertexToLasso(Ogre::Vector2(0, 2));
    lasso->addVertexToLasso(Ogre::Vector2(1, 2));
    lasso->addVertexToLasso(Ogre::Vector2(1, 1));
    bool is_inside = lasso->isPointInPolygon(Ogre::Vector2(1, 0));
    ASSERT_EQ(is_inside, false);
  }

  TEST_F(LassoTest, Polygon_Test_Case_F) {
    lasso->addVertexToLasso(Ogre::Vector2(0, 0));
    lasso->addVertexToLasso(Ogre::Vector2(0, 4));
    lasso->addVertexToLasso(Ogre::Vector2(2, 4));
    lasso->addVertexToLasso(Ogre::Vector2(1, 3));
    lasso->addVertexToLasso(Ogre::Vector2(2, 2));
    lasso->addVertexToLasso(Ogre::Vector2(2, 0));
    bool is_inside = lasso->isPointInPolygon(Ogre::Vector2(1, 1));
    ASSERT_EQ(is_inside, true);
  }

  TEST_F(LassoTest, Polygon_Test_Case_G) {
    lasso->addVertexToLasso(Ogre::Vector2(0, 0));
    lasso->addVertexToLasso(Ogre::Vector2(0, 2));
    lasso->addVertexToLasso(Ogre::Vector2(1, 3));
    lasso->addVertexToLasso(Ogre::Vector2(0, 4));
    lasso->addVertexToLasso(Ogre::Vector2(2, 4));
    lasso->addVertexToLasso(Ogre::Vector2(2, 0));
    bool is_inside = lasso->isPointInPolygon(Ogre::Vector2(1, 1));
    ASSERT_EQ(is_inside, true);
  }

  TEST_F(LassoTest, Polygon_Test_Case_H) {
    lasso->addVertexToLasso(Ogre::Vector2(0, 1));
    lasso->addVertexToLasso(Ogre::Vector2(1, 2));
    lasso->addVertexToLasso(Ogre::Vector2(0, 3));
    lasso->addVertexToLasso(Ogre::Vector2(2, 3));
    lasso->addVertexToLasso(Ogre::Vector2(2, 1));
    bool is_inside = lasso->isPointInPolygon(Ogre::Vector2(1, 0));
    ASSERT_EQ(is_inside, false);
  }
}  // namespace
