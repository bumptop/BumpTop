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

#include <vector>

#include "BumpTop/AnimationManager.h"
#include "BumpTop/BumpBox.h"
#include "BumpTop/BumpTopCommands.h"
#include "BumpTop/VisualPhysicsActorAnimation.h"
#include "BumpTop/VisualPhysicsActor.h"
#include "BumpTop/BumpFlatSquare.h"
#include "BumpTop/BumpTopApp.h"
#include "BumpTop/PersistenceManager.h"
#include "BumpTop/Physics.h"
#include "BumpTop/PhysicsActor.h"
#include "BumpTop/protoc/AllMessages.pb.h"
#include "BumpTop/Room.h"
#include "BumpTop/TestHelpers.h"

namespace {
  class UndoRedoTest : public QObject, public ::testing::Test {
    Q_OBJECT

   protected:
    BumpTopApp *app_;
    QDir test_dir_;
    Physics* physics_;
    Room* room_;
    BumpBox *test_box1_;
    BumpBox *test_box2_;

    void clearTestDirectory() {
      test_dir_ = getTestDataDirectory();
      test_dir_.setPath(test_dir_.filePath("TestDesktop"));

      if (test_dir_.exists()) {
        for_each(QFileInfo file_info, test_dir_.entryInfoList()) {
          test_dir_.remove(file_info.fileName());
        }
      }

      getTestDataDirectory().rmdir(test_dir_.dirName());
      ASSERT_TRUE(!test_dir_.exists());
    }

    virtual void SetUp() {
      app_ = BumpTopApp::singleton();
      clearTestDirectory();
      getTestDataDirectory().mkdir(test_dir_.dirName());
      animation_monitor_ = QHash<VisualPhysicsActor*, bool>();
      AnimationManager* animation_manager = AnimationManager::singleton();

      ASSERT_TRUE(QObject::connect(animation_manager, SIGNAL(onVisualPhysicsActorAnimationComplete(VisualPhysicsActorAnimation*)),  // NOLINT
                                   this, SLOT(VisualPhysicsActorAnimationComplete(VisualPhysicsActorAnimation*)))); // NOLINT

      physics_ = new Physics();
      physics_->init();
      room_ = new Room(app_, NULL, app_->ogre_scene_manager(), physics_, test_dir_.absolutePath());
      room_->init(app_->window_size().x, app_->window_size().y, 500);
    }

    virtual void TearDown() {
      delete room_;
      delete physics_;
      clearTestDirectory();
      app_->ogre_scene_manager()->clearScene();
    }

   public:
    QHash<VisualPhysicsActor*, bool> animation_monitor_;

   public slots: // NOLINT
    void VisualPhysicsActorAnimationComplete(VisualPhysicsActorAnimation* animation) {
      VisualPhysicsActor* sent_actor = animation->visual_physics_actor();
      if (animation_monitor_.contains(sent_actor)) {
          animation_monitor_[sent_actor] = true;
      }
    }

   public:
    void waitForAnimationsToComplete() {
      bool all_animations_done = false;
      while (!all_animations_done) {
        all_animations_done = true;
        app_->renderTick();  // moves the animations along
        for_each(bool done, animation_monitor_.values()) {
          if (!done) {
            all_animations_done = false;
          }
        }
      }
    }
  };


  TEST_F(UndoRedoTest, UndoThenRedoRestoresState) {
    test_box1_ = new BumpFlatSquare(app_->ogre_scene_manager(), physics_, room_);
    test_box1_->init();
    test_box1_->set_size(Ogre::Vector3(20, 20, 20));
    test_box1_->set_position(Ogre::Vector3(100, 20, 100));
    room_->addActor(test_box1_);

    test_box2_ = new BumpFlatSquare(app_->ogre_scene_manager(), physics_, room_);
    test_box2_->init();
    test_box2_->set_size(Ogre::Vector3(20, 20, 20));
    test_box2_->set_position(Ogre::Vector3(200, 20, 200));
    room_->addActor(test_box2_);

    Ogre::Vector3 previous_position1, previous_position2;
    Ogre::Vector3 position1, position2;
    int num_times_position_changed = 0;
    int num_times_stable = 0;
    do {
      previous_position1 = test_box1_->position();
      previous_position2 = test_box2_->position();
      physics_->stepSimulation(1.f/60.f, 10);
      position1 = test_box1_->position();
      position2 = test_box2_->position();
      if (previous_position1 == position1 && previous_position2 == position2) {
        num_times_stable++;
      } else {
        num_times_position_changed++;
      }
    } while (num_times_stable < 10);

    ASSERT_GT(num_times_position_changed, 1);

    // The boxes have fallen onto the floor and are still, this is the initial position of our test; save the state.
    // *State 1*
    Ogre::Vector3 expected_undo_position_box1 = test_box1_->position();
    Ogre::Quaternion expected_undo_orientation_box1 = test_box1_->orientation();
    Ogre::Vector3 expected_undo_position_box2 = test_box2_->position();
    Ogre::Quaternion expected_undo_orientation_box2 = test_box2_->orientation();

    // Move the box by applying an impulse -- simulates a toss by the mouse
    test_box1_->activatePhysics();
    test_box1_->applyCentralImpulse(Ogre::Vector3(30.0, 0, 30.0));
    room_->openNewUndoCommand();

    // Wait until everything settles
    num_times_position_changed = 0;
    num_times_stable = 0;
    do {
      previous_position1 = test_box1_->position();
      previous_position2 = test_box2_->position();
      physics_->stepSimulation(1.f/60.f, 10);
      position1 = test_box1_->position();
      position2 = test_box2_->position();
      if (previous_position1 == position1 && previous_position2 == position2) {
        num_times_stable++;
      } else {
        num_times_position_changed++;
      }
    } while (num_times_stable < 10);

    ASSERT_GT(num_times_position_changed, 1);

    // Everything is settled down, this is our *State 2* from which we will undo, and to which we will redo
    Ogre::Vector3 expected_redo_position_box1 = test_box1_->position();
    Ogre::Quaternion expected_redo_orientation_box1 = test_box1_->orientation();
    Ogre::Vector3 expected_redo_position_box2 = test_box2_->position();
    Ogre::Quaternion expected_redo_orientation_box2 = test_box2_->orientation();

    // Undo to *State 1* and verify that it everything is how it should be
    app_->renderTick();  // this is dirty, but we need to give

    BumpEnvironment env = BumpEnvironment(physics_, room_, app_->ogre_scene_manager());
    Undo::singleton()->execute(env);

    animation_monitor_[test_box1_] = false;
    animation_monitor_[test_box2_] = true;
    waitForAnimationsToComplete();

    ASSERT_NEAR(test_box1_->position().x, expected_undo_position_box1.x, 0.0001);
    ASSERT_NEAR(test_box1_->position().y, expected_undo_position_box1.y, 0.0001);
    ASSERT_NEAR(test_box1_->position().z, expected_undo_position_box1.z, 0.0001);
    ASSERT_NEAR(test_box1_->orientation().w, expected_undo_orientation_box1.w, 0.0001);
    ASSERT_NEAR(test_box1_->orientation().x, expected_undo_orientation_box1.x, 0.0001);
    ASSERT_NEAR(test_box1_->orientation().y, expected_undo_orientation_box1.y, 0.0001);
    ASSERT_NEAR(test_box1_->orientation().z, expected_undo_orientation_box1.z, 0.0001);

    ASSERT_NEAR(test_box2_->position().x, expected_undo_position_box2.x, 0.0001);
    ASSERT_NEAR(test_box2_->position().y, expected_undo_position_box2.y, 0.0001);
    ASSERT_NEAR(test_box2_->position().z, expected_undo_position_box2.z, 0.0001);
    ASSERT_NEAR(test_box2_->orientation().w, expected_undo_orientation_box2.w, 0.0001);
    ASSERT_NEAR(test_box2_->orientation().x, expected_undo_orientation_box2.x, 0.0001);
    ASSERT_NEAR(test_box2_->orientation().y, expected_undo_orientation_box2.y, 0.0001);
    ASSERT_NEAR(test_box2_->orientation().z, expected_undo_orientation_box2.z, 0.0001);


    // Check that the objects aren't moving
    ASSERT_EQ((test_box1_->linear_velocity()).dotProduct(Ogre::Vector3(1, 1, 1)), 0);
    ASSERT_EQ((test_box1_->angular_velocity()).dotProduct(Ogre::Vector3(1, 1, 1)), 0);
    ASSERT_EQ((test_box2_->linear_velocity()).dotProduct(Ogre::Vector3(1, 1, 1)), 0);
    ASSERT_EQ((test_box2_->angular_velocity()).dotProduct(Ogre::Vector3(1, 1, 1)), 0);

    // Redo to *State 2* and verify that everything is as it should be
    Redo::singleton()->execute(env);
    animation_monitor_[test_box1_] = false;
    animation_monitor_[test_box2_] = true;
    waitForAnimationsToComplete();

    ASSERT_NEAR(test_box1_->position().x, expected_redo_position_box1.x, 0.0001);
    ASSERT_NEAR(test_box1_->position().y, expected_redo_position_box1.y, 0.0001);
    ASSERT_NEAR(test_box1_->position().z, expected_redo_position_box1.z, 0.0001);
    ASSERT_NEAR(test_box1_->orientation().w, expected_redo_orientation_box1.w, 0.0001);
    ASSERT_NEAR(test_box1_->orientation().x, expected_redo_orientation_box1.x, 0.0001);
    ASSERT_NEAR(test_box1_->orientation().y, expected_redo_orientation_box1.y, 0.0001);
    ASSERT_NEAR(test_box1_->orientation().z, expected_redo_orientation_box1.z, 0.0001);

    ASSERT_NEAR(test_box2_->position().x, expected_redo_position_box2.x, 0.0001);
    ASSERT_NEAR(test_box2_->position().y, expected_redo_position_box2.y, 0.0001);
    ASSERT_NEAR(test_box2_->position().z, expected_redo_position_box2.z, 0.0001);
    ASSERT_NEAR(test_box2_->orientation().w, expected_redo_orientation_box2.w, 0.0001);
    ASSERT_NEAR(test_box2_->orientation().x, expected_redo_orientation_box2.x, 0.0001);
    ASSERT_NEAR(test_box2_->orientation().y, expected_redo_orientation_box2.y, 0.0001);
    ASSERT_NEAR(test_box2_->orientation().z, expected_redo_orientation_box2.z, 0.0001);

    // Check that the objects aren't moving
    ASSERT_EQ((test_box1_->linear_velocity()).dotProduct(Ogre::Vector3(1, 1, 1)), 0);
    ASSERT_EQ((test_box1_->angular_velocity()).dotProduct(Ogre::Vector3(1, 1, 1)), 0);
    ASSERT_EQ((test_box2_->linear_velocity()).dotProduct(Ogre::Vector3(1, 1, 1)), 0);
    ASSERT_EQ((test_box2_->angular_velocity()).dotProduct(Ogre::Vector3(1, 1, 1)), 0);
  }
}

#include "BumpTop/moc/moc_UndoRedoTest.cpp";
