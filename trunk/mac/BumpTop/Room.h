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

#ifndef BUMPTOP_ROOM_H_
#define BUMPTOP_ROOM_H_

#include <boost/shared_ptr.hpp>
#include <Ogre.h>
#include <QtCore/QHash>
#include <string>
#include <vector>

#include "BumpTop/FileDropReceiver.h"
#include "BumpTop/FileSystemEventDispatcher.h"
#include "BumpTop/GriddedPileManager.h"
#include "BumpTop/KeyboardEventManager.h"
#include "BumpTop/Timer.h"
#include "BumpTop/VisualPhysicsActor.h"
#include "BumpTop/VisualPhysicsActorId.h"
#include "BumpTop/VisualPhysicsActorList.h"

class BumpDummy;
class BumpToolbar;
class BumpTopApp;
class RoomSurface;
class BumpBox;
class MaterialLoader;
class MouseEvent;
class Physics;
class RoomBuffer;
class FileDropReceiver;
class RoomUndoRedoState;
class RoomStateUndoCommand;
class RoomSurface;
class SearchBox;
class UndoRedoStack;
class FileSystemWatcher;
class VisualPhysicsActor;
class VisualPhysicsActorAnimation;

const int kInitialActorSize = 95;
const Ogre::Real kRoomWallHeight = 500;
const Ogre::Real kRoomSurfaceOverlap = 50;

enum RoomSurfaceType {
  FLOOR = 0,
  LEFT_WALL = 1,
  FRONT_WALL = 2,
  RIGHT_WALL = 3,
  BACK_WALL = 4,
  CEILING = 5,
  LEFT_SAFETY_WALL = 6,
  FRONT_SAFETY_WALL = 7,
  RIGHT_SAFETY_WALL = 8,
  BACK_SAFETY_WALL = 9,
  NONE = 10
};

const std::string kDefaultFloorTexturePath = "bumptheme_floor.png";
const std::string kDefaultFrontWallTexturePath = "bumptheme_front.png";
const std::string kDefaultBackWallTexturePath = "bumptheme_front.png";
const std::string kDefaultRightWallTexturePath = "bumptheme_wall.png";
const std::string kDefaultLeftWallTexturePath = "bumptheme_wall.png";
const bool KDefaultUseFloorTextureForAll = false;

class Room : public FileSystemEventReceiver, public DropTarget {
  Q_OBJECT
 public:
  Room(BumpTopApp *app,
       Ogre::SceneNode *parent_ogre_scene_node,
       Ogre::SceneManager *scene_manager,
       Physics *physics,
       QString path);
  virtual ~Room();

  virtual void init(Ogre::Real floor_width, Ogre::Real floor_depth, Ogre::Real wall_height = 0);
  virtual bool initFromBuffer(RoomBuffer* buffer, Ogre::Vector2 window_size);
  virtual void setBirdsEyeCameraForRoom();
  virtual void setCameraForRoom(bool animate = true);
  virtual VisualPhysicsActor* actor_with_unique_id(VisualPhysicsActorId unique_id);
  virtual VisualPhysicsActor* flattened_actor_with_unique_id(VisualPhysicsActorId unique_id);
  virtual void addActor(VisualPhysicsActor* actor);
  virtual bool containsActorWithId(VisualPhysicsActorId actor_id);
  virtual bool containsDirectlyOrThroughChild(VisualPhysicsActorId actor_id);
  virtual bool removeActor(VisualPhysicsActorId actor_id);
  virtual void writeToBuffer(RoomBuffer* buffer);
  virtual void writeToRoomUndoRedoState(boost::shared_ptr<RoomUndoRedoState> state);
  virtual void updateCurrentState();
  virtual boost::shared_ptr<RoomUndoRedoState> current_state();
  virtual UndoRedoStack* undo_redo_stack();
  virtual void updateAllLabelPositions();

  virtual void openNewUndoCommand();
  virtual void expectDriveToBeAdded(QString path, Ogre::Vector3 position, RoomSurfaceType surface, Ogre::Vector3 size, BumpBoxLabelColour colour);
  virtual void expectFileToBeAdded(QString path, Ogre::Vector2 mouse_in_window_space);
  virtual void expectFilesToBeRestoredToOriginalGrouping(QHash<QString, VisualPhysicsActorId> files_paths_to_actors_ids,
                                                         QHash<VisualPhysicsActorId, VisualPhysicsActorId> actors_ids_to_parents_ids,  // NOLINT
                                                         QHash<VisualPhysicsActorId, VisualPhysicsActorType> parents_ids_to_parents_types,  // NOLINT
                                                         QHash<VisualPhysicsActorId, QList<VisualPhysicsActorId> > parents_ids_to_children_ids);  // NOLINT
  virtual void restoreFileToOriginalGrouping(VisualPhysicsActorId);
  virtual void draggingItemsComplete();
  virtual void draggingItemsBegan();

  // Setters
  virtual void set_position(Ogre::Vector3 position);
  virtual void set_orientation(Ogre::Quaternion orientation);

  // Getters
  virtual Ogre::SceneNode* ogre_scene_node();
  virtual VisualPhysicsActorList room_actor_list();
  virtual VisualPhysicsActorList floor_actor_list();
  virtual QHash<VisualPhysicsActorId, VisualPhysicsActor*> room_actors();
  virtual VisualPhysicsActorList selected_actors();
  virtual void deselectActors();
  virtual void activatePhysicsForAllActors();
  SearchBox* search_box();
  virtual FileDropReceiver* room_drop_receiver();

  virtual void remove_hidden_drives();

  virtual void resizeRoomForResolution(Ogre::Real floor_width, Ogre::Real floor_depth, Ogre::Real wall_height = 0);
  virtual void constrainRoomActorPoses();

  virtual Ogre::Vector3 min();
  virtual Ogre::Vector3 max();
  virtual Ogre::Real min_x();
  virtual Ogre::Real max_x();
  virtual Ogre::Real min_z();
  virtual Ogre::Real max_z();
  virtual Ogre::Real min_y();
  virtual Ogre::Real max_y();
  virtual Ogre::Vector3 center_of_floor();
  virtual Ogre::Real floor_width();
  virtual Ogre::Real floor_depth();
  virtual Ogre::Real wall_height();
  virtual const QString& path();
  GriddedPileManager* gridded_pile_manager();

  virtual QList<RoomSurface*> surfaces();
  RoomSurface* getSurface(RoomSurfaceType room_surface_type);

  virtual void set_lasso_selection_active(bool active);
  virtual void set_last_selected_actor(VisualPhysicsActorId last_selected_actor);
  virtual VisualPhysicsActorId last_selected_actor();
  virtual void process_arrow_key(ArrowKey arrow_key, int modifier_flags);
  virtual VisualPhysicsActorId adjacentActorOfRoomActor(VisualPhysicsActorId child_id, ArrowKey arrow_key);
  virtual void closeViewOfLastSelectedActor();

  virtual bool setAndAdjustMaterialForSurface(RoomSurfaceType room_surface_type, const QString& path, bool background_loaded = true);  // NOLINT
  virtual void applyMaterialForSurface(RoomSurfaceType room_surface_type, const QString& path, bool background_loaded);
  virtual void updateAllTexturesFromSettings(bool background_loaded = true);
  virtual void setTextureSettingForSurface(RoomSurfaceType wall_type, QString path);

  virtual void set_new_items_pile(VisualPhysicsActorId work_pile);
  VisualPhysicsActorId new_items_pile();

  virtual void updateBumpToolbar();
  BumpToolbar* bump_toolbar();
  virtual void hideToolbar();
  virtual void setToolbarNeedsUpdating();
  void addStickyPadInDefaultLocation();
  void addNewItemsPileInDefaultLocation();
  void removeNewItemsPile();

 public slots:  // NOLINT
  virtual void fileAdded(const QString& path);
  virtual void fileRemoved(const QString& path);
  virtual void fileRenamed(const QString& old_path, const QString& new_path);
  virtual void fileModified(const QString& path);
  virtual void deleteMaterialLoader(MaterialLoader* material);
  virtual void renderTick();
  virtual void draggingEntered(MouseEvent* mouse_event);
  virtual void draggingUpdated(MouseEvent* mouse_event);
  virtual void selectedActorsChanged(VisualPhysicsActorId actor_id);
  virtual void actorStoppedMoving(VisualPhysicsActorId actor_id);
  virtual void searchBoxActivated();
  virtual void searchBoxDismissed();
  virtual void createAndFadeActorCopy(VisualPhysicsActor* actor);
  virtual void remove_hidden_drives_recently_added(Timer* timer);

  virtual void deleteActorCopyWhenFadeFinishes(VisualPhysicsActorAnimation* animation);

 signals: // NOLINT
  void onSelectedActorsChanged();

 protected:
  virtual void update();
  bool isFileWithPathInRoom(QString path, VisualPhysicsActorList actors);
  QList<VisualPhysicsActor*> flattenedChildren();

  BumpTopApp *app_;
  Ogre::SceneManager *ogre_scene_manager_;
  Physics *physics_;
  Ogre::SceneNode *ogre_scene_node_, *parent_ogre_scene_node_;
  Ogre::Real floor_width_, floor_depth_, wall_height_;
  QHash<VisualPhysicsActorId, VisualPhysicsActor*> room_actors_;
  boost::shared_ptr<RoomUndoRedoState> current_state_;
  UndoRedoStack* undo_redo_stack_;

  bool lasso_is_active_;
  bool toolbar_needs_updating_;
  QHash <VisualPhysicsActorId, bool> moving_actors_to_wait_for_before_showing_toolbar_;

  QString path_;
  QStringList paths_being_watched_;
  FileDropReceiver *room_drop_receiver_;
  QHash<QString, Ogre::Vector3> expected_file_positions_;
  QHash<QString, RoomSurfaceType> expected_file_room_surface_;
  QHash<RoomSurfaceType, RoomSurface*> room_surfaces_;
  QHash<QString, Ogre::Vector3> expected_drive_positions_;
  QHash<QString, RoomSurfaceType> expected_drive_room_surface_;
  QHash<QString, Ogre::Vector3> expected_drive_icon_size_;
  QHash<QString, BumpBoxLabelColour> expected_drive_label_colour_;
  QHash<VisualPhysicsActorId, Timer*> drives_recently_added_;
  BumpToolbar* bump_toolbar_;
  SearchBox* search_box_;
  GriddedPileManager* gridded_pile_manager_;
  VisualPhysicsActorId bump_target_;
  VisualPhysicsActorId new_items_pile_id_;
  VisualPhysicsActorId last_selected_actor_;

  QHash<QString, VisualPhysicsActorId> files_paths_to_actors_ids_;
  QHash<VisualPhysicsActorId, VisualPhysicsActorId> actors_ids_to_parents_ids_;
  QHash<VisualPhysicsActorId, VisualPhysicsActorType> parents_ids_to_parents_types_;
  QHash<VisualPhysicsActorId, QList<VisualPhysicsActorId> > parents_ids_to_children_ids_;
  QHash<VisualPhysicsActorId, int> files_of_same_grouping_that_have_been_added_;
};

#endif  // BUMPTOP_ROOM_H_
