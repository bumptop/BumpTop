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

#include "BumpTop/Authorization.h"
#include "BumpTop/BumpTopApp.h"

namespace {
  class BumpTopAppTest : public ::testing::Test {
  };

  TEST_F(BumpTopAppTest, BumpTopApp_is_created) {
    ASSERT_NE(reinterpret_cast<BumpTopApp*>(NULL),
              BumpTopApp::singleton());
  }

  TEST_F(BumpTopAppTest, Ogre_Root_is_created) {
    ASSERT_NE(reinterpret_cast<Ogre::Root*>(NULL),
              Ogre::Root::getSingletonPtr());
  }

  TEST_F(BumpTopAppTest, RenderSystem_is_selected) {
    ASSERT_NE(reinterpret_cast<Ogre::RenderSystem*>(NULL),
              Ogre::Root::getSingleton().getRenderSystem());
  }

  TEST_F(BumpTopAppTest, SceneManager_is_created_and_is_generic) {
    ASSERT_NE(reinterpret_cast<Ogre::SceneManager*>(NULL),
              BumpTopApp::singleton()->ogre_scene_manager());
    ASSERT_NE(Ogre::StringConverter::toString(Ogre::ST_GENERIC),
              BumpTopApp::singleton()->ogre_scene_manager()->getTypeName());
  }

  TEST_F(BumpTopAppTest, Camera_is_created) {
    Ogre::SceneManager *scene_manager =
    BumpTopApp::singleton()->ogre_scene_manager();

    Ogre::Camera* camera = scene_manager->getCamera("MainCamera");
    ASSERT_NE(reinterpret_cast<Ogre::Camera*>(NULL),
              camera);
  }

  TEST_F(BumpTopAppTest, Viewport_is_created) {
    Ogre::RenderWindow *render_window = BumpTopApp::singleton()->render_window();
    ASSERT_NE(reinterpret_cast<Ogre::RenderWindow*>(NULL),
              render_window);
    ASSERT_EQ(1, render_window->getNumViewports());

    Ogre::Viewport *viewport = render_window->getViewport(0);
    ASSERT_NE(reinterpret_cast<Ogre::Viewport*>(NULL),
              viewport);

    Ogre::SceneManager *scene_manager = BumpTopApp::singleton()->ogre_scene_manager();
    Ogre::Camera* camera = scene_manager->getCamera("MainCamera");
    ASSERT_EQ(camera, viewport->getCamera());
  }

  TEST_F(BumpTopAppTest, Viewport_background_is_black) {
    Ogre::RenderWindow *render_window = BumpTopApp::singleton()->render_window();
    ASSERT_NE(reinterpret_cast<Ogre::RenderWindow*>(NULL),
              render_window);

    Ogre::Viewport *viewport = render_window->getViewport(0);
    ASSERT_NE(reinterpret_cast<Ogre::Viewport*>(NULL),
              viewport);

    Ogre::ColourValue background_color = viewport->getBackgroundColour();
    ASSERT_EQ(Ogre::ColourValue(0, 0, 0), background_color);
  }

  TEST_F(BumpTopAppTest, Camera_aspect_ratio_is_set) {
    Ogre::RenderWindow *render_window =
    BumpTopApp::singleton()->render_window();
    ASSERT_NE(reinterpret_cast<Ogre::RenderWindow*>(NULL),
               render_window);

    Ogre::SceneManager *scene_manager =
    BumpTopApp::singleton()->ogre_scene_manager();
    ASSERT_NE(reinterpret_cast<Ogre::SceneManager*>(NULL),
              scene_manager);

    Ogre::Camera* camera = scene_manager->getCamera("MainCamera");
    ASSERT_NE(reinterpret_cast<Ogre::Camera*>(NULL),
              camera);

    Ogre::Viewport *viewport = render_window->getViewport(0);
    ASSERT_NE(reinterpret_cast<Ogre::Viewport*>(NULL),
              viewport);

    ASSERT_EQ(Ogre::Real(viewport->getActualWidth()) / Ogre::Real(viewport->getActualHeight()),
              camera->getAspectRatio());
  }
}  // namespace
