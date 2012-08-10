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

#import "BumpTop/OSX/NSTaskManager.h"

#include "BumpTop/BumpTopApp.h"
#include "BumpTop/BumpTopScene.h"
#include "BumpTop/FileManager.h"
#include "BumpTop/FileSystemEventDispatcher.h"
#include "BumpTop/NSTaskDeletionManager.h"
#include "BumpTop/QStringHelpers.h"
#include "BumpTop/Room.h"
#include "BumpTop/VisualPhysicsActor.h"

static NSTaskManager *singleton_ = NULL;

@implementation NSTaskManager

-(void)taskExited:(NSNotification *)note {
  QString script = QStringFromNSString([[[note object] arguments] objectAtIndex:1]);
  if (script.lastIndexOf("make new alias") > 0) {
    // if task is for making alias then get alias name from the task and remove it from pending_alias_names
    int starting_index_of_alias_name = script.lastIndexOf("{") + 7;
    int ending_index_of_alias_name = script.lastIndexOf("}") - 1;
    QString alias_name = script.mid(starting_index_of_alias_name, ending_index_of_alias_name - starting_index_of_alias_name);
    FileManager::pending_alias_names.removeOne(alias_name);
  } else if (script.lastIndexOf("to the trash") > 0) {
    for_each(VisualPhysicsActor* actor, BumpTopApp::singleton()->scene()->room()->room_actor_list()) {
      if (actor->path() != "") {
        if (!QFileInfo(actor->path()).exists()) {
          FileSystemEventDispatcher::singleton()->removePathToWatchOnFileDeletion(actor->path());
          BumpTopApp::singleton()->scene()->room()->fileRemoved(actor->path());
        }
      }
    }
  } else if (script.lastIndexOf("Get finder label colour and set label colour of" > 0)) {
    BumpBoxLabelColour label_colour = FileManager::bumpBoxLabelColourFromFinderLabelColour([NSTaskManager standardOutAsInt:note]);

    VisualPhysicsActorId actor_id = script.mid(script.lastIndexOf(':')+1).toInt();
    if (BumpTopApp::singleton()->scene()->room()->containsDirectlyOrThroughChild(actor_id)) {
      VisualPhysicsActor* actor = BumpTopApp::singleton()->scene()->room()->flattened_actor_with_unique_id(actor_id);
      actor->set_label_colour(label_colour);
      actor->updateLabel(1);
    }
  }

  // we don't want to release the task here because of the chance for getting more than 1 notification for the same task
  // within a single render tick; so, we put the task into NSTaskDeletionManager to be released on the next render tick
  NSTaskDeletionManager::singleton()->add_task_to_delete([note object]);
}

+(NSString*)standardOutAsNSString:(NSNotification *)note {
  NSPipe* pipe = [[note object] standardOutput];
  NSFileHandle* file_handle = [pipe fileHandleForReading];
  NSData* data = [file_handle readDataToEndOfFile];
  return [[NSString alloc] initWithData:data encoding: NSUTF8StringEncoding];
}

+(int)standardOutAsInt:(NSNotification *)note {
  return QStringFromNSString([NSTaskManager standardOutAsNSString:note]).toInt();
}

+(NSTaskManager*)singleton {
  if (singleton_ == NULL) {
    singleton_ = [[NSTaskManager alloc] init];
  }
  return singleton_;
}

@end

