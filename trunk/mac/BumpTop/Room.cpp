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

#include "BumpTop/Room.h"

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <string>
#include <utility>
#include <vector>

#ifndef BUMPTOP_TEST
#import <Sparkle/Sparkle.h>
#endif

#include "BumpTop/AnimationManager.h"
#include "BumpTop/Authorization.h"
#include "BumpTop/AppSettings.h"
#include "BumpTop/Box.h"
#include "BumpTop/BumpPile.h"
#include "BumpTop/BumpFlatSquare.h"
#include "BumpTop/BumpDummy.h"
#include "BumpTop/BumpToolbar.h"
#include "BumpTop/BumpTopApp.h"
#include "BumpTop/BumpTopScene.h"
#include "BumpTop/CameraAnimation.h"
#include "BumpTop/DesktopItems.h"
#include "BumpTop/FileManager.h"
#include "BumpTop/FileSystemEventDispatcher.h"
#include "BumpTop/for_each.h"
#include "BumpTop/GriddedPile.h"
#include "BumpTop/GriddedPileManager.h"
#include "BumpTop/KeyboardEventManager.h"
#include "BumpTop/MaterialLoader.h"
#include "BumpTop/NewItemsPile.h"
#include "BumpTop/OgreHelpers.h"
#include "BumpTop/protoc/AllMessages.pb.h"
#include "BumpTop/ProtocolBufferHelpers.h"
#include "BumpTop/QStringHelpers.h"
#include "BumpTop/FileDropReceiver.h"
#include "BumpTop/RoomItemPoseConstraints.h"
#include "BumpTop/UndoCommands/BreakSinglePileUndoCommand.h"
#include "BumpTop/UndoCommands/PileizeUndoCommand.h"
#include "BumpTop/UndoCommands/RoomStateUndoCommand.h"
#include "BumpTop/RoomSurface.h"
#include "BumpTop/UndoCommands/RoomUndoRedoState.h"
#include "BumpTop/SearchBox.h"
#include "BumpTop/StickyNote.h"
#include "BumpTop/StickyNotePad.h"
#include "BumpTop/Timer.h"
#include "BumpTop/UndoRedoStack.h"
#include "BumpTop/VisualActor.h"
#include "BumpTop/VisualPhysicsActorAnimation.h"

Room::Room(BumpTopApp *app,
           Ogre::SceneNode *parent_ogre_scene_node,
           Ogre::SceneManager *scene_manager,
           Physics *physics,
           QString path)
: app_(app),
  bump_toolbar_(NULL),
  parent_ogre_scene_node_(parent_ogre_scene_node),
  ogre_scene_manager_(scene_manager),
  physics_(physics),
  path_(path),
  search_box_(NULL),
  gridded_pile_manager_(NULL),
  lasso_is_active_(false),
  toolbar_needs_updating_(false),
  new_items_pile_id_(0) {
}

Room::~Room() {
  // Delete all the BumpBoxes
  for_each(VisualPhysicsActor* actor, room_actors_.values())
    delete actor;

  Ogre::String light_name = "RoomLight" + addressToString(this);
  ogre_scene_manager_->destroyLight(light_name);

  for_each(RoomSurface* surface, room_surfaces_.values())
    delete surface;
  delete undo_redo_stack_;
  delete room_drop_receiver_;
  delete search_box_;

  parent_ogre_scene_node_->removeChild(ogre_scene_node_);
  ogre_scene_manager_->destroySceneNode(ogre_scene_node_);

  if (!path_.isNull() && path_ != "")
    FileSystemEventDispatcher::singleton()->removePathToWatch(path_, this);
}

struct RoomSurfaceInitArgs {
  PinnableReceiver is_pinnable_receiver;
  Ogre::Vector3 normal_vector;
  Ogre::Real x_size;
  Ogre::Real z_size;
  Ogre::Vector3 position;
  std::string image_path;
  QString image_overlay_path;
  RoomSurfaceType room_surface_type;
  bool visible;
};

// This method, given a width, depth and height, defines the standard 5-sided room
void Room::init(Ogre::Real floor_width, Ogre::Real floor_depth, Ogre::Real wall_height) {
  if (parent_ogre_scene_node_ == NULL) {
    parent_ogre_scene_node_ = ogre_scene_manager_->getRootSceneNode();
  }
  if (wall_height == 0)
    wall_height = kRoomWallHeight;

  floor_width_ = floor_width;
  floor_depth_ = floor_depth;
  wall_height_ = wall_height;

  AppSettings::singleton()->loadSettingsFile();

  Ogre::ColourValue room_colour = Ogre::ColourValue(0, 0.36, 0.68, 0.0);
  ogre_scene_node_ = parent_ogre_scene_node_->createChildSceneNode();

#define NUM_SURFACES 9
  RoomSurfaceInitArgs room_surface_init_args[NUM_SURFACES];

  // FLOOR
  RoomSurfaceInitArgs& floor = room_surface_init_args[0];
  floor.is_pinnable_receiver = NOT_PINNABLE_RECEIVER;
  floor.normal_vector = Ogre::Vector3::UNIT_Y;
  floor.x_size = floor_width + kRoomSurfaceOverlap;
  floor.z_size = floor_depth + kRoomSurfaceOverlap;
  floor.position = Ogre::Vector3(floor_width/2.0, -RoomSurface::kSurfaceThickness/2.0, floor_depth/2.0);
  floor.room_surface_type = FLOOR;
  floor.visible = true;

  // LEFT WALL
  RoomSurfaceInitArgs& left_wall = room_surface_init_args[1];
  left_wall.is_pinnable_receiver = PINNABLE_RECEIVER;
  left_wall.normal_vector = Ogre::Vector3::UNIT_X;
  left_wall.x_size = floor_depth + kRoomSurfaceOverlap;
  left_wall.z_size = wall_height + kRoomSurfaceOverlap;
  left_wall.position = Ogre::Vector3(-RoomSurface::kSurfaceThickness/2.0, wall_height/2.0, floor_depth/2.0);
  left_wall.room_surface_type = LEFT_WALL;
  left_wall.visible = true;

  // RIGHT WALL
  RoomSurfaceInitArgs& right_wall = room_surface_init_args[2];
  right_wall.is_pinnable_receiver = PINNABLE_RECEIVER;
  right_wall.normal_vector = -Ogre::Vector3::UNIT_X;
  right_wall.x_size = floor_depth + kRoomSurfaceOverlap;
  right_wall.z_size = wall_height + kRoomSurfaceOverlap;
  right_wall.position = Ogre::Vector3(floor_width + RoomSurface::kSurfaceThickness/2.0,
                                      wall_height/2.0,
                                      floor_depth/2.0);
  right_wall.room_surface_type = RIGHT_WALL;
  right_wall.visible = true;

  // BACK WALL
  RoomSurfaceInitArgs& back_wall = room_surface_init_args[3];
  back_wall.is_pinnable_receiver = PINNABLE_RECEIVER;
  back_wall.normal_vector = Ogre::Vector3::UNIT_Z;
  back_wall.x_size = floor_width + kRoomSurfaceOverlap;
  back_wall.z_size = wall_height + kRoomSurfaceOverlap;
  back_wall.position = Ogre::Vector3(floor_width/2.0, wall_height/2.0, -RoomSurface::kSurfaceThickness/2.0);
  back_wall.room_surface_type = BACK_WALL;
  back_wall.visible = true;

  // FRONT WALL
  RoomSurfaceInitArgs& front_wall = room_surface_init_args[4];
  front_wall.is_pinnable_receiver = PINNABLE_RECEIVER;
  front_wall.normal_vector = -Ogre::Vector3::UNIT_Z;
  front_wall.x_size = floor_width + kRoomSurfaceOverlap;
  front_wall.z_size = wall_height + kRoomSurfaceOverlap;
  front_wall.position = Ogre::Vector3(floor_width/2.0,
                                      wall_height/2.0,
                                      floor_depth + RoomSurface::kSurfaceThickness/2.0);
  front_wall.room_surface_type = FRONT_WALL;
  front_wall.visible = true;

  // The safety walls exist to prevent items from flying over the walls
  // LEFT SAFETY WALL
  RoomSurfaceInitArgs& left_safety_wall = room_surface_init_args[5];
  left_safety_wall.is_pinnable_receiver = NOT_PINNABLE_RECEIVER;
  left_safety_wall.normal_vector = Ogre::Vector3::UNIT_X;
  left_safety_wall.x_size = floor_depth + kRoomSurfaceOverlap;
  left_safety_wall.z_size = 10*wall_height + kRoomSurfaceOverlap;
  left_safety_wall.position = Ogre::Vector3(-RoomSurface::kSurfaceThickness/2.0,
                                            left_wall.position.y + left_wall.z_size/2.0
                                            + left_safety_wall.z_size/2.0, floor_depth/2.0);
  left_safety_wall.room_surface_type = LEFT_SAFETY_WALL;
  left_safety_wall.visible = false;

  // RIGHT SAFETY WALL
  RoomSurfaceInitArgs& right_safety_wall = room_surface_init_args[6];
  right_safety_wall.is_pinnable_receiver = NOT_PINNABLE_RECEIVER;
  right_safety_wall.normal_vector = -Ogre::Vector3::UNIT_X;
  right_safety_wall.x_size = floor_depth + kRoomSurfaceOverlap;
  right_safety_wall.z_size = 10*wall_height + kRoomSurfaceOverlap;
  right_safety_wall.position = Ogre::Vector3(floor_width + RoomSurface::kSurfaceThickness/2.0,
                                            right_wall.position.y + right_wall.z_size/2.0
                                            + right_safety_wall.z_size/2.0, floor_depth/2.0);
  right_safety_wall.room_surface_type = RIGHT_SAFETY_WALL;
  right_safety_wall.visible = false;

  // BACK SAFETY WALL
  RoomSurfaceInitArgs& back_safety_wall = room_surface_init_args[7];
  back_safety_wall.is_pinnable_receiver = NOT_PINNABLE_RECEIVER;
  back_safety_wall.normal_vector = Ogre::Vector3::UNIT_Z;
  back_safety_wall.x_size = floor_width + kRoomSurfaceOverlap;
  back_safety_wall.z_size = 10*wall_height + kRoomSurfaceOverlap;
  back_safety_wall.position = Ogre::Vector3(floor_width/2.0,
                                     back_wall.position.y + back_wall.z_size/2.0 + back_safety_wall.z_size/2.0,
                                     -RoomSurface::kSurfaceThickness/2.0);
  back_safety_wall.room_surface_type = BACK_SAFETY_WALL;
  back_safety_wall.visible = false;

  // FRONT SAFETY WALL
  RoomSurfaceInitArgs& front_safety_wall = room_surface_init_args[8];
  front_safety_wall.is_pinnable_receiver = NOT_PINNABLE_RECEIVER;
  front_safety_wall.normal_vector = -Ogre::Vector3::UNIT_Z;
  front_safety_wall.x_size = floor_width + kRoomSurfaceOverlap;
  front_safety_wall.z_size = 10*wall_height + kRoomSurfaceOverlap;
  front_safety_wall.position = Ogre::Vector3(floor_width/2.0,
                                     front_wall.position.y + front_wall.z_size/2.0 + front_safety_wall.z_size/2.0,
                                     floor_depth + RoomSurface::kSurfaceThickness/2.0);
  front_safety_wall.room_surface_type = FRONT_SAFETY_WALL;
  front_safety_wall.visible = false;

  for (int i = 0; i < NUM_SURFACES; i++) {
    RoomSurfaceInitArgs& init_args = room_surface_init_args[i];
    RoomSurface *surface = new RoomSurface(ogre_scene_manager_, physics_, ogre_scene_node_, path_, init_args.is_pinnable_receiver);  // NOLINT
    surface->init(init_args.normal_vector, init_args.x_size, init_args.z_size);
    surface->set_position(init_args.position);
    surface->set_room(this);
    surface->set_visible(init_args.visible);
    surface->set_room_surface_type(init_args.room_surface_type);
    surface->set_render_queue_group(Ogre::RENDER_QUEUE_BACKGROUND);
    assert(QObject::connect(surface->visual_actor(), SIGNAL(onDraggingEntered(MouseEvent*)),  // NOLINT
                            this, SLOT(draggingEntered(MouseEvent*))));  // NOLINT
    assert(QObject::connect(surface->visual_actor(), SIGNAL(onDraggingUpdated(MouseEvent*)),  // NOLINT
                            this, SLOT(draggingUpdated(MouseEvent*))));  // NOLINT
    room_surfaces_.insert(init_args.room_surface_type, surface);
  }

  updateAllTexturesFromSettings(false);

  gridded_pile_manager_ = new GriddedPileManager(this, physics_, ogre_scene_manager_);

  // Add a light to our scene
  // TODO: should this light be attached to the room's scene node?
  Ogre::String light_name = "RoomLight" + addressToString(this);
  Ogre::Light* scene_light = app_->ogre_scene_manager()->createLight(light_name);
  scene_light->setPosition(floor_width/2.0, 800, floor_depth/2.0);

  undo_redo_stack_ = new UndoRedoStack(this);
  undo_redo_stack_->init();

  room_drop_receiver_ = new FileDropReceiver(this);

  paths_being_watched_.push_back(path_);
  FileSystemEventDispatcher::singleton()->addPathToWatch(path_, this, true);
  FileSystemEventDispatcher::singleton()->addPathToWatch(FileManager::getApplicationDataPath() + "Stickies",
                                                         this, true);

  search_box_ = new SearchBox(this);
  search_box_->init();
  assert(QObject::connect(search_box_, SIGNAL(onSearchBoxDismissed()),  // NOLINT
                          this, SLOT(searchBoxDismissed())));  // NOLINT

  assert(QObject::connect(app_, SIGNAL(onRender()),  // NOLINT
                          this, SLOT(renderTick())));  // NOLINT

  setCameraForRoom(false);
  updateCurrentState();
}

void Room::resizeRoomForResolution(Ogre::Real floor_width, Ogre::Real floor_depth, Ogre::Real wall_height) {
  if (wall_height == 0)
    wall_height = kRoomWallHeight;

  floor_width_ = floor_width;
  floor_depth_ = floor_depth;
  wall_height_ = wall_height;

  getSurface(FLOOR)->set_size(floor_width + kRoomSurfaceOverlap, floor_depth + kRoomSurfaceOverlap);
  getSurface(FLOOR)->set_position(Ogre::Vector3(floor_width/2.0,
                                                -RoomSurface::kSurfaceThickness/2.0,
                                                floor_depth/2.0));
  getSurface(LEFT_WALL)->set_size(floor_depth + kRoomSurfaceOverlap, wall_height + kRoomSurfaceOverlap);
  getSurface(LEFT_WALL)->set_position(Ogre::Vector3(-RoomSurface::kSurfaceThickness/2.0,
                                                    wall_height/2.0,
                                                    floor_depth/2.0));
  getSurface(RIGHT_WALL)->set_size(floor_depth + kRoomSurfaceOverlap, wall_height + kRoomSurfaceOverlap);
  getSurface(RIGHT_WALL)->set_position(Ogre::Vector3(floor_width + RoomSurface::kSurfaceThickness/2.0,
                                                     wall_height/2.0,
                                                     floor_depth/2.0));
  getSurface(BACK_WALL)->set_size(floor_width + kRoomSurfaceOverlap, wall_height + kRoomSurfaceOverlap);
  getSurface(BACK_WALL)->set_position(Ogre::Vector3(floor_width/2.0,
                                                    wall_height/2.0,
                                                    -RoomSurface::kSurfaceThickness/2.0));
  getSurface(FRONT_WALL)->set_size(floor_width + kRoomSurfaceOverlap, wall_height + kRoomSurfaceOverlap);
  getSurface(FRONT_WALL)->set_position(Ogre::Vector3(floor_width/2.0,
                                                     wall_height/2.0,
                                                     floor_depth + RoomSurface::kSurfaceThickness/2.0));
  getSurface(LEFT_SAFETY_WALL)->set_size(floor_depth + kRoomSurfaceOverlap, 10*wall_height + kRoomSurfaceOverlap);
  getSurface(LEFT_SAFETY_WALL)->set_position(Ogre::Vector3(-RoomSurface::kSurfaceThickness/2.0,
                                                     getSurface(LEFT_WALL)->position().y + getSurface(LEFT_WALL)->size().z/2.0  // NOLINT
                                                     + getSurface(LEFT_SAFETY_WALL)->size().z/2.0, floor_depth/2.0));
  getSurface(RIGHT_SAFETY_WALL)->set_size(floor_depth + kRoomSurfaceOverlap, 10*wall_height + kRoomSurfaceOverlap);
  getSurface(RIGHT_SAFETY_WALL)->set_position(Ogre::Vector3(floor_width + RoomSurface::kSurfaceThickness/2.0,
                                                            getSurface(RIGHT_WALL)->position().y + getSurface(RIGHT_WALL)->size().z/2.0  // NOLINT
                                                            + getSurface(RIGHT_SAFETY_WALL)->size().z/2.0, floor_depth/2.0));  // NOLINT
  getSurface(BACK_SAFETY_WALL)->set_size(floor_width + kRoomSurfaceOverlap, 10*wall_height + kRoomSurfaceOverlap);
  getSurface(BACK_SAFETY_WALL)->set_position(Ogre::Vector3(floor_width/2.0,
                                                           getSurface(BACK_WALL)->position().y + getSurface(BACK_WALL)->size().z/2.0  // NOLINT
                                                           + getSurface(BACK_SAFETY_WALL)->size().z/2.0,
                                                           -RoomSurface::kSurfaceThickness/2.0));
  getSurface(FRONT_SAFETY_WALL)->set_size(floor_width + kRoomSurfaceOverlap, 10*wall_height + kRoomSurfaceOverlap);
  getSurface(FRONT_SAFETY_WALL)->set_position(Ogre::Vector3(floor_width/2.0,
                                                            getSurface(FRONT_WALL)->position().y + getSurface(FRONT_WALL)->size().z/2.0  // NOLINT
                                                            + getSurface(FRONT_SAFETY_WALL)->size().z/2.0,
                                                            floor_depth + RoomSurface::kSurfaceThickness/2.0));

  updateAllTexturesFromSettings(false);

  constrainRoomActorPoses();
  setCameraForRoom(false);
}

void Room::updateAllLabelPositions() {
  for_each(VisualPhysicsActor* actor, flattenedChildren()) {
    actor->updateLabelPosition();
  }
}

void Room::constrainRoomActorPoses() {
  QHash<VisualPhysicsActorId, BumpPose> desired_poses;
  for_each(VisualPhysicsActorId actor_id, room_actors_.keys()) {
    desired_poses.insert(actor_id, room_actors_[actor_id]->pose());
  }

  // Determine the positions after constraining the actors to the room and removing any intersecting items
  QHash<VisualPhysicsActorId, BumpPose> constrained_poses = getActorPosesConstrainedToRoom(desired_poses, this);
  constrained_poses = getActorPosesConstrainedToNoIntersections(constrained_poses, this);

  // Move the actors to their constrained positions
  for_each(VisualPhysicsActorId actor_id, constrained_poses.keys()) {
    VisualPhysicsActor* actor = room_actors_[actor_id];
    if (actor->room_surface()->is_pinnable_receiver())
      actor->removePhysicsConstraints();

    room_actors_[actor_id]->set_pose(constrained_poses[actor_id]);

    if (actor->room_surface()->is_pinnable_receiver())
      actor->pinToSurface(actor->room_surface());
  }
}

bool Room::initFromBuffer(RoomBuffer* buffer, Ogre::Vector2 window_size) {
  init(window_size.x, window_size.y);
  VisualPhysicsActor* new_actor;
  if (buffer->has_last_run_bumptop_version())
    AppSettings::singleton()->set_last_run_bumptop_version(QStringFromUtf8(buffer->last_run_bumptop_version()));

  for (int i = 0; i < buffer->saved_drive_positions_size(); i++) {
    SavedActorPosition* expected_drive = buffer->mutable_saved_drive_positions(i);
    QString path = QStringFromUtf8(expected_drive->path());
    expected_drive_positions_[path] = Vector3BufferToVector3(expected_drive->position());
    expected_drive_icon_size_[path] = Vector3BufferToVector3(expected_drive->size());
    expected_drive_room_surface_[path] = (RoomSurfaceType)expected_drive->room_surface();
    expected_drive_label_colour_[path] = (BumpBoxLabelColour)expected_drive->label_colour();
  }

  if (versionStringLessThanVersionString(AppSettings::singleton()->last_run_bumptop_version(), QString("0.975")) &&
      ![[SUUpdater sharedUpdater] sendsSystemProfile] &&
      ![[SUUpdater sharedUpdater] automaticallyDownloadsUpdates]) {
    [[SUUpdater sharedUpdater] setSendsSystemProfile:YES];
    [[SUUpdater sharedUpdater] setAutomaticallyDownloadsUpdates:YES];
  }

  bool found_stick_note_pad = false;
  bool found_work_pile = false;
  QHash<VisualPhysicsActorId, BumpPose> desired_poses;
  for (int i = 0; i < buffer->actor_size(); i++) {
    if (buffer->mutable_actor(i)->actor_type() == BUMP_BOX) {
      new_actor = new BumpFlatSquare(ogre_scene_manager_, physics_, this);
    } else if (buffer->mutable_actor(i)->actor_type() == BUMP_PILE) {
      new_actor = new BumpPile(ogre_scene_manager_, physics_, this);
    } else if (buffer->mutable_actor(i)->actor_type() == GRIDDED_PILE) {
      new_actor = new GriddedPile(ogre_scene_manager_, physics_, this, Ogre::Vector3::ZERO);
    } else if (buffer->mutable_actor(i)->actor_type() == STICKY_NOTE) {
      new_actor = new StickyNote(ogre_scene_manager_, physics_, this);
    } else if (buffer->mutable_actor(i)->actor_type() == STICKY_NOTE_PAD) {
      new_actor = new StickyNotePad(ogre_scene_manager_, physics_, this);
      found_stick_note_pad = true;
    } else {
      continue;
    }
    if (!new_actor->initFromBuffer(buffer->mutable_actor(i))) {
      delete new_actor;
    } else if (buffer->mutable_actor(i)->is_new_items_pile()
               && AppSettings::singleton()->use_new_items_pile_setting()) {
      if (buffer->mutable_actor(i)->actor_type() == GRIDDED_PILE) {
        new_items_pile_id_ = new_actor->unique_id();
      } else {
        set_new_items_pile(new_actor->unique_id());
      }
      found_work_pile = true;
    }
  }

  if (!found_stick_note_pad) {
    addStickyPadInDefaultLocation();
  }
  
  if (!found_work_pile) {
    addNewItemsPileInDefaultLocation();
  }

  for_each(DesktopItem drive, getDrives()) {
    if (!isFileWithPathInRoom(drive.file_path, room_actor_list())) {
      BumpBox* box = new BumpFlatSquare(ogre_scene_manager_, physics_, this);
      box->initWithPath(drive.file_path);
      // Check if expected drives added to desktop
      if (expected_drive_positions_.contains(drive.file_path)) {
        box->set_surface_and_position(getSurface(expected_drive_room_surface_[drive.file_path]),
                                      expected_drive_positions_[drive.file_path]);
        box->set_size(expected_drive_icon_size_[drive.file_path]);
      } else {
        box->set_position(Ogre::Vector3(drive.position_x, 100, drive.position_y));
        box->set_size(Ogre::Vector3(kInitialActorSize, kInitialActorSize, kInitialActorSize));
      }
    }
  }

  constrainRoomActorPoses();

  QHash<QString, QStringList*> paths_to_watch_and_their_contents;
  // Make sure that we watch the room's main path (typically the desktop)
  paths_to_watch_and_their_contents.insert(path_, new QStringList());

  // Initialize the file system watcher so that it uses as its reference point the state that the room is in
  for_each(VisualPhysicsActor* actor, room_actors_) {
    if (actor->path() != "") {
      QString actor_parent_path = FileManager::getParentPath(actor->path());
      if (!paths_to_watch_and_their_contents.contains(actor_parent_path)) {
        paths_to_watch_and_their_contents.insert(actor_parent_path, new QStringList());
      }
      paths_to_watch_and_their_contents[actor_parent_path]->push_back(actor->path());
    }

    QStringList actor_paths = actor->pathsOfDescendants();
    for_each(QString path, actor_paths) {
      if (path != "" && !paths_to_watch_and_their_contents.contains(FileManager::getParentPath(path))) {
        paths_to_watch_and_their_contents.insert(FileManager::getParentPath(path), new QStringList());
      }
      paths_to_watch_and_their_contents[FileManager::getParentPath(path)]->push_back(path);
    }
  }

  for_each(QString parent_path, paths_to_watch_and_their_contents.keys()) {
    paths_being_watched_.push_back(parent_path);
    FileSystemEventDispatcher::singleton()->addPathToWatch(parent_path, this, true);
    FileSystemEventDispatcher::singleton()->generateFileSystemEventsSinceLastSnapshot(parent_path, paths_to_watch_and_their_contents[parent_path]);  // NOLINT
  }

  updateCurrentState();
  return true;
}

bool Room::isFileWithPathInRoom(QString path, VisualPhysicsActorList actors) {
  for_each(VisualPhysicsActor* actor, actors) {
    if (path == actor->path()) {
      return true;
    } else if (!actor->children().empty()) {
      if (isFileWithPathInRoom(path, actor->children()))
        return true;
    }
  }
  return false;
}

void Room::addStickyPadInDefaultLocation() {
  StickyNotePad* sticky_note_pad = new StickyNotePad(app_->ogre_scene_manager(), app_->physics(), this);
  sticky_note_pad->init();
  addActor(sticky_note_pad);
  sticky_note_pad->set_position(Ogre::Vector3(max_x() - 200, 150, min_z()));
  sticky_note_pad->set_size(Ogre::Vector3(150, 150, 150));
  sticky_note_pad->set_pose(sticky_note_pad->getPoseForSurface(getSurface(BACK_WALL)));
  sticky_note_pad->set_room_surface(getSurface(BACK_WALL));
  sticky_note_pad->pinToSurface(getSurface(BACK_WALL));
}

void Room::addNewItemsPileInDefaultLocation() {
  if (!AppSettings::singleton()->use_new_items_pile_setting()) {
    return;
  }
  NewItemsPile* pile = new NewItemsPile(app_->ogre_scene_manager(), app_->physics(), this);
  pile->init();
  addActor(pile);
  pile->set_position(Ogre::Vector3((min_x()+max_x())/2.0, 400, (min_z()+max_z())/2.0));
  pile->set_size(Ogre::Vector3(AppSettings::singleton()->default_icon_size(),
                               AppSettings::singleton()->default_icon_size(),
                               AppSettings::singleton()->default_icon_size()));

  pile->stackViewOnInit();
  
  new_items_pile_id_ = pile->unique_id();
}

void Room::removeNewItemsPile() {
  VisualPhysicsActor* new_items_pile = actor_with_unique_id(new_items_pile_id_);
  VisualPhysicsActorList pile_actors;

  if (new_items_pile == NULL) {
    return;
  }

  new_items_pile->closeView();
  new_items_pile = actor_with_unique_id(new_items_pile_id_);

  // Break and pileize contents of New Items Pile
  for_each(VisualPhysicsActor* actor, new_items_pile->children()) {
    if (actor->actor_type() != BUMP_DUMMY) {
      pile_actors << actor;
    }
  }
  BreakSinglePileUndoCommand* break_single_pile_command = new BreakSinglePileUndoCommand(new_items_pile_id_, this, ogre_scene_manager_, physics_);
  break_single_pile_command->redo();
  PileizeUndoCommand* pileize_command = new PileizeUndoCommand(pile_actors, this, ogre_scene_manager_, physics_, new_items_pile->position());
  pileize_command->redo();

  // Remove New Items Pile and Dummy
  VisualPhysicsActor* dummy = new_items_pile->children()[0];
  new_items_pile->breakPile();
  removeActor(dummy->unique_id());
  delete dummy;
  delete new_items_pile;

  // Reset New Items Pile Id
  new_items_pile_id_ = 0;
}

void Room::setBirdsEyeCameraForRoom() {
  const Ogre::Real back_shift_factor = 0;
  const Ogre::Real upward_shift_factor = 1.0;

  Ogre::Radian fov_y = app_->camera()->getFOVy();

  // Calculates the camera height which will encapsulate the z extent of the floor, and then shifts it up
  // according to the constant "upward_shift_factor" for the angled view
  Ogre::Real cam_y = min_y() + upward_shift_factor*((floor_depth_/2.0)/Ogre::Math::Tan(fov_y/2.0));

  // Calculates the camera's z position as the middle z coordinate of the floor and then shifts it back
  // the constant back_shift_factor for the angled view
  Ogre::Real cam_z = min_z() + floor_depth_/2.0 + (floor_depth_/2.0)*back_shift_factor;

  // Calculates the camera's x position as the middle x coordinate of the floor
  Ogre::Real cam_x = min_x() + floor_width_/2.0;

  // Set the camera's location and set it to look at the center of the floor
  app_->camera()->setPosition(Ogre::Vector3(cam_x, cam_y, cam_z));
  app_->camera()->lookAt(center_of_floor());
}

void Room::setCameraForRoom(bool animate) {
  const Ogre::Real back_shift_factor = 1.025;
  const Ogre::Real upward_shift_factor = 1.075;
  Ogre::Radian fov_y = app_->camera()->getFOVy();

  // Calculates the camera height which will encapsulate the z extent of the floor, and then shifts it up
  // according to the constant "upward_shift_factor" for the angled view
  Ogre::Real cam_y = min_y() + upward_shift_factor*((floor_depth_/2.0)/Ogre::Math::Tan(fov_y/2.0));

  // Calculates the camera's z position as the middle z coordinate of the floor and then shifts it back
  // the constant back_shift_factor for the angled view
  Ogre::Real cam_z = min_z() + floor_depth_/2.0 + (floor_depth_/2.0)*back_shift_factor;

  // Calculates the camera's x position as the middle x coordinate of the floor
  Ogre::Real cam_x = min_x() + floor_width_/2.0;

  // Store the camera's current position / orientation
  Ogre::Vector3 current_position = app_->camera()->getPosition();
  Ogre::Quaternion current_orientation = app_->camera()->getOrientation();

  // Determine the camera's final position and orientation
  app_->camera()->setPosition(Ogre::Vector3(cam_x, cam_y, cam_z));
  app_->camera()->lookAt(center_of_floor() - Ogre::Vector3(0, 0, 0.055*floor_depth_));
  Ogre::Vector3 direction = app_->camera()->getDirection();
  Ogre::Vector3 up = Ogre::Vector3(0, -direction.z, direction.y);
  Ogre::Quaternion rotation = app_->camera()->getRealUp().getRotationTo(up);
  app_->camera()->rotate(rotation);

  if (animate) {
    // Store the final position and orientation to animate to
    Ogre::Vector3 final_position = app_->camera()->getPosition();
    Ogre::Quaternion final_orientation = app_->camera()->getOrientation();
    // Reset the camera's position and orientation in preparation for animation
    app_->camera()->setPosition(current_position);
    app_->camera()->setOrientation(current_orientation);

    CameraAnimation* camera_animation = new CameraAnimation(app_->camera(), 500, final_position, final_orientation);
    camera_animation->start();
  }

  app_->scene()->set_surface_that_camera_is_zoomed_to(NONE);
}

VisualPhysicsActor* Room::actor_with_unique_id(VisualPhysicsActorId unique_id) {
  if (room_actors_.contains(unique_id))
    return room_actors_[unique_id];
  else
    return NULL;
}

VisualPhysicsActor* Room::flattened_actor_with_unique_id(VisualPhysicsActorId unique_id) {
  for_each(VisualPhysicsActor* child, flattenedChildren()) {
    if (child->unique_id() == unique_id) {
      return child;
    }
  }
  return NULL;
}

void Room::addActor(VisualPhysicsActor* actor) {
  room_actors_.insert(actor->unique_id(), actor);
  QString parent_path = FileManager::getParentPath(actor->path());

  // TODO: deal with piles who have children
  if (actor->path() != "" && parent_path != "" && !paths_being_watched_.contains(parent_path)) {
    paths_being_watched_.push_back(parent_path);
    FileSystemEventDispatcher::singleton()->addPathToWatch(parent_path, this, true);
  }

  assert(QObject::connect(actor, SIGNAL(onSelectedChanged(VisualPhysicsActorId)),  // NOLINT
                          this, SLOT(selectedActorsChanged(VisualPhysicsActorId))));  // NOLINT
  assert(QObject::connect(actor, SIGNAL(onStoppedMoving(VisualPhysicsActorId)),  // NOLINT
                          this, SLOT(actorStoppedMoving(VisualPhysicsActorId))));  // NOLINT
}

bool Room::removeActor(VisualPhysicsActorId actor_id) {
  if (room_actors_.contains(actor_id)) {
    assert(QObject::disconnect(room_actors_[actor_id], SIGNAL(onSelectedChanged(VisualPhysicsActorId)),  // NOLINT
                            this, SLOT(selectedActorsChanged(VisualPhysicsActorId))));  // NOLINT
    room_actors_.remove(actor_id);
    return true;
  } else {
    return false;
  }
}

void Room::set_lasso_selection_active(bool active) {
  lasso_is_active_ = active;
}

void Room::set_last_selected_actor(VisualPhysicsActorId last_selected_actor) {
  last_selected_actor_ = last_selected_actor;
}

VisualPhysicsActorId Room::last_selected_actor() {
  return last_selected_actor_;
}

void Room::process_arrow_key(ArrowKey arrow_key, int modifier_flags) {
  bool shift = modifier_flags & NSShiftKeyMask;
  bool command = modifier_flags & NSCommandKeyMask;

  VisualPhysicsActor* last_actor = flattened_actor_with_unique_id(last_selected_actor());
  if (command) {
    if (arrow_key == ARROW_UP && ProAuthorization::singleton()->authorized()) {
      if (last_actor->actor_type() == BUMP_PILE) {
        last_actor->children().first()->set_selected(true);
        last_actor->revealChild(last_actor->children().first()->unique_id());
      }
    }
    if (arrow_key == ARROW_DOWN) {
      last_actor->launch();
    }
  } else if (containsDirectlyOrThroughChild(last_selected_actor())) {
    VisualPhysicsActorId adjacent_actor_id = last_actor->adjacent_actor(arrow_key);

    if (containsDirectlyOrThroughChild(adjacent_actor_id)) {
      VisualPhysicsActor* adjacent_actor = flattened_actor_with_unique_id(adjacent_actor_id);
      
      if (!shift) {
        deselectActors();
      }

      if (adjacent_actor->parent() != NULL) {
        adjacent_actor->parent()->revealChild(adjacent_actor->unique_id());
      }
      adjacent_actor->set_selected(true);
    }
  }
}

VisualPhysicsActorId Room::adjacentActorOfRoomActor(VisualPhysicsActorId child_id, ArrowKey arrow_key) {
  VisualPhysicsActor* child = actor_with_unique_id(child_id);
  VisualPhysicsActorList actors_in_direction;
  VisualPhysicsActor* closest_actor_in_direction = NULL;
  for_each(VisualPhysicsActor* actor, room_actors()) {
    if (actor == child) {
      continue;
    }
    if (actor->room_surface() != child->room_surface()) {
      continue;
    }

    Ogre::Vector3 difference = actor->world_position() - child->position();
    if (arrow_key == ARROW_UP) {
      if (difference.z < 0 && abs(difference.x) < abs(child->size().z)) {
        actors_in_direction.push_back(actor);
      }
    } else if (arrow_key == ARROW_DOWN) {
      if (difference.z > 0 && abs(difference.x) < abs(child->size().z)) {
        actors_in_direction.push_back(actor);
      }
    } else if (arrow_key == ARROW_LEFT) {
      if (difference.x < 0 && abs(difference.z) < abs(child->size().x)) {
        actors_in_direction.push_back(actor);
      }
    } else if (arrow_key == ARROW_RIGHT) {
      if (difference.x > 0 && abs(difference.z) < abs(child->size().x)) {
        actors_in_direction.push_back(actor);
      }
    }
  }
  if (actors_in_direction.count() > 0) {
    closest_actor_in_direction = actors_in_direction[0];

    for_each(VisualPhysicsActor* actor, actors_in_direction) {
      if (arrow_key == ARROW_UP) {
        if (actor->world_position().z > closest_actor_in_direction->world_position().z) {
          closest_actor_in_direction = actor;
        }
      } else if (arrow_key == ARROW_DOWN) {
        if (actor->world_position().z < closest_actor_in_direction->world_position().z) {
          closest_actor_in_direction = actor;
        }
      } else if (arrow_key == ARROW_LEFT) {
        if (actor->world_position().x > closest_actor_in_direction->world_position().x) {
          closest_actor_in_direction = actor;
        }
      } else if (arrow_key == ARROW_RIGHT) {
        if (actor->world_position().x < closest_actor_in_direction->world_position().x) {
          closest_actor_in_direction = actor;
        }
      }
    }
    return closest_actor_in_direction->unique_id();
  }

  return 0;
}

void Room::closeViewOfLastSelectedActor() {
  VisualPhysicsActor* last_actor = flattened_actor_with_unique_id(last_selected_actor());
  if (last_actor->parent() != NULL) {
    last_actor->parent()->closeView();
    last_actor->parent()->set_selected(true);
  }
}

void Room::setToolbarNeedsUpdating() {
  toolbar_needs_updating_ = true;
}

void Room::hideToolbar() {
  toolbar_needs_updating_ = false;
  bump_toolbar()->hide();
}

void Room::selectedActorsChanged(VisualPhysicsActorId actor_id) {
  AnimationManager::singleton()->endAnimationsForMaterial(AppSettings::singleton()->global_material_name(HIGHLIGHT));  // NOLINT
  setToolbarNeedsUpdating();
  emit onSelectedActorsChanged();
}

void Room::searchBoxActivated() {
  hideToolbar();
}

void Room::searchBoxDismissed() {
  setToolbarNeedsUpdating();
}

void Room::writeToBuffer(RoomBuffer* buffer) {
  Ogre::Vector3 position = ogre_scene_node_->getPosition();

  buffer->clear_actor();
  for_each(VisualPhysicsActor* actor, room_actors_.values()) {
    if (actor->serializable())
      actor->writeToBuffer(buffer->add_actor());
  }
  buffer->set_last_run_bumptop_version(utf8(BumpTopApp::singleton()->bumptopVersion()));

  // Store expected drives
  for_each(QString path, expected_drive_positions_.keys()) {
    SavedActorPosition* saved_actor_position_buffer = buffer->add_saved_drive_positions();
    saved_actor_position_buffer->set_path(path.toStdString());
    Vector3ToBuffer(expected_drive_positions_[path], saved_actor_position_buffer->mutable_position());
    Vector3ToBuffer(expected_drive_icon_size_[path], saved_actor_position_buffer->mutable_size());
    saved_actor_position_buffer->set_room_surface(expected_drive_room_surface_[path]);
    saved_actor_position_buffer->set_label_colour((int)expected_drive_label_colour_[path]);
  }
}

void Room::writeToRoomUndoRedoState(boost::shared_ptr<RoomUndoRedoState> state) {
  for_each(VisualPhysicsActor* actor, room_actors_.values()) {
    state->add_actor(actor);
  }
}

void Room::updateCurrentState() {
  current_state_ = boost::shared_ptr<RoomUndoRedoState> (new RoomUndoRedoState());
  writeToRoomUndoRedoState(current_state_);
}

boost::shared_ptr<RoomUndoRedoState> Room::current_state() {
  return current_state_;
}

UndoRedoStack* Room::undo_redo_stack() {
  return undo_redo_stack_;
}

void Room::openNewUndoCommand() {
  updateCurrentState();
  undo_redo_stack_->openNewRoomStateUndoCommand(current_state_);
}

BumpToolbar* Room::bump_toolbar() {
  if (bump_toolbar_ == NULL) {
    bump_toolbar_ = new BumpToolbar(this);
    bump_toolbar_->init();
  }
  return bump_toolbar_;
}

void Room::updateBumpToolbar() {
  BumpToolbar* toolbar = bump_toolbar();
  VisualPhysicsActorList selected_actors_without_parents;

  for_each(VisualPhysicsActor* actor, selected_actors()) {
    if (actor->parent() == NULL)
      selected_actors_without_parents.push_back(actor);
  }

  if (selected_actors_without_parents.size() > 0 && !(lasso_is_active_ || search_box()->is_open() ||
      app_->keyboard_event_manager()->command_key_down())) {
    toolbar->updateForActorList(selected_actors_without_parents);
    toolbar->show();
  } else {
    hideToolbar();
  }
}

void Room::renderTick() {
  if (toolbar_needs_updating_) {
    updateBumpToolbar();
    toolbar_needs_updating_ = false;
  }
}

// Show the toolbar when we're finished dragging items
void Room::draggingItemsComplete() {
  bool items_moving = false;

  moving_actors_to_wait_for_before_showing_toolbar_.clear();
  for_each(VisualPhysicsActor* actor, selected_actors()) {
    // TODO: why is this needed !actor->room_surface()->is_pinnable_receiver()
    // seems like there's something wrong with the isMoving() code.
    if (actor->isMoving() && !actor->room_surface()->is_pinnable_receiver()) {
      items_moving = true;
      moving_actors_to_wait_for_before_showing_toolbar_.insert(actor->unique_id(), false);
    }
  }
  if (!items_moving) {
    setToolbarNeedsUpdating();
  }
}

void Room::actorStoppedMoving(VisualPhysicsActorId actor_id) {
  if (moving_actors_to_wait_for_before_showing_toolbar_.contains(actor_id)) {
    moving_actors_to_wait_for_before_showing_toolbar_[actor_id] = true;
    bool all_stopped = true;
    for_each(bool stopped, moving_actors_to_wait_for_before_showing_toolbar_.values()) {
      all_stopped = all_stopped && stopped;
    }
    if (all_stopped) {
      setToolbarNeedsUpdating();
      moving_actors_to_wait_for_before_showing_toolbar_.clear();
    }
  }
}

// Hide the toolbar when we begin dragging items
void Room::draggingItemsBegan() {
  hideToolbar();
}

VisualPhysicsActorList Room::selected_actors() {
  VisualPhysicsActorList selected_actors = VisualPhysicsActorList();
  for_each(VisualPhysicsActor* actor, flattenedChildren()) {
    if (actor->selected())
      selected_actors.push_back(actor);
  }
  return selected_actors;
}

void Room::deselectActors() {
  for_each(VisualPhysicsActor* actor, room_actors()) {
    actor->set_selected(false);
    for_each(VisualPhysicsActor* child, actor->children()) {
      child->set_selected(false);
    }
  }
}

void Room::fileAdded(const QString& path) {
  if (isFileWithPathInRoom(path, room_actor_list())) {
    return;
  } else if (path.startsWith("/Volumes") && !(QFileInfo(path).readLink() == "" || QFileInfo(path).readLink() == "/")) {
    // Apple stores all its mounted drives and a link to the startup disk in folder "/Volumes"
    // the original path of the startup disk is "/".
    // Sometimes other special items for example the iDisk can get into Volumes, these items are not drives but links
    // to drives so in this case we don't want to include them in room (Apple does not include them on desktop as well)
    return;
  } else if (FileManager::isVolume(path)) {
    if (!FileManager::isEjectableDrive(path)) {
      FileManager::addEjectableDrive(path);
    }
  }

  VisualPhysicsActorId expected_actor_id = 0;
  if (files_paths_to_actors_ids_.keys().contains(path)) {
    expected_actor_id = files_paths_to_actors_ids_[path];
  }

  BumpBox* new_box;
  // here we need some special code to deal with sticky notes
  if (FileManager::getParentPath(path) == FileManager::getApplicationDataPath() + "Stickies") {
    if (StickyNoteCounter::singleton()->count() >= 2 && !ProAuthorization::singleton()->authorized()) {
      return;
    }
    if (QFileInfo(path).suffix() == "txt") {
      new_box = new StickyNote(ogre_scene_manager_, physics_, this, expected_actor_id);
      new_box->initWithPath(path, false);
    } else {
      return;  // we only watch for text files in the sticky note directory
    }
  } else {
    new_box = new BumpFlatSquare(ogre_scene_manager_, physics_, this, expected_actor_id);
    new_box->initWithPath(path, false);
    new_box->set_size(Ogre::Vector3(AppSettings::singleton()->default_icon_size(),
                                    AppSettings::singleton()->default_icon_size(),
                                    AppSettings::singleton()->default_icon_size()));
    new_box->updateLabel();
    FileManager::getAndSetLabelColourThroughNSTask(new_box->unique_id());
  }

  if (expected_drive_positions_.contains(path)) {
    new_box->set_surface_and_position(getSurface(expected_drive_room_surface_[path]),
                                      expected_drive_positions_[path]);
    new_box->set_size(expected_drive_icon_size_[path]);
    new_box->set_label_colour(expected_drive_label_colour_[path]);
    new_box->updateLabel();
  } else if (expected_file_positions_.contains(path)) {
    new_box->set_surface_and_position(getSurface(expected_file_room_surface_[path]),
                                      expected_file_positions_[path]);
    expected_file_positions_.remove(path);
    expected_file_room_surface_.remove(path);
  } else {
    new_box->set_pose(getActorPoseConstrainedToRoomAndNoIntersections(new_box, this));
    if (containsActorWithId(new_items_pile()))
    {
      new_box->set_position(Ogre::Vector3(actor_with_unique_id(new_items_pile())->position().x, 225, // 225 is a random number as well as 15^2, change as required/desired.
                                          actor_with_unique_id(new_items_pile())->position().z));
      actor_with_unique_id(new_items_pile())->addActorToPileAndUpdatePileView(new_box,Ogre::Vector3::ZERO,new_box->orientation());
    }
    else
        // new_box->set_position(Ogre::Vector3(bump_target_->position().x, 50, bump_target_->position().z));
        new_box->set_position(Ogre::Vector3((min_x()+max_x())/2.0, 400, (min_z()+max_z())/2.0));
  }

  new_box->set_physics_enabled(true);
  new_box->activatePhysics();

  if (expected_actor_id != 0) {
    if (actors_ids_to_parents_ids_[expected_actor_id] != 0) {
      files_of_same_grouping_that_have_been_added_[actors_ids_to_parents_ids_[expected_actor_id]]++;
      restoreFileToOriginalGrouping(expected_actor_id);
    }
    files_paths_to_actors_ids_.remove(path);
    actors_ids_to_parents_ids_.remove(expected_actor_id);
  }

  // Undo addition of drives not shown
  if (FileManager::isVolume(path)) {
    drives_recently_added_[new_box->unique_id()] = new Timer;
    drives_recently_added_[new_box->unique_id()]->start(100);

    assert(QObject::connect(drives_recently_added_[new_box->unique_id()], SIGNAL(onTick(Timer*)), this, SLOT(remove_hidden_drives_recently_added(Timer*))));
  }
}

void Room::createAndFadeActorCopy(VisualPhysicsActor* actor) {
  VisualPhysicsActor* visual_copy_of_actor = actor->createVisualCopyOfSelf();
  // Ensure that a copy is created, as not all VPAs implement createVisualCopyOfSelf()
  if (visual_copy_of_actor != NULL) {
    VisualPhysicsActorAnimation* actor_animation = new VisualPhysicsActorAnimation(visual_copy_of_actor, 350,
                                                                                   visual_copy_of_actor->position(),
                                                                                   visual_copy_of_actor->orientation(),
                                                                                   NULL, 0);
    actor_animation->start();
    assert(QObject::connect(actor_animation, SIGNAL(onAnimationComplete(VisualPhysicsActorAnimation*)),  // NOLINT
                            this, SLOT(deleteActorCopyWhenFadeFinishes(VisualPhysicsActorAnimation*)))); // NOLINT
  }
}

void Room::fileRemoved(const QString& path) {
  // Here we need to copy the path since if the associated actor gets deleted
  // and the path was passed as actor->path(), then the variable
  // path is invalidated and can crash (since it is a const ref)
  QString path_copy = path;
  QHash<VisualPhysicsActorId, VisualPhysicsActor*> actors = room_actors();
  for_each(VisualPhysicsActorId key, actors.keys()) {
    if (path_copy == room_actors_[key]->path()) {
      if (FileManager::getFileKind(path_copy) == VOLUME) {
        this->expectDriveToBeAdded(path_copy,
                                   room_actors_[key]->position(),
                                   room_actors_[key]->room_surface()->room_surface_type(),
                                   room_actors_[key]->scale(),
                                   room_actors_[key]->label_colour());
      }
      if (FileManager::isConnectedServer(path)) {
        FileManager::removeConnectedServer(path);
      }
      VisualPhysicsActor* actor = room_actors_[key];
      createAndFadeActorCopy(actor);
      removeActor(actor->unique_id());
      delete actor;
    }
  }
  activatePhysicsForAllActors();
  setToolbarNeedsUpdating();
}

void Room::deleteActorCopyWhenFadeFinishes(VisualPhysicsActorAnimation* animation) {
  VisualPhysicsActor* actor = animation->visual_physics_actor();
  delete actor;
}

void Room::activatePhysicsForAllActors() {
  for_each(VisualPhysicsActor* actor, room_actors_.values())
    actor->activatePhysics();
}

void Room::fileRenamed(const QString& old_path, const QString& new_path) {
  if (old_path == path_)
    path_ = new_path;
}

void Room::fileModified(const QString& path) {
}

void Room::deleteMaterialLoader(MaterialLoader* material_loader) {
  delete material_loader;
}

void Room::draggingEntered(MouseEvent* mouse_event) {
  draggingUpdated(mouse_event);
}

void Room::draggingUpdated(MouseEvent* mouse_event) {
  // if items are being dropped internally, make sure none of the items being dragged are volumes
  //   or live in our directory already

  // TODO: This solves the pile of sticky notes that's being dragged. In general, I don't think this should
  // ever return true if it's dealing with actors inside a room, right?
  if (mouse_event->items_being_dropped.size() > 0)
    return;

  if (mouse_event->drag_operations & NSDragOperationGeneric)
    mouse_event->drag_operations = NSDragOperationGeneric;
  else if (mouse_event->drag_operations & NSDragOperationMove)
    mouse_event->drag_operations = NSDragOperationMove;
  else if (mouse_event->drag_operations & NSDragOperationCopy)
    mouse_event->drag_operations = NSDragOperationCopy;
  else if (mouse_event->drag_operations & NSDragOperationLink)
    mouse_event->drag_operations = NSDragOperationLink;
  else
    mouse_event->drag_operations = NSDragOperationNone;

  mouse_event->drop_receiver = room_drop_receiver_;
  mouse_event->handled = true;
}

void Room::expectDriveToBeAdded(QString path, Ogre::Vector3 position, RoomSurfaceType surface, Ogre::Vector3 size, BumpBoxLabelColour colour) {
  expected_drive_positions_[path] = position;
  expected_drive_room_surface_[path] = surface;
  expected_drive_icon_size_[path] = size;
  expected_drive_label_colour_[path] = colour;
}

void Room::expectFileToBeAdded(QString path, Ogre::Vector2 mouse_in_window_space) {
  for_each(RoomSurfaceType surface_type, room_surfaces_.keys()) {
    std::pair<bool, Ogre::Vector3> new_position_result;
    new_position_result = getSurface(surface_type)->mouseIntersectionAbove(mouse_in_window_space, 0.0);
    if (new_position_result.first) {
      expected_file_positions_[path] = new_position_result.second;
      expected_file_room_surface_[path] = surface_type;
    }
  }
}

void Room::expectFilesToBeRestoredToOriginalGrouping(QHash<QString, VisualPhysicsActorId> files_paths_to_actors_ids,
                                                     QHash<VisualPhysicsActorId, VisualPhysicsActorId> actors_ids_to_parents_ids,  // NOLINT
                                                     QHash<VisualPhysicsActorId, VisualPhysicsActorType> parents_ids_to_parents_types,  // NOLINT
                                                     QHash<VisualPhysicsActorId, QList<VisualPhysicsActorId> > parents_ids_to_children_ids) {  // NOLINT
  files_paths_to_actors_ids_ = files_paths_to_actors_ids;
  actors_ids_to_parents_ids_ = actors_ids_to_parents_ids;
  parents_ids_to_parents_types_ = parents_ids_to_parents_types;
  parents_ids_to_children_ids_ = parents_ids_to_children_ids;
  for_each(VisualPhysicsActorId parent_id, parents_ids_to_parents_types.keys()) {
    files_of_same_grouping_that_have_been_added_[parent_id] = 0;
  }
}

void Room::restoreFileToOriginalGrouping(VisualPhysicsActorId actor_id) {
  VisualPhysicsActorId parent_id = actors_ids_to_parents_ids_[actor_id];
  VisualPhysicsActor* parent = actor_with_unique_id(parent_id);
  if (parent != NULL) {
    parent->addActorToPileAndUpdatePileView(actor_with_unique_id(actor_id),
                                            Ogre::Vector3::ZERO,
                                            Ogre::Quaternion::IDENTITY);
  } else {
    QList<VisualPhysicsActor*> actors_to_be_grouped;

    // find all actors in room that were originally part of the same pile or gridded pile
    for_each(VisualPhysicsActor* actor, room_actor_list()) {
      if (parents_ids_to_children_ids_[parent_id].contains(actor->unique_id())) {
        actors_to_be_grouped.append(actor);
      }
    }

    if (actors_to_be_grouped.count() == 2) {
      if (parents_ids_to_parents_types_[parent_id] == BUMP_PILE) {
        // create a new pile with the same unique id as the original pile
        PileizeUndoCommand* pileize_command = new PileizeUndoCommand(actors_to_be_grouped,
                                                                     this, ogre_scene_manager_,
                                                                     physics_, Ogre::Vector3::ZERO,
                                                                     "", parent_id);
        pileize_command->redo();
        actor_with_unique_id(parent_id)->set_selected(false);
      } else if (parents_ids_to_parents_types_[parent_id] == GRIDDED_PILE) {
        // create a new gridded pile with the same unique id as the original gridded pile
        GriddedPile* original_gridded_pile = new GriddedPile(ogre_scene_manager_, physics_,
                                                             this, actor_with_unique_id(actor_id)->position(),
                                                             parent_id);

        QList<Ogre::Vector3> offsets;
        QList<Ogre::Quaternion> orientations;
        for (int i = 0; i < 2; i++) {
          offsets.append(Ogre::Vector3::ZERO);
          orientations.append(Ogre::Quaternion::IDENTITY);
        }
        original_gridded_pile->initWithActors(actors_to_be_grouped, offsets,
                                              orientations, actor_with_unique_id(actor_id)->position());
      }
    }

    if (parents_ids_to_children_ids_[parent_id].count() == files_of_same_grouping_that_have_been_added_[parent_id]) {
      // remove the unique_id from expected_files_original_grouping_ and files_of_same_grouping_that_have_been_added_
      // once all the expected actors are added into room
      parents_ids_to_parents_types_.remove(parent_id);
      parents_ids_to_children_ids_.remove(parent_id);
    }
  }
}

void Room::set_position(Ogre::Vector3 position) {
  ogre_scene_node_->setPosition(position);
  update();
}

void Room::set_orientation(Ogre::Quaternion orientation) {
  ogre_scene_node_->setOrientation(orientation);
  update();
}

Ogre::SceneNode* Room::ogre_scene_node() {
  return ogre_scene_node_;
}

VisualPhysicsActorList Room::room_actor_list() {
  return room_actors_.values();
}

VisualPhysicsActorList Room::floor_actor_list() {
  VisualPhysicsActorList floor_actors;

  for_each(VisualPhysicsActor* actor, room_actor_list()) {
    if (actor->room_surface() == getSurface(FLOOR)) {
      floor_actors.push_back(actor);
    }
  }

  return floor_actors;
}

QHash<VisualPhysicsActorId, VisualPhysicsActor*> Room::room_actors() {
  return room_actors_;
}

bool Room::containsActorWithId(VisualPhysicsActorId actor_id) {
  return room_actors_.contains(actor_id);
}

QList<VisualPhysicsActor*> Room::flattenedChildren() {
  QList<VisualPhysicsActor*> children;
  for_each(VisualPhysicsActor* actor, room_actors_.values()) {
    children.push_back(actor);
    for_each(VisualPhysicsActor* child, actor->flattenedChildren()) {
      children.push_back(child);
    }
  }
  return children;
}

// Checks if an actor is contained in the room directly, or as a child of an
// actor that is contained directly.
bool Room::containsDirectlyOrThroughChild(VisualPhysicsActorId actor_id) {
  if (room_actors_.contains(actor_id)) {
    return true;
  }
  for_each(VisualPhysicsActor* actor, room_actors_.values()) {
    if (actor->flattenedChildrenIds().contains(actor_id)) {
      return true;
    }
  }
  return false;
}

Ogre::Vector3 Room::min() {
  return Ogre::Vector3(min_x(), min_y(), min_z());
}

Ogre::Vector3 Room::max() {
  return Ogre::Vector3(max_x(), max_y(), max_z());
}

Ogre::Real Room::min_x() {
  Ogre::Vector3 position = ogre_scene_node_->getPosition();
  return position.x;
}

Ogre::Real Room::max_x() {
  Ogre::Vector3 position = ogre_scene_node_->getPosition();
  return position.x + floor_width_;
}

Ogre::Real Room::min_z() {
  Ogre::Vector3 position = ogre_scene_node_->getPosition();
  return position.z;
}

Ogre::Real Room::max_z() {
  Ogre::Vector3 position = ogre_scene_node_->getPosition();
  return position.z + floor_depth_;
}

Ogre::Real Room::min_y() {
  Ogre::Vector3 position = ogre_scene_node_->getPosition();
  return position.y;
}

Ogre::Real Room::max_y() {
  Ogre::Vector3 position = ogre_scene_node_->getPosition();
  return position.y + wall_height_;
}

Ogre::Vector3 Room::center_of_floor() {
  return Ogre::Vector3(min_x() + floor_width_/2.0, 0, min_z() + floor_depth_/2.0);
}

Ogre::Real Room::floor_width() {
  return floor_width_;
}

Ogre::Real Room::floor_depth() {
  return floor_depth_;
}

Ogre::Real Room::wall_height() {
  return wall_height_;
}

const QString& Room::path() {
  return path_;
}

GriddedPileManager* Room::gridded_pile_manager() {
  return gridded_pile_manager_;
}

QList<RoomSurface*> Room::surfaces() {
  return room_surfaces_.values();
}

RoomSurface* Room::getSurface(RoomSurfaceType room_surface_type) {
  return room_surfaces_[room_surface_type];
}

void Room::update() {
  // TODO: updating may have some redundancy here
  ogre_scene_node_->_update(true, true);
  for_each(RoomSurface* surface, room_surfaces_.values()) {
    surface->update();
  }
  for_each(VisualPhysicsActor* actor, room_actors_.values()) {
    actor->update();
  }
}

bool Room::setAndAdjustMaterialForSurface(RoomSurfaceType room_surface_type, const QString& path, bool background_loaded) {  //  NOLINT
  QCryptographicHash sha1_hasher(QCryptographicHash::Sha1);
  sha1_hasher.addData(utf8(path).c_str());
  QString hashed_path = QString(sha1_hasher.result().toHex());

  QString background_path = FileManager::getBackgroundCachePath();
  QString cache_format = ".tiff";

  QDir background_dir = QDir(background_path);
  QSize source_resolution;

  try {
    if (!background_dir.exists(hashed_path + cache_format)) {
      source_resolution = MaterialLoader::copyImageAndChangeFormatBasedOnExtensions(path, background_path +
                                                                                    hashed_path + cache_format);
    } else {
      source_resolution = MaterialLoader::getImageResolution(background_path + hashed_path + cache_format);
    }
  } catch(const Ogre::Exception &e) {
    std::cout << e.getFullDescription() << std::endl;
    return false;
  }

  QSize max_image_resolution = MaterialLoader::maxResolutionForBackground();
  QSize desired_image_resolution = MaterialLoader::desiredBGSizeGivenMaxDimensionsAndSourceImageSize(max_image_resolution, source_resolution);  // NOLINT

  if (desired_image_resolution != source_resolution) {
    QString correctly_sized_background = background_path + hashed_path + "_" +
                                         QString::number(desired_image_resolution.width()) + "x" +
                                         QString::number(desired_image_resolution.height()) + cache_format;

    if (!background_dir.exists(correctly_sized_background)) {
      MaterialLoader::createImageWithResolution(background_path + hashed_path + cache_format,
                                                correctly_sized_background, desired_image_resolution);
    }

    applyMaterialForSurface(room_surface_type, correctly_sized_background, background_loaded);
  } else {
    applyMaterialForSurface(room_surface_type, background_path + hashed_path + cache_format, background_loaded);
  }
  return true;
}

void Room::applyMaterialForSurface(RoomSurfaceType room_surface_type, const QString& path, bool background_loaded) {
  QString old_material_name;
  if (room_surfaces_.contains(room_surface_type) && room_surfaces_[room_surface_type] != NULL)
    old_material_name = room_surfaces_[room_surface_type]->material_name();

  // this is a guard for the first time we want to load a material
  if (old_material_name != "") {
    QString end_overlay_tag = "<ENDOVERLAY>";
    int image_path_index = old_material_name.indexOf(end_overlay_tag);
    assert(image_path_index != -1);
    image_path_index += end_overlay_tag.size();
    QString old_texture_name = old_material_name.mid(image_path_index);

    BumpTextureManager::singleton()->decrementReferenceCountAndDeleteIfZero(old_texture_name);
    BumpMaterialManager::singleton()->decrementReferenceCountAndDeleteIfZero(old_material_name);
  }

  MaterialLoader *material = new MaterialLoader();
  assert(QObject::connect(material, SIGNAL(backgroundLoadingComplete(MaterialLoader*)),  // NOLINT
            this, SLOT(deleteMaterialLoader(MaterialLoader*))));  // NOLINT
  if (room_surface_type == FLOOR) {
    material->initAsImageAndOverlayWithFilePaths(path, "floor_overlay.png", background_loaded);
  } else {
    material->initAsImageAndOverlayWithFilePaths(path, "wall_overlay.png", background_loaded);
  }

  room_surfaces_[room_surface_type]->set_material_name(material->name());
  room_surfaces_[room_surface_type]->set_render_queue_group(Ogre::RENDER_QUEUE_BACKGROUND);
}

void Room::updateAllTexturesFromSettings(bool background_loaded) {
  if (AppSettings::singleton()->apply_floor_to_all_surfaces()) {
    if (!setAndAdjustMaterialForSurface(FLOOR, QStringFromUtf8(AppSettings::singleton()->floor_image_path()),
                                        background_loaded))
      setTextureSettingForSurface(FLOOR, QStringFromUtf8(kDefaultFloorTexturePath));
    if (!setAndAdjustMaterialForSurface(FRONT_WALL, QStringFromUtf8(AppSettings::singleton()->floor_image_path()),
                                        background_loaded))
      setTextureSettingForSurface(FLOOR, QStringFromUtf8(kDefaultFloorTexturePath));
    if (!setAndAdjustMaterialForSurface(BACK_WALL, QStringFromUtf8(AppSettings::singleton()->floor_image_path()),
                                        background_loaded))
      setTextureSettingForSurface(FLOOR, QStringFromUtf8(kDefaultFloorTexturePath));
    if (!setAndAdjustMaterialForSurface(RIGHT_WALL, QStringFromUtf8(AppSettings::singleton()->floor_image_path()),
                                        background_loaded))
      setTextureSettingForSurface(FLOOR, QStringFromUtf8(kDefaultFloorTexturePath));
    if (!setAndAdjustMaterialForSurface(LEFT_WALL, QStringFromUtf8(AppSettings::singleton()->floor_image_path()),
                                        background_loaded))
      setTextureSettingForSurface(FLOOR, QStringFromUtf8(kDefaultFloorTexturePath));
  } else {
    if (!setAndAdjustMaterialForSurface(FLOOR, QStringFromUtf8(AppSettings::singleton()->floor_image_path()),
                                        background_loaded))
      setTextureSettingForSurface(FLOOR, QStringFromUtf8(kDefaultFloorTexturePath));
    if (!setAndAdjustMaterialForSurface(FRONT_WALL, QStringFromUtf8(AppSettings::singleton()->front_wall_image_path()),
                                        background_loaded))
      setTextureSettingForSurface(FRONT_WALL, QStringFromUtf8(kDefaultFloorTexturePath));
    if (!setAndAdjustMaterialForSurface(BACK_WALL, QStringFromUtf8(AppSettings::singleton()->back_wall_image_path()),
                                        background_loaded))
      setTextureSettingForSurface(BACK_WALL, QStringFromUtf8(kDefaultFloorTexturePath));
    if (!setAndAdjustMaterialForSurface(RIGHT_WALL, QStringFromUtf8(AppSettings::singleton()->right_wall_image_path()),
                                        background_loaded))
      setTextureSettingForSurface(RIGHT_WALL, QStringFromUtf8(kDefaultFloorTexturePath));
    if (!setAndAdjustMaterialForSurface(LEFT_WALL, QStringFromUtf8(AppSettings::singleton()->left_wall_image_path()),
                                        background_loaded))
      setTextureSettingForSurface(LEFT_WALL, QStringFromUtf8(kDefaultFloorTexturePath));
  }
}

void Room::setTextureSettingForSurface(RoomSurfaceType wall_type, QString path) {
  QString original_path;
  switch (wall_type) {
    case FLOOR:
      original_path = QString(AppSettings::singleton()->floor_image_path().c_str());
      AppSettings::singleton()->set_floor_image_path(utf8(path));
      break;
    case FRONT_WALL:
      original_path = QStringFromUtf8(AppSettings::singleton()->front_wall_image_path());
      AppSettings::singleton()->set_front_wall_image_path(utf8(path));
      break;
    case BACK_WALL:
      original_path = QStringFromUtf8(AppSettings::singleton()->back_wall_image_path());
      AppSettings::singleton()->set_back_wall_image_path(utf8(path));
      break;
    case RIGHT_WALL:
      original_path = QStringFromUtf8(AppSettings::singleton()->right_wall_image_path());
      AppSettings::singleton()->set_right_wall_image_path(utf8(path));
      break;
    case LEFT_WALL:
      original_path = QStringFromUtf8(AppSettings::singleton()->left_wall_image_path());
      AppSettings::singleton()->set_left_wall_image_path(utf8(path));
      break;
  }

  // if no surface is using this texture, we need to delete all records of it from our cache
  if (QStringFromUtf8(AppSettings::singleton()->left_wall_image_path()) != original_path &&
      QStringFromUtf8(AppSettings::singleton()->right_wall_image_path()) != original_path &&
      QStringFromUtf8(AppSettings::singleton()->back_wall_image_path()) != original_path &&
      QStringFromUtf8(AppSettings::singleton()->front_wall_image_path()) != original_path &&
      QStringFromUtf8(AppSettings::singleton()->floor_image_path()) != original_path) {
    // comment to add clarity of where code block starts
    QCryptographicHash sha1_hasher(QCryptographicHash::Sha1);
    sha1_hasher.addData(utf8(original_path).c_str());
    QString hashed_path = QString(sha1_hasher.result().toHex());

    QDir background_dir = QDir(FileManager::getBackgroundCachePath());
    QStringList file_list = background_dir.entryList();
    for_each(QString file, file_list) {
      if (file.startsWith(hashed_path)) {
        background_dir.remove(file);
      }
    }
  }
}

SearchBox* Room::search_box() {
  return search_box_;
}

FileDropReceiver* Room::room_drop_receiver() {
  return room_drop_receiver_;
}

void Room::remove_hidden_drives_recently_added(Timer* timer) {
  VisualPhysicsActorId actor_id = drives_recently_added_.key(timer);
  assert(timer->disconnect());
  drives_recently_added_.remove(actor_id);
  delete timer;
  if (!containsActorWithId(actor_id)) {
    return;
  }
  VisualPhysicsActor* actor = actor_with_unique_id(actor_id);
  NSString* script_format = @"tell application \"Finder\" to local volume of (first disk whose name is \"%@\")";
  NSString* script = [NSString stringWithFormat:script_format, [NSStringFromQString(actor->path()) lastPathComponent]];
  NSDictionary* error;

  // Run script
  NSAppleScript *script_local_volume = [[NSAppleScript alloc] initWithSource:script];
  NSAppleEventDescriptor *script_return = [script_local_volume executeAndReturnError:&error];
  [script_local_volume release];

  NSLog(@"Loading Disk Properties:\n");

  // Drive can be removed while applescript is running
  if (!containsActorWithId(actor_id)) {
    NSLog(@"Error: %@\n", [error valueForKey:NSAppleScriptErrorMessage]);
    return;
  }
  NSLog(@"Success!\n");

  // Applescript did not run correctly
  if (script_return == nil) {
    return;
  }

  if ([script_return booleanValue]) {
    // Undo addition of external drives not shown
    if (!AppSettings::singleton()->desktop_shows_external_hard_disks()) {
      fileRemoved(actor->path());
    }    
  } else {
    // Undo addition of network drives not shown
    if (!AppSettings::singleton()->desktop_shows_connected_servers()){
      fileRemoved(actor->path());
    } else {
      FileManager::addConnectedServer(actor->path());
    }
  }
}

void Room::remove_hidden_drives() {
  for_each(VisualPhysicsActor* actor, flattenedChildren()) {
    if (FileManager::isStartupDrive(actor->path())
               && !AppSettings::singleton()->desktop_shows_hard_disks()) {
      FileSystemEventDispatcher::singleton()->fileRemoved(actor->path());
    } else if (FileManager::isVolume(actor->path())
               && !FileManager::isStartupDrive(actor->path())
               && !FileManager::isConnectedServer(actor->path())
               && !AppSettings::singleton()->desktop_shows_external_hard_disks()) {
      // Any volume not that isn't the startup drive and isn't a connected server is assumed to be an external drive
      FileSystemEventDispatcher::singleton()->fileRemoved(actor->path());
    } else if (FileManager::isConnectedServer(actor->path())
               && !AppSettings::singleton()->desktop_shows_connected_servers()) {
      FileSystemEventDispatcher::singleton()->fileRemoved(actor->path());
    }
  }
}

void Room::set_new_items_pile (VisualPhysicsActorId actor_id) {
  if (!containsActorWithId(actor_id)) {
    return;
  }
  VisualPhysicsActor* actor = actor_with_unique_id(actor_id);
  NewItemsPile* pile;
  if (actor->actor_type() == BUMP_PILE) {
    pile = new NewItemsPile(app_->ogre_scene_manager(), physics_, this, actor_id);
    VisualPhysicsActorList children = actor->children();
    Ogre::Vector3 position = actor->position();
    QList<Ogre::Vector3> offsets = actor->children_offsets();
    QList<Ogre::Quaternion> orientations = actor->children_orientations();
    actor->breakPile();
    delete actor;
    
    pile->initWithActors(children, position, offsets, orientations);
  } else {
    pile = new NewItemsPile(app_->ogre_scene_manager(), physics_, this);
    pile->initWithActors(VisualPhysicsActorList() << actor, actor->position(),
                         QList<Ogre::Vector3>() << Ogre::Vector3::ZERO,
                         QList<Ogre::Quaternion>() << Ogre::Quaternion::IDENTITY);
  }

  pile->stackViewOnInit();

  new_items_pile_id_ = pile->unique_id();
}

VisualPhysicsActorId Room::new_items_pile() {
  return new_items_pile_id_;
}

#include "moc/moc_Room.cpp"
