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

#include "BumpTop/Clickable.h"
#include "BumpTop/OSX/EventModifierFlags.h"

namespace {
  class ClickableTest : public QObject, public ::testing::Test {
    Q_OBJECT

   public:
    virtual void SetUp() {
      c_ = new ClickableOverlay();
      x_ = y_ = 0.0;
    }

    virtual void TearDown() {
      delete c_;
    }

   public slots:  // NOLINT
    void onMouseHandler(MouseEvent* mouse_event) {
      x_ = mouse_event->mouse_in_window_space.x;
      y_ = mouse_event->mouse_in_window_space.y;
    }
   protected:
    ClickableOverlay *c_;
    float x_, y_;
  };

  TEST_F(ClickableTest, Clickable_class_exists) {
    ASSERT_NE(reinterpret_cast<Clickable*>(NULL), c_);
  }

  TEST_F(ClickableTest, Clickable_class_accepts_mouse_down) {
    MouseEvent mouse_event = MouseEvent(Ogre::Vector2::ZERO, true, NULL, Ogre::Vector3::ZERO, 1, NO_KEY_MODIFIERS_MASK,
                                        false, false);
    c_->mouseDown(&mouse_event);
  }

  TEST_F(ClickableTest, Clickable_class_accepts_mouse_up) {
    MouseEvent mouse_event = MouseEvent(Ogre::Vector2::ZERO, true, NULL, Ogre::Vector3::ZERO, 1, NO_KEY_MODIFIERS_MASK,
                                        false, false);
    c_->mouseUp(&mouse_event);
  }

  TEST_F(ClickableTest, Clickable_class_accepts_mouse_dragged) {
    MouseEvent mouse_event = MouseEvent(Ogre::Vector2::ZERO, true, NULL, Ogre::Vector3::ZERO, 1, NO_KEY_MODIFIERS_MASK,
                                        false, false);
    c_->mouseDragged(&mouse_event);
  }

  TEST_F(ClickableTest, Mouse_down_triggers_onMouseDown_signal) {
    ASSERT_TRUE(QObject::connect(c_, SIGNAL(onMouseDown(MouseEvent*)),  // NOLINT
                                 this, SLOT(onMouseHandler(MouseEvent*))));  // NOLINT
    MouseEvent mouse_event = MouseEvent(Ogre::Vector2(1, 1), true, NULL, Ogre::Vector3::ZERO, 1, NO_KEY_MODIFIERS_MASK,
                                        false, false);
    c_->mouseDown(&mouse_event);
    EXPECT_EQ(1.0, x_);
    EXPECT_EQ(1.0, y_);
  }

  TEST_F(ClickableTest, Mouse_up_triggers_onMouseUp_signal) {
    ASSERT_TRUE(QObject::connect(c_, SIGNAL(onMouseUp(MouseEvent*)),  // NOLINT
                                 this, SLOT(onMouseHandler(MouseEvent*))));  // NOLINT
    MouseEvent mouse_event = MouseEvent(Ogre::Vector2(1, 1), true, NULL, Ogre::Vector3::ZERO, 1, NO_KEY_MODIFIERS_MASK,
                                        false, false);
    c_->mouseUp(&mouse_event);
    EXPECT_EQ(1.0, x_);
    EXPECT_EQ(1.0, y_);
  }

  TEST_F(ClickableTest, Mouse_dragged_triggers_onMouseDragged_signal) {
    ASSERT_TRUE(QObject::connect(c_, SIGNAL(onMouseDragged(MouseEvent*)),  // NOLINT
                                 this, SLOT(onMouseHandler(MouseEvent*))));  // NOLINT
    MouseEvent mouse_event = MouseEvent(Ogre::Vector2(1, 1), true, NULL, Ogre::Vector3::ZERO, 1, NO_KEY_MODIFIERS_MASK,
                                        false, false);
    c_->mouseDragged(&mouse_event);
    EXPECT_EQ(1.0, x_);
    EXPECT_EQ(1.0, y_);
  }

}  // namespace

#include "BumpTop/moc/moc_ClickableTest.cpp"
