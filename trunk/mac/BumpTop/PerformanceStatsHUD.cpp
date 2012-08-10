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

#include "BumpTop/PerformanceStatsHUD.h"

#include "BumpTop/BumpTopApp.h"
#include "BumpTop/FileManager.h"
#include "BumpTop/QStringHelpers.h"

SINGLETON_IMPLEMENTATION(PerformanceStatsHUD)

bool PerformanceStatsHUD::HUD_resources_are_loaded_ = false;

PerformanceStatsHUD::PerformanceStatsHUD()
: show_HUD_(false) {
  if (!HUD_resources_are_loaded_) {
    QString resource_path = FileManager::getResourcePath();
    Ogre::ResourceGroupManager::getSingleton().addResourceLocation(utf8(resource_path + "/OgreCore.zip"),
                                                                   "Zip", "Bootstrap");
    Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup("Bootstrap");
    HUD_resources_are_loaded_ = true;
  }

  debug_overlay_ = Ogre::OverlayManager::getSingleton().getByName("Core/DebugOverlay");
  avg_FPS_ = Ogre::OverlayManager::getSingleton().getOverlayElement("Core/AverageFps");
  curr_FPS_ = Ogre::OverlayManager::getSingleton().getOverlayElement("Core/CurrFps");
  best_FPS_ = Ogre::OverlayManager::getSingleton().getOverlayElement("Core/BestFps");
  worst_FPS_ = Ogre::OverlayManager::getSingleton().getOverlayElement("Core/WorstFps");
  memory_use_in_bytes = Ogre::OverlayManager::getSingleton().getOverlayElement("Core/NumTris");

  assert(QObject::connect(BumpTopApp::singleton(), SIGNAL(onRender()),  // NOLINT
                          this, SLOT(renderTick())));  // NOLINT
}

void PerformanceStatsHUD::toggleHUD() {
  show_HUD_ = !show_HUD_;
  if (show_HUD_) {
    debug_overlay_->show();
  } else {
    debug_overlay_->hide();
  }

  BumpTopApp::singleton()->markGlobalStateAsChanged();
}

void PerformanceStatsHUD::renderTick() {
  if (show_HUD_) {
    Ogre::RenderWindow* render_window = BumpTopApp::singleton()->render_window();
    const Ogre::RenderTarget::FrameStats& stats = render_window->getStatistics();

    avg_FPS_->setCaption("Average FPS: " + Ogre::StringConverter::toString(stats.avgFPS));
    curr_FPS_->setCaption("Current FPS: " + Ogre::StringConverter::toString(stats.lastFPS));
    best_FPS_->setCaption("Best FPS: " + Ogre::StringConverter::toString(stats.bestFPS)
                          + " " + Ogre::StringConverter::toString(stats.bestFrameTime) + " ms");
    worst_FPS_->setCaption("Worst FPS: " + Ogre::StringConverter::toString(stats.worstFPS)
                         + " " + Ogre::StringConverter::toString(stats.worstFrameTime) + " ms");
    Ogre::Real mem_in_mb = Ogre::TextureManager::getSingleton().getMemoryUsage()/1048576.0;
    memory_use_in_bytes->setCaption("Video memory (MB): " + Ogre::StringConverter::toString(mem_in_mb));
  }
}

#include "BumpTop/moc/moc_PerformanceStatsHUD.cpp"
