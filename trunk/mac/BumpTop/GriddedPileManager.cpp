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

#include "BumpTop/GriddedPileManager.h"

#include "BumpTop/GriddedPile.h"
#include "BumpTop/GriddedPileStencilBufferSetter.h"
#include "BumpTop/Room.h"
#include "BumpTop/RoomSurface.h"

GriddedPileManager::GriddedPileManager(Room* room, Physics* physics, Ogre::SceneManager* scene_manager)
: room_(room),
  physics_(physics),
  ogre_scene_manager_(scene_manager) {
  if (!GriddedPileStencilBufferSetter::singleton()->added_to_scene_manager()) {
    ogre_scene_manager_->addRenderQueueListener(GriddedPileStencilBufferSetter::singleton());
    GriddedPileStencilBufferSetter::singleton()->set_added_to_scene_manager(true);
  }
}


void GriddedPileManager::registerAndChangeMyPosition(GriddedPile *gridded_pile) {
  gridded_piles.push_back(gridded_pile);

  Ogre::Real gridded_floor_height = kFloorMinHeight + hidden_floors.size()*kFloorSpacing;


  RoomSurface *gridded_pile_floor = new RoomSurface(ogre_scene_manager_, physics_,
                                                    room_->ogre_scene_node(), "", NOT_PINNABLE_RECEIVER);
  gridded_pile_floor->init(Ogre::Vector3::UNIT_Y, room_->floor_width(), room_->floor_depth(), NO_VISUAL_ACTOR);

  gridded_pile_floor->set_position(Ogre::Vector3(room_->floor_width()/2.0,
                                                 gridded_floor_height,
                                                 room_->floor_depth()/2.0));
  gridded_pile_floor->set_thickness(kGriddedPileFloorThickness);
  gridded_pile_floor->set_room(room_);
  gridded_pile_floor->setCollisionsToGriddedPiles();
  gridded_pile->set_render_queue_group(kMinGriddedPileRenderQueueGroup + hidden_floors.size()*3);

  gridded_pile->set_room_surface(gridded_pile_floor);

  hidden_floors.push_back(gridded_pile_floor);

  gridded_pile->set_position(Ogre::Vector3(gridded_pile->position().x,
                                           gridded_floor_height + gridded_pile_floor->size().y/2 + gridded_pile->size().y/2,  // NOLINT
                                           gridded_pile->position().z));
}

void GriddedPileManager::unregister(GriddedPile *gridded_pile) {
  int pile_being_unregistered_index = gridded_piles.indexOf(gridded_pile);
  assert(pile_being_unregistered_index != -1);
  if (pile_being_unregistered_index != -1) {
    gridded_piles.removeAt(pile_being_unregistered_index);

    int last_floor_index = hidden_floors.size() - 1;
    delete hidden_floors.value(last_floor_index);
    hidden_floors.removeAt(last_floor_index);
    for (int i = pile_being_unregistered_index; i < gridded_piles.size(); i++) {
      GriddedPile *gridded_pile = gridded_piles.value(i);
      RoomSurface *gridded_pile_floor = hidden_floors.value(i);

      Ogre::Real gridded_floor_height = kFloorSpacing + i*kFloorSpacing;
      gridded_pile->set_position(Ogre::Vector3(gridded_pile->position().x,
                                               gridded_floor_height + gridded_pile_floor->size().y/2 + gridded_pile->size().y/2,  // NOLINT
                                               gridded_pile->position().z));
      gridded_pile->set_render_queue_group(kMinGriddedPileRenderQueueGroup + hidden_floors.size()*3);
      gridded_pile->set_room_surface(gridded_pile_floor);
    }
  }
}
