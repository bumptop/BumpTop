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

#ifndef BUMPTOP_SEARCHBOX_H_
#define BUMPTOP_SEARCHBOX_H_

class BumpBox;
class BumpPile;
class FileItem;
class SearchBoxOverlay;
class VisualPhysicsActor;

#include <utility>

#include "BumpTop/Timer.h"
#include "BumpTop/VisualPhysicsActorList.h"

class Room;

class SearchBox : public QObject {
  Q_OBJECT
 public:
  explicit SearchBox(Room* room);
  virtual ~SearchBox();

  void init();
  void startSearch(QString key);
  void findMatch(VisualPhysicsActorList room_actor);
  void updateSearchBox();
  void deleteSearchBox();
  void updateSearchBoxPosition();
  std::pair<int, VisualPhysicsActorList > matchingActors(VisualPhysicsActorList room_actors);
  int numberOfMatchesFoundInActor(VisualPhysicsActor* actor);
  void addToUserSearchString(QString key);
  void deleteLastKeyFromUserSearchString();
  void activateSearchBox();
  void deactivateSearchBox();
  virtual bool is_open();

 public slots: // NOLINT
  void startFadingAnimation();
  void hideOverlayAfterFading();

 signals:
  void onSearchBoxDismissed();
  void onSearchBoxActivated();

 protected:
  SearchBoxOverlay* overlay_;
  Room* room_;
  QString user_search_string_;
  QString overlay_text_;
  Timer timer_;
  bool is_open_;
};

#endif  // BUMPTOP_SEARCHBOX_H_
