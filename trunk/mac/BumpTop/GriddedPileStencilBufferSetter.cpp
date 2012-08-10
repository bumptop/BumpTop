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

#include "BumpTop/GriddedPileStencilBufferSetter.h"

#include "BumpTop/GriddedPileManager.h"

SINGLETON_IMPLEMENTATION(GriddedPileStencilBufferSetter)

GriddedPileStencilBufferSetter::GriddedPileStencilBufferSetter()
: added_to_scene_manager_(false) {
}

GriddedPileStencilBufferSetter::~GriddedPileStencilBufferSetter() {
}

void GriddedPileStencilBufferSetter::renderQueueStarted(uint8 queueGroupId,
                                                        const Ogre::String &invocation, bool &skipThisInvocation) {
  if (kMinGriddedPileRenderQueueGroup <= queueGroupId && queueGroupId < kMaxGriddedPileRenderQueueGroup &&
      queueGroupId % 3 == 1) {
    Ogre::RenderSystem * render_system = Ogre::Root::getSingleton().getRenderSystem();
    render_system->clearFrameBuffer(Ogre::FBT_STENCIL);
    render_system->setStencilCheckEnabled(true);
    render_system->setStencilBufferParams(Ogre::CMPF_ALWAYS_PASS,  // The comparison function applied
                                          1,                       // The reference value used in the comparison
                                          0xFFFFFFFF,              // The bitmask applied to both the stencil value
                                                                       // and the reference value before comparison
                                          Ogre::SOP_KEEP,          // The action to perform when the stencil check fails
                                          Ogre::SOP_KEEP,          // The action to perform when the stencil check
                                                                       // passes, but the depth buffer check still fails
                                          Ogre::SOP_REPLACE);      // The action to take when both the stencil and depth
                                                                       // check pass
  }

  if (kMinGriddedPileRenderQueueGroup <= queueGroupId && queueGroupId < kMaxGriddedPileRenderQueueGroup &&
      queueGroupId % 3 == 2) {
    Ogre::RenderSystem * render_system = Ogre::Root::getSingleton().getRenderSystem();
    render_system->setStencilCheckEnabled(true);
    render_system->setStencilBufferParams(Ogre::CMPF_EQUAL,        // The comparison function applied
                                          1,                       // The reference value used in the comparison
                                          0xFFFFFFFF,              // The bitmask applied to both the stencil value
                                                                       // and the reference value before comparison
                                          Ogre::SOP_KEEP,          // The action to perform when the stencil check fails
                                          Ogre::SOP_KEEP,          // The action to perform when the stencil check
                                                                       // passes, but the depth buffer check still fails
                                          Ogre::SOP_KEEP);         // The action to take when both the stencil and depth
                                                                       // check pass
  }
}

void GriddedPileStencilBufferSetter::renderQueueEnded(uint8 queueGroupId,
                                                      const Ogre::String &invocation, bool &repeatThisInvocation) {
  if (kMinGriddedPileRenderQueueGroup <= queueGroupId && queueGroupId < kMaxGriddedPileRenderQueueGroup) {
    Ogre::RenderSystem * render_system = Ogre::Root::getSingleton().getRenderSystem();
    render_system->setStencilCheckEnabled(false);
    render_system->setStencilBufferParams();
  }
}

bool GriddedPileStencilBufferSetter::added_to_scene_manager() {
  return added_to_scene_manager_;
}

void GriddedPileStencilBufferSetter::set_added_to_scene_manager(bool value) {
  added_to_scene_manager_ = value;
}
