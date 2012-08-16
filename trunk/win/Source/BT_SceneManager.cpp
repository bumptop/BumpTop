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
#include "BT_Bubbles.h"
#include "BT_Camera.h"
#include "BT_FileSystemActor.h"
#include "BT_FileSystemActorFactory.h"
#include "BT_FileSystemManager.h"
#include "BT_FileSystemPile.h"
#include "BT_MenuAction.h"
#include "NetworkAccessManager.h"
#include "BT_OverlayComponent.h"
#include "BT_PbPersistenceHelpers.h"
#include "BT_PbPersistenceManager.h"
#include "BT_Pile.h"
#include "BT_RenderManager.h"
#include "BT_Replayable.h"
#include "BT_SceneManager.h"
#include "BT_Selection.h"
#include "BT_StatsManager.h"
#include "BT_StickyNoteActor.h"
#include "BT_TextManager.h"
#include "BT_Util.h"
#include "BT_WatchedObjects.h"
#include "BT_WebThumbnailActor.h"
#include "BT_WidgetManager.h"
#include "BT_WindowsOS.h"
#include "BumpTop.pb.h"
#include "Nx.pb.h"
#include "TextPixmapBuffer.h"
#include "BT_LibraryManager.h"

#ifdef DXRENDER
	#include "BT_DXRender.h"
#endif

SceneManager::SceneManager()
: messagesContainer(0)
, hudTextBuffer()
, _libraryMenuControl(NULL)
#ifdef DXRENDER
, _dxRef(dxr)
{	
	// Lock reference to DXRender
	_dxRef.lock();
#else
{
#endif
	// initialize vars
	DWORD curThreadId = GetCurrentThreadId();

	trialDays = 14;

	skipSavingSceneFile = false;
	isInSandboxMode = false;
	testUpdater = false;
	isShellExtension = false;
	isInInfiniteDesktopMode = false;
	isBumpPhotoMode = false;
	isChildProcess = false;
	childProcess = NULL;
	runBumpTopTestsOnStartup = false;
	runAutomatedJSONTestsOnStartup = false;
	disableRendering = false;
	skipAllPromptDialogs = false;
	zoomToAngleBoundsTempOverride = 0;
	isMultiSamplingSupported = true;
	skipAnimations = false;

	// replayable
	_replayable = NULL;
	_trainingIntroRunner = NULL;

	// actor data vars
	isInteraction = false;
	touchGestureBrowseMode = false;
	supportedExtensions.append(".");
	supportedExtensions.append((char *) ilGetString(IL_LOAD_EXT));
	supportedExtensions.replace(" ", ".");
	// NOTE: some devil image extensions overlap with other common file extensions, ignore these
	supportedExtensions.replace(".mp3", "");

	// old pile vars
	gPause = false;
	gPhysicsSDK = NULL;
	gScene = NULL;

	// util vars
	exitBumpTopFlag = false;
	DrawWalls = true;

	mode = None;
	straightIconOri = Quat(90, Vec3(1,0,0));
	factor = -0.27f;

	// Buffer used when zooming in and out of the bumptop desktop
	ZoomBuffer = -5;

	// font manager
	nearClippingPlane = 1.0f;
	farClippingPlane = 2000.0f;

	// operating system
	mouseUpTriggered = false;
	consoleWindowHandle = NULL;
	MouseOverWall = false;
	PinWall = NULL;

	// bumptop statics
	lastEye = Vec3(0.0f);
	widgetFirstMovePosX = -1; 
	widgetFirstMovePosY = -1;
	SelectionIncludesAPile = false;
	mbutton = 0;
	mstate = 0;
	mkey = 0;
	startTimer = false;
	mouseMoving = false;
	shiftTally=0;
	dblClickSize = 10;
	sglClickSize = 2;
	disallowMenuInvocation = false;
	WallHeight = 112.0f;
	maxDelayBetweenKeyDown = 1.45f;
	defaultActorGrowFactor = 1.0;

	useSingleClickExpiration = false;

	// context menu statics
	g_IContext2 = NULL;
	g_IContext3 = NULL;

	// drag and drop statics
	isDragInProgress = false;

	// windows os statics
	windowXOffset = 0;
	windowYOffset = 0;

	// actor data statics
	id_count = 0;

	// local statics
	clickDistance = 0;
	framesPerSecond = 1;
	avgFramesPerSecond = 1; 
	maxFramesPerSecond = 0;
	frameCounter = 1;
	framesPerSecondCounter = 0;
	firstTime = true;
	firstReshape = true;
	checked = false;
	dialogHwnd = NULL;

	// Windows System
	windowXOffset = 0;
	windowYOffset = 0;
	mouseUpTriggered = false;

	// sharing
	enableSharingMode = false;
	isInSharingMode = false;

	isSceneLoaded = false;
	_threadId = GetCurrentThreadId();
}

SceneManager::~SceneManager()
{
	fsManager->removeObject(this);

	// delete the overlays
	for (int i = 0; i < overlays.size(); ++i)
	{
		delete overlays[i];
	}
	overlays.clear();
	
#ifdef DXRENDER
	// Remove hold on DXRender
	_dxRef.reset();
#endif
}

void SceneManager::init()
{
#ifdef WIN7LIBRARIES
	assert(winOS->GetLibraryManager());
	init(winOS->GetLibraryManager()->getDesktopLibrary());
#else
	init(winOS->GetSystemPath(DesktopDirectory));
#endif

}

void SceneManager::init(QString workingDir)
{
	setWorkingDirectory(workingDir);

	// default scene bump file is in the data directory
	currentSceneFile = make_file(winOS->GetDataDirectory(), "scene.bump");

	// default backup scene bump file is the scene bump file + .bak extension
	backupSceneFile = QFile(native(currentSceneFile) + ".bak");

	// the new scene format files are in the data directory as well
	setScenePbBumpFile(make_file(winOS->GetDataDirectory(), "scene.pb.bump"));
}

void SceneManager::init(QSharedPointer<Library>& library)
{
	setCurrentLibrary(library);
}

bool SceneManager::addObject(BumpObject *newObject)
{
	assert(GetCurrentThreadId() == _threadId);
	if (isRegistered(newObject) == -1)
	{
		// If this object is not in our system already, add it
		_bumpObjects.push_back(newObject);

		// If we're in infinite desktop mode, update the list of all actors
		
		if (GLOBAL(isInInfiniteDesktopMode))
		{
			cam->watchActor(newObject);
		}

		return true;
	}

	return false;
}

bool SceneManager::containsObject(BumpObject *object)
{
	assert(GetCurrentThreadId() == _threadId);
	return isRegistered(object) > -1;
}

bool SceneManager::removeObject(BumpObject *newObject)
{
	assert(GetCurrentThreadId() == _threadId);
	int indx = -1;
  
	for (uint i = 0; i < _bumpObjects.size(); i++)
	{
		// We found our objects, return the index
		if (_bumpObjects[i] == newObject)
			indx = i;
		if (_bumpObjects[i]->isBumpObjectType(BumpActor) && newObject == ((Actor *)_bumpObjects[i])->getObjectToMimic())
			((Actor *)_bumpObjects[i])->setObjectToMimic(NULL);
	}

	if (indx > -1)
	{
		// Remove the object from our system
		_bumpObjects.erase(_bumpObjects.begin() + uint(indx));

		// If we're in infinite desktop mode, update the list of all actors
		
		if (GLOBAL(isInInfiniteDesktopMode))
		{
			cam->unwatchActor(newObject);
		}

		// clear the camera slideshow mode if the item being removed is currently being watched
		if (cam->isWatchedActorHighlighted(newObject))
			Key_DisableSlideShow();

		return true;
	}

	return false;
}

int SceneManager::isRegistered(BumpObject *object)
{
	assert(GetCurrentThreadId() == _threadId);
	for (uint i = 0; i < _bumpObjects.size(); i++)
	{
		// We found our objects, return the index
		if (_bumpObjects[i] == object)
		{
			return int(i);
		}
	}

	// Not Found
	return -1;
}

const vector<BumpObject *>& SceneManager::getBumpObjects()
{
	assert(GetCurrentThreadId() == _threadId);
	return _bumpObjects;
}

vector<BumpObject *> SceneManager::getBumpObjects( QString filePathFilter, const ObjectType& objType, bool isRegex )
{
	assert(GetCurrentThreadId() == _threadId);
	/* if there is no filter, then we return all objects of the specified type */

	// filter the filesystem actors of the specified type
	vector<BumpObject *> objects;
	for (int i = 0; i < _bumpObjects.size(); ++i)
	{
		if (_bumpObjects[i]->getObjectType() == objType)
		{
			if (filePathFilter.isEmpty())
				objects.push_back(_bumpObjects[i]);
			else
			{
				// to check against the path filter, the object must be filesystem actors
				if (_bumpObjects[i]->getObjectType() == ObjectType(BumpActor, FileSystem))
				{
					FileSystemActor * fsActor = (FileSystemActor *) _bumpObjects[i];
					if (!isRegex && fsManager->isIdenticalPath(fsActor->getFullPath(), filePathFilter))
					{
						objects.push_back(fsActor);
					}
					else
					{
						QString filePath = fsActor->getFullPath();
						QRegExp filter(filePathFilter);
						filter.setCaseSensitivity(Qt::CaseInsensitive);
						if (filter.indexIn(filePath) > -1)
						{
							objects.push_back(fsActor);
						}
					}

				}
			}		
		}
	}
	return objects;
}

vector<BumpObject *> SceneManager::getBumpObjects( const ObjectType& objType )
{	
	assert(GetCurrentThreadId() == _threadId);

	vector<BumpObject *> objects;
	for (int i = 0; i < _bumpObjects.size(); ++i)
	{
		if (_bumpObjects[i]->getObjectType() == objType)
			objects.push_back(_bumpObjects[i]);
	}
	return objects;
}

vector<FileSystemActor *> SceneManager::getFileSystemActors(unsigned int fileSystemTypeMask, bool ignoreInvisible/*=false*/) const
{
	assert(GetCurrentThreadId() == _threadId);
	// get the set of bump objects
	vector<BumpObject *> objects;
	if (ignoreInvisible)
		objects = getVisibleBumpActorsAndPiles();
	else
		objects = _bumpObjects;	

	// filter the filesystem actors of the specified type
	vector<FileSystemActor *> fsActors;
	for (int i = 0; i < objects.size(); ++i)
	{
		if (objects[i]->getObjectType() == ObjectType(BumpActor, FileSystem, fileSystemTypeMask))
		{
			fsActors.push_back(dynamic_cast<FileSystemActor *>(objects[i]));
		}
	}
	return fsActors;
}

vector<FileSystemActor *> SceneManager::getFileSystemActors(QString filePathFilter, bool ignoreNonFreeActors, bool isRegex)
{
	assert(GetCurrentThreadId() == _threadId);
	// get the set of bump objects
	vector<BumpObject *> objects;
	if (ignoreNonFreeActors)
		objects = getVisibleBumpActorsAndPiles();
	else
		objects = _bumpObjects;	

	// filter the filesystem actors of the specified type
	vector<FileSystemActor *> fsActors;
	for (int i = 0; i < objects.size(); ++i)
	{
		if (objects[i]->getObjectType() == ObjectType(BumpActor, FileSystem))
		{
			FileSystemActor * fsActor = dynamic_cast<FileSystemActor *>(objects[i]);
			if (!isRegex && fsManager->isIdenticalPath(fsActor->getFullPath(), filePathFilter))
			{
				fsActors.push_back(fsActor);
			}
			else if (isRegex)
			{
				QString filePath = fsActor->getFullPath();
				QRegExp filter(filePathFilter);
				filter.setCaseSensitivity(Qt::CaseInsensitive);
				if (filter.indexIn(filePath) > -1)
				{
					fsActors.push_back(fsActor);
				}
			}
		}
	}
	return fsActors;
}

//
// getVisibleBumpActorsAndPiles()
//
// only returns objects that are actors and piles, that are not invisible, 
// and that are not in a pile that is Stack(ed) or in NoState
// (ie. BumpObjects in a stacked pile are not included, because they're buried in the pile; 
//		once the pile is fanned out, however, the BumpObjects inside it are included)
//
vector<BumpObject *> SceneManager::getVisibleBumpActorsAndPiles( bool includeItemsInStackedPiles /*= false*/ ) const
{
	assert(GetCurrentThreadId() == _threadId);
	vector<BumpObject *> visibleBumpObjects;

	for (int i = 0; i < _bumpObjects.size(); i++)
	{
		BumpObject *obj = _bumpObjects[i];

		if (obj && (obj->isBumpObjectType(BumpActor) || obj->isBumpObjectType(BumpPile)))
		{
			Actor *aData = obj->isBumpObjectType(BumpActor) ? (Actor *) obj : NULL;
			Pile *pData = obj->isBumpObjectType(BumpPile) ? (Pile *) obj : NULL;

			// Check for the visibility of an item

			// An item is visible if it's a pile... 
			bool isVisible = pData != NULL;

			// or if it's a regular actor that's not invisible...
			if (aData && !aData->isActorType(Invisible)) {

				isVisible = true;

				// and that's not in a pile that's Stack(ed) or in NoState
				BumpObject *parentObj = aData->getParent();

				if (parentObj)
				{
					Pile *parentPile = parentObj->isBumpObjectType(BumpPile) ? (Pile *) parentObj : NULL;
					if (parentPile)
					{
						PileState parentPileState = parentPile->getPileState();
						if (parentPileState == NoState || (!includeItemsInStackedPiles && parentPileState == Stack))
						{
							isVisible = false;
						}
						else if (parentPileState == Leaf && (parentPile->getActiveLeafItem() != obj))
						{
							isVisible = false;
						}
					}
				}
				
			}

			// account for gridded pile items
			if (pData)
			{
				if (includeItemsInStackedPiles && (pData->getPileState() == Grid))
				{
					vector<BumpObject *> items = pData->getPileItems();
					for (int j = 0; j < items.size(); ++j)
					{
						assert(items[j]->isBumpObjectType(BumpActor));
						if (((Actor *)items[j])->isActorType(Invisible))
						{
							visibleBumpObjects.push_back(items[j]);
						}
					}
				}
			}

			if (isVisible)
			{
				visibleBumpObjects.push_back(obj);
			}
		}
	}

	return visibleBumpObjects;
}


QDir SceneManager::getWorkingDirectory() const
{
	return currentWorkingDirectory;
}

QSharedPointer<Library> SceneManager::getCurrentLibrary() const
{
	return currentLibrary;
}

QFileInfo SceneManager::getSceneBumpFile() const
{
	return currentSceneFile;
}

QFileInfo SceneManager::getSceneBumpBackupFile() const
{
	return backupSceneFile;
}

void SceneManager::setWorkingDirectory(QDir p)
{
	if (exists(p))
	{
		fsManager->removeObject(this);

		// Remove any library
		currentLibrary.clear();
		
		// set the scene path to the desired path
		currentWorkingDirectory = p;
		{
			QDir wd = currentWorkingDirectory;
			QString hashedPath;
			do 
			{
				if (!wd.dirName().isEmpty())
				{
					hashedPath = QString(QCryptographicHash::hash(wd.dirName().toUtf8(), QCryptographicHash::Md5).toHex()) + "/" + hashedPath;
				}
			}
			// i18n: FIX ME
			while (wd.cdUp());			
			statsManager->getStats().bt.workingDirectory = stdString(hashedPath);
		}
		if (!GLOBAL(isInTrainingMode))
		{
			fsManager->addObject(this);
		}

		SAFE_DELETE(_libraryMenuControl);

		winOS->ReRegisterDropHandler();
	}
}

// Sets the current working directory to the library's default
// directory
void SceneManager::setCurrentLibrary(QSharedPointer<Library>& library)
{
	if (!library)
		return;

	// Set the current Library and add it to the file system watcher
	QList<QString> dirs = library->getFolderPaths();
	assert(dirs.size() > 0);
	
	if (exists(dirs.front()))
	{
		fsManager->removeObject(this);
		currentLibrary = library;
		currentWorkingDirectory = dirs.front();
		GLOBAL(settings).currentLibrary = library->getHashKey();
		winOS->SaveSettingsFile();

		fsManager->addObject(this);
		winOS->ReRegisterDropHandler();
		
		// Create the correctly named scene file
		QString sceneFileName;
		if (library->getHashKey().startsWith(QT_NT("usr_")))
		{
			QString sceneHash = QString(QCryptographicHash::hash(currentLibrary->getFolderPaths().front().toUtf8(), QCryptographicHash::Md5).toHex());	
			sceneFileName = QString_NT("%1_scene.pb.bump").arg(sceneHash);
		}
		else
		{
			sceneFileName = QString_NT("%1_scene.pb.bump").arg(currentLibrary->getName());
			if (currentLibrary->getHashKey() == QT_NT("def_Desktop"))
				sceneFileName = QString_NT("scene.pb.bump");
		}
		setScenePbBumpFile(make_file(winOS->GetDataDirectory(), sceneFileName));
	}
	else
	{
		// Add handling here
		LOG(QString_NT("Bad library. Cannot set as current library."));
		printUnique(QT_NT("Library"), QT_NT("This location does not exist"));
	}

	if (winOS->GetLibraryManager() && GLOBAL(settings).enableLibraryOverlay)
	{
		//if (winOS->GetLibraryManager()->getLibraries().size() > 1)
		{
			if (!_libraryMenuControl)
				_libraryMenuControl = new LibraryMenuControl();
			_libraryMenuControl->show(currentLibrary);
		}
	}
}

void SceneManager::setSceneBumpFile(QFileInfo p)
{
	// NOTE: It is possible for the specified path not to exist
	currentSceneFile = p;

	// update backup path with changes to the scene bump file
	backupSceneFile = QFile(native(currentSceneFile) + ".bak");
}

vector<Pile *> SceneManager::getPiles(bool ignoreEmptyPiles)
{
	assert(GetCurrentThreadId() == _threadId);
	vector<Pile *> pileCnt;

	for (uint i = 0; i < _bumpObjects.size(); i++)
	{
		if (_bumpObjects[i]->isBumpObjectType(BumpPile))
		{
			Pile * p = dynamic_cast<Pile *>(_bumpObjects[i]);
			if (!ignoreEmptyPiles || (p->getNumItems() > 0))
				pileCnt.push_back(p);
		}
	}

	return pileCnt;
}

void SceneManager::destroyScene()
{
	assert(GetCurrentThreadId() == _threadId);
	clearBumpTop();

	// Destroy Physics
	if (gPhysicsSDK && gScene)
	{
		// -------------------------------------------------------------------------
		// NOTE: Crashes the BT shell extension when multiple windows are open, and
		//		 one is closed. The Physics engine (SDK) cannot be deleted within
		//       each instance of BT because many instances use the same Physics
		//       DLL.
		// -------------------------------------------------------------------------
		gPhysicsSDK->releaseScene(*gScene);
		gScene = NULL;
	}
}

void SceneManager::clearBumpTop()
{
	assert(GetCurrentThreadId() == _threadId);
	vector<Pile *> piles = getPiles();
	

	// Delete All Piles 
	while (piles.size() > 0)
	{
		for(int i=0; i < piles.size(); i++)
		{
			Pile *p = (Pile *) piles[i];

			if (p->getPileType() == HardPile)
			{
				// Folderize any hard piles
				FileSystemPile *fsPile = (FileSystemPile *) p;
				fsPile->folderize(false);
			}

			DeletePile(p, true, false, true);
		}

		piles = getPiles();
	}

	// Go through all actors and delete them from the scene
	for (int i = 0; i < activeNxActorList.size(); i++)
	{
		if (GetBumpActor(activeNxActorList[i]))
		{
			BumpObject *obj = (BumpObject *) activeNxActorList[i];
			SAFE_DELETE(obj);
			i--;
		}
	}

	//reset the bubble manager
	bubbleManager->reset();
}

StrList SceneManager::getWatchDir()
{
	StrList wList;
	
	if (currentLibrary)
	{
		QListIterator<QString> dirIt(currentLibrary->getFolderPaths());
		while (dirIt.hasNext())
			wList.push_back(dirIt.next());
	}
	else
	{
		wList.push_back(native(currentWorkingDirectory));
	}

	// Return the directory we are watching
	return wList;
}

void SceneManager::onFileAdded(QString strFileName)
{
	// ensure that the file actually exists
	if (!exists(strFileName))
		return;
	
	// handle drag and drop
	assert(GetCurrentThreadId() == _threadId);
	static int numDroppedFilesRemainingPreviously = 0;
	int numDroppedFilesRemaining = 0;
	
	// We don't allow hidden files
	unsigned int fileAttr = fsManager->getFileAttributes(strFileName);
	bool isVolumePath = (native(parent(strFileName)) == strFileName);
	if (!isVolumePath)
		if (!GLOBAL(settings).LoadHiddenFiles && (fileAttr & Hidden)) return;

	if (WebThumbnailActor::isValidWebThumbnailActorUrlFile(strFileName))
	{
		// check if there is already a page with this url (and return if so)
		vector<BumpObject *> pages = scnManager->getBumpObjects(ObjectType(BumpActor, Webpage));
		for (int i = 0; i < pages.size(); ++i) 
		{
			WebActor * page = (WebActor *) pages[i];
			if (page->getPageData().compare(strFileName, Qt::CaseInsensitive) == 0)
				return;
		}

		// load this new page
		WebActor * actor = new WebActor();
		actor->load(FileSystemActorFactory::parseURLFileForLink(strFileName));		
		actor->setFreshnessAlphaAnim(1.0f, 150);
		actor->setGlobalPosition(winOS->GetDropPoint(numDroppedFilesRemaining, 50));

		// delete the actual url
		fsManager->deleteFileByName(strFileName);

		// select this new actor	
		if (numDroppedFilesRemainingPreviously == 0)
			sel->clear();
		sel->add(actor);
	}
	else 
	{		
		FileSystemActor *fsData;

		// Look for collisions
		for (uint i = 0; i < _bumpObjects.size(); i++)
		{
			if (_bumpObjects[i]->getObjectType() == ObjectType(BumpActor, FileSystem))
			{
				fsData = (FileSystemActor *) _bumpObjects[i];

				if (fsManager->isIdenticalPath(fsData->getFullPath(), strFileName))
				{
					// This item is already in the scene
					return;
				}
			}
		}

		// Create the new Actor
		const float scaleFactor = 2.0f;
		FileSystemActor *actor = FileSystemActorFactory::createFileSystemActor(strFileName);
		Vec3 dims = actor->getDefaultDims();
	#ifdef TABLE
		dims.x *= scaleFactor;
		dims.y *= scaleFactor;
	#endif
		actor->setDims(dims);
		actor->setAlpha(1.0f);
		actor->setFilePath(strFileName);
		actor->setLinearVelocity(Vec3(0, -0.75f, 0));
		actor->setFreshnessAlphaAnim(1.0f, 150);

		// enable thumbnails where possible 
		// (only actually load thumbnails if the file is not still being loaded.
		// (in that case, the next modified event that gets sent will refresh the thumbnail) 
		if (actor->isFileSystemType(Thumbnail))
			actor->enableThumbnail(true, !winOS->IsFileInUse(strFileName));

		// trim the recent deleted positions
		QHash<QString, RecentlyFSDeletedFile>::iterator iter = _recentFSDeletionPositions.begin();
		while (iter != _recentFSDeletionPositions.end())
		{
			if (iter.value().timer.elapsed() > 1000)
				_recentFSDeletionPositions.erase(iter++);
			else
				++iter;
		}

		// give this actor a random orientation and position
		const int distDelta = 20;
		Vec3 randPos(winOS->GetDropPoint(numDroppedFilesRemaining));
		randPos.x += (rand() % distDelta);
		randPos.z += (rand() % distDelta);
		randPos.y += (actor->getDims().z * numDroppedFilesRemaining);
		actor->setGlobalPosition(randPos);
		actor->setGlobalOrientation(Quat(45, Vec3(1,0,0)));

		// update the shell extension status bar if necessary
		if (scnManager->isShellExtension)
			winOS->ShellExtUpdateStatusBar();

		// jump into infinite desktop mode if necessary
		if (scnManager->isInInfiniteDesktopMode && 
			!cam->isWatchingActors())
		{
			vector<BumpObject *> objs;
			objs.push_back(actor);
			cam->pushWatchActors(objs);
		}

		// select this new actor	
		if (numDroppedFilesRemainingPreviously == 0)
			sel->clear();
		sel->add(actor);
	}
	
	numDroppedFilesRemainingPreviously = numDroppedFilesRemaining;
}

void SceneManager::onFileRemoved(QString strFileName)
{
	assert(GetCurrentThreadId() == _threadId);
	FileSystemActor *fsData;
	QString shortFilePathQS; 
		
	if (fsManager->isValidFileName(strFileName) && strFileName.endsWith(".lnk", Qt::CaseInsensitive))
	{
		// Don't remove FS Actors that still exist in the working directory
		return;
	}

	for (uint i = 0; i < _bumpObjects.size(); i++)
	{
		// Only work on filesystem actors that do not have parents
		if (_bumpObjects[i]->getObjectType() == ObjectType(BumpActor, FileSystem))
		{
			fsData = (FileSystemActor *) _bumpObjects[i];
			
			if (fsManager->isIdenticalPath(fsData->getFullPath(), strFileName) ||
				fsManager->isIdenticalPath(fsData->getShortPath(), strFileName))
			{
				// save this position in case a new file is added with the same name in the next little while
				_recentFSDeletionPositions[fsData->getFullPath()] = RecentlyFSDeletedFile(fsData->getGlobalPose(), fsData->getDims());

				// Folderize before deleting
				if (fsData->isPileized())
				{
					fsData->getPileizedPile()->folderize(false);
				}

				FadeAndDeleteActor((Actor *) fsData);

				// update the shell extension status bar if necessary
				if (scnManager->isShellExtension)
					winOS->ShellExtUpdateStatusBar();

				return;
			}
		}
	}
}

void SceneManager::onFileNameChanged(QString strOldFileName, QString strNewFileName)
{
	assert(GetCurrentThreadId() == _threadId);
	FileSystemActor *fsData;

	for (uint i = 0; i < _bumpObjects.size(); i++)
	{
		// Only work on filesystem actors that do not have parents
		if (_bumpObjects[i]->getObjectType() == ObjectType(BumpActor, FileSystem))
		{
			fsData = (FileSystemActor *) _bumpObjects[i];

			if (fsManager->isIdenticalPath(fsData->getFullPath(), strOldFileName))
			{
				_recentFSDeletionPositions[fsData->getFullPath()] = RecentlyFSDeletedFile(fsData->getGlobalPose(), fsData->getDims());

				// Set a new name for this item
				if (fsData->getPileizedPile())
					fsManager->removeObject(fsData->getPileizedPile());
				fsData->setFilePath(strNewFileName);	
				if (fsData->getPileizedPile())
					fsManager->addObject(fsData->getPileizedPile());
				fsData->setFreshnessAlphaAnim(1.0f, 150);

				// update all the piles that have this actor as an owner
				for (uint j = 0; j < _bumpObjects.size(); ++j)
				{
					if (_bumpObjects[j]->getObjectType() == ObjectType(BumpPile, HardPile))
					{
						FileSystemPile * fsPile = (FileSystemPile *) _bumpObjects[j];
						if (fsPile->getOwner() == fsData)
						{
							fsPile->setText(fsData->getFileName(fsData->isFileSystemType(Link) || fsData->isFileSystemType(DeadLink)));

							// also update the path of each of the items inside that pile
							// NOTE: we don't need to do this if we are a shortcut
							if (!fsData->isFileSystemType(Link))
							{
								vector<BumpObject *> pileItems = fsPile->getPileItems();
								for (int k = 0; k < pileItems.size(); ++k)
								{
									BumpObject * obj = pileItems[k];
									if (obj->getObjectType() == ObjectType(BumpActor, FileSystem))
									{
										FileSystemActor * fsActor = (FileSystemActor *) obj;
										QString newFilePath = strNewFileName + fsActor->getFullPath().mid(strOldFileName.size());
										fsActor->setFilePath(newFilePath);
									}
								}
							}
						}
					}
				}

				// if this was a temporary file that is being renamed to a file that
				// was recently on the desktop (as done by word, e-texteditor, etc.)
				// then we should restore the previous pose and remove any temporary
				// actors that were created in the deletion of the temporary file 
				// that was created
				// NOTE: we don't know which temporary actor it is, but it is unlikely
				// that there is another temporary actor animating (and worse case, that
				// gets finished as well)
				if (_recentFSDeletionPositions.contains(strNewFileName))
				{
					// set the file position
					fsData->finishAnimation();
					fsData->setGlobalPose(_recentFSDeletionPositions[strNewFileName].pose);
					fsData->setDims(_recentFSDeletionPositions[strNewFileName].dims);
					_recentFSDeletionPositions.remove(strNewFileName);

					// remove all the temporary animating objs
					vector<Animatable*> animatables = animManager->getAnimatingObjs();
					for (int i = 0; i < animatables.size(); ++i)
					{
						Actor * a = dynamic_cast<Actor *>(animatables[i]);
						if (a && a->isActorType(Temporary))
							animManager->finishAnimation(a);
					}
				}
				
				rndrManager->invalidateRenderer();

				return;
			}
		}
	}
}

void SceneManager::onFileModified(QString strFileName)
{
	// ensure that the file actually exists
	if (!exists(strFileName))
		return;

	assert(GetCurrentThreadId() == _threadId);
	FileSystemActor *fsData;
	uint fileAttr;
	fileAttr = fsManager->getFileAttributes(strFileName);
	bool fileIsHidden = fileAttr & Hidden;
	for (uint i = 0; i < _bumpObjects.size(); i++)
	{
		BumpObject * obj = _bumpObjects[i];

		// NOTE: The onFileModified operation is ONLY done on FileSystemActors and not Hard Piles.
		//       If you want to modify a HardPile, all modifications are done to its owner.
		if (!(_bumpObjects[i]->getObjectType() == ObjectType(BumpActor, FileSystem))) continue;

		// Only work on filesystem actors that do not have parents, or actors in soft piles
		bool isFreeActor = !_bumpObjects[i]->getParent();
		bool isSoftPiledActor = _bumpObjects[i]->getParent() && (_bumpObjects[i]->getParent()->getObjectType() == ObjectType(BumpPile, SoftPile));
		if (isFreeActor || isSoftPiledActor)
		{
			fsData = (FileSystemActor *) _bumpObjects[i];

			if (fsManager->isIdenticalPath(fsData->getFullPath(), strFileName) ||
				fsManager->isIdenticalPath(fsData->getShortPath(), strFileName))
			{
				fsData->setFreshnessAlphaAnim(1.0f, 150);

				if (!GLOBAL(settings).LoadHiddenFiles && fileIsHidden)
				{
					FadeAndDeleteActor((Actor *) fsData);
					return;
				}

				// If the file is a shortcut see if we can resolve its target
				if (strFileName.endsWith(".lnk", Qt::CaseInsensitive) && fsData->isFileSystemType(DeadLink))
				{
					// Attempt to try and resolve the shortcut
					fsData->setFilePath(strFileName);
					return;
				}
				else if (fsData->isFileSystemType(Thumbnail))
				{
					fsData->refreshThumbnail();
					return;
				}
				else if (fsData->isFileSystemType(StickyNote))
				{
					((StickyNoteActor *) fsData)->syncStickyNoteWithFileContents();
					return;
				}
				return;
			}
		}
	}

	if (!fileIsHidden)
	{
		// The file has been marked as not hidden. Create and show an actor for this file
		onFileAdded(strFileName);
	}
}

#ifdef DXRENDER
void SceneManager::onRelease()
{
	hudTextBuffer.onRelease();
	vector<BumpObject*>::iterator it;
	for (it = _bumpObjects.begin(); it != _bumpObjects.end(); it++)
	{
		(*it)->onRelease();
	}

	vector<OverlayLayout*>::iterator it2;
	for (it2 = overlays.begin(); it2 != overlays.end(); it2++)
	{
		(*it2)->onRelease();
	}
}
#endif

void SceneManager::crossReference()
{
	// get the list of objects in the working directory (use both the global & user desktops in the desktop case)
	vector<QString> workingDirFiles = fsManager->getWorkingDirectoryContents();
	if (winOS->GetSystemPath(DesktopDirectory) == native(currentWorkingDirectory))
		workingDirFiles = mergeVectors(workingDirFiles, fsManager->getDirectoryContents(winOS->GetSystemPath(AllUsersDesktopDir)));

	// add all the active widget's working directories
	vector<Widget *> widgets = widgetManager->getActiveWidgets();
	for (int i = 0; i < widgets.size(); ++i)
		workingDirFiles = mergeVectors(workingDirFiles, fsManager->getDirectoryContents(widgets[i]->workingDirectory));

	// put it into a set so lookups are quicker later
	set<QString> workingDirFilesSet;
	for (int i = 0; i < workingDirFiles.size(); ++i)
		workingDirFilesSet.insert(workingDirFiles[i]);

	// remove all items in the scene that don't exist in the filesystem
	BumpObject * parent = NULL;
	vector<BumpObject *> objs = _bumpObjects;
	for (int i = 0; i < objs.size(); ++i)
	{
		// skip items in hard piles
		parent = objs[i]->getParent();
		if (!parent || !(parent->getObjectType() == ObjectType(BumpPile, HardPile)))
		{
			// skip non-fsactors, or photo frames and virtual actors
			// also skip logical volumes
			ObjectType objType = objs[i]->getObjectType();
			if (objType == ObjectType(BumpActor, FileSystem))
			{
				FileSystemActor * actor = (FileSystemActor *) objs[i];
				if (!actor->isFileSystemType(PhotoFrame) &&
					!actor->isFileSystemType(Virtual) &&
					!actor->isFileSystemType(LogicalVolume))
				{
					QString filePath = actor->getFullPath();
					QFileInfo actorPath(filePath);
					set<QString>::iterator iter = workingDirFilesSet.begin();
					bool exists = false;
					while (iter != workingDirFilesSet.end())
					{
						if (fsManager->isIdenticalPath(*iter, filePath))
						{
							workingDirFilesSet.erase(iter++);
							exists = true;
						}
						else
							iter++;
					}

					if (!exists)
					{
						if (parent && parent->isObjectType(BumpPile))
						{
							// remove it from the parent's pile if necessary
							Pile * p = (Pile *) parent;
							p->removeFromPile(actor);
						}

						// no longer exists, so delete it from the scene
						SAFE_DELETE(actor);
					}
				}

				// we manually update the removable logical drives since the order of the drives may have changed since
				else if (actor->isFileSystemType(LogicalVolume) && 
						 actor->isFileSystemType(Removable))
				{
					// (initially assume non-mounted)
					QString volumeName;
					actor->setText(volumeName);
					if (fsManager->isVolumeADisc(actor->getFullPath()))
						actor->setTextureID("icon.removable.disc");
					else
						actor->setTextureID("icon.removable.drive");
					actor->setMounted(false);
					if (fsManager->resolveVolumeName(actor->getFullPath(), volumeName))
						actor->setMounted(true, volumeName);
				}
			}
		}
	}

	// add the remaining "new" items
	set<QString>::const_iterator iter = workingDirFilesSet.begin();
	while (iter != workingDirFilesSet.end())
	{
		onFileAdded(*iter);
		iter++;
	}
	// note that the positions for new files are messed up when this is done on startup 
	// (since the window dimensions are wrong, etc.) so position them manually at 0,0,0
	vector<BumpObject *> objects = scnManager->getBumpObjects();
	for (int i = 0; i < objects.size(); ++i)
	{
		if (objects[i]->getObjectType() == ObjectType(BumpActor, FileSystem))
		{
			FileSystemActor * fsActor = (FileSystemActor *) objects[i];
			QString actorPath = fsActor->getFullPath();
			set<QString>::const_iterator iter = workingDirFilesSet.begin();
			while (iter != workingDirFilesSet.end())
			{
				if (fsManager->isIdenticalPath(actorPath, *iter))
				{
					Vec3 dims = fsActor->getDims();
					fsActor->setGlobalPosition(Vec3(0, dims.z, 0));
				}
				iter++;
			}
		}
	}
}

void SceneManager::registerOverlay(OverlayLayout * overlayLayout)
{
	// ensure the overlay does not already exist in the scene
	vector<OverlayLayout *>::iterator iter = overlays.begin();
	while (iter != overlays.end())
	{
		if (*iter == overlayLayout)
		{
			return;
		}
		iter++;
	}

	// add the overlay
	overlays.insert(overlays.begin(), overlayLayout);
	overlayLayout->onSize(Vec3(float(winOS->GetWindowWidth()), float(winOS->GetWindowHeight()), 0));
}

void SceneManager::unregisterOverlay(OverlayLayout * overlayLayout)
{
	// erase the overlay from the registered overlays
	vector<OverlayLayout *>::iterator iter = overlays.begin();
	while (iter != overlays.end())
	{
		if (*iter == overlayLayout)
		{
			overlays.erase(iter);
			return;
		}
		iter++;
	}
}

const vector<OverlayLayout *>& SceneManager::getOverlays() const
{
	return overlays;
}

void SceneManager::renderOverlays(uint flags)
{
#ifdef DXRENDER
#else
	if (!(flags & RenderSkipModelViewChange))
		switchToOrtho();

	glPushAttribToken token(GL_ENABLE_BIT);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
#endif

	Bounds initialBounds;
	initialBounds.setInfinite();

	// keep track of the ones we are ordering manually
	set<OverlayLayout *> manualOrder;
		manualOrder.insert(cursor(NULL, NULL));
		manualOrder.insert(messages());
		manualOrder.insert(nameables());	// skip the actual rendering of the nameables since we do it manually later

	// render all the overlays that *aren't* ordered manually
	for (int i = overlays.size()-1; i >= 0; --i)
	{
		if (manualOrder.find(overlays[i]) == manualOrder.end())
#ifdef DXRENDER
			overlays[i]->onRender(Vec3(0,0,0), initialBounds);
#else
			overlays[i]->onRender();
#endif
	}

	
#ifdef DXRENDER
	// cursor is the next-highest rendered overlay
	cursor(NULL, NULL)->onRender(Vec3(0,0,0), initialBounds);
	// messages are rendered on top
	messages()->getParent()->onRender(Vec3(0,0,0), initialBounds);
#else
	cursor(NULL, NULL)->onRender();
	messages()->getParent()->onRender();
#endif
	
	if (!(flags & RenderSkipModelViewChange))
		switchToPerspective();
}

MessageContainer * SceneManager::messages()
{
	// create the messages container
	if (!messagesContainer)
	{
		messagesContainer = new MessageContainer;
		OverlayLayout * messagesRoot = new OverlayLayout;
			// position the messages to bottom of the screen
			messagesRoot->getStyle().setOffset(Vec3(0.5f, 0.15f, 0));
			messagesRoot->addItem(messagesContainer);
		scnManager->registerOverlay(messagesRoot);
	}
	return messagesContainer;
}

OverlayLayout * SceneManager::windowControls()
{
	// windows controls
	if (!windowControlsContainer)
	{
		windowControlsContainer = new MessageContainer;
		OverlayLayout * windowControlsRoot = new OverlayLayout;
		// position the messages to the top left of the screen
		windowControlsRoot->getStyle().setOffset(Vec3(1.0f, 1.0f, 0));
		windowControlsRoot->addItem(windowControlsContainer);
		scnManager->registerOverlay(windowControlsRoot);
	}
	return windowControlsContainer;
}

MessageContainer * SceneManager::growlMessages()
{
	messages();

	// growl-like messages
	if (!growlContainer)
	{
		growlContainer = new MessageContainer;
		OverlayLayout * growlRoot = new OverlayLayout;
			// position the messages to the top left of the screen
			growlRoot->getStyle().setOffset(Vec3(0.9f, 0.05f, 0));
			growlRoot->addItem(growlContainer);
		scnManager->registerOverlay(growlRoot);
	}
	return growlContainer;
}

NamableOverlayLayout * SceneManager::nameables()
{
	// create the nameables container
	if (!nameablesContainer)
	{
		// workaround to ensure that the nameables are created first
		messages();

		nameablesContainer = new NamableOverlayLayout;
		nameablesContainer->setPosition(Vec3(0,0,0));
		scnManager->registerOverlay(nameablesContainer);
	}
	return nameablesContainer;
}

AbsoluteOverlayLayout * SceneManager::cursor(TextOverlay ** message, OverlayLayout ** layout)
{
	// create the cursor's nameables container
	if (!cursorContainer)
	{
		// workaround to ensure that the nameables are created first
		messages();

		cursorContainer = new NamableOverlayLayout;
		cursorContainer->getStyle().setPadding(AllEdges, 4.0f);
		scnManager->registerOverlay(cursorContainer);
		cursorContainer->setPosition(Vec3(0,0,0));

		// create the message
		HorizontalOverlayLayout * hor = new HorizontalOverlayLayout;
			hor->setAlpha(0.0f);
			hor->getStyle().setBackgroundColor(ColorVal(215, 20, 25, 30));
			hor->getStyle().setPadding(AllEdges, 2.0f);
			hor->getStyle().setPadding(LeftRightEdges, 4.0f);
		TextOverlay * comp = new TextOverlay("");
		QString fontName = themeManager->getValueAsFontFamilyName("ui.message.font.family","");
			comp->setFont(FontDescription(fontName, 14));
		hor->addItem(comp);
		cursorContainer->addItem(hor);
	}
	
	OverlayLayout * l = (OverlayLayout *) cursorContainer->items().front();
	if (layout) 
		*layout = l;
	if (message)
		*message = (TextOverlay *) (l->items().front());

	return cursorContainer;
}

LibraryMenuControl* SceneManager::getLibraryControl() const
{
	return _libraryMenuControl;
}

bool SceneManager::isWorkingDirectoryPrimarilyOfFileType(unsigned int fileSystemTypeMask)
{
	const float fileTypeCountThresholdPct = 0.9f;
	int fileTypeCount = 0;
	for (int i = 0; i < _bumpObjects.size(); ++i)
	{
		if (_bumpObjects[i]->getObjectType() == ObjectType(BumpActor, FileSystem, fileSystemTypeMask))
		{
			++fileTypeCount;
		}
	}
	return (float(fileTypeCount) / _bumpObjects.size()) > fileTypeCountThresholdPct;
}

void SceneManager::setReplayable(Replayable * replay)
{
	// just stop the replay
	if (_replayable)
	{
		if (_replayable->getPlayState() != Replayable::Stopped)
			_replayable->stop();
		assert(_replayable->getPlayState() == Replayable::Stopped);
		SAFE_DELETE(_replayable);
	}

	_replayable = replay;
}

Replayable * SceneManager::getReplayable() const
{
	return _replayable;
}

void SceneManager::setTrainingIntroRunner( TrainingIntroRunner * training )
{
	_trainingIntroRunner = training;
}

TrainingIntroRunner * SceneManager::getTrainingIntroRunner()
{
	return _trainingIntroRunner;
}

bool SceneManager::serializeToPb(PbScene * pbScene)
{
	assert(pbScene);

	// save the working directory
	QString workingDir = native(getWorkingDirectory());
	pbScene->set_working_directory(stdString(workingDir));

	cam->restorePreviousVisibleNameables(true, true);

	// save all the free objects
	{
		BumpObject * tmp = NULL;
		QList<BumpObject *> freeObjects;
		for (int i = 0; i < _bumpObjects.size(); ++i)
		{
			tmp = _bumpObjects[i];
			assert(tmp);
			if (tmp->getParent() && tmp->isParentType(BumpPile)) continue;
			if (tmp->isBumpObjectType(BumpPile)) continue;
			if (tmp->isBumpObjectType(BumpWidget)) continue;			
			if (tmp->isBumpObjectType(BumpCluster)) continue;		
			if (tmp->isBumpObjectType(BumpActor))
			{
				if (((Actor *) tmp)->isActorType(Temporary))
					continue;
			}
			freeObjects.append(tmp);
		}
		
		QListIterator<BumpObject *> iter(freeObjects);
		while (iter.hasNext())
		{
			BumpObject * obj = iter.next();
			PbBumpObject * object = pbScene->add_free_objects();
			if (!obj->serializeToPb(object))
			{
				__debugbreak();
				return false;
			}
#ifdef BTDEBUG
			if (ObjectType::fromUInt(object->type()) == ObjectType(BumpActor, Invisible))
				consoleWrite(QString("(SerializeToPb) Actor is hidden: %1\n").arg(obj->getFullText()));
#endif
		}
	}

	// save all the (soft) piles
	{
		BumpObject * tmp = NULL;
		QList<Pile *> softPiles;
		for (int i = 0; i < _bumpObjects.size(); ++i)
		{
			tmp = _bumpObjects[i];
			if (!tmp->isObjectType(ObjectType(BumpPile, SoftPile))) continue;			
			softPiles.append((Pile *) tmp);
		}

		QListIterator<Pile *> iter(softPiles);
		while (iter.hasNext())
		{
			PbBumpObject * pile = pbScene->add_piles();
			if (!iter.next()->serializeToPb(pile))
			{
				__debugbreak();
				return false;
			}
		}
	}

	cam->storePreviousVisibleNameables(true, true);

	// save the camera
	if (!cam->serializeToPb(pbScene->mutable_camera()))
		return false;

	Singleton<NetworkCookieJar>::getInstance()->serialize();

	return pbScene->IsInitialized();
}

bool SceneManager::deserializeFromPb(const PbScene * pbScene)
{
	assert(pbScene);

	// deserialize the working directory (for debugging mostly)
	if (pbScene->has_working_directory())
	{
		QString workingDir = qstring(pbScene->working_directory());
	}

	// deserialize all the objects
	for (int i = 0; i < pbScene->free_objects().size(); ++i)
	{
		const PbBumpObject& object = pbScene->free_objects(i);
		BumpObject * obj = PbPersistenceManager::getInstance()->deserializeBumpObjectFromPb(&object);
#if BTDEBUG
		assert(obj);
#endif
	}

	// deserialize all the piles
	for (int i = 0; i < pbScene->piles().size(); ++i)
	{
		const PbBumpObject& pile = pbScene->piles(i);
		BumpObject * obj = PbPersistenceManager::getInstance()->deserializeBumpObjectFromPb(&pile);
#if BTDEBUG
		assert(obj);
#endif
	}

	// deserialize the camera
	if (pbScene->has_camera())
	{
		if (!cam->deserializeFromPb(&pbScene->camera()))
			return false;
	}
	
	LOG_LINE_REACHED();
	return true;
}

void SceneManager::setScenePbBumpFile(const QFileInfo& p)
{
	// NOTE: It is possible for the specified path not to exist
	_sceneFile = p;
	_backupSceneFile = QFile(native(_sceneFile) + ".bak");
}

const QFileInfo& SceneManager::getScenePbBumpFile() const
{
	return _sceneFile;
}

const QFileInfo& SceneManager::getBackupScenePbBumpFile() const
{
	return _backupSceneFile;
}

RecentlyFSDeletedFile::RecentlyFSDeletedFile( const Mat34& p, const Vec3 d )
: pose(p)
, dims(d)
{}

RecentlyFSDeletedFile::RecentlyFSDeletedFile()
{}
