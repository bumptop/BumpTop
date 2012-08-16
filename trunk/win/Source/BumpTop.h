// Copyright 2012 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef BUMPTOP_H
#define BUMPTOP_H

#include "BT_Singleton.h"

class FileSystemActor;
class Pile;
class StickyNoteActor;

// workaround to allow for translated strings
class BumpTopStrings
{
	Q_DECLARE_TR_FUNCTIONS(BumpTopStrings)

	QHash<QString, QString> _strs;

private:
	friend class Singleton<BumpTopStrings>;
	BumpTopStrings();

public:
	QString getString(QString key);
};
#define BumpTopStr Singleton<BumpTopStrings>::getInstance()


struct less_distance_to_cam_eye;
class PhotoFrameActor;
class NxActorWrapper;
class BumpObject;


void DrawCrumpled(NxActorWrapper* a, float t_x, float t_z, bool DrawShadowVersion=false);
void DrawPeeledActor(NxActorWrapper* a, bool DrawShadowVersion=false);
void CalcMouseRaycastReport();
void EnableRotation(vector<NxActorWrapper*> v);
void ZeroAllActorMotion(NxActorWrapper* a, bool putToSleep = false);
void FindClosestPileToActor(NxActorWrapper* a, float & minDistSq, int & minI);
void InitNx();
void PressureLockEvent();
void setOnWallFocusHandler(boost::function<void()> onWallFocusHandler);
void setOnFloorFocusHandler(boost::function<void()> onFloorFocusHandler);
void HandleDoubleClick(NxActorWrapper * lastPickedActor, NxActorWrapper * currentlyPickedActor, int x, int y);
void MouseCallback(int button, int state, int x, int y);
void MotionCallback(int x, int y);
void KeyboardCallback(uint key, int x, int y);
void ArrowKeyCallback(int key, int x, int y);
bool RenderCallback();
void prePhysicsTimerCallback();
void physicsTimerCallback(int value);
void postPhysicsTimerCallback(int value);
void UpdateCameraPlanes();
void *CreateBumpObjectsFromDirectory(QString DirectoryPath, bool createFirstRunObjects);
PhotoFrameActor * CreatePhotoFrameHelper(QString sourcePath);
void CreateDefaultScenePhotoFrames();
void CreateCustomEmailActor();
void CreateCustomStickyNotePadActor();
void CreateCustomPrinterActor();
void CreateCustomFacebookActor();
void CreateCustomTwitterActor();
void CreateCustomFlickrActor();
QString CreateStickyNoteFileNameHelper();
StickyNoteActor * CreateStickyNote();
bool getStickyNote(QString filepath, QString * postItTextOut);
void writeStickyNote(QString filepath, QString text);
void processTossing();
void processInPileGhosting(bool render, int& indexOut, Pile ** pileOut);

#endif // BUMPTOP_H
