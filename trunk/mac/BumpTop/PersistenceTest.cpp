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

#include "BumpTop/BumpBox.h"
#include "BumpTop/BumpFlatSquare.h"
#include "BumpTop/BumpTopApp.h"
#include "BumpTop/PersistenceManager.h"
#include "BumpTop/Physics.h"
#include "BumpTop/Room.h"
#include "BumpTop/TestHelpers.h"
#include "BumpTop/protoc/AllMessages.pb.h"

namespace {
  class PersistenceTest : public ::testing::Test {
   protected:
    BumpTopApp *app_;
    QDir test_dir_;

    void clearTestDirectory() {
      test_dir_ = getTestDataDirectory();
      test_dir_.setPath(test_dir_.filePath("TestDesktop"));

      if (test_dir_.exists())
        for_each(QFileInfo file_info, test_dir_.entryInfoList())
        test_dir_.remove(file_info.fileName());

      getTestDataDirectory().rmdir(test_dir_.dirName());
      ASSERT_TRUE(!test_dir_.exists());
    }

    virtual void SetUp() {
      app_ = BumpTopApp::singleton();
      clearTestDirectory();
      getTestDataDirectory().mkdir(test_dir_.dirName());
    }

    virtual void TearDown() {
      clearTestDirectory();
      app_->ogre_scene_manager()->clearScene();
    }
  };


  TEST_F(PersistenceTest, ItemNumberTest) {
    Physics* physics = new Physics();
    physics->init();

    Room *room = new Room(app_, NULL, app_->ogre_scene_manager(), physics, test_dir_.absolutePath());
    room->init(app_->window_size().x, app_->window_size().y, 500);

    BumpBox *test_box1 = new BumpFlatSquare(app_->ogre_scene_manager(), physics, room);
    test_box1->init();
    test_box1->set_size(Ogre::Vector3(20, 20, 20));
    test_box1->set_position(Ogre::Vector3(100, 100, 100));
    room->addActor(test_box1);

    BumpBox *test_box2 = new BumpFlatSquare(app_->ogre_scene_manager(), physics, room);
    test_box2->init();
    test_box2->set_size(Ogre::Vector3(20, 20, 20));
    test_box2->set_position(Ogre::Vector3(200, 200, 200));
    room->addActor(test_box2);

    QString test_data_path = getTestDataDirectory().absolutePath();
    writeRoomToFile(room, test_data_path);
    delete room;

    room = new Room(app_, NULL, app_->ogre_scene_manager(), physics, test_dir_.absolutePath());
    loadRoomFromFile(room, test_data_path, Ogre::Vector2(480, 320));

    VisualPhysicsActorList actors = room->room_actor_list();
    int number_of_actors = actors.size();

    ASSERT_EQ(number_of_actors, 2);
    delete room;
    delete physics;
  }

  TEST_F(PersistenceTest, ItemLocationTest) {
    Physics* physics = new Physics();
    physics->init();

    Room *room = new Room(app_, NULL, app_->ogre_scene_manager(), physics, test_dir_.absolutePath());
    room->init(app_->window_size().x, app_->window_size().y, 500);

    VisualPhysicsActor *test_box = new BumpFlatSquare(app_->ogre_scene_manager(), physics, room);
    test_box->init();
    test_box->set_size(Ogre::Vector3(20, 20, 20));
    test_box->set_position(Ogre::Vector3(100, 100, 100));
    room->addActor(test_box);

    QString test_data_path = getTestDataDirectory().absolutePath();
    writeRoomToFile(room, test_data_path);
    delete room;

    room = new Room(app_, NULL, app_->ogre_scene_manager(), physics, test_dir_.absolutePath());
    loadRoomFromFile(room, test_data_path, Ogre::Vector2(480, 320));
    VisualPhysicsActorList actors = room->room_actor_list();

    test_box = actors[0];

    Ogre::Vector3 position_after_load = test_box->position();
    EXPECT_NEAR(100, position_after_load.x, 0.001);
    EXPECT_NEAR(100, position_after_load.y, 0.001);
    EXPECT_NEAR(100, position_after_load.z, 0.001);

    test_box->set_position(Ogre::Vector3(200, 200, 200));
    writeRoomToFile(room, test_data_path);
    delete room;

    room = new Room(app_, NULL, app_->ogre_scene_manager(), physics, test_dir_.absolutePath());
    loadRoomFromFile(room, test_data_path, Ogre::Vector2(480, 320));

    actors = room->room_actor_list();
    test_box = actors[0];
    position_after_load = test_box->position();

    EXPECT_NEAR(200, position_after_load.x, 0.001);
    EXPECT_NEAR(200, position_after_load.y, 0.001);
    EXPECT_NEAR(200, position_after_load.z, 0.001);

    delete physics;
  }

  TEST_F(PersistenceTest, UnicodeTest) {
    ASSERT_EQ(true, true);
  }
}
