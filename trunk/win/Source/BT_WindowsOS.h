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

#ifndef _BT_WINDOWS_OS_
#define _BT_WINDOWS_OS_

// -----------------------------------------------------------------------------

#include "BT_ColorVal.h"
#include "BT_Singleton.h"
#include "BT_DropHandler.h"
#include "BT_DragHandler.h"
#include "BT_KeyCombo.h"
#include "BT_MousePointer.h"
#include "BT_SmartBoard.h"
#include "BT_Stopwatch.h"

// -----------------------------------------------------------------------------

class Pile;
class ActorUndoData;
class DesktopLock;
class FileSystemActor;
class MousePointer;
class SettingsAppMessageHandler;
class ThreadableUnit;
class Updater;
class Watchable;
class Windows7Multitouch;
class LibraryManager;
 
// -----------------------------------------------------------------------------

// XXX: see CBTRegistryManager
#define BT_REGISTRY_KEY L"SOFTWARE\\Bump Technologies, Inc.\\BumpTop"

// -----------------------------------------------------------------------------

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK GlobalWndProc(HWND Hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
bool EmailCrashReport(QString recvAddress, QString title, QString body, QString recipientEmail, QString attachmentFilePath, bool onlyDXDiag = false);

// -----------------------------------------------------------------------------

typedef BOOL (WINAPI *MINIDUMPWRITEDUMP)(HANDLE hProcess, DWORD dwPid, HANDLE hFile, MINIDUMP_TYPE DumpType,
										 CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
										 CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
										 CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam
										 );

// -----------------------------------------------------------------------------

// BumpTop QApplication
class BumpTopApplication : public QApplication 
{
public:
	BumpTopApplication(int &argc, char**argv);

	// delegate some of the windows events to the actual bumptop window
	bool winEventFilter(MSG *msg, long *result);
};

// -----------------------------------------------------------------------------

struct iconDetails
{
	QString iconName;
	int x;
	int y;
	unsigned long index;
	bool VIRTUALFolder;

	iconDetails(QString name = QString(), int xLoc = 0, int yLoc = 0, unsigned long indx = 0, bool vFolder = false)
	{
		iconName = name;
		x = xLoc;
		y = yLoc;
		index = indx;
		VIRTUALFolder = vFolder;
	}
};

// -----------------------------------------------------------------------------

// Items to be checked in the main loop
enum ConditionalFlags
{
	SignificantChange		= 1,
	ThreadSafeProcessing	= 2
};

// -----------------------------------------------------------------------------

enum FileLocations
{
	Desktop				= 0x0000,
	InternetExplorer	= 0x0001,
	ControlPanel		= 0x0003,
	MyDocuments			= 0x0005,
	MyPictures			= 0x0027,
	RecycleBin			= 0x000a,
	// see: http://blogs.msdn.com/oldnewthing/archive/2009/07/30/9852685.aspx#comments
	//   as to why we should not be using CSIDL_DESKTOPDIRECTORY
	DesktopDirectory	= 0x0010,
	MyComputer			= 0x0011,
	NetworkNeighborhood	= 0x0012,
	FontsDirectory		= 0x0014,
	AllUsersDesktopDir	= 0x0019,
	UserApplicationData	= 0x001c,
	ProgramFiles		= 0x0026,
	RecentlyUsedDocuments = CSIDL_RECENT
};

// -----------------------------------------------------------------------------

enum WindowState
{
	ShellExt,		// In explorer
	Windowed,		// bordered, !fullscreen
	FullWindow,		// bordered, fullscreen
	WorkArea,		// !bordered, !fullscreen
	FullScreen		// !bordered, fullscreen	
};

// -----------------------------------------------------------------------------

enum WindowsOSVersion
{
	WindowsXP,
	WindowsVista,
	Windows7
};

// -----------------------------------------------------------------------------

enum BumpTopEdition
{
	UndefinedEdition,
	Standard,
	VIP,
};

// -----------------------------------------------------------------------------

// workaround to allow for translated strings
class WindowsOSStrings
{
	Q_DECLARE_TR_FUNCTIONS(WindowsOSStrings)

	QHash<QString, QString> _strs;

private:
	friend class Singleton<WindowsOSStrings>;
	WindowsOSStrings();

public:
	QString getString(QString key);
};
#define WinOSStr Singleton<WindowsOSStrings>::getInstance()

// -----------------------------------------------------------------------------

class WindowsSystem
{
	Q_DECLARE_TR_FUNCTIONS(WindowsSystem)

	// Private Members
	bool exitFlag;
	
	// window properties
	QHash<QString, MONITORINFOEX> monitors;
	WindowState				windowState;
	WindowState				postSplashscreenWindowState; 
	RECT					prevWindowedRect;
	bool					skipFullScreenWindowState;
	UINT_PTR				windowTimerCallback;
	UINT_PTR				statsManagerUploadCheckTimerCallback;
	UINT_PTR				checkForToggleDesktopTimerCallback;
	UINT_PTR				reauthorizationTimerCallback;
	UINT_PTR				virtualIconSynchronizationTimerCallback;

	DesktopLock				*desktopLock;

	// Multitouch & multi-mouse support
#ifdef SMART_SUPPORT
	SmartBoardHandler		smartBoard;
#endif
	Windows7Multitouch		*windows7Multitouch;
	LibraryManager			*libraryManager;
	
	// win32
	HINSTANCE				hInstance;
	HWND					windowHwnd;
	HWND					topLevelWindowHwnd;
	HWND					parentHwnd;
	HWND					shellExtProxyHwnd;
	RECT					parentRect;
	ULONG_PTR				gdiplusToken;
	
	// recycler
	UINT_PTR				hTimerCallback;
	uint					fpsTimer, fpsCurTimer;
	QMutex					_memberMutex;
	bool					_isRecycleBinFull;
	bool					_recycleBinStateChanged;
	ThreadableUnit *		_recyclerThread;

	// drag and drop
	DragHandler				dragHandler;
	DropHandler				dropHandler;
	int						numDroppedFilesFromLastDrag;	// whether the last drop point was from a user drag

	// misc
    IImageList				*largeIcons;
	bool					winMinimized;
	bool					winActive;
	bool					isLoaded;
	int						timerAnchor;
	int						conditionalFlags;
	bool					largeIconSupport;
	long					uniqueIdCounter;
	BumpTopEdition			_bumptopEdition;

	bool					_ignoreGDI;

	QHash<QString, int>		_virtualIconNameTypeMap;
	QHash<int, QString>		_virtualIconTypeNameMap;
	QHash<int, QString>		_virtualIconTypePathMap;

	MousePointer*			defaultMousePointer;
	UINT					BUMPTOP_UPDATE_DOWNLOADED; // constant for a bumptop-specific registered message

	// updater stuff
	Updater *				_updater;
	boost::thread*			_updaterThread;

	// settings app communication
	SettingsAppMessageHandler* _settingsAppHandler;

	// splash screen
	QSplashScreen *			_splashScreen;

	// interaction stats timing 
	StopwatchInSeconds		accumulatedInterationTime;
	UINT_PTR				interactionTimeOutTimer;
	UINT_PTR				screenshotTimer;
	void					startAccumulatingInteractionTimer();
	void					stopAccumulatingInteractionTimer();

	// Private Functions
	HICON					GetSystemIcon(QString FilePath, int indx);
	Gdiplus::Bitmap			*GetIconPixelData(HICON Icon);
	HWND					FindListView();
	HANDLE					FindExplorerProcess(HWND slaveHwnd);
	bool					QueryDataObject(IDataObject *dataObject);
	void					ResizeWindow();

	bool					SetAbsoluteWindowPos(HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags);

	// Hander Functions
	bool Create(TCHAR *cmdLine);
	void Destroy();
	void OnSize();
	void OnRotate(int newWidth, int newHeight);
	void OnKeyUp(WPARAM wParam);
	void OnKeyDown(WPARAM wParam);
	void OnSysKeyUp();
	void OnSysKeyDown();
	void OnSetCursor();
	void OnTimer(WPARAM wParam);

	friend class Singleton<WindowsSystem>;
	friend class EventManager;
	friend class Windows7Multitouch;

	WindowsSystem();

public:
	~WindowsSystem();

	enum WallPaperStyle {
		CENTER,
		FILL,
		STRETCH,
		FIT,
		TILED,
		UNABLETOREAD
	};

	// Static Public Functions
	static LRESULT CALLBACK GlobalMsgProc(int nCode, WPARAM wParam, LPARAM lParam);
	static BOOL CALLBACK EnumAndRefreshMonitors(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);
	static DWORD WINAPI WatchingThread(LPVOID param);

	// Public Functions
	bool			Init(HINSTANCE hInst, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow);
	void			ReRegisterDropHandler();
	void			DestroyBTWindow();
	bool			CreateBTWindow();
	bool			ParseCommandLine(TCHAR * cmdLine);
	LRESULT			MessageProc(HWND Hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void			Render();
	void			ToggleFullScreen();
	void			ToggleWindowMode();
	static void		ToggleShowWindowsDesktop();
	void			ExitBumpTop();
	void			AsyncExitBumpTop();
	bool			InstallUpdate();
	void			checkForUpdate(); // Creates a thread and calls Updater::run.
	bool			RenameSoftPile(Pile * softPile);
	bool			RenameFile(FileSystemActor *actor, QString newName);
	bool			IsInShowDesktopMode();
	void			StartDragOnCurrentSelection();
	void			attachToDesktopWindow();
	static BOOL CALLBACK EnumProc(HWND hwnd, LPARAM lp);
	void			detatchFromDesktopWindow();
	bool			isAttachedToDesktopWindow();
	bool			isTopMostDesktopWindowChild();
	void			makeTopWindow();
	HBITMAP			ExtractThumbnail_Vista(QString path, IThumbnailCache ** ptcOut, IShellItem ** psiOut, ISharedBitmap ** psbOut);
	void			LoadSettingsFile();
	void			SaveSettingsFile();
	bool			IsFileInUse(QString file);
	void			FailOnDrivers();
	void			FailOnBpp();
	bool			ignoreGDI(); // returns whether check for GDI renderer should be bypassed - for internal VM testing
	void			ignoreGDI(bool value); // sets whether check for GDI renderer should be bypassed - for internal VM testing
	void			SplashScreenIntro(bool startSplashScreen);


	// Getter Functions
	QString			GetFileTypeDescription(QString filename);
	Gdiplus::Bitmap	*GetIconGraphic(QString Path);
	Gdiplus::Bitmap	*GetIconGraphic(int Loc);
	QString			GetSystemIconInfo(QString FileName);
	QString			GetSystemPath(int SystemLocation);
	QDir			GetUserApplicationDataPath();
	QDir			GetUserLocalApplicationDataPath();
	HWND			GetWindowsHandle();
	QString			GetUIDFromVirtualIcon(int icon);
	void			GetIconAvailability(int IconType, bool& isVisibleOut);
	bool			SetIconAvailability(int IconType, bool visible);
	bool			GetIconPositions(vector<iconDetails> &icons);
	vector<iconDetails>			GetIconPositionsHelper();
	vector<iconDetails>			GetIconPositionsHelper64();
	QString			GetFileNameFromIcon(int SpecialIcon);
	int				GetIconTypeFromFileName(QString filename);
	void			GetMousePosition(int &x, int &y);
	int				GetWindowWidth();
	int				GetWindowHeight();
	int				GetMonitorWidth();
	int				GetMonitorHeight();
	int				GetWindowDPIX();
	int				GetWindowDPIY();
	int				GetWindowBpp();
	void			GetWorkArea(int &r, int &l, int &t, int &b);
	void			GetMainMonitorCoords(int &x, int &y);
	void			LoadDesktopTexture();
	ColorVal		GetWindowsBackgroundColor();
	WallPaperStyle	GetWallPaperStyle();
	QString			GetWallPaperPath();
	bool			GetConditionalFlag(int flag);
	QString			GetUniqueID();
	void			GetLargeIconSupport(bool forceLowResOnVista=false);
	QString			GetBuildNumber();
	BumpTopEdition	GetBumpTopEdition();
	QString			BumpTopEditionName(BumpTopEdition edition);
	void			SetDropPoint(const POINT & point, unsigned int expectedCount); // only called after file operations to set position for new file actors
	Vec3			GetDropPoint(int& numDroppedFilesRemainingOut, float yAxis = 100);
	void			SetLastDropPointFromDrag(unsigned int numFilesDropped);
	QFileInfo		GetExecutablePath();
	QDir			GetExecutableDirectory();
	QDir			GetThemesDirectory(bool appendDefaultDirectory=false);
	QDir			GetUserThemesDirectory(bool appendDefaultDirectory=false);
	QDir			GetWidgetsDirectory();
	QDir			GetCacheDirectory();
	QDir			GetFramesDirectory();
	QDir			GetTexturesDirectory();
	QDir			GetTestsDirectory();
	QDir			GetLanguagesDirectory();
	QDir			GetDataDirectory();
	QDir			GetTrainingDirectory();
	QDir			GetTradeshowDirectory();
	QDir			GetUpdateDirectory();
	QDir			GetStatsDirectory();
	QDir			GetSystemDirectory();
	static bool		IsWindowTopMost();
	static bool		IsGrandfatherForegroundWindow();
	static void		BringWindowToForeground();
	static void		SetFocusOnWindow();
	bool			IsFullscreen() const;
	IShellFolder2 * GetShellFolderFromAbsDirPath(QString p);
	LPITEMIDLIST	GetAbsolutePidlFromAbsFilePath(QString p);
	LPITEMIDLIST 	GetRelativePidlFromAbsFilePath(QString p);
	WindowState		GetWindowState() const;
	void			SetWindowState(WindowState newWindowState);
	HINSTANCE		GetInstanceHandle() const;
	bool			IsKeyDown(KeyboardValues key);
	bool			IsButtonDown(MouseButtons button);
	HWND			GetShellExtProxyWindow();
	void			ShellExtBrowseToChild(QString path);
	void			ShellExtUpdateStatusBar();
	LPITEMIDLIST	GetPidlFromName(int SystemLocation);
	unsigned int	GetTime() const;
	bool			IsWindowsVersion(WindowsOSVersion version);
	bool			IsWindowsVersionGreaterThanOrEqualTo(WindowsOSVersion version);
	OSVERSIONINFO	GetOSVersionInfo();
	bool			Is64BitWindows();
	void			RelaunchBumpTop();
	bool			isInvalidBitmap(Gdiplus::Bitmap * bitmap);
	bool			isInvalidBitmap(const BITMAP& bitmap);
	void			MoveToNextMonitor();
	QFileInfo		CreateDxDiagLog();
	vector<QString>	GetGraphicsCardIDs();
	SettingsAppMessageHandler* GetSettingsAppMessageHandler();
	bool			QueryRecycleBin();
	QString			GetLocaleLanguage();
	Windows7Multitouch *GetWindows7Multitouch();
	LibraryManager* GetLibraryManager();
	bool				GetShortPathName(QString fullPathName, QString &shortPathName);
	QString			GetSharingWidgetPath();
	
	void OnMouseMove(int x, int y, WPARAM wParam);
	void OnMouse(UINT uMsg, int x, int y, WPARAM wParam);

	// System specific
	void			onPowerSuspend();
	void			onPowerResume();

	// Setter Functions
	void			SetConditionalFlag(int flag, bool value);

	// registry
	DWORD			getRegistryDwordValue(QString valueName);
	DWORD			getRegistryDwordValue(QString valueName, bool& valueExistsOut);
	DWORD			getRegistryDirectDwordValue(QString path, QString valueName, bool& valueExistsOut);
	void			setRegistryDwordValue(QString valueName, DWORD value);
	QString			getRegistryStringValue(QString valueName);
	void			setRegistryStringValue(QString valueName, QString value);

	// guid
	QString			GetGUID();
};

// -----------------------------------------------------------------------------

#define winOS Singleton<WindowsSystem>::getInstance()

// -----------------------------------------------------------------------------

#else
	struct iconDetails;
	class WindowsSystem;
	enum KeyboardValues;
	enum ConditionalFlags;
	enum FileLocations;
	enum WindowState;
#endif
