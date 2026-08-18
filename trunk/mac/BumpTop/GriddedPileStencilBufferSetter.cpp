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
  // Ported to Ogre 14's StencilState API. (The old setStencilBufferParams
  // call kept compiling against the deprecated wrapper, but Ogre inserted a
  // writeMask parameter mid-signature, silently shifting every argument:
  // the stencil was never written and gridded-pile contents clipped away.)
  if (kMinGriddedPileRenderQueueGroup <= queueGroupId && queueGroupId < kMaxGriddedPileRenderQueueGroup &&
      queueGroupId % 3 == 1) {
    // Phase 1: the pile's background quad writes 1s into the stencil.
    Ogre::RenderSystem* render_system = Ogre::Root::getSingleton().getRenderSystem();
    render_system->clearFrameBuffer(Ogre::FBT_STENCIL);
    Ogre::StencilState stencil_state;
    stencil_state.enabled = true;
    stencil_state.compareOp = Ogre::CMPF_ALWAYS_PASS;
    stencil_state.referenceValue = 1;
    stencil_state.compareMask = 0xFFFFFFFF;
    stencil_state.writeMask = 0xFFFFFFFF;
    stencil_state.stencilFailOp = Ogre::SOP_KEEP;
    stencil_state.depthFailOp = Ogre::SOP_KEEP;
    stencil_state.depthStencilPassOp = Ogre::SOP_REPLACE;
    render_system->setStencilState(stencil_state);
  }

  if (kMinGriddedPileRenderQueueGroup <= queueGroupId && queueGroupId < kMaxGriddedPileRenderQueueGroup &&
      queueGroupId % 3 == 2) {
    // Phase 2: pile members only draw where the stencil equals 1.
    // (BUMPTOP_NO_STENCIL=1 disables the clip for debugging.)
    Ogre::RenderSystem* render_system = Ogre::Root::getSingleton().getRenderSystem();
    Ogre::StencilState stencil_state;
    stencil_state.enabled = true;
    stencil_state.compareOp = getenv("BUMPTOP_NO_STENCIL") != NULL ?
                              Ogre::CMPF_ALWAYS_PASS : Ogre::CMPF_EQUAL;
    stencil_state.referenceValue = 1;
    stencil_state.compareMask = 0xFFFFFFFF;
    stencil_state.writeMask = 0xFFFFFFFF;
    stencil_state.stencilFailOp = Ogre::SOP_KEEP;
    stencil_state.depthFailOp = Ogre::SOP_KEEP;
    stencil_state.depthStencilPassOp = Ogre::SOP_KEEP;
    render_system->setStencilState(stencil_state);
  }
}

void GriddedPileStencilBufferSetter::renderQueueEnded(uint8 queueGroupId,
                                                      const Ogre::String &invocation, bool &repeatThisInvocation) {
  if (kMinGriddedPileRenderQueueGroup <= queueGroupId && queueGroupId < kMaxGriddedPileRenderQueueGroup) {
    Ogre::RenderSystem* render_system = Ogre::Root::getSingleton().getRenderSystem();
    Ogre::StencilState stencil_state;  // defaults: disabled
    render_system->setStencilState(stencil_state);
  }
}

bool GriddedPileStencilBufferSetter::added_to_scene_manager() {
  return added_to_scene_manager_;
}

void GriddedPileStencilBufferSetter::set_added_to_scene_manager(bool value) {
  added_to_scene_manager_ = value;
}
