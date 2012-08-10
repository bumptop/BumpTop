//
//  Copyright 2012 Google Inc. All Rights Reserved.
//  
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  
//      http://www.apache.org/licenses/LICENSE-2.0
//  
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//

#include "BumpTop/FileManager.h"

#include "BumpTop/BumpTopApp.h"
#include "BumpTop/BumpTopScene.h"
#include "BumpTop/Room.h"
#include "BumpTop/OSX/NSTaskManager.h"
#include "BumpTop/QStringHelpers.h"

enum MoveOrCopyOperation {
  MoveOperation,
  CopyOperation
};

QStringList FileManager::pending_alias_names = QStringList();

SINGLETON_IMPLEMENTATION(FileManager)

// convience function for running apple scripts through osascript
void FileManager::runAppleScriptThroughNSTask(NSString* script) {
  NSMutableArray *args = [NSMutableArray arrayWithObjects:@"-e", script, nil];
  NSTask *task = [[NSTask alloc] init];
  [task setLaunchPath:@"/usr/bin/osascript"];
  [task setArguments:args];
  [task setStandardOutput:[NSPipe pipe]];
  [[NSNotificationCenter defaultCenter] addObserver:[NSTaskManager singleton]
                                           selector:@selector(taskExited:)
                                               name:NSTaskDidTerminateNotification
                                             object:task];
  [task launch];
}

void FileManager::getAndSetLabelColourThroughNSTask(VisualPhysicsActorId actor_id) {
  // Setting the label colour of an actor to its label colour in finder
  // Using comments so NSTaskManager recognizes the script and to pass actor id
  NSString* script = [NSString stringWithFormat:@"tell application \"Finder\" to label index of item \"%@\"\
                                                --Get finder label colour and set label colour of:%@",
                                                NSStringFromQString(BumpTopApp::singleton()->scene()->room()->actor_with_unique_id(actor_id)->display_name()),
                                                [NSNumber numberWithInt:actor_id]];
  runAppleScriptThroughNSTask(script);
}

void FileManager::setFinderLabelColourThroughNSTask(QString filename, BumpBoxLabelColour label_colour) {
  // Setting the label colour of an item in finder
  NSString* script = [NSString stringWithFormat:@"tell application \"Finder\" to set label index of item \"%@\"to %@",
                      NSStringFromQString(filename),
                      [NSNumber numberWithInt:finderLabelColourFromBumpBoxLabelColour(label_colour)]];
  runAppleScriptThroughNSTask(script);
}

BumpBoxLabelColour FileManager::bumpBoxLabelColourFromFinderLabelColour(int colour) {
  // Colours in Finder are ordered differently than in BumpTop
  switch (colour) {
    case 0:
      return COLOURLESS;
    case 1:
      return ORANGE;
    case 2:
      return RED;
    case 3:
      return YELLOW;
    case 4:
      return BLUE;
    case 5:
      return PURPLE;
    case 6:
      return GREEN;
    case 7:
      return GREY;
    default:
      return COLOURLESS;
  }
}

int FileManager::finderLabelColourFromBumpBoxLabelColour(BumpBoxLabelColour colour) {
  // Colours in Finder are ordered differently than in BumpTop
  switch (colour) {
    case COLOURLESS:
      return 0;
    case RED:
      return 2;
    case ORANGE:
      return 1;
    case YELLOW:
      return 3;
    case GREEN:
      return 6;
    case BLUE:
      return 4;
    case PURPLE:
      return 5;
    case GREY:
      return 7;
    default:
      return COLOURLESS;
  }
}

QString FileManager::getPathsInAppleScriptFormat(QStringList files_paths) {
  QString paths = "{";
  for_each(QString path, files_paths) {
    paths += "POSIX file \"" + path + "\", ";
  }

  // remove the extra ", " at the end of paths
  paths.chop(2);
  paths += "}";
  return paths;
}

bool moveOrCopyFileHelper(QString file_path, QString new_file_parent_dir,
                          BehaviorOnConflict behavior_on_conflict, MoveOrCopyOperation operation) {
  QString file_name = QFileInfo(file_path).fileName();
  QString new_file_path = new_file_parent_dir + "/" + file_name;

  bool created_temp_file = false;
  QString temp_file_name;

  NSString* script_format;
  NSString* script;

  if (QFileInfo(new_file_path).exists()) {
    if (behavior_on_conflict == kPromptUserOnConflict) {
      NSAlert *alert = [[NSAlert alloc] init];
      [alert addButtonWithTitle:@"Stop"];
      [alert addButtonWithTitle:@"Replace"];
      if (operation == MoveOperation) {
        [alert setMessageText:@"Move"];
      } else if (operation == CopyOperation) {
        [alert setMessageText:@"Copy"];
      } else {
        assert(false && "value of enum of type MoveOrCopy is not either Move or Copy");
      }

      QString informative_text = QString("An item named \"%1\" already exists in this location. "
                                         "Do you want to replace it with the one you're moving?").arg(file_name);
      [alert setInformativeText:NSStringFromQString(informative_text)];
      [alert setAlertStyle:NSCriticalAlertStyle];

      BumpTopApp::makeSelfForegroundApp();
      NSInteger button_selection = [alert runModal];
      [alert release];

      if (button_selection == NSAlertFirstButtonReturn) {
        return false;
      } else {
        temp_file_name = file_name + ".backup";
        int i = 2;
        while (QFileInfo(QDir(new_file_parent_dir).filePath(temp_file_name)).exists()) {
          temp_file_name = file_name + QString(".backup (%1)").arg(i);
          i++;
        }
        created_temp_file = true;
        if (!QDir(new_file_parent_dir).rename(file_name, temp_file_name)) {
          return false;
        }
      }
    }
  }

  NSString* target_name;
  if (FileManager::isVolume(new_file_parent_dir)) {
    QString volume_name = new_file_parent_dir.mid(QString("/Volumes/").size());
    target_name = [NSString stringWithFormat:@"disk \"%@\"", NSStringFromQString(volume_name)];
  } else {
    target_name = [NSString stringWithFormat:@"item POSIX file \"%@\"", NSStringFromQString(new_file_parent_dir)];
  }

  if (operation == MoveOperation) {
    // construct apple script for move operation
    script_format = @"tell application \"Finder\"\n"
                        @"try\n"
                            @"set macpath to POSIX file \"%@\" as text\n"
                            @"move {item macpath} to %@\n"
                        @"on error error_message\n"
                            @"display dialog error_message\n"
                        @"end try\n"
                    @"end tell";
    script = [NSString stringWithFormat:script_format,
                        NSStringFromQString(file_path),
                        target_name];

    FileManager::runAppleScriptThroughNSTask(script);
    if (created_temp_file) {
      // TODO: should only do this if the applescript was successful
      QDir(new_file_parent_dir).remove(temp_file_name);
    }
    return true;
  } else if (operation == CopyOperation) {
    // construct apple script for copy operation
    script_format = @"tell application \"Finder\"\n"
                        @"try\n"
                            @"set macpath to POSIX file \"%@\" as text\n"
                            @"copy {item macpath} to %@\n"
                        @"on error error_message\n"
                            @"display dialog error_message\n"
                        @"end try\n"
                    @"end tell";
    script = [NSString stringWithFormat:script_format,
                        NSStringFromQString(file_path),
                        target_name];

    FileManager::runAppleScriptThroughNSTask(script);

    if (created_temp_file) {
      // TODO: should only do this if the applescript was successful
      QDir(new_file_parent_dir).remove(temp_file_name);
    }
    return true;
  } else {
    assert(false && "value of enum of type MoveOrCopy is not either Move or Copy");
    return false;
  }
}

bool FileManager::moveFile(QString file_path, QString new_file_parent_dir, BehaviorOnConflict behavior_on_conflict) {
  return moveOrCopyFileHelper(file_path, new_file_parent_dir, behavior_on_conflict, MoveOperation);
}

void FileManager::copyFile(QString file_path, QString new_file_parent_dir, BehaviorOnConflict behavior_on_conflict) {
  moveOrCopyFileHelper(file_path, new_file_parent_dir, behavior_on_conflict, CopyOperation);
}

bool FileManager::linkFile(QString file_path, QString new_file_parent_dir) {
  QString file_name = QFileInfo(file_path).fileName();
  QString new_file_path = new_file_parent_dir + "/" + file_name + " alias";

  int i = 2;
  while (QFileInfo(new_file_path).exists()) {
    new_file_path = new_file_parent_dir + "/" + file_name + QString(" alias %1").arg(i);
    i++;
  }

  NSError *error;
  BOOL success = [[NSFileManager defaultManager] createSymbolicLinkAtPath:NSStringFromQString(new_file_path)
                                                      withDestinationPath:NSStringFromQString(file_path)
                                                                    error:&error];
  if (!success) {
    [NSApp presentError:error];
  }

  return success;
}

QString FileManager::getParentPath(QString path) {
  return QFileInfo(path).path();
}

void FileManager::moveFileToTrash(QStringList paths_of_files_to_move_to_trash) {
  QString paths = getPathsInAppleScriptFormat(paths_of_files_to_move_to_trash);

  NSString* script_format = @"tell application \"Finder\" \n"
                                @"move %@ to the trash\n"
                            @"end tell";
  NSString* script = [NSString stringWithFormat:script_format, NSStringFromQString(paths)];

  FileManager::runAppleScriptThroughNSTask(script);
}

void FileManager::ejectDrive(QString file_path) {
  // if user is not admin, then local partitions can not be ejected (this action requires admin permission)
  // USB keys and Network can still be ejected.
  QString drive_name = QFileInfo(file_path).fileName();
  NSString* script_format_for_is_local = @"tell application \"Finder\" \n"
                                             @"local volume of disk \"%@\"\n"
                                         @"end tell";
  NSString* script_for_is_local = [NSString stringWithFormat:script_format_for_is_local, NSStringFromQString(drive_name)];
  NSAppleScript *desktop_script = [[NSAppleScript alloc] initWithSource:script_for_is_local];
  [desktop_script executeAndReturnError:nil];
  NSAppleEventDescriptor *desktop_script_return = [desktop_script executeAndReturnError:nil];
  [desktop_script release];

  if (![desktop_script_return booleanValue]) {
    NSTask* eject_drive = [[NSTask alloc] init];
    [eject_drive setLaunchPath:@"/sbin/umount"];
    [eject_drive setArguments: [NSArray arrayWithObjects:NSStringFromQString(file_path), nil]];
    [[NSNotificationCenter defaultCenter] addObserver:[NSTaskManager singleton]
                                             selector:@selector(taskExited:)
                                                 name:NSTaskDidTerminateNotification
                                               object:eject_drive];
    [eject_drive launch];
  } else {
    NSTask* eject_drive = [[NSTask alloc] init];
    [eject_drive setLaunchPath:@"/usr/sbin/diskutil"];
    [eject_drive setArguments: [NSArray arrayWithObjects:@"eject", NSStringFromQString(file_path), nil]];
    [[NSNotificationCenter defaultCenter] addObserver:[NSTaskManager singleton]
                                             selector:@selector(taskExited:)
                                                 name:NSTaskDidTerminateNotification
                                               object:eject_drive];
    [eject_drive launch];
  }
}

QString FileManager::getResourcePath() {
  return QStringFromNSString([[NSBundle mainBundle] resourcePath]);
}

QString FileManager::getApplicationDataPath() {
  NSArray* paths;
  paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, NO);
  NSString* application_support_path = [[paths objectAtIndex:0] stringByExpandingTildeInPath];
  NSString* bumptop_support_path = [application_support_path stringByAppendingString:@"/BumpTop/"];

  // Create the path if it doesn't exist
  if (![[NSFileManager defaultManager]  fileExistsAtPath:bumptop_support_path])
    [[NSFileManager defaultManager] createDirectoryAtPath:bumptop_support_path attributes:nil];

  return QStringFromNSString(bumptop_support_path);
}

QString FileManager::getBackgroundCachePath() {
  QString background_path = getApplicationDataPath() + "/backgrounds/";

  // First we make sure the directory exists and make it if not
  QDir background_dir = QDir(background_path);
  if (!background_dir.exists())
    background_dir.mkpath(background_dir.absolutePath());

  return background_path;
}

QString FileManager::getApplicationsPath() {
  NSArray* paths;
  paths = NSSearchPathForDirectoriesInDomains(NSApplicationDirectory, NSUserDomainMask, NO);
  NSString* application_path = [[paths objectAtIndex:0] stringByExpandingTildeInPath];

  return QStringFromNSString(application_path);
}

QString FileManager::getUserPath() {
  NSString* user_path = NSHomeDirectory();
  return QStringFromNSString(user_path);
}

QString FileManager::getDesktopPath() {
  NSArray* paths = NSSearchPathForDirectoriesInDomains(NSDesktopDirectory, NSUserDomainMask, NO);
  NSString* desktop_path = [[paths objectAtIndex:0] stringByExpandingTildeInPath];
  return QStringFromNSString(desktop_path);
}

QString FileManager::pathForResource(QString resource_name) {
  QFileInfo info = QFileInfo(resource_name);
  NSString* path = [[NSBundle mainBundle] pathForResource:NSStringFromQString(info.baseName())
                                                   ofType:NSStringFromQString(info.suffix())];
  return QStringFromNSString(path);
}

FileKind FileManager::getFileKind(QString path) {
  // TODO: looking into if this is the cause for Pat's computer's delay
  // suspect that it is all these QFileInfo operations that are causing the delays
  if (isVolume(path)) {
    return VOLUME;
  } else if (QFileInfo(path).suffix() == "app") {
    return APPLICATION;
  } else if (QFileInfo(path).isSymLink()) {
    return ALIAS;
  } else {
    return REGULAR_FILE;
  }
}

bool FileManager::isStartupDrive(QString path) {
  // the actual path of startup drive is "/" which is the local host but the path we are using is "/Volumes/(name)" which
  // is a link to the "/" so we can just check if the file is in Volumes and links to "/"
  return isVolume(path) && QFileInfo(path).readLink() == "/";
}

bool FileManager::isVolume(QString path) {
  return path.startsWith("/Volumes/") || path == "/";
}

bool FileManager::isEjectableDrive(QString path) {
  return FileManager::singleton()->ejectable_drives_.contains(path);
}

void FileManager::addEjectableDrive(QString path) {
  // Used to check if a drive is ejectable (BumpTopCommands)
  FileManager::singleton()->ejectable_drives_.push_back(path);
}

bool FileManager::isConnectedServer(QString path) {
  // Whether a drive is a connected server.
  // Used when hiding hidden connected servers.
  return FileManager::singleton()->connected_servers_.contains(path);
}

void FileManager::addConnectedServer(QString path) {
  FileManager::singleton()->connected_servers_.push_back(path);
}

void FileManager::removeConnectedServer(QString path) {
  FileManager::singleton()->connected_servers_.removeOne(path);
}

QString FileManager::getStartupDriveName() {
  if (FileManager::singleton()->startup_drive_name_ == "") {
    NSString* script = @"tell application \"Finder\"\n"
                          @"name of startup disk of application \"Finder\"\n"
                       @"end tell";
    NSAppleScript *desktop_script = [[NSAppleScript alloc] initWithSource:script];
    NSAppleEventDescriptor *desktop_script_return = [desktop_script executeAndReturnError:nil];
    [desktop_script release];
    FileManager::singleton()->startup_drive_name_ = QStringFromNSString([desktop_script_return stringValue]);
  }
  return FileManager::singleton()->startup_drive_name_;
}

bool FileManager::arePathsOnSameVolume(QString path1, QString path2) {
  QString volumes_prefix = "/Volumes/";
  bool path1_is_not_on_startup_volume = path1.startsWith(volumes_prefix) && !isStartupDrive(path1);
  bool path2_is_not_on_startup_volume = path2.startsWith(volumes_prefix) && !isStartupDrive(path2);

  if (path1_is_not_on_startup_volume != path2_is_not_on_startup_volume) {
    return false;
  } else if (!path1_is_not_on_startup_volume && !path2_is_not_on_startup_volume) {
    return true;
  } else {
    int third_slash_position_in_path1 = path1.indexOf("/", volumes_prefix.size());
    int third_slash_position_in_path2 = path2.indexOf("/", volumes_prefix.size());
    if (third_slash_position_in_path1 == -1 ||
        third_slash_position_in_path2 == -1) {
      return false;
    }
    return path1.mid(0, third_slash_position_in_path1) == path2.mid(0, third_slash_position_in_path2);
  }
}

QString FileManager::getDeletedFilesPath() {
  NSString *deleted_files_path = [NSHomeDirectory() stringByAppendingPathComponent:@".Trash"];
  return QStringFromNSString(deleted_files_path);
}

bool FileManager::launchPath(QString path) {
  NSString *ns_path = NSStringFromQString(path);
  NSURL *url_path = [NSURL fileURLWithPath: ns_path];
  return [[NSWorkspace sharedWorkspace] openURL: url_path];
}

// original_path is a full path, and new_name is just the name+extension
bool FileManager::renameFile(QString original_path, QString new_name) {
  NSString* ns_original_path = NSStringFromQString(original_path);
  NSString* ns_new_name = NSStringFromQString(new_name);
  NSString* new_path = [[ns_original_path stringByDeletingLastPathComponent] stringByAppendingPathComponent:ns_new_name];
  return [[NSFileManager defaultManager] movePath:ns_original_path toPath:new_path handler:nil];
}

bool FileManager::chooseApplicationAndLaunchPaths(QStringList paths) {
  NSOpenPanel *panel = [NSOpenPanel openPanel];
  [panel setDirectory:@"/Applications/"];
  [panel setTitle:@"Choose Application"];
  [panel setAllowedFileTypes:[NSArray arrayWithObjects:@"app", nil]];
  if (paths.count() == 1) {
    NSString* message = [NSString stringWithFormat:@"Choose an application to open the document \"%@\".",
                                                    NSStringFromQString(QFileInfo(paths[0]).completeBaseName())];
    [panel setMessage:message];
  }
  int result = [panel runModal];
  if (result == NSOKButton) {
    QString selected_app = QStringFromNSString([panel filename]);
    for_each(QString path, paths) {
      launchPath(path, selected_app);
    }
    return true;
  }
  return false;
}

bool FileManager::launchPath(QString path, QString app) {
  return [[NSWorkspace sharedWorkspace] openFile:NSStringFromQString(path) withApplication:NSStringFromQString(app)];
}

void FileManager::duplicateFile(QStringList paths_of_files_to_duplicate) {
  QString paths = getPathsInAppleScriptFormat(paths_of_files_to_duplicate);

  NSString* script_format = @"tell application \"Finder\" \n"
                                @"duplicate %@\n"
                            @"end tell";
  NSString* script = [NSString stringWithFormat:script_format, NSStringFromQString(paths)];

  FileManager::runAppleScriptThroughNSTask(script);
}

QString createAliasName(QDir dir, const QString& file_path) {
  QString alias_name = QFileInfo(file_path).fileName() + " alias";
  int number_to_append_to_alias_name = 2;

  while (QFileInfo(dir.filePath(alias_name)).exists() || FileManager::pending_alias_names.contains(alias_name)) {
    QString string_to_append_to_alias_name;
    string_to_append_to_alias_name.setNum(number_to_append_to_alias_name);
    string_to_append_to_alias_name = " " + string_to_append_to_alias_name;
    alias_name = QFileInfo(file_path).fileName() +" alias" + string_to_append_to_alias_name;
    number_to_append_to_alias_name++;
  }
  return alias_name;
}

void FileManager::makeAliasOfFile(QString file_path) {
  NSString* script;
  QString original_path;

  if (getFileKind(file_path) == ALIAS) {
    // if file is an alias, we want to get the original item the alias is pointing to and make alias of that
    if (QFileInfo(file_path).readLink() == "/") {
      // if the link is "/", it means that the alias is pointing to the startup drive; so, we want to get the
      // startup disk thats in /Volumes/
      original_path = "/Volumes/" + getStartupDriveName();
    } else {
      // gets the path to the original item the alias is pointing to
      original_path = QFileInfo(file_path).readLink();
    }
  } else {
    original_path = file_path;
  }

  QString desktop_path = BumpTopApp::singleton()->scene()->room()->path();

  // we need to create our own file name which is "alias _" with an incremental number at the end
  QString alias_name = createAliasName(QDir(desktop_path), original_path);
  pending_alias_names.append(alias_name);

  NSString* script_format = @"tell application \"Finder\"\n"
                                @"set macpath to POSIX file \"%@\" as text\n"
                                @"set folderpath to POSIX file \"%@\" as text\n"
                                @"make new alias file at folder folderpath to item macpath with properties {name:\"%@\"}\n"
                            @"end tell";
  script = [NSString stringWithFormat:script_format, NSStringFromQString(original_path),
                                                     NSStringFromQString(desktop_path),
                                                     NSStringFromQString(alias_name)];
  FileManager::runAppleScriptThroughNSTask(script);
}

void FileManager::emptyTrash() {
  NSString* script = @"tell application \"Finder\" to empty the trash";

  FileManager::runAppleScriptThroughNSTask(script);
}
