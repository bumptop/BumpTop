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

#include "BumpTop/QuickLookPreviewPanel.h"

#include "BumpTop/BumpTopApp.h"
#  if (MAC_OS_X_VERSION_MAX_ALLOWED == MAC_OS_X_VERSION_10_6)
#include "BumpTop/OSX/QuickLookSnowLeopard.h"
#endif
#include "BumpTop/Room.h"
#include "BumpTop/VisualPhysicsActor.h"
#include "BumpTop/OSX/OSXCocoaBumpTopApplication.h"
#include "BumpTop/OSX/QuickLookLeopard.h"

QuickLookPreviewPanel::QuickLookPreviewPanel(Room* room)
: room_(room),
  preview_panel_needs_updating_(false) {
  assert(QObject::connect(room_, SIGNAL(onSelectedActorsChanged()),  // NOLINT
                          this, SLOT(selectedActorsChanged())));  // NOLINT
    if (NSClassFromString(@"QLPreviewPanel") != NULL) {
#  if (MAC_OS_X_VERSION_MAX_ALLOWED == MAC_OS_X_VERSION_10_6)
      if (OSXCocoaBumpTopApplication::isRunningLeopardOrEarlier())
        quick_look_interface_ = new LeopardQuickLookInterface();
      else
        quick_look_interface_ = new SnowLeopardQuickLookInterface();
#  else
      quick_look_interface_ = new LeopardQuickLookInterface();
#endif
    }
}

QuickLookPreviewPanel::~QuickLookPreviewPanel() {
}

void QuickLookPreviewPanel::toggle() {
  if (quick_look_interface_->sharedPreviewPanelIsOpen()) {
    quick_look_interface_->closeSharedPreviewPanel();
  } else {
    quick_look_interface_->previewPathsInSharedPreviewPanel(getRoomSelectedActorPaths());
  }
}

void QuickLookPreviewPanel::selectedActorsChanged() {
  if (quick_look_interface_->sharedPreviewPanelIsOpen()) {
    previewPanelNeedsUpdating();
  }
}

void QuickLookPreviewPanel::previewPanelNeedsUpdating() {
  if (!preview_panel_needs_updating_) {
    preview_panel_needs_updating_ = true;
    assert(QObject::connect(BumpTopApp::singleton(), SIGNAL(onRender()),  // NOLINT
                            this, SLOT(renderTick())));  // NOLINT
  }
}

void QuickLookPreviewPanel::renderTick() {
  if (quick_look_interface_->sharedPreviewPanelIsOpen()) {
    quick_look_interface_->setSharedPreviewPanelPaths(getRoomSelectedActorPaths());
  }

  assert(QObject::disconnect(BumpTopApp::singleton(), SIGNAL(onRender()),  // NOLINT
                           this, SLOT(renderTick())));  // NOLINT
  preview_panel_needs_updating_ = false;
}

QHash<QString, VisualPhysicsActorId> QuickLookPreviewPanel::getRoomSelectedActorPaths() {
  VisualPhysicsActorList selected_actors = room_->selected_actors();
  QHash<QString, VisualPhysicsActorId> paths;

  // Initialize the file system watcher so that it uses as its reference point the state that the room is in
  for_each(VisualPhysicsActor* actor, selected_actors) {
    if (actor->path() != "") {
      paths.insert(actor->path(), actor->unique_id());
    }
    QStringList decendants_paths = actor->pathsOfDescendants();
    for_each(QString decendant_path, decendants_paths) {
      if (decendant_path != "") {
        paths.insert(decendant_path, actor->unique_id());
      }
    }
  }
  if (paths.count()  == 0) {
    paths.insert(room_->path(), 0);
  }
  return paths;
}


#include "moc/moc_QuickLookPreviewPanel.cpp"

