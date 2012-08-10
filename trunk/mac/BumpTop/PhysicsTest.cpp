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
#include "BumpTop/StaticBox.h"
#include "BumpTop/PhysicsActor.h"

namespace {
  class PhysicsTest : public ::testing::Test {
   protected:
    BumpTopApp *bumptop_;

    virtual void SetUp() {
      bumptop_ = BumpTopApp::singleton();
    }

    virtual void TearDown() {
      bumptop_->ogre_scene_manager()->clearScene();
    }
  };

  TEST_F(PhysicsTest, Physics_exists) {
    Physics *physics = new Physics();

    ASSERT_NE(reinterpret_cast<Physics*>(NULL), physics);
    physics->init();
    delete physics;
  }

  TEST_F(PhysicsTest, Initialization_works) {
    Physics *physics = new Physics();

    ASSERT_EQ(NULL, physics->dynamics_world());

    physics->init();
    ASSERT_NE(reinterpret_cast<btDiscreteDynamicsWorld*>(NULL), physics->dynamics_world());
    delete physics;
  }

  TEST_F(PhysicsTest, Physics_meshes_with_other_stuff) {
    Physics *physics = new Physics();
    physics->init();

    // Create cube
    Cube cube(bumptop_->ogre_scene_manager(), physics, NULL);
    cube.init();
    cube.set_position(Ogre::Vector3(0, 3, 0));
    cube.set_size(2);

    // Create floor
    StaticBox floor(bumptop_->ogre_scene_manager(), physics, NULL);
    floor.init();
    floor.set_size(Ogre::Vector3(1000, 1, 1000));
    cube.applyCentralImpulse(Ogre::Vector3(1.0, 0.0, 0.0));

    Ogre::Vector3 previous_position, position;
    int num_times_position_changed = -1;
    do {
      previous_position = cube.position();
      physics->stepSimulation(1.f/60.f, 10);
      position = cube.position();
      num_times_position_changed++;
    } while (previous_position != position);

    ASSERT_GT(num_times_position_changed, 0);
    EXPECT_NEAR(0.55, cube.position().x, 0.001);
    EXPECT_NEAR(1.5, cube.position().y, 0.001);
    EXPECT_NEAR(0.0, cube.position().z, 0.001);

    cube.set_position(Ogre::Vector3(2, 1.5, 2));
    EXPECT_NEAR(2, cube.position().x, 0.001);
    EXPECT_NEAR(1.5, cube.position().y, 0.001);
    EXPECT_NEAR(2, cube.position().z, 0.001);

    position = cube.position();
    physics->stepSimulation(1.f/60.f, 10);  // do this a few times
    physics->stepSimulation(1.f/60.f, 10);
    physics->stepSimulation(1.f/60.f, 10);
    physics->stepSimulation(1.f/60.f, 10);
    physics->stepSimulation(1.f/60.f, 10);

    EXPECT_NEAR(2, cube.position().x, 0.001);
    EXPECT_NEAR(1.5, cube.position().y, 0.001);
    EXPECT_NEAR(2, cube.position().z, 0.001);

    cube.set_position(Ogre::Vector3(0, 10.0, 0));
    ASSERT_EQ(Ogre::Vector3(0, 10.0, 0), cube.position());

    num_times_position_changed = -1;
    do {
      previous_position = cube.position();
      physics->stepSimulation(1.f/60.f, 10);
      position = cube.position();
      num_times_position_changed++;
    } while (previous_position != position);

    ASSERT_GT(num_times_position_changed, 1);
  }
}  // namespace

