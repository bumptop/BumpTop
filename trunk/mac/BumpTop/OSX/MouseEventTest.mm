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

#include "BumpTop/BumpTopApp.h"
#include "BumpTop/Cube.h"
#include "BumpTop/OSX/EventModifierFlags.h"
#include "BumpTop/MouseEventManager.h"
#include "BumpTop/OSX/OSXCocoaBumpTopApplication.h"
#include "BumpTop/Physics.h"
#include "BumpTop/QStringHelpers.h"
#include "BumpTop/VisualActor.h"

// Testing two things:
// 1. OSXCocoaBumpTopApplication::processOneEvent(int num_milliseconds_to_wait)
// 2. Getting mouse events from the system
//
// If these tests fail, either (1) or (2) could have failed

namespace {
  class MouseEventTest : public QObject, public ::testing::Test {
    Q_OBJECT

   public slots:
    void mouseHandler(float x, float y, int num_clicks, int modifier_flags) {
      x_ = x;
      y_ = y;
    }
    void mouseHandler(MouseEvent* mouse_event) {
      x_ = mouse_event->mouse_in_window_space.x;
      y_ = mouse_event->mouse_in_window_space.y;
      mouse_event->handled = true;
    }

   protected:
    BumpTopApp *bumptop_;
    Physics *physics_;
    float x_;
    float y_;

    virtual void SetUp() {
      bumptop_ = BumpTopApp::singleton();
      x_ = 0;
      y_ = 0;

      physics_ = new Physics();
      physics_->init();
    }

    virtual void TearDown() {
      bumptop_->ogre_scene_manager()->clearScene();
      delete physics_;
    }

    NSEvent *createMouseEvent(NSEventType event_type, float x, float y) {
      NSEvent *fake_mouse_event =
      [NSEvent mouseEventWithType: event_type
                         location: NSMakePoint(x, y)
                    modifierFlags:nil
                        timestamp:nil
                     windowNumber:[[OSXCocoaBumpTopApplication::singleton()->ogre_view() window] windowNumber]
                          context:nil
                      eventNumber:nil
                       clickCount:0
                         pressure:nil];
      return fake_mouse_event;
    }
  };

  TEST_F(MouseEventTest, Test_that_we_are_on_mac) {
      ASSERT_EQ("OSX", bumptop_->platform());
  }

  TEST_F(MouseEventTest, Test_that_create_event_is_not_null) {
    NSEvent *fake_mouse_event = createMouseEvent(NSLeftMouseDown, 240, 180);
    ASSERT_NE((NSEvent*)nil, fake_mouse_event);
  }

  TEST_F(MouseEventTest, Mouse_event_manager_is_created) {
    ASSERT_NE((MouseEventManager*)NULL, bumptop_->mouse_event_manager());
  }

  TEST_F(MouseEventTest, Event_loop_eats_an_event) {
    NSEvent *current_event_before = [[NSApplication sharedApplication] currentEvent];

    // send an event
    NSEvent *fake_mouse_event = createMouseEvent(NSLeftMouseDown, 20, 20);
    [[NSApplication sharedApplication] postEvent: fake_mouse_event atStart: YES];
    bumptop_->processOneEvent();

    // check the current event has changed
    NSEvent *current_event_after = [[NSApplication sharedApplication] currentEvent];

    ASSERT_NE(current_event_before, current_event_after);
  }

  TEST_F(MouseEventTest, Mouse_down_event_gets_propagated) {
    // register ourselves to be notified of the event
    ASSERT_TRUE(QObject::connect(bumptop_, SIGNAL(onMouseDown(float, float, int, int)),
                                 this, SLOT(mouseHandler(float, float, int, int))));

    // send the event
    NSEvent *fake_mouse_event = createMouseEvent(NSLeftMouseDown, 20, 20);
    [[NSApplication sharedApplication] postEvent: fake_mouse_event atStart: YES];
    bumptop_->processOneEvent();

    EXPECT_EQ(20, x_);
    EXPECT_EQ(340, y_);
  }

  TEST_F(MouseEventTest, Mouse_dragged_event_gets_propagated) {
    // register ourselves to be notified of the event
    ASSERT_TRUE(QObject::connect(bumptop_, SIGNAL(onMouseDragged(float, float, int, int)),
                                 this, SLOT(mouseHandler(float, float, int, int))));

    // send the event
    NSEvent *fake_mouse_event2 = createMouseEvent(NSLeftMouseDragged, 20, 20);
    [[NSApplication sharedApplication] postEvent: fake_mouse_event2 atStart: YES];
    bumptop_->processOneEvent();

    EXPECT_EQ(20, x_);
    EXPECT_EQ(340, y_);
  }

  TEST_F(MouseEventTest, Mouse_up_event_gets_propagated) {
    // register ourselves to be notified of the event
    ASSERT_TRUE(QObject::connect(bumptop_, SIGNAL(onMouseUp(float, float, int, int)),
                                 this, SLOT(mouseHandler(float, float, int, int))));

    // send the event
    NSEvent *fake_mouse_event = createMouseEvent(NSLeftMouseUp, 20, 20);
    [[NSApplication sharedApplication] postEvent: fake_mouse_event atStart: YES];
    bumptop_->processOneEvent();

    EXPECT_EQ(20, x_);
    EXPECT_EQ(340, y_);
  }



  TEST_F(MouseEventTest, Mouse_down_event_gets_projected) {
    // we'll be testing to see if this Cube is hit (it should be)
    Cube *cube = new Cube(bumptop_->ogre_scene_manager(), physics_, NULL);
    cube->init();
    cube->ogre_scene_node()->setPosition(0, 0, -200);
    cube->ogre_scene_node()->scale(Ogre::Vector3(0.5, 0.5, 0.5));

    // register ourselves to be notified of the event
    ASSERT_TRUE(QObject::connect(cube->visual_actor(), SIGNAL(onMouseDown(MouseEvent* )),
                                 this, SLOT(mouseHandler(MouseEvent* ))));

    // it seems that we need to call render to either update the position of the cube,
    //    or to register it with the scene in the first place
    bumptop_->renderTick();

    // send the event
    bumptop_->mouseDown(240, 180, 1, NO_KEY_MODIFIERS_MASK);
    EXPECT_EQ(240, x_);
    EXPECT_EQ(180, y_);
  }

  TEST_F(MouseEventTest, Mouse_dragged_event_gets_projected) {
    // we'll be testing to see if this Cube is hit (it should be)
    Cube *cube = new Cube(bumptop_->ogre_scene_manager(), physics_, NULL);
    cube->init();
    cube->ogre_scene_node()->setPosition(0, 0, -200);
    cube->ogre_scene_node()->scale(Ogre::Vector3(0.5, 0.5, 0.5));

    // register ourselves to be notified of the event
    ASSERT_TRUE(QObject::connect(cube->visual_actor(), SIGNAL(onMouseDragged(MouseEvent*)),
                                 this, SLOT(mouseHandler(MouseEvent*))));

    // it seems that we need to call render to either update the position of the cube,
    //    or to register it with the scene in the first place
    bumptop_->renderTick();

    // send the event
    bumptop_->mouseDragged(240, 180, 1, NO_KEY_MODIFIERS_MASK);
    EXPECT_EQ(240, x_);
    EXPECT_EQ(180, y_);
  }

  TEST_F(MouseEventTest, Mouse_up_event_gets_projected) {
    // we'll be testing to see if this Cube is hit (it should be)
    Cube *cube = new Cube(bumptop_->ogre_scene_manager(), physics_, NULL);
    cube->init();
    cube->ogre_scene_node()->setPosition(0, 0, -200);
    cube->ogre_scene_node()->scale(Ogre::Vector3(0.5, 0.5, 0.5));

    // register ourselves to be notified of the event
    ASSERT_TRUE(QObject::connect(cube->visual_actor(), SIGNAL(onMouseUp(MouseEvent*)),
                                 this, SLOT(mouseHandler(MouseEvent*))));

    // it seems that we need to call render to either update the position of the cube,
    //    or to register it with the scene in the first place
    BumpTopApp::singleton()->markGlobalStateAsChanged();
    bumptop_->renderTick();

    // send the event
    bumptop_->mouseUp(240, 180, 1, NO_KEY_MODIFIERS_MASK);
    EXPECT_EQ(240, x_);
    EXPECT_EQ(180, y_);
  }

  TEST_F(MouseEventTest, Mouse_event_is_projected_on_individual_triangles) {
    // we'll be testing to see if cube2 is hit (it should be)
    //    cube1's AABB bounding box is in front of cube2's, but cube2 is in front of cube1
    //    by default, OGRE just does raycast tests with the bounding boxes, so we need to do
    //    polygon-level raycasting, which this is testing

    Cube *cube1 = new Cube(bumptop_->ogre_scene_manager(), physics_, NULL);
    cube1->init();
    cube1->ogre_scene_node()->setPosition(-10, 0, -300);
    cube1->ogre_scene_node()->scale(Ogre::Vector3(0.5, 0.5, 0.5));
    cube1->ogre_scene_node()->yaw(Ogre::Degree(45.0));

    Cube *cube2 = new Cube(bumptop_->ogre_scene_manager(), physics_, NULL);
    cube2->init();
    cube2->ogre_scene_node()->setPosition(+20, 0, -300);
    cube2->ogre_scene_node()->scale(Ogre::Vector3(0.3, 0.3, 0.3));


    // register ourselves to be notified of the event
    ASSERT_TRUE(QObject::connect(cube2->visual_actor(), SIGNAL(onMouseDown(MouseEvent*)),
                                 this, SLOT(mouseHandler(MouseEvent*))));

    // it seems that we need to call render to either update the position of the cube,
    //    or to register it with the scene in the first place
    BumpTopApp::singleton()->markGlobalStateAsChanged();
    bumptop_->renderTick();

    // send the event
    NSEvent *fake_mouse_event = createMouseEvent(NSLeftMouseDown, 270, 180);
    [[NSApplication sharedApplication] postEvent: fake_mouse_event atStart: YES];
    bumptop_->processOneEvent();

    EXPECT_EQ(270, x_);
    EXPECT_EQ(180, y_);
  }
}  // namespace

#include "BumpTop/moc/moc_MouseEventTest.mm";
