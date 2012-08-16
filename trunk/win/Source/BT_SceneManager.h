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

#pragma once

#ifndef _BT_SCENE_MANAGER_
#define _BT_SCENE_MANAGER_

// -----------------------------------------------------------------------------

#include "BT_ObjectType.h"
#include "BT_Singleton.h"
#include "BT_Watchable.h"
#include "BT_Settings.h"
#include "BT_Stopwatch.h"
#include "BT_Nameable.h"
#include "TextPixmapBuffer.h"
#include "BT_LibraryOverlay.h"
#ifdef DXRENDER
	#include "BT_DXRender.h"
#endif

// -----------------------------------------------------------------------------

class FileSystemActor;
class CustomActor;
class NxActorWrapper;
class Pile;
class BumpObject;
struct TaggedList;
class OverlayLayout;
class ImageOverlay;
class MessageContainer;
class AbsoluteOverlayLayout;
class NamableOverlayLayout;
class TextOverlay;
class Replayable;
class TrainingIntroRunner;
class PbScene;
class Library;

// -----------------------------------------------------------------------------

enum Modes
{ 
	None, 
	FanoutOnStrokeMode, 
	MoveMode, 
	CreaseMode, 
	CrumpleMode, 
	InPileShifting, 
	InPileGhosting,
};

// -----------------------------------------------------------------------------

class SharedDesktop : public Nameable
{
public:
	vector<BumpObject *> objects;
	Vec3 offset;
	QString bgTextureOverride;
	NameableOverlay * name;
	ImageOverlay * image;
};

// -----------------------------------------------------------------------------

class RecentlyFSDeletedFile
{
public:
	Vec3 dims;
	Mat34 pose;
	Stopwatch timer;

public:
	RecentlyFSDeletedFile();
	RecentlyFSDeletedFile( const Mat34& p, const Vec3 d );
};

// -----------------------------------------------------------------------------

class SceneManager : public Watchable
{
#ifdef DXRENDER
	weak_ptr<DXRender> _dxRef;
#endif
	vector<BumpObject *> _bumpObjects;
	QHash<QString, RecentlyFSDeletedFile> _recentFSDeletionPositions;

	// overlays
	vector<OverlayLayout *> overlays;
	MessageContainer * growlContainer;
	MessageContainer * messagesContainer;
	OverlayLayout * windowControlsContainer;
	NamableOverlayLayout * nameablesContainer;
	NamableOverlayLayout * cursorContainer;

	// replayables
	Replayable * _replayable;
	TrainingIntroRunner * _trainingIntroRunner;

	// working directory/resources
	QDir currentWorkingDirectory;
	QFileInfo currentSceneFile;
	QFileInfo backupSceneFile;
	QSharedPointer<Library> currentLibrary;

	// Library overlays
	LibraryMenuControl* _libraryMenuControl;

	QFileInfo _sceneFile;
	QFileInfo _backupSceneFile;

	int isRegistered(BumpObject *object);

public:
	// global variables

	int trialDays;

	// Scene saving mutex lock
	mutable QMutex saveSceneMutex;

	// Skip saving scene to file
	bool skipSavingSceneFile; 

	// running in sandbox mode?
	bool isInSandboxMode;

	// running in training mode
	bool isInTrainingMode;

	// testing the update code?
	bool testUpdater;

	// running in shell extension mode?
	bool isShellExtension;

	// running in the special photo-specific mode?
	bool isBumpPhotoMode;

	// For demo & prototype purposes, we can launch a second instance of BumpTop
	bool isChildProcess; // True only in the child process
	QProcess *childProcess; // Pointer from the parent to the child process

	bool runBumpTopTestsOnStartup;
	bool runAutomatedJSONTestsOnStartup;
	bool disableRendering;
	bool skipAllPromptDialogs; // Skip prompts when testing
	bool generateFullDump; // Whether MiniDumps will be a full memory dump when crashes, set using -fullDump

	// TEMP override for zoom to angled bounds speed
	int zoomToAngleBoundsTempOverride;

	// whether AA is supported or not
	bool isMultiSamplingSupported;

	// Skip all animations
	bool skipAnimations;

	// actor data vars
	Vec3 PinPoint;
	bool isInteraction;
	bool touchGestureBrowseMode;
	QString supportedExtensions;

	// old pile
	bool gPause;
	NxPhysicsSDK* gPhysicsSDK;
	NxScene* gScene;
	GlobalSettings settings;

	// util
	TextPixmapBuffer hudTextBuffer;
	bool exitBumpTopFlag;
	bool DrawWalls;
	vector<NxActorWrapper *> activeNxActorList;
	vector<NxActorWrapper*> Walls;
	Vec3List WallsPos;
	float WallHeight;
	vector <NxActorWrapper *> CamWalls;
	vector<BumpObject *> Tossing;
	bool isInInfiniteDesktopMode;
	Modes mode;
	Mat33 straightIconOri;
	int mx, my;
	float factor;
	Vec2 oldMousePos;
	float ZoomBuffer;
	
	// Keyboard Manager
	float maxDelayBetweenKeyDown;	// Used when determining if a string should be reset when calling the onKeyDown event

	// font manager
	float nearClippingPlane;
	float farClippingPlane;

	// texture manager
	StopwatchInSeconds loadingTimer;

	// operating system
	QString texturesDir;
	QString dataDirectory;
	uint generalTimer;
	bool mouseUpTriggered;
	HANDLE consoleWindowHandle;
	bool MouseOverWall;
	NxActorWrapper* PinWall;

	// windows system
	float dblClickInterval;

	// XXX: hackish way to get the single click type event
	bool useSingleClickExpiration;
	StopwatchInSeconds singleClickExpirationTimer;

	// bumptop statics
	vector<BumpObject *> initialDragObjects;
	Vec3 lastEye;
	int widgetFirstMovePosX; //true if first mouse movement after clicking a widget was to the Right. -1 if not set.
	int widgetFirstMovePosY; //true if first mouse movement after clicking a widget was to the Down.  -1 if not set.
	bool SelectionIncludesAPile;
	vector<BumpObject *> gettingShared;
	int mbutton;
	int mstate;
	int mkey;
	StopwatchInSeconds dblClickTimer;
	StopwatchInSeconds sglClickTimer;
	StopwatchInSeconds sglClickOnDownTimer;
	bool startTimer;
	Vec3 sglClickPos; //x,y position of first click in float-click
	NxActorWrapper* lastPicked;
	Vec3List mousePointList;
	bool mouseMoving;
	int shiftTally;
	Vec3List printVecs;
	float dblClickSize;
	float sglClickSize;
	bool disallowMenuInvocation;

	// Defaults to 1.0, but can be changed to make actors bigger by default
	float defaultActorGrowFactor;

	// context menu statics
	IContextMenu2 *g_IContext2;
	IContextMenu3 *g_IContext3;
	WNDPROC OldWndProc;

	// drag and drop statics
	bool isDragInProgress;

	// windows os statics
	int windowXOffset;
	int windowYOffset;

	// actor data statics
	int id_count;	

	// local statics
	float clickDistance;		// BumpTop::MouseCallback
	StopwatchInSeconds Elapsed; // BumpTop::RenderCallback
	int framesPerSecond;
	int avgFramesPerSecond;
	int maxFramesPerSecond; 
	int frameCounter;
	int framesPerSecondCounter;
	bool firstTime;
	bool firstReshape;
	StopwatchInSeconds bindTimer;	
	StopwatchInSeconds timeElapsed;	// MarkingMenu::invoke
	bool checked;				// WindowsSystem::TopLevelFilter
	QDir currentPath;
	QDir themesPath;
	QDir defaultThemePath;
	QDir userThemesPath;
	QDir userDefaultThemePath;
	QDir widgetsPath;			// WindowsSystem::GetWidgetsDirectory
	QDir cachePath;				// WindowsSystem::GetCacheDirectory
	QDir framesPath;			// WindowsSystem::GetFramesDirectory
	QDir texturesPath;			// WindowsSystem::GetTexturesDirectory
	QDir testsPath;				// WindowsSystem::GetTestsDirectory
	QDir languagesPath;			// WindowsSystem::GetLanguagesDirectory
	QDir dataPath;				// WindowsSystem::GetDataDirectory
	QDir trainingPath;			// WindowsSystem::GetTrainingDirectory
	QDir tradeshowPath;			// WindowsSystem::GetTradeshowDirectory
	QDir updatePath;			// WindowsSystem::GetUpdateDirectory
	QDir JSONTestScriptPath;
	QDir JSONTestLogPath;
	QDir JSONTestFilesPath; 
	QString touchDebugLogFilename; // if null, no log will be written
	HWND dialogHwnd;			// WindowsSystem::CreateModelessDialog
	DWORD _threadId;

	bool enableSharingMode;
	bool isInSharingMode;
	vector<SharedDesktop *> sharedDesktops;

	// If non-null, the directory to load a demo scene from, e.g. the Tradeshow dir
	QString startupDemoSceneDir;

	bool isSceneLoaded;

	// Singleton
	friend class Singleton<SceneManager>;
	SceneManager();

public:

	~SceneManager();

	// Actions
	void init();
	void init(QString workingDir);
	void init(QSharedPointer<Library>& library);
	void destroyScene();
	void clearBumpTop();
	void crossReference();

	// Object Registration
	bool addObject(BumpObject *newObject);
	bool containsObject(BumpObject *object);
	bool removeObject(BumpObject *newObject);

	// Events
	void onFileAdded(QString strFileName);
	void onFileRemoved(QString strFileName);
	void onFileNameChanged(QString strOldFileName, QString strNewFileName);
	void onFileModified(QString strFileName);

#ifdef DXRENDER
	void onRelease();
#endif

	// Overlays
	void registerOverlay(OverlayLayout * overlayLayout);
	void unregisterOverlay(OverlayLayout * overlayLayout);
	void renderOverlays(uint flags);
	const vector<OverlayLayout *>& getOverlays() const;
	MessageContainer * messages();
	MessageContainer * growlMessages();
	OverlayLayout * windowControls();
	NamableOverlayLayout * nameables();
	AbsoluteOverlayLayout * cursor(TextOverlay ** message, OverlayLayout ** layout);
	LibraryMenuControl* getLibraryControl() const;

	// Replayable (scnManager does not own them)
	void setReplayable(Replayable * replay);
	Replayable * getReplayable() const;

	void setTrainingIntroRunner(TrainingIntroRunner * training);
	TrainingIntroRunner * getTrainingIntroRunner();

	// Getters
	StrList getWatchDir();
	const vector<BumpObject *>& getBumpObjects();
	vector<BumpObject *> getBumpObjects(QString filePathFilter, const ObjectType& objType, bool isRegex);
	vector<BumpObject *> getBumpObjects(const ObjectType& objType);
	vector<FileSystemActor *> getFileSystemActors(unsigned int fileSystemTypeMask=0, bool ignoreInvisible=false) const;
	vector<FileSystemActor *> getFileSystemActors(QString filePathFilter, bool ignoreNonFreeActors, bool isRegex);
	vector<Pile *> getPiles(bool ignoreEmptyPiles=false);
	template<class T> CustomActor * getCustomActor();

	QDir getWorkingDirectory() const;
	QSharedPointer<Library> getCurrentLibrary() const;
	QFileInfo getSceneBumpFile() const;
	QFileInfo getSceneBumpBackupFile() const;
	vector<BumpObject *> getVisibleBumpActorsAndPiles(bool includeItemsInStackedPiles = false) const;

	bool isWorkingDirectoryPrimarilyOfFileType(unsigned int fileSystemTypeMask);

	// Setters
	void setWorkingDirectory(QDir p);
	void setCurrentLibrary(QSharedPointer<Library>& library);
	void setSceneBumpFile(QFileInfo p);

	// protocol buffers
	void setScenePbBumpFile(const QFileInfo& p);
	const QFileInfo& getScenePbBumpFile() const;
	const QFileInfo& getBackupScenePbBumpFile() const;
	bool serializeToPb(PbScene * scene);
	bool deserializeFromPb(const PbScene * scene);
};

// -----------------------------------------------------------------------------

#include "BT_SceneManager.inl"

// -----------------------------------------------------------------------------

#define scnManager Singleton<SceneManager>::getInstance()

#endif

// -----------------------------------------------------------------------------

// Special Macros for the SceneManager
#define GLOBAL(var) scnManager->var
#define printUnique(key, str) 	scnManager->messages()->addMessage(new Message(key, str, Message::Ok));	
#define printUniqueError(key, str)	scnManager->messages()->addMessage(new Message(key, str, Message::Error));	
#define printStr(str) 	scnManager->messages()->addMessage(new Message(generateUniqueGUID(), str, Message::Ok));	
#define dismiss(key) scnManager->messages()->dismissMessage(key);
