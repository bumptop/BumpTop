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

#include <utility>

#include "BumpTop/StaticBox.h"
#include "BumpTop/BumpTopApp.h"
#include "BumpTop/OSX/EventModifierFlags.h"
#include "BumpTop/MouseEventManager.h"
#include "BumpTop/Physics.h"
#include "BumpTop/RoomSurface.h"
#include "BumpTop/VisualActor.h"

namespace {
  class RoomSurfaceTest : public QObject, public ::testing::Test {
    Q_OBJECT

   public slots:  // NOLINT
    void mouseHandler(MouseEvent* mouse_event) {
      mouse_in_window_space_ = mouse_event->mouse_in_window_space;
      mouse_in_world_space_ = mouse_event->mouse_in_world_space;
    }

   protected:
    BumpTopApp *bumptop_;
    Ogre::SceneManager *scene_manager_;
    Physics *physics_;
    Ogre::Vector2 mouse_in_window_space_;
    Ogre::Vector3 mouse_in_world_space_;
    RoomSurface *room_surface_;

    virtual void SetUp() {
      bumptop_ = BumpTopApp::singleton();
      scene_manager_ = BumpTopApp::singleton()->ogre_scene_manager();

      physics_ = new Physics();
      physics_->init();

      mouse_in_window_space_ = Ogre::Vector2::ZERO;
      mouse_in_world_space_ = Ogre::Vector3::ZERO;

      room_surface_ = new RoomSurface(scene_manager_, physics_, NULL, "", NOT_PINNABLE_RECEIVER);
      room_surface_->init(Ogre::Vector3(0, 1, 0), 100, 1000);
      room_surface_->set_position(Ogre::Vector3(0, -100, -125));
    }

    virtual void TearDown() {
      scene_manager_->clearScene();
      delete physics_;
    }
  };

  TEST_F(RoomSurfaceTest, Test_intersect_results_are_correct) {
    float simulated_mouse_x = 236.0;
    float simulated_mouse_y = 301.0;

    Ogre::Vector2 mouse_point(simulated_mouse_x, simulated_mouse_y);
    std::pair<bool, Ogre::Vector3> intersect_result = room_surface_->mouseIntersection(mouse_point);

    ASSERT_TRUE(intersect_result.first);

    // Test what the room_surface_ gets, make sure its the same
    ASSERT_TRUE(QObject::connect(room_surface_->visual_actor(), SIGNAL(onMouseDown(MouseEvent*)),  // NOLINT
                                 this, SLOT(mouseHandler(MouseEvent*))));  // NOLINT

    // it seems that we need to call render to either update the position of the cube,
    //    or to register it with the scene in the first place
    bumptop_->renderTick();
    bumptop_->mouseDown(simulated_mouse_x, simulated_mouse_y, 1, NO_KEY_MODIFIERS_MASK);

    Ogre::Vector3 point = intersect_result.second;
    EXPECT_NEAR(mouse_in_world_space_.x, point.x, 0.001);
    EXPECT_NEAR(mouse_in_world_space_.y, point.y, 0.001);
    EXPECT_NEAR(mouse_in_world_space_.z, point.z, 0.001);
  }

  TEST_F(RoomSurfaceTest, Test_intersect_above) {
    float simulated_mouse_x = 236.0;
    float simulated_mouse_y = 301.0;

    Ogre::Vector2 mouse_point(simulated_mouse_x, simulated_mouse_y);
    std::pair<bool, Ogre::Vector3> intersect_result = room_surface_->mouseIntersectionAbove(mouse_point, 5);
    ASSERT_TRUE(intersect_result.first);

    Ogre::Vector3 point = intersect_result.second;
    EXPECT_NEAR(-3.124, point.x, 0.001);
    EXPECT_NEAR(-94.5, point.y, 0.001);
    EXPECT_NEAR(-243.489, point.z, 0.001);
  }

  TEST_F(RoomSurfaceTest, Test_intersect_outside_the_bounds) {
    float simulated_mouse_x = 296.0;
    float simulated_mouse_y = 287.0;

    Ogre::Vector2 mouse_point(simulated_mouse_x, simulated_mouse_y);
    std::pair<bool, Ogre::Vector3> intersect_result = room_surface_->mouseIntersection(mouse_point);
    ASSERT_FALSE(intersect_result.first);
  }
}  // namespace

#include "BumpTop/moc/moc_RoomSurfaceTest.cpp";
