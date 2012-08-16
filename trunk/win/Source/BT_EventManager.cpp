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
#include "BT_AnimationManager.h"
#include "BT_Authorization.h"
#include "BT_AutomatedDemo.h"
#include "BT_Camera.h"
#include "BT_CustomActor.h"
#include "BT_DragDrop.h"
#include "BT_EventManager.h"
#include "BT_FileSystemManager.h"
#include "BT_GLTextureManager.h"
#include "BT_KeyboardManager.h"
#include "BT_LassoMenu.h"
#include "BT_Logger.h"
#include "BT_MarkingMenu.h"
#include "BT_MouseEventManager.h"
#include "BT_OverlayComponent.h"
#include "BT_PhotoFrameActor.h"
#include "BT_Profiler.h"
#include "BT_RenderManager.h"
#include "BT_SceneManager.h"
#include "BT_Selection.h"
#include "BT_SlideShow.h"
#include "BT_StatsManager.h"
#include "BT_Stopwatch.h"
#include "BT_TextManager.h"
#include "BT_Util.h"
#include "BT_UndoStack.h"
#include "BT_WindowsOS.h"
#include "BT_WebActor.h"
#include "BumpTop.h"
#include "BT_SystemTray.h"
#include "BT_LibraryManager.h"

// -----------------------------------------------------------------

EventManager::EventManager() :
	pShutdownBlockReasonCreate(NULL),
	pShutdownBlockReasonDestroy(NULL),
	pShutdownBlockReasonQuery(NULL),
	_hLibModule(NULL),
	_supportsShutdownBlock(false),
	_remoteConnectionActive(false),
	oldACPowerStatus(EventManager::UnknownPower),
	exitFlag(false),
	asyncExitFlag(false),
	isInsideApp(true),
	isSuspended(false),
	hWnd(NULL),
	ignoreMessages(false),
	renderOnFinishedBinding(false),
	exitAfterMSTimeout(0),
	saveMSTimeout(0),
	_qtGuiWndProc(NULL)
{
	_hLibModule = LoadLibrary(_T("User32.dll"));

	if (_hLibModule)
	{
		pShutdownBlockReasonCreate = (ShutdownBlockReasonCreateSignature) GetProcAddress(_hLibModule, "ShutdownBlockReasonCreate");
		pShutdownBlockReasonDestroy = (ShutdownBlockReasonDestroySignature) GetProcAddress(_hLibModule, "ShutdownBlockReasonDestroy");
		pShutdownBlockReasonQuery = (ShutdownBlockReasonQuerySignature) GetProcAddress(_hLibModule, "ShutdownBlockReasonQuery");
	}

	if (pShutdownBlockReasonCreate && 
		pShutdownBlockReasonDestroy && 
		pShutdownBlockReasonQuery)
	{
		_supportsShutdownBlock = true;
	}
}

EventManager::~EventManager()
{
	if (_hLibModule)
	{
		FreeLibrary(_hLibModule);
	}
}

BOOL tryPeekMessage(
    __out LPMSG lpMsg,
    __in_opt HWND hWnd,
    __in UINT wMsgFilterMin,
    __in UINT wMsgFilterMax,
    __in UINT wRemoveMsg)
{
	__try
	{
		return PeekMessage(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		logger->logException(__FILE__, __LINE__, GetExceptionCode());
		return FALSE;
	}
}

void EventManager::mainLoop()
{
	MSG uMsg = {0};

	// show the window
	hWnd = winOS->GetWindowsHandle();
	UpdateWindow(hWnd);

#ifdef USE_QPC
	// ensure that the main thread is always running on the same processor
	// for consistency in timing (especially with the QueryPerformance* calls)
	// see: http://jongampark.wordpress.com/2008/04/26/how-to-solve-weirdness-of-the-high-resolution-counter/
	DWORD newProcessAffinityMask = 0x01;
	SetProcessAffinityMask(GetCurrentProcess(), (DWORD_PTR) &newProcessAffinityMask);
#endif

	// alter the timeGetTime() resolution timeBeginPeriod also affects Sleep()
	timeBeginPeriod(1);

	// timing information
#ifdef USE_QPC
	LARGE_INTEGER frequency;
	LARGE_INTEGER beginFrameCount /*, endFrameCount */;
	LARGE_INTEGER endMessageHandlingCount, endRenderCount;
	LARGE_INTEGER previousPhysicsCount, currentPhysicsCount;
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&previousPhysicsCount);
#else
	DWORD beginFrameCount;
	DWORD endRenderCount;
	DWORD currentPhysicsCount;
	DWORD previousPhysicsCount = timeGetTime();
#endif
	int msCountToNextFrame = 0;
	int msFrame = 0;
	int numFramesPerSec = 60;
	int msElapsedSinceFrameBegan = 0;
	int msElapsedSinceLastPhysics = 0;
	float msAverageElapsedSinceLastPhysics = 1;
	long long averageElapsedSinceLastPhysicsCount = 1;
	int msInitialAllottedPerFrame = 1000 / numFramesPerSec; // initially, 60fps or 16ms/frame
	int msRemainingPerFrame = 0;
	int msTimerPhysicsMaxStep = (msInitialAllottedPerFrame / 2);
	int msTimerCurStep = 0;
	int msTimerPhysicsCurStep = 0;
	int msTimerCount = 0;
	int msTimerPhysicsCount = 0;
	bool continueCustomExecution = true;
	assert(msInitialAllottedPerFrame % 2 == 0);

	while (!exitFlag) 
	{
		// start the timer for this frame
#ifdef USE_QPC
		QueryPerformanceCounter(&beginFrameCount);
#else
		beginFrameCount = timeGetTime();
#endif

		// handle windows messages
		continueCustomExecution = true;
		if (tryPeekMessage(&uMsg, NULL, 0, 0, PM_REMOVE | PM_NOYIELD) > 0)
		{
			if (uMsg.message == WM_QUIT)
				break;
			TranslateMessage(&uMsg);
			DispatchMessage(&uMsg);
			continueCustomExecution = false;
		}		
#ifdef USE_QPC
		QueryPerformanceCounter(&endMessageHandlingCount);		
#endif

		if (continueCustomExecution)
		{
			// handle physics
#ifdef USE_QPC
			QueryPerformanceCounter(&currentPhysicsCount);		
			msElapsedSinceLastPhysics = (int)(((currentPhysicsCount.QuadPart - previousPhysicsCount.QuadPart) * 1000.0) / frequency.QuadPart);
#else
			currentPhysicsCount = timeGetTime();
			msElapsedSinceLastPhysics = (currentPhysicsCount - previousPhysicsCount);
#endif
			{				
				// update the physics
				msTimerCount = msCountToNextFrame;
				msFrame = NxMath::min((int) msAverageElapsedSinceLastPhysics, msElapsedSinceLastPhysics);
				msCountToNextFrame = roundToNext(msFrame, msInitialAllottedPerFrame) - 	msFrame;
				while (msTimerCount < msFrame)
				{					
					msTimerCurStep = NxMath::min(msElapsedSinceLastPhysics - msTimerCount, msInitialAllottedPerFrame);

					// make pre-physics, per-render calls
					prePhysicsTimerCallback();

					msTimerPhysicsCount = 0;
					while (msTimerPhysicsCount < msTimerCurStep)
					{
						msTimerPhysicsCurStep = NxMath::min(msTimerCurStep - msTimerPhysicsCount, msTimerPhysicsMaxStep);
						physicsTimerCallback(msTimerPhysicsCurStep);
						msTimerPhysicsCount += msTimerPhysicsCurStep;
					}
					
					// make post-physics, per render calls
					postPhysicsTimerCallback(msTimerCurStep);
						
					msTimerCount += msTimerCurStep;
#ifdef USE_QPC
					QueryPerformanceCounter(&previousPhysicsCount);	
#else
					previousPhysicsCount = timeGetTime();
#endif
				}
				onUpdate();

				// update the average elapsed ms and trim the count if necessary
				msAverageElapsedSinceLastPhysics = ((averageElapsedSinceLastPhysicsCount * msAverageElapsedSinceLastPhysics) + msElapsedSinceLastPhysics) / float(averageElapsedSinceLastPhysicsCount + 1);
				++averageElapsedSinceLastPhysicsCount;
				if (averageElapsedSinceLastPhysicsCount > (2 << 24))
					averageElapsedSinceLastPhysicsCount -= (averageElapsedSinceLastPhysicsCount >> 1);
			}
			
			// render if necessary
			if (isRenderRequired())
				onRender();
#ifdef USE_QPC
			QueryPerformanceCounter(&endRenderCount);
#else
			endRenderCount = timeGetTime();
#endif

			// sleep for the rest of the frame if possible
			// according to http://www.geisswerks.com/ryan/FAQS/timing.html, this will 
			// give you a higher precision Sleep() than passing in the larger value of
			// msRemainingPerFrame.
#ifdef USE_QPC
			msElapsedSinceFrameBegan = (int)(((endRenderCount.QuadPart - beginFrameCount.QuadPart) * 1000.0) / frequency.QuadPart);
#else
			msElapsedSinceFrameBegan = (endRenderCount - beginFrameCount);
#endif
			msRemainingPerFrame = msInitialAllottedPerFrame - msElapsedSinceFrameBegan;
			while (msRemainingPerFrame > 0)
			{
				Sleep(1);
#ifdef USE_QPC
				QueryPerformanceCounter(&endRenderCount);
				msElapsedSinceFrameBegan = (int)(((endRenderCount.QuadPart - beginFrameCount.QuadPart) * 1000.0) / frequency.QuadPart);
#else
				endRenderCount = timeGetTime();
				msElapsedSinceFrameBegan = (endRenderCount - beginFrameCount);
#endif
				msRemainingPerFrame = msInitialAllottedPerFrame - msElapsedSinceFrameBegan;				
			}
		}
	}

	// reset the timeGetTime resolution
	timeEndPeriod(1);

	// Somewhat of a hack: for demo and prototype purposes, we sometimes create
	// a child BumpTop process. Probably the right thing to do is to do have the
	// child periodically ping the parent process via IPC, and if the parent
	// process stops responding, the child could shut itself down. This will
	// work for now though.
	if (scnManager->childProcess)
	{
		scnManager->childProcess->kill();
		SAFE_DELETE(scnManager->childProcess);
	}

	if (asyncExitFlag)
		winOS->ExitBumpTop();
}


LRESULT EventManager::messageProc(HWND Hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// -------------------------------------------------------------------------
	// NOTE: Do Not put code here that is longer then two lines of code. The 
	//       purpose of this is to be clean. If you need to put complex logic
	//       based off an event, create a function for it.
	// -------------------------------------------------------------------------

	// Ignore messages if we are exiting
	if (exitFlag) return DefWindowProc(Hwnd, uMsg, wParam, lParam);
	if (ignoreMessages) return DefWindowProc(Hwnd, uMsg, wParam, lParam);
	if (exitAfterMSTimeout) return DefWindowProc(Hwnd, uMsg, wParam, lParam);

	switch (uMsg)
	{
		case WM_TIMER:
			onTimer(wParam);
			break;

		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
			onMouseDown(uMsg, wParam, lParam);
			break;

		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
			onMouseUp(uMsg, wParam, lParam);
			break;

		case WM_MOUSEMOVE:
			onMouseMove(wParam, lParam);
			break;

		case WM_MOUSEWHEEL:
			onMouseUp(uMsg, wParam, lParam);
			break;

		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_IME_CHAR:
		case WM_CHAR:
			{
#ifdef ENABLE_WEBKIT
				vector<BumpObject *> objs = scnManager->getBumpObjects(ObjectType(BumpActor, Webpage));
				for (int i = 0; i < objs.size(); ++i)
				{
					WebActor * actor = (WebActor *) objs[i];
					if (actor->isFocused())
					{
						return forwardEvent(actor->getViewWidget(), uMsg, wParam, lParam);
					}
				}
#endif
			}
			break;

		case WM_KEYDOWN:
			{
#ifdef ENABLE_WEBKIT
				if (KeyEscape != wParam)
				{
					vector<BumpObject *> objs = scnManager->getBumpObjects(ObjectType(BumpActor, Webpage));
					for (int i = 0; i < objs.size(); ++i)
					{
						WebActor * actor = (WebActor *) objs[i];
						if (actor->isFocused())
						{
							if (KeyF5 == wParam)
							{	
								actor->reload();
								return true;
							}
							else
								return forwardEvent(actor->getViewWidget(), uMsg, wParam, lParam);
						}
					}
				}
#endif
			}
			onKeyDown(wParam);
			break;

		case WM_KEYUP:
			{
#ifdef ENABLE_WEBKIT
				if (KeyEscape != wParam)
				{
					vector<BumpObject *> objs = scnManager->getBumpObjects(ObjectType(BumpActor, Webpage));
					for (int i = 0; i < objs.size(); ++i)
					{
						WebActor * actor = (WebActor *) objs[i];
						if (actor->isFocused())
						{
							return forwardEvent(actor->getViewWidget(), uMsg, wParam, lParam);
						}
					}
				}
#endif
			}
			onKeyUp(wParam);
			break;

		case WM_TABLET_FLICK:
			{
				FLICK_DATA * fd = (FLICK_DATA *) &wParam;
				FLICK_POINT * fp = (FLICK_POINT *) &lParam;
				onFlick(NxMath::abs(fd->iFlickDirection), Vec3((float)fp->x, (float)fp->y, 0));
				return FLICK_WM_HANDLED_MASK;
			}

		case WM_PAINT:
			onRender();
			ValidateRect(hWnd, NULL);
			break;

		case WM_ERASEBKGND:
			return TRUE;

		case WM_POWERBROADCAST:
			switch (wParam)
			{			
				// This event can occur when battery life drops to less than 5 minutes, 
				// or when the percentage of battery life drops below 10 percent, or if the battery life changes by 3 percent.
				case PBT_APMPOWERSTATUSCHANGE:
					onPowerStatusChange();
					break;	
				case PBT_APMQUERYSUSPEND:
				case PBT_APMSUSPEND:
					onPowerSuspend();
					break;
				case PBT_APMQUERYSUSPENDFAILED:
				case PBT_APMRESUMEAUTOMATIC:
				case PBT_APMRESUMECRITICAL:
				case PBT_APMRESUMESUSPEND:
					onPowerResume();
					break;
			}
			break;

		case WM_QUERYENDSESSION:
			if (GLOBAL(saveSceneMutex).tryLock())
			{
				GLOBAL(saveSceneMutex).unlock();
				return true;
			}
			else
			{
				if (_supportsShutdownBlock)
					pShutdownBlockReasonCreate(winOS->GetWindowsHandle(), L"Saving your scene file...");			
				return true;
			}
			break;
		
		case WM_ENDSESSION:
			if (lParam != ENDSESSION_CLOSEAPP)
			{
				SaveSceneToFile();
				if (_supportsShutdownBlock)
					pShutdownBlockReasonDestroy(winOS->GetWindowsHandle());
			}

			ExitBumptop();
			exit(0);
			break;

		case WM_WTSSESSION_CHANGE:
			if (wParam == WTS_REMOTE_CONNECT)
			{
				onRDPConnect();
				// We don't want to render when a remote connection is detected,
				// so break here.
				break;
			}
			else if (wParam == WTS_REMOTE_DISCONNECT)
				onRDPDisconnect();
			
			rndrManager->invalidateRenderer();
			break;
		
		case WM_SETFOCUS:
			statsManager->startTimer(statsManager->getStats().bt.window.focusedTime);
			break;

		case WM_KILLFOCUS:
			statsManager->finishTimer(statsManager->getStats().bt.window.focusedTime);
			break;

		case WM_DEVICECHANGE:
			switch (wParam)
			{
			case DBT_DEVICEARRIVAL:
				onDeviceAdded((DEV_BROADCAST_HDR *) lParam);
				break;
			case DBT_DEVICEREMOVECOMPLETE:
				onDeviceRemoved((DEV_BROADCAST_HDR *) lParam);
				break;
			default:
				break;
			}
			break;

		case WM_LIBRARYCHANGE:
			{
				long eventId;
				PIDLIST_ABSOLUTE *pidlArray;
				HANDLE notifyLock = SHChangeNotification_Lock((HANDLE)wParam, (DWORD)lParam, &pidlArray, &eventId);
				if (notifyLock)
				{
					if (winOS->GetLibraryManager())
						winOS->GetLibraryManager()->onLibraryChangeEvent(eventId, pidlArray[0], pidlArray[1]);
					SHChangeNotification_Unlock(notifyLock);
				}

				// If the current library has been invalidated, request a new library
				// (either the same updated one, or a completely new one if the old 
				// one was deleted)
				QSharedPointer<Library> lib = scnManager->getCurrentLibrary();
				if (winOS->GetLibraryManager() && lib)
				{
					if (!lib->isValid())
					{
						QSharedPointer<Library> newLibrary = winOS->GetLibraryManager()->getLibraryByKey(lib->getHashKey());
						SwitchToLibrary(newLibrary);
					}
					else
					{
						LibraryMenuControl* libMenu = scnManager->getLibraryControl();
						if (libMenu)
							libMenu->show(lib);
					}
				}

			}
			break;

		case WM_DESTROY:
		case WM_CLOSE:
			ExitBumptop();
			return 1;
			break;
	}



	return DefWindowProc(Hwnd, uMsg, wParam, lParam);
}

void EventManager::postMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// Send a message to our message pump
	SendMessage(hWnd, uMsg, wParam, lParam);
}

void EventManager::checkMouseCursorPos()
{
	POINT cursor;
	RECT r;

	// Get current Position of mouse and window
	GetCursorPos(&cursor);
	GetWindowRect(winOS->GetWindowsHandle(), &r);

	if (isInsideApp)
	{
		// Mouse is inside the Application

		if ((WindowFromPoint(cursor) != hWnd) ||
			(cursor.x < r.left || cursor.x > r.right || cursor.y < r.top || cursor.y > r.bottom))
		{
			// Cursor is now outside the Application
			onMouseExitWindow();
			mouseManager->lastMouseButtons = mouseManager->mouseButtons;

			// initiate drag if possible
			if (sel->getPickedActor())
			{
				if (!scnManager->isShellExtension)
				{
					winOS->StartDragOnCurrentSelection();	
				}else{
					MessageClearPolicy clearPolicy;
						clearPolicy.setTimeout(3);
					scnManager->messages()->addMessage(new Message("ShellExt_StartDrag", QT_TR_NOOP("Press Ctrl, Alt, or Shift to initiate dragging out of BumpTop"), Message::Ok, clearPolicy));
				}
			}

			// Notify that the mouse exited the window
			onMouseExitWindow();
		}
	}else{
		// Mouse is outside the Application
		if (scnManager->isShellExtension)
		{
			// check if we are currently dragging
			if (!DragAndDrop::IsDragInProgress() &&
				(mouseManager->lastMouseButtons & MouseButtonLeft) &&
				sel->getPickedActor())
			{
				bool isShiftDown = (GetAsyncKeyState(KeyShift) & 0x8000);
				bool isCtrlDown = (GetAsyncKeyState(KeyControl) & 0x8000);
				bool isAltDown = (GetAsyncKeyState(KeyAlt) & 0x8000);

				bool isMove = isShiftDown;
				bool isCopy = isCtrlDown;
				bool isLink = (isShiftDown && isCtrlDown) || isAltDown;

				onMouseExitWindow();

				if (isMove || isCopy || isLink)
				{
					// we dragged out earlier and are currently only starting
					// the actual drag
					winOS->StartDragOnCurrentSelection();
				}
			}
		}

		if ((WindowFromPoint(cursor) == hWnd) ||
			(cursor.x >= r.left && cursor.x <= r.right && cursor.y >= r.top && cursor.y <= r.bottom))
		{
			// Mouse entered the window
			onMouseEnterWindow();
		}
	}
}

void EventManager::checkTextureLoaded()
{
	GLTextureObject obj;
	if (texMgr->loadQueuedTexture(obj))
	{
		if (obj.key == "floor.desktop.buffer") 
		{
			// Swapping the floor.desktop.buffer texture with the floor.desktop texture ensures that we
			// can see it because we only render the floor.desktop texture
			texMgr->swapLoadedTextureKeys("floor.desktop", "floor.desktop.buffer");
			texMgr->deleteTexture("floor.desktop.buffer");
		}
		else 
		{
			// loop through all the actors and find the ones that are tied to 
			// this texture
			vector<BumpObject *> bumpObjs = scnManager->getBumpObjects();
			for (int i = 0; i < bumpObjs.size(); ++i)
			{
				bool resizeActor = false;

				// only do this for actors
				if (!bumpObjs[i]->isBumpObjectType(BumpActor)) continue;

				Actor * actor = (Actor *) bumpObjs[i];
				if (actor->isActorType(FileSystem))
				{
					FileSystemActor * fsActor = (FileSystemActor *) actor;
					bool isThumbnail = fsActor->isFileSystemType(Thumbnail);
					bool isMatchingId = 
						(fsActor->getThumbnailID() == obj.key) || 
						(fsActor->getAlternateThumbnailId() == obj.key);
					resizeActor = isMatchingId;

					// make the texure event callback
					if (resizeActor)
					{
						if (obj.resultCode == NoError)
							fsActor->onTextureLoadComplete(obj.key);
						else
							fsActor->onTextureLoadError(obj.key);
					}

					// check if we have to swap the two keys (ie. when the
					// nameable has an icon)
					if (!isThumbnail && isMatchingId)
					{
						// only do this for actual files
						if (winOS->GetIconTypeFromFileName(fsActor->getTargetPath()) == -1)
						{
							// set the icon to be the thumbnail, and then set
							// the nameable image to be the icon
							QString tempID = fsActor->getTextureID();
							fsActor->setTextureID(obj.key);
							if (!fsActor->isFileSystemType(Folder) && 
								!fsActor->isFileSystemType(Link))
								fsActor->setTextIcon(tempID);
						}
					}
				}
				else if (actor->isActorType(Custom))
				{
					CustomActor * csActor = (CustomActor *) actor;
					resizeActor = (csActor->getTextureID() == obj.key);
				}

				// animate to the new size (depends on the thumbnail)
				if (resizeActor)
				{
					// make sure that if it is sizing, it finishes that first
					actor->finishAnimation();

					// resize the icons to be an aspect ratio image of the original size
					Vec3 curSize = actor->getDims();
					int width = obj.resultTextureData->dimensions.x;
					int height = obj.resultTextureData->dimensions.y;
					float biggest = curSize.x > curSize.y ? curSize.x : curSize.y;
					float aspect = curSize.x / curSize.y;
					float newAspect = (float)width/height;

					if (aspect >= (newAspect + 0.01) || 
						aspect <= (newAspect - 0.01))
					{
						// Determine whether to scale the x or the y (they are always bound by the largest side)
						Vec3 newSize(0.0f);
						if (newAspect > 1.0f)
							newSize.set(biggest, biggest/newAspect, curSize.z);
						else
							newSize.set(biggest*newAspect, biggest, curSize.z);

						// Animate the dimensions of the object to its new aspect ratio
						actor->setSizeAnim(curSize, newSize, 30);
					}

					/*
					// XXX
					if (cam->isWatchedActorHighlighted(actor))
					cam->rehighlightWatchedActor();
					*/
				}
			}

			textManager->invalidate();
		}

		// render
		rndrManager->invalidateRenderer();

		// Empty the working set so that we get a clear picture of the memory usage
		// after the textures have been passed through the system memory
		EmptyWorkingSet(GetCurrentProcess());
	}
}

void EventManager::checkSaveSceneRequired()
{
	// NOTE: we can't use event manager's exitFlag because the main loop depends on 
	// that being set to completely end the program (there may still be animations
	// running while GLOBAL(exitBumpTopFlag) is set, but not exitFlag)
	if (GLOBAL(exitBumpTopFlag) || isSuspended)
	{
		saveMSTimeout = 0;
		return;
	}

	const int minTimeBetweenSaves = 1000;
	const float creepEpsilon = 2.0f;

	if (saveMSTimeout > 0)
	{
		// save the scene if either a timeout has elapsed...
		bool timerHasElapsed = (saveTimeoutTimer.elapsed() > saveMSTimeout);
		
		// ... or no movement has been detected
		vector<BumpObject *> objs = scnManager->getBumpObjects();
		unsigned int size = objs.size();
		bool hasNoMovingObjects = true;
		for (int i = 0; hasNoMovingObjects && (i < size); ++i)
		{
			BumpObject * obj = objs[i];
			if (obj->isMoving() && timerHasElapsed)
			{
				// try and put this moving object to sleep if it is just creeping along 
				// for the timeout duration
				map<BumpObject *, Vec3>::iterator iter = potentialCreepingObjects.find(obj);
				if (iter != potentialCreepingObjects.end())
				{
					BumpObject * obj = iter->first;
					const Vec3& prevPos = iter->second;

					if (prevPos.distanceSquared(obj->getGlobalPosition()) <= creepEpsilon)
						obj->putToSleep();
					
					potentialCreepingObjects.erase(iter);
				}
			}
			hasNoMovingObjects = (!obj->isMoving() && !obj->isDragging() && !obj->isAnimating());
		}
		
		if (timerHasElapsed || (hasNoMovingObjects && saveTimeoutTimer.elapsed() > minTimeBetweenSaves))
		{
			SaveSceneToFile();
			saveMSTimeout = 0;
			saveTimeoutTimer.restart();
			
			// clear the set of potentially creeping objects
			potentialCreepingObjects.clear();
		}
	}
	else 
	{
		const vector<BumpObject *>& objs = scnManager->getBumpObjects();
		unsigned int size = objs.size();
		for (int i = 0; i < size; ++i)
		{
			BumpObject * obj = objs[i];
			// initiate a save-to-be if there is an object that is moving via 
			// physics?
			if (obj->isMoving() && !obj->isDragging() && !obj->isAnimating())
			{
				// reset the save timeout timer, and set the timeout to 2.5 seconds
				saveTimeoutTimer.restart();
				saveMSTimeout = 2000;

				// add the item to the set of objects to be checked for creeping
				potentialCreepingObjects.insert(make_pair(obj, obj->getGlobalPosition()));
			}
		}
	}
}

void EventManager::onTimer(uint timerID)
{
	// Call legacy code
	winOS->OnTimer(timerID);
}

void EventManager::onUpdate()
{
	// save the scene when necessary
	checkSaveSceneRequired();

	// Keep track of where the cursor is relative to our window
	checkMouseCursorPos();
	checkTextureLoaded(); // REFACTOR: Eventually use onTextureLoaded()

	// If we are exiting and BumpTop is still visible, hide it until exit
	if (!GLOBAL(isShellExtension) && GLOBAL(exitBumpTopFlag) && !animManager->isAnimating())
	{
		// Force Hide
		ShowWindow(hWnd, SW_HIDE);
	}
}

void EventManager::onRender()
{
	GLOBAL(Elapsed).unpause(); // we're rendering frames, keep timer going if necessary

	// Call legacy code
	winOS->Render();
}

void EventManager::onKeyDown(uint keyVal)
{
	// Call legacy code
	winOS->OnKeyDown(keyVal);
	interruptIdleTimer();
}

void EventManager::onKeyUp(uint keyVal)
{
	// Call legacy code
	winOS->OnKeyUp(keyVal);
	interruptIdleTimer();
}

void EventManager::onMouseDown(UINT msg, WPARAM wParam, LPARAM lParam)
{
	POINT p;
	p.x = GET_X_LPARAM(lParam);
	p.y = GET_Y_LPARAM(lParam);
	winOS->OnMouse(msg, p.x, p.y, wParam);
}

void EventManager::onMouseUp(UINT msg, WPARAM wParam, LPARAM lParam)
{
	POINT p;
	p.x = GET_X_LPARAM(lParam);
	p.y = GET_Y_LPARAM(lParam);
	winOS->OnMouse(msg, p.x, p.y, wParam);
}

void EventManager::onMouseMove(WPARAM wParam, LPARAM lParam)
{
	POINT p;
	p.x = GET_X_LPARAM(lParam);
	p.y = GET_Y_LPARAM(lParam);
	winOS->OnMouseMove(p.x, p.y, wParam);
}

void EventManager::onMouseEnterWindow()
{
	if (isInsideApp)
		return;
	isInsideApp = true;

	SHORT leftButtonState = GetSystemMetrics(SM_SWAPBUTTON) ? GetAsyncKeyState(VK_RBUTTON) : GetAsyncKeyState(VK_LBUTTON);

	if (!(leftButtonState & 0x8000)) // ie. if left mouse button is not down
	{
		//// Release the mouse button
		unpick();
		lassoMenu->reset();
	}

	// Reset the flag variables
	mouseManager->lastMouseButtons = 0;
}

void EventManager::onMouseExitWindow()
{
	isInsideApp = false;
}

void EventManager::setExitFlag(bool exitNow)
{
	exitFlag = exitNow;
}

void EventManager::setAsyncExitFlag(bool asyncExit)
{
	asyncExitFlag = asyncExit;
}

bool EventManager::isRenderRequired()
{	
	GlobalSettings& settings = GLOBAL(settings);
	bool reqRenderReason = settings.drawFramesPerSecond;	

	// Check for specific conditionals
	// NOTE: the logic is expanded and we return early since this is a tight loop
	//		 that gets called ALOT
	if (reqRenderReason)
		renderReason.clear();
	if (!settings.useCPUOptimizations) 
	{ 
		if (reqRenderReason)
			renderReason = "Optimizations Off"; 
		return true; 
	}
	if (settings.enableMultimouse) 
	{ 
		if (reqRenderReason)
			renderReason = "Multi-Touch On"; 
		return true; 
	}
	if (rndrManager->isRenderRequired()) 
	{ 
		if (reqRenderReason)
			renderReason = "RndrManager said so"; 
		return true; 
	}
	if (GLOBAL(isInteraction)) 
	{
		if (reqRenderReason)
			renderReason = "Interaction"; 
		return true; 
	}
	if (markingMenu->isAnimating()) 
	{ 
		if (reqRenderReason)
			renderReason = "markingMenu->isAnimating()"; 
		return true; 
	}
	if (animManager->isAnimating())	
	{ 
		if (reqRenderReason)
			renderReason = QString::number(animManager->getNumObjsAnimating()) + " Objs Animating"; 
		return true; 
	}
	
	// Nothing was updated that requires a render
	// Movement check has moved to after physics update, which will invalidate accordingly
	return false;
}


EventManager::PowerStatus EventManager::getACPowerStatus()
{
	SYSTEM_POWER_STATUS sps;
	BOOL ret;

	// Get the System power status
	ret = GetSystemPowerStatus(&sps);

	if (ret)
	{
		// 0 = offline, 1 = online, 255 = unknownstatus
		if (sps.ACLineStatus == 0)
			return EventManager::Unplugged;
		else if (sps.ACLineStatus == 1)
			return EventManager::PluggedIn;
	}

	return EventManager::UnknownPower;
}

bool EventManager::isExiting()
{
	return exitFlag;
}

void EventManager::forceLoopUpdate()
{
	MSG uMsg;

	ignoreMessages = true;

	// Force a loop of all messages to our system, ignoring them all together
	while (PeekMessage(&uMsg, NULL, 0, 0, PM_REMOVE) == TRUE)
	{
		LOG_START(formatMessageDescription(uMsg));
		TranslateMessage(&uMsg);
		DispatchMessage(&uMsg);
		LOG_FINISH(formatMessageDescription(uMsg));
	}

	ignoreMessages = false;
}

void EventManager::onPowerSuspend()
{
	if (isSuspended)
		return;
	isSuspended = true;

	/*
	// stop all the stats from uploading
	statsManager->onPowerSuspend();

	// suspend all photoframe actors
	vector<FileSystemActor *> photoFrames = scnManager->getFileSystemActors(PhotoFrame, false);
	for (int i = 0; i < photoFrames.size(); ++i)
	{
		PhotoFrameActor * pfActor = (PhotoFrameActor *) photoFrames[i];
		pfActor->onPowerSuspend();
	}
	*/

	// let the texture manager know to suspend loading of textures
	texMgr->onPowerSuspend();

	// unwatch all the directories
	fsManager->onPowerSuspend();

	// interrupt the idle timer
	interruptIdleTimer();
}

void EventManager::onPowerResume()
{
	if (!isSuspended)
		return;
	isSuspended = false;

	// let the texture manager know to resume loading of textures
	texMgr->onPowerResume();

	// re-watch all the directories
	fsManager->onPowerResume();

	/*
	// resume stats uploading
	statsManager->onPowerResume();

	// resume all photoframe actors
	vector<FileSystemActor *> photoFrames = scnManager->getFileSystemActors(PhotoFrame, false);
	for (int i = 0; i < photoFrames.size(); ++i)
	{
		PhotoFrameActor * pfActor = (PhotoFrameActor *) photoFrames[i];
		pfActor->onPowerResume();
	}
	*/

	// force render
	rndrManager->invalidateRenderer();

	// interrupt the idle timer
	interruptIdleTimer();
}

void EventManager::onPowerStatusChange()
{
	bool onAC = getACPowerStatus();
	if (GLOBAL(settings).disablePhotoframesOnBattery)
	{
		vector<FileSystemActor *> photoFrames;
		photoFrames = scnManager->getFileSystemActors(PhotoFrame, false);
		if(onAC && !oldACPowerStatus) // Switching from Battery power to AC power
		{
			// We are on AC power so enable all photo frames
			for (int i = 0; i < photoFrames.size(); i++)
			{
				PhotoFrameActor * pfActor;
				pfActor = (PhotoFrameActor *) photoFrames[i];
				if(isSlideshowModeActive()) {
					if(!(photoFrames[i] == cam->getHighlightedWatchedActor())) {//this frame is not being watched
						pfActor->enableUpdate();
					}
				}
				else if(!pfActor->isUpdating()) {
					pfActor->enableUpdate();
				}
			}	
		}
		else if (!onAC && oldACPowerStatus) // Switching from AC to Battery power
		{
			// We are on battery power so disable all photo frames
			for (int i = 0; i < photoFrames.size(); i++)
			{
				PhotoFrameActor * pfActor;
				pfActor = (PhotoFrameActor *) photoFrames[i];
				if(pfActor->isUpdating())
					pfActor->disableUpdate();

			}
		}
	}
	oldACPowerStatus = getACPowerStatus();
}

void EventManager::forceExitAfterTimeout(unsigned int millis)
{
	exitAfterMSTimeout = millis;
	exitAfterTimeoutTimer.restart();
}

void EventManager::interruptIdleTimer()
{
	idleTimer.setCondition(false);
}

/*
This function generated by copying the table from
http://wiki.winehq.org/List_Of_Windows_Messages
into a text file and running the following Python script:

messages_file = file('WM.txt', 'r')
messages_file_contents = messages_file.read().strip().replace('\n\t\n\n\t\n\n', ' ')
messages_file.close()

messages = [message.split() for message in messages_file_contents.split('\n\n')]
messages = [(int(message[1]), message[2]) for message in messages]

messages_dict = dict(messages) # will clobber some messages that share the same ID

print """
string GetMessageName(UINT message)
{
switch(message)
{"""

for message_id, message_name in messages_dict.iteritems():
print '		case %d: return string("%s");' % (message_id, message_name)

print """
default: return string("");
}
}"""


*/




QString GetMessageName(UINT message)
{
	switch(message)
	{
	case 0: return QString(QT_NT("WM_NULL"));
	case 1: return QString(QT_NT("WM_CREATE"));
	case 2: return QString(QT_NT("WM_DESTROY"));
	case 3: return QString(QT_NT("WM_MOVE"));
	case 5: return QString(QT_NT("WM_SIZE"));
	case 6: return QString(QT_NT("WM_ACTIVATE"));
	case 7: return QString(QT_NT("WM_SETFOCUS"));
	case 8: return QString(QT_NT("WM_KILLFOCUS"));
	case 10: return QString(QT_NT("WM_ENABLE"));
	case 11: return QString(QT_NT("WM_SETREDRAW"));
	case 12: return QString(QT_NT("WM_SETTEXT"));
	case 13: return QString(QT_NT("WM_GETTEXT"));
	case 14: return QString(QT_NT("WM_GETTEXTLENGTH"));
	case 15: return QString(QT_NT("WM_PAINT"));
	case 16: return QString(QT_NT("WM_CLOSE"));
	case 17: return QString(QT_NT("WM_QUERYENDSESSION"));
	case 18: return QString(QT_NT("WM_QUIT"));
	case 19: return QString(QT_NT("WM_QUERYOPEN"));
	case 20: return QString(QT_NT("WM_ERASEBKGND"));
	case 21: return QString(QT_NT("WM_SYSCOLORCHANGE"));
	case 22: return QString(QT_NT("WM_ENDSESSION"));
	case 24: return QString(QT_NT("WM_SHOWWINDOW"));
	case 25: return QString(QT_NT("WM_CTLCOLOR"));
	case 26: return QString(QT_NT("WM_WININICHANGE"));
	case 27: return QString(QT_NT("WM_DEVMODECHANGE"));
	case 28: return QString(QT_NT("WM_ACTIVATEAPP"));
	case 29: return QString(QT_NT("WM_FONTCHANGE"));
	case 30: return QString(QT_NT("WM_TIMECHANGE"));
	case 31: return QString(QT_NT("WM_CANCELMODE"));
	case 32: return QString(QT_NT("WM_SETCURSOR"));
	case 33: return QString(QT_NT("WM_MOUSEACTIVATE"));
	case 34: return QString(QT_NT("WM_CHILDACTIVATE"));
	case 35: return QString(QT_NT("WM_QUEUESYNC"));
	case 36: return QString(QT_NT("WM_GETMINMAXINFO"));
	case 38: return QString(QT_NT("WM_PAINTICON"));
	case 39: return QString(QT_NT("WM_ICONERASEBKGND"));
	case 40: return QString(QT_NT("WM_NEXTDLGCTL"));
	case 42: return QString(QT_NT("WM_SPOOLERSTATUS"));
	case 43: return QString(QT_NT("WM_DRAWITEM"));
	case 44: return QString(QT_NT("WM_MEASUREITEM"));
	case 45: return QString(QT_NT("WM_DELETEITEM"));
	case 46: return QString(QT_NT("WM_VKEYTOITEM"));
	case 47: return QString(QT_NT("WM_CHARTOITEM"));
	case 48: return QString(QT_NT("WM_SETFONT"));
	case 49: return QString(QT_NT("WM_GETFONT"));
	case 50: return QString(QT_NT("WM_SETHOTKEY"));
	case 51: return QString(QT_NT("WM_GETHOTKEY"));
	case 55: return QString(QT_NT("WM_QUERYDRAGICON"));
	case 57: return QString(QT_NT("WM_COMPAREITEM"));
	case 61: return QString(QT_NT("WM_GETOBJECT"));
	case 65: return QString(QT_NT("WM_COMPACTING"));
	case 68: return QString(QT_NT("WM_COMMNOTIFY"));
	case 70: return QString(QT_NT("WM_WINDOWPOSCHANGING"));
	case 71: return QString(QT_NT("WM_WINDOWPOSCHANGED"));
	case 72: return QString(QT_NT("WM_POWER"));
	case 74: return QString(QT_NT("WM_COPYDATA"));
	case 75: return QString(QT_NT("WM_CANCELJOURNAL"));
	case 78: return QString(QT_NT("WM_NOTIFY"));
	case 80: return QString(QT_NT("WM_INPUTLANGCHANGEREQUEST"));
	case 81: return QString(QT_NT("WM_INPUTLANGCHANGE"));
	case 82: return QString(QT_NT("WM_TCARD"));
	case 83: return QString(QT_NT("WM_HELP"));
	case 84: return QString(QT_NT("WM_USERCHANGED"));
	case 85: return QString(QT_NT("WM_NOTIFYFORMAT"));
	case 123: return QString(QT_NT("WM_CONTEXTMENU"));
	case 124: return QString(QT_NT("WM_STYLECHANGING"));
	case 125: return QString(QT_NT("WM_STYLECHANGED"));
	case 126: return QString(QT_NT("WM_DISPLAYCHANGE"));
	case 127: return QString(QT_NT("WM_GETICON"));
	case 128: return QString(QT_NT("WM_SETICON"));
	case 129: return QString(QT_NT("WM_NCCREATE"));
	case 130: return QString(QT_NT("WM_NCDESTROY"));
	case 131: return QString(QT_NT("WM_NCCALCSIZE"));
	case 132: return QString(QT_NT("WM_NCHITTEST"));
	case 133: return QString(QT_NT("WM_NCPAINT"));
	case 134: return QString(QT_NT("WM_NCACTIVATE"));
	case 135: return QString(QT_NT("WM_GETDLGCODE"));
	case 136: return QString(QT_NT("WM_SYNCPAINT"));
	case 8217: return QString(QT_NT("OCM_CTLCOLOR"));
	case 160: return QString(QT_NT("WM_NCMOUSEMOVE"));
	case 161: return QString(QT_NT("WM_NCLBUTTONDOWN"));
	case 162: return QString(QT_NT("WM_NCLBUTTONUP"));
	case 163: return QString(QT_NT("WM_NCLBUTTONDBLCLK"));
	case 164: return QString(QT_NT("WM_NCRBUTTONDOWN"));
	case 165: return QString(QT_NT("WM_NCRBUTTONUP"));
	case 166: return QString(QT_NT("WM_NCRBUTTONDBLCLK"));
	case 167: return QString(QT_NT("WM_NCMBUTTONDOWN"));
	case 168: return QString(QT_NT("WM_NCMBUTTONUP"));
	case 169: return QString(QT_NT("WM_NCMBUTTONDBLCLK"));
	case 171: return QString(QT_NT("WM_NCXBUTTONDOWN"));
	case 172: return QString(QT_NT("WM_NCXBUTTONUP"));
	case 173: return QString(QT_NT("WM_NCXBUTTONDBLCLK"));
	case 255: return QString(QT_NT("WM_INPUT"));
	case 256: return QString(QT_NT("WM_KEYFIRST"));
	case 257: return QString(QT_NT("WM_KEYUP"));
	case 258: return QString(QT_NT("WM_CHAR"));
	case 259: return QString(QT_NT("WM_DEADCHAR"));
	case 260: return QString(QT_NT("WM_SYSKEYDOWN"));
	case 261: return QString(QT_NT("WM_SYSKEYUP"));
	case 262: return QString(QT_NT("WM_SYSCHAR"));
	case 263: return QString(QT_NT("WM_SYSDEADCHAR"));
	case 264: return QString(QT_NT("WM_KEYLAST"));
	case 265: return QString(QT_NT("WM_WNT_CONVERTREQUESTEX"));
	case 266: return QString(QT_NT("WM_CONVERTREQUEST"));
	case 267: return QString(QT_NT("WM_CONVERTRESULT"));
	case 268: return QString(QT_NT("WM_INTERIM"));
	case 269: return QString(QT_NT("WM_IME_STARTCOMPOSITION"));
	case 270: return QString(QT_NT("WM_IME_ENDCOMPOSITION"));
	case 271: return QString(QT_NT("WM_IME_KEYLAST"));
	case 272: return QString(QT_NT("WM_INITDIALOG"));
	case 273: return QString(QT_NT("WM_COMMAND"));
	case 274: return QString(QT_NT("WM_SYSCOMMAND"));
	case 275: return QString(QT_NT("WM_TIMER"));
	case 276: return QString(QT_NT("WM_HSCROLL"));
	case 277: return QString(QT_NT("WM_VSCROLL"));
	case 278: return QString(QT_NT("WM_INITMENU"));
	case 279: return QString(QT_NT("WM_INITMENUPOPUP"));
	case 8239: return QString(QT_NT("OCM_CHARTOITEM"));
	case 287: return QString(QT_NT("WM_MENUSELECT"));
	case 288: return QString(QT_NT("WM_MENUCHAR"));
	case 289: return QString(QT_NT("WM_ENTERIDLE"));
	case 290: return QString(QT_NT("WM_MENURBUTTONUP"));
	case 291: return QString(QT_NT("WM_MENUDRAG"));
	case 292: return QString(QT_NT("WM_MENUGETOBJECT"));
	case 293: return QString(QT_NT("WM_UNINITMENUPOPUP"));
	case 294: return QString(QT_NT("WM_MENUCOMMAND"));
	case 295: return QString(QT_NT("WM_CHANGEUISTATE"));
	case 296: return QString(QT_NT("WM_UPDATEUISTATE"));
	case 297: return QString(QT_NT("WM_QUERYUISTATE"));
	case 306: return QString(QT_NT("WM_CTLCOLORMSGBOX"));
	case 307: return QString(QT_NT("WM_CTLCOLOREDIT"));
	case 308: return QString(QT_NT("WM_CTLCOLORLISTBOX"));
	case 309: return QString(QT_NT("WM_CTLCOLORBTN"));
	case 310: return QString(QT_NT("WM_CTLCOLORDLG"));
	case 311: return QString(QT_NT("WM_CTLCOLORSCROLLBAR"));
	case 312: return QString(QT_NT("WM_CTLCOLORSTATIC"));
	case 8249: return QString(QT_NT("OCM_COMPAREITEM"));
	case 8270: return QString(QT_NT("OCM_NOTIFY"));
	case 512: return QString(QT_NT("WM_MOUSEMOVE"));
	case 513: return QString(QT_NT("WM_LBUTTONDOWN"));
	case 514: return QString(QT_NT("WM_LBUTTONUP"));
	case 515: return QString(QT_NT("WM_LBUTTONDBLCLK"));
	case 516: return QString(QT_NT("WM_RBUTTONDOWN"));
	case 517: return QString(QT_NT("WM_RBUTTONUP"));
	case 518: return QString(QT_NT("WM_RBUTTONDBLCLK"));
	case 519: return QString(QT_NT("WM_MBUTTONDOWN"));
	case 520: return QString(QT_NT("WM_MBUTTONUP"));
	case 521: return QString(QT_NT("WM_MOUSELAST"));
	case 522: return QString(QT_NT("WM_MOUSEWHEEL"));
	case 523: return QString(QT_NT("WM_XBUTTONDOWN"));
	case 524: return QString(QT_NT("WM_XBUTTONUP"));
	case 525: return QString(QT_NT("WM_XBUTTONDBLCLK"));
	case 528: return QString(QT_NT("WM_PARENTNOTIFY"));
	case 529: return QString(QT_NT("WM_ENTERMENULOOP"));
	case 530: return QString(QT_NT("WM_EXITMENULOOP"));
	case 531: return QString(QT_NT("WM_NEXTMENU"));
	case 532: return QString(QT_NT("WM_SIZING"));
	case 533: return QString(QT_NT("WM_CAPTURECHANGED"));
	case 534: return QString(QT_NT("WM_MOVING"));
	case 536: return QString(QT_NT("WM_POWERBROADCAST"));
	case 537: return QString(QT_NT("WM_DEVICECHANGE"));
	case 544: return QString(QT_NT("WM_MDICREATE"));
	case 545: return QString(QT_NT("WM_MDIDESTROY"));
	case 546: return QString(QT_NT("WM_MDIACTIVATE"));
	case 547: return QString(QT_NT("WM_MDIRESTORE"));
	case 548: return QString(QT_NT("WM_MDINEXT"));
	case 549: return QString(QT_NT("WM_MDIMAXIMIZE"));
	case 550: return QString(QT_NT("WM_MDITILE"));
	case 551: return QString(QT_NT("WM_MDICASCADE"));
	case 552: return QString(QT_NT("WM_MDIICONARRANGE"));
	case 553: return QString(QT_NT("WM_MDIGETACTIVE"));
	case 560: return QString(QT_NT("WM_MDISETMENU"));
	case 561: return QString(QT_NT("WM_ENTERSIZEMOVE"));
	case 562: return QString(QT_NT("WM_EXITSIZEMOVE"));
	case 563: return QString(QT_NT("WM_DROPFILES"));
	case 564: return QString(QT_NT("WM_MDIREFRESHMENU"));
	case 640: return QString(QT_NT("WM_IME_REPORT"));
	case 641: return QString(QT_NT("WM_IME_SETCONTEXT"));
	case 642: return QString(QT_NT("WM_IME_NOTIFY"));
	case 643: return QString(QT_NT("WM_IME_CONTROL"));
	case 644: return QString(QT_NT("WM_IME_COMPOSITIONFULL"));
	case 645: return QString(QT_NT("WM_IME_SELECT"));
	case 646: return QString(QT_NT("WM_IME_CHAR"));
	case 648: return QString(QT_NT("WM_IME_REQUEST"));
	case 656: return QString(QT_NT("WM_IME_KEYDOWN"));
	case 657: return QString(QT_NT("WM_IME_KEYUP"));
	case 672: return QString(QT_NT("WM_NCMOUSEHOVER"));
	case 673: return QString(QT_NT("WM_MOUSEHOVER"));
	case 674: return QString(QT_NT("WM_NCMOUSELEAVE"));
	case 675: return QString(QT_NT("WM_MOUSELEAVE"));
	case 768: return QString(QT_NT("WM_CUT"));
	case 769: return QString(QT_NT("WM_COPY"));
	case 770: return QString(QT_NT("WM_PASTE"));
	case 771: return QString(QT_NT("WM_CLEAR"));
	case 772: return QString(QT_NT("WM_UNDO"));
	case 773: return QString(QT_NT("WM_RENDERFORMAT"));
	case 774: return QString(QT_NT("WM_RENDERALLFORMATS"));
	case 775: return QString(QT_NT("WM_DESTROYCLIPBOARD"));
	case 776: return QString(QT_NT("WM_DRAWCLIPBOARD"));
	case 777: return QString(QT_NT("WM_PAINTCLIPBOARD"));
	case 778: return QString(QT_NT("WM_VSCROLLCLIPBOARD"));
	case 779: return QString(QT_NT("WM_SIZECLIPBOARD"));
	case 780: return QString(QT_NT("WM_ASKCBFORMATNAME"));
	case 781: return QString(QT_NT("WM_CHANGECBCHAIN"));
	case 782: return QString(QT_NT("WM_HSCROLLCLIPBOARD"));
	case 783: return QString(QT_NT("WM_QUERYNEWPALETTE"));
	case 784: return QString(QT_NT("WM_PALETTEISCHANGING"));
	case 785: return QString(QT_NT("WM_PALETTECHANGED"));
	case 786: return QString(QT_NT("WM_HOTKEY"));
	case 791: return QString(QT_NT("WM_PRINT"));
	case 792: return QString(QT_NT("WM_PRINTCLIENT"));
	case 793: return QString(QT_NT("WM_APPCOMMAND"));
	case 856: return QString(QT_NT("WM_HANDHELDFIRST"));
	case 863: return QString(QT_NT("WM_HANDHELDLAST"));
	case 864: return QString(QT_NT("WM_AFXFIRST"));
	case 895: return QString(QT_NT("WM_AFXLAST"));
	case 896: return QString(QT_NT("WM_PENWINFIRST"));
	case 897: return QString(QT_NT("WM_RCRESULT"));
	case 898: return QString(QT_NT("WM_HOOKRCRESULT"));
	case 899: return QString(QT_NT("WM_PENMISCINFO"));
	case 900: return QString(QT_NT("WM_SKB"));
	case 901: return QString(QT_NT("WM_PENCTL"));
	case 902: return QString(QT_NT("WM_PENMISC"));
	case 903: return QString(QT_NT("WM_CTLINIT"));
	case 904: return QString(QT_NT("WM_PENEVENT"));
	case 911: return QString(QT_NT("WM_PENWINLAST"));
	case 1024: return QString(QT_NT("WM_USER"));
	case 1025: return QString(QT_NT("WM_PSD_FULLPAGERECT"));
	case 1026: return QString(QT_NT("WM_PSD_MINMARGINRECT"));
	case 1027: return QString(QT_NT("WM_PSD_MARGINRECT"));
	case 1028: return QString(QT_NT("WM_PSD_GREEKTEXTRECT"));
	case 1029: return QString(QT_NT("WM_PSD_ENVSTAMPRECT"));
	case 1030: return QString(QT_NT("WM_PSD_YAFULLPAGERECT"));
	case 1031: return QString(QT_NT("TTM_RELAYEVENT"));
	case 1032: return QString(QT_NT("TTM_GETTOOLINFOA"));
	case 1033: return QString(QT_NT("TTM_SETTOOLINFOA"));
	case 1034: return QString(QT_NT("WIZ_QUERYNUMPAGES"));
	case 1035: return QString(QT_NT("WIZ_NEXT"));
	case 1036: return QString(QT_NT("WIZ_PREV"));
	case 1037: return QString(QT_NT("TTM_GETTOOLCOUNT"));
	case 1038: return QString(QT_NT("TTM_ENUMTOOLSA"));
	case 1039: return QString(QT_NT("TTM_GETCURRENTTOOLA"));
	case 1040: return QString(QT_NT("TTM_WINDOWFROMPOINT"));
	case 1041: return QString(QT_NT("TTM_TRACKACTIVATE"));
	case 1042: return QString(QT_NT("TTM_TRACKPOSITION"));
	case 1043: return QString(QT_NT("TTM_SETTIPBKCOLOR"));
	case 1044: return QString(QT_NT("TTM_SETTIPTEXTCOLOR"));
	case 1045: return QString(QT_NT("TTM_GETDELAYTIME"));
	case 1046: return QString(QT_NT("TTM_GETTIPBKCOLOR"));
	case 1047: return QString(QT_NT("TTM_GETTIPTEXTCOLOR"));
	case 1048: return QString(QT_NT("TTM_SETMAXTIPWIDTH"));
	case 1049: return QString(QT_NT("TTM_GETMAXTIPWIDTH"));
	case 1050: return QString(QT_NT("TTM_SETMARGIN"));
	case 1051: return QString(QT_NT("TTM_GETMARGIN"));
	case 1052: return QString(QT_NT("TTM_POP"));
	case 1053: return QString(QT_NT("TTM_UPDATE"));
	case 1054: return QString(QT_NT("TTM_GETBUBBLESIZE"));
	case 1055: return QString(QT_NT("TTM_ADJUSTRECT"));
	case 1056: return QString(QT_NT("TTM_SETTITLEA"));
	case 1057: return QString(QT_NT("TTM_SETTITLEW"));
	case 1058: return QString(QT_NT("RB_GETBANDBORDERS"));
	case 1059: return QString(QT_NT("TB_GETTOOLTIPS"));
	case 1060: return QString(QT_NT("TB_SETTOOLTIPS"));
	case 1061: return QString(QT_NT("TB_SETPARENT"));
	case 1062: return QString(QT_NT("RB_GETPALETTE"));
	case 1063: return QString(QT_NT("TB_SETROWS"));
	case 1064: return QString(QT_NT("TB_GETROWS"));
	case 1065: return QString(QT_NT("TB_GETBITMAPFLAGS"));
	case 1066: return QString(QT_NT("TB_SETCMDID"));
	case 1067: return QString(QT_NT("TB_CHANGEBITMAP"));
	case 1068: return QString(QT_NT("TB_GETBITMAP"));
	case 1069: return QString(QT_NT("TB_GETBUTTONTEXTA"));
	case 1070: return QString(QT_NT("TB_REPLACEBITMAP"));
	case 1071: return QString(QT_NT("TB_SETINDENT"));
	case 1072: return QString(QT_NT("TB_SETIMAGELIST"));
	case 1073: return QString(QT_NT("TB_GETIMAGELIST"));
	case 1074: return QString(QT_NT("TTM_ADDTOOLW"));
	case 1075: return QString(QT_NT("TTM_DELTOOLW"));
	case 1076: return QString(QT_NT("TTM_NEWTOOLRECTW"));
	case 1077: return QString(QT_NT("TTM_GETTOOLINFOW"));
	case 1078: return QString(QT_NT("TTM_SETTOOLINFOW"));
	case 1079: return QString(QT_NT("TTM_HITTESTW"));
	case 1080: return QString(QT_NT("TTM_GETTEXTW"));
	case 1081: return QString(QT_NT("TTM_UPDATETIPTEXTW"));
	case 1082: return QString(QT_NT("TTM_ENUMTOOLSW"));
	case 1083: return QString(QT_NT("TTM_GETCURRENTTOOLW"));
	case 1084: return QString(QT_NT("TB_SETMAXTEXTROWS"));
	case 1085: return QString(QT_NT("TB_GETTEXTROWS"));
	case 1086: return QString(QT_NT("TB_GETOBJECT"));
	case 1087: return QString(QT_NT("TB_GETBUTTONINFOW"));
	case 1088: return QString(QT_NT("TB_SETBUTTONINFOW"));
	case 1089: return QString(QT_NT("TB_GETBUTTONINFOA"));
	case 1090: return QString(QT_NT("TB_SETBUTTONINFOA"));
	case 1091: return QString(QT_NT("TB_INSERTBUTTONW"));
	case 1092: return QString(QT_NT("TB_ADDBUTTONSW"));
	case 1093: return QString(QT_NT("TB_HITTEST"));
	case 1094: return QString(QT_NT("TB_SETDRAWTEXTFLAGS"));
	case 1095: return QString(QT_NT("TB_GETHOTITEM"));
	case 1096: return QString(QT_NT("TB_SETHOTITEM"));
	case 1097: return QString(QT_NT("TB_SETANCHORHIGHLIGHT"));
	case 1098: return QString(QT_NT("TB_GETANCHORHIGHLIGHT"));
	case 1099: return QString(QT_NT("TB_GETBUTTONTEXTW"));
	case 1100: return QString(QT_NT("TB_SAVERESTOREW"));
	case 1101: return QString(QT_NT("TB_ADDSTRINGW"));
	case 1102: return QString(QT_NT("TB_MAPACCELERATORA"));
	case 1103: return QString(QT_NT("TB_GETINSERTMARK"));
	case 1104: return QString(QT_NT("TB_SETINSERTMARK"));
	case 1105: return QString(QT_NT("TB_INSERTMARKHITTEST"));
	case 1106: return QString(QT_NT("TB_MOVEBUTTON"));
	case 1107: return QString(QT_NT("TB_GETMAXSIZE"));
	case 1108: return QString(QT_NT("TB_SETEXTENDEDSTYLE"));
	case 1109: return QString(QT_NT("TB_GETEXTENDEDSTYLE"));
	case 1110: return QString(QT_NT("TB_GETPADDING"));
	case 1111: return QString(QT_NT("TB_SETPADDING"));
	case 1112: return QString(QT_NT("TB_SETINSERTMARKCOLOR"));
	case 1113: return QString(QT_NT("TB_GETINSERTMARKCOLOR"));
	case 1114: return QString(QT_NT("TB_MAPACCELERATORW"));
	case 1115: return QString(QT_NT("TB_GETSTRINGW"));
	case 1116: return QString(QT_NT("TB_GETSTRINGA"));
	case 8720: return QString(QT_NT("OCM_PARENTNOTIFY"));
	case 1123: return QString(QT_NT("TAPI_REPLY"));
	case 1124: return QString(QT_NT("WM_CAP_UNICODE_START"));
	case 1125: return QString(QT_NT("WM_CHOOSEFONT_SETLOGFONT"));
	case 1126: return QString(QT_NT("WM_CHOOSEFONT_SETFLAGS"));
	case 1127: return QString(QT_NT("WM_CAP_SET_CALLBACK_STATUSW"));
	case 1128: return QString(QT_NT("UDM_GETPOS"));
	case 1129: return QString(QT_NT("UDM_SETBUDDY"));
	case 1130: return QString(QT_NT("UDM_GETBUDDY"));
	case 1131: return QString(QT_NT("UDM_SETACCEL"));
	case 1132: return QString(QT_NT("UDM_GETACCEL"));
	case 1133: return QString(QT_NT("UDM_SETBASE"));
	case 1134: return QString(QT_NT("UDM_GETBASE"));
	case 1135: return QString(QT_NT("UDM_SETRANGE32"));
	case 1136: return QString(QT_NT("WM_CAP_DRIVER_GET_NAMEW"));
	case 1137: return QString(QT_NT("WM_CAP_DRIVER_GET_VERSIONW"));
	case 1138: return QString(QT_NT("UDM_GETPOS32"));
	case 1139: return QString(QT_NT("PSM_SETFINISHTEXTA"));
	case 1140: return QString(QT_NT("PSM_GETTABCONTROL"));
	case 1141: return QString(QT_NT("PSM_ISDIALOGMESSAGE"));
	case 1142: return QString(QT_NT("PSM_GETCURRENTPAGEHWND"));
	case 1143: return QString(QT_NT("PSM_INSERTPAGE"));
	case 1144: return QString(QT_NT("WM_CAP_FILE_SET_CAPTURE_FILEW"));
	case 1145: return QString(QT_NT("WM_CAP_FILE_GET_CAPTURE_FILEW"));
	case 1147: return QString(QT_NT("WM_CAP_FILE_SAVEASW"));
	case 1148: return QString(QT_NT("MCIWNDM_GETFILENAMEA"));
	case 1149: return QString(QT_NT("WM_CAP_FILE_SAVEDIBW"));
	case 1150: return QString(QT_NT("PSM_SETHEADERTITLEW"));
	case 1151: return QString(QT_NT("PSM_SETHEADERSUBTITLEA"));
	case 1152: return QString(QT_NT("PSM_SETHEADERSUBTITLEW"));
	case 1153: return QString(QT_NT("PSM_HWNDTOINDEX"));
	case 1154: return QString(QT_NT("PSM_INDEXTOHWND"));
	case 1155: return QString(QT_NT("PSM_PAGETOINDEX"));
	case 1156: return QString(QT_NT("PSM_INDEXTOPAGE"));
	case 1157: return QString(QT_NT("PSM_IDTOINDEX"));
	case 1158: return QString(QT_NT("PSM_INDEXTOID"));
	case 1159: return QString(QT_NT("PSM_GETRESULT"));
	case 1160: return QString(QT_NT("PSM_RECALCPAGESIZES"));
	case 1164: return QString(QT_NT("MCIWNDM_GET_SOURCE"));
	case 1165: return QString(QT_NT("MCIWNDM_PUT_SOURCE"));
	case 1166: return QString(QT_NT("MCIWNDM_GET_DEST"));
	case 1167: return QString(QT_NT("MCIWNDM_PUT_DEST"));
	case 1168: return QString(QT_NT("MCIWNDM_CAN_PLAY"));
	case 1169: return QString(QT_NT("MCIWNDM_CAN_WINDOW"));
	case 1170: return QString(QT_NT("MCIWNDM_CAN_RECORD"));
	case 1171: return QString(QT_NT("MCIWNDM_CAN_SAVE"));
	case 1172: return QString(QT_NT("MCIWNDM_CAN_EJECT"));
	case 1173: return QString(QT_NT("MCIWNDM_CAN_CONFIG"));
	case 1174: return QString(QT_NT("MCIWNDM_PALETTEKICK"));
	case 1175: return QString(QT_NT("IE_SETINK"));
	case 1176: return QString(QT_NT("IE_GETPENTIP"));
	case 1177: return QString(QT_NT("IE_SETPENTIP"));
	case 1178: return QString(QT_NT("IE_GETERASERTIP"));
	case 1179: return QString(QT_NT("IE_SETERASERTIP"));
	case 1180: return QString(QT_NT("IE_GETBKGND"));
	case 1181: return QString(QT_NT("IE_SETBKGND"));
	case 1182: return QString(QT_NT("IE_GETGRIDORIGIN"));
	case 1183: return QString(QT_NT("IE_SETGRIDORIGIN"));
	case 1184: return QString(QT_NT("IE_GETGRIDPEN"));
	case 1185: return QString(QT_NT("IE_SETGRIDPEN"));
	case 1186: return QString(QT_NT("IE_GETGRIDSIZE"));
	case 1187: return QString(QT_NT("IE_SETGRIDSIZE"));
	case 1188: return QString(QT_NT("IE_GETMODE"));
	case 1189: return QString(QT_NT("IE_SETMODE"));
	case 1190: return QString(QT_NT("WM_CAP_SET_MCI_DEVICEW"));
	case 1191: return QString(QT_NT("WM_CAP_GET_MCI_DEVICEW"));
	case 1204: return QString(QT_NT("WM_CAP_PAL_OPENW"));
	case 1205: return QString(QT_NT("WM_CAP_PAL_SAVEW"));
	case 1208: return QString(QT_NT("IE_GETAPPDATA"));
	case 1209: return QString(QT_NT("IE_SETAPPDATA"));
	case 1210: return QString(QT_NT("IE_GETDRAWOPTS"));
	case 1211: return QString(QT_NT("IE_SETDRAWOPTS"));
	case 1212: return QString(QT_NT("IE_GETFORMAT"));
	case 1213: return QString(QT_NT("IE_SETFORMAT"));
	case 1214: return QString(QT_NT("IE_GETINKINPUT"));
	case 1215: return QString(QT_NT("IE_SETINKINPUT"));
	case 1216: return QString(QT_NT("IE_GETNOTIFY"));
	case 1217: return QString(QT_NT("IE_SETNOTIFY"));
	case 1218: return QString(QT_NT("IE_GETRECOG"));
	case 1219: return QString(QT_NT("IE_SETRECOG"));
	case 1220: return QString(QT_NT("IE_GETSECURITY"));
	case 1221: return QString(QT_NT("IE_SETSECURITY"));
	case 1222: return QString(QT_NT("IE_GETSEL"));
	case 1223: return QString(QT_NT("IE_SETSEL"));
	case 1224: return QString(QT_NT("MCIWNDM_NOTIFYMODE"));
	case 1225: return QString(QT_NT("IE_GETCOMMAND"));
	case 1226: return QString(QT_NT("IE_GETCOUNT"));
	case 1227: return QString(QT_NT("MCIWNDM_NOTIFYMEDIA"));
	case 1228: return QString(QT_NT("IE_GETMENU"));
	case 1229: return QString(QT_NT("MCIWNDM_NOTIFYERROR"));
	case 1230: return QString(QT_NT("IE_GETPDEVENT"));
	case 1231: return QString(QT_NT("IE_GETSELCOUNT"));
	case 1232: return QString(QT_NT("IE_GETSELITEMS"));
	case 1233: return QString(QT_NT("IE_GETSTYLE"));
	case 1243: return QString(QT_NT("MCIWNDM_SETTIMEFORMATW"));
	case 1244: return QString(QT_NT("MCIWNDM_GETTIMEFORMATW"));
	case 1245: return QString(QT_NT("EM_GETSCROLLPOS"));
	case 1246: return QString(QT_NT("EM_SETSCROLLPOS"));
	case 1247: return QString(QT_NT("EM_SETFONTSIZE"));
	case 1248: return QString(QT_NT("MCIWNDM_GETFILENAMEW"));
	case 1249: return QString(QT_NT("MCIWNDM_GETDEVICEW"));
	case 1252: return QString(QT_NT("MCIWNDM_GETERRORW"));
	case 8192: return QString(QT_NT("OCMBASE"));
	case 32768: return QString(QT_NT("WM_APP"));
	case 1536: return QString(QT_NT("FM_GETFOCUS"));
	case 1537: return QString(QT_NT("FM_GETDRIVEINFOA"));
	case 1538: return QString(QT_NT("FM_GETSELCOUNT"));
	case 1539: return QString(QT_NT("FM_GETSELCOUNTLFN"));
	case 1540: return QString(QT_NT("FM_GETFILESELA"));
	case 1541: return QString(QT_NT("FM_GETFILESELLFNA"));
	case 1542: return QString(QT_NT("FM_REFRESH_WINDOWS"));
	case 1543: return QString(QT_NT("FM_RELOAD_EXTENSIONS"));
	case 1553: return QString(QT_NT("FM_GETDRIVEINFOW"));
	case 1556: return QString(QT_NT("FM_GETFILESELW"));
	case 1557: return QString(QT_NT("FM_GETFILESELLFNW"));
	case 8236: return QString(QT_NT("OCM_MEASUREITEM"));
	case 8237: return QString(QT_NT("OCM_DELETEITEM"));
	case 1625: return QString(QT_NT("WLX_WM_SAS"));
	case 8235: return QString(QT_NT("OCM_DRAWITEM"));
	case 8465: return QString(QT_NT("OCM_COMMAND"));
	case 8238: return QString(QT_NT("OCM_VKEYTOITEM"));
	case 52429: return QString(QT_NT("WM_RASDIALEVENT"));
	case 8468: return QString(QT_NT("OCM_HSCROLL"));
	case 8469: return QString(QT_NT("OCM_VSCROLL"));
	case 8498: return QString(QT_NT("OCM_CTLCOLORMSGBOX"));
	case 8499: return QString(QT_NT("OCM_CTLCOLOREDIT"));
	case 8500: return QString(QT_NT("OCM_CTLCOLORLISTBOX"));
	case 8501: return QString(QT_NT("OCM_CTLCOLORBTN"));
	case 8502: return QString(QT_NT("OCM_CTLCOLORDLG"));
	case 8503: return QString(QT_NT("OCM_CTLCOLORSCROLLBAR"));
	case 8504: return QString(QT_NT("OCM_CTLCOLORSTATIC"));
	case 2024: return QString(QT_NT("WM_CPL_LAUNCH"));
	case 2025: return QString(QT_NT("WM_CPL_LAUNCHED"));
	case 2026: return QString(QT_NT("UM_GETUSERSELW"));
	case 2027: return QString(QT_NT("UM_GETGROUPSELA"));
	case 2028: return QString(QT_NT("UM_GETGROUPSELW"));
	case 2029: return QString(QT_NT("UM_GETCURFOCUSA"));
	case 2030: return QString(QT_NT("UM_GETCURFOCUSW"));
	case 2031: return QString(QT_NT("UM_GETOPTIONS"));
	case 2032: return QString(QT_NT("UM_GETOPTIONS2"));

	default: return QString();
	}
}


QString EventManager::formatMessageDescription(MSG msg )
{
	return QString("%1\t%2").arg(msg.message).arg(GetMessageName(msg.message));
}

void EventManager::onFlick( int dir, const Vec3& pt )
{
	if (sel->getBumpObjects().empty())
	{
		MouseCallback(MouseButtonLeft, GLUT_DOWN, int(pt.x), int(pt.y));
		MouseCallback(MouseButtonLeft, GLUT_UP, int(pt.x), int(pt.y));
	}

	if (GLOBAL(isInSharingMode))
	{
		int newDesktopIndex = -1;
		int curDesktopIndex = -1;

		// determine which desktop the current picked actor is on
		BumpObject * pickedActor = sel->getPickedActor();
		if (pickedActor)
		{
			for (int i = 1; i < GLOBAL(sharedDesktops).size(); ++i)
			{
				SharedDesktop * desktop = GLOBAL(sharedDesktops)[i];
				if (find(desktop->objects.begin(), desktop->objects.end(), pickedActor) != desktop->objects.end())
				{
					curDesktopIndex = i;
					break;
				}
			}
		}

		switch (dir)
		{
		case FLICKDIRECTION_LEFT:
			if (curDesktopIndex <= 0)
				newDesktopIndex = 1;		// depends on how we initialize the shared desktops
			else if (curDesktopIndex == 1)
				newDesktopIndex = -1;
			else if (curDesktopIndex == 2)
				newDesktopIndex = 0;
			else if (curDesktopIndex == 3)
				newDesktopIndex = -1;
			break;
		case FLICKDIRECTION_RIGHT:
			if (curDesktopIndex <= 0)
				newDesktopIndex = 2;		// depends on how we initialize the shared desktops
			else if (curDesktopIndex == 1)
				newDesktopIndex = 0;
			else if (curDesktopIndex == 2)
				newDesktopIndex = -1;
			else if (curDesktopIndex == 3)
				newDesktopIndex = -1;
			break;
		case FLICKDIRECTION_UP:
			if (curDesktopIndex <= 0)
				newDesktopIndex = 3;		// depends on how we initialize the shared desktops
			else if (curDesktopIndex == 1)
				newDesktopIndex = -1;
			else if (curDesktopIndex == 2)
				newDesktopIndex = -1;
			else if (curDesktopIndex == 3)
				newDesktopIndex = -1;
			break;
		case FLICKDIRECTION_DOWN:
			if (curDesktopIndex <= 0)
				newDesktopIndex = -1;		// depends on how we initialize the shared desktops
			else if (curDesktopIndex == 1)
				newDesktopIndex = -1;
			else if (curDesktopIndex == 2)
				newDesktopIndex = -1;
			else if (curDesktopIndex == 3)
				newDesktopIndex = 0;
			break;
		default:
			break;
		}

		if (newDesktopIndex > -1)
		{
			// animate the object to the new position
			if (newDesktopIndex < GLOBAL(sharedDesktops).size())
			{
				SharedDesktop * desktop = GLOBAL(sharedDesktops)[newDesktopIndex];
				vector<BumpObject *> selection = sel->getBumpObjects();
				float yOffset = 0;
				QString imageTextureId;
	
				// remove each object from their existing shared desktops
				for (int i = 0; i < selection.size(); ++i)
				{
					for (int j = 1; j < GLOBAL(sharedDesktops).size(); ++j)
					{
						SharedDesktop * desktop = GLOBAL(sharedDesktops)[j];
						vector<BumpObject *>::iterator iter = find(desktop->objects.begin(), desktop->objects.end(), selection[i]);
						if (iter != desktop->objects.end())
						{
							desktop->objects.erase(iter);
							break;
						}
					}
				}

				// add each object to their new shared desktops
				for (int i = 0; i < selection.size(); ++i)
				{
					BumpObject * obj = selection[i]; 

					if (obj->getObjectType() == ObjectType(BumpActor, FileSystem))
					{
						FileSystemActor * fsActor = (FileSystemActor *) obj;
						if (fsActor->isThumbnailized())
						{
							imageTextureId = fsActor->getThumbnailID();
						}
					}

					obj->setLinearMomentum(Vec3(0.0f));
					obj->setLinearVelocity(Vec3(0.0f));
					if (newDesktopIndex == 2)	// toshiba photo frame
					{
						deque<Mat34> poses, tmpPoses;
						Mat34 endOri(obj->getGlobalOrientation(), desktop->offset + Vec3(0, 50.0f + yOffset, 0));
						int steps = 20;
						poses = lerpMat34RangeRaw(obj->getGlobalPose(), endOri, steps, SoftEase);
						tmpPoses = lerpMat34RangeRaw(endOri, obj->getGlobalPose(), steps, SoftEase);
						poses.insert(poses.end(), tmpPoses.begin(), tmpPoses.end());
						obj->setPoseAnim(poses);
					}
					else
					{
						desktop->objects.push_back(obj);
						obj->setPoseAnim(obj->getGlobalPose(), Mat34(obj->getGlobalOrientation(), desktop->offset + Vec3(0, 50.0f + yOffset, 0)), 40);
					}
					

					yOffset += (2.5f * obj->getDims().z);
				}

				if (!imageTextureId.isEmpty() && 
					(newDesktopIndex == 2))	// flicking image to photo frame
				{
					desktop->bgTextureOverride = imageTextureId;
				}
			}
		}
	}
	else
	{
		switch (dir)
		{
		case FLICKDIRECTION_LEFT:
			ArrowKeyCallback(KeyLeft, 0, 0);
			break;
		case FLICKDIRECTION_RIGHT:
			ArrowKeyCallback(KeyRight, 0, 0);
			break;
		case FLICKDIRECTION_UP:
			ArrowKeyCallback(KeyUp, 0, 0);
			break;
		case FLICKDIRECTION_DOWN:
			ArrowKeyCallback(KeyDown, 0, 0);
			break;
		default:
			break;
		}
	}
}

void EventManager::onDeviceAdded(DEV_BROADCAST_HDR * data)
{
	if (GLOBAL(settings).freeOrProLevel == AL_FREE)
		return;

	QList<QString> volumes;
	QString volumeName;
	if (fsManager->resolveVolume(data, volumes))
	{
		for (int i = 0; i < volumes.size(); ++i) 
		{
			QString volume = volumes[i];

			if (!fsManager->isValidVolume(volume))
				return;

			if (fsManager->resolveVolumeName(volume, volumeName))
			{
				// check if there are any fs actors to this drive right now					
				vector<FileSystemActor *> actors = scnManager->getFileSystemActors(volume, false, false);
				if (actors.empty())
				{
					// create a new filesystem actor to this drive
					scnManager->onFileAdded(volume);
					actors = scnManager->getFileSystemActors(volume, false, false);
					if (!actors.empty())
					{
						FileSystemActor * actor = actors.front();
						actor->pushFileSystemType(Folder);
						actor->pushFileSystemType(LogicalVolume);
						actor->pushFileSystemType(Removable);
						actor->setText(volumeName);
						if (fsManager->isVolumeADisc(volume))
							actor->setTextureID("icon.removable.disc");
						else
							actor->setTextureID("icon.removable.drive");
						actor->setMounted(true, volumeName);
					}
				}
				else
				{
					FileSystemActor * actor = actors.front();
					actor->setMounted(true, volumeName);
				}

				rndrManager->invalidateRenderer();
				textManager->invalidate();
			}
		}
	}
}

void EventManager::onDeviceRemoved(DEV_BROADCAST_HDR * data)
{
	QList<QString> volumes;
	if (fsManager->resolveVolume(data, volumes))
	{		
		for (int i = 0; i < volumes.size(); ++i)
		{
			QString volume = volumes[i];

			// check if there are any fs actors to this drive right now					
			vector<FileSystemActor *> actors = scnManager->getFileSystemActors(volume, false, false);
			if (!actors.empty())
			{
				// mark the actor as unmounted and make it invisible
				FileSystemActor * actor = actors.front();
				actor->setMounted(false);
			}
		}

		rndrManager->invalidateRenderer();
		textManager->invalidate();
	}
}

bool EventManager::isRDPConnected() {
	return _remoteConnectionActive;
}

void EventManager::onRDPConnect()
{
	_remoteConnectionActive = true;
	sysTray->MinimizeToTray(winOS->GetWindowsHandle());
	sysTray->setLaunchUrl(SysTray::NoURL);
	sysTray->postNotification("Remote Connection Detected", QT_TRANSLATE_NOOP("EventManager", "BumpTop is temporarily disabled"), 2500);
}

void EventManager::onRDPDisconnect()
{
	_remoteConnectionActive = false;
	sysTray->RestoreFromTray(winOS->GetWindowsHandle());
}

#ifdef ENABLE_WEBKIT
LRESULT EventManager::forwardEvent( QWidget * widget, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if (!_qtGuiWndProc)
	{
		TCHAR className[256];
		QWidget w;
		GetClassName(w.winId(), className, 256);
				
		WNDCLASSEX wcinfo;
		GetClassInfoEx((HINSTANCE) qWinAppInst(), className, &wcinfo);
		_qtGuiWndProc = wcinfo.lpfnWndProc;
	}

	return _qtGuiWndProc(widget->winId(), uMsg, wParam, lParam);
}
#endif