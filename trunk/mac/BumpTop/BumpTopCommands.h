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

#ifndef BUMPTOP_BUMPTOPCOMMANDS_H_
#define BUMPTOP_BUMPTOPCOMMANDS_H_

class Physics;
class Room;
class VisualPhysicsActor;

#include <cstdarg>

#include "BumpTop/Singleton.h"
#include "BumpTop/VisualPhysicsActorId.h"
#include "BumpTop/VisualPhysicsActorList.h"

class BumpTopCommand;
class CommandButton;

typedef QSet<BumpTopCommand*> BumpTopCommandSet;

BumpTopCommandSet* MakeQSet(int count, ...);

struct BumpEnvironment {
  BumpEnvironment();
  BumpEnvironment(Physics* physics, Room* room, Ogre::SceneManager* ogre_scene_manager);
  Physics* physics;
  Room* room;
  Ogre::SceneManager* ogre_scene_manager;
};

class BumpTopCommand : public QObject {
 public:
  virtual ~BumpTopCommand();
  virtual QString name() = 0;
  virtual void applyToActors(const BumpEnvironment& env,
                             VisualPhysicsActorList list_of_actors, int subcommand = 0) = 0;
  virtual int number_of_separators_above_me() = 0;
  virtual int position_within_my_category() = 0;
  virtual bool canBeAppliedToActors(const BumpEnvironment& env, VisualPhysicsActorList list_of_actors);
  virtual bool isSeparator();
  virtual bool has_subcommands();
  virtual bool is_toolbar_command();
  virtual CommandButton* command_button();
  virtual CGImageRef iconForSubcommand(VisualPhysicsActorList actors, int subcommand);
  virtual QStringList subcommand_names(VisualPhysicsActorList list_of_actors);

  static QStringList pending_archive_zip_paths;
};

class BumpTopToolbarCommand : public BumpTopCommand {
 public:
  BumpTopToolbarCommand();
  virtual ~BumpTopToolbarCommand();

  virtual bool is_toolbar_command();
  virtual bool should_dismiss_toolbar();
  virtual CommandButton* command_button();
  virtual QString toolbar_button_material_inactive();
  virtual QString toolbar_button_material_active();
 protected:
  virtual QString toolbar_button_path_inactive() = 0;
  virtual QString toolbar_button_path_active() = 0;
  QString toolbar_button_material_inactive_;
  QString toolbar_button_material_active_;
  CommandButton* command_button_;
};

class SelectAllActors : public BumpTopCommand {
  SINGLETON_HEADER(SelectAllActors)
 public:
  virtual QString name();
  virtual void applyToActors(const BumpEnvironment& env, VisualPhysicsActorList list_of_actors, int subcommand = 0);
  void execute(const BumpEnvironment& env);
  virtual int number_of_separators_above_me();
  virtual int position_within_my_category();
};

class Undo : public BumpTopCommand {
  SINGLETON_HEADER(Undo)
 public:
  virtual QString name();
  virtual void applyToActors(const BumpEnvironment& env, VisualPhysicsActorList list_of_actors, int subcommand = 0);
  void execute(const BumpEnvironment& env);
  virtual int number_of_separators_above_me();
  virtual int position_within_my_category();
};

class Redo : public BumpTopCommand {
  SINGLETON_HEADER(Redo)
 public:
  virtual QString name();
  virtual void applyToActors(const BumpEnvironment& env, VisualPhysicsActorList list_of_actors, int subcommand = 0);
  void execute(const BumpEnvironment& env);
  virtual int number_of_separators_above_me();
  virtual int position_within_my_category();
};

class CreatePile : public BumpTopToolbarCommand {
  SINGLETON_HEADER(CreatePile)
 public:
  virtual QString name();
  virtual void applyToActors(const BumpEnvironment& env, VisualPhysicsActorList list_of_actors, int subcommand = 0);
  virtual int number_of_separators_above_me();
  virtual int position_within_my_category();
  virtual bool canBeAppliedToActors(const BumpEnvironment& env, VisualPhysicsActorList list_of_actors);

 protected:
  virtual QString toolbar_button_path_inactive();
  virtual QString toolbar_button_path_active();
};

class BreakPile : public BumpTopToolbarCommand {
  SINGLETON_HEADER(BreakPile)
 public:
  virtual QString name();
  virtual void applyToActors(const BumpEnvironment& env, VisualPhysicsActorList list_of_actors, int subcommand = 0);
  virtual int number_of_separators_above_me();
  virtual int position_within_my_category();
  virtual bool canBeAppliedToActors(const BumpEnvironment& env, VisualPhysicsActorList actors);

 protected:
  virtual QString toolbar_button_path_inactive();
  virtual QString toolbar_button_path_active();
};

class Open : public BumpTopCommand {
  SINGLETON_HEADER(Open)
 public:
  virtual QString name();
  virtual void applyToActors(const BumpEnvironment& env, VisualPhysicsActorList list_of_actors, int subcommand = 0);
  virtual int number_of_separators_above_me();
  virtual int position_within_my_category();
};

class OpenWith : public BumpTopCommand {
  SINGLETON_HEADER(OpenWith)
 public:
  virtual QString name();
  virtual void applyToActors(const BumpEnvironment& env, VisualPhysicsActorList list_of_actors, int subcommand = 0);
  virtual int number_of_separators_above_me();
  virtual int position_within_my_category();
  virtual bool has_subcommands();
  virtual QStringList subcommand_names(VisualPhysicsActorList list_of_actors);
  virtual bool canBeAppliedToActors(const BumpEnvironment& env, VisualPhysicsActorList actors);
  CGImageRef iconForSubcommand(VisualPhysicsActorList actors, int subcommand);
  QString shortenedName(QString app_path);
 protected:
  QStringList CFStringArrayTOQStringList(CFArrayRef array);
  virtual QStringList validApplicationList(VisualPhysicsActorList actors);
  virtual QStringList removeDuplicateApplicationInstances(QStringList app_list);
};

class MoveToTrash : public BumpTopCommand {
  SINGLETON_HEADER(MoveToTrash)
 public:
  virtual QString name();
  virtual void applyToActors(const BumpEnvironment& env, VisualPhysicsActorList list_of_actors, int subcommand = 0);
  virtual int number_of_separators_above_me();
  virtual int position_within_my_category();
  virtual bool canBeAppliedToActors(const BumpEnvironment& env, VisualPhysicsActorList actors);
};

class Eject : public BumpTopCommand {
  SINGLETON_HEADER(Eject)
 public:
  virtual QString name();
  virtual void applyToActors(const BumpEnvironment& env, VisualPhysicsActorList list_of_actors, int subcommand = 0);
  virtual int number_of_separators_above_me();
  virtual int position_within_my_category();
  virtual bool canBeAppliedToActors(const BumpEnvironment& env, VisualPhysicsActorList actors);
 protected:
  QString name_;
};

class SortAlphabetically : public BumpTopCommand {
  SINGLETON_HEADER(SortAlphabetically)
 public:
  virtual QString name();
  virtual void applyToActors(const BumpEnvironment& env, VisualPhysicsActorList list_of_actors, int subcommand = 0);
  virtual int number_of_separators_above_me();
  virtual int position_within_my_category();
  virtual bool canBeAppliedToActors(const BumpEnvironment& env, VisualPhysicsActorList list_of_actors);
 protected:
  QString name_;
};

class GetInfo : public BumpTopCommand {
  SINGLETON_HEADER(GetInfo)
 public:
  virtual QString name();
  virtual void applyToActors(const BumpEnvironment& env, VisualPhysicsActorList list_of_actors, int subcommand = 0);
  virtual int number_of_separators_above_me();
  virtual int position_within_my_category();
};

class HideFilename : public BumpTopCommand {
  SINGLETON_HEADER(HideFilename)
 public:
  virtual QString name();
  virtual void applyToActors(const BumpEnvironment& env, VisualPhysicsActorList list_of_actors, int subcommand = 0);
  virtual int number_of_separators_above_me();
  virtual int position_within_my_category();
  virtual bool canBeAppliedToActors(const BumpEnvironment& env, VisualPhysicsActorList list_of_actors);
 protected:
  QString name_;
};

class ShowFilename : public BumpTopCommand {
  SINGLETON_HEADER(ShowFilename)
 public:
  virtual QString name();
  virtual void applyToActors(const BumpEnvironment& env, VisualPhysicsActorList list_of_actors, int subcommand = 0);
  virtual int number_of_separators_above_me();
  virtual int position_within_my_category();
  virtual bool canBeAppliedToActors(const BumpEnvironment& env, VisualPhysicsActorList list_of_actors);
 protected:
  QString name_;
};

class Duplicate : public BumpTopCommand {
  SINGLETON_HEADER(Duplicate)
 public:
  virtual QString name();
  virtual void applyToActors(const BumpEnvironment& env, VisualPhysicsActorList list_of_actors, int subcommand = 0);
  virtual int number_of_separators_above_me();
  virtual int position_within_my_category();
};

class MakeAlias : public BumpTopCommand {
  SINGLETON_HEADER(MakeAlias)
 public:
  virtual QString name();
  virtual void applyToActors(const BumpEnvironment& env, VisualPhysicsActorList list_of_actors, int subcommand = 0);
  virtual int number_of_separators_above_me();
  virtual int position_within_my_category();
};

class NewFolder : public BumpTopCommand {
  SINGLETON_HEADER(NewFolder)
 public:
  virtual QString name();
  virtual void applyToActors(const BumpEnvironment& env, VisualPhysicsActorList list_of_actors, int subcommand = 0);
  virtual int number_of_separators_above_me();
  virtual int position_within_my_category();
  virtual bool canBeAppliedToActors(const BumpEnvironment& env, VisualPhysicsActorList list_of_actors);
};

class ChangeBackground : public BumpTopCommand {
  SINGLETON_HEADER(ChangeBackground)
 public:
  virtual QString name();
  virtual void applyToActors(const BumpEnvironment& env, VisualPhysicsActorList list_of_actors, int subcommand = 0);
  void execute(const BumpEnvironment& env);
  virtual int number_of_separators_above_me();
  virtual int position_within_my_category();
};

class Grow : public BumpTopToolbarCommand {
  SINGLETON_HEADER(Grow)
 public:
  virtual QString name();
  virtual void applyToActors(const BumpEnvironment& env, VisualPhysicsActorList list_of_actors, int subcommand = 0);
  virtual int number_of_separators_above_me();
  virtual int position_within_my_category();
  virtual bool canBeAppliedToActors(const BumpEnvironment& env, VisualPhysicsActorList actors);

 protected:
  virtual QString toolbar_button_path_inactive();
  virtual QString toolbar_button_path_active();
};

class Shrink : public BumpTopToolbarCommand {
  SINGLETON_HEADER(Shrink)
 public:
  virtual QString name();
  virtual void applyToActors(const BumpEnvironment& env, VisualPhysicsActorList list_of_actors, int subcommand = 0);
  virtual int number_of_separators_above_me();
  virtual int position_within_my_category();
  virtual bool canBeAppliedToActors(const BumpEnvironment& env, VisualPhysicsActorList actors);

 protected:
  virtual QString toolbar_button_path_inactive();
  virtual QString toolbar_button_path_active();
};

class Compress : public BumpTopCommand {
  SINGLETON_HEADER(Compress)
 public:
  virtual QString name();
  virtual void applyToActors(const BumpEnvironment& env, VisualPhysicsActorList list_of_actors, int subcommand = 0);
  virtual int number_of_separators_above_me();
  virtual int position_within_my_category();
  virtual bool canBeAppliedToActors(const BumpEnvironment& env, VisualPhysicsActorList list_of_actors);
 protected:
  QString name_;
};

class PileByTypeForAllActors : public BumpTopCommand {
  SINGLETON_HEADER(PileByTypeForAllActors)
 public:
  virtual QString name();
  virtual void applyToActors(const BumpEnvironment& env, VisualPhysicsActorList list_of_actors, int subcommand = 0);
  virtual int number_of_separators_above_me();
  virtual int position_within_my_category();
};

class PileByTypeForSelectedActors : public BumpTopCommand {
  SINGLETON_HEADER(PileByTypeForSelectedActors)
 public:
  virtual QString name();
  virtual void applyToActors(const BumpEnvironment& env, VisualPhysicsActorList list_of_actors, int subcommand = 0);
  virtual int number_of_separators_above_me();
  virtual int position_within_my_category();
  virtual bool canBeAppliedToActors(const BumpEnvironment& env, VisualPhysicsActorList list_of_actors);
};

class ShowOriginal : public BumpTopCommand {
  SINGLETON_HEADER(ShowOriginal)
 public:
  virtual QString name();
  virtual void applyToActors(const BumpEnvironment& env, VisualPhysicsActorList list_of_actors, int subcommand = 0);
  virtual int number_of_separators_above_me();
  virtual int position_within_my_category();
  virtual bool canBeAppliedToActors(const BumpEnvironment& env, VisualPhysicsActorList list_of_actors);
};

class Copy : public BumpTopCommand {
  SINGLETON_HEADER(Copy)
 public:
  virtual QString name();
  virtual void applyToActors(const BumpEnvironment& env, VisualPhysicsActorList list_of_actors, int subcommand = 0);
  virtual int number_of_separators_above_me();
  virtual int position_within_my_category();
  virtual bool canBeAppliedToActors(const BumpEnvironment& env, VisualPhysicsActorList list_of_actors);
 protected:
  QString name_;
};

class Paste : public BumpTopCommand {
  SINGLETON_HEADER(Paste)
 public:
  virtual QString name();
  virtual void applyToActors(const BumpEnvironment& env, VisualPhysicsActorList list_of_actors, int subcommand = 0);
  void execute(const BumpEnvironment& env);
  virtual int number_of_separators_above_me();
  virtual int position_within_my_category();
  virtual bool canBeAppliedToActors(const BumpEnvironment& env, VisualPhysicsActorList list_of_actors);
 protected:
  QString name_;
};

class SeparatorBumpTopCommand : public BumpTopCommand {
 public:
  explicit SeparatorBumpTopCommand(int menu_index);
  virtual QString name();
  virtual void applyToActors(const BumpEnvironment& env, VisualPhysicsActorList list_of_actors, int subcommand = 0);
  virtual int number_of_separators_above_me();
  virtual int position_within_my_category();
  int menu_index();
  virtual bool isSeparator();
 protected:
  int menu_index_;
};

class Rename : public BumpTopCommand {
  Q_OBJECT
  SINGLETON_HEADER(Rename)
 public:
  virtual QString name();
  virtual void applyToActors(const BumpEnvironment& env, VisualPhysicsActorList list_of_actors, int subcommand = 0);
  virtual int number_of_separators_above_me();
  virtual int position_within_my_category();
  virtual bool canBeAppliedToActors(const BumpEnvironment& env, VisualPhysicsActorList list_of_actors);

 public slots:  // NOLINT
  virtual void textBoxChanged(const QString & text);
  virtual void editingComplete();
  virtual void renderTick();
  virtual void poseChanged(VisualPhysicsActorId actor_id);
  virtual void actorRemoved(VisualPhysicsActorId actor_id);

 protected:
  QLineEdit* line_edit_;
  VisualPhysicsActor* actor_;
  QRect line_edit_size_;
  Ogre::Vector2 label_pos_;
};

class GridView : public BumpTopToolbarCommand {
  SINGLETON_HEADER(GridView)
 public:
  virtual QString name();
  virtual void applyToActors(const BumpEnvironment& env, VisualPhysicsActorList list_of_actors, int subcommand = 0);
  virtual int number_of_separators_above_me();
  virtual int position_within_my_category();
  virtual bool canBeAppliedToActors(const BumpEnvironment& env, VisualPhysicsActorList list_of_actors);

 protected:
  QList<VisualPhysicsActor*> last_gridded_actors_;
  virtual QString toolbar_button_path_inactive();
  virtual QString toolbar_button_path_active();
};

class ChangeLabelColour : public BumpTopCommand {
  SINGLETON_HEADER(ChangeLabelColour)
public:
  virtual QString name();
  virtual void applyToActors(const BumpEnvironment& env, VisualPhysicsActorList list_of_actors, int subcommand = 0);
  void execute(const BumpEnvironment& env);
  virtual int number_of_separators_above_me();
  virtual int position_within_my_category();
  virtual bool has_subcommands();
  virtual QStringList subcommand_names(VisualPhysicsActorList list_of_actors);
  virtual bool canBeAppliedToActors(const BumpEnvironment& env, VisualPhysicsActorList list_of_actors);
  CGImageRef iconForSubcommand(VisualPhysicsActorList actors, int subcommand);
protected:
  QString name_;
};

#endif  // BUMPTOP_BUMPTOPCOMMANDS_H_

