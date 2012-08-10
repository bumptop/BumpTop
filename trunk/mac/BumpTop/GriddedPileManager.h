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

#ifndef BUMPTOP_GRIDDEDPILEMANAGER_H_
#define BUMPTOP_GRIDDEDPILEMANAGER_H_

class GriddedPile;
class Physics;
class Room;
class RoomSurface;

const Ogre::Real kFloorMinHeight = 25.0;
const Ogre::Real kFloorSpacing = 25.0;
const Ogre::Real kGriddedPileFloorThickness = 5.0;
const uint8 kMinGriddedPileRenderQueueGroup = 60;
const uint8 kMaxGriddedPileRenderQueueGroup = 94;

class GriddedPileManager {
 public:
  GriddedPileManager(Room* room, Physics* physics, Ogre::SceneManager* scene_manager);

  void registerAndChangeMyPosition(GriddedPile *gridded_pile);
  void unregister(GriddedPile *gridded_pile);
 protected:
  QList<GriddedPile*> gridded_piles;
  QList<RoomSurface*> hidden_floors;
  Room *room_;
  Physics *physics_;
  Ogre::SceneManager *ogre_scene_manager_;
};



// contains datastructre of all GriddedPiles, and their heights
#endif  // BUMPTOP_GRIDDEDPILEMANAGER_H_
