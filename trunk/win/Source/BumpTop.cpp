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

#include "BT_Common.h" 
#include "BT_Actor.h"
#include "BT_AnimationManager.h"
#include "BT_Authorization.h"
#include "BT_BumpObject.h"
#include "BT_Camera.h"
#include "BT_Cluster.h"
#include "BT_CustomActor.h"
#include "BT_CustomQLineEdit.h"
#include "BT_DialogManager.h"
#include "BT_EventManager.h"
#include "BT_FileSystemActor.h"
#include "BT_FileSystemActorFactory.h"
#include "BT_FileSystemManager.h"
#include "BT_FileSystemPile.h"
#include "BT_FileTransferManager.h"
#include "BT_Find.h"
#include "BT_GLTextureManager.h"
#include "BT_KeyCombo.h"
#include "BT_KeyboardManager.h"
#include "BT_LassoMenu.h"
#include "BT_Logger.h"
#include "BT_Macros.h"
#include "BT_MarkingMenu.h"
#include "BT_MouseEventManager.h"
#include "BT_MousePointer.h"
#include "BT_MultipleMice.h"
#ifdef DXRENDER
	#include "BT_DXRender.h"
#endif
#include "BT_OverlayComponent.h"
#include "BT_PhotoFrameActor.h"
#include "BT_PhotoFrameSource.h"
#include "BT_Pile.h"
#include "BT_Profiler.h"
#include "BT_RaycastReports.h"
#include "BT_Rename.h"
#include "BT_RenderManager.h"
#include "BT_Replayable.h"
#include "BT_RepositionManager.h"
#include "BT_SceneManager.h"
#include "BT_Selection.h"
#include "BT_Selection.h"
#include "BT_SlideShow.h"
#include "BT_StatsManager.h"
#include "BT_StickyNoteActor.h"
#include "BT_StickyNoteMenuActionCustomizer.h"
#include "BT_Struct.h"
#include "BT_TextManager.h"
#include "BT_Timer.h"
#include "BT_Training.h"
#include "BT_UndoStack.h"
#include "BT_Util.h"
#include "BT_WatchedObjects.h"
#include "BT_WebActor.h"
#include "BT_Windows7Multitouch.h"
#include "BT_WindowsOS.h"
#include "BT_WebThumbnailActor.h"
#include "BT_Library.h"

#include "BT_Bubbles.h"

#include "BumpTop.h"

//Texture and OBJ Variables
#define MAX_DESKTOP_ITEM_TEXTURES 12 //How many of the first g_Texture's to use for the Random Desktop Items

void CalcMouseRaycastReport()
{
	SimpleRaycast gMyReport;

	//Check if Mouse is Over Wall
	Vec3 line_start, line_end;
	window2world(GLOBAL(mx), GLOBAL(my), line_start, line_end);
	assert(line_start.isFinite() && line_end.isFinite());
	Ray worldRay;
	worldRay.orig	= line_start;
	worldRay.dir	= line_end - line_start;
	worldRay.dir.normalize();
	GLOBAL(gScene)->raycastAllShapes(worldRay, gMyReport, NX_STATIC_SHAPES);

	// fix up the selected actor orientation
	if (sel->getPickedActor())
	{
		BumpObject * obj = sel->getPickedActor();
		if (obj->isObjectType(ObjectType(BumpActor)) && !obj->isParentType(BumpPile))
		{
			bool adjustInsideWalls = false;

			if (GLOBAL(MouseOverWall))
			{
				NxActorWrapper * wall = GLOBAL(PinWall);
				
				// we just disable rotations when we are over a wall
				// (this get re-enabled on the object's onDragEnd() handler)
				obj->setRotation(false);
					
				// reorient the actor to the wall orientation that we are currently over
				// consoleWrite(QString("angleFromFloor: %1\n").arg((float) obj->getFrontFacePlane().normal.dot(Vec3(0,1,0))));
				if (obj->getFrontFacePlane().normal.dot(wall->getFrontFacePlane().normal) * 180 / PI < 45)
				{
					obj->setGlobalOrientation(wall->getGlobalOrientation());
					adjustInsideWalls = true;
				}
			}
			else
			{				
				// otherwise, reorient to the floor orientation
				if (obj->getFrontFacePlane().normal.dot(Vec3(0,1,0)) * 180 / PI < 45)
				{
					obj->setGlobalOrientation(GLOBAL(straightIconOri));
					adjustInsideWalls = true;
				}
			}

			if (adjustInsideWalls) 
			{
				// adjust the box to be inside the bounds
				Box obb = obj->getBox();
				adjustBoxToInsideWalls(obb);
				obj->setGlobalPosition(obb.center);
			}
		}
	}
}

void EnableRotation(vector<NxActorWrapper*> v)
{
	for(int i=0; i<v.size(); i++)
		v[i]->clearBodyFlag(NX_BF_FROZEN_ROT);
}

void ZeroAllActorMotion(NxActorWrapper* a, bool putToSleep)
{
	if(a!=NULL)
	{
		a->setLinearMomentum(Vec3(0.0f));
		a->setLinearVelocity(Vec3(0.0f));
		a->setAngularMomentum(Vec3(0.0f));
		a->setAngularVelocity(Vec3(0.0f));
		a->setForce(Vec3(0.0f));
		a->setTorque(Vec3(0.0f));
		if(putToSleep) a->putToSleep();

		//a->raiseBodyFlag
		//a->raiseActorFlag(NX_AF_DISABLE_COLLISION);
	}
}

//Searches for closest pile in 'piles' to Actor a.
//Note: Does NOT search selection!  Returns mindist=minI=-1 if no pile close
void FindClosestPileToActor(NxActorWrapper* a, float & minDistSq, int & minI)
{	
	Vec3 c= a->getGlobalPosition();
	BumpObject *object = (BumpObject *) GetBumpActor(a);
	minDistSq=-1;
	minI=-1;

	if (!object->getParent())
	{
		for(int i=0; i<GLOBAL(getPiles()).size(); i++)
		{
			Pile *p = GLOBAL(getPiles())[i];
			if (p->getNumItems() > 0)
			{
				float d=((*p)[0]->getGlobalPosition() - c).magnitudeSquared();
				if(minDistSq==-1 || d<minDistSq)
				{
					minDistSq=d;
					minI=i;
				}
			}
		}
	}
}

void InitNx()
{
	QString renderer;
#ifdef DXRENDER
	D3DADAPTER_IDENTIFIER9 adapterIdentifier = {0};
	renderer = QT_NT("DXR - ");
	if (SUCCEEDED(dxr->d3d->GetAdapterIdentifier(D3DADAPTER_DEFAULT, 0, &adapterIdentifier)))
		renderer += QString(adapterIdentifier.Description);
#else
	renderer = QString((char *) glGetString(GL_RENDERER));
	// notify the user if they are using a GDI renderer and they are on the desktop
	if (!scnManager->isShellExtension && (renderer.startsWith("gdi", Qt::CaseInsensitive)) && !(winOS->ignoreGDI()))
	{
		winOS->FailOnDrivers();
	}
#endif
	consoleWrite(QString_NT("Renderer:  %1\n").arg(renderer)); //are we using a hardware renderer
	statsManager->getStats().hardware.vidRenderer = stdString(renderer);

	

	CreateWalls();

	// Get the size of the work are afrom windows
	int r, t, l, b;
	winOS->GetWorkArea(r, l, t, b);
	ResizeWallsToWorkArea(r - l, b - t);

	// Move the camera into position so it encalpsuates the entire work area
	Bounds bounds;
	Vec3 oldEye = cam->getEye(), oldUp = cam->getUp(), oldDir = cam->getDir();
	Vec3 newEye, newUp, newDir;

	// Find out the region where this camera will look at
	for (int i = 0; i < GLOBAL(Walls).size(); i++)
	{
		Vec3 wallsPos = GLOBAL(WallsPos)[i];
		wallsPos.y = 0;
		bounds.include(wallsPos);
	}

#ifdef HWMANFDEMOMODE
	scnManager->zoomToAngleBoundsTempOverride = 0;
#endif

	if (!GLOBAL(isInTrainingMode))
	{
		// load the startup scene if necessary
		if (!scnManager->startupDemoSceneDir.isNull())
		{
			loadDemoSceneFromDirectory(QDir(scnManager->startupDemoSceneDir));
		}
		else if (LoadSceneFromFile())
		{
			// undoManager->invalidate();
		}
		else
		{
			if (scnManager->isShellExtension)
				Key_ToggleInfiniteDesktopMode();
			CreateBumpObjectsFromWorkingDirectory();
		}
	}

	// In photo mode, we want certain actors to be frozen
	if (scnManager->isBumpPhotoMode)
	{
		CustomActor* actor = scnManager->getCustomActor<EmailActorImpl>();
		actor->raiseBodyFlag(NX_BF_FROZEN);
		actor->raiseBodyFlag(NX_BF_FROZEN_ROT);
		actor = scnManager->getCustomActor<TwitterActorImpl>();
		actor->raiseBodyFlag(NX_BF_FROZEN);
		actor->raiseBodyFlag(NX_BF_FROZEN_ROT);
		actor = scnManager->getCustomActor<FacebookActorImpl>();
		actor->raiseBodyFlag(NX_BF_FROZEN);
		actor->raiseBodyFlag(NX_BF_FROZEN_ROT);
	}

	if (GLOBAL(settings).dropItemsIntoSceneOnStartup &&
		!scnManager->isShellExtension && !scnManager->isInInfiniteDesktopMode)
	{
		// position each item above the camera and let them drop?
		// (if we animate them slowly, we can get away with setting them at the top
		// of the desktop box)
		Box desktop = GetDesktopBox();
		vector<BumpObject *> objects = scnManager->getBumpObjects();
		float randNum = float((rand() % 100)) / 25.0f;	// 0 <= rN < 0.02
		for (int i = 0; i < objects.size(); ++i)
		{
			BumpObject * obj = objects[i];
			if (obj->getParent()) continue;
			if (obj->isBumpObjectType(BumpPile)) continue;

			obj->setAlpha(0.0f);
			obj->setAlphaAnim(0.0f, 1.0f, 100);
			obj->setGlobalOrientation(obj->getGlobalOrientation() * Mat33(Quat(float((rand() % 60) + 10), Vec3(0, 1, 0))));
			obj->setLinearVelocity(Vec3(0, float((rand() % 200) + 150), 0));
		}
	}
	else
	{
		// zoom the camera out if we have resized the walls
		if (!GLOBAL(DrawWalls))
		{
			if (GLOBAL(isInInfiniteDesktopMode))
			{
				// zoom camera outwards from close in
				cam->skipAnimation();
				cam->setEye(Vec3(0,10,0));

				// re-add the popped actors (from Key_SetCameraAsAngled) if in zui mode
				cam->pushWatchActors(scnManager->getBumpObjects());
			}
			else
			{
#ifdef TABLE			
				Key_SetCameraAsTopDown();
#else
				Key_SetCameraAsAngled();
#endif
			}
		}
		// always enable infinite desktop mode for bump photo mode
		else
		{					
			if (scnManager->isBumpPhotoMode)
			{
				// XXX: Temporary hack to fix camera issues in photo mode
				cam->finishAnimation();
				animManager->finishAnimation(cam);
				Key_ToggleInfiniteDesktopMode();
			}
		}
	}

	if (!scnManager->isShellExtension)
	{
		// workaround for the fact that the window may not be active initially
		// if the window is top most then try and bring focus to it
		if (winOS->IsWindowTopMost())
		{
			winOS->SetFocusOnWindow();
		}
	}
	else if (scnManager->isShellExtension)
	{
		// so that when we click "Bump this folder", we can immediately type keyboard 
		// shortcuts or names of files
		winOS->SetFocusOnWindow();
	}

	// update the shell extension status bar if necessary after loading  the scene
	if (scnManager->isShellExtension)
		winOS->ShellExtUpdateStatusBar();

	// load the camera view
	if (!GLOBAL(isInInfiniteDesktopMode))
	{
		cam->loadCameraFromPreset(GLOBAL(settings).cameraPreset);
		cam->finishAnimation();
	}
}

void PressureLockEvent()
{
	BumpObject * obj = sel->getPickedActor();

	// ensure valid/not parented object
	if (!obj || obj->isParentType(BumpPile))
		return;
	assert(!obj->isBumpObjectType(BumpWidget));

	/*
	// NOTE: we don't allow piles to be pinned since any operation on them currently 
	//		 throw them to the floor
	if (obj->isBumpObjectType(BumpPile))
	return;
	assert(obj->isBumpObjectType(BumpActor));
	*/

	// do raycast
	CalcMouseRaycastReport();

	// pin the actor to the wall and freeze it's motion
	Actor * data = (Actor *) obj;
	assert(!obj->isBumpObjectType(BumpWidget));
	assert(!data->isActorType(Invisible));

	if(GLOBAL(MouseOverWall))
	{
		// to check whether an item is near the wall, we are just going to 
		// check whether it is NOT in the min bounds of the desktop
		// Box minDesktopBox = GetDesktopBox(-10.0f);
		// if (!minDesktopBox.containsPoint(obj->getGlobalPosition()))

		if (!obj->isPinned() && !obj->isBumpObjectType(BumpPile))
			PinItemToWall(obj);
	}
}

boost::function<void()> _onWallFocusHandler;
void setOnWallFocusHandler( boost::function<void()> onWallFocusHandler )
{
	_onWallFocusHandler = onWallFocusHandler;
}

boost::function<void()> _onFloorFocusHandler;
void setOnFloorFocusHandler( boost::function<void()> onFloorFocusHandler )
{
	_onFloorFocusHandler = onFloorFocusHandler;
}

void HandleDoubleClick(NxActorWrapper * lastPickedActor, NxActorWrapper * currentlyPickedActor, int x, int y)
{
	Key_UnsetSharingMode();
	markingMenu->destroy();

	// do a line pick if we are handling a raw dbl click message
	if (!lastPickedActor && !currentlyPickedActor)
	{
		pickAndSet(x, y);
		lastPickedActor = sel->getPickedActor();
	}

	BumpObject * obj = GetBumpActor(currentlyPickedActor);
	if (lastPickedActor && currentlyPickedActor && obj && currentlyPickedActor == lastPickedActor)
	{	
		if (obj->isBumpObjectType(BumpCluster))
		{
			//the coordinates must be picking an actor, so assert it picks it again
			Cluster *c = (Cluster*)obj;
			c->onLaunch();
		}
		else if (obj->isBumpObjectType(BumpPile))
		{
			Pile * pile = (Pile *) obj;
			pile->onLaunch();
		}
		else if (obj->isBumpObjectType(BumpActor))
		{
			bool launchTarget = true;
			Actor * actor = (Actor *) obj;
			FileSystemActor * fsActor = NULL;
			if (actor->isActorType(FileSystem))
				fsActor = (FileSystemActor *) actor;

			// try and launch the folder as a gridded pile
			if (GLOBAL(settings).launchFoldersAsInGrid &&
				fsActor &&
				fsActor->isFileSystemType(Folder) &&
				!fsActor->isFileSystemType(Removable))
			{
				// NOTE: make sure that it is not already in a gridded pile
				if (!fsActor->getParent())
				{
					// ensure that this folder has things in it first
					StrList dirListing = fsManager->getDirectoryContents(fsActor->getTargetPath());
					if (!dirListing.empty())
					{
						sel->clear();
						sel->add(actor);

						// save the pile references
						vector<Pile *> existingPiles = scnManager->getPiles();

						int threshold = PILE_GRID_ROWSIZE * PILE_GRID_COLSIZE;
						if (dirListing.size() <= threshold)
						{						
							Vec3 pos = actor->getGlobalPosition();
							Key_PileizeAndGridView();
							launchTarget = false;

							// cross reference with the old set of piles, marking the new piles
							// to folderize on close
							vector<Pile *> newPiles = scnManager->getPiles();
							for (int i = 0; i < newPiles.size(); ++i)
							{
								Pile * p = (Pile *) newPiles[i];
								if (p->getObjectType() == ObjectType(BumpPile, HardPile))
								{
									FileSystemPile * fsPile = (FileSystemPile *) p;
									if (find(existingPiles.begin(), existingPiles.end(), newPiles[i]) ==
										existingPiles.end())
									{
										fsPile->setFolderizeOnClose(true, pos);
									}
								}
							}
						}
					}
				}
			}

			if (launchTarget && actor) {
				// move this actor to the top of the selection set
				sel->insert(actor, 0);

				// if I don't set the picked actor to null here, additional mouse events are processed for it
				// and after launching, the item could shoot off in bumptop
				sel->setPickedActor(NULL);

				// This is a float click on an actor
				actor->onLaunch();
			}

			// finish the mode before the animation launches and bt loses focus
			FinishModeBasedOnSelection();
		}
	}
	else 
	{						
		if (winOS->IsWindowTopMost()) 
		{
			Vec2 pt;
			pt.setx(x);
			pt.sety(y);
			if (!currentlyPickedActor && GLOBAL(MouseOverWall))
			{
				Vec3 camPos, camDir;
				cam->lookAtWall(GLOBAL(PinWall), camPos, camDir);
				cam->animateTo(camPos, camDir);
			} 
			else
			{
				// toggle between the top down and the default view if we are in
				// that mode
				if (cam->getCurrentCameraView() == Camera::DefaultView)
					cam->setCurrentCameraView(Camera::TopDownView, true);
				else if (cam->getCurrentCameraView() == Camera::TopDownView)
					cam->setCurrentCameraView(Camera::DefaultView, true);
				else
					cam->loadCameraFromPreset(GLOBAL(settings).cameraPreset);
			}

			// when camera zooms out (overhead or wall), also zoom out web actor if zoomed in
			if (!cam->getZoomedObjects().empty())
				WebActor::zoomOutFocusedWebActor();

			if (!GLOBAL(MouseOverWall) && !GLOBAL(isInInfiniteDesktopMode) && !_onFloorFocusHandler.empty())
				_onFloorFocusHandler();
			else if (GLOBAL(MouseOverWall) && !_onWallFocusHandler.empty())
				_onWallFocusHandler();			
		} 
		else if (!winOS->IsWindowTopMost()) 
		{ 
			// don't jump into Show Desktop mode if nothing is obscuring BumpTop anyway
			if (winOS->GetWindowState() == WorkArea && winOS->isAttachedToDesktopWindow())
			{
				printUnique("showDesktopNotification", BumpTopStr->getString("ShowDesktopNotification"));
				winOS->ToggleShowWindowsDesktop();
				statsManager->getStats().bt.window.activatedFromDoubleClick++;

			}
		}
	}	
}

void MouseCallback(int button, int state, int x, int y)
{
	bool WallsClicked = false;
	GLOBAL(isInteraction) = true;
	int dx = GLOBAL(mx) - x;
	int dy = GLOBAL(my) - y;
	int activeButton = 0;
	int touchBrowseThreshold = 75; // num pixels in either direction that is perpendicular of the direction that is being dragged in
	GLOBAL(mouseMoving) = false;

	GLOBAL(mx) = x;
	GLOBAL(my) = y;

	GLOBAL(mstate) = state;
	activeButton = button;
	rndrManager->invalidateRenderer();

	if (GLOBAL(settings).enableTouchGestureBrowse && GLOBAL(touchGestureBrowseMode))
	{
		if (activeButton & MouseButtonLeft && state == GLUT_DOWN)
		{
			sel->setPickedActor(NULL);
			GLOBAL(clickDistance) = GLOBAL(sglClickPos).distanceSquared(Vec3(float(x),float(y),0));
			GLOBAL(sglClickPos).set(float(x),float(y),0);

		}else if (activeButton & MouseButtonLeft && state == GLUT_UP)
		{
			pickAndSet(x, y);

			if (abs(GLOBAL(sglClickPos).x - float(x)) < 5 && abs(GLOBAL(sglClickPos).y - float(y)) < 5)
			{
				// Removed double click on FileSystemActor to launch in touchGestureBrowseMode
			}else{
				// float click on the floor (or anwher else where there are no actors)
				if (sel->getPickedActor() && !GetBumpActor(sel->getPickedActor()) && GLOBAL(dblClickTimer).elapsed() < GLOBAL(dblClickInterval) && 
					GLOBAL(clickDistance) <= GLOBAL(dblClickSize) * GLOBAL(dblClickSize)) //float Click
				{
					ArrowKeyCallback(KeyDown, 0, 0);
				}else{
					// Release
					if (GLOBAL(mousePointList).size() > 2)
					{
						bool isCorrectDirection = true;

						// The start and End pointers are either horisontal or vertical
						for (int i = 0; isCorrectDirection && i < GLOBAL(mousePointList).size(); i++)
						{
							// Figure out if the line is in a correct direction.
							if (!(abs(GLOBAL(mousePointList)[0].x - GLOBAL(mousePointList)[i].x) <= touchBrowseThreshold ||
								abs(GLOBAL(mousePointList)[0].y - GLOBAL(mousePointList)[i].y) <= touchBrowseThreshold))
							{
								isCorrectDirection = false;
							}
						}

						if (isCorrectDirection && 
							winOS->GetWindows7Multitouch() &&
							!winOS->GetWindows7Multitouch()->areMinimumTouchPointsActive(2))
						{
							// Figure out the direction 
							if (abs(GLOBAL(mousePointList).front().x - GLOBAL(mousePointList).back().x) > abs(GLOBAL(mousePointList).front().y - GLOBAL(mousePointList).back().y))
							{
								// the Move was left/right
								if (GLOBAL(mousePointList).front().x <= GLOBAL(mousePointList).back().x)
								{
									// The move was left
									ArrowKeyCallback(KeyLeft, 0, 0);
								}
								else
								{
									// the move was right
									ArrowKeyCallback(KeyRight, 0, 0);
								}
							}
							else
							{
								// the move was up/down
								if (GLOBAL(mousePointList).front().y > GLOBAL(mousePointList).back().y)
								{
									// Up key sim
									ArrowKeyCallback(KeyUp, 0, 0);
								}
								else
								{
									// Down key sim
									ArrowKeyCallback(KeyDown, 0, 0);
								}
							}
						}

						GLOBAL(mousePointList).clear();
					}
				}

				sel->setPickedActor(NULL);


				GLOBAL(dblClickTimer).restart();

			}

		}
	}
	else
	{
		if (activeButton & MouseButtonScrollUp) //Zoom in
		{
			vector<BumpObject *> selection = sel->getBumpObjects();
			if (!selection.empty() &&
				(winOS->IsKeyDown(KeyControl) && !winOS->IsKeyDown(KeyLeftShift)) || GLOBAL(settings).scrollingGrowsItems)
			{
				// grow the selection
				for (int i = 0; i < selection.size(); ++i)
					selection[i]->grow(15, 1.4f);
			}
			else
			{
				BumpObject * obj = NULL;
				Pile * pile = NULL;

				// check if there is a pile under the cursor
				tuple<NxActorWrapper*, BumpObject*, Vec3> pickResult = pick(x, y);
				obj = pickResult.get<1>();
				if (obj && obj->isBumpObjectType(BumpPile)) 
					pile = (Pile *) obj;

				// if there is no pile under the cursor, the check if there is one in the selection
				if (!pile) 
				{
					if (selection.size() == 1)
					{
						obj = selection.front();
						if (obj->isBumpObjectType(BumpPile))
							pile = (Pile *) obj;
						else if (obj->isParentType(BumpPile))
							pile = (Pile *) obj->getParent();
					}
				}

				// if there is some pile that we can forward the scroll event to, then do it
				if (pile)
				{
					dismiss("MouseCallback_Leaf_InvalidState");
					switch (pile->getPileState())
					{
					case Stack:
						if (GLOBAL(settings).freeOrProLevel == AL_PRO)
							pile->leafToBottom();
						break;
					case Leaf:
						if (GLOBAL(settings).freeOrProLevel == AL_PRO)
							pile->leafUp();
						break;
					case Grid:
						pile->scrollGridRow(-1);
						break;
					default:
						printUnique("MouseCallback_Leaf_InvalidState", BumpTopStr->getString("LeafStackedPiles"));
						break;
					}
				}
			}

			// Dont carry over this mouse action
			button = 0;
			activeButton = 0;
		}
		else if (activeButton & MouseButtonScrollDn) //Zoom out
		{
			vector<BumpObject *> selection = sel->getBumpObjects();
			if (!selection.empty() &&
				(winOS->IsKeyDown(KeyControl) && !winOS->IsKeyDown(KeyLeftShift)) || GLOBAL(settings).scrollingGrowsItems)
			{
				// shrink the selection
				for (int i = 0; i < selection.size(); ++i)
					selection[i]->shrink(15, 0.7f);
			}
			else
			{
				BumpObject * obj = NULL;
				Pile * pile = NULL;

				// check if there is a pile under the cursor
				tuple<NxActorWrapper*, BumpObject*, Vec3> pickResult = pick(x, y);
				obj = pickResult.get<1>();
				if (obj && obj->isBumpObjectType(BumpPile)) 
					pile = (Pile *) obj;

				// if there is no pile under the cursor, the check if there is one in the selection
				if (!pile) 
				{
					if (selection.size() == 1)
					{
						obj = selection.front();
						if (obj->isBumpObjectType(BumpPile))
							pile = (Pile *) obj;
						else if (obj->isParentType(BumpPile))
							pile = (Pile *) obj->getParent();
					}
				}

				// if there is some pile that we can forward the scroll event to, then do it
				if (pile)
				{
					dismiss("MouseCallback_Leaf_InvalidState");
					switch (pile->getPileState())
					{
					case Stack:
					case Leaf:
						if (GLOBAL(settings).freeOrProLevel == AL_PRO)
							pile->leafDown();
						break;
					case Grid:
						pile->scrollGridRow(1);
						break;
					default:
						printUnique("MouseCallback_Leaf_InvalidState", BumpTopStr->getString("LeafStackedPiles"));
						break;
					}
				}
			}

			// Dont carry over this mouse action
			button = 0;
			activeButton = 0;
		}
		else if (activeButton & MouseButtonLeft)//Left mouse hardwired pick + drag.
		{
			if (state == GLUT_DOWN)
			{
				if (GLOBAL(mode) != None)
				{
					FinishModeBasedOnSelection();

				}else{
					if (!markingMenu->isEnabled())
					{
						// because the overlays now handle the selection of the actor's text names, 
						// we should not reset and pick through if the picked actor has already been set
						// if (!sel->getPickedActor())
						pickAndSet(x, y);

						GLOBAL(clickDistance) = GLOBAL(sglClickPos).distanceSquared(Vec3(float(x),float(y),0));
						GLOBAL(sglClickPos).set(float(x),float(y),0);

						if (sel->getPickedActor())
						{
							BumpObject *obj = (BumpObject *) sel->getPickedActor();

							if (obj && obj->isDynamic() && !obj->isBumpObjectType(BumpWidget))
							{
								Pile * pile =  NULL;
								if (obj->isParentType(BumpPile)) pile = (Pile*)obj->getParent();
								else if (obj->isBumpObjectType(BumpPile)) pile = (Pile*)obj;
								
								bool addedPickedActor = false;
								if (obj)
								{
									if (vecContains(sel->getBumpObjects(), obj) == -1)
									{
										// Click on new Item (clear and selection new item)										
										if (!winOS->IsKeyDown(KeyControl))
											sel->clear();

										sel->add(obj);
										if (obj->isBumpObjectType(BumpActor)) 
										{
											Vec2 point;
											point.x = (NxReal)x;
											point.y = (NxReal)y;
											((Actor*)obj)->onTouchDown(point, mouseManager->getPrimaryTouch());
										}

										sel->setPickedActor(obj);
										obj->onClick();
									}
									else
									{
										if (winOS->IsKeyDown(KeyControl) && 
											sel->isInSelection(obj) &&
											!(GLOBAL(dblClickTimer).elapsed() < GLOBAL(dblClickInterval) && 
											GLOBAL(clickDistance) <= GLOBAL(dblClickSize) * GLOBAL(dblClickSize)))
										{
											// XXX: Temporarily disabling the deselection of lasso'd items due to 
											// mouse handling quirks
											sel->remove(obj);
											sel->setPickedActor(NULL);
										}
										else
										{
											// Clicked on a selected item, enable dragging on the group
											vector<BumpObject *> objs = sel->getBumpObjects();

											// Flag as being dragged
											for (uint i = 0; i < objs.size(); i++)
											{
												sel->add(objs[i]);
												if (objs[i]->isBumpObjectType(BumpActor))
												{
													Vec2 point;
													point.x = (NxReal)x;
													point.y = (NxReal)y;
													((Actor*)objs[i])->onTouchDown(point, mouseManager->getPrimaryTouch());
												}
											}

											sel->setPickedActor(obj);
											obj->onClick();
										}
									}
								}
							}
						}
						else
						{
							sel->setPickedActor(NULL);

							// if there is only one leafed item and the user selects on the desktop,
							// then we can try closing the pile
							vector<Pile *> piles = scnManager->getPiles();
							for (int i = 0; i < piles.size(); ++i) 
							{
								if (piles[i]->getPileState() == Leaf)
									piles[i]->close();
							}

							// if there is a zoomed web actor, then clicking outside should be zooming out
							if (!WebActor::zoomOutFocusedWebActor()) //else clicking outside should be deselection
							{
								if (!winOS->IsKeyDown(KeyControl))
									sel->clear();
							}
						}

						// XXX: HACKY way to act on ONLY single click events...
						//		we set the useSingleClickExpiration, and if we hit another
						//		click before the expiration is over, then we disable it.  
						//		otherwise, in the timer callback, if singleClickExpiration
						//		is used, and we are over the expiration interval, then we
						//		do the work.  
						if (isSlideshowModeActive())
						{
							if (GLOBAL(useSingleClickExpiration))
							{
								// we are in the single click, and have received the second click.
								// if the time is within the interval, disable single click expiration checking
								if (GLOBAL(singleClickExpirationTimer).elapsed() < GLOBAL(dblClickInterval))
								{
									GLOBAL(useSingleClickExpiration) = false;
								}
							}
							else
							{
								// this is the first click, so enable single click expiration checking
								GLOBAL(useSingleClickExpiration) = true;
								GLOBAL(singleClickExpirationTimer).restart();
								BumpObject* obj = GetBumpObject(sel->getPickedActor());
								if (obj)
									obj->putToSleep();
							}
						}

						textManager->invalidate();
					}

				}

			}
			else //LEFT BUTTON UP, MouseUp
			{
				// Finish Dragging when user mouses up
				bool isDraggingObjects = false;
				bool isDraggingOverDropObject = false;
				vector<BumpObject *> objs = sel->getBumpObjects();
				for (int i = 0; i < objs.size(); i++)
				{
					if (objs[i]->isDragging())
					{
						isDraggingObjects = true;
						isDraggingOverDropObject = objs[i]->isDragHovering();
						objs[i]->onDragEnd();
						if (objs[i]->isBumpObjectType(BumpActor))
						{
							Vec2 point;
							point.x = (NxReal)x;
							point.y = (NxReal)y;
							((Actor*)objs[i])->onTouchUp(point, mouseManager->getPrimaryTouch());
						}

						objs = sel->getBumpObjects();
						i = -1;
					}
				}
				
				// if there were no items dragging (ie. only a single click on another actor,
				// close any leafed piles)
				BumpObject* pickedObj = sel->getPickedActor();
				Pile* parent = NULL;
				if (pickedObj)
				{
					if (pickedObj->getParent() && pickedObj->getParent()->isObjectType(BumpPile))
						parent = (Pile*)pickedObj->getParent();
				}
				
				if (!isDraggingObjects)
				{
					vector<Pile *> piles = scnManager->getPiles();
					for (int i = 0; i < piles.size(); ++i) 
					{
						if (piles[i]->getPileState() == Leaf && piles[i] != parent)
							piles[i]->close();
					}
				}

				// clear the previously saved poses
				GLOBAL(initialDragObjects).clear();

				FinishModeBasedOnSelection();

				// Invoke a close widget
				if (sel->getPickedActor() && GetBumpActor(sel->getPickedActor())->getParent() && sel->getPickedActor()->isBumpObjectType(BumpWidget))
				{
					// XXX-TEMP: should actually invoke widget->onClick which should then do something on
					//			 it's parent pile
					dynamic_cast<Pile *>(GetBumpActor(sel->getPickedActor())->getParent())->onWidgetClick(sel->getPickedActor());
				}

				// Pin picked actor to wall
				if (GLOBAL(MouseOverWall))
				{
					// only invoke pin if there is nothing droppable
					if (!isDraggingOverDropObject)
						PressureLockEvent();
				}

				GLOBAL(widgetFirstMovePosX)=GLOBAL(widgetFirstMovePosY)=-1;
				GLOBAL(MouseOverWall)=false;

				if (!GLOBAL(settings).useRightClickMenuInvocation)
				{
					// Mouse Has not moved and its not a double click, Invoke Menu on this Item
					if (!GLOBAL(useSingleClickExpiration) && 
						!GLOBAL(disallowMenuInvocation) && sel->getPickedActor() && sel->getPickedActor()->isDynamic() && sel->getPickedActor() && 
						GLOBAL(sglClickPos).distanceSquared(Vec3(float(x), float(y), 0)) < GLOBAL(dblClickSize) * GLOBAL(dblClickSize)

						)
					{
						markingMenu->invoke(GLOBAL(sglClickPos));
						markingMenu->offsetFromMouse(Vec2(float(x), float(y), 0.0f));

						// record this mm invocation
						statsManager->getStats().bt.interaction.markingmenu.invokedByClick++;
					}

					GLOBAL(disallowMenuInvocation) = false;
				}

				//Minimize 
				BumpObject* obj = GetBumpObject(sel->getPickedActor());
				if (obj && obj->isBumpObjectType(BumpActor))
				{
					Actor *data = (Actor *) obj;
					if (data->isMaximized())
						data->onLaunch();
				}

				// Try to see which wall was selected, if any
				CalcMouseRaycastReport();

				// Single click
				if (!winOS->IsKeyDown(KeyControl))
				{
					vector<BumpObject *> selection = sel->getBumpObjects();
					{
						float s = GLOBAL(sglClickPos).distanceSquared(Vec3(float(x), float(y), 0));
						float t = GLOBAL(sglClickSize) * GLOBAL(sglClickSize);
						if ((selection.size() > 1) && (s <= t))
						{
							BumpObject * picked = sel->getPickedActor();
							if (picked)
							{
								sel->clear();
								sel->add(picked);
								sel->setPickedActor(picked);
							}
						}
					}
				}

				// Double click
				if (GLOBAL(dblClickTimer).elapsed() < GLOBAL(dblClickInterval) && 
					GLOBAL(clickDistance) <= GLOBAL(dblClickSize) * GLOBAL(dblClickSize)) //float Click
				{
					HandleDoubleClick(sel->getLastPickedActor(), sel->getPickedActor(), x, y);
					Renamer->setTimerActive(false);
				}

				GLOBAL(dblClickTimer).restart();

				if (lassoMenu->isVisible())
				{
					// undoManager->invalidate();

				}else //MOUSE UP drag OR float-click
				{

					if(GLOBAL(mode)==InPileShifting)
						GLOBAL(mode)=None;


					//its not close enough to any pile to be added.  Check for Tossing
					// NOTE: checking for gravity is to accomodate for the floating icons crash bug
					else if((GLOBAL(settings.enableTossing)) && (sel->getPickedActor()!=NULL && sel->getPickedActor()->getLinearVelocity().magnitudeSquared() >= GLOBAL(settings).MinStartTossVelocitySq
						&& !sel->getPickedActor()->readBodyFlag(NX_BF_DISABLE_GRAVITY)))
					{
						vector<BumpObject *> selected = sel->getBumpObjects();
						for (int sel_i = 0; sel_i < selected.size(); ++sel_i)
							GLOBAL(Tossing).push_back(selected[sel_i]);
					}

					sel->setPickedActor(NULL);
				}


			}
		}

	}


	if ((activeButton & MouseButtonRight) && state == GLUT_DOWN)
	{
		// If there is a focused WebActor, right-clicking outside should be unfocusing, else handle as normal
		if (!WebActor::zoomOutFocusedWebActor())
		{
			bool shouldClearSelection = true;

			FinishModeBasedOnSelection();
			pickAndSet(x, y);

			// this needs to be done after pickAndSet so that we get the
			// actual new picked actor
			BumpObject * selectionPickedActor = sel->getPickedActor();
			vector<BumpObject *> selBumpObjs = sel->getBumpObjects();
			for (int i = 0; i < selBumpObjs.size(); ++i)
			{
				if (selBumpObjs[i] == selectionPickedActor)
				{
					shouldClearSelection = false;
				}
			}

			if (shouldClearSelection)
			{
				BumpObject *obj = sel->getPickedActor();

				// clear the whole selection so that the menu shows for the 
				// picked actor
				sel->clear();
				sel->add(obj);
			}
			else
			{
				// don't clear the selection, but clear the picked actor so that
				// the selection is not continually updated
				sel->setPickedActor(NULL);
			}

			markingMenu->destroy();
			lassoMenu->reset();

			if (GLOBAL(settings).useRightClickMenuInvocation)
			{
				if (sel->getPickedActor())
				{
					BumpObject * obj = GetBumpObject(sel->getPickedActor());

					if (obj)
					{
						sel->add(obj);
					}

				}

				// if the ctrl key is down, then just invoke the native context menu directly
				if (winOS->IsKeyDown(KeyControl))
				{
					winOS->Render();

					// we need to let the marking menu prepare the bumptop menus that are to be 
					// prepended to the native menu
					Key_MoreOptions();
					markingMenu->destroy();
				}
				else
				{
					markingMenu->invoke(Vec2(float(x), float(y), 0));

					// record this mm invocation
					statsManager->getStats().bt.interaction.markingmenu.invokedByClick++;
				}
			}

			sel->setPickedActor(NULL);
		}
		GLOBAL(mbutton) |= MouseButtonRight;
	}

	if (state == GLUT_UP)
	{
		GLOBAL(mbutton) &= ~button;
	}else{
		GLOBAL(mbutton) |= button;
	}

	textManager->invalidate();

	// update the resultant mouse properties
	CalcMouseRaycastReport();
}

void MotionCallback(int x, int y)
{
	// we want to unpin the current actor once we pull a pinned actor too far
	Vec3 mouseDownPos = GLOBAL(sglClickPos);
	Vec3 curMousePos(float(x), float(y), 0.0f);
	float distanceFromMouseDown = mouseDownPos.distance(curMousePos);

	int dx = GLOBAL(mx) - x;
	int dy = GLOBAL(my) - y;
	Vec3 line_start, line_end;

	// Toggle rendering on for mouse movement callbacks
	if (GLOBAL(mbutton) & MouseButtonLeft)
	{
		// textManager->invalidate();
		GLOBAL(isInteraction) = true;

		if (sel->getSize() > 0)
		{
			// If we moved our selection far enough, dont allow a menu invocation when
			// we return to its position and mouse up.
			if (abs(GLOBAL(sglClickPos).x - x) > 5 || abs(GLOBAL(sglClickPos).y - y) > 5)
			{
				GLOBAL(disallowMenuInvocation) = true;
			}
		}
	}

	if (GLOBAL(touchGestureBrowseMode) && !cam->isAnimating())
	{
		if (GLOBAL(mbutton) & MouseButtonLeft)
		{
			// On Mouse Down
			GLOBAL(mousePointList).push_back(Vec3(float(x), float(y), 0));
		}else{

		}
	}

	GLOBAL(mx) = x;
	GLOBAL(my) = y;

	mouseManager->update();

	if(GLOBAL(mbutton) & MouseButtonLeft)
	{
		if (distanceFromMouseDown > GLOBAL(dblClickSize)) 
		{
			// break the pin on the selected object
			if (sel->getPickedActor())
				sel->getPickedActor()->breakPin();
		}

		GLOBAL(mouseMoving) = true;
		NxActorWrapper *moverActor = NULL;

		if (sel->getPickedActor() && GLOBAL(initialDragObjects).empty() &&
			distanceFromMouseDown > GLOBAL(sglClickSize))
		{
			// start the drag and save the current poses, so that we can revert to it if we drag out			
			vector<BumpObject *> selectedObjs = sel->getBumpObjects();
			for (int i = 0; i < selectedObjs.size(); ++i) 
			{
				BumpObject * obj = selectedObjs[i];
				assert(!selectedObjs[i]->isDragging());
				selectedObjs[i]->onDragBegin();
				GLOBAL(initialDragObjects).push_back(obj);

				//spread back rename function. TODO
				bubbleManager->collapseSpreadedCluster();
			}
		}

		// Choses the mode needed based on selection (This ignores clicks without movement)
		if (!lassoMenu->isVisible())
		{
			StartModeBasedOnSelection(x, y);
		}

		//Check/Do Widget Cross (if I've clicked an item in a Pile & Widgets Exist)
		Pile* p;
		BumpObject *obj = sel->getPickedActor();
		p = (Pile *) (obj ? obj->getPileParent() : NULL);
		/*if(mode==None && pickedActor!=NULL && InAPile(pickedActor, p)!=-1 && p->getPileState() == Stack)
		{
		// Forcefully call the Mover actor
		if (moverActor && GetActor(sel->getPickedActor()) && GetActor(sel->getPickedActor())->widgetType == NotAWidget)
		{
		pickedActor=ActiveWidget=moverActor;
		MoverMouseDown();
		pickedActor=ActiveWidget=moverActor = NULL;
		}
		}
		else */if(GLOBAL(mode)==None && sel->getPickedActor()!=NULL && p) //dragging on item in a laidout pile
		{
			if(p->getPileState()==LaidOut) //Default action for DRAGGING on item in a laid out pile (ie, shifting order)
			{
				Actor * actor = GetBumpActor(sel->getPickedActor());
				if (actor)
				{
					if (sel->getSize() == 0 || sel->getSize() == 1)
					{
						sel->clear();
						sel->add(actor);
					}

					GLOBAL(mode)=InPileShifting;
					GLOBAL(shiftTally)=0;
				}
			}
		}
		else if(GLOBAL(mode) == InPileGhosting)
		{
			// Process Ghost gridding in a pile
			if (!sel->getPickedActor())
			{
				GLOBAL(mode) = None;
			}
		}
		else if(GLOBAL(mode)==InPileShifting && !lassoMenu->isVisible())
		{
			// REFACTOR: Possibly check to see if the pickedActor is part of the selected Items in a pile?
			vector<NxActorWrapper *> selectedPileItems = sel->getLegacyActorList();
			GLOBAL(shiftTally) += dx;

			if (abs(GLOBAL(shiftTally)) > GLOBAL(settings).shiftOn && selectedPileItems.size()>0 && 
				sel->getPickedActor() && sel->getPickedActor()->getParent())
			{
				// Find out range of indicies
				vector<int> indicies;

				int shiftSpots = int(float(GLOBAL(shiftTally)) / float(GLOBAL(settings).shiftOn));
				int dir = shiftSpots > 0 ? -1 : 1;

				Pile *p = dynamic_cast<Pile*>(((Actor *) GetBumpActor(selectedPileItems[0]))->getParent());
				//if (laidOutPile->getPileState() == Grid) dir *= -1;

				// Make a vector of Indicies that represent the Actors to move
				if (p)
				{
					for (int i = selectedPileItems.size() - 1; i >= 0; i--)
					{
						int ind = p->isInPile(GetBumpObject(selectedPileItems[i]));

						if(ind > -1)
						{
							indicies.push_back(ind);
						}
					}
				}

				if (indicies.size())
				{
					// Sort the indicies
					sort(indicies.begin(), indicies.end(), sortIntegers());

					// Shift Selection around
					for (uint j = 0; j < abs(shiftSpots); j++)
					{
						if (dir > 0)
						{
							for (int i = int(indicies.size() - 1); i >= 0; i--)
							{
								if (!p->shiftItems(indicies[i], dir))
								{
									// End the movement because we cant move any more
									j = abs(shiftSpots);
									i = -1;
								}
							}
						}else{
							for (uint i = 0; i < indicies.size(); i++)
							{
								if (!p->shiftItems(indicies[i], dir))
								{
									// End the movement because we cant move any more
									j = abs(shiftSpots);
									i = indicies.size();
								}
							}
						}
					}

					p->animateItemsToRelativePos();
					p->updatePileItems(true);
				}

				GLOBAL(shiftTally) = 0;
			}
		}
	}else if (GLOBAL(mbutton) == 0)
	{
		vector<Pile *> piles = sel->getFullPiles();

		if (piles.size() == 1 && piles[0]->getPileState() == LayingOut)
		{
			if (!(piles[0])->fanoutTick(CurrentWorldMousePos()))
			{
				FinishModeBasedOnSelection();
			}else{
				piles[0]->updatePileItems(true);
			}

			rndrManager->invalidateRenderer();
		}
	}

	// update the resultant mouse properties
	CalcMouseRaycastReport();
}

void KeyboardCallback(uint key, int x, int y)
{
	bool wasDraggingFromPile = (GLOBAL(mode) == InPileGhosting);
	FinishModeBasedOnSelection();

	// Toggle rendering on for key presses
	GLOBAL(isInteraction) = true;

	// Key: [Shift][Ctrl][Alt][KeyVal] = 32Bits.
	unsigned char k = ((key << 24) >> 24);
	bool alt = (key & (KeyAlt << 8)) > 0 ? true : false;
	bool ctrl = (key & (KeyControl << 16)) > 0 ? true : false;
	bool shift = (key & (KeyShift << 24)) > 0 ? true : false;

	//get selected objects
	vector<BumpObject*> selBumpObjects = sel->getBumpObjects();

	keyManager->onKeyDown(KeyCombo(k, ctrl, shift, alt));

	// Clear the selection upon Escape key being hit
	if (key == KeyEscape)
	{
		if (isSlideshowModeActive())
		{
			Key_DisableSlideShow();
		}
		else if (wasDraggingFromPile)
		{
			if (sel->getPickedActor())
			{
				BumpObject * actor = sel->getPickedActor();
				if (actor->isBumpObjectType(BumpActor) && 
					((Actor *) actor)->getObjectToMimic())
				{
					BumpObject * mimic = ((Actor *) actor)->getObjectToMimic();
					if (mimic->isBumpObjectType(BumpActor))
					{
						actor->markDragCancelled(true);
						actor->setGlobalPosition(mimic->getGlobalPosition());
						actor->onDragEnd();
						sel->clear();
						sel->setPickedActor(NULL);
						sel->add(mimic);
					}
				}
			}
		}
		else if (Finder->isActive())
		{
			Finder->cancel();
		}
		else if (selBumpObjects.size()==1 &&
				 selBumpObjects.front()->isObjectType(ObjectType(BumpActor, Webpage)) &&
				 ((WebActor*)selBumpObjects.front())->isFocused())
		{
			// if a webactor has been zoomed into, zoom it out but maintain selection
			WebActor::zoomOutFocusedWebActor();
		}
		else
		{
			vector<Pile *> piles = sel->getFullPiles();
			bool hasSingleOpenPileSelected = (piles.size() == 1) && !piles[0]->isDragging() && (piles[0]->getPileState() != Stack);

			if (hasSingleOpenPileSelected)
			{
				// close the selected pile if it is open and not dragging
				Key_ClosePile();
			}
			else
			{
				// clear the selected bump objects and end the drag
			set<BumpObject *> parents;
			for (int i = 0; i < selBumpObjects.size(); ++i)
			{
					if (selBumpObjects[i]->isDragging())
					{
						selBumpObjects[i]->markDragCancelled(true);
						selBumpObjects[i]->onDragEnd();
					}

				if (selBumpObjects[i]->isParentType(BumpPile))
					parents.insert(selBumpObjects[i]->getParent());
			}
			sel->clear();

			if (!parents.empty())
			{
				// select the object's parents
				set<BumpObject *>::const_iterator iter = parents.begin();
				while (iter != parents.end())
				{	
					// close the parent pile if it's child is selected
					((Pile *) *iter)->close();					
					
					sel->add(*iter);
					iter++;
					}
				}
			}
		}
	}
}

void ArrowKeyCallback(int key, int x, int y)
{


	// clear marking menu
	markingMenu->destroy();

	// check if there is an image in the selection
	/*
	if (sel->getSize() > 0)
	{
	bool hasImagesInSelection = false;
	vector<BumpObject *> selectedObjects = sel->getBumpObjects();
	for (int i = 0; i < selectedObjects.size(); ++i)
	{
	if (selectedObjects[i]->getObjectType() & ObjectType(BumpActor, FileSystem, Image))
	{
	hasImagesInSelection = true;
	}
	}
	}
	*/

	// if we are currently not watching any actors and the up/left/right keys
	// are pressed, then push the current selection into the camera watch list
	vector<BumpObject *> selObjs = sel->getBumpObjects();
	Pile * selectedPile = NULL;
	bool hasPileSelected = false;

	if (selObjs.size() == 1)
	{
		if (selectedPile = dynamic_cast<Pile *>(selObjs.front()))
		{
			hasPileSelected = true;
		}
		else if (selObjs.front()->getParent())
		{
			selectedPile = dynamic_cast<Pile *>(selObjs.front()->getParent());
			if (selectedPile->getPileState() == Leaf ||
				selectedPile->getPileState() == Stack)
				hasPileSelected = true;
		}
	} 

	switch (key)
	{
	case KeyLeft:
		if (hasPileSelected)
		{
			if (GLOBAL(settings).freeOrProLevel == AL_PRO)
				selectedPile->leafDown();
		}
		else
		{
			// move to the next watched actor (or start the slideshow if none is started)
			if (!isSlideshowModeActive()) 
				Key_EnableSlideShow();
			else
				cam->highlightNextWatchedActor(false);
		}
		break;
	case KeyRight:
		if (hasPileSelected)
		{
			if (GLOBAL(settings).freeOrProLevel == AL_PRO)
				selectedPile->leafUp();
		}
		else
		{
			// move to the previous watched actor (or start the slideshow if none is started)
			if (!isSlideshowModeActive()) 
				Key_EnableSlideShow();
			else
				cam->highlightNextWatchedActor(true);
		}
		break;
	case KeyUp:
		if (hasPileSelected && GLOBAL(settings).freeOrProLevel == AL_PRO)
		{
			selectedPile->leafTo(0);
		}
		else
		{
			// start watching the selection, and highlight the first actor
			Key_ToggleSlideShow();
		}
		break;
	case KeyDown: // Down
		if (isSlideshowModeActive())
		{
			// clear marking menu
			markingMenu->destroy();

			// REFACTOR: broken
			/*if (antiGravBrowseMode)
			{
			antiGravBrowseMode = false;
			lasso.clear();

			selection.RestorePosesEtc();

			for (int i = 0; i < selection.pileItems.size(); i++)
			{
			selection.pileItems[i]->clearActorFlag(NX_AF_DISABLE_COLLISION);
			selection.pileItems[i]->clearBodyFlag(NX_BF_DISABLE_GRAVITY);
			selection.pileItems[i]->wakeUp();
			}

			selection.lassoLayoutState = Messy;
			}
			*/

			// disable the slideshow
			Key_DisableSlideShow();
		}
		else if (hasPileSelected && GLOBAL(settings).freeOrProLevel == AL_PRO)
		{
			selectedPile->leafToBottom();			
		}
		break;	
	default: break;
	}

}

#ifndef DXRENDER
void PrepareDrawDesktop(bool shadingOverlay)
{
	glPushAttrib(GL_ENABLE_BIT);
	glEnable(GL_BLEND);
	if(shadingOverlay) //blend on ao_texture, equivalent of Photoshop:Darken layer blending mode.  
	{
		glDisable(GL_DEPTH_TEST);
		// don't push these attribs, since we manually pop them later
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
		glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_COLOR);			
	}
}

/* -1 - Floor
* [0,4) - Walls [T,B,R,L]
*/
void DrawDesktopSide(int side, QString texture, const Vec3& position, const Vec3& size, const Vec3& color=Vec3(1.0f), const Vec3& uvInset=Vec3(0.0f))
{
	bool hasTexture = !texture.isEmpty();
	if (hasTexture)
	{
		unsigned int textureId = texMgr->getGLTextureId(texture);
		glBindTexture(GL_TEXTURE_2D,  textureId);
	}
	else
		glDisable(GL_TEXTURE_2D);

	glPushMatrix();
	glTranslatef(position.x, position.y, position.z);
	glColor4f(color.x, color.y, color.z, 1.0f);
	glBegin(GL_POLYGON);
	float halfWidth = size.x / 2.0f;
	float halfHeight = size.y / 2.0f;
	float topUVMargin = 0.025f;
	float topMargin = 2.0f;
	float fadeAlpha = 0.8f;

	switch (side)
	{
	case -1:	// floor
		glNormal3f(0,1,0);
		glTexCoord2f(0.0f+uvInset.x, 1.0f-uvInset.y); 	
		glVertex3f(halfWidth, 0, -halfHeight);
		glTexCoord2f(1.0f-uvInset.x, 1.0f-uvInset.y);
		glVertex3f(-halfWidth, 0, -halfHeight);
		glTexCoord2f(1.0f-uvInset.x, 0.0f+uvInset.y);  
		glVertex3f(-halfWidth, 0, halfHeight);
		glTexCoord2f(0.0f+uvInset.x, 0.0f + uvInset.y);  
		glVertex3f(halfWidth, 0, halfHeight);
		break;
	case 0:		// top wall
		glNormal3f(0,0,-1);
		glTexCoord2f(0.0f+uvInset.x, 0.0f + uvInset.y + topUVMargin);  
		glVertex3f(halfWidth, halfHeight - topMargin, 0);
		glTexCoord2f(0.0f+uvInset.x, 1.0f-uvInset.y); 	
		glVertex3f(halfWidth, -halfHeight, 0);
		glTexCoord2f(1.0f-uvInset.x, 1.0f-uvInset.y);
		glVertex3f(-halfWidth, -halfHeight, 0);
		glTexCoord2f(1.0f-uvInset.x, 0.0f+uvInset.y + topUVMargin);  
		glVertex3f(-halfWidth, halfHeight - topMargin, 0);

		glColor4f(0.0f, 0.0f, 0.0f, fadeAlpha);
		glTexCoord2f(1.0f-uvInset.x, 0.0f+uvInset.y);  
		glVertex3f(-halfWidth, halfHeight, 0);
		glTexCoord2f(0.0f+uvInset.x, 0.0f + uvInset.y);  
		glVertex3f(halfWidth, halfHeight, 0);
		break;
	case 1:		// bottom wall
		glNormal3f(0,0,1);
		glTexCoord2f(0.0f+uvInset.x, 0.0f + uvInset.y + topUVMargin);  
		glVertex3f(-halfWidth, halfHeight - topMargin, 0);
		glTexCoord2f(0.0f+uvInset.x, 1.0f-uvInset.y); 	
		glVertex3f(-halfWidth, -halfHeight, 0);
		glTexCoord2f(1.0f-uvInset.x, 1.0f-uvInset.y);
		glVertex3f(halfWidth, -halfHeight, 0);
		glTexCoord2f(1.0f-uvInset.x, 0.0f+uvInset.y + topUVMargin);  
		glVertex3f(halfWidth, halfHeight - topMargin, 0);

		glColor4f(0.0f, 0.0f, 0.0f, fadeAlpha);
		glTexCoord2f(1.0f-uvInset.x, 0.0f+uvInset.y);  
		glVertex3f(halfWidth, halfHeight, 0);
		glTexCoord2f(0.0f+uvInset.x, 0.0f + uvInset.y);  
		glVertex3f(-halfWidth, halfHeight, 0);
		break;
	case 2:		// right wall
		glNormal3f(1,0,0);
		glTexCoord2f(0.0f+uvInset.x, 0.0f + uvInset.y + topUVMargin);  
		glVertex3f(0, halfHeight - topMargin, halfWidth);
		glTexCoord2f(0.0f+uvInset.x, 1.0f-uvInset.y); 
		glVertex3f(0, -halfHeight, halfWidth);
		glTexCoord2f(1.0f-uvInset.x, 1.0f-uvInset.y);
		glVertex3f(0, -halfHeight, -halfWidth);
		glTexCoord2f(1.0f-uvInset.x, 0.0f+uvInset.y + topUVMargin);  
		glVertex3f(0, halfHeight - topMargin, -halfWidth);

		glColor4f(0.0f, 0.0f, 0.0f, fadeAlpha);
		glTexCoord2f(1.0f-uvInset.x, 0.0f+uvInset.y);  
		glVertex3f(0, halfHeight, -halfWidth);
		glTexCoord2f(0.0f+uvInset.x, 0.0f+uvInset.y);  
		glVertex3f(0, halfHeight, halfWidth);
		break;
	case 3:		// left wall
		glNormal3f(-1,0,0);

		glTexCoord2f(0.0f+uvInset.x, 0.0f+uvInset.y + topUVMargin);  
		glVertex3f(0, halfHeight - topMargin, -halfWidth);
		glTexCoord2f(0.0f+uvInset.x, 1.0f-uvInset.y); 	
		glVertex3f(0, -halfHeight, -halfWidth);
		glTexCoord2f(1.0f-uvInset.x, 1.0f-uvInset.y);
		glVertex3f(0, -halfHeight, halfWidth);
		glTexCoord2f(1.0f-uvInset.x, 0.0f+uvInset.y + topUVMargin);  
		glVertex3f(0, halfHeight - topMargin, halfWidth);

		glColor4f(0.0f, 0.0f, 0.0f, fadeAlpha);
		glTexCoord2f(1.0f-uvInset.x, 0.0f+uvInset.y);  
		glVertex3f(0, halfHeight, halfWidth);
		glTexCoord2f(0.0f+uvInset.x, 0.0f+uvInset.y);  
		glVertex3f(0, halfHeight, -halfWidth);
		break;
	default:
		assert(false);
		break;
	}
	glEnd();
	glPopMatrix();
}

void FinalizeDrawDesktop(bool shadingOverlay)
{
	if (shadingOverlay)
	{
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);		
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	glPopAttrib();
}
#endif

struct less_distance_to_cam_eye : public std::binary_function<NxActorWrapper*, NxActorWrapper*, bool>
{
	bool operator()(NxActorWrapper* x, NxActorWrapper* y)
	{
		Vec3 xToEye = x->getGlobalPosition() - cam->getEye();
		Vec3 yToEye = y->getGlobalPosition() - cam->getEye();
		return xToEye.magnitudeSquared() > yToEye.magnitudeSquared(); 
	}
};

//bool lastRenderInited=false;
//CHighTime lastRender;
bool RenderCallback()
{
	if(scnManager->disableRendering)
		return true;

#ifdef DXRENDER
	if (!dxr->tryDevice())
		return true;
#endif

	Stopwatch profilerTimer;

	if (GLOBAL(firstReshape))
	{
		// update the text labels once before the first render
		textManager->invalidate();
		vector<BumpObject *> objects = scnManager->getBumpObjects();
		for (int i = 0; i < objects.size(); ++i) {
			objects[i]->syncNameableOverlayToDims();
		}
		textManager->forceUpdate();
		GLOBAL(firstReshape) = false;
	}

	// ----------------------------------------------------------------------------------------------------
#ifndef DXRENDER
	if (GLOBAL(settings).PrintMode)
	{
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	}
	else
	{
		if (GLOBAL(isInSharingMode))
			glClearColor(0.12078f, 0.12078f, 0.12078f, 1.0f);
		else
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	}

	// Clear buffers, Setup Camera
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	switchToPerspective();

	profiler->incrementTime("RenderingSum>Misc", profilerTimer.restart());
#endif
	
	if (GLOBAL(firstTime))
	{
		// disable the splash screen
		winOS->SplashScreenIntro(false);
		consoleWrite(QString("Time to first render: %1s\n").arg(GLOBAL(loadingTimer).elapsed()));
		GLOBAL(firstTime) = false;		
	}

#ifdef DXRENDER
	dxr->updateCamera(cam->getEye(), cam->getDir(), cam->getUp());

	HRESULT hr = S_OK;
	hr = dxr->device->BeginScene();
	VASSERT(D3D_OK == hr, QString_NT("BeginScene failed with hr = %1").arg(hr));
	
	dxr->device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER , 0, 1, 0);
	
	dxr->switchToPerspective();

	IDirect3DTexture9 * baseTextures[] = {texMgr->getGLTextureId("floor.desktop"), texMgr->getGLTextureId("wall.top"),
		texMgr->getGLTextureId("wall.bottom"), texMgr->getGLTextureId("wall.right"), texMgr->getGLTextureId("wall.left")};
	IDirect3DTexture9 * overlayTextures[] = {texMgr->getGLTextureId("floor.ao_overlay"), texMgr->getGLTextureId("wall.tb.ao_overlay"), 
		texMgr->getGLTextureId("wall.tb.ao_overlay"), texMgr->getGLTextureId("wall.lr.ao_overlay"), texMgr->getGLTextureId("wall.lr.ao_overlay")};
			
	dxr->renderDesktop(baseTextures, overlayTextures);

#endif
	profiler->incrementTime("RenderingSum>Slideshow", profilerTimer.restart());

#ifndef DXRENDER
	if (!GLOBAL(settings).PrintMode)
	{
		float iconMargin = 40.0f;
		float imageScale = 36.0f;
		SharedDesktop * desktop = NULL;
		float interDesktopMargin = 45.0f;
		if (GLOBAL(sharedDesktops).empty())
		{
			// center desktop
			desktop = new SharedDesktop;
			desktop->offset = Vec3(0, 0, 0);
			desktop->name = new NameableOverlay(desktop);
			desktop->setText("My Desk");
			desktop->hideText(true);
			desktop->getNameableOverlay()->getTextOverlay()->setFont(FontDescription("Arial Bold", 20));				
			desktop->getNameableOverlay()->getTextOverlay()->getTextBuffer().setMaxBounds(QSize(256, 0));
			desktop->getNameableOverlay()->getTextOverlay()->getTextBuffer().popFlag(TextPixmapBuffer::Truncated);
			desktop->getNameableOverlay()->getTextOverlay()->getTextBuffer().update();
			desktop->getNameableOverlay()->getTextOverlay()->getStyle().setPadding(LeftEdge, iconMargin * 1.2f);				
			desktop->getNameableOverlay()->getTextOverlay()->getStyle().setPadding(BottomEdge, iconMargin / 4.5f);
			desktop->image = new ImageOverlay("user.me");
			desktop->image->getStyle().setScaledDimensions(Vec3(imageScale, imageScale, 0));
			desktop->image->getStyle().setVisible(true);
			GLOBAL(sharedDesktops).push_back(desktop);

			// left desktop
			desktop = new SharedDesktop;
			desktop->offset = Vec3((2.25f * GLOBAL(WallsPos)[3].x) + interDesktopMargin, 0, 0);
			desktop->bgTextureOverride = "user.bg.vista";
			desktop->name = new NameableOverlay(desktop);
			desktop->setText("David");
			desktop->hideText(true);
			desktop->getNameableOverlay()->getTextOverlay()->setFont(FontDescription("Arial Bold", 20));					
			desktop->getNameableOverlay()->getTextOverlay()->getTextBuffer().setMaxBounds(QSize(128, 0));
			desktop->getNameableOverlay()->getTextOverlay()->getTextBuffer().popFlag(TextPixmapBuffer::Truncated);
			desktop->getNameableOverlay()->getTextOverlay()->getTextBuffer().update();
			desktop->getNameableOverlay()->getTextOverlay()->getStyle().setPadding(LeftEdge, iconMargin * 1.2f);				
			desktop->getNameableOverlay()->getTextOverlay()->getStyle().setPadding(BottomEdge, iconMargin / 4.5f);
			desktop->image = new ImageOverlay("user.left");
			desktop->image->getStyle().setScaledDimensions(Vec3(imageScale, imageScale, 0));
			desktop->image->getStyle().setVisible(true);
			GLOBAL(sharedDesktops).push_back(desktop);

			// right desktop
			desktop = new SharedDesktop;
			desktop->offset = Vec3((2.25f * GLOBAL(WallsPos)[2].x) - interDesktopMargin, 0, 0);
			desktop->name = new NameableOverlay(desktop);
			desktop->setText("My Picture Frame");
			desktop->hideText(true);
			desktop->getNameableOverlay()->getTextOverlay()->setFont(FontDescription("Arial Bold", 20));			
			desktop->getNameableOverlay()->getTextOverlay()->getTextBuffer().setMaxBounds(QSize(128, 0));	
			desktop->getNameableOverlay()->getTextOverlay()->getTextBuffer().popFlag(TextPixmapBuffer::Truncated);
			desktop->getNameableOverlay()->getTextOverlay()->getTextBuffer().update();	
			desktop->getNameableOverlay()->getTextOverlay()->getStyle().setPadding(LeftEdge, iconMargin * 1.2f);				
			desktop->getNameableOverlay()->getTextOverlay()->getStyle().setPadding(BottomEdge, iconMargin / 4.5f);
			desktop->image = NULL;
			GLOBAL(sharedDesktops).push_back(desktop);

			// top desktop
			desktop = new SharedDesktop;
			desktop->offset = Vec3(0, 0, (2.25f * GLOBAL(WallsPos)[0].z) + 2.0f * interDesktopMargin);
			desktop->bgTextureOverride = "user.bg.vista_2";
			desktop->name = new NameableOverlay(desktop);
			desktop->setText("Dad");
			desktop->hideText(true);
			desktop->getNameableOverlay()->getTextOverlay()->setFont(FontDescription("Arial Bold", 20));			
			desktop->getNameableOverlay()->getTextOverlay()->getTextBuffer().setMaxBounds(QSize(128, 0));	
			desktop->getNameableOverlay()->getTextOverlay()->getTextBuffer().popFlag(TextPixmapBuffer::Truncated);
			desktop->getNameableOverlay()->getTextOverlay()->getTextBuffer().update();	
			desktop->getNameableOverlay()->getTextOverlay()->getStyle().setPadding(LeftEdge, iconMargin * 1.2f);				
			desktop->getNameableOverlay()->getTextOverlay()->getStyle().setPadding(BottomEdge, iconMargin / 4.5f);
			desktop->image = new ImageOverlay("user.top");
			desktop->image->getStyle().setScaledDimensions(Vec3(imageScale, imageScale, 0));
			desktop->image->getStyle().setVisible(true);
			GLOBAL(sharedDesktops).push_back(desktop);
		}

		if (GLOBAL(isInSharingMode))
		{
			// update the shared desktop name positions
			float desktopNameMargin = 50.0f;
			for (int i = 0; i < GLOBAL(sharedDesktops).size(); ++i)
			{
				SharedDesktop * desktop = GLOBAL(sharedDesktops)[i];
				float offsetZ = GLOBAL(WallsPos)[1].z - desktopNameMargin;
				Vec3 offset = desktop->offset;
				offset.z += offsetZ;
				if (i == 3)	// top desktop
					offset.z -= 15.0f;

				QSize textBounds = desktop->getNameableOverlay()->getTextOverlay()->getTextBuffer().getActualSize();
				Vec3 worldPos = WorldToClient(offset, true);
				worldPos.y = winOS->GetWindowHeight() - worldPos.y;
				worldPos.x -= (textBounds.width() / 2.0f) + iconMargin;

				desktop->getNameableOverlay()->getStyle().setOffset(worldPos);
			}
		}

		uint numDesktops = GLOBAL(isInSharingMode) ? GLOBAL(sharedDesktops).size() : 1;
		for (int desktop = 0; desktop < numDesktops; ++desktop)
		{
			glLoadIdentity();
			SharedDesktop * sharedDesktop = NULL;
			if (GLOBAL(isInSharingMode))
			{
				sharedDesktop = GLOBAL(sharedDesktops)[desktop];
				glTranslatef(sharedDesktop->offset.x, sharedDesktop->offset.y, sharedDesktop->offset.z);		
				if (desktop == 2)
				{
					glTranslatef(-30.0f, 0, 0);
					glScalef(0.7f, 0.7f, 0.7f);

					Quat result;
					result.id();
					result.multiply(Quat(-20, Vec3(0, 1, 0)), Quat(30, Vec3(-1, 0, 0)));
					Mat33 ori(result);

					// apply transform
					float glmat[16] = {0};	//4x4 column major matrix for OpenGL.
					ori.getColumnMajorStride4(&(glmat[0]));
					glmat[15] = 1.0f;
					glMultMatrixf(&(glmat[0]));
				}
				else if (desktop > 0 && desktop < 3)
					glScalef(0.9f, 0.9f, 0.9f);
			}

			//      ^  y+
			//      |			BumpTop's World Coordinate System.  Where X & Z are a plane that makes up the floor and Y is normal/up of that plane
			//      |   
			//      | / z+
			//      |/ 
			//<-----
			//x+
			//drawAxis(8.0f, true);
			vector<NxActorWrapper*> walls = GLOBAL(Walls);
			Vec3 mainWorkspaceDims(abs(GLOBAL(WallsPos)[3].x - GLOBAL(WallsPos)[2].x) - GLOBAL(Walls)[2]->getDims().z,
				abs(GLOBAL(WallsPos)[0].z - GLOBAL(WallsPos)[1].z) - GLOBAL(Walls)[1]->getDims().z,
				0);
			const char * baseTextures[] = {"floor.desktop", "wall.top", "wall.bottom", "wall.right", "wall.left"};
			const char * overlayTextures[] = {"floor.ao_overlay", "wall.tb.ao_overlay", "wall.tb.ao_overlay", "wall.lr.ao_overlay", "wall.lr.ao_overlay"};
			bool wallHasTexture[4] = {0};

			// draw the desktop
			for (int iteration = 0; iteration < 2; ++iteration)
			{
				const char ** textures = (iteration == 0) ? baseTextures : overlayTextures;
				PrepareDrawDesktop((iteration > 0));
				// draw the floor
				if(!GLOBAL(DrawWalls))
				{
					// infinite desktop mode
					DrawDesktopSide(-1, "floor.infinite", Vec3(0.0f), 7.0f * mainWorkspaceDims);
					FinalizeDrawDesktop((iteration > 0));
					break;
				}
				else 
				{
					if (GLOBAL(isInSharingMode))
					{
						if (desktop == 2)
						{
							DrawDesktopSide(-1, "user.overlay.toshibaFrame", Vec3(0.0f, 0.0f, 0.0f), mainWorkspaceDims + Vec3(2.0f));
							glEnable(GL_DEPTH_TEST);
							if (!sharedDesktop->bgTextureOverride.isEmpty())
								DrawDesktopSide(-1, sharedDesktop->bgTextureOverride, Vec3(0.0f, 0.1f, 0.0f), (0.75f * mainWorkspaceDims) + Vec3(2.0f));
							else
								DrawDesktopSide(-1, textures[0], Vec3(0.0f, 0.1f, 0.0f), (0.75f * mainWorkspaceDims) + Vec3(2.0f));
							glDisable(GL_DEPTH_TEST);
						}
						else if (!sharedDesktop->bgTextureOverride.isEmpty())
							DrawDesktopSide(-1, sharedDesktop->bgTextureOverride, Vec3(0.0f), mainWorkspaceDims + Vec3(2.0f));
						else
							DrawDesktopSide(-1, textures[0], Vec3(0.0f), mainWorkspaceDims + Vec3(2.0f));
					}
					else
					{
						if (iteration == 0)
						{
							if (texMgr->isTextureState("floor.desktop", TextureLoaded))
							{
								DrawDesktopSide(-1, "floor.desktop", Vec3(0.0f), mainWorkspaceDims + Vec3(2.0f));
							}
							else if (texMgr->isTextureState("floor.desktop.buffer", TextureLoaded))
							{
								DrawDesktopSide(-1, "floor.desktop.buffer", Vec3(0.0f), mainWorkspaceDims + Vec3(2.0f));
							}
						}
						else
							DrawDesktopSide(-1, textures[0], Vec3(0.0f), mainWorkspaceDims + Vec3(2.0f));
					}
				}
				FinalizeDrawDesktop((iteration > 0));
			}
			profiler->incrementTime("RenderingSum>Background", profilerTimer.restart());

			// draw the walls
			if (GLOBAL(DrawWalls) && !GLOBAL(isInSharingMode))
			{
				for (int iteration = 0; iteration < 2; ++iteration)
				{
					// ensure that we actually want the overlays
					if (iteration > 0 && !themeManager->getValueAsBool("textures.wall.allowOverlay",true))
						break;
					/*
					if (iteration > 0 && desktop > 0 && GLOBAL(isInSharingMode))
					break;
					*/

					const char ** textures = (iteration == 0) ? baseTextures : overlayTextures;
					PrepareDrawDesktop((iteration > 0));

					bool wallsAreLoaded = true;
					for (int i = 0; wallsAreLoaded && i < walls.size(); ++i)
						wallsAreLoaded = texMgr->isTextureState(baseTextures[i+1], TextureLoaded);

					// draw the walls
					for (int i = 0; i < walls.size(); ++i)
					{
						Vec3 dims = walls[i]->getDims() * 2.0f;
						Vec3 pos = GLOBAL(Walls)[i]->getGlobalPosition();
						float posYOffset = abs((GLOBAL(Walls)[i])->getBoundingBox().getMin().y);
						float posZOffset = (dims.z / 2.0f);
						pos.y += posYOffset;
						if (i == 0) pos.z -= posZOffset;
						else if (i == 1) pos.z += posZOffset;
						else if (i == 2) pos.x += posZOffset;
						else if (i == 3) pos.x -= posZOffset;
						if (iteration == 0 && !wallsAreLoaded /* && (!wallsAreLoaded || (desktop > 0 && GLOBAL(isInSharingMode)))*/ )
						{
							// just render a white quad
							DrawDesktopSide(i, QString(), pos, dims, Vec3(0.5f));
						}
						else
						{
							DrawDesktopSide(i, textures[i+1], pos, dims, Vec3(1.0f), Vec3(0.01f));
						}
					}

					FinalizeDrawDesktop((iteration > 0));
				}
			}


			// draw the names
			if (GLOBAL(isInSharingMode))
			{
				switchToOrtho();
				glPushMatrix();					
				const Vec3& pos = sharedDesktop->getNameableOverlay()->getStyle().getOffset();
				const Vec3& dims = sharedDesktop->getNameableOverlay()->getSize();
				glTranslatef(pos.x, pos.y, 0.0f);
				// offset and render the image
				if (sharedDesktop->image) 
				{
					const Vec3& imageDims = sharedDesktop->image->getStyle().getScaledDimensions();
					glPushMatrix();
					sharedDesktop->image->getPreferredDimensions();	// this forces an update of the internal dims
					sharedDesktop->image->onRender();
					glPopMatrix();
				}
				// render the name
				glTranslatef(0, -dims.y/2, 0);
				sharedDesktop->getNameableOverlay()->getTextOverlay()->onRender();
				glPopMatrix();
				switchToPerspective();
			}

			profiler->incrementTime("RenderingSum>Cage and interior walls", profilerTimer.restart());
		}
		glLoadIdentity();
	}

	// render the lasso under the actors
	vector<Pile *> scnPiles = scnManager->getPiles();
	for (uint i = 0; i < scnPiles.size(); i++)
	{
		if (scnPiles[i]->getPileState() == LaidOut ||
			scnPiles[i]->getPileState() == LayingOut)
		{
			RenderLine(scnPiles[i]->getFanoutLasso(), false, 0);
		}
	}
	profiler->incrementTime("RenderingSum>Lasso", profilerTimer.restart());
#endif

	// call the render manager to render all actors
	rndrManager->onRender();

	// Draw InPileShifting Separator
	if (GLOBAL(mode) == InPileGhosting)
	{
		int dummyIndex = 0;
		Pile * dummyPile = NULL;
		processInPileGhosting(true, dummyIndex, &dummyPile);
	}

	profiler->incrementTime("RenderingSum>InPileShifting separator", profilerTimer.restart());

#ifdef DXRENDER
	dxr->switchToOrtho();
	dxr->billboardMaterial.Diffuse = dxr->billboardMaterial.Ambient = D3DXCOLOR(0xffffffff);
	dxr->device->SetMaterial(&dxr->billboardMaterial);
	dxr->beginRenderBillboard();
#else
	switchToOrtho();
#endif
	// render the actor names
	vector<BumpObject *> objs = scnManager->getBumpObjects();
	for (int i = 0; i < objs.size(); ++i)
	{
		if (objs[i]->shouldRenderText() && !objs[i]->isSelected())
			((Nameable *)objs[i])->onRenderText();
	}
	// render the selected actor's names on top
#ifndef DXRENDER
	glDisable(GL_DEPTH_TEST);
#endif
	vector<BumpObject *> selObjs = sel->getBumpObjects();
	for (int i = 0; i < selObjs.size(); ++i)
	{
		if (selObjs[i]->shouldRenderText())
			((Nameable *)selObjs[i])->onRenderText();
	}

	profiler->incrementTime("RenderingSum>Actors", profilerTimer.restart());

	// render the overlays
	scnManager->renderOverlays(RenderSkipModelViewChange);

	// Render Multi-touch overlays
	if (winOS->GetWindows7Multitouch())
		winOS->GetWindows7Multitouch()->onRender();
	profiler->incrementTime("RenderingSum>MultitouchOverlay separator", profilerTimer.restart());
#ifdef DXRENDER
	dxr->endRenderBillboard();
#endif
	profiler->incrementTime("RenderingSum>Overlays & Text", profilerTimer.restart());

	// render the menus & lasso
	lassoMenu->onRender(RenderSkipModelViewChange);
	markingMenu->onRender(RenderSkipModelViewChange);

	profiler->incrementTime("RenderingSum>Menus", profilerTimer.restart());

#ifdef DXRENDER
	dxr->switchToPerspective();
#else
	switchToPerspective();
#endif

	// misc
	lassoMenu->getStackIcon().onRender();
	multiMice->onRender();
	
	profiler->incrementTime("RenderingSum>Misc", profilerTimer.restart());

	// FPS stuffs
	int prevmaxFramesPerSecond = scnManager->maxFramesPerSecond;
	static Stopwatch fpsRateTimer;
	static int fpsRatingState = 0;
	static bool shouldUseCPUOptimizations = GLOBAL(settings).useCPUOptimizations;
	float fpsLowerThreshold = 30.0f;

	if (fpsRatingState == 0)
	{
		bool hasUserAAOverride = false;
		winOS->getRegistryDwordValue("DisableAntiAliasing", hasUserAAOverride);
		if (!hasUserAAOverride)
		{
			fpsRateTimer.restart();
			fpsRatingState = 1;
			GLOBAL(settings).useCPUOptimizations = false;
		}
		else
		{
			fpsRatingState = 2;
		}
	}
	else if (fpsRatingState == 1)
	{
		if (fpsRateTimer.elapsed() > 2000)
		{
			fpsRatingState = 2;
			GLOBAL(settings).useCPUOptimizations = shouldUseCPUOptimizations;

			// check the max fps calculated over that time, and if its < the threshold, then 
			// disable multisampling
			// NOTE: this is a lower bound threshold for a normal bt experience
			if (scnManager->maxFramesPerSecond < fpsLowerThreshold) 
			{
				// disable things in order to try and improve the frame rate
				if (rndrManager->isMultisamplingEnabled())
					rndrManager->setMultisamplingEnabled(false);
			}
		}
	}

	if (GLOBAL(Elapsed).elapsed() >= 1.0)
	{
		double elapsedTime = GLOBAL(Elapsed).elapsed();
		// Calculate average frames per second
		GLOBAL(frameCounter)++;
		GLOBAL(framesPerSecond) = (int)(GLOBAL(framesPerSecondCounter) / elapsedTime);
		GLOBAL(avgFramesPerSecond) += (int)(GLOBAL(framesPerSecondCounter) / elapsedTime);
		scnManager->maxFramesPerSecond = NxMath::max(scnManager->maxFramesPerSecond, GLOBAL(framesPerSecond));
		GLOBAL(framesPerSecondCounter) = 0;
		GLOBAL(Elapsed).restart();
	}

#ifdef DXRENDER
	dxr->switchToOrtho();
#endif
	if (GLOBAL(settings).drawFramesPerSecond)
	{
#ifndef DXRENDER
		// FPS drawing code
		glPushAttribToken token2(GL_ENABLE_BIT);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glEnable(GL_TEXTURE_2D);

		// Switch to Ortho Mode
		switchToOrtho();		
#endif
		float leftMargin = 10; //distance from left hand side of screen to start drawing
		float topMargin = 20;
		float yJump = 20;
		TextPixmapBuffer* hudTextBuffer = &scnManager->hudTextBuffer;
		static bool updated = false;
		static Stopwatch hudUpdate;
		if (hudUpdate.elapsed() > 750)
		{
			hudUpdate.restart();
			
			QString str;
			QTextStream s(&str);
			static int barCounter;
			s << QString(++barCounter % 30, '|') << endl;
			s << "Frames Per Second: " << GLOBAL(framesPerSecond) << endl;
			s << "Average FPS:       " << GLOBAL(avgFramesPerSecond) / GLOBAL(frameCounter) << endl;
			s << "Max Average FPS:   " << scnManager->maxFramesPerSecond << endl;
			s << endl; //new line
			s << "Bump Objects:      " << scnManager->getBumpObjects().size() << endl;
			s << "Actors:            " << scnManager->getFileSystemActors().size() << endl;
			s << "Piles:             " << scnManager->getPiles().size() << endl;
			s << endl;
			if(animManager->getNumObjsAnimating() > 0)
				s << "Animating Objs:      " << animManager->getNumObjsAnimating() << endl;
			if(evtManager->RenderReason() != "")
				s << "Render Reason:      "  << evtManager->RenderReason() << endl;
			if (rndrManager->isMultisamplingEnabled())
				s << "Anti-Aliasing:      Enabled";

			// update the text			
			if (!updated)
			{
				FontDescription font("Arial", 14);

				hudTextBuffer->popFlag(TextPixmapBuffer::RenderShadow);
				hudTextBuffer->pushFlag(TextPixmapBuffer::RenderFastShadow);
				hudTextBuffer->setTextAlignment(Qt::AlignLeft);
				hudTextBuffer->setMaxBounds(QSize(winOS->GetWindowWidth(), 0));
				hudTextBuffer->setFont(fontManager->getFont(font));
				updated = true;
			}
			hudTextBuffer->setText(str);
			hudTextBuffer->update();			
		}

		// render the text
		if (updated)
		{
			Vec3 maxUVs;
			hudTextBuffer->bindAsGLTexture(maxUVs);

			const QSize& textSize = hudTextBuffer->getActualSize();
						
#ifdef DXRENDER
			dxr->beginRenderBillboard();
			dxr->renderBillboard(Vec3(0.0f, winOS->GetWindowHeight() - textSize.height(), 0),
								 Vec3(textSize.width(), textSize.height(), 0),
								 D3DXCOLOR(0xffffffff),
								 Vec3(0.0f),
								 maxUVs);
			dxr->endRenderBillboard();
#else
			const int xOffset = 0;
			const int yOffset = 0;
			glPushMatrix();
			glColor3f(1.0, 1.0, 1.0f);
			glTranslatef(0, winOS->GetWindowHeight() - (textSize.height()), 0);
			glBegin(GL_QUADS);
			glTexCoord2f(1,0); 	glVertex2f(textSize.width() + xOffset,textSize.height() + yOffset);
			glTexCoord2f(0,0); 	glVertex2f(xOffset,textSize.height() + yOffset);
			glTexCoord2f(0,1); 	glVertex2f(xOffset,yOffset);
			glTexCoord2f(1,1);	glVertex2f(textSize.width() + xOffset,yOffset);
			glEnd();
			glPopMatrix();
#endif			
		}
	}
	// Increment frames per second
	GLOBAL(framesPerSecondCounter)++;

	profiler->incrementTime("RenderingSum>FPS", profilerTimer.restart());

	GLOBAL(isInteraction) = false;
	profiler->incrementTime("RenderingSum>Misc", profilerTimer.restart());

#ifdef DXRENDER
	// The following is a dirty hack in order to resolve alpha blending issues that appear on certain video cards
	// Namely: Radeon 4550
	//		   Intel 4 series
	// ---------------------------------------------START OF WORKAROUND--------------------------------------------------------------------------------------------
	dxr->device->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA);
	
	dxr->device->SetTransform(D3DTS_WORLD, &dxr->identity);
	dxr->device->SetTexture(0, dxr->nullTexture);
	dxr->beginRenderBillboard();
	dxr->renderBillboard(Vec3(0.0f), Vec3(winOS->GetWindowWidth(), winOS->GetWindowHeight(), 0.0f), D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f));
	dxr->endRenderBillboard();
	
	dxr->device->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA | D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
	// -----------------------------------------------------------------------------------------------------------------------------------------------------------
	
	hr = dxr->device->EndScene();
	_ASSERT(D3D_OK == hr);
	dxr->Present();
	
#endif
	return true;
}

void prePhysicsTimerCallback()
{
	// we hit this function a lot, so to save on the calls, we will locally use the scenemanager
	SceneManager * sceneManager = scnManager;

	// pre-physics updates
	LOG_STATEMENT(texMgr->onTimer());
	LOG_STATEMENT(animManager->update());
#ifdef ENABLE_SLIDESHOW_CLASS
	LOG_STATEMENT(slideShow->update(value));
#endif
	LOG_STATEMENT(lassoMenu->update());
	LOG_STATEMENT(markingMenu->update());
	LOG_STATEMENT(fsManager->update());
	LOG_STATEMENT(repoManager->update());
	LOG_STATEMENT(Singleton<TimerManager>::getInstance()->onUpdate());
	if (scnManager->getReplayable())
	{	
		Replayable::State playState;
		LOG_STATEMENT(playState = scnManager->getReplayable()->update());
		if(playState == Replayable::Stopped)
			scnManager->setReplayable(NULL);
	}
	if (scnManager->getTrainingIntroRunner())
	{
		TrainingIntroRunner *t = scnManager->getTrainingIntroRunner();
		t->update();
	}

	LOG_STATEMENT(sel->update());
	LOG_STATEMENT(mouseManager->update());

	LOG_STATEMENT(bubbleManager->update());
}

void physicsTimerCallback(int value)
{	
	// physics updates
	float timeElapsed = NxMath::max(0.0f, value / 1000.0f);
	if(scnManager->gScene && !scnManager->gPause)
		LOG_STATEMENT(doNovodexTick(timeElapsed));
}

void postPhysicsTimerCallback(int value)
{	
	// we hit this function a lot, so to save on the calls, we will locally use the scenemanager
	SceneManager * sceneManager = scnManager;
	float timeElapsed = value > 0 ? (float) value / 1000 : 0.0f;	

	// post-physics updates
	LOG_STATEMENT(cam->update(timeElapsed));

	// Update pile movement based on phantom actor
	LOG_START("Update pile movement");
	vector<Pile *> piles = sceneManager->getPiles();
	for (uint i = 0; i < piles.size(); i++)
	{
		piles[i]->updatePileItems();
	}
	LOG_FINISH("Update pile movement");

	if (sel->getPickedActor())
	{
		BumpObject *obj = GetBumpObject(sel->getPickedActor());
		if (obj) obj->onDragMove();
	}

	// 30 Degree cap on Bumptop objects
	// If the rotation limit degree is less than the maxRotationLimitDegree
	// then we will straighten the icon. Otherwise we skip the straighten logic
	int maxRotationLimitDegree = 180;
	if (sceneManager->settings.RotationLimitDegrees < maxRotationLimitDegree)
	{
		const vector<BumpObject *>& objs = sceneManager->getBumpObjects();
		unsigned int size = objs.size();

		for (uint i = 0; i < size; i++)
		{
			BumpObject *obj = objs[i];

			if (obj->isPinned()) continue;
			if (obj->isFrozen()) continue;
			if (obj->isAnimating()) continue;
			if (obj->isParentType(BumpPile)) continue;
			if (obj->isSelected() && sceneManager->MouseOverWall) continue;

			// Grab their rotational values
			const Mat33 & mat = obj->getGlobalPose().M;

			float z;
			ToEulerAngleZ(mat, z);
			z *= 180 / PI;

			float epsilon = 1.0f;
			if (abs(z) > (sceneManager->settings.RotationLimitDegrees + epsilon))
			{
				// If its more then 30°, right them.
				z = sceneManager->settings.RotationLimitDegrees * (abs(z) / z);
				Quat a(z, Vec3(0, 0, -1));
				Quat r(90, Vec3(1, 0 ,0));
				r *= a;

				// Apply the quaternion to the object
				obj->setGlobalOrientationQuat(r);
				obj->setAngularVelocity(Vec3(0.0f));
			}
		}
	}

	if (sceneManager->settings.camWallsEnabled)
	{
		// Camera planes need to be updated after a move
		LOG_STATEMENT(UpdateCameraPlanes());
	}

	if (sceneManager->exitBumpTopFlag && !animManager->isAnimating())
	{
		// Exit bumptop at next Message Loop interval
		winOS->ExitBumpTop();
	}

	if(GLOBAL(settings.enableTossing))
	{
		LOG_STATEMENT(processTossing());
	}

	const vector<BumpObject *>& objects = sceneManager->getBumpObjects();
	unsigned int size = objects.size();

	//check for any object changes requiring rendering
	if (!rndrManager->isRenderRequired())
	{
#ifdef DXRENDER
		if (dxr->shouldResetDevice())
		{
			rndrManager->invalidateRenderer();
		}
		else
#endif
		{
			for (int i = 0; i < size; ++i)
			{
				if (objects[i]->isRequiringRender() != 0)
				{
					rndrManager->invalidateRenderer();
					break;
				}
			}
		}
	}

	//Remove Broken Joints 
	for (int i = 0; i < size; ++i)
	{
		NxJoint * joint = objects[i]->getPinJoint();
		if (joint && (joint->getState() == NX_JS_BROKEN))
		{
			//printf("releasing broken joint, %d", GLOBAL(PinJoints).size());
			sceneManager->gScene->releaseJoint(*joint);
			objects[i]->setPinJoint(NULL);
			joint = NULL;
			//printf(", %d", GLOBAL(PinJoints).size());
		}		
		if (!joint && objects[i]->isPinned())
			objects[i]->breakPin();
	}

	// XXX: HACKY way to catch ONLY single click events
	if (sceneManager->useSingleClickExpiration)
	{
		// check if the interval has expired, and if so, do the work, and 
		// disable single click expiration checking
		if (sceneManager->singleClickExpirationTimer.elapsed() > sceneManager->dblClickInterval)
		{
			sceneManager->useSingleClickExpiration = false;

			if((sceneManager->touchGestureBrowseMode)&&(!isSlideshowModeActive())) { //Delayed single click
				ArrowKeyCallback(KeyUp,0,0);
			}
		}
	}

	// update the overlays
	vector<OverlayLayout *> overlays = sceneManager->getOverlays();
	for (int i = 0; i < overlays.size(); ++i)
	{
		overlays[i]->onTimer(TimerOverlayEvent(overlays[i], value));
	}

	const vector<BumpObject *>& objs = sceneManager->getBumpObjects();
	for (int i = 0; i < objs.size(); ++i)
	{
		if (objs[i]->isBumpObjectType(BumpActor))
		{
			Actor *actor = (Actor *) objs[i];

			// Check if any Temporary actors are lingering around
			// Check for useless Temporary Actors
			if (actor && actor->isActorType(Temporary) &&
				!actor->isActorType(Invisible) &&
				!actor->isAnimating() &&
				!sel->getPickedActor())
			{
				repoManager->removeFromPileSpace(actor);
				SAFE_DELETE(actor);

				// Force a render after delete
				rndrManager->invalidateRenderer();
			}

			// update any animated texture'd actors as well
			if (actor)
			{
				if (actor->isActorType(FileSystem))
				{
					FileSystemActor * fsActor = (FileSystemActor *) actor;
					if (fsActor->hasAnimatedTexture())
						fsActor->updateAnimatedTexture();
#ifdef ENABLE_WEBKIT
					if (fsActor->isFileSystemType(WebThumbnail))
					{
						WebThumbnailActor * webActor = (WebThumbnailActor *) actor;
						webActor->onUpdate();
					}
#endif
				}
				else if (actor->isActorType(Webpage))
				{
#ifdef ENABLE_WEBKIT
					WebActor * webActor = (WebActor *) actor;
					webActor->onUpdate();
#endif
				}
			}
		}
	}

	// update any photo frames
	vector<FileSystemActor *> photoFrames = scnManager->getFileSystemActors(PhotoFrame);
	for (int i = 0; i < photoFrames.size(); ++i)
	{
		PhotoFrameActor * pfActor = (PhotoFrameActor *) photoFrames[i];
		pfActor->onUpdate();
	}

	// after updating the actor positions, etc. update the labels
	if (sceneManager->settings.RenderText) 
	{
		textManager->updateRelativeNameablePositions();
		textManager->updateAbsoluteNameablePositions();
	}

	Renamer->textBoxUpdate();
}

void UpdateCameraPlanes()
{

	// Any type or movement, adjust boxes
	Ray r[4];
	Vec3 v[4], w[4], ev[4], ew[4];
	NxPlane floor = NxPlane(Vec3(0.0f), Vec3(0,1,0));
	NxF32 dist;
	Vec3 pt[4], dir;
	float angle = AngleBetweenVectors(Vec3(0, -1, 0), cam->getDir()) * 180/PI;
	float distVert, distHoriz;
	int camShrinkFactor = 50; //number of pixels to move walls inwards
	float camSideWallsFudgeFactor = 0.85f; // This is a percentage (85%) of how far the side walls are angled

	if (!cam->isAnimating() && GLOBAL(settings).camWallsEnabled && !GLOBAL(touchGestureBrowseMode))
	{
		// Update the Walls
		// +---0---+ <----- Floor
		// |       |	
		// 3       1 <----- pt[]
		// |       |	
		// +---2---+		
		// Get the coordinates in the world based on corners of screen
		window2world(winOS->GetWindowWidth() / 2, camShrinkFactor, v[0], w[0]);
		window2world(winOS->GetWindowWidth() - int(camShrinkFactor * 2.0f * camSideWallsFudgeFactor), winOS->GetWindowHeight() / 2, v[1], w[1]);
		window2world(winOS->GetWindowWidth() / 2, winOS->GetWindowHeight() - camShrinkFactor, v[2], w[2]);
		window2world(int(camShrinkFactor * 2.0f * camSideWallsFudgeFactor), winOS->GetWindowHeight() / 2, v[3], w[3]);

		// Corners for Angled View
		// 0----2----+
		//  \       /
		//   \     /
		//    1-3-+
		// Find out the angle between this angled view and the straight view
		window2world(0, 0, ev[0], ew[0]);
		window2world(0, winOS->GetWindowHeight(), ev[1], ew[1]);
		window2world(winOS->GetWindowWidth() / 2, 0, ev[2], ew[2]);
		window2world(winOS->GetWindowWidth() / 2, winOS->GetWindowHeight(), ev[3], ew[3]);
		
		Vec3 angledVec, straightVec, dir;
		for (int i = 0; i < 4; i++)
		{
			// Project this ray to the floor
			dir = ew[i] - ev[i];
			dir.normalize();
			r[i] = Ray(ev[i], dir);
			NxRayPlaneIntersect(r[i], floor, dist, pt[i]);
		}

		// Get the angle of the side of the screen in 3D space
		angledVec = pt[0] - pt[1];
		straightVec = pt[2] - pt[3];
		angledVec.normalize();
		straightVec.normalize();

		// Figure out the angle between the angled vector and the straight vector
		float angleBetweenRays = acos(angledVec.dot(straightVec)) * 180 / PI;
		//consoleWrite("%lf\n", angleBetweenRays);

		// Create rays from the near clipping plane to the far clipping plane
		dir = w[0] - v[0];
		dir.normalize();
		r[0] = Ray(v[0], dir);
		dir = w[1] - v[1];
		dir.normalize();
		r[1] = Ray(v[1], dir);
		dir = w[2] - v[2];
		dir.normalize();
		r[2] = Ray(v[2], dir);
		dir = w[3] - v[3];
		dir.normalize();
		r[3] = Ray(v[3], dir);

		for (int i = 0; i < 4; i++)
		{
			// Project this ray to the floor
			NxRayPlaneIntersect(r[i], floor, dist, pt[i]);

			if (abs(angle) > GLOBAL(settings).CAMWALLS_MAX_ANGLE)
			{
				// mvoe the Location of these walls above teh camera
				pt[i].x = pt[i].z = 0;
				pt[i].y = cam->getEye().y + 1000;
				GLOBAL(CamWalls)[i]->raiseActorFlag(NX_AF_DISABLE_COLLISION);
			}else{
				GLOBAL(CamWalls)[i]->clearActorFlag(NX_AF_DISABLE_COLLISION);
			}

			// Update the Walls
			// +---0---+ <----- Floor
			// |       |	
			// 3       1 <----- pt[]
			// |       |	
			// +---2---+
			GLOBAL(CamWalls)[i]->setGlobalPosition(pt[i]);

			if (i % 2)
			{
				// Index 1 and 3 should be angled
				GLOBAL(CamWalls)[i]->setGlobalOrientationQuat(Quat(i == 1 ? angleBetweenRays * camSideWallsFudgeFactor : -angleBetweenRays * camSideWallsFudgeFactor, Vec3(0, -1, 0)));
			}
		}

		// Figure out the size of the walls
		if (abs(angle) > GLOBAL(settings).CAMWALLS_MAX_ANGLE)
		{
			// These walls are VERY small, they should be out of the way of everything
			distVert = distHoriz = 1;
		}else{
			distVert = DistanceBetweenPoints(pt[0], pt[2]);
			distHoriz = DistanceBetweenPoints(pt[3], pt[1]);
		}
	}else{
		// move the Location of these walls above teh camera
		for (int i = 0; i < 4; i++)
		{
			pt[i].x = pt[i].z = 0;
			pt[i].y = cam->getEye().y + 1000;
			GLOBAL(CamWalls)[i]->setGlobalPosition(pt[i]);
			GLOBAL(CamWalls)[i]->raiseActorFlag(NX_AF_DISABLE_COLLISION);
		}

		// Tiny walls.
		distVert = distHoriz = 1;
	}

	// Set the size so the boxes overlap
	for (int i = 0; i < 4; i++)
	{
		GLOBAL(CamWalls)[i]->setDims(Vec3(i % 2 ? cam->getEye().y / 25 : distHoriz + 5, cam->getEye().y, i % 2 ? distVert + 5 : cam->getEye().y / 25));
	}

	GLOBAL(lastEye) = cam->getEye();

}

void CreateBumpObjectsFromWorkingDirectory(bool createFirstRunObjects)
{
	if (scnManager->getCurrentLibrary())
		CreateBumpObjectsFromLibrary(scnManager->getCurrentLibrary(), createFirstRunObjects);
	else
		CreateBumpObjectsFromDirectory(native(scnManager->getWorkingDirectory()), createFirstRunObjects);
}

void CreateBumpObjectsFromLibrary(QSharedPointer<Library>& library, bool createFirstRunObjects)
{
	// Add objects from the other library folders too
	if (library)
	{
		QList<QString> dirs = library->getFolderPaths();
		QListIterator<QString> dirIt(dirs);
		assert(dirs.size() > 0);
		while (dirIt.hasNext())
		{
			CreateBumpObjectsFromDirectory(dirIt.next(), createFirstRunObjects);
		}
	}
}

void *CreateBumpObjectsFromDirectory(QString DirectoryPath, bool createFirstRunObjects)
{
	GLOBAL(settings).curNumStickyNotes = 0;

	QDir dir(DirectoryPath);
	FileSystemActor *fsActor;
	QString fileName;
	vector<iconDetails> icons;
	iconDetails icon;
	float minX = 0, minY = 0;
	float maxX = -1, maxY = -1;
	float dropX = 0, dropZ = 0, dropY = 1;
	vector<int> specialIcons;
	bool foundIcon = false;
	int IconIndex = -1;
	int x, y, r, l, t, b;
	Vec3 endPos;

	bool isTradeShow = (dir == winOS->GetTradeshowDirectory());

	if (fsManager->getFileAttributes(DirectoryPath) & Directory)
	{
		if (!scnManager->isShellExtension)
		{
			winOS->GetWorkArea(r, l, t, b);
		}
		else
		{
			RECT windowDims;
			GetClientRect(winOS->GetWindowsHandle(), &windowDims);
			l = windowDims.left;
			r = windowDims.right;
			t = windowDims.top;
			b = windowDims.bottom;
		}

		// This is only to use our very hackish approach of getting icon positions
		bool isDefaultWorkingDirectory = (DirectoryPath == winOS->GetSystemPath(DesktopDirectory));
		if (isDefaultWorkingDirectory)
		{
			WindowState oldWinState = winOS->GetWindowState();
			if(oldWinState == WorkArea)
			{
				winOS->detatchFromDesktopWindow();
				Sleep(250);
			}

			// 			winOS->SetWindowState(Windowed); //NOTE:  This looks awkward.  Goes to windowed mode before it completely resets.  isn't seamless.  

			winOS->GetIconPositions(icons);
			if(oldWinState == WorkArea) 
				winOS->SetWindowState(oldWinState);
			winOS->GetMainMonitorCoords(x, y);

			// Subtract the location of the main monitor
			for (int i = 0; i < icons.size(); i++)
			{
				icons[i].x += x + l;
				icons[i].y += y + t;
			}

			// Save the size of the work area
			maxX = float(r);
			maxY = float(b);
			minX = float(l);
			minY = float(t);
			// shift in case we are not in the primary work area
			if (minX < 0) {
				maxX -= minX;
				minX -= minX;
			}
			if (minY < 0) {
				maxY -= minY;
				minY -= minY;
			}

			// Reshape the walls so that they are bound to the size of the desktop
			// Dont' resize the walls to the work area if we are in infinite desktop mode
			if (!scnManager->isInInfiniteDesktopMode)
				ResizeWallsToWorkArea(r - l, b - t);
		}
		else
		{
			// i18n: TODO, use QDir listings
			// Iterate through all the icons in this directory
			//directory_iterator EndIterator;
			StrList dirListing = fsManager->getDirectoryContents(DirectoryPath);

			//for (directory_iterator Iterator(Directory); Iterator != EndIterator; Iterator++)
			for (uint i = 0; i < dirListing.size(); ++i)
			{						
				//fileName = Iterator->native_directory_string();
				fileName = dirListing[i];

				bool isHidden = (fsManager->getFileAttributes(fileName) & Hidden) > 0;
				if (!isHidden || GLOBAL(settings).LoadHiddenFiles)
				{
					// Add each file to the icons listing
					icons.push_back(iconDetails(fileName));
					maxX = maxY = 1;
				}

			}
		}

		if ((false) && isDefaultWorkingDirectory)
		{
			// lay them out in a flowing grid
			// [T, B, R, L]
			vector<NxActorWrapper *> walls = GLOBAL(Walls);
			vector<Vec3> wallsPos = GLOBAL(WallsPos);
			Vec3 topLeft(wallsPos[3].x - walls[3]->getDims().z,
				0, wallsPos[0].z - walls[0]->getDims().z);
			float bottom = wallsPos[1].z + walls[1]->getDims().z;
			Vec3 iconDims(GLOBAL(settings).xDist, GLOBAL(settings).zDist, GLOBAL(settings).yDist);
			float iconScale = 2.0f;
			iconDims.x *= iconScale;
			iconDims.y *= iconScale;

			// lay out each of the items 
			float borderOffset = 10.0f;
			float leading = 10.0f;
			Vec3 pos = topLeft;
			pos.x -= borderOffset;
			pos.z -= borderOffset;

			for (int i = 0; i < icons.size(); ++i)
			{
				if ((pos.z - (3.0f * iconDims.y)) < bottom)
				{
					pos.x -= (3.0f * iconDims.x) - leading;
					pos.z = topLeft.z - borderOffset;
				}

				BumpObject * newObj = NULL;
				if (WebThumbnailActor::isValidWebThumbnailActorUrlFile(icons[i].iconName))
				{
					// load this new page
					WebActor * actor = new WebActor();
					actor->load(FileSystemActorFactory::parseURLFileForLink(icons[i].iconName));		
					actor->setDims(iconDims);
					newObj = actor;

					// delete the actual url
					fsManager->deleteFileByName(icons[i].iconName);
				}
				else
				{
					// Load the icons as the texture of this actor
					// Note: iconDims must be set before setFilePath in case there are
					// 	 any sticky notes which need to be updated to the current dims.
					fsActor = FileSystemActorFactory::createFileSystemActor(icons[i].iconName);
					fsActor->setDims(iconDims);
					fsActor->setFilePath(icons[i].iconName);
					newObj = fsActor;
				}
				newObj->setGlobalPosition(Vec3(pos.x - iconDims.x, GLOBAL(settings).yDist, pos.z - iconDims.y));
				newObj->setGlobalOrientation(GLOBAL(straightIconOri));	

				pos.z -= (3.0f * iconDims.y);
			}
		}
		else
		{
			int itemIndex = 0;

			// we use max to bound the width, so that it doesn't get too wide
			float gridWidthHeightRatio = float(r - l)/(b - t);
			float sqrtItemsCount = sqrt(float(icons.size()));
			const int gridWidth = NxMath::max(1, int(int(sqrtItemsCount) * gridWidthHeightRatio) - 1);
			const int gridHeight = int(sqrtItemsCount);
			const int halfGridWidth = (gridWidth / 2);
			const int halfGridHeight = (gridHeight / 2);
			float gridXSpacingMultiplier = 85.0f;
			float gridZSpacingMultiplier = 85.0f;

			// XXX: Hack to give a nice layout for our particular test photos.
			if (GLOBAL(isBumpPhotoMode))
			{
				gridXSpacingMultiplier = 115.0f;
				gridZSpacingMultiplier = 100.0f;
			}

			// See what Icons are available on the desktop that are not files
			for (int i = 0; i < icons.size(); i++, itemIndex++)
			{
				bool isHidden = (fsManager->getFileAttributes(icons[i].iconName) & Hidden) > 0;
				if (!isHidden || GLOBAL(settings).LoadHiddenFiles)
				{
					// NOTE: we don't bound the icons to those available on the main desktop 
					//		 (technically, the main desktop isn't even necessarily between 0,max)
					// if (icons[i].x >= 0 && icons[i].y >= 0 && icons[i].x < maxX && icons[i].y < maxY)
					{
						// A brute force way of finding out whether this icon is available on the desktop
						if (maxX != -1 && maxY != -1)
						{
							dropX = (icons[i].x - minX) - (maxX - minX) / 2;
							dropZ = (icons[i].y - minY) - (maxY - minY) / 2;
						}else{
							dropX = dropY = 0;
						}

						// grid the items
						if (isDefaultWorkingDirectory)
						{
							endPos = Vec3(dropX * GLOBAL(factor) - (GLOBAL(settings).yDist), 
								dropY, 
								dropZ * GLOBAL(factor) - (GLOBAL(settings).zDist));
						}
						else
						{
							endPos = Vec3(((i % gridWidth)-halfGridWidth) * gridXSpacingMultiplier * GLOBAL(factor) - (GLOBAL(settings).yDist), 
								dropY, 
								((i / gridWidth)-halfGridHeight) * gridZSpacingMultiplier * GLOBAL(factor) - (GLOBAL(settings).zDist));
						}

						QString filePath;

						// Set up the Icon graphic for this icon
						if (!icons[i].iconName.isEmpty())
						{
							filePath = icons[i].iconName;

							BumpObject * newObj = NULL;
							if (WebThumbnailActor::isValidWebThumbnailActorUrlFile(filePath))
							{
								// load this new page
								WebActor * actor = new WebActor();
								actor->setDimsToDefault();
								actor->load(FileSystemActorFactory::parseURLFileForLink(filePath));		
								newObj = actor;

								// delete the actual url
								fsManager->deleteFileByName(filePath);
							}
							else
							{
								// NOTE: we need to set the file path _after_ we set the dimensions so that
								// the sticky notes are relaidout to the correct size
								// Load the icons as the texture of this actor
								fsActor = FileSystemActorFactory::createFileSystemActor(filePath);
								fsActor->setDimsToDefault();
								fsActor->setFilePath(filePath);
								newObj = fsActor;
							}
							newObj->setGlobalPosition(Vec3(endPos.x, GLOBAL(settings).yDist, endPos.z));
							newObj->setGlobalOrientation(GLOBAL(straightIconOri));	
						}
					}
				}
			}

			if ((scnManager->isBumpPhotoMode || scnManager->isInInfiniteDesktopMode) && 
				scnManager->isWorkingDirectoryPrimarilyOfFileType(Image | Folder))
			{
				vector<FileSystemActor *> fsActors = scnManager->getFileSystemActors();
				for (int i = 0; i < fsActors.size(); ++i)
				{
					if (fsActors[i]->isFileSystemType(Image))
					{
						fsActors[i]->grow();
						fsActors[i]->finishAnimation();
					}
				}
			}
		}

		if (createFirstRunObjects)
		{
			// create a new photo frame actor in the scene if we are not in shell extension 
			// are on the desktop, and have not deleted previous auto-generated photo frames
			if (!scnManager->isShellExtension) 
			{
				if (native(scnManager->getWorkingDirectory()) == winOS->GetSystemPath(DesktopDirectory))
					CreateDefaultScenePhotoFrames();

				// create the default custom actors
				CreateCustomEmailActor();
				CreateCustomTwitterActor();
				Key_CreateFacebookWidget();
				//Key_CreateSharingWidget();
				if (!isTradeShow)			
				{
					CreateCustomStickyNotePadActor();
					CreateCustomPrinterActor(); // icon collides with trade show icons
				}

				//CreateCustomFlickrActor();
			}

			// XXX: Some custom hacks for the photo mode demo
			if (scnManager->isBumpPhotoMode)
			{
				vector<FileSystemActor*> actors = scnManager->getFileSystemActors(QString(".*Starred"), true, true);
				if (actors.size() == 1)
				{
					FileSystemActor* starredPile = actors.front();
					starredPile->grow(25, 2.0f);

					// Set its position just below the other custom actors
					float edgeBuffer = 30.0f;
					Box desktopBox = GetDesktopBox();
					Quat ori;

					Vec3 pos(
						desktopBox.center.x - desktopBox.extents.x, 
						getHeightForResetLayout(&desktopBox, -starredPile->getDims().y), 
						desktopBox.center.z + desktopBox.extents.z - 5.5f * (edgeBuffer - starredPile->getDims().z));
					ori = Quat(90, Vec3(1, 0, 0));
					pos.y = starredPile->getDims().z;

					starredPile->setGlobalPosition(pos);
					starredPile->setGlobalOrientation(ori);
					starredPile->pileize();
				}
			}

			// Create a new post-it in the scene, when appropriate. 
			if (!scnManager->isShellExtension && !scnManager->isBumpPhotoMode && !isTradeShow && !scnManager->runAutomatedJSONTestsOnStartup)
			{
				// check if there's a sticky note already
				bool stickyNoteFound = false;
				vector<BumpObject *> objs = scnManager->getBumpObjects();
				for (int i = 0; i < objs.size(); ++i)
				{
					if (objs[i]->getObjectType() == ObjectType(BumpActor, FileSystem, StickyNote))
						stickyNoteFound = true;
				}

				if (!stickyNoteFound)
				{
					StickyNoteActor * stickyNote = CreateStickyNote();
					if (stickyNote)
					{
						stickyNote->modifyWithString(BumpTopStr->getString("EditStickyNote"));
						stickyNote->shrink(25, 0.85f);

						// pin to the left wall
						float edgeBuffer = 30.0f;
						Box desktopBox = GetDesktopBox();
						Quat rotY(90, Vec3(0,1,0));
						Quat ori;
						ori.multiply(rotY, stickyNote->getGlobalOrientationQuat());
						Vec3 pos(desktopBox.center.x + desktopBox.extents.x, 
							getHeightForResetLayout(&desktopBox),
							desktopBox.center.z + desktopBox.extents.z - edgeBuffer - stickyNote->getDims().z);
						stickyNote->setGlobalPosition(pos);
						stickyNote->setGlobalOrientation(ori);
						stickyNote->onPin();
					}
				}
			}
		}

	}
	else if (fsManager->getFileAttributes(DirectoryPath) != 0)
	{
		if (fsManager->getFileAttributes(DirectoryPath) & Hidden && !GLOBAL(settings).LoadHiddenFiles)
		{
			// Hidden File
		}
		else
		{
			endPos = Vec3(dropX * GLOBAL(factor) - (GLOBAL(settings).yDist), 
				dropY, 
				dropZ * GLOBAL(factor) - (GLOBAL(settings).zDist));

			// This is a single file. Create it and Load its texture as the icon
			// Load the icons as the texture of this actor
			fsActor = FileSystemActorFactory::createFileSystemActor(DirectoryPath);
			fsActor->setDimsToDefault();
			fsActor->setGlobalPosition(Vec3(endPos.x, GLOBAL(settings).yDist, endPos.z));
			fsActor->setGlobalOrientation(GLOBAL(straightIconOri));

			// Add this item to the list where it can be returned
			fsActor->setFilePath(DirectoryPath);

			CreateRandomAnimPath(fsActor, cam->getEye(), fsActor->getGlobalPosition(), 40);
		}
	}

	textManager->invalidate();

	return NULL; //DirectoryListing;
}

/*
* Creates a couple default photo frames in the scene, positioning them on the walls.
*/
PhotoFrameActor * CreatePhotoFrameHelper(QString sourcePath)
{
	PhotoFrameSource * source = PhotoFrameActor::resolveSourceFromString(sourcePath);
	if (source)
	{
		// ensure that there are no duplicate frames
		vector<BumpObject *> objects = scnManager->getBumpObjects();
		for (int i = 0; i < objects.size(); ++i)
		{
			if (objects[i]->getObjectType() == ObjectType(BumpActor, FileSystem, PhotoFrame))
			{
				if (dynamic_cast<PhotoFrameActor *>(objects[i])->equals(source))
				{
					// select the existing frame and notify the user
					sel->clear();
					sel->add(objects[i]);
					objects[i]->setFreshnessAlphaAnim(1.0f, 80);
					printUnique("CreatePhotoFrameHelper", BumpTopStr->getString("PhotoFrameExists"));
					return NULL;
				}
			}
		}

		PhotoFrameActor * obj = new PhotoFrameActor(source);
		// create a _vertical_ photoframe
		// obj->setGlobalOrientation(GLOBAL(straightIconOri));

		// use the aspect of the loading photo frame image if available
		float width = GLOBAL(settings).xDist;
		float height = GLOBAL(settings).zDist;
		Vec3 dims = texMgr->getTextureDims("photoframe.loading");
		if (!dims.isNotUsed())
		{
			float aspect = (dims.x / dims.y);
			height /= aspect;
		}
		obj->setDims(Vec3(width, height, GLOBAL(settings).yDist));
		obj->grow(25, 4.0f);
		obj->finishAnimation();
		return obj;
	}
	return NULL;
}

void CreateDefaultScenePhotoFrames()
{
	// create a photo frame with the specified path
	PhotoFrameActor * myPicturesFrame = CreatePhotoFrameHelper(winOS->GetSystemPath(MyPictures));

	// position each of the frames 
	Box desktopBox = GetDesktopBox();
	float edgeBuffer = 30.0f;

	// My Pictures photo frame 
	if (myPicturesFrame)
	{
		// move the My Pictures frame to be near the left edge of the back wall
		Vec3 myPicturesFramePos(desktopBox.center.x + desktopBox.extents.x - myPicturesFrame->getDims().x - edgeBuffer, 
			getHeightForResetLayout(&desktopBox), 
			desktopBox.center.z + desktopBox.extents.z);
		myPicturesFrame->setGlobalPosition(myPicturesFramePos);
		myPicturesFrame->onPin();
	}
};

void CreateCustomEmailActor()
{
	CustomActor * actor = scnManager->getCustomActor<EmailActorImpl>();
	if (!actor)
	{
		// create the email actor 
		CustomActorInfo * info = new CustomActorInfo;
		EmailActorImpl * eaImpl = new EmailActorImpl(info);
		CustomActor * emailActor = new CustomActor(info);

		// pin to the right wall
		float edgeBuffer = 30.0f;
		Box desktopBox = GetDesktopBox();
		Quat ori;
		Vec3 pos(desktopBox.center.x - desktopBox.extents.x + emailActor->getDims().z,
			getHeightForResetLayout(&desktopBox, -emailActor->getDims().y),
			desktopBox.center.z + desktopBox.extents.z - edgeBuffer - emailActor->getDims().z);

		// If we are in photo mode, place the item on the floor instead
		// of on the wall, because the wall doesn't exist
		if (GLOBAL(isBumpPhotoMode))
		{
			ori = Quat(90, Vec3(1, 0, 0));
			pos.y = emailActor->getDims().z;
		}
		else
		{
			ori = Quat(-90, Vec3(0,1,0));
		}

		emailActor->setGlobalPosition(pos);
		emailActor->setGlobalOrientation(ori);
		emailActor->onPin();
	}
}

void CreateCustomStickyNotePadActor()
{
	CustomActor * actor = scnManager->getCustomActor<StickyNotePadActorImpl>();
	if (!actor)
	{
		// create the email actor 
		CustomActorInfo * info = new CustomActorInfo;
		StickyNotePadActorImpl * snpImpl = new StickyNotePadActorImpl(info);
		CustomActor * stickyNotePadActor = new CustomActor(info);

		// pin to the right wall
		float edgeBuffer = 30.0f;
		Box desktopBox = GetDesktopBox();
		Quat ori;
		Vec3 pos(desktopBox.center.x + desktopBox.extents.x - stickyNotePadActor->getDims().z,
			getHeightForResetLayout(&desktopBox, -stickyNotePadActor->getDims().y),
			desktopBox.center.z + desktopBox.extents.z - edgeBuffer - stickyNotePadActor->getDims().z);

		// If we are in photo mode, place the item on the floor instead
		// of on the wall, because the wall doesn't exist
		if (GLOBAL(isBumpPhotoMode))
		{
			ori = Quat(90, Vec3(1, 0, 0));
			pos.y = stickyNotePadActor->getDims().z;
		}
		else
		{
			ori = Quat(90, Vec3(0,1,0));
		}

#ifdef ALLOW_DRAG_CREATE_STICKY_WC_JAN10
		stickyNotePadActor->getShapes()[0]->setGroup(ISOLATED_GROUP_NUMBER);
		stickyNotePadActor->setFrozen(true);
#endif
		stickyNotePadActor->setGlobalPosition(pos);
		stickyNotePadActor->setGlobalOrientation(ori);
		stickyNotePadActor->onPin();
	}
}

void CreateCustomPrinterActor()
{
	CustomActor * actor = scnManager->getCustomActor<PrinterActorImpl>();
	if (!actor)
	{
		// create the email actor 
		CustomActorInfo * info = new CustomActorInfo;
		PrinterActorImpl * paImpl = new PrinterActorImpl(info);
		CustomActor * printerActor = new CustomActor(info);

		// pin to the right wall
		float edgeBuffer = 30.0f;
		Box desktopBox = GetDesktopBox();
		Quat ori(-90, Vec3(0,1,0));		
		Vec3 pos(desktopBox.center.x + desktopBox.extents.x - printerActor->getDims().z,
			getHeightForResetLayout(&desktopBox, -printerActor->getDims().y),
			desktopBox.center.z + desktopBox.extents.z - 2.5f * (edgeBuffer - printerActor->getDims().z));
		printerActor->setGlobalPosition(pos);
		printerActor->setGlobalOrientation(ori);
		printerActor->onPin();
	}
}

void CreateCustomFacebookActor()
{
	assert(false);
}

void CreateCustomTwitterActor()
{
	CustomActor * actor = scnManager->getCustomActor<TwitterActorImpl>();
	if (!actor)
	{
		// Create the actor
		CustomActorInfo * info = new CustomActorInfo;
		TwitterActorImpl * faImpl = new TwitterActorImpl(info);
		CustomActor * twitterActor = new CustomActor(info);

		// Pin it to the right wall, if it exists

		float edgeBuffer = 30.0f;
		Box desktopBox = GetDesktopBox();
		Quat ori(180, Vec3(1,0,0));
		Vec3 pos(desktopBox.center.x - desktopBox.extents.x + twitterActor->getDims().z,
			getHeightForResetLayout(&desktopBox, -twitterActor->getDims().y),
			desktopBox.center.z + desktopBox.extents.z - 2.5f * (edgeBuffer - twitterActor->getDims().z));

		// If we are in photo mode, place the item on the floor instead
		// of on the wall, because the wall doesn't exist
		if (GLOBAL(isBumpPhotoMode))
		{
			ori = Quat(90, Vec3(1, 0, 0));
			pos.y = twitterActor->getDims().z;
		}
		else
		{
			ori = Quat(-90, Vec3(0,1,0));
		}

		twitterActor->setGlobalPosition(pos);
		twitterActor->setGlobalOrientation(ori);
		twitterActor->onPin();
	}
}

void CreateCustomFlickrActor()
{
	// POSITION IS CURRENTLY INVALID
	// NEED TO DEFINE PROPER PLACEMENT FOR FLICKR WIDGET
	CustomActor * actor = scnManager->getCustomActor<FlickrActorImpl>();
	if (!actor)
	{
		// Create the actor
		CustomActorInfo * info = new CustomActorInfo;
		FlickrActorImpl * faImpl = new FlickrActorImpl(info);
		CustomActor * flickrActor = new CustomActor(info);

		// Pin it to the right wall, if it exists

		float edgeBuffer = 30.0f;
		Box desktopBox = GetDesktopBox();
		Quat ori(180, Vec3(1,0,0));

		Vec3 pos(desktopBox.center.x - desktopBox.extents.x + flickrActor->getDims().z,
			getHeightForResetLayout(&desktopBox, -flickrActor->getDims().y),
			desktopBox.center.z + desktopBox.extents.z - 5.0f * (edgeBuffer - flickrActor->getDims().z));

		// If we are in photo mode, place the item on the floor instead
		// of on the wall, because the wall doesn't exist
		if (GLOBAL(isBumpPhotoMode))
		{
			ori = Quat(90, Vec3(1, 0, 0));
			pos.y = flickrActor->getDims().z;
		}
		else
		{
			ori = Quat(-90, Vec3(0,1,0));
		}

		flickrActor->setGlobalPosition(pos);
		flickrActor->setGlobalOrientation(ori);
		flickrActor->onPin();
	}
}

QString CreateStickyNoteFileNameHelper()
{
	// determine new postit filename
	int count = 1;
	QString filename("StickyNote");
	filename.append(QString::number(QDateTime::currentDateTime().toTime_t()));
	filename.append(".txt");
	QFileInfo newSticky = fsManager->getUniqueNewFilePathInWorkingDirectory(filename);
	return native(newSticky);
}

StickyNoteActor * CreateStickyNote()
{
	if (GLOBAL(settings).freeOrProLevel == AL_FREE && StickyNoteMenuActionCustomizer::hasExceededMaxNumStickyNotes())
	{
		printUnique("CreateStickyNote", BtUtilStr->getString("StickyNoteLimit"));
		return NULL;
	}

	reauthorize(false);

	QString stickyNoteName = CreateStickyNoteFileNameHelper();
	if (stickyNoteName.isEmpty())
		return NULL;

	QFileInfo stickyNotePath = make_file(scnManager->getWorkingDirectory(), stickyNoteName);

	// create new file
	write_file_utf8(QString(), native(stickyNotePath));

	StickyNoteActor * stickyNote = new StickyNoteActor();
	stickyNote->setDimsToDefault();
	stickyNote->grow(25, 3.0f);
	stickyNote->finishAnimation();
	stickyNote->setFilePath(native(stickyNotePath));	
	stickyNote->generateRandomStickyNote();

	return stickyNote;
}

void processTossing()
{	
	const vector<BumpObject *>& objList = scnManager->getBumpObjects();
	float distBetweenItems = 8;

	// Process Tossing
	for(int i=0; i<GLOBAL(Tossing).size(); i++)
	{
		BumpObject *tossObj = GLOBAL(Tossing)[i];
		vector<BumpObject *> tossedObj;
		tossedObj.push_back(tossObj);

		// Removes the item if it has a parent (Tossing/onDrop bug)
		if (GetBumpObject(GLOBAL(Tossing)[i])->getParent())
		{
			sel->remove(GLOBAL(Tossing)[i]);

			// Item no longer being tossed
			GLOBAL(Tossing).erase(GLOBAL(Tossing).begin() + i);
			i--;
			continue;
		}

		if(GLOBAL(Tossing)[i]->getLinearVelocity().magnitudeSquared() < GLOBAL(settings).EndTossVelocitySq) //no longer being tossed
		{
			GLOBAL(Tossing).erase(GLOBAL(Tossing).begin()+i);
			i--;
		}
		else //Handle Tossing to Other BumpTops, this is all probably broken :/
		{
			if (GLOBAL(isInSharingMode))
			{
				if (!GLOBAL(Tossing)[i]->isAnimating())
				{
					// find the direction of the toss
					Vec3 vel = GLOBAL(Tossing)[i]->getLinearVelocity();
					vel.normalize();
					Vec3 left(1, 0, 0);
					Vec3 right(-1, 0, 0);
					Vec3 up(0, 0, 1);
					Vec3 down(0, 0, -1);
					float deg = 0.0f;

					deg = acos(vel.dot(left)) * 180.0f / PI;
					if (deg < 45.0f)
						evtManager->onFlick(FLICKDIRECTION_LEFT, Vec3((float)GLOBAL(mx), (float)GLOBAL(my), 0));
					deg = acos(vel.dot(right)) * 180.0f / PI;
					if (deg < 45.0f)
						evtManager->onFlick(FLICKDIRECTION_RIGHT, Vec3((float)GLOBAL(mx), (float)GLOBAL(my), 0));
					deg = acos(vel.dot(up)) * 180.0f / PI;
					if (deg < 45.0f)
						evtManager->onFlick(FLICKDIRECTION_UP, Vec3((float)GLOBAL(mx), (float)GLOBAL(my), 0));
					deg = acos(vel.dot(down)) * 180.0f / PI;
					if (deg < 45.0f)
						evtManager->onFlick(FLICKDIRECTION_DOWN, Vec3((float)GLOBAL(mx), (float)GLOBAL(my), 0));
				}
			}
			else
			{
				if (GLOBAL(settings).DrawOtherBumpTops)
				{
					//If pile is close enough, add to that pile
					Vec3 Pos = GLOBAL(Tossing)[i]->getGlobalPosition();

					// Resets it when its moved back to the owner bumptop
					uint indx = vecContains(GLOBAL(gettingShared), GLOBAL(Tossing)[i]);
					if (indx != -1)
					{
						if (Pos.x > -200 || Pos.x < 200 || Pos.z < 200)
						{
							GLOBAL(gettingShared).erase(GLOBAL(gettingShared).begin() + indx);
						}
					}

					if (vecContains(GLOBAL(gettingShared), GLOBAL(Tossing)[i]) == -1 && (Pos.x > GLOBAL(WallsPos)[3].x || Pos.x < GLOBAL(WallsPos)[2].x || Pos.z > GLOBAL(WallsPos)[0].z))
					{
						// Check if its outside the Background and toss it to the desktop
						if (Pos.x > GLOBAL(WallsPos)[3].x)
						{
							ZeroAllActorMotion(GLOBAL(Tossing)[i], false);
							GetBumpActor(GLOBAL(Tossing)[i])->setPoseAnim(GLOBAL(Tossing)[i]->getGlobalPose(), Mat34(GLOBAL(Tossing)[i]->getGlobalOrientation(), Vec3(500, 100, 0)), 100);
						}else if (Pos.x < GLOBAL(WallsPos)[2].x)
						{
							ZeroAllActorMotion(GLOBAL(Tossing)[i], false);
							GetBumpActor(GLOBAL(Tossing)[i])->setPoseAnim(GLOBAL(Tossing)[i]->getGlobalPose(), Mat34(GLOBAL(Tossing)[i]->getGlobalOrientation(), Vec3(-500, 100, 0)), 100);
						}else if (Pos.z > GLOBAL(WallsPos)[0].z)
						{
							ZeroAllActorMotion(GLOBAL(Tossing)[i], false);
							GetBumpActor(GLOBAL(Tossing)[i])->setPoseAnim(GLOBAL(Tossing)[i]->getGlobalPose(), Mat34(GLOBAL(Tossing)[i]->getGlobalOrientation(), Vec3(0, 100, 500)), 100);
						}

						GLOBAL(gettingShared).push_back(GLOBAL(Tossing)[i]);
						continue;
					}
				}

				// Process tossing to items and Piles
				for (uint j = 0; j < objList.size(); j++)
				{
					if (scnManager->containsObject(objList[j]))				
					{
						if ((objList[j]->getObjectType() == ObjectType(BumpActor) ||  objList[j]->getObjectType() == ObjectType(BumpPile)) &&
							!objList[j]->getParent() && objList[j] != tossObj && !sel->isInSelection(objList[j]))
						{
							Vec3 objPos = objList[j]->getGlobalPosition();
							Vec3 tossObjPos = tossObj->getGlobalPosition();
							objPos.y = tossObjPos.y = 0.0f;		// don't factor y-position into tossing check
							float targetSphereRad = objList[j]->getDims().magnitudeSquared();
							float tossSphereRad = tossObj->getDims().magnitudeSquared();
							float curDist = (objPos - tossObjPos).magnitudeSquared();
							curDist -= targetSphereRad + tossSphereRad;

							// Check if the distance of the item being tossed is in range of a target
							if (curDist < distBetweenItems * distBetweenItems)
							{
								// If this target accepts the tossed item, have it receive the item
								if (objList[j]->isValidTossTarget() && objList[j]->isValidToss(sel->getBumpObjects()))
								{
									objList[j]->onTossRecieve(sel->getBumpObjects());
									sel->clear();

									if (i < GLOBAL(Tossing).size())
										GLOBAL(Tossing).erase(GLOBAL(Tossing).begin() + i);
									i--;
								}
							}
						}
					}
				}
			}
		}
	}


}

void processInPileGhosting(bool render, int& indexOut, Pile ** pileOut)
{
	assert(pileOut);

	// reset values
	indexOut = -2; // -2 if the ray didnt hit the pile, -1 if the ray didnt hit any quadrant
	*pileOut = NULL;

	if (!sel->getPickedActor() || 
		!sel->getPickedActor()->isBumpObjectType(BumpActor) || 
		!((Actor *)sel->getPickedActor())->getObjectToMimic())
	{
		GLOBAL(mode) = None;
		return;
	}

	Actor * actor = (Actor *) sel->getPickedActor();
	Vec3 drawPoint(0.0f);
	Vec3 lineStart(0.0f), lineEnd(0.0f), dir(0.0f), pointOnPlane(0.0f);
	Vec3 worldHitPos(0.0f);
	Ray ray;
	float hitDist(0.0f);

	QSet<BumpObject *> objectsUnderCursor = getAllObjectsUnderCursor(GLOBAL(mx), GLOBAL(my));
	QSetIterator<BumpObject *> iter(objectsUnderCursor);
	while (iter.hasNext())
	{
		BumpObject * object = iter.next();
		if (object->isBumpObjectType(BumpPile))
		{
			Pile * pile = (Pile *) object;
			if (pile->getPileState() == Grid)
			{
				// if we are hovering over a gridded pile, then find out where to put the 
				// actor in terms of indices in the pile

				// find the local space coordinate on the pile
				window2world(GLOBAL(mx), GLOBAL(my), lineStart, lineEnd);
				dir = lineEnd - lineStart;
				dir.normalize();
				ray = Ray(lineStart, dir);
				NxPlane plane(Vec3(0, -pile->getGlobalPosition().y, 0), Vec3(0,1,0));
				if (NxRayPlaneIntersect(ray, plane, hitDist, pointOnPlane))
				{
					worldHitPos = pointOnPlane;

					// find the closest object in the pile to this point
					float minDistance(999999999999.0f);
					float tmpDistance(0.0f);
					int minDistanceIndex = -1;
					int pickedActorIndex = pile->getNumItems();
					vector<BumpObject *> children = pile->getPileItems();
					for (int i = 0; i < children.size(); ++i)
					{
						/* debug
						consoleWrite("Child: " + QString::number(i), children[i]->getGlobalPosition());
						*/

						tmpDistance = abs(children[i]->getGlobalPosition().distance(worldHitPos));
						if (tmpDistance < minDistance)
						{
							minDistance = tmpDistance;
							minDistanceIndex = i;
						}
						if (children[i] == actor->getObjectToMimic())
							pickedActorIndex = i;
					}

					/* debug
					consoleWrite("worldHitPos", worldHitPos);					
					consoleWrite(QString("pile index: %1/%2, %3\n").arg(minDistanceIndex).arg(pickedActorIndex).arg(minDistance));
					*/

					// set this at the potential drop point
					if (!actor->getObjectToMimic()->getParent())
					{
						indexOut = -1;
						*pileOut = pile;
					}
					else
					{
						if ((minDistanceIndex > pickedActorIndex) || (actor->getObjectToMimic()->getParent() != pile))
							indexOut = minDistanceIndex + 1;	// we do this to move it on the index of the one dropped
						else
							indexOut = minDistanceIndex;
					}
					*pileOut = pile;
					drawPoint = children[minDistanceIndex]->getGlobalPosition();
				}				
			}
		}
	}

	// render a preview of the item at that index
	// TODO: draw a little stack of items if there are multiple ones?
	if (render && (indexOut > -2))
	{
		float multiplier = 5.0f;
		Vec3 dims = actor->getDims();
		dims.normalize();
#ifdef DXRENDER
		dxr->renderSideLessBox(drawPoint, actor->getGlobalOrientation(), dims * multiplier, actor->getTextureNum());
#else
		glPushAttribToken token(GL_ENABLE_BIT);
		glEnable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
		glBindTexture(GL_TEXTURE_2D, actor->getTextureNum());
		glColor4f(1,1,1,0.5f);

		ShapeVis::renderSideLessBox(drawPoint, actor->getGlobalOrientation(), dims * multiplier);
#endif
	}
}

bool getStickyNote(QString filepath, QString * postItTextOut)
{
	// Check if the postItNote has fileName StickyNote
	QString stickyNoteFileName = "StickyNote";
	if (!filename(filepath).contains(stickyNoteFileName, Qt::CaseInsensitive))
		return false;

	// read the post-it
	bool firstLine = true;
	QString postItHeader("BUMPTOP_STICKY_NOTE");

	// ensure it starts with a sticky note header
	if (!validate_file_startswith_utf8(filepath, postItHeader))
		return false;

	QString line;
	QString sstr = read_file_utf8(filepath);
	if (!sstr.startsWith(postItHeader))
		return false;

	// copy out the text if desired
	if (postItTextOut)
	{
		if (!sstr.isEmpty())
		{
			*postItTextOut = sstr.mid(postItHeader.size()).trimmed();
		}
	}

	return !sstr.isEmpty();
}

void writeStickyNote(QString filepath, QString text)
{
	QString postItHeader("BUMPTOP_STICKY_NOTE");
	QString postItText = postItHeader + text;
	write_file_utf8(postItText, filepath);
}

BumpTopStrings::BumpTopStrings()
{
	_strs.insert("LeafStackedPiles", QT_TR_NOOP("You can only leaf through Stacked piles!"));
	_strs.insert("EditStickyNote", QT_TR_NOOP("Sticky Note\n \n-Double-click\n to edit"));
	_strs.insert("PhotoFrameExists", QT_TR_NOOP("The photo frame you want already exists!"));
	_strs.insert("ForwardDriversPage", QT_TR_NOOP("Forwarding you to BumpTop Drivers page"));
	_strs.insert("ShowDesktopNotification", QT_TR_NOOP("BumpTop Revealed"));
	_strs.insert("StickyNoteLimit", QT_TR_NOOP("Creating more than two sticky notes is a PRO feature"));
} 

QString BumpTopStrings::getString( QString key )
{
	QHash<QString, QString>::const_iterator iter = _strs.find(key);
	if (iter != _strs.end())
		return iter.value();
	assert(false);
	return QString();
}
