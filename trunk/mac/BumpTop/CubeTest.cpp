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

#include "BumpTop/BumpTopApp.h"
#include "BumpTop/Cube.h"
#include "BumpTop/Physics.h"

namespace {
  class CubeTest : public ::testing::Test {
  protected:
    Cube *cube_;
    Ogre::SceneManager *scene_manager_;
    Physics *physics_;

    virtual void SetUp() {
      physics_ = new Physics();
      physics_->init();

      scene_manager_ = BumpTopApp::singleton()->ogre_scene_manager();
      cube_ = new Cube(scene_manager_, physics_, NULL);
      cube_->init();
    }

    virtual void TearDown() {
      scene_manager_->clearScene();
      delete physics_;
    }
  };

  TEST_F(CubeTest, Cube_scene_node_is_created) {
    Ogre::String scene_node_name = "VisualActor" +
                                    Ogre::StringConverter::toString(reinterpret_cast<int>(cube_->visual_actor()));
    ASSERT_EQ(cube_->ogre_scene_node(),
              scene_manager_->getSceneNode(scene_node_name));
  }

  TEST_F(CubeTest, Cube_size_is_set_correctly) {
    cube_->set_size(5.0);
    EXPECT_EQ(Ogre::Vector3(5.0, 5.0, 5.0), cube_->size());
  }

  TEST_F(CubeTest, Cube_default_position) {
    ASSERT_EQ(Ogre::Vector3(0, 0, 0), cube_->position());
    ASSERT_EQ(Ogre::Quaternion(1, 0, 0, 0), cube_->orientation());
  }

  TEST_F(CubeTest, Cube_entity_is_created) {
    Ogre::SceneNode* scene_node = cube_->ogre_scene_node();
    ASSERT_EQ(1, scene_node->numAttachedObjects());

    Ogre::MovableObject* movable_object = scene_node->getAttachedObject(0);
    ASSERT_EQ("Entity", movable_object->getMovableType());

    Ogre::AxisAlignedBox bounding_box = movable_object->getBoundingBox();
    ASSERT_EQ(Ogre::Vector3(51, 51, 51), bounding_box.getMaximum());
    ASSERT_EQ(Ogre::Vector3(-51, -51, -51), bounding_box.getMinimum());
  }


}  // namespace
