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

#include "BumpTop/AnimationManager.h"
#include "BumpTop/AppSettings.h"
#include "BumpTop/BumpBoxLabel.h"
#include "BumpTop/BumpTopApp.h"
#include "BumpTop/BumpTopScene.h"
#include "BumpTop/BumpTopCommands.h"
#include "BumpTop/CommandButton.h"
#include "BumpTop/CustomQLineEdit.h"
#include "BumpTop/FileItem.h"
#include "BumpTop/FileManager.h"
#include "BumpTop/UndoCommands/PileizeUndoCommand.h"
#include "BumpTop/UndoCommands/PileBreakUndoCommand.h"
#include "BumpTop/UndoCommands/PileByTypeUndoCommand.h"
#include "BumpTop/UndoCommands/ShrinkUndoCommand.h"
#include "BumpTop/UndoCommands/GrowUndoCommand.h"
#include "BumpTop/UndoCommands/GridViewUndoCommand.h"
#include "BumpTop/UndoCommands/MoveFileOutOfBumpTopUndoCommand.h"
#include "BumpTop/OSX/OSXCocoaBumpTopApplication.h"
#include "BumpTop/QStringHelpers.h"
#include "BumpTop/Room.h"
#include "BumpTop/FileDropReceiver.h"
#include "BumpTop/RoomSurface.h"
#include "BumpTop/UndoRedoStack.h"
#include "BumpTop/VisualPhysicsActor.h"

// Commands for Items
// Finder commands
// number of separator above me = 0
const int kPositionOfOpen = 1;
const int kPositionOfOpenWith = 2;

// number of separator above me = 1
// specific to drives
const int kPositionOfEject = 1;

// specific to aliases
const int kPositionOfShowOriginal = 1;

// to all except for drives
const int kPositionOfMoveToTrash = 2;

// number of separator above me = 2
const int kPositionOfGetInfo = 1;
const int kPositionOfCompress = 2;
const int kPositionOfDuplicate = 3;
const int kPositionOfMakeAlias = 4;
const int kPositionOfHideFilename = 5;
const int kPositionOfShowFilename = 6;

// BumpTop commands
// number of separator above me = 3
// specific to more than 1 item selected
const int kPositionOfCreatePile = 1;
const int kPositionOfGridView = 2;
const int kPositionOfPileByTypeForSelectedActors = 3;
const int kPositionOfCopy = 4;
const int kPositionOfPaste = 5;

// specific to only 1 pile selected
const int kPositionOfBreakPile = 1;
const int kPositionOfRename = 2;

// Label Colours
// nuber of seperator above me = 4
const int kPositionOfChangeLabelColour = 1;

// Sorting commands
// number of separator above me = 5
const int kPositionOfSortAlphabetically = 1;

// number of separator above me = 6
const int kPositionOfGrow = 1;
const int kPositionOfShrink = 2;

//////////////////////////////////////////////////////////////////////////

// Commands for Desktop
// number of separator above me = 0
const int kPositionOfNewFolder = 1;

// number of separator above me = 1

// number of separator above me = 2
// const int kPositionOfGetInfo = 1  // this is already defined above

// number of separator above me = 3
const int kPositionOfUndo = 1;
const int kPositionOfRedo = 2;

// number of separator above me = 4
const int kPositionOfChangeBackground = 1;
const int kPositionOfPileByTypeForAllActors = 2;


QStringList BumpTopCommand::pending_archive_zip_paths;

BumpEnvironment::BumpEnvironment()
: physics(NULL),
  room(NULL),
  ogre_scene_manager(NULL) {
}

BumpEnvironment::BumpEnvironment(Physics* physics, Room* room, Ogre::SceneManager* ogre_scene_manager)
: physics(physics),
  room(room),
  ogre_scene_manager(ogre_scene_manager) {
}

BumpTopCommandSet* MakeQSet(int count, ...) {
  va_list arguments_pointer;
  BumpTopCommandSet* set = new BumpTopCommandSet();
  va_start(arguments_pointer, count);
  for (int i = 0; i < count; i++)
    set->insert(va_arg(arguments_pointer, BumpTopCommand*));
  va_end(arguments_pointer);
  return set;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BumpTopCommand::~BumpTopCommand() {
}

bool BumpTopCommand::canBeAppliedToActors(const BumpEnvironment& env, VisualPhysicsActorList actors) {
  return true;
};

bool BumpTopCommand::isSeparator() {
  return false;
};

bool BumpTopCommand::has_subcommands() {
  return false;
}

QStringList BumpTopCommand::subcommand_names(VisualPhysicsActorList actors) {
  return QStringList();
}

CGImageRef BumpTopCommand::iconForSubcommand(VisualPhysicsActorList actors, int subcommand) {
  return NULL;
}

bool BumpTopCommand::is_toolbar_command() {
  return false;
}

CommandButton* BumpTopCommand::command_button() {
  return NULL;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


BumpTopToolbarCommand::BumpTopToolbarCommand()
: toolbar_button_material_inactive_(""),
  toolbar_button_material_active_(""),
  command_button_(NULL) {
}

BumpTopToolbarCommand::~BumpTopToolbarCommand() {
}

bool BumpTopToolbarCommand::is_toolbar_command() {
  return true;
}

bool BumpTopToolbarCommand::should_dismiss_toolbar() {
  return false;
}

CommandButton* BumpTopToolbarCommand::command_button() {
  if (command_button_ == NULL) {
    command_button_ = new CommandButton(this);
    command_button_->init();
  }
  return command_button_;
}

QString BumpTopToolbarCommand::toolbar_button_material_inactive() {
  if (toolbar_button_material_inactive_ == "") {
    MaterialLoader material_loader;
    material_loader.initAsImageWithFilePath(toolbar_button_path_inactive(), false);
    toolbar_button_material_inactive_ = material_loader.name();
  }
  return toolbar_button_material_inactive_;
}

QString BumpTopToolbarCommand::toolbar_button_material_active() {
  if (toolbar_button_material_active_ == "") {
    MaterialLoader material_loader;
    material_loader.initAsImageWithFilePath(toolbar_button_path_active(), false);
    toolbar_button_material_active_ = material_loader.name();
  }
  return toolbar_button_material_active_;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SINGLETON_IMPLEMENTATION(Undo)

QString Undo::name() {
  return "Undo";
}

int Undo::number_of_separators_above_me() {
  return 3;
}

int Undo::position_within_my_category() {
  return kPositionOfUndo;
}

void Undo::applyToActors(const BumpEnvironment& env, VisualPhysicsActorList actors, int subcommand) {
  execute(env);
}

void Undo::execute(const BumpEnvironment& env) {
  env.room->updateCurrentState();
  env.room->undo_redo_stack()->undo(env.room->current_state());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SINGLETON_IMPLEMENTATION(Redo)

QString Redo::name() {
  return "Redo";
}

int Redo::number_of_separators_above_me() {
  return 3;
}

int Redo::position_within_my_category() {
  return kPositionOfRedo;
}

void Redo::applyToActors(const BumpEnvironment& env, VisualPhysicsActorList actors, int subcommand) {
  execute(env);
}

void Redo::execute(const BumpEnvironment& env) {
  env.room->updateCurrentState();
  env.room->undo_redo_stack()->redo(env.room->current_state());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SINGLETON_IMPLEMENTATION(CreatePile)

QString CreatePile::name() {
  return "Create Pile";
}

int CreatePile::number_of_separators_above_me() {
  return 3;
}

int CreatePile::position_within_my_category() {
  return kPositionOfCreatePile;
}

void CreatePile::applyToActors(const BumpEnvironment& env, VisualPhysicsActorList actors, int subcommand) {
  PileizeUndoCommand* pileize_command = new PileizeUndoCommand(actors, env.room, env.ogre_scene_manager, env.physics);
  env.room->updateCurrentState();
  env.room->undo_redo_stack()->push(pileize_command, env.room->current_state());
}

bool CreatePile::canBeAppliedToActors(const BumpEnvironment& env, VisualPhysicsActorList list_of_actors) {
  int valid_actor_count = 0;
  for_each(VisualPhysicsActor* actor, list_of_actors) {
    if (!actor->room_surface()->is_pinnable_receiver()) {
      valid_actor_count++;
    }
  }
  return valid_actor_count >= 2;
}

QString CreatePile::toolbar_button_path_inactive() {
  return FileManager::getResourcePath() + "/create_pile.png";
}

QString CreatePile::toolbar_button_path_active() {
  return FileManager::getResourcePath() + "/create_pile_active.png";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SINGLETON_IMPLEMENTATION(BreakPile)

QString BreakPile::name() {
  return "Break Pile";
}

int BreakPile::number_of_separators_above_me() {
  return 3;
}

int BreakPile::position_within_my_category() {
  return kPositionOfBreakPile;
}

void BreakPile::applyToActors(const BumpEnvironment& env, VisualPhysicsActorList actors, int subcommand) {
  PileBreakUndoCommand* pile_break_command = new PileBreakUndoCommand(actors, env.room,
                                                                      env.ogre_scene_manager, env.physics);
  env.room->updateCurrentState();
  env.room->undo_redo_stack()->push(pile_break_command, env.room->current_state());
}

bool BreakPile::canBeAppliedToActors(const BumpEnvironment& env, VisualPhysicsActorList actors) {
  for_each(VisualPhysicsActor* actor, actors) {
    if (actor->parent() != NULL)
      return false;
  }
  return true;
}

QString BreakPile::toolbar_button_path_inactive() {
  return FileManager::getResourcePath() + "/break_pile.png";
}

QString BreakPile::toolbar_button_path_active() {
  return FileManager::getResourcePath() + "/break_pile_active.png";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SINGLETON_IMPLEMENTATION(Open)

QString Open::name() {
  return "Open";
}

int Open::number_of_separators_above_me() {
  return 0;
}

int Open::position_within_my_category() {
  return kPositionOfOpen;
}

void Open::applyToActors(const BumpEnvironment& env, VisualPhysicsActorList actors, int subcommand) {
  for_each(VisualPhysicsActor* actor, actors) {
    actor->launch();
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SINGLETON_IMPLEMENTATION(OpenWith)

QString OpenWith::name() {
  return "Open With";
}

int OpenWith::number_of_separators_above_me() {
  return 0;
}

int OpenWith::position_within_my_category() {
  return kPositionOfOpenWith;
}

bool OpenWith::has_subcommands() {
  return true;
}

#import <Cocoa/Cocoa.h>
#include "BumpTop/OSX/FileIconLoader.h"
QStringList OpenWith::CFStringArrayTOQStringList(CFArrayRef array) {
  QStringList strings;
  if (array) {
    for (int i = 0; i < CFArrayGetCount(array); i++) {
      strings.push_back(QStringFromNSString([((NSURL*) CFArrayGetValueAtIndex(array, i)) path]));  // NOLINT
    }
  }
  return strings;
}

bool shortenedLessThan(const QString &s1, const QString &s2) {
  return (OpenWith::singleton()->shortenedName(s1) < OpenWith::singleton()->shortenedName(s2));
}

QStringList OpenWith::removeDuplicateApplicationInstances(QStringList app_list) {
  QStringList app_list_copy = app_list;
  QStringList apps_to_remove;
  // remove multiple instances of a given application -- default to the one stored
  // in /Applications, or if none is stored there, choose one randomly
  for_each(QString app, app_list) {
    QString short_name = shortenedName(app);
    int count = 0;
    bool duplicate_found_in_applications_folder = false;
    for_each(QString app_copy, app_list_copy) {
      if (app_copy.contains("/" + short_name + ".app")) {
        count++;
        if (app_copy == "/Applications/" + short_name + ".app")
          duplicate_found_in_applications_folder = true;
      }
    }
    // there are duplicates
    if (count > 1) {
      int count_2 = 0;
      apps_to_remove.clear();
      for_each(QString app_copy, app_list_copy) {
        if (app_copy.contains("/" + short_name + ".app")) {
          count_2++;
          if (duplicate_found_in_applications_folder && app_copy != "/Applications/" + short_name + ".app")
            apps_to_remove.push_back(app_copy);
          else if (!duplicate_found_in_applications_folder) {
            if (count_2 < count) {
              apps_to_remove.push_back(app_copy);
            } else {
              break;
            }
          }
        }
      }
      for_each(QString app_to_remove, apps_to_remove)
        app_list_copy.removeAll(app_to_remove);
    }
  }
  return app_list_copy;
}

QStringList OpenWith::validApplicationList(VisualPhysicsActorList actors) {
  QSet<QString> candidate_applications;
  QString default_application;

  // ^ represents exclusive or, so in this case represents substraction
  int roles = kLSRolesAll ^ kLSRolesNone ^ kLSRolesShell;

  if (actors.count() > 0) {
    // Get complete list of candidate applications
    CFURLRef path_url = (CFURLRef) [NSURL fileURLWithPath:NSStringFromQString(actors[0]->path())];
    candidate_applications = CFStringArrayTOQStringList(LSCopyApplicationURLsForURL(path_url, roles)).toSet();

    // Get the default application for this file
    CFURLRef default_app_url = NULL;
    FSRef default_app_fsref;
    LSGetApplicationForURL(path_url, roles, &default_app_fsref, &default_app_url);
    if (default_app_url != NULL)
      default_application = QStringFromNSString([(NSURL*) default_app_url path]);  // NOLINT
  }

  // here we use set intersection to get a list of a common applications for each file
  // set intersection is denoted in short-hand by &=
  for (int i = 1; i < actors.count(); i++) {
    // Get complete list of candidate applications
    CFURLRef path_url = (CFURLRef) [NSURL fileURLWithPath:NSStringFromQString(actors[i]->path())];
    candidate_applications &= CFStringArrayTOQStringList(LSCopyApplicationURLsForURL(path_url, roles)).toSet();

    // Get the default application for this file
    CFURLRef default_app_url = NULL;
    FSRef default_app_fsref;
    LSGetApplicationForURL(path_url, roles, &default_app_fsref, &default_app_url);
    if (default_app_url != NULL) {
      QString check_default_app = QStringFromNSString([(NSURL*) default_app_url path]);  // NOLINT

      // if the files don't share a common default application, then no default will be displayed
      if (check_default_app != default_application)
        default_application = "";
    }
  }

  // rearrange the list to have the default application at the top and add a spacer
  QStringList sorted_list = candidate_applications.toList();
  qSort(sorted_list.begin(), sorted_list.end(), shortenedLessThan);
  if (default_application != "") {
    sorted_list.removeAll(default_application);
    sorted_list.push_front("");  // spacer
    sorted_list.push_front(default_application);
  }

  return removeDuplicateApplicationInstances(sorted_list);
}

CGImageRef OpenWith::iconForSubcommand(VisualPhysicsActorList actors, int subcommand) {
  QStringList application_list = validApplicationList(actors);
  if (subcommand < application_list.count())
    return iconForContextMenu(application_list[subcommand]);
  else
    return NULL;
}

QString OpenWith::shortenedName(QString app_path) {
  return QFileInfo(app_path).completeBaseName();
}

QStringList OpenWith::subcommand_names(VisualPhysicsActorList actors) {
  QStringList names = validApplicationList(actors);
  QStringList shortened_names;
  for_each(QString name, names) {
    shortened_names.push_back(shortenedName(name));
  }

  // If the second item is a spacer (ie. ""), then the first is the default app
  // and we indicate that in the name
  if (shortened_names.count() >= 2)
    if (shortened_names[1] == "")
      shortened_names[0].append(" (default)");

  shortened_names.push_back("");
  shortened_names.push_back("Other...");

  return shortened_names;
}

bool OpenWith::canBeAppliedToActors(const BumpEnvironment& env, VisualPhysicsActorList actors) {
  for_each(VisualPhysicsActor* actor, actors) {
    QString original_path = actor->path();
    QFileInfo file_info = QFileInfo(actor->path());
    FileKind file_kind = FileManager::getFileKind(actor->path());
    if (file_kind == ALIAS) {
      original_path = QFileInfo(actor->path()).readLink();
      file_kind = FileManager::getFileKind(original_path);
    }

    if (file_kind == VOLUME || file_kind == APPLICATION ||
        file_info.isDir() || !file_info.exists()) {
      return false;
    }
  }
  return true;
}

void OpenWith::applyToActors(const BumpEnvironment& env, VisualPhysicsActorList actors, int subcommand) {
  QStringList application_list = validApplicationList(actors);
  if (subcommand < application_list.count()) {
    QString app = application_list[subcommand];

    for_each(VisualPhysicsActor* actor, actors) {
      actor->launch(app);
    }
  // If the subcommand index surpasses the valid application list, the user must have selected
  // Open With "Other"
  } else {
    QStringList paths;
    for_each(VisualPhysicsActor* actor, actors)
      paths.push_back(actor->path());
    FileManager::chooseApplicationAndLaunchPaths(paths);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SINGLETON_IMPLEMENTATION(MoveToTrash)

QString MoveToTrash::name() {
  return "Move to Trash";
}

int MoveToTrash::number_of_separators_above_me() {
  return 1;
}

int MoveToTrash::position_within_my_category() {
  return kPositionOfMoveToTrash;
}

void MoveToTrash::applyToActors(const BumpEnvironment& env, VisualPhysicsActorList actors, int subcommand) {
  QString target_path = FileManager::getUserPath() + "/.trash";
  MoveFileOutOfBumpTopUndoCommand* move_to_trash_undo_command = new MoveFileOutOfBumpTopUndoCommand(actors,
                                                                                                    false,
                                                                                                    MOVE_TO_TRASH_COMMAND,  // NOLINT
                                                                                                    env.room,
                                                                                                    target_path);
  env.room->updateCurrentState();
  env.room->undo_redo_stack()->push(move_to_trash_undo_command, env.room->current_state(), true);
}

bool MoveToTrash::canBeAppliedToActors(const BumpEnvironment& env, VisualPhysicsActorList actors) {
  for_each(VisualPhysicsActor* actor, actors) {
    if (FileManager::getFileKind(actor->path()) == VOLUME) {
      return false;
    } else {
      for_each(VisualPhysicsActor* child, actor->children()) {
        if (FileManager::getFileKind(child->path()) == VOLUME) {
          return false;
        }
      }
    }
  }
  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SINGLETON_IMPLEMENTATION(Eject)

QString Eject::name() {
  return "Eject" + name_;
}

int Eject::number_of_separators_above_me() {
  return 1;
}

int Eject::position_within_my_category() {
  return kPositionOfEject;
}

void Eject::applyToActors(const BumpEnvironment& env, VisualPhysicsActorList actors, int subcommand) {
  // we need to save the ids on the stack, and then do eject, because a previous eject command might eject multiple
  // drives
  QList<VisualPhysicsActorId> actor_ids;
  for_each(VisualPhysicsActor* actor, actors) {
    actor_ids.push_back(actor->unique_id());
  }
  for (int i = 0; i < actors.size(); i++) {
    VisualPhysicsActorId actor_id = actor_ids[i];
    if (env.room->containsDirectlyOrThroughChild(actor_id)) {
      VisualPhysicsActor* actor = actors[i];
      FileManager::ejectDrive(actor->path());
    }
  }
}

bool Eject::canBeAppliedToActors(const BumpEnvironment& env, VisualPhysicsActorList actors) {
  for_each(VisualPhysicsActor* actor, actors) {
    if (!FileManager::isEjectableDrive(actor->path())) {
      return false;
    }
  }
  name_ = "";
  if (actors.count() == 1)
    name_ = " \"" + QFileInfo(actors[0]->path()).fileName() + "\"";
  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SINGLETON_IMPLEMENTATION(SortAlphabetically)

QString SortAlphabetically::name() {
  return "Sort Alphabetically";
}

int SortAlphabetically::number_of_separators_above_me() {
  return 5;
}

int SortAlphabetically::position_within_my_category() {
  return kPositionOfSortAlphabetically;
}

void SortAlphabetically::applyToActors(const BumpEnvironment& env, VisualPhysicsActorList actors, int subcommand) {
  if (canBeAppliedToActors(env, actors)) {
    for_each(VisualPhysicsActor* actor, actors) {
      actor->sortAlphabetically();
    }
  }
}

bool SortAlphabetically::canBeAppliedToActors(const BumpEnvironment& env, VisualPhysicsActorList list_of_actors) {
  for_each(VisualPhysicsActor* actor, list_of_actors) {
    if (actor->children().count() == 0) {
      return false;
    }
  }
  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SINGLETON_IMPLEMENTATION(GetInfo)

QString GetInfo::name() {
  return "Get Info";
}

int GetInfo::number_of_separators_above_me() {
  return 2;
}

int GetInfo::position_within_my_category() {
  return kPositionOfGetInfo;
}

void GetInfoHelper(const QString& file_path) {
  NSString* script_format = @"tell application \"Finder\"\n"
                                @"set macpath to POSIX file \"%@\" as text\n"
                                @"open information window of item macpath\n"
                            @"end tell";
  NSString* script = [NSString stringWithFormat:script_format, NSStringFromQString(file_path)];

  FileManager::runAppleScriptThroughNSTask(script);
}

void GetInfo::applyToActors(const BumpEnvironment& env, VisualPhysicsActorList actors, int subcommand) {
  for_each(VisualPhysicsActor* actor, actors)
    GetInfoHelper(actor->path());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SINGLETON_IMPLEMENTATION(HideFilename)

QString HideFilename::name() {
  return name_;
}

int HideFilename::number_of_separators_above_me() {
  return 2;
}

int HideFilename::position_within_my_category() {
  return kPositionOfHideFilename;
}

void HideFilename::applyToActors(const BumpEnvironment& env, VisualPhysicsActorList actors, int subcommand) {
  if (canBeAppliedToActors(env, actors)) {
    for_each(VisualPhysicsActor* actor, actors) {
      actor->set_name_hidden(true);
    }
  }
}

bool HideFilename::canBeAppliedToActors(const BumpEnvironment& env, VisualPhysicsActorList list_of_actors) {
  for_each(VisualPhysicsActor* actor, list_of_actors) {
    if (actor->children().count() > 0) {
      return false;
    }
    if (AppSettings::singleton()->image_name_on_walls_hidden() && actor->is_an_image_on_wall()) {
      return false;
    }
  }
  name_ = "Hide Filename";
  if (list_of_actors.size() > 1) {
    name_ += "s";
  }
  for_each(VisualPhysicsActor* actor, list_of_actors) {
    // if not directly contained by room
    if (!env.room->containsActorWithId(actor->unique_id())) {
      return false;
    }
  }
  for_each(VisualPhysicsActor* actor, list_of_actors) {
    if (actor->label_visible()) {
      return true;
    }
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SINGLETON_IMPLEMENTATION(ShowFilename)

QString ShowFilename::name() {
  return name_;
}

int ShowFilename::number_of_separators_above_me() {
  return 2;
}

int ShowFilename::position_within_my_category() {
  return kPositionOfShowFilename;
}

void ShowFilename::applyToActors(const BumpEnvironment& env, VisualPhysicsActorList actors, int subcommand) {
  if (canBeAppliedToActors(env, actors)) {
    for_each(VisualPhysicsActor* actor, actors) {
      actor->set_name_hidden(false);
    }
  }
}

bool ShowFilename::canBeAppliedToActors(const BumpEnvironment& env, VisualPhysicsActorList list_of_actors) {
  for_each(VisualPhysicsActor* actor, list_of_actors) {
    if (actor->children().count() > 0) {
      return false;
    }
    if (AppSettings::singleton()->image_name_on_walls_hidden() && actor->is_an_image_on_wall()) {
      return false;
    }
  }
  name_ = "Show Filename";
  if (list_of_actors.size() > 1) {
    name_ += "s";
  }
  for_each(VisualPhysicsActor* actor, list_of_actors) {
    if (!(actor->label_visible())) {
      return true;
    }
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SINGLETON_IMPLEMENTATION(Duplicate)

QString Duplicate::name() {
  return "Duplicate";
}

int Duplicate::number_of_separators_above_me() {
  return 2;
}

int Duplicate::position_within_my_category() {
  return kPositionOfDuplicate;
}

void Duplicate::applyToActors(const BumpEnvironment& env, VisualPhysicsActorList actors, int subcommand) {
  QStringList paths_of_files_to_duplicate;
  for_each(VisualPhysicsActor* actor, actors) {
    // We are only duplicating items that are not startup disk; this is different from what apple does.
    // For apple, it's going to pop up a dialog box and stop the operation if we attempt to duplicate a bunch
    // of items containing the startup disk.
    if (!FileManager::isStartupDrive(actor->path())) {
      paths_of_files_to_duplicate.append(actor->path());
    }
  }
  FileManager::duplicateFile(paths_of_files_to_duplicate);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SINGLETON_IMPLEMENTATION(MakeAlias)

QString MakeAlias::name() {
  return "Make Alias";
}

int MakeAlias::number_of_separators_above_me() {
  return 2;
}

int MakeAlias::position_within_my_category() {
  return kPositionOfMakeAlias;
}

void MakeAlias::applyToActors(const BumpEnvironment& env, VisualPhysicsActorList actors, int subcommand) {
  for_each(VisualPhysicsActor* actor, actors) {
    FileManager::makeAliasOfFile(actor->path());
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SINGLETON_IMPLEMENTATION(NewFolder)

QString NewFolder::name() {
  return "New Folder";
}

int NewFolder::number_of_separators_above_me() {
  return 0;
}

int NewFolder::position_within_my_category() {
  return kPositionOfNewFolder;
}

void NewFolder::applyToActors(const BumpEnvironment& env, VisualPhysicsActorList actors, int subcommand) {
  assert(actors.size() == 1);
  QDir dir(actors.value(0)->path());
  int number_to_append = 2;
  QString new_folder_name = "untitled folder";

  while (QFileInfo(dir.filePath(new_folder_name)).exists()) {
    new_folder_name = QString("untitled folder %1").arg(number_to_append);
    number_to_append++;
  }
  dir.mkdir(new_folder_name);
}

bool NewFolder::canBeAppliedToActors(const BumpEnvironment& env, VisualPhysicsActorList actors) {
  return actors.size() == 1 && QFileInfo(actors.value(0)->path()).isDir();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SINGLETON_IMPLEMENTATION(ChangeBackground)

QString ChangeBackground::name() {
  return "Change Background"+QString(133);
}

int ChangeBackground::number_of_separators_above_me() {
  return 4;
}

int ChangeBackground::position_within_my_category() {
  return kPositionOfChangeBackground;
}

#import "BumpTop/OSX/PreferencesController.h"
void ChangeBackground::applyToActors(const BumpEnvironment& env, VisualPhysicsActorList actors, int subcommand) {
  execute(env);
}

void ChangeBackground::execute(const BumpEnvironment& env) {
#ifndef BUMPTOP_TEST
  [[PreferencesController singleton] showWindowWithView:@"Backgrounds"];
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SINGLETON_IMPLEMENTATION(Grow)

QString Grow::name() {
  return "Grow";
}

int Grow::number_of_separators_above_me() {
  return 6;
}

int Grow::position_within_my_category() {
  return kPositionOfGrow;
}

void Grow::applyToActors(const BumpEnvironment& env, VisualPhysicsActorList actors, int subcommand) {
  GrowUndoCommand* grow_undo_command = new GrowUndoCommand(actors, env.room);
  env.room->updateCurrentState();
  env.room->undo_redo_stack()->push(grow_undo_command, env.room->current_state());
}

bool Grow::canBeAppliedToActors(const BumpEnvironment& env, VisualPhysicsActorList actors) {
  for_each(VisualPhysicsActor* actor, actors) {
    if (actor->parent() != NULL)
      return false;
  }
  return true;
}

QString Grow::toolbar_button_path_inactive() {
  return FileManager::getResourcePath() + "/grow.png";
}

QString Grow::toolbar_button_path_active() {
  return FileManager::getResourcePath() + "/grow_active.png";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SINGLETON_IMPLEMENTATION(Shrink)

QString Shrink::name() {
  return "Shrink";
}

int Shrink::number_of_separators_above_me() {
  return 6;
}

int Shrink::position_within_my_category() {
  return kPositionOfShrink;
}

void Shrink::applyToActors(const BumpEnvironment& env, VisualPhysicsActorList actors, int subcommand) {
  ShrinkUndoCommand* shrink_undo_command = new ShrinkUndoCommand(actors, env.room);
  env.room->updateCurrentState();
  env.room->undo_redo_stack()->push(shrink_undo_command, env.room->current_state());
}

bool Shrink::canBeAppliedToActors(const BumpEnvironment& env, VisualPhysicsActorList actors) {
  for_each(VisualPhysicsActor* actor, actors) {
    if (actor->parent() != NULL)
      return false;
  }
  return true;
}

QString Shrink::toolbar_button_path_inactive() {
  return FileManager::getResourcePath() + "/shrink.png";
}

QString Shrink::toolbar_button_path_active() {
  return FileManager::getResourcePath() + "/shrink_active.png";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SINGLETON_IMPLEMENTATION(Compress)

QString Compress::name() {
  return name_;
}

int Compress::number_of_separators_above_me() {
  return 2;
}

int Compress::position_within_my_category() {
  return kPositionOfCompress;
}

bool collidesWithPendingArchive(QString file_path) {
  bool does_same_path_exist = false;
  for_each(QString path, BumpTopCommand::pending_archive_zip_paths) {
    if (file_path == path)
      does_same_path_exist = true;
  }
  return does_same_path_exist;
}

QString createFileName(QDir dir, QString file_path) {
  int number_to_append_to_file_name = 2;
  QString string_to_append_to_file_name;
  string_to_append_to_file_name.setNum(number_to_append_to_file_name);
  QString file_path_temp = file_path;
  QString file_name_temp = QFileInfo(file_path_temp).fileName();

  while (QFileInfo(dir.filePath(file_name_temp)).exists() || collidesWithPendingArchive(file_path_temp)) {
    file_path_temp = file_path;
    file_path_temp.insert(file_path_temp.lastIndexOf("."), QString(" " + string_to_append_to_file_name));
    number_to_append_to_file_name++;
    string_to_append_to_file_name.setNum(number_to_append_to_file_name);
    file_name_temp = QFileInfo(file_path_temp).fileName();
  }
  BumpTopCommand::pending_archive_zip_paths.append(file_path_temp);
  return file_path_temp;
}

#import "BumpTop/OSX/CompressProgressDialogController.h"
void Compress::applyToActors(const BumpEnvironment& env, VisualPhysicsActorList actors, int subcommand) {
  if (actors.size() > 1 || !actors[0]->pathsOfDescendants().isEmpty()) {
    QDir dir(FileManager::getApplicationDataPath());
    int number_to_append = 2;
    QString archive_path = "Archive";

    while (QFileInfo(dir.filePath(archive_path)).exists()) {
      archive_path = QString("Archive %1").arg(number_to_append);
      number_to_append++;
    }
    archive_path = FileManager::getApplicationDataPath() + archive_path;

    Room* room = actors[0]->room_surface()->room();
    QString target_path = createFileName(QDir(room->path()), room->path() + "/Archive.zip");

    NSMutableArray *file_paths = [[NSMutableArray alloc] initWithObjects:nil];
    for_each(VisualPhysicsActor* actor, actors) {
      if (actor->path() != "")
        [file_paths addObject:NSStringFromQString(actor->path())];
      for_each(QString path_of_child, actor->pathsOfDescendants())
        [file_paths addObject:NSStringFromQString(path_of_child)];
    }
    CompressProgressDialogController* compress_window = [[CompressProgressDialogController alloc] init];
    [compress_window showCompressProgressDialogForFiles:file_paths
                                         withArchivePath:NSStringFromQString(archive_path)
                                          andTargetPath:NSStringFromQString(target_path)];
  } else {
    Room* room = actors[0]->room_surface()->room();

    QDir dir(room->path());
    QString source_path = actors[0]->path();
    QString target_path = source_path;
    if (target_path.lastIndexOf(".") != -1)
      target_path.remove(target_path.lastIndexOf("."), target_path.length() + 1);
    target_path.append(".zip");
    target_path = createFileName(dir, target_path);

    CompressProgressDialogController* compress_window = [[CompressProgressDialogController alloc] init];
    [compress_window showCompressProgressDialogForFile:NSStringFromQString(source_path)
                                        withTargetPath:NSStringFromQString(target_path)];
  }
}

bool Compress::canBeAppliedToActors(const BumpEnvironment& env, VisualPhysicsActorList list_of_actors) {
  if (list_of_actors.size() == 1 && list_of_actors.value(0)->path().endsWith(".zip"))
    return false;
  int num_actors = 0;
  for_each(VisualPhysicsActor* actor, list_of_actors) {
    if (FileManager::getFileKind(actor->path()) == VOLUME) {
      return false;
    }
    if (!actor->path().isEmpty()) {
      num_actors++;
    }
    // check if there is any drives in a pile and return false if there is any
    for_each(VisualPhysicsActor* child, actor->children()) {
      if (FileManager::getFileKind(child->path()) == VOLUME) {
        return false;
      }
    }
    num_actors += actor->pathsOfDescendants().size();
  }
  if (num_actors > 1)
    name_ = QString("Compress %1 Items").arg(num_actors);
  else
    name_ = "Compress";

  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SINGLETON_IMPLEMENTATION(SelectAllActors)

QString SelectAllActors::name() {
  return "Select All";
}

int SelectAllActors::number_of_separators_above_me() {
  return 4;
}

int SelectAllActors::position_within_my_category() {
  return 0;  // does not appear on context menu
}

void SelectAllActors::applyToActors(const BumpEnvironment& env, VisualPhysicsActorList actors, int subcommand) {
  execute(env);
}

void SelectAllActors::execute(const BumpEnvironment& env) {
  AnimationManager::singleton()->endAnimationsForMaterial(AppSettings::singleton()->global_material_name(HIGHLIGHT));  // NOLINT
  VisualPhysicsActorList selected_actors = env.room->room_actor_list();
  for_each(VisualPhysicsActor* actor, selected_actors)
    actor->set_selected(true);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SINGLETON_IMPLEMENTATION(PileByTypeForAllActors)

QString PileByTypeForAllActors::name() {
  return "Pile by Type";
}

int PileByTypeForAllActors::number_of_separators_above_me() {
  return 4;
}

int PileByTypeForAllActors::position_within_my_category() {
  return kPositionOfPileByTypeForAllActors;
}

void PileByTypeForAllActors::applyToActors(const BumpEnvironment& env, VisualPhysicsActorList actors, int subcommand) {
  PileByTypeUndoCommand* pile_by_type_command = new PileByTypeUndoCommand(env.room->room_actor_list(), env.room,
                                                                          env.ogre_scene_manager, env.physics);
  env.room->updateCurrentState();
  env.room->undo_redo_stack()->push(pile_by_type_command, env.room->current_state());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SINGLETON_IMPLEMENTATION(PileByTypeForSelectedActors)

QString PileByTypeForSelectedActors::name() {
  return "Pile by Type";
}

int PileByTypeForSelectedActors::number_of_separators_above_me() {
  return 3;
}

int PileByTypeForSelectedActors::position_within_my_category() {
  return kPositionOfPileByTypeForSelectedActors;
}

void PileByTypeForSelectedActors::applyToActors(const BumpEnvironment& env, VisualPhysicsActorList actors, int subcommand) {  // NOLINT
  PileByTypeUndoCommand* pile_by_type_command = new PileByTypeUndoCommand(actors, env.room,
                                                                          env.ogre_scene_manager, env.physics);
  env.room->updateCurrentState();
  env.room->undo_redo_stack()->push(pile_by_type_command, env.room->current_state());
}

bool PileByTypeForSelectedActors::canBeAppliedToActors(const BumpEnvironment& env, VisualPhysicsActorList list_of_actors) {  // NOLINT
  // this functionality needs more work, right now piles created for pile by type on selected actors are very
  // disorganized so it does more hurt than good

  if (list_of_actors.size() == 1)
    return false;
  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SINGLETON_IMPLEMENTATION(ShowOriginal)

QString ShowOriginal::name() {
  return "Show Original";
}

int ShowOriginal::number_of_separators_above_me() {
  return 1;
}

int ShowOriginal::position_within_my_category() {
  return kPositionOfShowOriginal;
}

void ShowOriginal::applyToActors(const BumpEnvironment& env, VisualPhysicsActorList actors, int subcommand) {
  QStringList paths_of_files_to_show_original;
  for_each(VisualPhysicsActor* actor, actors) {
    paths_of_files_to_show_original.append(QFileInfo(actor->path()).readLink());
  }
  QString paths = FileManager::getPathsInAppleScriptFormat(paths_of_files_to_show_original);

  NSString* script_format = @"tell application \"Finder\"\n"
                                @"reveal %@\n"
                                @"set frontmost of process \"Finder\" to true\n"
                            @"end tell";
  NSString* script = [NSString stringWithFormat:script_format, NSStringFromQString(paths)];

  FileManager::runAppleScriptThroughNSTask(script);
}

bool ShowOriginal::canBeAppliedToActors(const BumpEnvironment& env, VisualPhysicsActorList list_of_actors) {
  for_each(VisualPhysicsActor* actor, list_of_actors) {
    if (FileManager::getFileKind(actor->path()) != ALIAS) {
      return false;
    }
  }
  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SINGLETON_IMPLEMENTATION(Copy)

QString Copy::name() {
  return name_;
}

int Copy::number_of_separators_above_me() {
  return 3;
}

int Copy::position_within_my_category() {
  return kPositionOfCopy;
}

void Copy::applyToActors(const BumpEnvironment& env, VisualPhysicsActorList actors, int subcommand) {
  NSPasteboard *pasteboard = [NSPasteboard pasteboardWithName: NSGeneralPboard];
  [pasteboard declareTypes:[NSArray arrayWithObjects:NSFilenamesPboardType, nil] owner:nil];

  NSMutableArray* file_paths = [NSMutableArray arrayWithCapacity:actors.count()];
  for_each(VisualPhysicsActor* actor, actors) {
    if (actor->children().empty()) {
      [file_paths addObject:NSStringFromQString(actor->path())];
    } else {
      for_each(VisualPhysicsActor* child, actor->children()) {
        if (child->children().empty()) {
          [file_paths addObject:NSStringFromQString(child->path())];
        }
      }
    }
  }

  [pasteboard setPropertyList:file_paths forType:NSFilenamesPboardType];
}

bool Copy::canBeAppliedToActors(const BumpEnvironment& env, VisualPhysicsActorList list_of_actors) {
  bool all_paths_are_valid = true;
  int number_of_items_to_copy = 0;
  for_each(VisualPhysicsActor* actor, list_of_actors) {
    if (FileManager::isVolume(actor->path())) {
      return false;
    } else if (actor->children().empty()) {
      if (actor->path() == "") {
        all_paths_are_valid = false;
      }
      number_of_items_to_copy++;
    } else {
      for_each(VisualPhysicsActor* child, actor->children()) {
        if (child->path() == "") {
          all_paths_are_valid = false;
        }
        number_of_items_to_copy++;
      }
    }
  }
  if (number_of_items_to_copy > 1) {
    name_ = QString("Copy %1 Items").arg(number_of_items_to_copy);
  } else if (number_of_items_to_copy == 1) {
    name_ = QFileInfo(list_of_actors[0]->path()).fileName();
    if (name_.size() > 30)
      name_ = name_.left(15) + "..." + name_.right(15);
    name_ = "Copy " + name_;
  }

  return (all_paths_are_valid && number_of_items_to_copy > 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SINGLETON_IMPLEMENTATION(Paste)

QString Paste::name() {
  return name_;
}

int Paste::number_of_separators_above_me() {
  return 3;
}

int Paste::position_within_my_category() {
  return kPositionOfPaste;
}

void Paste::applyToActors(const BumpEnvironment& env, VisualPhysicsActorList actors, int subcommand) {
  execute(env);
}

void Paste::execute(const BumpEnvironment& env) {
  NSPasteboard *pasteboard =[NSPasteboard pasteboardWithName: NSGeneralPboard];
  NSMutableArray *file_paths = [pasteboard propertyListForType:NSFilenamesPboardType];
  QStringList files_to_paste;
  for (int i = 0; i < [file_paths count]; i++) {
    QString path = QStringFromNSString([file_paths objectAtIndex:i]);
    FileKind file_kind = FileManager::getFileKind(path);
    if (file_kind != VOLUME)
      files_to_paste.push_back(QStringFromNSString([file_paths objectAtIndex:i]));
  }

  if (files_to_paste.count() > 0)
    env.room->room_drop_receiver()->copyFilesToRoom(files_to_paste, BumpTopApp::singleton()->mouse_location());
}

bool Paste::canBeAppliedToActors(const BumpEnvironment& env, VisualPhysicsActorList list_of_actors) {
  NSPasteboard *pasteboard =[NSPasteboard pasteboardWithName: NSGeneralPboard];
  NSMutableArray* file_paths = [pasteboard propertyListForType:NSFilenamesPboardType];

  QStringList pastable_files;
  for (int i = 0; i < [file_paths count]; i++) {
    QString path = QStringFromNSString([file_paths objectAtIndex:i]);
    FileKind file_kind = FileManager::getFileKind(path);
    // For now we're not letting them paste in volumes because I'm not sure how FileManager deals with them
    if (file_kind != VOLUME)
      pastable_files.push_back(QStringFromNSString([file_paths objectAtIndex:i]));
  }

  name_ = QString("Paste %1 items").arg(pastable_files.count());

  return (pastable_files.count() > 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SINGLETON_IMPLEMENTATION(Rename)

QString Rename::name() {
  return "Rename";
}

int Rename::number_of_separators_above_me() {
  return 3;
}

int Rename::position_within_my_category() {
  return kPositionOfRename;
}

#include "BumpTop/OSX/OgreController.h"
#include "BumpTop/ToolTipManager.h"
void Rename::applyToActors(const BumpEnvironment& env, VisualPhysicsActorList actors, int subcommand) {
  actor_ = actors[0];
  label_pos_ = actor_->labelPositionForCurrentPosition() + Ogre::Vector2(0, MENU_BAR_HEIGHT);

  line_edit_ = new CustomQLineEdit();
  line_edit_->setWindowFlags(Qt::FramelessWindowHint);
  line_edit_->setContextMenuPolicy(Qt::PreventContextMenu);

  if (actor_->actor_type() == BUMP_PILE) {
    ToolTipManager::singleton()->hideNamePileTooltip();
  }

  QFileInfo info = QFileInfo(actor_->path());
  QString display_name;
  if (actor_->path() == "") {
    display_name = actor_->display_name();
  } else {
    display_name = info.fileName();
  }

  line_edit_->setText(display_name);
  textBoxChanged(display_name);
  line_edit_->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
  if (info.completeBaseName() != "" && !info.isDir())
    line_edit_->setSelection(0, info.completeBaseName().length());
  else
    line_edit_->setSelection(0, display_name.length());

  line_edit_->show();
  line_edit_->grabKeyboard();
  actor_->set_label_visible(false);

  assert(QObject::connect(line_edit_, SIGNAL(textChanged(const QString&)),
                          this, SLOT(textBoxChanged(const QString&))));

  assert(QObject::connect(line_edit_, SIGNAL(textEdited(const QString&)),
                          this, SLOT(textBoxChanged(const QString&))));

  assert(QObject::connect(line_edit_, SIGNAL(editingFinished()),
                          this, SLOT(editingComplete())));

  assert(QObject::connect(actor_, SIGNAL(onPoseChanged(VisualPhysicsActorId)),
                          this, SLOT(poseChanged(VisualPhysicsActorId))));

  assert(QObject::connect(actor_, SIGNAL(onRemoved(VisualPhysicsActorId)),
                          this, SLOT(actorRemoved(VisualPhysicsActorId))));

  NSView* line_edit_view = reinterpret_cast<NSView *>(line_edit_->winId());
  NSWindow* line_edit_window = [line_edit_view window];
  BumpTopApp::singleton()->set_should_force_bumptop_window_to_front(false);
  [line_edit_window setLevel:kCGDesktopIconWindowLevel];
  [line_edit_window orderFrontRegardless];
}

// this function is to deal with the weird character produced by backspacing after erasing a composing international
// character
bool does_string_have_display_size(QString str) {
  QPainter temp_painter;

  QFont label_font = QFont("Lucida Grande");
  label_font.setBold(true);
  label_font.setPointSize(13);

  QImage dummy_image  = QImage(1, 1, QImage::Format_ARGB32);
  temp_painter.begin(&dummy_image);
  temp_painter.setFont(label_font);

  QRect bounding_rect_of_space = temp_painter.boundingRect(0, 0, 0, 0, Qt::AlignLeft | Qt::TextSingleLine, " ");
  QRect bounding_rect_of_line_edit_and_space = temp_painter.boundingRect(0, 0, 0, 0, Qt::AlignLeft | Qt::TextSingleLine, " " + str);  // NOLINT

  temp_painter.end();
  return (bounding_rect_of_space.width() != bounding_rect_of_line_edit_and_space.width());
}



void Rename::textBoxChanged(const QString& str) {
  QPainter temp_painter;

  QFont label_font = QFont("Lucida Grande");
  label_font.setBold(true);
  label_font.setPointSize(13);

  QImage dummy_image  = QImage(1, 1, QImage::Format_ARGB32);
  temp_painter.begin(&dummy_image);
  temp_painter.setFont(label_font);

  QString sizing_string;
  sizing_string = str;

  bool is_sizing_string_length_zero = (sizing_string.length() == 0);
  if (is_sizing_string_length_zero) {
    sizing_string = " ";
  } else if (sizing_string.length() == 1) {
    // when string size is 1 we want to avoid getting character that has no display width by checking its bounding boxes
    if (!does_string_have_display_size(sizing_string)) {
      sizing_string = " ";
      is_sizing_string_length_zero = true;
      // remove unwanted character from line_edit_
      line_edit_->setText("");
    }
  }

  QRect bounding_rect = temp_painter.boundingRect(0, 0, 0, 0, Qt::AlignLeft | Qt::TextSingleLine, sizing_string);

  if (is_sizing_string_length_zero) {
    bounding_rect.setWidth(1);
  }

  line_edit_size_ = bounding_rect;
  temp_painter.end();

  line_edit_->setFixedSize(line_edit_size_.width() + 8, line_edit_size_.height() + 5);
  line_edit_->move(label_pos_.x - line_edit_size_.width()/2.0 - 4, label_pos_.y);
}

void Rename::poseChanged(VisualPhysicsActorId actor_id) {
  Ogre::Vector2 new_pos;
  if (actor_ != NULL)
    new_pos = actor_->labelPositionForCurrentPosition() + Ogre::Vector2(0, MENU_BAR_HEIGHT);
  if (line_edit_ != NULL && (label_pos_ - new_pos).length() > 5) {
    label_pos_ = new_pos;
    line_edit_->move(label_pos_.x - line_edit_size_.width()/2.0 - 4, label_pos_.y);
    line_edit_->setFocus();
  }
}

void Rename::editingComplete() {
  BumpTopApp::singleton()->set_should_force_bumptop_window_to_front(true);
  if (actor_ != NULL) {
    actor_->set_label_visible(true);

    // when string size is 1 we want to avoid getting character that has no display width by checking its bounding boxes
    if (line_edit_->text().length() == 1 && does_string_have_display_size(line_edit_->text())) {
      actor_->rename(line_edit_->text());
    } else {
      actor_->rename(line_edit_->text());
    }
  }

  assert(QObject::connect(BumpTopApp::singleton(), SIGNAL(onRender()),
                          this, SLOT(renderTick())));
}

void Rename::actorRemoved(VisualPhysicsActorId actor_id) {
  actor_ = NULL;
  if (line_edit_ != NULL) {
    delete line_edit_;
    line_edit_ = NULL;
  }
}

void Rename::renderTick() {
  if (actor_ != NULL) {
    assert(QObject::disconnect(actor_, SIGNAL(onPoseChanged(VisualPhysicsActorId)),  // NOLINT
                               this, SLOT(poseChanged(VisualPhysicsActorId))));  // NOLINT
    assert(QObject::disconnect(actor_, SIGNAL(onRemoved(VisualPhysicsActorId)),  // NOLINT
                               this, SLOT(actorRemoved(VisualPhysicsActorId))));  // NOLINT
  }
  assert(QObject::disconnect(BumpTopApp::singleton(), SIGNAL(onRender()),  // NOLINT
                            this, SLOT(renderTick())));  // NOLINT
  if (line_edit_ != NULL) {
    delete line_edit_;
    line_edit_ = NULL;
  }
  actor_ = NULL;
}

bool Rename::canBeAppliedToActors(const BumpEnvironment& env, VisualPhysicsActorList list_of_actors) {
  if (list_of_actors.size() == 1)
    if (list_of_actors[0]->nameable())
      return true;
  return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SINGLETON_IMPLEMENTATION(GridView)

QString GridView::name() {
  return "Grid View";
}

int GridView::number_of_separators_above_me() {
  return 3;
}

int GridView::position_within_my_category() {
  return kPositionOfGridView;
}

void GridView::applyToActors(const BumpEnvironment& env, VisualPhysicsActorList actors, int subcommand) {
  if (UndoRedoStack::is_last_command_grid_view == true) {
    bool are_actors_already_gridded = true;
    if (last_gridded_actors_.count() == actors.count()) {
      for_each(VisualPhysicsActor* actor, actors) {
        if (!last_gridded_actors_.contains(actor)) {
          are_actors_already_gridded = false;
        }
      }
      if (are_actors_already_gridded) {
        return;
      }
    }
  }

  last_gridded_actors_ = actors;
  GridViewUndoCommand* grid_view_command = new GridViewUndoCommand(actors, env.room,
                                                                   env.ogre_scene_manager, env.physics);
  env.room->updateCurrentState();
  env.room->undo_redo_stack()->push(grid_view_command, env.room->current_state());
  UndoRedoStack::is_last_command_grid_view = true;
}

bool GridView::canBeAppliedToActors(const BumpEnvironment& env, VisualPhysicsActorList list_of_actors) {
  int valid_actor_count = 0;
  for_each(VisualPhysicsActor* actor, list_of_actors) {
    if (!actor->room_surface()->is_pinnable_receiver()) {
      valid_actor_count++;
    }
  }
  return valid_actor_count >= 2;
}

QString GridView::toolbar_button_path_inactive() {
  return FileManager::getResourcePath() + "/clean_up.png";
}

QString GridView::toolbar_button_path_active() {
  return FileManager::getResourcePath() + "/clean_up_active.png";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SINGLETON_IMPLEMENTATION(ChangeLabelColour)

QString ChangeLabelColour::name() {
  return "Label Colour";
}

int ChangeLabelColour::number_of_separators_above_me() {
  return 4;
}

int ChangeLabelColour::position_within_my_category() {
  return kPositionOfChangeLabelColour;
}

void ChangeLabelColour::applyToActors(const BumpEnvironment& env, VisualPhysicsActorList actors, int subcommand) {
  for_each(VisualPhysicsActor* actor, actors) {
    actor->set_label_colour((BumpBoxLabelColour)subcommand);
    actor->updateLabel(1);

    FileManager::setFinderLabelColourThroughNSTask(actor->display_name(),(BumpBoxLabelColour)subcommand);
  }
}

bool ChangeLabelColour::has_subcommands() {
  return true;
}

CGImageRef ChangeLabelColour::iconForSubcommand(VisualPhysicsActorList actors, int subcommand) {
  QStringList colours = subcommand_names(actors);
  return NULL;
}

QStringList ChangeLabelColour::subcommand_names(VisualPhysicsActorList actors) {
  QStringList colours;
  colours << "Colourless"
          << "Red"
          << "Orange"
          << "Yellow"
          << "Green"
          << "Blue"
          << "Purple"
          << "Grey";
  return colours;
}

bool ChangeLabelColour::canBeAppliedToActors(const BumpEnvironment& env, VisualPhysicsActorList list_of_actors) {
  for_each(VisualPhysicsActor* actor, list_of_actors) {
    if (actor->label() == NULL) {
      return false;
    }
    if (actor->label()->visible() == false) {
      return false;
    }
  }
  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SeparatorBumpTopCommand::SeparatorBumpTopCommand(int menu_index)
: menu_index_(menu_index) {
}

QString SeparatorBumpTopCommand::name() {
  return QString();
}

void SeparatorBumpTopCommand::applyToActors(const BumpEnvironment& env,
                                            VisualPhysicsActorList list_of_actors, int subcommand) {
}

int SeparatorBumpTopCommand::number_of_separators_above_me() {
  return 0;
}

int SeparatorBumpTopCommand::position_within_my_category() {
  return 0;  // does not appear in context menu
}

int SeparatorBumpTopCommand::menu_index() {
  return menu_index_;
}

bool SeparatorBumpTopCommand::isSeparator() {
  return true;
};

#include "moc/moc_BumpTopCommands.cpp"
