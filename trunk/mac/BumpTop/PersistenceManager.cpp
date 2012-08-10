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

#include "BumpTop/PersistenceManager.h"

#include <fstream>  // NOLINT
#include <string>
#include <vector>

#include "BumpTop/AppSettings.h"
#include "BumpTop/ArrayOnStack.h"
#include "BumpTop/BumpBox.h"
#include "BumpTop/BumpFlatSquare.h"
#include "BumpTop/BumpTopApp.h"
#include "BumpTop/BumpTopInstanceLock.h"
#include "BumpTop/DesktopItems.h"
#include "BumpTop/FileManager.h"
#include "BumpTop/protoc/AllMessages.pb.h"
#include "BumpTop/ProtocolBufferHelpers.h"
#include "BumpTop/QStringHelpers.h"
#include "BumpTop/Room.h"
#include "BumpTop/RoomItemPoseConstraints.h"
#include "BumpTop/VisualPhysicsActorList.h"

QString getRoomBufferFileName() {
  if (BumpTopInstanceLock::is_running_in_sandbox()) {
    return "Room.sandbox.bump";
  } else {
    return "Room.bump";
  }
}

// Add default picture frame
void addHangingActorOnFirstRun(Room* room) {
  QDir other_desktop_items_path = QDir(FileManager::getApplicationDataPath() + "OtherDesktopItems/");
  QFile pin_me;
  pin_me.setFileName(FileManager::pathForResource("PinMe.png"));
  if (!other_desktop_items_path.exists()) {
    other_desktop_items_path.mkpath(other_desktop_items_path.path());
    pin_me.copy(other_desktop_items_path.absoluteFilePath("PinMe.png"));

    BumpBox* box = new BumpFlatSquare(BumpTopApp::singleton()->ogre_scene_manager(),
                                      BumpTopApp::singleton()->physics(), room);
    box->initWithPath(other_desktop_items_path.absoluteFilePath("PinMe.png"));
    box->set_position(Ogre::Vector3(200, 150, 100));
    box->set_size(Ogre::Vector3(2*kInitialActorSize, 2*kInitialActorSize, 2*kInitialActorSize));
    box->set_pose(box->getPoseForSurface(room->getSurface(BACK_WALL)));
    box->set_room_surface(room->getSurface(BACK_WALL));
    box->pinToSurface(room->getSurface(BACK_WALL));
  } else {
    if (versionStringLessThanVersionString(AppSettings::singleton()->last_run_bumptop_version(),
                                           BumpTopApp::singleton()->bumptopVersion()) &&
        versionStringLessThanVersionString(AppSettings::singleton()->last_run_bumptop_version(), QString("0.1.8"))) {
        QFile(other_desktop_items_path.absoluteFilePath("PinMe.png")).remove();
        pin_me.copy(other_desktop_items_path.absoluteFilePath("PinMe.png"));
    }
  }
}

bool isFileInRoom(QString path, Room* room) {
  for_each(VisualPhysicsActor* actor, room->room_actor_list()) {
    if (path == actor->path()) {
      return true;
    } else if (!actor->children().empty()) {
      for_each(VisualPhysicsActor* child, actor->children()) {
        if (path == child->path()) {
          return true;
        }
      }
    }
  }
  return false;
}

// Loads the BumpTop desktop based on the items / locations on the user's desktop
void loadRoomFromDesktop(Room* room) {
  Ogre::Vector2 window_size = BumpTopApp::singleton()->screen_resolution();
#if defined(OS_WIN)
  room->init(window_size.x/4, window_size.y/4, kRoomWallHeight);
#else
  room->init(window_size.x, window_size.y, kRoomWallHeight);
#endif
  room->setCameraForRoom(false);

  if (room->path() == FileManager::getDesktopPath()) {
    // Get desktop items
    for_each(DesktopItem desktop_item, getDesktopItems()) {
      if (!isFileInRoom(desktop_item.file_path, room)) {
        BumpBox* box = new BumpFlatSquare(BumpTopApp::singleton()->ogre_scene_manager(),
                                          BumpTopApp::singleton()->physics(), room);
        box->initWithPath(desktop_item.file_path);
        box->set_position(Ogre::Vector3(desktop_item.position_x, 100, desktop_item.position_y));
        box->set_size(Ogre::Vector3(kInitialActorSize, kInitialActorSize, kInitialActorSize));
        FileManager::getAndSetLabelColourThroughNSTask(box->unique_id());
      }
    }
  } else {
    QDir room_dir(room->path());
    for_each(QFileInfo file_info, room_dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot)) {
      QString file_path = file_info.absoluteFilePath();
      BumpBox* box = new BumpFlatSquare(BumpTopApp::singleton()->ogre_scene_manager(),
                                        BumpTopApp::singleton()->physics(), room);

      box->initWithPath(file_path);  // NOLINT
      box->set_position(Ogre::Vector3(room->floor_width()/2, 100, room->floor_depth()/2));
      box->set_size(Ogre::Vector3(kInitialActorSize, kInitialActorSize, kInitialActorSize));
      FileManager::getAndSetLabelColourThroughNSTask(box->unique_id());
    }
  }
  addHangingActorOnFirstRun(room);
  room->addStickyPadInDefaultLocation();
  room->addNewItemsPileInDefaultLocation();
  room->constrainRoomActorPoses();

  AppSettings::singleton()->updateFinderPreferences();
  room->remove_hidden_drives();
}

// Loads the BumpTop desktop from a locally saved serialized version of a Room
bool loadRoomFromFile(Room* room, QString path, Ogre::Vector2 window_size) {
  // read in dat file for room
  RoomBuffer room_buffer;
  if (!loadBufferFromFile(&room_buffer, path + "/" + getRoomBufferFileName())) {
    return false;
  }

  room->initFromBuffer(&room_buffer, window_size);
  addHangingActorOnFirstRun(room);

  AppSettings::singleton()->updateFinderPreferences();
  room->remove_hidden_drives();
  return true;
}

// Saves a seriallized version of a Room locally
bool writeRoomToFile(Room* room, QString path) {
  RoomBuffer room_buffer;
  room->writeToBuffer(&room_buffer);
  return saveBufferToFile(&room_buffer, path + "/" + getRoomBufferFileName());
}
