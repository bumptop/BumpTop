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

#ifndef BUMPTOP_GRIDDEDPILESTENCILBUFFERSETTER_H_
#define BUMPTOP_GRIDDEDPILESTENCILBUFFERSETTER_H_

#include "BumpTop/Singleton.h"

class GriddedPileStencilBufferSetter : public Ogre::RenderQueueListener {
  SINGLETON_HEADER(GriddedPileStencilBufferSetter)
 public:
  GriddedPileStencilBufferSetter();
  virtual ~GriddedPileStencilBufferSetter();
  virtual void renderQueueStarted(uint8 queueGroupId, const Ogre::String &invocation, bool &skipThisInvocation);
  virtual void renderQueueEnded(uint8 queueGroupId, const Ogre::String &invocation, bool &repeatThisInvocation);
  bool added_to_scene_manager();
  void set_added_to_scene_manager(bool value);
 protected:
  bool added_to_scene_manager_;
};

#endif  // BUMPTOP_GRIDDEDPILESTENCILBUFFERSETTER_H_
