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
#include "BT_Authorization.h"
#include "BT_DesktopLock.h"
#include "BT_DialogManager.h"
#include "BT_DragDrop.h"
#include "BT_EventManager.h"
#include "BT_ExplorerAliveChecker.h"
#include "BT_FileSystemActor.h"
#include "BT_FileSystemActorFactory.h"
#include "BT_FileSystemManager.h"
#include "BT_FileSystemPile.h"
#include "BT_FileTransferManager.h"
#include "BT_Flyout.h"
#include "BT_FontManager.h"
#include "BT_GLTextureManager.h"
#include "BT_KeyboardManager.h"
#include "BT_LassoMenu.h"
#include "BT_Logger.h"
#include "BT_MarkingMenu.h"
#include "BT_MenuActionManager.h"
#include "BT_MouseEventManager.h"
#include "BT_MousePointer.h"
#include "BT_MultipleMice.h"
#include "BT_OverlayComponent.h"
#include "BT_Pile.h"
#include "BT_Profiler.h"
#include "BT_RenderManager.h"
#include "BT_SettingsAppMessageHandler.h"
#include "BT_SceneManager.h"
#include "BT_Selection.h"
#include "BT_SlideShow.h"
#include "BT_StatsManager.h"
#include "BT_SVNRevision.h"
#include "BT_SystemTray.h"
#include "BT_TextManager.h"
#include "BT_ThemeManager.h"
#include "BT_ThreadableUnit.h"
#include "BT_Training.h"
#include "BT_UpdateInstaller.h"
#include "BT_UpdateServer.h"
#include "BT_Updater.h"
#include "BT_Util.h"
#include "BT_WebActor.h"
#include "BT_WidgetManager.h"
#include "BT_Windows7Multitouch.h"
#include "BT_LibraryManager.h"
#include "BT_WindowsOS.h"
#include "BumpTop.h"
#include "EnumProc.h"
#include "BT_WindowsHelper.h"
#include "BT_Camera.h"
#include "BT_AnimationManager.h"
#include "BT_AnimationEntry.h"

#ifdef DXRENDER
#include "BT_DXRender.h"
#endif

// BumpTop QApplication
BumpTopApplication::BumpTopApplication(int &argc, char**argv)
: QApplication(argc, argv) 
{}

bool BumpTopApplication::winEventFilter(MSG *msg, long *result)
{
	switch (msg->message)
	{
	case WM_QUERYENDSESSION:
	case WM_ENDSESSION:
		*result = winOS->MessageProc(msg->hwnd, msg->message, msg->wParam, msg->lParam);
		return true;
	}
	return false;
}


// +--------------------------------------------------------------------------+
// |                                                                          |
// |                     Initialization And Error Checking	                  |
// |                                                                          |
// +--------------------------------------------------------------------------+

bool PostCrashReport(QString subject, QString body, QString userEmail, QString statsPath, QString dxdiagPath, QString dumpPath, QString themePath, QString settingsPath, QString logPath)
{
	QHash<QString, QString> params;
	QSet<QString> areFileParams;
	QString serverAddr = QT_NT("http://feedback.bumptop.com/stats_upload/uploadFeedback.php");

	// fill the params
	params.insert("subject", subject);
	params.insert("userEmail", userEmail);
	params.insert("body", body);

	// mark which of the params are files
	if (!statsPath.isEmpty())
	{
		params.insert("statsFile", statsPath);
		areFileParams.insert("statsFile");
	}
	if (!dxdiagPath.isEmpty())
	{
		params.insert("dxdiagFile", dxdiagPath);
		areFileParams.insert("dxdiagFile");
	}
	if (!dumpPath.isEmpty())
	{
		params.insert("dumpFile", dumpPath);
		areFileParams.insert("dumpFile");
	}
	if (!themePath.isEmpty())
	{
		params.insert("themeFile", themePath);
		areFileParams.insert("themeFile");
	}
	if (!settingsPath.isEmpty())
	{
		params.insert("settingsFile", settingsPath);
		areFileParams.insert("settingsFile");
	}
	if (!logPath.isEmpty())
	{
		params.insert("logFile", logPath);
		areFileParams.insert("logFile");
	}

	// post the email
	FileTransfer ft(FileTransfer::Upload, NULL);
		ft.setUrl(serverAddr);
		ft.setParams(params);
		ft.setFileParamKeys(areFileParams);
		ft.setLogin(QT_NT("bumptop_stats_upload"));
		ft.setPassword(QT_NT("free57love"));
		ft.setTemporaryString();
		ft.setTimeout(60);
	bool succeeded = ftManager->addPostTransfer(ft, true) && (ft.getTemporaryString().trimmed().startsWith("ok"));
	return succeeded;
}

bool EmailCrashReport(QString recvAddress, QString title, QString body, QString userEmail, QString dumpPath, bool onlyDXDiag)
{
#ifndef DISABLE_PHONING
	if (!ftManager->hasInternetConnection())
		return false;

	QMessageBox progressDialog(QMessageBox::Information, QT_TRANSLATE_NOOP("WindowsOSStrings", "Please wait..."), QT_TRANSLATE_NOOP("WindowsOSStrings", "Sending the info along to BumpTop HQ..."));
	
	// show the notification
	if (!onlyDXDiag)
	{
		progressDialog.setWindowModality(Qt::NonModal);
		progressDialog.show();
		progressDialog.button(QMessageBox::Ok)->hide();
		qApp->processEvents();
	}
	
	QString userName = QT_NT("[Reporter] ") + userEmail;
	QString encoding = QT_NT("text/html");
	QString subject = title + QString(QT_NT(" rev%1 - ")).arg(SVN_VERSION_NUMBER);

	// enter some defaults if none entered
	if (userEmail.isEmpty())
	{
		userEmail = QT_NT("noreply@bumptop.com");
		userName = QT_NT("BumpTop Reporter");
	}

	// convert to html newlines for the message
	body.replace("\n", "<br/>");

	// append the pro invite key to the subject
	if ((GLOBAL(settings).freeOrProLevel == AL_PRO) &&
		!GLOBAL(settings).proInviteCode.isEmpty())
		subject += QString(" [%1]").arg(GLOBAL(settings).proInviteCode);

	// append the manf participant id#
#ifdef HWMANFDEMOMODE
#error Not tracking participant IDs anymore
	subject.append("tbone study participant id# ");
	subject.append(QString::number(GLOBAL(settings).hwManParticipantID));
#endif

	// load the stats into memory
	QString statsPathStr;
	if (!onlyDXDiag)
	{
		statsManager->saveStats();
		QString statsAttachment = statsManager->getStatFileName();
		QFileInfo statsPath(winOS->GetStatsDirectory(), statsAttachment);
		if (exists(statsPath))
			statsPathStr = native(statsPath);
	}	
	// load the dxdiag into memory
	QString dxdiagContents;
	QString dxDiagOutPathStr;
	QFileInfo dxDiagOutPath = winOS->CreateDxDiagLog();
	if (exists(dxDiagOutPath))
		dxDiagOutPathStr = native(dxDiagOutPath);

	// load the themes path
	QString themesPathStr;
	QString	themesPath;
	if (!onlyDXDiag)
	{
		QDir userDefaultThemePath = winOS->GetUserThemesDirectory(true);
		QFileInfo userDefaultThemePropertiesPath = make_file(userDefaultThemePath, "theme.json");
		themesPath = native(userDefaultThemePropertiesPath);
		if (exists(themesPath))
			themesPathStr = native(themesPath);
	}
	// load the settings path
	QString settingsPathStr;
	QString settingsPath;
	if (!onlyDXDiag)
	{
		settingsPath = native(make_file(winOS->GetDataDirectory(), "settings.json"));
		if (exists(settingsPath))
			settingsPathStr = native(settingsPath);
	}
	// load the legacy scene.legacy.bump files
	QString legacyScenePathStr;
	QString legacyScenePath;
	if (!onlyDXDiag)
	{
		legacyScenePath = native(make_file(winOS->GetDataDirectory(), "scene.legacy.bump"));
		if (exists(legacyScenePath))
			legacyScenePathStr = native(legacyScenePath);
	}
	// load the log path
	QString logPath;
	logger->closeLog();
	if (!onlyDXDiag && exists(logger->getFilepath()))
		logPath = native(logger->getFilepath());

	// try and post the crash report first
	bool successful = PostCrashReport(subject, body, userEmail, statsPathStr, dxDiagOutPathStr , dumpPath, themesPath, settingsPath, logPath);
#if 0
	if (!successful)
	{
		// if failed to post, then try and email it
		CSmtp SmtpServer;

		// set the mail to address
		CSmtpAddress SmtpAddress;
			SmtpAddress.Address = (LPCWSTR) recvAddress.utf16();

		// set the sender info
		CSmtpAddress SmtpSender;
			SmtpSender.Address = (LPCWSTR) userEmail.utf16();
			SmtpSender.Name = (LPCWSTR) userName.utf16();

		// set the body
		CSmtpMessageBody SmtpBody;
			SmtpBody = (LPCWSTR) body.utf16();
			SmtpBody.Encoding = (LPCWSTR) encoding.utf16();

		// setup the message
		CSmtpMessage SmtpMsg;
			SmtpMsg.MimeType = mimeRelated;
			SmtpMsg.Message.Add(SmtpBody);
			SmtpMsg.Recipient = SmtpAddress;
			SmtpMsg.Sender = SmtpSender;
			SmtpMsg.Subject = (LPCWSTR) subject.utf16();

		// attach the file specified
		if (!dumpPath.isEmpty())
		{
			CSmtpAttachment SmtpAttach;
				SmtpAttach = (LPCWSTR) dumpPath.utf16();
				SmtpAttach.ContentId = QT_NT(L"file_1234567");
			SmtpMsg.Attachments.Add(SmtpAttach);
		}

		// attach the stats file
		if (!statsPathStr.isEmpty())
		{
			CSmtpAttachment SmtpAttach;
				SmtpAttach = (LPCWSTR) statsPathStr.utf16();
				SmtpAttach.ContentId = QT_NT(L"stats_1234567");
			SmtpMsg.Attachments.Add(SmtpAttach);
		}

		// attach the dxdiag file
		if (!dxDiagOutPathStr.isEmpty())
		{
			CSmtpAttachment SmtpAttach;
				SmtpAttach = (LPCWSTR) dxDiagOutPathStr.utf16();
				SmtpAttach.ContentId = QT_NT(L"dxdiag_1234567");
			SmtpMsg.Attachments.Add(SmtpAttach);
		}

		// attach the theme file
		if (!themesPathStr.isEmpty())
		{
			CSmtpAttachment SmtpAttach;
				SmtpAttach = (LPCWSTR) themesPathStr.utf16();
				SmtpAttach.ContentId = QT_NT(L"theme_1234567");
			SmtpMsg.Attachments.Add(SmtpAttach);
		}

		// attach the setting file
		if (!settingsPathStr.isEmpty())
		{
			CSmtpAttachment SmtpAttach;
				SmtpAttach = (LPCWSTR) settingsPathStr.utf16();
				SmtpAttach.ContentId = QT_NT(L"settings_1234567");
			SmtpMsg.Attachments.Add(SmtpAttach);
		}

		// attach the legacy settings file if there is one
		if (!legacyScenePathStr.isEmpty())
		{
			CSmtpAttachment SmtpAttach;
				SmtpAttach = (LPCWSTR) legacyScenePathStr.utf16();
				SmtpAttach.ContentId = QT_NT(L"legacy_scene_1234567");
			SmtpMsg.Attachments.Add(SmtpAttach);
		}

		// attach the log file if there is one
		if (!logPath.isEmpty())
		{
			CSmtpAttachment SmtpAttach;
				SmtpAttach = (LPCWSTR) logPath.utf16();
				SmtpAttach.ContentId = QT_NT(L"log_1234567");
			SmtpMsg.Attachments.Add(SmtpAttach);
		}

		// send the email
		SmtpServer.Connect(L"mail.bumptop.com", L"bugs@bumptop.com", L"killbugs");
			bool emailSent = SmtpServer.SendMessage(SmtpMsg) == 0 ? true : false;
		SmtpServer.Close();

		// clear the dialog
		progressDialog.hide();

		return emailSent;
	}
#endif
	
	// clear the dialog
	progressDialog.hide();
#endif
	return true;
}

LONG WINAPI TopLevelFilter(struct _EXCEPTION_POINTERS *pExceptionInfo)
{
	LONG retval = EXCEPTION_CONTINUE_SEARCH;
	HWND hParent = NULL;
	HMODULE hDll = NULL;
	QString dumpFileName;
	QString errorMessage;
	MINIDUMPWRITEDUMP pDump;
	bool readyToEmail = false;
	bool restartBumpTop = true;
	
	if (winOS->getRegistryDwordValue("ShutdownIncomplete"))
	{
		// if the previous shutdown was unsuccessful (due to an error) and
		// we've likely already shown the error dialog, so just let the user know
		// and shut down
		::MessageBox(GetForegroundWindow(), (LPCWSTR)QT_TRANSLATE_NOOP("WindowsOSStrings", "BumpTop has crashed.\n\n"
			"You can find the Bump.dump file associated with the crash in the BumpTop folder in your Application Data directory.").utf16(),
			(LPCWSTR)QT_TRANSLATE_NOOP("WindowsOSStrings", "Crash").utf16(), MB_ICONERROR);
		winOS->setRegistryDwordValue("ShutdownIncomplete", 0);
		return 0;
	}

	if (!GLOBAL(checked))
	{
		GLOBAL(checked) = true;
		winOS->SetConditionalFlag(ThreadSafeProcessing, true);

		dumpFileName = native(QFileInfo(winOS->GetDataDirectory(), "Bump.dmp"));

		// Load the Debug Help library to help us with a stack trace
		hDll = LoadLibrary(L"DBGHELP.DLL");

		if (hDll)
		{
			// Get the address of the MiniDumpWriteDump() function inside the library
			pDump = (MINIDUMPWRITEDUMP) GetProcAddress(hDll, "MiniDumpWriteDump");

			if (pDump)
			{
				// create the file
				HANDLE hFile = CreateFile((LPCWSTR) dumpFileName.utf16(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
				if (hFile != INVALID_HANDLE_VALUE)
				{
					_MINIDUMP_EXCEPTION_INFORMATION ExInfo = {0};

					ExInfo.ThreadId = GetCurrentThreadId();
					ExInfo.ExceptionPointers = pExceptionInfo;
					ExInfo.ClientPointers = NULL;

					if (GLOBAL(generateFullDump))
					{
						QString fullDumpPath = native(winOS->GetDataDirectory() / "fullBumpDump.dmp");
						HANDLE fullDumpFile = CreateFile((LPCWSTR)fullDumpPath.utf16(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
						if (INVALID_HANDLE_VALUE != fullDumpFile)
							if (pDump(GetCurrentProcess(), GetCurrentProcessId(), fullDumpFile, MINIDUMP_TYPE(MiniDumpWithFullMemory | MiniDumpIgnoreInaccessibleMemory), &ExInfo, NULL, NULL))
								LOG("Full crash dump written to " + fullDumpPath);
					}
					
					// Write the information into the dump
					if (pDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpWithIndirectlyReferencedMemory, &ExInfo, NULL, NULL))
					{
						readyToEmail = true;
						retval = EXCEPTION_EXECUTE_HANDLER;
					}else{
						errorMessage = QT_TRANSLATE_NOOP("WindowsOSStrings", "Failed to save dump file to Bump.dmp.");
					}
					
					CloseHandle(hFile);

#if DISABLE_PHONING									
					dlgManager->clearState();
					dlgManager->setPrompt(QT_TRANSLATE_NOOP("WindowsOSStrings", "Boom! BumpTop experienced an error.\nPlease restart the application."));
					dlgManager->promptDialog(DialogOK);
#else
					if (readyToEmail)
					{
						dlgManager->clearState();	
					#ifdef HWMANFDEMOMODE
						QString prompt = WinOSStr->getString("BumpTopErrorSafe");
					#else
						QString prompt = WinOSStr->getString("BumpTopError");
					#endif
						dlgManager->setPrompt(prompt);
						dlgManager->setCaption(QT_TRANSLATE_NOOP("WindowsOSStrings", "BumpTop - Submit Crash Report"));

						// load, update, and save stats, incrementing number of crashes
						// not using the global singleton instance because memory might have been corrupted
						StatsManager statsManagerForCrash;
						statsManagerForCrash.loadStats();
						statsManagerForCrash.getStats().bt.crashes++;
						statsManagerForCrash.saveStats();

						// Logic is handled inside BT_CrashDialog
						dlgManager->setHasParent(false);
						dlgManager->promptDialog(DialogCrash);
						restartBumpTop = false;

						// clean up the system tray
						sysTray->hideTrayIcon();
					}
#endif
				}else{
					errorMessage = QT_TRANSLATE_NOOP("WindowsOSStrings", "Cannot create Bump.dump. File I/O error.");
				}

			}else{
				errorMessage = QT_TRANSLATE_NOOP("WindowsOSStrings", "Failed to attach to MiniDumpWriteDump(). DBGHELP.DLL might be too old.");
			}
		}else{
			errorMessage = QT_TRANSLATE_NOOP("WindowsOSStrings", "DBGHELP.DLL not found.");
		}
		
		if (errorMessage.size() > 0)
		{
			dlgManager->clearState();
			dlgManager->setPrompt(errorMessage);
			dlgManager->setCaption(QT_TRANSLATE_NOOP("WindowsOSStrings", "BumpTop Error Reporting"));
			dlgManager->promptDialog(DialogOK);
		}

		// restart bumptop 
		winOS->ExitBumpTop();
		if (restartBumpTop && !scnManager->isShellExtension)
		{			
			winOS->RelaunchBumpTop();
		}		
	}

	return retval;
}

// Temporary class
struct LaunchProSiteOverlayHandler : public MouseOverlayEventHandler
{
	virtual bool onMouseDown(MouseOverlayEvent& mouseEvent) { return true; }
	virtual bool onMouseUp(MouseOverlayEvent& mouseEvent)
	{
		launchBumpTopProPage("buyProOverlay");
		SetCursor(LoadCursor(NULL, IDC_ARROW));
		return false;
	}
	virtual bool onMouseMove(MouseOverlayEvent& mouseEvent) 
	{
		float alpha = mouseEvent.getTarget()->getStyle().getAlpha();
		if (mouseEvent.intersectsTarget())
		{
			if ((1.0f - mouseEvent.getTarget()->getStyle().getAlpha()) > 0.005f)
				mouseEvent.getTarget()->setAlphaAnim(alpha, 1.0f, 25);
			SetCursor(LoadCursor(NULL, IDC_HAND));
		}
		else
		{
			if ((mouseEvent.getTarget()->getStyle().getAlpha() - 0.5f) > 0.005f)
				mouseEvent.getTarget()->setAlphaAnim(alpha, 0.5f, 25);
			SetCursor(LoadCursor(NULL, IDC_ARROW));
		}

		return false; 
	}
};

struct MinimizeBumpTopWindowToTrayOverlayHandler : public MouseOverlayEventHandler
{
	virtual bool onMouseDown(MouseOverlayEvent& mouseEvent) { return true; }
	virtual bool onMouseUp(MouseOverlayEvent& mouseEvent)
	{		
		// hide the bumptop window 
		ShowWindow(winOS->GetWindowsHandle(), SW_HIDE);		

		SetCursor(LoadCursor(NULL, IDC_ARROW));
		return true;
	}
	virtual bool onMouseMove(MouseOverlayEvent& mouseEvent) 
	{
		float alpha = mouseEvent.getTarget()->getStyle().getAlpha();
		if (mouseEvent.intersectsTarget())
		{
			if ((1.0f - mouseEvent.getTarget()->getStyle().getAlpha()) > 0.005f)
				mouseEvent.getTarget()->setAlphaAnim(alpha, 1.0f, 15);
			// TODO: set the cursor
			SetCursor(LoadCursor(NULL, IDC_HAND));
		}
		else
		{
			if ((mouseEvent.getTarget()->getStyle().getAlpha() - 0.25f) > 0.005f)
				mouseEvent.getTarget()->setAlphaAnim(alpha, 0.25f, 15);			
			// TODO: disable the cursor
			SetCursor(LoadCursor(NULL, IDC_ARROW));
		}

		return false; 
	}
};

inline void forceUnhandledException() 
{
	__debugbreak();
    int*z = 0; *z=13; 
}

inline void signalHandler(int)
{
	forceUnhandledException();
}

inline void __cdecl invalidParameterHandler(const wchar_t *, const wchar_t *, const wchar_t *, unsigned int, uintptr_t)
{
	forceUnhandledException();
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// Initialize COM and OLE
	OleInitialize(NULL);
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	// initialize common controls
	INITCOMMONCONTROLSEX cc = {0};
	cc.dwSize = sizeof(INITCOMMONCONTROLSEX);
	cc.dwICC = ICC_STANDARD_CLASSES | ICC_WIN95_CLASSES;
	InitCommonControlsEx(&cc);

	// _set_purecall_handler(myPurecallHandler);
	::signal(SIGABRT, signalHandler);
	_set_abort_behavior(0, _WRITE_ABORT_MSG|_CALL_REPORTFAULT);

	set_terminate( &forceUnhandledException );
	set_unexpected( &forceUnhandledException );
	_set_purecall_handler( &forceUnhandledException );
	_set_invalid_parameter_handler( &invalidParameterHandler );

	SetUnhandledExceptionFilter(TopLevelFilter);

	__try
	{	
		GLOBAL(loadingTimer).restart();

		// Initialize the Windows System
		if (winOS->Init(hInstance, hPrevInstance, GetCommandLine(), nCmdShow))
		{
			statsManager->getStats().bt.window.instantiations++;
			statsManager->startTimer(statsManager->getStats().bt.window.activeTime);

			// Start the program up
			evtManager->mainLoop();
		}
	} 
	__except(TopLevelFilter(GetExceptionInformation()))
	{
		exit(0);
	}	
	return 0;
}

LRESULT CALLBACK GlobalWndProc(HWND Hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	__try
	{	
		// Certain message are only received by top-level windows, but the main
		// BumpTop window is a child of the desktop window. These messages are
		// captured by a special top-level window and treated as if they were sent
		// to the main BumpTop window.
		if (Hwnd != winOS->GetWindowsHandle())
		{
			switch (uMsg)
			{
			case WM_DEVICECHANGE:
			case WM_QUERYENDSESSION:
			case WM_ENDSESSION:
			case WM_POWERBROADCAST:
			case WM_CLOSE:
				return winOS->MessageProc(winOS->GetWindowsHandle(), uMsg, wParam, lParam);
			default:
				return DefWindowProc(Hwnd, uMsg, wParam, lParam);
			}
		}
		else
		{
			return winOS->MessageProc(Hwnd, uMsg, wParam, lParam);
		}
	} 
	__except(TopLevelFilter(GetExceptionInformation()))
	{
		exit(0);
	}	
}

// +--------------------------------------------------------------------------+
// |                                                                          |
// |                            WindowsSystem Class                           |
// |                                                                          |
// +--------------------------------------------------------------------------+

WindowsSystem::WindowsSystem()
: _recyclerThread(NULL)
, _splashScreen(NULL)
, libraryManager(NULL)
{
	// This process needs to be DPI aware for dpi scaling to work properly 
	if(IsWindowsVersionGreaterThanOrEqualTo(WindowsVista)) {
		HMODULE _hMod = LoadLibrary(_T("User32.dll"));
		typedef BOOL (__stdcall *SetProcessDPIAwareSignature)(void);
		SetProcessDPIAwareSignature f = GetProcAddress(_hMod,"SetProcessDPIAware");
		f();

		FreeLibrary(_hMod);
	}

	// set the default window rect size as the work area - 50
	SystemParametersInfo(SPI_GETWORKAREA, 0, &prevWindowedRect, 0);
	prevWindowedRect.left = 50;
	prevWindowedRect.top = 50;
	prevWindowedRect.right -= 50;
	prevWindowedRect.bottom -= 50;
	postSplashscreenWindowState = WorkArea; 

	hTimerCallback = NULL;
	conditionalFlags = 0;

	// Keep the program running until destroyed
	exitFlag = false;
	windowHwnd = NULL;
	topLevelWindowHwnd = NULL;
	shellExtProxyHwnd = NULL;
	isLoaded = false;
	largeIconSupport = false;
	largeIcons = NULL;
	winMinimized = false;
	uniqueIdCounter = 0;

	// ignore GDI renderer for internal VM testing
	ignoreGDI(false);

	// interaction stats timing
	interactionTimeOutTimer = NULL;
	screenshotTimer = NULL;
	BUMPTOP_UPDATE_DOWNLOADED = RegisterWindowMessage(L"BUMPTOP_UPDATE_DOWNLOADED");

	_bumptopEdition = UndefinedEdition;

	_settingsAppHandler = new SettingsAppMessageHandler();

	_recycleBinStateChanged = true;
	
	LOG("\tnew DesktopLock");
	desktopLock = new DesktopLock("desktop.lock");
	LOG("\t~new DesktopLock");
}

WindowsSystem::~WindowsSystem()
{
	exitFlag = true;

	// UnRegister the WM_WTSSESSION_CHANGE notification
	WTSUnRegisterSessionNotification(winOS->GetWindowsHandle());

	SAFE_DELETE(libraryManager);
	
	// release the explorer host references
	SAFE_DELETE(_recyclerThread);
	SAFE_DELETE(desktopLock);
	SAFE_DELETE(_settingsAppHandler);
}

//5/14/2008
//BumpTop as the "real desktop"
//===============================
//
//There's not too much to this-- BumpTop is made the child of the desktop window.
//That makes it the grandchild of the Program Manager, and the child of 
//SHELLDLL_DefView.
//
//When Show Desktop mode is invoked in windows, the deskop gets pushed to the front
//of the z-order. The way this is done is to actually take the SHELLDLL_DefView and
//make it a child of another window that's high on the zorder. THis window has class
//WorkerW.

bool WindowsSystem::Init(HINSTANCE hInst, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
#if ENABLE_WEBKIT
    qInstallMsgHandler(qtMessageHandler);
	int argc = 0; 	
	QApplication * app = new BumpTopApplication(argc, NULL);
	app->setApplicationName(QString::fromUtf16((const ushort *) WINDOW_NAME));
	app->setApplicationVersion(SVN_VERSION_NUMBER);
	app->setStyleSheet(QT_NT("QLineEdit#RenamerBox {  border: 1px solid gray; border-radius: 0px; }"));
	app->setQuitOnLastWindowClosed(false);
#endif

	logger->setIsLogging(true);
	logger->ignoreStartOrEnd(true);
	guidLogger->setIsLogging(true);

	// load the settings
	winOS->LoadSettingsFile();

#ifndef DISABLE_PHONING
	if (wcsstr(lpCmdLine, QT_NT(L"-sendUninstallStats")))
	{
		EmailCrashReport(QT_NT("feedback@bumptop.com"), QT_NT("Uninstall"), QT_NT("Upload stats due to uninstall."), "", "", true);
		return false;
	}
#endif

	// check that this version of BumpTop is authorized
	if (!scnManager->isBumpPhotoMode)	
	{
		if (isProBuild())
		{
			GLOBAL(settings).freeOrProLevel = AL_PRO;
		}
		else
		{
			// NOTE: workaround the issue where existing beta users may have the auth code
			// (if both pro and normal auth codes exist, then clear the free authCode)
			if (!GLOBAL(settings).proAuthCode.isEmpty() && !GLOBAL(settings).authCode.isEmpty())
				GLOBAL(settings).authCode.clear();

			bool isExistingUser = validateAuthorization(AL_FREE, GLOBAL(settings).authCode.split(" "));

			// also, run validate authorization so Pro users get their version bumped up
			bool isProUser = validateAuthorization(AL_PRO, GLOBAL(settings).proAuthCode.split(" "));
			bool authorized = GLOBAL(settings).hasRunOnce || isProUser; 

			if (!authorized)
			{
				// if we are not authorized, then prompt
				dlgManager->clearState();
				dlgManager->setHasParent(false);
				dlgManager->setExpiredChooseVersionDialog(false);

				if (dlgManager->promptDialog(DialogChooseVersion))
				{
					dlgManager->clearState();
					dlgManager->setHasParent(false);
					dlgManager->promptDialog(DialogThankYou);	
					authorized = true;						
				}
			}

			reauthorize(false);
		}
	}

	// mark the app as having run once
	GLOBAL(settings).hasRunOnce = true;
	SaveSettingsFile();

	if (!exists(GetUserApplicationDataPath()))
	{
		::MessageBox(NULL, (LPCWSTR)QT_TR_NOOP("BumpTop can not find your User's Application Data directory.\nCurrently,"
			"non-English versions of Windows are not supported.").utf16(),
			(LPCWSTR)QT_TR_NOOP("Could not find Application Data directory").utf16(), MB_OK | MB_ICONERROR);
		return false;
	}

	if (!GLOBAL(settings).completedTutorial)
	{
		GLOBAL(isInTrainingMode) = true;
		GLOBAL(skipSavingSceneFile) = true;
	}

	// load the settings file
	LoadSettingsFile();

#ifdef WIN7LIBRARIES
	// Create the library manager
	libraryManager = new LibraryManager();
#endif

	// init scene
	LOG(QT_NT("SceneManager::Init"));
	if (libraryManager)
	{
		// Load the saved folder locations from the settings file
		QListIterator<QString> otherLibsIter(GLOBAL(settings).otherLibraries);
		while (otherLibsIter.hasNext())
		{
			QString folderPath = otherLibsIter.next();
			libraryManager->addFolderAsLibrary(folderPath);
		}
		
		CleanUpSceneFiles();

		// Load the saved Library. Will return the Desktop if the saved library cannot be found
		QSharedPointer<Library> library = libraryManager->getLibraryByKey(GLOBAL(settings).currentLibrary);
		scnManager->init(library);
	}
	else
		scnManager->init();
	
	// parse the command line
	LOG(QString_NT("ParseCommandLine: %1").arg(QString::fromUtf16((const unsigned short *)lpCmdLine)));
	bool parsedCommandLine = ParseCommandLine(lpCmdLine);

	// load the current stats 
	// (NOTE: this must be done After we've parsed the command line to get the correct working dir
	// to get the proper stats file to load)
	LOG(QT_NT("StatsManager::loadStats"));
	statsManager->loadStats();
	
#ifndef ZZBTDEBUG
	if (!scnManager->isInSandboxMode && !scnManager->isBumpPhotoMode && winOS->postSplashscreenWindowState == WorkArea )
	{
		LOG("DesktopLog");
		// Check if we have another instance of bumptop running on the desktop,
		// and if so, restore it (unless it is a relaunched instance).

		if (!winOS->desktopLock->tryLock())
		{
			LOG("\tDesktopLock::tryLock");

			HWND hWnd = FindWindowEx(FindWindowEx(FindWindow(L"Progman", NULL), NULL, L"SHELLDLL_DefView", NULL), NULL, L"BumpTop", NULL);

			if (!scnManager->isShellExtension && hWnd)
			{
				// We found another version of ourself. Let's defer to it:
				if (IsIconic(hWnd))
				{
					ShowWindow(hWnd, SW_RESTORE);
				}

				SetForegroundWindow(hWnd);
				// If this app actually had any functionality, we would
				// also want to communicate any action that our "twin"
				// should now perform based on how the user tried to
				// execute us.
			}
			return false;
		}
	}
#endif

	// save instance handle	
	hInstance = hInst;

	// Get the position of all monitors on the screen
	EnumDisplayMonitors(NULL, NULL, &WindowsSystem::EnumAndRefreshMonitors, NULL);

	// ensure that the monitor exists
	bool monitorFound = false;
	QString mon = GLOBAL(settings).monitor;
	QHash<QString, MONITORINFOEX>::const_iterator iter = monitors.begin();
	while (!monitorFound && (iter != monitors.end()))
	{
		if (iter.key() == mon)
			monitorFound = true;
		iter++;
	}
	if (!monitorFound)
	{
		GLOBAL(settings).monitor = monitors.begin().key();
		SaveSettingsFile();
	}

	LOG("Register window class");
	// register the main window class
	WNDCLASSEX winClass = {0};
	winClass.cbSize			= sizeof(WNDCLASSEX); 
	winClass.style			= CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	winClass.lpfnWndProc	= (WNDPROC) GlobalWndProc;
	winClass.cbClsExtra		= 0;
	winClass.cbWndExtra		= 0;
	winClass.hInstance		= hInstance;
	winClass.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	winClass.hIconSm		= (HICON) LoadImage(hInstance, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 16, 16, 0);
	winClass.hCursor		= LoadCursor(NULL, IDC_ARROW);
	winClass.hbrBackground	= NULL;
	winClass.lpszMenuName	= NULL;
	winClass.lpszClassName	= WINDOW_NAME;

	if (parsedCommandLine &&
		!RegisterClassEx(&winClass)) {
		cerr << "Could not register window class" << endl;
		return false;
	}

	// load the theme
	LOG("Loading Theme");
	QString themeLoadingError;
	try
	{
		if (!themeManager->reloadDefaultTheme())
			themeLoadingError = "Unspecified";
	}
	catch (std::exception& e)
	{
		themeLoadingError = QString::fromStdString(e.what());
		if (themeLoadingError.isEmpty())
		{
			// it's already been taken care of
			return false;
		}
	}		
	if (!themeLoadingError.isEmpty())
	{
		MessageBox(windowHwnd, (LPCWSTR) ("Could not load your BumpTop Theme:\n" + themeLoadingError).utf16(), 
			L"BumpTop Theme Error", MB_OK | MB_ICONERROR);
		return false;
	}

	LOG("Creating window");

	if (Create(lpCmdLine))				// create window
	{
		libraryManager->notifyLibraryChanges(true);

		// init startup gl properties
		LOG("Initializing OpenGL");
		if (!rndrManager->initGL())
			winOS->FailOnDrivers();
		
		LOG("Initializing Novodex");
		InitNovodex();

		LOG("Initializing updater thread");

		// initialize the updater
		if (!scnManager->isShellExtension)
		{
			// note: it's important to do this after initGL, because only after then do we have a stable HWND
			// (initGL might destroy and create an HWND for multisampling)
			UpdateServer *server;
			if (scnManager->testUpdater)
				server = (UpdateServer *) new StagingUpdateServer(native(winOS->GetUpdateDirectory()));
			else
			{
				BumpTopEdition edition = winOS->GetBumpTopEdition();
				QString updateDir = native(winOS->GetUpdateDirectory());

				if (edition == VIP)
					server = (UpdateServer *) new VIP_UpdateServer(updateDir); 
				else
					server = (UpdateServer *) new UpdateServerImpl(updateDir);
			}

			int msToWaitIfUpdateCheckFails = scnManager->testUpdater ? 1000 : 1000*60*60;
			int msBetweenUpdateChecks = scnManager->testUpdater ? 1000 : 1000*60*60;
			bool manualUpdatesOnly = true;
			
			_updaterThread = NULL;
			_updater = new Updater(
				server,													// where we get new version
				atoi(SVN_VERSION_NUMBER),								// current version
				native(winOS->GetUpdateDirectory()),					// directory to download things into
				msToWaitIfUpdateCheckFails,								// normally 1 min
				msBetweenUpdateChecks,									// normally 60 min
				winOS->GetWindowsHandle(),
				manualUpdatesOnly);

			// start this in a new thread
			
			_updaterThread = new boost::thread(boost::bind<void>(&Updater::run, _updater));
		}

		LOG("Starting splash screen");

		// need to do this so dialog boxes during authorization are properly placed
		if (!scnManager->isShellExtension)
		{
			int l, r, t, b;
			GetWorkArea(r, l, t, b);
			SetWindowPos(winOS->GetWindowsHandle(), NULL, l, t, r-l, b-t, NULL);
		}

		// TODO: This should be abstracted away into a factory method or something.
		windows7Multitouch = new Windows7Multitouch();

		// set the double click timer
		GLOBAL(dblClickInterval) = (GetDoubleClickTime() / 1000.0f);

		if (!scnManager->isShellExtension)
		{
			LOG("SplashScreenIntro");
			// show the splash screen
			SplashScreenIntro(true);
			LOG("~SplashScreenIntro");

			// show the system tray 
			LOG("~SysTrayInit");
			sysTray->Init(windowHwnd, hInstance, true);
			LOG("~SysTrayInit");
		}
		else
		{
			SplashScreenIntro(false);
		}

#ifdef DXRENDER
		LOG("Starting DirectX rendering system");
		HRESULT hr = dxr->initializeD3D(windowHwnd);
		if (FAILED(hr))
			FailOnDrivers();
#endif

		// load the widgets
		LOG("Starting widget loading");
		if (!scnManager->isShellExtension)
			widgetManager->initializeActiveWidgets();

		// load textures
		LOG("Starting texture loading");
		texMgr->initIL();
		texMgr->init();

		// intialize Novodex Physics engine
		LOG("Initializing Novodex part ii");
		srand((unsigned) time(NULL));
		uint loadSceneTimer = timeGetTime();
		InitNx();
		consoleWrite(QString("Time to Load Scene: %1ms\n").arg(timeGetTime() - loadSceneTimer));

		LOG("Resetting all motion");

		// reset the motion of everything
		MotionCallback(0, 0);
		
#ifdef SMART_SUPPORT
		smartBoard.Create(windowHwnd);
#endif

		LOG("start explorer alive checker - sends a WM_CLOSE if explorer dies");

		ExplorerAliveChecker * explorerAliveChecker = new ExplorerAliveChecker(winOS->GetWindowsHandle(), 
			scnManager->isShellExtension ? parentHwnd : FindWindow(L"Progman", NULL));
		explorerAliveChecker->init();

		LOG("start recycle bin updater");

		// start recycle bin updater
		if (winOS->GetSystemPath(DesktopDirectory) == native(scnManager->getWorkingDirectory()))
		{
			_recyclerThread = new ThreadableUnit;
			_recyclerThread->run(boost::bind(&WindowsSystem::QueryRecycleBin, this), ThreadableUnit::RecycleBinCheckThread);
			hTimerCallback = SetTimer(windowHwnd, BT_TIMERCALLBACK, 1000, NULL);
		}
		SetConditionalFlag(ThreadSafeProcessing, false);

		LOG("Start Timers");

		// 
		windowTimerCallback = SetTimer(windowHwnd, WINDOW_TIMERCALLBACK, 750, NULL);
		statsManagerUploadCheckTimerCallback = SetTimer(windowHwnd, STATS_TIMERCALLBACK, statsManager->getStatsUploadCheckIntervalInMin() * 60 * 1000, NULL);	// min to ms
		checkForToggleDesktopTimerCallback = SetTimer(windowHwnd, CHECK_FOR_TOGGLE_DESKTOP_TIMERCALLBACK, 500, NULL);
		reauthorizationTimerCallback = SetTimer(windowHwnd, REAUTHORIZATION_TIMERCALLBACK, 500, NULL);
		virtualIconSynchronizationTimerCallback = SetTimer(windowHwnd, VIRTUALICONSYNC_TIMERCALLBACK, 5000, NULL);

		// Inits
		LOG("Initialize keyManager");
		keyManager->init();
		LOG("Initialize lassoMenu");
		lassoMenu->init();
		LOG("Initialize markingMenu");
		markingMenu->init();
		markingMenu->registerMouseHandler();
		markingMenu->registerKeyboardHandler();
		LOG("Initialize menuManager");
		menuManager->init();

		LOG("Clear messages");
		// clear the messages up to this point
		scnManager->messages()->clearMessages();

		// initialize a mouse pointer that represents the regular mouse
		defaultMousePointer = new MousePointer();

		LOG("set the new window as foreground if we are relaunching");
		// set the new window as foreground if we are relaunching
		if (!scnManager->isShellExtension &&
			winOS->getRegistryDwordValue("RelaunchBumpTopWindow"))
		{
			// If we're a child of the desktop, and we're not the topmost window, 
			if (winOS->GetWindowState() == WorkArea && !winOS->IsWindowTopMost() && winOS->isAttachedToDesktopWindow()) 
			{
				winOS->ToggleShowWindowsDesktop();
			}
			winOS->setRegistryDwordValue("RelaunchBumpTopWindow", 0);
		}

#ifdef BT_UTEST
		// run the tests and return the result
		if (scnManager->runBumpTopTestsOnStartup)
		{
			if (!runBumptopTests(true))
			{
				MessageBox(GetTopWindow(NULL), L"Tests Failed!\nCheck your output console for more information.\nPlease fix these issues before submitting your change!", L"BumpTop Tests Failed!", MB_ICONERROR | MB_OK);
				ExitBumptop();
			}
			ExitBumptop();
		}
#endif // BT_UTEST

		ReRegisterDropHandler();

		// it got this far, so mark the previous incomplete shutdown as completed
		LOG("mark the previous incomplete shutdown as completed");
		if (winOS->getRegistryDwordValue("ShutdownIncomplete"))
			winOS->setRegistryDwordValue("ShutdownIncomplete", 0);

		if (GLOBAL(isInTrainingMode))
		{
			TrainingIntroRunner* training = new TrainingIntroRunner();
			scnManager->setTrainingIntroRunner(training);
			training->start();
		}

		if (false)
		{
			// add the close button
			TextOverlay * close = new TextOverlay("X");
				close->getStyle().setPadding(TopEdge, 3.0f);
				close->getStyle().setPadding(RightEdge, 3.0f);
				close->getStyle().setAlpha(0.25f);
				close->addMouseEventHandler(new MinimizeBumpTopWindowToTrayOverlayHandler);
			HorizontalOverlayLayout * horz = new HorizontalOverlayLayout();
				horz->getStyle().setAlpha(0.25f);
				// horz->addItem(minimize);
				horz->addItem(close);

			OverlayLayout * windowControls = scnManager->windowControls();
				windowControls->getStyle().setCornerRadius(BottomLeftCorner, 10.0f);
				windowControls->getStyle().setCornerRadius(TopLeftCorner|TopRightCorner|BottomRightCorner, 0.0f);
				ColorVal bg = windowControls->getStyle().getBackgroundColor();
				bg.bigEndian.a = 64;	// 0.25f alpha
				windowControls->getStyle().setBackgroundColor(bg);
				windowControls->addItem(horz);
		}

		AnimationEntry postIntro = AnimationEntry(cam, (FinishedCallBack) PostIntroLoad, NULL, true);
		animManager->addQueuedAnimation(postIntro);

		// Register to receive remote connection messages so we can tell when to disable BumpTop.
		if (!WTSRegisterSessionNotification(winOS->GetWindowsHandle(), NOTIFY_FOR_ALL_SESSIONS))
		{
			int errorCode = GetLastError();
			consoleWrite(QString::number(errorCode));
		}

		return (GLOBAL(gScene) && GLOBAL(gPhysicsSDK)) ? true : false;
	}

	return false;
}

// When switching working directories, reregistering will allow dropping into the 
// correct directory.
void WindowsSystem::ReRegisterDropHandler()
{
	LOG("Registering drag & drop");

	// register drag and drop
	dropHandler.RevokeHandler();
	dropHandler.RegisterHandler(windowHwnd, true);
}

bool WindowsSystem::ParseCommandLine(TCHAR * cmdLine)
{
	// only support the directory flag for now
	const QString shortDirFlag = QT_NT("-d");
	const QString shortParentHwndFlag = QT_NT("-ph");
	const QString shortParentRectFlag = QT_NT("-pr");
	const QString shortParentDirectoryFlag = QT_NT("-pd");
	const QString shortProxyHwndFlag = QT_NT("-pxh");
	const QString shortSandboxFlag = QT_NT("-sandbox");
	const QString shortTestUpdaterFlag = QT_NT("-testupdater");
	const QString logVerbose = QT_NT("-logVerbose");
	const QString logIgnoreAllFlag = QT_NT("-logIgnoreAll");
	const QString logIgnoreFlag = QT_NT("-logIgnore");
	const QString logDontIgnoreStartOrEnd = QT_NT("-logDontIgnoreStartOrEnd");
	const QString longHelpFlag = QT_NT("-help");
	const QString longRunTestsFlag = QT_NT("-runTests");
	const QString skipAnimationsFlag = QT_NT("-skipAnimations");
	const QString disableRenderingFlag = QT_NT("-skipRendering");
	const QString dellDemoFlag = QT_NT("-del");
	const QString toshibaDemoFlag = QT_NT("-toshiba");
	const QString crash = QT_NT("-crash");
	const QString fullDumpFlag = QT_NT("-fullDump");
	const QString blockForRemoteDebugger = QT_NT("-remoteDebug");
	const QString dumpVersionInfo = QT_NT("-dumpVersionInfo");
	const QString ignoreGDI = QT_NT("-ignoreGDI");
	const QString photoDirFlag = QT_NT("-photoDir");
	const QString childProcessFlag = QT_NT("-childProcess");
	const QString tradeshowFlag = QT_NT("-tradeshow");
	const QString loadDemoSceneFlag = QT_NT("-loadDemoScene");
	const QString automatedJSONTestingFlag = QT_NT("-automatedJSONTesting");
	const QString JSONTestScriptDirFlag = QT_NT("-JSONTestScriptDir");
	const QString JSONLogFileDirFlag = QT_NT("-JSONLogFileDir");
	const QString JSONTestFilesDirFlag = QT_NT("-JSONTestFilesDir");
	const QString touchDebugLogFlag = QT_NT("-touchDebugLog");
	const QString liveMeshDemoFlag = QT_NT("-liveMeshDemo");

	try {	
		// retrieve the arguments
		vector<std::wstring> wargs = split_winmain(cmdLine);
		vector<QString> args;
		for (int i = 0; i < wargs.size(); ++i)
			args.push_back(QString::fromUtf16((const ushort *) wargs[i].c_str()));
		QString customDir;
		for (int i = 0; i < args.size(); ++i)
		{
			QString arg = args[i];
			// handle the help flag if there ever is one
			if (arg == longHelpFlag)
			{
				// print out help message
				MessageBox(GetForegroundWindow(), L"Usage: BumpTop.exe\n"
					 L"\t-d <directory>\tPoints BumpTop at the specified directory\n"
					 L"\t-help\t\tDisplays this help message",
					 L"BumpTop Startup Flags", MB_OK);

				exit(1);
			}

			if (arg == dumpVersionInfo)
			{
				QString version = QString("BumpTop %1 %2").arg(winOS->GetBuildNumber()).arg(winOS->BumpTopEditionName(GetBumpTopEdition()));

				FILE * pFile;
				pFile = fopen ("BUMPTOP_VERSION.txt","w");
				if (pFile!=NULL)
				{
					fputs (version.toAscii(),pFile);
					fclose (pFile);
					exit(0);
				}
			}

			// handle the run-tests flag if there ever is one
			if (arg.startsWith(longRunTestsFlag))
			{
				// Set flag to run automated demo after initialization
				scnManager->runBumpTopTestsOnStartup = true;
			}

			// Enable automated JSON testing
			if (arg.startsWith(automatedJSONTestingFlag))
			{
				// Set flag to run automated tests after initialization
				scnManager->runAutomatedJSONTestsOnStartup = true; //Currently configured to skip rendering
				scnManager->skipAllPromptDialogs = true; // No pop-up boxes for certain actions while running tests
				scnManager->skipAnimations = true;
				// Set default directories for JSON scripts, logfile output and testing file directories
				scnManager->JSONTestScriptPath = winOS->GetTestsDirectory();
				scnManager->JSONTestFilesPath = winOS->GetTestsDirectory()/"Environment";
				scnManager->JSONTestLogPath = winOS->GetTestsDirectory();
			}
			// Manually set directory for JSON test scripts. Ex: -JSONTestScriptDir c:\test
			if(scnManager->runAutomatedJSONTestsOnStartup && arg.startsWith(JSONTestScriptDirFlag))
			{
				++i;
				if (i < args.size())
				{
					ensureUnquoted(args[i]);
					scnManager->JSONTestScriptPath = QDir(args[i]);
				}
			}
			// Manually set directory to output log files. Ex: -JSONLogFileDir c:\test
			if(scnManager->runAutomatedJSONTestsOnStartup && arg.startsWith(JSONLogFileDirFlag))
			{
				++i;
				if (i < args.size())
				{
					ensureUnquoted(args[i]);
					scnManager->JSONTestLogPath = QDir(args[i]);
				}
			}
			// Manually set directory for files to test on. Ex: -JSONTestFilesDir c:\test
			if(scnManager->runAutomatedJSONTestsOnStartup && arg.startsWith(JSONTestFilesDirFlag))
			{
				++i;
				if (i < args.size())
				{
					ensureUnquoted(args[i]);
					scnManager->JSONTestFilesPath = QDir(args[i]);
				}
			}

			if (arg.startsWith(disableRenderingFlag))
			{
				// Allows BumpTop to run on machines without necessary drivers installed, displays a black screen
				scnManager->disableRendering = true;
			}

			if (arg.startsWith(skipAnimationsFlag))
			{
				scnManager->skipAnimations = true;
			}

			if (arg.startsWith(crash))
			{
				//crash
				int* a = NULL;
				a[0] = 0;
			}
			if (arg == fullDumpFlag)
			{
				GLOBAL(generateFullDump) = true;
			}
			
			if (arg.startsWith(childProcessFlag))
				scnManager->isChildProcess = true;

			// Check for any flags which are followed by a path argument
			bool hasDellDemoFlag = (arg.startsWith(dellDemoFlag));
			bool hasToshibaDemoFlag = (arg.startsWith(toshibaDemoFlag));
			bool hasParentDirFlag = (arg.startsWith(shortParentDirectoryFlag));
			bool hasPhotoDirFlag = (arg.startsWith(photoDirFlag));

			if (!hasDellDemoFlag && !hasToshibaDemoFlag &&
				(arg.startsWith(shortDirFlag) || hasParentDirFlag || hasPhotoDirFlag))
			{
				// check if the directory is attached to this arg without spaces
				// This is only valid with -d
				if (arg.startsWith(shortDirFlag) && (arg.size() > shortDirFlag.size()))
				{
					customDir = arg.mid(shortDirFlag.size());
				}
				else
				{
					// get the directory in the following argument
					++i;
					if (i < args.size())
						customDir = args[i];
				}
			}
			if (hasPhotoDirFlag)
				scnManager->isBumpPhotoMode = true;

			// check if there is a parent hwnd specified
			if (arg.startsWith(shortParentHwndFlag))
			{
				// retrieve the parent hwnd
				++i;
				if (i < args.size())
				{			
					parentHwnd = (HWND) args[i].toUInt();
					scnManager->isShellExtension = true;
					postSplashscreenWindowState = ShellExt;
					statsManager->getStats().bt.shellExt.foldersBumped++;
				}
			}

			// check if there is a proxy window specified
			if (arg.startsWith(shortProxyHwndFlag))
			{
				// retrieve the proxy window handle
				++i;
				if (i < args.size())
				{
					shellExtProxyHwnd = (HWND) args[i].toUInt();
				}
			}

			// check if we want to wait for a remote debugger
			if (arg.startsWith(blockForRemoteDebugger))
			{
				while (true)
				{
					if (IsDebuggerPresent())
					{
						__debugbreak();
						break;
					}

					Sleep(1000);
				}
			}

			// check if there is an initial rect specified
			if (arg.startsWith(shortParentRectFlag))
			{
				// retrieve the rectangle

				++i;
				if (i < args.size()) {
					parentRect.left = args[i].toUInt();
				}
				else throw std::exception("Invalid argument");

				++i;
				if (i < args.size()) {
					parentRect.right = args[i].toUInt();
				} else throw std::exception("Invalid argument");

				++i;
				if (i < args.size()) {
					parentRect.top = args[i].toUInt();
				}
				else throw std::exception("Invalid argument");

				++i;
				if (i < args.size())  {
					parentRect.bottom = args[i].toUInt();
				} else throw std::exception("Invalid argument");
			}

			if (arg.startsWith(shortSandboxFlag))
			{
				scnManager->isInSandboxMode = true;
			}

			if (arg.startsWith(shortTestUpdaterFlag))
			{
				scnManager->testUpdater = true;
			}

			if (arg == logVerbose)
			{
				logger->setVerboseLogging(true);
				LOG(QString_NT("Verbose Logging Enabled"));
			}
			if (arg == logIgnoreAllFlag)
			{
				logger->setIsLogging(false);
				guidLogger->setIsLogging(false);
			}
			if (arg == logIgnoreFlag)
			{
				if (args.size() > i + 1)
				{
					const QStringList & ignoreFiles = args[i + 1].split(',', QString::SkipEmptyParts);
					for (int j = 0; j < ignoreFiles.size(); j++)
						logger->ignoreSourceFile(ignoreFiles[j]);
					i++;
				}
			}
			if (arg == logDontIgnoreStartOrEnd)
			{
				logger->ignoreStartOrEnd(false);
			}

			if (arg.startsWith(ignoreGDI))
			{
				winOS->ignoreGDI(true);
			}

			// This flag names a directory to load a demo scene from, e.g. the 
			// Tradeshow directory. All files in the dir are copied to a temp 
			// dir, and the scene is loaded from a JSON file in the directory.
			// Sample use:
			// -demoSceneDir c:\path\to\dir
			if (arg.startsWith(loadDemoSceneFlag))
			{
				// Look for the dir name in the next arg
				if (++i < args.size())
				{
					QString dirPath = args[i];
					ensureUnquoted(dirPath);
					QDir dir(dirPath);
					if (dir.exists())
						scnManager->startupDemoSceneDir = dir.absolutePath();
				}
				if (scnManager->startupDemoSceneDir.isEmpty())
					throw std::exception("Invalid argument");
			}

			// -tradeshow is a shortcut for "-loadDemoScene <BumpTop_root>\Tradeshow"
			if (arg.startsWith(tradeshowFlag))
			{
				QDir dir(parent(winOS->GetTexturesDirectory()) / "Tradeshow");
				if (dir.exists())
					scnManager->startupDemoSceneDir = dir.absolutePath();
				else
					throw std::exception("Tradeshow directory not found");
			}

			// -liveMeshDemo is a shortcut for "-loadDemoScene <BumpTop_root>\LiveMeshDemo\Scene
			if (arg.startsWith(liveMeshDemoFlag))
			{
				QDir dir(parent(winOS->GetTexturesDirectory()) / "LiveMeshDemo" / "Scene");
				if (dir.exists())
				{
					scnManager->startupDemoSceneDir = dir.absolutePath();
					scnManager->defaultActorGrowFactor = 2.2f; // Make actors bigger by default
				}
				else
				{
					throw std::exception("'LiveMeshDemo/Scene' not found");
				}
			}

			// This argument specifies where the log file for gestures will be
			// written. The directory must exist. If the file does not exist,
			// it will be created. If it does exist, it will be appended to.
			if (arg.startsWith(touchDebugLogFlag))
			{
				// The name to be used for the log file is in the next arg
				if (++i < args.size())
				{
					QString filename = args[i];
					ensureUnquoted(filename);
					QFileInfo fileInfo(filename);
					if (QDir(fileInfo.path()).exists())
						scnManager->touchDebugLogFilename = filename;
				}
				if (scnManager->touchDebugLogFilename.isEmpty())
					throw std::exception("Invalid argument");
			}
		}

		if (scnManager->isInSandboxMode)
		{
			GLOBAL(dataPath) = QDir(); // make this empty again, so GetDataDirectory below will set it correctly
			assert(winOS->GetLibraryManager());
			scnManager->setCurrentLibrary(winOS->GetLibraryManager()->getDesktopLibrary());
		}

		// check if we have a directory
		if (!customDir.isEmpty())
		{
			// discard the quotes
			ensureUnquoted(customDir);

			// ensure final backslash exists
			QDir properDir(customDir);

			// ensure valid directory
			if (exists(properDir)) 
			{
				// update the working path with the absolute path because relative paths are causing
				// troubles with ShFileOperation
				QFileInfo relPath(GetExecutableDirectory(), properDir.absolutePath());
				if (exists(relPath))
				{
					properDir = QDir(relPath.absoluteFilePath());
				}

				if (native(properDir) != winOS->GetSystemPath(DesktopDirectory)) 
				{
					scnManager->setWorkingDirectory(properDir);

					// try and get the scene file for that directory
					scnManager->setSceneBumpFile(make_file(properDir, "scene.bump"));
					scnManager->setScenePbBumpFile(make_file(properDir, "scene.pb.bump"));

					// if the directory is not the desktop directory
					// then make it windowed
					if (!scnManager->isShellExtension)
					{
						if (scnManager->isBumpPhotoMode)
							postSplashscreenWindowState = FullWindow;
						else
							postSplashscreenWindowState = Windowed;
					}
				}
			}
			else
			{
				// invalid custom dir!
				throw runtime_error("Invalid directory specified.\n"
					"Defaulting to the Desktop directory.");
			}
		}		
	} catch (std::exception& e) {			
		// error parsing command line
		QString errStr = e.what();
		errStr.append("\n\nRun BumpTop with the -help flag for more information");
		MessageBox(GetForegroundWindow(), (LPCWSTR) errStr.utf16(), L"Error Parsing Command Line", 
			MB_ICONEXCLAMATION);
	}

	return true;
}

LRESULT WindowsSystem::MessageProc(HWND Hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{	
	LRESULT result = 0;
	MINMAXINFO * minMaxInfo = 0;

	// Don't bother if we are exiting
	if (evtManager->isExiting() || GLOBAL(exitBumpTopFlag)) return DefWindowProc(Hwnd, uMsg, wParam, lParam);

	if (windowHwnd)
	{
		// System tray message passing
		if (sysTray->MsgProc(Hwnd, uMsg, wParam, lParam))
			return 1;

		if (!GetConditionalFlag(ThreadSafeProcessing))
		{
			if (uMsg == BUMPTOP_UPDATE_DOWNLOADED)
			{
				if (wParam == 1) // this means that it's just being tested
				{
					return 3; // the magic number our test expects
				}
				else
				{
					if (InstallUpdate())
						ExitBumpTop();
					return 3;
				}
			}

	#ifdef SMART_SUPPORT
			if (uMsg == CSBSDK2::SBSDK_NEW_MESSAGE)
			{
				result = smartBoard.OnSBSDKNewMessage(wParam, lParam);
			}
	#endif	
	
			switch (uMsg) 
			{
				case WM_ACTIVATE:
					if(evtManager->isRDPConnected()) {
						evtManager->onRDPConnect();
					} else {
						if (wParam == WA_CLICKACTIVE)
						{
							statsManager->getStats().bt.window.activatedFromClick++;			

							// Qt does not detect focus lost events from desktop, so we
							// force-send a lose-focus event to the top level widget
							QWidget *focusWidget = QApplication::focusWidget();

							if (focusWidget)
							{
								while (focusWidget->parentWidget())
									focusWidget = focusWidget->parentWidget();
								QFocusEvent focusEvent(QEvent::FocusOut, Qt::OtherFocusReason);
								QApplication::sendEvent(focusWidget, &focusEvent);
							}
						}
						else if (wParam == WA_ACTIVE)
						{
							statsManager->getStats().bt.window.activatedFromNonClick++;
							winOS->BringWindowToForeground();
						}
					}
					break;
				case WM_SYSCOMMAND:
					if (wParam == SC_MINIMIZE)
					{
						/*
						// Minimize our window to tray
						sysTray->MinimizeToTray(Hwnd);
						SetWindowLong(Hwnd, DWL_MSGRESULT, 0);
						return 0;
						*/
					}
					else if (wParam == SC_MAXIMIZE)
					{
						// skip the fullscreen setting
						skipFullScreenWindowState = true;
						ResizeWindow();
					}
					break;
					
				case WM_DISPLAYCHANGE:
					// refresh the monitor dimensions
					EnumDisplayMonitors(NULL, NULL, &WindowsSystem::EnumAndRefreshMonitors, NULL);
					break;
				case WM_SIZE:					
					OnSize();
					Render();
					break;

				case WM_GETMINMAXINFO:
					minMaxInfo = (MINMAXINFO *) lParam;						
					{
						// set the max size of the window to be the working area of 
						HMONITOR wrMon = MonitorFromWindow(windowHwnd, MONITOR_DEFAULTTONEAREST);
						MONITORINFOEX mi;
						mi.cbSize = sizeof(MONITORINFOEX);
						GetMonitorInfo(wrMon, &mi);
						minMaxInfo->ptMaxSize.x = mi.rcWork.right - mi.rcWork.left;
						minMaxInfo->ptMaxSize.y = mi.rcWork.bottom - mi.rcWork.top;
					}
					break;

				case WM_SYSKEYUP:
					OnSysKeyUp();
					break;

				// This is where we get raw input events, which we use to get multiple mouse support.
				// RegisterRawInputDevices must be called to receive this event
				case WM_INPUT:
					if (GLOBAL(settings).enableMultimouse)
						multiMice->onRawInput(wParam, lParam);
					break;

				// Multitouch messages. Must call RegisterTouchWindow to receive these
				case WM_TOUCH:
					windows7Multitouch->onTouchInput(uMsg, wParam, lParam);
					rndrManager->invalidateRenderer();
					return 0;

				case WM_TABLET_QUERY_SYSTEM_GESTURE_STATUS:
					// According to http://msdn.microsoft.com/en-us/library/ms699430%28VS.85%29.aspx,
					// this will disable press-and-hold right-click.
					return SYSTEM_GESTURE_STATUS_NOHOLD;

				case WM_MOUSEACTIVATE:
					switch(windowState)
					{
					case ShellExt:
						SetFocus(windowHwnd);
						break;
					case WorkArea:
						return MA_ACTIVATE;
					case Windowed:
						return MA_ACTIVATE;
						return MA_ACTIVATE;
					case FullWindow:
						return MA_ACTIVATE;
					default:
						assert(false); // unhandled case
					}
					break;

				case WM_LBUTTONUP:
				case WM_MBUTTONUP:
				case WM_RBUTTONUP:
					if (!windows7Multitouch->overridesMouseEvent(uMsg) && !windows7Multitouch->isGestureActive())
					{
						// forward to the event manager
						return evtManager->messageProc(Hwnd, uMsg, wParam, lParam);
					}
					break;

				case WM_LBUTTONDOWN:
				case WM_MBUTTONDOWN:
				case WM_RBUTTONDOWN:
					if(!windows7Multitouch->overridesMouseEvent(uMsg) && !windows7Multitouch->isGestureActive())
					{
						// forward to the event manager
						return evtManager->messageProc(Hwnd, uMsg, wParam, lParam);
					}
					break;

				case WM_MOUSEMOVE:
					if (!windows7Multitouch->isGestureActive())
					{
						// forward to the event manager
						return evtManager->messageProc(Hwnd, uMsg, wParam, lParam);
					}
					break;

				case WM_CAPTURECHANGED:
					ReleaseCapture();
					return 0;

				case WM_MOUSEWHEEL:
					if(!windows7Multitouch->isGestureActive())
					{
						POINT cursor;
						GetCursorPos(&cursor);
						if (winOS->GetWindowsHandle() == WindowFromPoint(cursor))
						{
							// forward to the event manager
							return evtManager->messageProc(Hwnd, uMsg, wParam, lParam);
						}
					}
					break;

				default:		
					if (_settingsAppHandler->handleMessage(uMsg, wParam, lParam, result))
						return result;
					else
						return evtManager->messageProc(Hwnd, uMsg, wParam, lParam);
					break;
			}
		}
		else
		{
			switch (uMsg) 
			{
				case WM_PAINT:
					Render();
					ValidateRect(Hwnd, NULL);
					break;
			}
		}
	}

	return DefWindowProc(Hwnd, uMsg, wParam, lParam);
}

QDir WindowsSystem::GetUserThemesDirectory(bool appendDefaultPath) 
{
	// User Themes belong in the Application Data folder
	// i.e. C:/Documents and Settings/<User>/Application Data/BumpTop Technologies, Inc./Bumptop

	// create the paths if they don'e already exist
	if (empty(GLOBAL(userThemesPath))) 
	{
		QDir themesDir = GetUserApplicationDataPath() / "Themes";
		QDir defaultThemeDir = themesDir / "Default";

		// ensure themes dir exists
		if (!exists(themesDir))
			QDir().mkpath(themesDir.absolutePath());

		GLOBAL(userThemesPath) = themesDir;
		GLOBAL(userDefaultThemePath) = defaultThemeDir;
	}

	if (appendDefaultPath)
		return GLOBAL(userDefaultThemePath);		
	else
		return GLOBAL(userThemesPath);
}

QFileInfo WindowsSystem::GetExecutablePath()
{
	TCHAR buffer[MAX_PATH + 1];
	GetModuleFileName(NULL, buffer, MAX_PATH);
	return QFileInfo(QString::fromUtf16((const ushort *) buffer));
}

QDir WindowsSystem::GetExecutableDirectory()
{
	if (empty(GLOBAL(currentPath)))
		GLOBAL(currentPath) = GetExecutablePath().dir();

	return GLOBAL(currentPath);
}

QDir WindowsSystem::GetThemesDirectory(bool appendDefaultPath) 
{
	if (empty(GLOBAL(themesPath))) 
	{
		QDir appPath = GetExecutableDirectory();
		QString relThemesName = "Themes";
		GLOBAL(themesPath) = appPath / relThemesName;
		// check parent directory for themes dir if not found
		if (!exists(make_file(appPath, relThemesName))) {
			appPath = parent(appPath);
			if (exists(make_file(appPath, relThemesName))) {
				GLOBAL(themesPath) = appPath / relThemesName;
			}
		}

		GLOBAL(defaultThemePath) = GLOBAL(themesPath) / "Default";		
	}

	if (appendDefaultPath)
		return GLOBAL(defaultThemePath);		
	else
		return GLOBAL(themesPath);
}

QDir WindowsSystem::GetWidgetsDirectory()
{
	if (empty(GLOBAL(widgetsPath))) {
		QDir appData = GetUserApplicationDataPath();
		QString relWidgetsName = "Widgets";
		GLOBAL(widgetsPath) = appData / relWidgetsName;
		// check parent directory for widgets dir if not found
		if (!exists(make_file(appData, relWidgetsName))) {
			appData = parent(appData);
			if (exists(make_file(appData, relWidgetsName))) {
				GLOBAL(widgetsPath) = appData/ relWidgetsName;
			}
		}
	}
	return GLOBAL(widgetsPath);
}

QDir WindowsSystem::GetCacheDirectory()
{
	if (empty(GLOBAL(cachePath))) {
		QDir appData = GetUserApplicationDataPath();
		QString relCacheName = "Cache";
		GLOBAL(cachePath) = appData / relCacheName;
		// check parent directory for cache dir if not found
		if (!exists(make_file(appData, relCacheName))) {
			appData = parent(appData);
			if (exists(make_file(appData, relCacheName))) {
				GLOBAL(cachePath) = appData/ relCacheName;
			}
		}
	}
	return GLOBAL(cachePath);
}

// Get directory that stores cached frames
QDir WindowsSystem::GetFramesDirectory()
{
	if (empty(GLOBAL(framesPath))) {
		QDir cacheDir = GetCacheDirectory();
		QString relFramesName = "Frames";
		GLOBAL(framesPath) = cacheDir / relFramesName;
		// check parent directory for cache dir if not found
		if (!exists(make_file(cacheDir, relFramesName))) {
			cacheDir = parent(cacheDir);
			if (exists(make_file(cacheDir, relFramesName))) {
				GLOBAL(cachePath) = cacheDir/ relFramesName;
			}
		}
	}
	return GLOBAL(framesPath);
}

// Retreives directory: \Source\Tests
QDir WindowsSystem::GetTestsDirectory()
{	
	if (empty(GLOBAL(testsPath))) 
	{
		QDir appPath = GetExecutableDirectory();
		QString relTestsName = "Tests";
		GLOBAL(testsPath) = appPath / relTestsName;
		// check parent directory for tests dir if not found
		if (!exists(make_file(appPath, relTestsName))) {
			appPath = parent(appPath);
			if (exists(make_file(appPath, relTestsName))) {
				GLOBAL(testsPath) = appPath / relTestsName;
			}
		}
	}
	return GLOBAL(testsPath);
}

QDir WindowsSystem::GetTexturesDirectory()
{	
//	path GLOBAL(texturesPath);
	if (empty(GLOBAL(texturesPath))) 
	{
		QDir appPath = GetExecutableDirectory();
		QString relTexturesName = "Textures";
		GLOBAL(texturesPath) = appPath / relTexturesName;
		// check parent directory for textures dir if not found
		if (!exists(make_file(appPath, relTexturesName))) {
			appPath = parent(appPath);
			if (exists(make_file(appPath, relTexturesName))) {
				GLOBAL(texturesPath) = appPath / relTexturesName;
			}
		}
	}
	return GLOBAL(texturesPath);
}

QDir WindowsSystem::GetLanguagesDirectory()
{	
	//	path GLOBAL(languagesPath);
	if (empty(GLOBAL(languagesPath))) 
	{
		QDir appPath = GetExecutableDirectory();
		QString relLanguagesName = "Languages";
		GLOBAL(languagesPath) = appPath / relLanguagesName;
		// check parent directory for textures dir if not found
		if (!exists(make_file(appPath, relLanguagesName))) {
			appPath = parent(appPath);
			if (exists(make_file(appPath, relLanguagesName))) {
				GLOBAL(languagesPath) = appPath / relLanguagesName;
			}
		}
	}
	return GLOBAL(languagesPath);
}

QDir WindowsSystem::GetStatsDirectory()
{
	// always keep the stats in the Application Data folder
	// i.e. C:/Documents and Settings/<User>/Application Data/BumpTop Technologies, Inc./Bumptop

	QFileInfo statsPath(GetUserApplicationDataPath(), "stats");
	QDir statsDir(statsPath.absoluteFilePath());

	// ensure stats path exists
	if (!exists(statsPath))
		GetUserApplicationDataPath().mkdir(statsPath.fileName());

	return statsDir;
}

QDir WindowsSystem::GetDataDirectory()
{
	if (scnManager->isInSandboxMode)
	{
		//	path GLOBAL(dataPath);
		if (empty(GLOBAL(dataPath))) {
			QDir appPath = winOS->GetExecutableDirectory();
			QString relDataName("SandboxData");
			GLOBAL(dataPath) = appPath / relDataName;
			// check parent directory for data dir if not found
			if (!exists(make_file(appPath, relDataName))) {
				appPath = parent(appPath);
				if (exists(make_file(appPath, relDataName))) {
					GLOBAL(dataPath) = appPath / relDataName;
				}
			}
		}
		return GLOBAL(dataPath);
	}
	else if (scnManager->runBumpTopTestsOnStartup)
	{
		//	path GLOBAL(dataPath);
		if (empty(GLOBAL(dataPath))) {
			QDir appPath = winOS->GetExecutableDirectory();
			QString relDataName("UnitTestData");
			GLOBAL(dataPath) = appPath / relDataName;
			// check parent directory for data dir if not found
			if (!exists(make_file(appPath, relDataName))) {
				appPath = parent(appPath);
				if (exists(make_file(appPath, relDataName))) {
					GLOBAL(dataPath) = appPath / relDataName;
				}
			}
		}
		return GLOBAL(dataPath);
	}
	else
	{
		// Data directory should be in the local Application Data folder
		// i.e. %AppData%/Bump Technologies, Inc/BumpTop
		if (empty(GLOBAL(dataPath)))
			GLOBAL(dataPath) = GetUserApplicationDataPath();
		return GLOBAL(dataPath);
	}
}

QDir WindowsSystem::GetTrainingDirectory()
{
	// Data directory should be in the local Application Data folder
	// i.e. %AppData%/Bump Technologies, Inc/BumpTop
	if (empty(GLOBAL(trainingPath)))
		GLOBAL(trainingPath) = GetDataDirectory() / "Training";

	if (!exists(GLOBAL(trainingPath)))
		create_directory(GLOBAL(trainingPath));

	return GLOBAL(trainingPath);
}

QDir WindowsSystem::GetTradeshowDirectory()
{
	// Data directory should be in the local Application Data folder
	// i.e. %AppData%/Bump Technologies, Inc/BumpTop
	if (empty(GLOBAL(tradeshowPath)))
		GLOBAL(tradeshowPath) = GetDataDirectory() / "Tradeshow";

	if (!exists(GLOBAL(tradeshowPath)))
		create_directory(GLOBAL(tradeshowPath));

	return GLOBAL(tradeshowPath);
}


QDir WindowsSystem::GetUpdateDirectory()
{
	GLOBAL(updatePath) = GetDataDirectory() / "Updates";
	return GLOBAL(updatePath);
}

QDir WindowsSystem::GetSystemDirectory()
{
	TCHAR buffer[MAX_PATH + 1];
	::GetSystemDirectory(buffer, MAX_PATH);
	return QDir(QString::fromUtf16((const ushort *) buffer));
}

bool WindowsSystem::Create(TCHAR * cmdLine) 
{
	// initialize any dependent managers
	menuManager;

	// get the current executable/data directory	
	// and check this directory for textures subdir
	if (empty(GetTexturesDirectory())) {
		MessageBox(GetForegroundWindow(), (LPCWSTR)QT_TR_NOOP("BumpTop couldn't find its internal textures directory.  "
			"Please re-install BumpTop.").utf16(), (LPCWSTR)QT_TR_NOOP("BumpTop Error").utf16(), MB_OK | MB_ICONERROR);
		return false;
	}

#ifdef HWMANFDEMOMODE
	// collect the participant's ID number, if it's not already been entered
	bool ranOnce = false;
	while (GLOBAL(settings).hwManParticipantID < 0)
	{
		dlgManager->clearState();
		dlgManager->setPrompt((ranOnce ? QT_TR_NOOP("ID must be a positive integer. ") : "") + QT_TR_NOOP("Please enter your participant ID:"));
		ranOnce = true;
		if (dlgManager->promptDialog(DialogInput))
		{
			if (dlgManager->getText() == "0")
				GLOBAL(settings).hwManParticipantID = 0;
			else if (dlgManager->getText().toInt() != 0)
				GLOBAL(settings).hwManParticipantID = dlgManager->getText().toInt();
		}
	}
	if (ranOnce)
	{
		GLOBAL(settings).save(make_file(winOS->GetDataDirectory(), "settings.json"));
	}

	MessageBox(NULL, (LPCWSTR)QT_TR_NOOP("Welcome to BumpTop!\n\nWe'll start with a tour.").utf16(), L"BumpTop", MB_OK);
	ShellExecute(NULL, L"open", L"http://bumptop.com/help", NULL, NULL, SW_SHOWNORMAL);
#endif

	// start the console
	startConsoleWin();

	consoleWrite(QString("BumpTop v2.0 BUILD %1.\n").arg(GetBuildNumber()));
	statsManager->getStats().bt.build = stdString(GetBuildNumber());
#ifdef BTDEBUG
	consoleWrite("Debug\n");
#else
	consoleWrite("Release\n");
#endif

	// create the bt window
	if (!CreateBTWindow()) return false;

	// parent the bt dialogs
	dlgManager->setHasParent(true);

	// initialize gdi
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	if (Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL) != Gdiplus::Ok)
		return false;

	// set window properties
	SetConditionalFlag(ThreadSafeProcessing, true);
	SetForegroundWindow(windowHwnd);
	SetCursor(LoadCursor(NULL, IDC_ARROW));

	// refresh the system image lists
	static bool fileIconInited = false;
	if (!fileIconInited)
	{
		BOOL result;
		BOOL (WINAPI* pfnFileIconInit)(BOOL) = NULL;
		HMODULE hLib = GetModuleHandle(L"shell32");
		if (hLib)
		{
			// load the FileInitIcon function ptr
			(FARPROC&) pfnFileIconInit = GetProcAddress(hLib, "FileIconInit");
			if (!pfnFileIconInit)
				(FARPROC&) pfnFileIconInit = GetProcAddress(hLib, MAKEINTRESOURCEA(660));	// see FileIconInit() on msdn

			if (pfnFileIconInit)
				result = pfnFileIconInit(TRUE);
		} 
		fileIconInited = true;
	}	
	return true;
}

void WindowsSystem::LoadSettingsFile()
{
	QFileInfo settingsPath = make_file(GetDataDirectory(), "settings.json");
	bool saveNewSettings = true;
	if (exists(settingsPath)) {
		try {
			// merge the global settings while loading, just in case 
			// the user is missing any keys 
			GLOBAL(settings).load(settingsPath);
			GLOBAL(settings).save(settingsPath);
			saveNewSettings = false;
		} catch (...) {
			MessageBox(GetForegroundWindow(), (LPCWSTR)QT_TR_NOOP("There was an error reading your settings file (Data/settings.json)."
				"Using default settings.").utf16(), L"BumpTop", MB_OK | MB_ICONINFORMATION);
		}
	}
	else
	{
		GLOBAL(settings).initFirstTime();
	}

	// save a new copy of the settings if there was a problem loading
	if (saveNewSettings) {
		// override the previous global settings because it might have read in some
		// corrupted data and used it to set stuff
		GLOBAL(settings) = GlobalSettings();
		GLOBAL(settings).save(settingsPath);
	}
}

void WindowsSystem::SaveSettingsFile()
{
	QFileInfo settingsPath = make_file(GetDataDirectory(), "settings.json");
	GLOBAL(settings).save(settingsPath);
}

void WindowsSystem::Render()
{
	if (GetWindowsHandle() && RenderCallback())
	{
		bool isProfiling = profiler->isProfiling();
		if (isProfiling)
		{
			profiler->onFrame();
		}
		Stopwatch timer;
#ifndef DXRENDER
			rndrManager->swapBuffers();
#endif
		if (isProfiling)
		{
			profiler->incrementTime("RenderingSum>SwapBuffers", timer.restart());
		}
	}
}

void WindowsSystem::Destroy()
{
	if (!windowHwnd)
		return; 

	// Shutdown COM
	CoUninitialize();
	OleUninitialize();

	// destroy gdi
	Gdiplus::GdiplusShutdown(gdiplusToken);

	// Kill the Idle Timer
	if (hTimerCallback)
	{
		KillTimer(windowHwnd, hTimerCallback);
		hTimerCallback = NULL;
	}

	// Kill the window timer
	if (windowTimerCallback)
	{
		KillTimer(windowHwnd, windowTimerCallback);
		windowTimerCallback = NULL;
	}

	// Kill the Stats timer
	if (statsManagerUploadCheckTimerCallback)
	{
		KillTimer(windowHwnd, statsManagerUploadCheckTimerCallback);
		statsManagerUploadCheckTimerCallback = NULL;
	}

	// Kill the screenshot timer
	if (screenshotTimer)
	{
		KillTimer(windowHwnd, screenshotTimer);
		screenshotTimer = NULL;
	}

	// Kill the toggle desktop timer
	if (checkForToggleDesktopTimerCallback)
	{
		KillTimer(windowHwnd, checkForToggleDesktopTimerCallback);
		checkForToggleDesktopTimerCallback = NULL;
	}

	// Kill the reauthorization timer
	if (reauthorizationTimerCallback)
	{
		KillTimer(windowHwnd, reauthorizationTimerCallback);
		reauthorizationTimerCallback = NULL;
	}

	// kill the virtual icon synchronization timer
	if (virtualIconSynchronizationTimerCallback)
	{
		KillTimer(windowHwnd, virtualIconSynchronizationTimerCallback);
		virtualIconSynchronizationTimerCallback = NULL;
	}

	// stop the input accumulation timer if necessary
	stopAccumulatingInteractionTimer();

	windowHwnd = NULL;
}

bool WindowsSystem::IsInShowDesktopMode()
{
	//apparently, when you hit Show Desktop, the desktop ShellView32 instance's grandfather changes from
	// progman to a window with class WorkerW
	HWND grandfather = GetAncestor(GetAncestor(winOS->GetWindowsHandle(), GA_PARENT), GA_PARENT);
	TCHAR buffer[MAX_PATH+1];

	GetClassName(grandfather, buffer, MAX_PATH);

	return (QString::fromUtf16((const ushort *) buffer) == QString("WorkerW"));
}

void WindowsSystem::LoadDesktopTexture()
{

	QString key = QT_NT("floor.desktop.buffer");
	if (!texMgr->hasTexture(QT_NT("floor.desktop")))
		key = QT_NT("floor.desktop");

	bool stretched = false; //This is a unique case, because the default bumptop behaviour is to stretch the image anyway

	// Make the wallpaper conform to windows' settings
	WindowsSystem::WallPaperStyle wallStyle = GetWallPaperStyle();
	ColorVal winDesktopBackColor = GetWindowsBackgroundColor();

	stretched = (wallStyle == STRETCH);

	if(!stretched) {
		QImage floorTex = QImage(GetWindowWidth(),GetWindowHeight(),QImage::Format_ARGB32);
		QImage backImg = QImage(GetWallPaperPath());
		if (backImg.isNull())
		{
			// There is a chance that Windows is writing to the wallpaper file when we try to open it.
			// Exit the function in this case and try again later
			return;
		}

		uint fillColorA = 256 << 24;
		uint fillColorR = winDesktopBackColor.bigEndian.r << 16;
		uint fillColorG = winDesktopBackColor.bigEndian.g << 8;
		uint fillColorB = winDesktopBackColor.bigEndian.b;
		uint fillColor = 0;

		fillColor = fillColor | fillColorA | fillColorR | fillColorG | fillColorB;

		floorTex.fill(fillColor);
		QPainter p(&floorTex);

		switch(wallStyle) {
					case CENTER:
						{
							p.drawImage(floorTex.width()/2 - backImg.width()/2, floorTex.height()/2 - backImg.height()/2, backImg);
							break;
						}
					case FILL:
						{
							float scalex = 1.0f*floorTex.width()/backImg.width();
							float scaley = 1.0f*floorTex.height()/backImg.height();
							QImage scaleBack;
							if(scalex > scaley) {
								scaleBack = backImg.scaledToWidth(floorTex.width());
							} else {
								scaleBack = backImg.scaledToHeight(floorTex.height());
							}
							p.drawImage(floorTex.width()/2 - scaleBack.width()/2,floorTex.height()/2 - scaleBack.height()/2,scaleBack);
							break;
						}
					case FIT:
						{
							float scalex = 1.0f*floorTex.width()/backImg.width();
							float scaley = 1.0f*floorTex.height()/backImg.height();
							QImage scaleBack;
							if(scalex < scaley) {
								scaleBack = backImg.scaledToWidth(floorTex.width());
							} else {
								scaleBack = backImg.scaledToHeight(floorTex.height());
							}			
							p.drawImage(floorTex.width()/2 - scaleBack.width()/2,floorTex.height()/2 - scaleBack.height()/2,scaleBack);
							break;
						}
					case TILED:
						{
							float tilex = 1.0f*floorTex.width()/backImg.width();
							float tiley = 1.0f*floorTex.height()/backImg.height();

							for(int x = 0; x < tilex; x++) {
								for(int y = 0; y < tiley; y++) {
									p.drawImage(x*backImg.width(),y*backImg.height(),backImg);
								}
							}
							break;
						}
					case UNABLETOREAD:
						{
							break;
						}
		}

		QString path;

		QFileInfo newBgFile = QFileInfo(GetWallPaperPath());
		path = newBgFile.absoluteDir().absolutePath();
		path.append(QT_NT("\\Bump")).append(newBgFile.baseName()).append(QT_NT(".bmp"));


		QFile floorSave(path);
		floorTex.save(&floorSave, QT_NT("BMP"));

		texMgr->loadPersistentTexture(GLTextureObject(Load|Reload|Compress, key, path, HiResImage, HighPriority));
	
	}else{
		texMgr->loadPersistentTexture(GLTextureObject(Load|Reload|Compress, key, GetWallPaperPath(), HiResImage, HighPriority));
	}

}

void WindowsSystem::OnTimer(WPARAM wParam)
{
	if (wParam == checkForToggleDesktopTimerCallback)
	{
		// Problem: when we're in Show Desktop mode, the normal shellview-style Desktop gets keyboard focus
		//			(even though it's obscured by the BumpTop desktop window)
		// Solution: if we detect that we're in the Show Desktop mode, and if the topmost window that Bumptop is attached to
		//			 (its grandfather) is the foreground window, there are two possibilities: either Bumptop
		//			 has keyboard focus, or the shellview-style desktop has keyboard focus. Unfortunately, I 
		//			 couldn't find a way to find out which one; I tried AttachThreadInput and GetActiveWindow,
		//			 AttachThreadInput and GetFocus, as well as GetGUIThreadInfo. So, what I do is just give 
		//			 BumpTop focus whenever its grandfather is the foreground window

		if (winOS->GetWindowState() == WorkArea && winOS->IsInShowDesktopMode() && winOS->IsGrandfatherForegroundWindow())
		{
			winOS->SetFocusOnWindow();
			statsManager->getStats().bt.window.activatedFromShowDesktop++;
		}

		// Problem: sometimes BumpTop fails in attaching itself to the desktop
		// Solution: check if BumpTop had failed, and if there is something to attach to, 
		// and if so, try to reattach again

		if (!GLOBAL(exitBumpTopFlag) && winOS->GetWindowState() == WorkArea && !winOS->isAttachedToDesktopWindow() && NULL != FindWindowEx(FindWindow(L"Progman", NULL), NULL, L"SHELLDLL_DefView", NULL))
		{
			winOS->attachToDesktopWindow();
		}

		// Problem: when you have only one window in Windows XP and hit Alt-Tab, BumpTop disappears
		//			this is because the FolderView sibling for some reason is moved up in the z-order
		// Solution: detect this situation and move BumpTop up in the z-order
		if (!GLOBAL(exitBumpTopFlag) && winOS->GetWindowState() == WorkArea && winOS->isAttachedToDesktopWindow() && !winOS->isTopMostDesktopWindowChild())
		{
			winOS->makeTopWindow();
		}
	}
	else if (wParam == windowTimerCallback)
	{	
		// check if the work area has changed
		if (!scnManager->isShellExtension && windowState == WorkArea) 
		{
			ResizeWindow();
		}

		// also check if the background has changed
		if (GLOBAL(settings).useWindowsBackgroundImage)
		{
			// We are comparing file sizes instead of paths since the path does not change
			static qint64 prevWallpaperFileSize = QFileInfo(GetWallPaperPath()).size();
			qint64 newWallpaperFileSize = QFileInfo(GetWallPaperPath()).size();

			static WallPaperStyle prevWallpaperStyle = UNABLETOREAD;
			WallPaperStyle newWallpaperStyle = GetWallPaperStyle();

			static ColorVal prevBackgroundColor = ColorVal(0,0,0,0);
			ColorVal newBackgroundColor = GetWindowsBackgroundColor();
			
			bool somethingChanged = false;

			// using .val32 as a hack because colorval has no compare operator
			somethingChanged = ((prevWallpaperFileSize != newWallpaperFileSize) || (prevWallpaperStyle != newWallpaperStyle) || (prevBackgroundColor.val32 != newBackgroundColor.val32));

			if (somethingChanged)
			{
				LoadDesktopTexture();
				prevWallpaperFileSize = newWallpaperFileSize;
				prevWallpaperStyle = newWallpaperStyle;
				prevBackgroundColor = newBackgroundColor;
			}
		}
	} 
	else if (wParam == statsManagerUploadCheckTimerCallback)
	{
		statsManager->saveStats();

		// this is called every 5 minutes
		bool shouldTakeScreenshot = false;
		if (statsManager->shouldUploadStats(shouldTakeScreenshot))
		{

			// MessageClearPolicy clearPolicy;
			//     clearPolicy.setTimeout(1);
			// scnManager->messages()->addMessage(new Message("statsUploadCheck", "Submitting anonymous statistics to BumpTop HQ", Message::Ok, clearPolicy));
			statsManager->uploadStats();
		}

		/* NOTE: disable taking screenshots for now
		if (shouldTakeScreenshot)
		{
			screenshotTimer = SetTimer(windowHwnd, SCREENSHOT_TIMERCALLBACK, 100, NULL);
		}
		*/
	}
	else if (wParam == virtualIconSynchronizationTimerCallback)
	{
		// NOTE, we try and synchronize the virtual icons here. we compare with items in the current
		//		 scene against visible virtual folders on the desktop
		if (!GLOBAL(isInTrainingMode) &&
			(winOS->GetSystemPath(DesktopDirectory) == native(scnManager->getWorkingDirectory())))
		{
			// NOTE: recycle bin in queried elsewhere, don't do it here (in WinXp)
			vector<int> virtualIcons;
			virtualIcons.push_back(MyComputer);
			if (IsWindowsVersionGreaterThanOrEqualTo(WindowsVista))
			{
				virtualIcons.push_back(RecycleBin);
				virtualIcons.push_back(ControlPanel);
			}
			virtualIcons.push_back(MyDocuments);
			virtualIcons.push_back(NetworkNeighborhood);

			for (int i = 0; i < virtualIcons.size(); ++i)
			{
				QString virtualIconName = winOS->GetFileNameFromIcon(virtualIcons[i]);
				bool hasVirtualActor = true;
				winOS->GetIconAvailability(virtualIcons[i], hasVirtualActor);
				vector<FileSystemActor *> actors = scnManager->getFileSystemActors(virtualIconName, false, false);
				assert(actors.size() <= 1);
				// ConsoleWriteGuard guard(QString("%1: %2:%3").arg(virtualIconName).arg(hasVirtualActor).arg(actors.size()));
				if (hasVirtualActor)
				{
					if (actors.empty())
					{
						// create the new Actor
						const float scaleFactor = 2.0f;
						FileSystemActor *actor = FileSystemActorFactory::createFileSystemActor(virtualIconName);
						Vec3 dims = actor->getDefaultDims();
#ifdef TABLE
						dims.x *= scaleFactor;
						dims.y *= scaleFactor;
#endif
						actor->setDims(dims);
						actor->setAlpha(1.0f);
						actor->setFilePath(virtualIconName);
						actor->setLinearVelocity(Vec3(0, -0.75f, 0));
						actor->setFreshnessAlphaAnim(1.0f, 150);

						// give this actor a random orientation and position
						const int distDelta = 15;
						int dummy = 0;
						Vec3 randPos(winOS->GetDropPoint(dummy));
						randPos.x += (rand() % distDelta);
						randPos.z += (rand() % distDelta);
						actor->setGlobalPosition(randPos);
						actor->setGlobalOrientation(Quat(45, Vec3(1,0,0)));
					}
				}
				else
				{
					if (!actors.empty())
					{
						// remove the icon
						FadeAndDeleteActor(actors.front());
					}
				}
			}
		}
	}
	else if (wParam == screenshotTimer)
	{
		// iterate through windows above the bt window and determine if any overlap bumptop
		// if so, then restart the timer, otherwise, take the screenshot and return
		RECT windowRect, prevWindowRect, tempRect;
		GetWindowRect(windowHwnd, &windowRect);

		// we must use the top level window to walk the zorder hierarchy
		HWND parent = 0;
		if (scnManager->isShellExtension)
		{
			parent = windowHwnd;
			while (GetParent(parent))
			{
				parent = GetParent(parent);
			}
		}
		else
		{
			parent = FindWindow(NULL, L"Program Manager");
		}

		// walk the zorder hierarchy backwards
		HWND prevWnd = GetWindow(parent, GW_HWNDPREV);
		while (prevWnd)
		{
			// check if prevWnd overlaps with our window
			GetWindowRect(prevWnd, &prevWindowRect);
			if (!GetParent(prevWnd) && IsWindowVisible(prevWnd) && 
				IntersectRect(&tempRect, &windowRect, &prevWindowRect))
			{
				// restart the timer and break
				screenshotTimer = SetTimer(windowHwnd, SCREENSHOT_TIMERCALLBACK, 1000, NULL);
				return;
			}
			prevWnd = GetWindow(prevWnd, GW_HWNDPREV);
		}
		// we've gotten here because there is no overlapping windows, so just take the screenshot
		statsManager->takeScreenshot();
		screenshotTimer = NULL;
	}
	else if (wParam == interactionTimeOutTimer)
	{
		// this is called every 150ms if it is on
		// and if we hit this callback, then we need to accumulate the interaction time for the stats
		statsManager->getStats().bt.window.interactiveTime += float(accumulatedInterationTime.elapsed());
		stopAccumulatingInteractionTimer();
	}
	else
	{
		if (winOS->GetSystemPath(DesktopDirectory) == native(scnManager->getWorkingDirectory()))
		{
			// update the recycle bin icon
			// NOTE: we MUST lock the mutex before accessing the recycle bin full var
			vector<FileSystemActor *> fsActors = scnManager->getFileSystemActors(winOS->GetFileNameFromIcon(RecycleBin), false, false);
			assert(fsActors.size() <= 1);
			if (!fsActors.empty())
			{
				_memberMutex.lock();
				bool isRecycleBinFull = _isRecycleBinFull;
				bool recycleBinStateChanged = _recycleBinStateChanged;
				_memberMutex.unlock();
	
				if (isRecycleBinFull)
				{
					// Check to see if the Recycle bin is full
					if (!texMgr->isTextureState("RecycleBin_Full", TextureLoaded))
					{
						// If we don't have an icon for a loaded Recycle Bin, see if we can poll for one.
						texMgr->loadPersistentTexture(GLTextureObject(Load, "RecycleBin_Full", winOS->GetFileNameFromIcon(RecycleBin), FileIcon, NormalPriority,false));
					}
				}else{
					if (!texMgr->isTextureState("RecycleBin_Empty", TextureLoaded))
					{
						// If there is no empty Recycle bin Icon, then attempt to grab it
						texMgr->loadPersistentTexture(GLTextureObject(Load, "RecycleBin_Empty", winOS->GetFileNameFromIcon(RecycleBin), FileIcon, NormalPriority,false));
					}
				}

				if (recycleBinStateChanged)
				{
					fsActors.front()->setTextureID(isRecycleBinFull ? "RecycleBin_Full" : "RecycleBin_Empty");
					rndrManager->invalidateRenderer();
				}
			}
		}
	}

	/*
	// XXX: we're not doing drag hover current
	if (wParam == BT_TIMERCALLBACK)
	{
		oneSecondTick();
	}
	*/
}

void WindowsSystem::OnRotate(int newWidth, int newHeight)
{
	// pass through to WM_SIZE, see http://msdn.microsoft.com/en-us/library/ms812142.aspx	
	bool isNewLandscape = (newWidth > newHeight);
	bool isCurrentLandscape = (GetWindowWidth() > GetWindowHeight());
	if (isCurrentLandscape == isNewLandscape)
		return;

	map<BumpObject *, Vec3> newPositions;
	// then we should rotate the icons so that they now face the other way
	// do this by saving the current locations with the x,z swapped and 
	// then setting them after the size has changed
	vector<BumpObject *> objects = scnManager->getBumpObjects();
	Vec3 pos;
	float tmp;
	for (int i = 0; i < objects.size(); ++i)
	{
		pos = objects[i]->getGlobalPosition();
		tmp = -pos.z;
		pos.z = pos.x;
		pos.x = tmp;
		newPositions[objects[i]] = pos;
	}

	OnSize();

	map<BumpObject *, Vec3>::const_iterator iter = newPositions.begin();
	while (iter != newPositions.end())
	{
		BumpObject * obj = iter->first;
		Quat newQ;
			newQ.multiply(Quat(90, Vec3(0,1,0)), obj->getGlobalOrientationQuat());
		Mat33 newMat(newQ);
		obj->setPoseAnim(obj->getGlobalPose(), 
			Mat34(newMat, iter->second), 30);
		iter++;
	}
} 

void WindowsSystem::OnSize()
{
	static Vec3 prevSize; 

	// update the scene overlays
	vector<OverlayLayout *> overlays = scnManager->getOverlays();
	int width = GetWindowWidth();
	int height = GetWindowHeight();
	
	// If the window is not visible (i.e. we get 0 as width or height)
	// don't resize.
	if (height == 0 || width == 0)
		return;

	Vec3 newDims(float(width), float(height), 0);
	for (int i = 0; i < overlays.size(); ++i)
	{
		overlays[i]->onSize(newDims);
	}

	// Dont' resize the walls to the work area if we are in infinite desktop mode
	if (!scnManager->isInInfiniteDesktopMode)
	{

		ResizeWallsToWorkArea(width, height);
		
		// only resize the walls if the aspect ratio has changed
		float prevAspectRatio = (prevSize.x / prevSize.y);
		float curAspectRatio = ((float) width / height);
		if ((prevAspectRatio <= 1.0f && curAspectRatio > 1.0f) ||
			(prevAspectRatio > 1.0f && curAspectRatio <= 1.0f))
		{
			// aspect ratio changed
		
			// if we're switching from a wide aspect ratio -> narrow
			bool wideToNarrowAspectRatioSwitch = prevAspectRatio > 1.0f && curAspectRatio <= 1.0f;
			int xCoefficient = wideToNarrowAspectRatioSwitch ? 1 : -1;
			int zCoefficient = wideToNarrowAspectRatioSwitch ? -1 : 1;
			float degreesToRotate = wideToNarrowAspectRatioSwitch ? -90.0f : 90.0f;

			// rotate the positions of all the objects
			for_each(BumpObject* bumpObj, scnManager->getBumpObjects())
			{
				Vec3 position = bumpObj->getGlobalPosition();
				bumpObj->setGlobalPosition(Vec3(xCoefficient*position.z, position.y, zCoefficient*position.x));

				// re-pin pinned objects, and rotate them
				// (other objects don't need to be rotated because bumptop will do that automatically for you)
				if (bumpObj->isPinned())
				{
					bumpObj->breakPin();

					Quat orientation = bumpObj->getGlobalOrientationQuat();
					Quat new_orientation;
					new_orientation.multiply(orientation, Quat(degreesToRotate, Vec3(0, 1, 0)));
					bumpObj->setGlobalOrientationQuat(new_orientation);
					bumpObj->onPin();
				}
			}

			winOS->detatchFromDesktopWindow();
			winOS->ResizeWindow();
			winOS->attachToDesktopWindow();

		}
	}
	Key_UnsetSharingMode();
#ifdef TABLE			
	Key_SetCameraAsTopDown();
#else
	Key_ZoomToSavedCamera();
#endif
	rndrManager->onSize(width, height);
	textManager->invalidate();

	prevSize = Vec3((float) width, (float) height, 0.0f);

	const vector<BumpObject *> & objs = scnManager->getBumpObjects();
	for (unsigned int i = 0; i < objs.size(); i++)
	{
		if (objs[i]->isObjectType(ObjectType(BumpActor, Webpage)))
			((WebActor *)objs[i])->updateExpectedSize();
	}
}

void WindowsSystem::OnKeyUp(WPARAM wParam)
{
	startAccumulatingInteractionTimer();
}

void WindowsSystem::OnKeyDown(WPARAM wParam)
{
	startAccumulatingInteractionTimer();
	SHORT vKey;
	uint keyModifier = 0;

	if (wParam >= KeyLeft && wParam <= KeyDown)
	{
		// Handle the Arrow Keys
		ArrowKeyCallback(wParam, 0, 0);
	}else{

		//if (wParam != KeyShift && wParam != KeyControl)
		{
			keyModifier = wParam;

			// Add in the control key
			vKey = GetAsyncKeyState(KeyControl);
			if (vKey & 0x8000) 
			{
				if (keyModifier == KeyControl) keyModifier = 0;
				keyModifier += (KeyControl << 16);
			}

			// Add in the Alt key
			vKey = GetAsyncKeyState(KeyAlt);
			if (vKey & 0x8000) 
			{
				if (keyModifier == KeyAlt) keyModifier = 0;
				keyModifier += (KeyAlt << 8);
			}

			// Add in the shift key
			vKey = GetAsyncKeyState(KeyShift);
			if (vKey & 0x8000)
			{
				if (keyModifier == KeyShift) keyModifier = 0;
				keyModifier += (KeyShift << 24);
			}

			// Handle all other Keys
			KeyboardCallback(keyModifier, 0, 0);
		}
	}
}

void WindowsSystem::OnSysKeyUp()
{
	startAccumulatingInteractionTimer();
}

void WindowsSystem::OnSysKeyDown()
{
	startAccumulatingInteractionTimer();
}

void WindowsSystem::OnSetCursor()
{
}

void WindowsSystem::OnMouseMove(int x, int y, WPARAM wParam)
{
	startAccumulatingInteractionTimer();

	if (!GLOBAL(settings).enableMultimouse)
	{
		int mouseButtons = 0;
		if ((wParam & MK_LBUTTON) != 0) mouseButtons |= MouseButtonLeft;
		if ((wParam & MK_MBUTTON) != 0) mouseButtons |= MouseButtonMiddle;
		if ((wParam & MK_RBUTTON) != 0) mouseButtons |= MouseButtonRight;
		defaultMousePointer->setMouseButtons((MouseButtons)mouseButtons);

		// Let all listeners know about mouse events
		mouseManager->onMouseEvent(WM_MOUSEMOVE, x, y, 0, defaultMousePointer);
	}

	// update the cursor message
	const int cursorOffset = 5;
	AbsoluteOverlayLayout * cursor = scnManager->cursor(NULL, NULL);
	if (cursor->getStyle().getAlpha() > 0.0f)
	{
		OverlayLayout * hor = (OverlayLayout *) cursor->items().front();
		TextOverlay * ctext = (TextOverlay *) hor->items().front();
		if (hor->getStyle().getAlpha() > 0.0f && !ctext->getText().isEmpty())
		{
			hor->getStyle().setOffset(Vec3(x + cursorOffset, GetWindowHeight() - y + hor->getSize().y + cursorOffset, 0));
			rndrManager->invalidateRenderer();
		}
	}
}

void WindowsSystem::OnMouse(UINT uMsg, int x, int y, WPARAM wParam)
{
	switch (uMsg)
	{
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		::SetCapture(windowHwnd);
		break;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		::ReleaseCapture();
		break;
	default:
		break;
	}

	startAccumulatingInteractionTimer();	
	evtManager->interruptIdleTimer();
	
	if (!GLOBAL(settings).enableMultimouse)
	{
		int mouseButtons = 0;
		if ((wParam & MK_LBUTTON) != 0) mouseButtons |= MouseButtonLeft;
		if ((wParam & MK_MBUTTON) != 0) mouseButtons |= MouseButtonMiddle;
		if ((wParam & MK_RBUTTON) != 0) mouseButtons |= MouseButtonRight;
		defaultMousePointer->setMouseButtons((MouseButtons)mouseButtons);

		mouseManager->onMouseEvent(uMsg, x, y, GET_WHEEL_DELTA_WPARAM(wParam), defaultMousePointer);
	}
}

Gdiplus::Bitmap* WindowsSystem::GetIconGraphic(int Loc)
{	
	winOS->GetLargeIconSupport();	

	SHFILEINFO FileInfo = {0}; 
	LPITEMIDLIST TempPidl;
	HICON IconHandle;
	int IconIndex = -1;
	QString textureID;

	// Retrieve the Desktop path
	if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, Loc, &TempPidl)))
	{
		// Get a list of files specified by Path
		SHGetFileInfo((LPCTSTR) TempPidl, -1, &FileInfo, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_PIDL | SHGFI_SYSICONINDEX);
		IconIndex = FileInfo.iIcon;
		IconHandle = largeIconSupport ? GetSystemIcon("", IconIndex) : FileInfo.hIcon;

		textureID.append("*");
		textureID.append(ICON_PATH_CONCAT);
		textureID.append(QString::number(IconIndex));

		// Release the PIDL
		CoTaskMemFree(TempPidl);
	}

	if (IconIndex > -1)
	{
		// Get the pixel data and add it to the map
		return GetIconPixelData(IconHandle); //new Gdiplus::Bitmap(IconHandle);
	}

	timerAnchor = timeGetTime();
	return NULL;
}

bool WindowsSystem::isInvalidBitmap(Gdiplus::Bitmap * bitmap)
{
	const int minAlphaColor = (1 << 5) - 1;	// 31
	bool isInvalid = true;
	UINT height = bitmap->GetHeight();
	UINT width = bitmap->GetWidth();
	if ((height > 0) && (width > 0))
	{
		Gdiplus::Color color;
		UINT x = height / 2;
		UINT y = width / 2;

		// check the horizontal cross-section
		for (int i = 2; i < width && isInvalid; ++i)
		{
			bitmap->GetPixel(i, y, &color);
			if (color.GetAlpha() > minAlphaColor)
				isInvalid = false;
		}

		// check the vertical cross-section
		for (int i = 2; i < height && isInvalid; ++i)
		{
			bitmap->GetPixel(x, i, &color);
			if (color.GetAlpha() > minAlphaColor)
				isInvalid = false;
		}
	}
	return isInvalid;
}

bool WindowsSystem::isInvalidBitmap(const BITMAP& bm)
{
	const unsigned char minAlphaColor = (1 << 5) - 1;	// 31
	bool isInvalid = true;
	UINT height = bm.bmHeight;
	UINT width = bm.bmWidth;
	int bytesPerPixel = bm.bmBitsPixel / 8;
	unsigned char * bytes = (unsigned char *) bm.bmBits;
	unsigned int pixelIndex = 0;
	unsigned char tmpAlpha = 0;

	if ((height > 0) && (width > 0) && (bytesPerPixel == 4))
	{
		unsigned int x = height / 2;
		unsigned int y = width / 2;

		// check the horizontal cross-section
		for (int i = 2; i < width && isInvalid; ++i)
		{
			// NOTE: this is the current pixel index at the current scan line
			pixelIndex = (y * width * bytesPerPixel) + (i * bytesPerPixel) + 3;
			tmpAlpha = bytes[pixelIndex];
			if (tmpAlpha > minAlphaColor)
				isInvalid = false;
		}

		// check the vertical cross-section
		for (int i = 2; i < height && isInvalid; ++i)
		{
			// NOTE: this is the current pixel index at the current scan line
			pixelIndex = (i * width * bytesPerPixel) + (x * bytesPerPixel) + 3;
			tmpAlpha = bytes[pixelIndex];
			if (tmpAlpha > minAlphaColor)
				isInvalid = false;
		}
	}
	else
	{
		assert(false);
	}
	return isInvalid;
}

Gdiplus::Bitmap* WindowsSystem::GetIconGraphic(QString Path)
{
	winOS->GetLargeIconSupport();					
	uint indx = 0;
	QString Rtrn;
	HICON hIcon;

	try
	{
		// Get a the file that reside at this Location
		Rtrn = Path;

		// If this item does not exist in the map
		if (!texMgr->isTextureState(Rtrn, TextureLoaded))
		{
			QStringList tok = Rtrn.split(ICON_PATH_CONCAT);
			assert(tok.size() > 1);
			if (tok.size() <= 1)
				return NULL;
			bool isNum = false;
			indx = tok[1].toInt(&isNum);		
			assert(isNum);
			hIcon = GetSystemIcon(tok[0], indx);

			// In case of the file not being there (listener will handle deletion) get the default icon based on its extension
			if (!hIcon)
			{
				timerAnchor = timeGetTime();
				return NULL;
			}

			// Get the pixel data and add it to the map
			Gdiplus::Bitmap * bitmap = GetIconPixelData(hIcon);

			// check if this is a valid icon, or an improperly scaled one
			if (IsWindowsVersionGreaterThanOrEqualTo(WindowsVista))
			{
				// check if the icon has all alpha along it's crosses
				if (isInvalidBitmap(bitmap))
				{
					// if this is in-fact an invalid bitmap, then reload it?
					winOS->GetLargeIconSupport(true);		
					hIcon = GetSystemIcon(tok[0], indx);

					// In case of the file not being there (listener will handle deletion) get the default icon based on its extension
					if (!hIcon)
					{
						timerAnchor = timeGetTime();
						return NULL;
					}

					// Get the pixel data and add it to the map
					bitmap = GetIconPixelData(hIcon);
				}
			}

			return bitmap;
		}
	}catch (std::exception &)
	{
		timerAnchor = timeGetTime();
		return NULL;
	}

	timerAnchor = timeGetTime();
	return NULL;
}

Gdiplus::Bitmap *WindowsSystem::GetIconPixelData(HICON hIcon)
{
	ICONINFO iconInfo;
	GetIconInfo(hIcon, &iconInfo);

	BITMAP iconBmp = {0};
	GetObject(iconInfo.hbmColor, sizeof(BITMAP),&iconBmp);

	unsigned int width = iconBmp.bmWidth, height = iconBmp.bmHeight;
	Gdiplus::Rect rect(0, 0, width, height);

	Gdiplus::Bitmap* bitmap = new Gdiplus::Bitmap(width, height, PixelFormat32bppARGB);
	Gdiplus::BitmapData bitmapData = {0};
	bitmap->LockBits(&rect, Gdiplus::ImageLockModeWrite, PixelFormat32bppARGB, &bitmapData);
	_ASSERT(bitmapData.Stride > 0);
	bool hasAlpha = false;
	{
		// We have to read the raw pixels of the bitmap to get proper transparency information
		// (not sure why, all we're doing is copying one bitmap into another)

		Gdiplus::Bitmap colorBitmap(iconInfo.hbmColor, NULL);
		Gdiplus::BitmapData colorData = {0};
		
		// !!! We must lock using colorBitmap.GetPixelFormat(). 
		// It could be 32bpp RGB but still contain alpha information, so if it's locked with ARGB, 
		// GDI sets alpha to 0 even though alpha exists.
		if (32 != Gdiplus::GetPixelFormatSize(colorBitmap.GetPixelFormat()))
			colorBitmap.LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &colorData);
		else
			colorBitmap.LockBits(&rect, Gdiplus::ImageLockModeRead, colorBitmap.GetPixelFormat(), &colorData);
		unsigned int stride = abs(colorData.Stride);
		if (colorData.Stride < 0) // If the pixel data is stored bottom-up, the Stride data member is negative.
		{ // !!! Scan is actually backwards. So Scan0 is pointing to start of last (top) scan line.
			for (unsigned int y = 0; y < height; y++)
			{
				unsigned int * colorScan = (unsigned int *)colorData.Scan0 - y * width;
				memcpy((unsigned int *)bitmapData.Scan0 + y * width, colorScan, stride);
				unsigned int colorOR = 0, colorAND = 0xFF000000;
				if (!hasAlpha)
				{
					for (const unsigned int * colorPixel = colorScan; colorPixel < colorScan + width; colorPixel++)
					{
						colorOR |= *colorPixel;
						colorAND &= *colorPixel;
					}
					hasAlpha = ((colorOR & 0xFF000000) != 0) && (colorAND != 0xFF000000); // hasAlpha if not all 255 or 0 alpha
				}
			}

		}
		else
		{
			memcpy(bitmapData.Scan0, colorData.Scan0, stride * height);
			unsigned int colorOR = 0, colorAND = 0xFF000000;
			for (const unsigned int * colorPixel = (unsigned int *)colorData.Scan0; colorPixel < (unsigned int *)colorData.Scan0 + width * height; colorPixel++)
			{
				colorOR |= *colorPixel;
				colorAND &= *colorPixel;
			}
			hasAlpha = ((colorOR & 0xFF000000) != 0) && (colorAND != 0xFF000000); // hasAlpha if not all 255 or 0 alpha
		}		
			
		colorBitmap.UnlockBits(&colorData);
	}

	if (!hasAlpha)
	{
		// If there's no alpha transparency information, we need to use the mask to turn back on visible pixels
		// White 0xFFFFFFFF means invisible, black 0xFF000000 means visible
		Gdiplus::Bitmap maskBitmap(iconInfo.hbmMask,NULL);
		Gdiplus::BitmapData maskData = {0};
		maskBitmap.LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &maskData);
		_ASSERT(maskData.Stride > 0);
		unsigned int * maskPixel = (unsigned int *) maskData.Scan0;
		unsigned int * bitmapPixel = (unsigned int *) bitmapData.Scan0;
		for (unsigned int i = 0; i < width * height; i++)
		{
			const unsigned int mask = ~((*maskPixel) << 24); 
			*bitmapPixel |= mask & 0xFF000000; // Turn pixel opaque if mask is not white
			*bitmapPixel &= mask; // Turn pixel transparent if mask is white.
			bitmapPixel++;
			maskPixel++;
		}
		maskBitmap.UnlockBits(&maskData);
	}
	
	bitmap->UnlockBits(&bitmapData);
	
	// free the icon
	DestroyIcon(hIcon);

	return bitmap;
}

ColorVal WindowsSystem::GetWindowsBackgroundColor() {
	HKEY hKey;
	TCHAR lszValue[MAX_PATH];
	DWORD dwType = REG_SZ;
	DWORD dwSize = MAX_PATH;
	LONG returnStatus = RegOpenKeyEx(HKEY_CURRENT_USER, L"Control Panel\\Colors", 0L, KEY_READ, &hKey);
	if (returnStatus == ERROR_SUCCESS)
	{
		returnStatus = RegQueryValueEx(hKey, L"Background", NULL, &dwType, (LPBYTE)&lszValue, &dwSize);
		if (returnStatus == ERROR_SUCCESS)
		{
			QString backgroundString = QString::fromUtf16((const ushort *) lszValue);
			uchar r, g, b;
			int div1, div2; //indices of the two spaces

			div1 = backgroundString.indexOf(" ");
			div2 = backgroundString.lastIndexOf(" ");

			r = backgroundString.mid(0,div1).toInt();
			g = backgroundString.mid(div1,div2-div1).toInt();
			b = backgroundString.mid(div2+1,-1).toInt();

			return ColorVal(255,r,g,b);
		}
	}

	return ColorVal(255,0,0,0);
}

WindowsSystem::WallPaperStyle WindowsSystem::GetWallPaperStyle() {

	HKEY hKey;
	TCHAR lszValue[MAX_PATH];
	DWORD dwType = REG_SZ;
	DWORD dwSize = MAX_PATH;
	LONG returnStatus = RegOpenKeyEx(HKEY_CURRENT_USER, L"Control Panel\\Desktop", 0L, KEY_READ, &hKey);
	if (returnStatus == ERROR_SUCCESS)
	{
		returnStatus = RegQueryValueEx(hKey, L"TileWallpaper", NULL, &dwType, (LPBYTE)&lszValue, &dwSize);
		if (returnStatus == ERROR_SUCCESS)
		{
			QString styleString = QString::fromUtf16((const ushort *) lszValue);
			if (styleString != "0")
				return WindowsSystem::TILED;
		}

		TCHAR lszValue2[MAX_PATH];
		DWORD dwType2 = REG_SZ;
		DWORD dwSize2 = MAX_PATH;

		returnStatus = RegQueryValueEx(hKey, L"WallpaperStyle", NULL, &dwType2, (LPBYTE)&lszValue2, &dwSize2);
		if (returnStatus == ERROR_SUCCESS)
		{
			QString styleString = QString::fromUtf16((const ushort *) lszValue2);
			if(styleString == "0")
				return WindowsSystem::CENTER;
			else if (styleString == "2")
				return WindowsSystem::STRETCH;
			else if (styleString == "6")
				return WindowsSystem::FIT;
			else if (styleString == "10")
				return WindowsSystem::FILL;
		}
		RegCloseKey(hKey);
	}

	return WindowsSystem::UNABLETOREAD;
}

QString WindowsSystem::GetWallPaperPath()
{
	// NOTE: we use the converted wallpaper path in the registry where possible because
	// windows converts that into Wallpaper1.bmp which is reported via SystemParametersInfo
	TCHAR lszValue[MAX_PATH];
	HKEY hKey;
	DWORD dwType = REG_SZ;
	DWORD dwSize = MAX_PATH;
	LONG returnStatus = RegOpenKeyEx(HKEY_CURRENT_USER, L"Control Panel\\Desktop", 0L, KEY_READ, &hKey);
	if (returnStatus == ERROR_SUCCESS)
	{
		returnStatus = RegQueryValueEx(hKey, L"ConvertedWallpaper", NULL, &dwType, (LPBYTE)&lszValue, &dwSize);
		if (returnStatus == ERROR_SUCCESS)
		{
			RegCloseKey(hKey);
			return QString::fromUtf16((const ushort *) lszValue);
		}
		RegCloseKey(hKey);
	}

	// poll for it the normal way via SystemParametersInfo
	TCHAR wallPaperPath[MAX_PATH] = { 0 };
	bool loadCheck = false;

	// Get the path the the current wallpaper
	SystemParametersInfo(SPI_GETDESKWALLPAPER, MAX_PATH, wallPaperPath, NULL);
	return QString::fromUtf16((const ushort *) wallPaperPath);
}

HICON WindowsSystem::GetSystemIcon(QString FilePath, int indx)
{
	/*
	path p(FilePath, native);
	if (exists(p))
	{
		IExtractIcon * pei = NULL;
		IShellFolder2 * psf = winOS->GetShellFolderFromAbsDirPath(p.branch_path().native_directory_string());
		LPCITEMIDLIST * pidlArray = new LPCITEMIDLIST[1];
			pidlArray[0] = winOS->GetRelativePidlFromAbsFilePath(FilePath);
		if (SUCCEEDED(psf->GetUIObjectOf(NULL, 1, pidlArray, IID_IExtractIcon, NULL, (void **) &pei)))
		{
			int iconIndex = 0;
			TCHAR iconPath[MAX_PATH];
			UINT result = 0;
			
			if (SUCCEEDED(pei->GetIconLocation(GIL_FORSHELL, iconPath, MAX_PATH, &iconIndex, &result)))
			{
				HICON largeIcon;
				HICON smallIcon;
				if (SUCCEEDED(pei->Extract(iconPath, iconIndex, &largeIcon, &smallIcon, 256)))
				{
					return largeIcon ? largeIcon : smallIcon;
				}
			}
		}
	}
	*/

	
	SHFILEINFO FileInfo = {0}; 
	HICON largeIcon = NULL;
	UINT rtrn = NULL, iconId;

	if (indx == -1 || !GLOBAL(settings).enableHiResIcons || (FilePath.size() > 3 && FilePath.contains("lnk")))
	{
		// Get a list of files specified by Path
		SHGetFileInfo((LPCWSTR) FilePath.utf16(), -1, &FileInfo, sizeof(SHFILEINFO), SHGFI_ICON);
		return FileInfo.hIcon;
	}else{
		// Lower Case it
		FilePath = FilePath.toLower();

		// Poll using this function (EXE only)
		if (FilePath.size() > 3 && FilePath.contains("exe"))
		{
			// Get the high res version of this icon
			if (GLOBAL(settings).enableHiResIcons && FilePath.contains("lnk"))
			{
				fsManager->getShortcutTarget(FilePath, &FilePath);
			}

			rtrn = PrivateExtractIcons((LPCWSTR) FilePath.utf16(), 0, 256, 256, &largeIcon, &iconId, 1, 0);

			if (!largeIcon)
			{
				// Failsafe if the above line fails, passes it on to the ImageList
				SHGetFileInfo((LPCWSTR) FilePath.utf16(), -1, &FileInfo, sizeof(SHFILEINFO), SHGFI_SYSICONINDEX);
				indx = FileInfo.iIcon;
			}
		}

		if (largeIcon == NULL)
		{
			// Poll all other filenames using this functions (non-EXE)
			if (largeIcons == NULL || !largeIconSupport || 
				S_OK != largeIcons->GetIcon(indx, ILD_NORMAL, &largeIcon))
			{
				// Failsafe if the above line fails
				// (gets the 32x32 icon)
				SHGetFileInfo((LPCWSTR) FilePath.utf16(), -1, &FileInfo, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_LARGEICON);
				return FileInfo.hIcon;		
			}
		}

		return largeIcon;
	}
}

QString WindowsSystem::GetSystemIconInfo(QString FileName)
{
	if (FileName.isEmpty())
		return QString();

	QString thisFileName(FileName);
	SHFILEINFO FileInfo = {0}; 
	int Indx = 0;
	bool Rtrn = false;

	if (GLOBAL(settings).enableHiResIcons && 
		FileName.endsWith(".lnk", Qt::CaseInsensitive))
	{
		// Resolve the LNK target
		fsManager->getShortcutTarget(thisFileName, &thisFileName);
		bool loadLinkIcon = false;
		QString fullPathExtension = fsManager->getFileExtension(thisFileName);
		// NOTE: we only get the full high-res icons if we pull the data from the exe
		// if (fullPathExtension != ".exe")
			loadLinkIcon = true;

		if (loadLinkIcon)
		{
			if (GLOBAL(settings).useThemeIconOverrides &&
				(fsManager->getFileAttributes(thisFileName) & Directory) && 
				!winOS->IsWindowsVersionGreaterThanOrEqualTo(WindowsVista))
			{
				return "override.ext.folder";
			}
			else if (fsManager->getFileAttributes(thisFileName))
			{
				thisFileName = FileName;
			}
		}
	}

	// Get its Index
	SHGetFileInfo((LPCWSTR) thisFileName.utf16(), -1, &FileInfo, sizeof(SHFILEINFO), SHGFI_SYSICONINDEX);
	Indx = FileInfo.iIcon;

	thisFileName = thisFileName.toLower();

	// Cope over the path of the resource
	QString temp;
	if (thisFileName.contains("exe"))
	{
		temp.append(thisFileName);
		Indx = 0;
	}else{
		temp.append("*");
	}

	temp.append(ICON_PATH_CONCAT);
	temp.append(QString::number(Indx));
	return temp;
}

QString WindowsSystem::GetFileTypeDescription(QString filename)
{
	SHFILEINFO FileInfo = {0};

	// Get the type of file
	SHGetFileInfo((LPCWSTR) filename.utf16(), 
				  -1,
				  &FileInfo, 
				  sizeof(SHFILEINFO), 
				  SHGFI_TYPENAME);

	return QString::fromUtf16((const ushort *) FileInfo.szTypeName);
}

QString WindowsSystem::GetSystemPath(int SystemLocation)
{
	// return the cached value if possible
	if (_virtualIconTypePathMap.contains(SystemLocation))
		return _virtualIconTypePathMap[SystemLocation];

	QString pathStr;
	TCHAR path[MAX_PATH];

	if (SystemLocation == DesktopDirectory)
	{
		// special case for the sandbox mode
		if (scnManager->isInSandboxMode)
			pathStr = native(winOS->GetExecutableDirectory() / "SandboxDesktop");

		// special case for unit testing mode
		else if (scnManager->runBumpTopTestsOnStartup) 
			pathStr = native(winOS->GetExecutableDirectory() / "UnitTestDesktop");

		if (!pathStr.isEmpty())
		{
			_virtualIconTypePathMap.insert(Desktop, pathStr);
			return pathStr;
		}
	}

	LPITEMIDLIST itemIdList;

	// Retrieve the Desktop path
	if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, SystemLocation, &itemIdList)))
	{
		if (SUCCEEDED(SHGetPathFromIDList(itemIdList, path)))
		{	
			// set the path string
			pathStr = QString::fromUtf16((const ushort *) path);
			LOG(QString_NT("special folder %1: %2").arg(SystemLocation).arg(pathStr));
			_virtualIconTypePathMap.insert(SystemLocation, pathStr);
		}
		CoTaskMemFree(itemIdList);
	}

	return pathStr;
}

QDir WindowsSystem::GetUserApplicationDataPath()
{
	static QDir btAppDataPath;
	QString str = btAppDataPath.absolutePath();
	if (empty(btAppDataPath))
	{
		QDir appPath = winOS->GetSystemPath(CSIDL_APPDATA);
		QDir companyPath = appPath / "Bump Technologies, Inc";
		QDir productName = companyPath / "BumpTop";

		// ensure company path exists	
		if (!exists(companyPath))
			create_directory(companyPath);

		// ensure product path exists
		if (!exists(productName))
			create_directory(productName);

		btAppDataPath = productName;
	}
	return btAppDataPath;
}

QDir WindowsSystem::GetUserLocalApplicationDataPath()
{
	static QDir btAppDataPath;
	if (empty(btAppDataPath))
	{
		QDir appPath = winOS->GetSystemPath(CSIDL_LOCAL_APPDATA);
		QDir companyPath = appPath / "Bump Technologies, Inc";
		QDir productName = companyPath / "BumpTop";

		// ensure company path exists	
		if (!exists(companyPath))
			create_directory(companyPath);

		// ensure product path exists
		if (!exists(productName))
			create_directory(productName);

		btAppDataPath = productName;
	}
	return btAppDataPath;
}

HWND WindowsSystem::GetWindowsHandle()
{
    // Get current DC from wgl
    return windowHwnd;
}

vector<iconDetails> WindowsSystem::GetIconPositionsHelper()
{
	HWND listViewHwnd = 0;
	HANDLE explorer = 0;
	unsigned long iconCount = 0;
	iconDetails* iconBuffer = 0;
	void* iconPos = 0;
	void* iconLabel = 0;
	TCHAR* iconNameBuffer = 0;
	bool error = false;
	LRESULT result = 0;
	POINT iconPoint;
	LVITEM iconListLabel;
	TCHAR buffer[sizeof(TCHAR) * (MAX_PATH+1)];
	vector<iconDetails> iconDetailsList;

	// Get the HWND of the listview
	listViewHwnd = FindListView();

	// Get the total number of icons on the desktop
	iconCount = (unsigned long) SendMessage(listViewHwnd, LVM_GETITEMCOUNT, 0, 0);

	// DOnt bother iterating when no icons exist
	if (iconCount > 0)
	{
		// Get the PID of the process that houses the listview, i.e.: Explorer.exe
		explorer = FindExplorerProcess(listViewHwnd);

		// Here we allocate the shared memory buffers to use in our little IPC.
		iconPos = VirtualAllocEx(explorer, NULL, sizeof(POINT), MEM_COMMIT, PAGE_READWRITE);
		if (!winOS->Is64BitWindows())
		{
			iconLabel = VirtualAllocEx(explorer, NULL, sizeof(LVITEM), MEM_COMMIT, PAGE_READWRITE);
			iconNameBuffer = (TCHAR *) VirtualAllocEx(explorer, NULL, sizeof(sizeof(TCHAR) * (MAX_PATH + 1)), MEM_COMMIT, PAGE_READWRITE);
		}


		try
		{
			iconBuffer = new iconDetails;
		} catch (char *)
		{
			MessageBox(GetForegroundWindow(), (LPCWSTR)QT_TR_NOOP("Error Occured in WindowsSystem::GetIconPositions(). \n"
				"Could Not Allocated Memory for this Task.").utf16(), (LPCWSTR)QT_TR_NOOP("Icon Fetching Error").utf16(), MB_OK | MB_ICONERROR);
			exit(0);
		}

		for (int i = 0; i < iconCount; i++)
		{
			// First we get the icon position.
			result = SendMessage(listViewHwnd, LVM_GETITEMPOSITION, i, (LPARAM) iconPos);

			if (result) 
			{
				// Get the data from the shared memory
				ReadProcessMemory(explorer, iconPos, &iconPoint, sizeof(POINT), NULL);


				// Set stuff up to retrieve the label of the icon.
					iconListLabel.iSubItem = 0;
					iconListLabel.cchTextMax = MAX_PATH;
					iconListLabel.mask = LVIF_TEXT;
					iconListLabel.pszText = (LPTSTR) iconNameBuffer;


					// Request the label.
					// Write the list source
					WriteProcessMemory(explorer, iconLabel, &iconListLabel, sizeof(LVITEM), NULL);

					// this causes lots of badness on 64-bit windows. since we're trying to pass
					// pointers from a 32-bit process to a 64-bit one, something is getting messed
					// up. explorer memory usage balloons by hundreds of megabytes.
					// plus, it doesn't work anyway. -mvj
					result = SendMessage(listViewHwnd, LVM_GETITEMTEXT, i, (LPARAM) iconLabel);
				


				if (SUCCEEDED(result))
				{
					ReadProcessMemory(explorer, iconNameBuffer, &buffer, sizeof(buffer), NULL);

					iconBuffer->x = iconPoint.x;
					iconBuffer->y = iconPoint.y;
					iconBuffer->iconName = QString::fromUtf16((const ushort *) buffer);
					iconBuffer->index = i;

					iconDetailsList.push_back(*iconBuffer);

					
				}
			}
		}

		SAFE_DELETE(iconBuffer);

		// Always clear up afterwards.
		VirtualFreeEx(explorer, iconPos, NULL, MEM_RELEASE);
		VirtualFreeEx(explorer, iconLabel, NULL, MEM_RELEASE);
		VirtualFreeEx(explorer, iconNameBuffer, NULL, MEM_RELEASE);

		CloseHandle(explorer);
	}

	return iconDetailsList;

}

vector<iconDetails> WindowsSystem::GetIconPositionsHelper64()
{
	vector<iconDetails> iconDetailsList;

	QDir getIconPositionsApp = winOS->GetExecutableDirectory() / "x64" / "GetIconPositions.exe";
	QString iconPositionsAppStr = native(getIconPositionsApp);
	QString dataDirectoryStr = native(winOS->GetDataDirectory());

	STARTUPINFO         si;
	PROCESS_INFORMATION pi;

	ZeroMemory (&si, sizeof(si));
	si.cb=sizeof (si);
	BOOL success = CreateProcess((LPCWSTR) iconPositionsAppStr.utf16(),
		NULL,
		NULL,
		NULL,
		FALSE,
		CREATE_NO_WINDOW,
		NULL,
		(LPCWSTR) dataDirectoryStr.utf16(),
		&si,
		&pi);
	if (!success)
		return iconDetailsList;
	WaitForSingleObject(pi.hProcess, INFINITE);
	
	QFileInfo icon_positions_filename =	make_file(dataDirectoryStr, "iconpositions.json");
	if (!exists(icon_positions_filename))
		return iconDetailsList;

	Json::Reader reader;
	Json::Value root;

	QString iconPositionsStr = read_file_utf8(native(icon_positions_filename));
	QFile::remove(icon_positions_filename.absoluteFilePath());

	QByteArray tmp = iconPositionsStr.toUtf8();
	reader.parse(tmp.constData(), root);

	for (int i = 0; i < root.size(); i++)
	{
		Json::Value iconposition_dict = root[i];
		if (!iconposition_dict.isNull())
		{
			iconDetails id;
			id.iconName = qstringFromValue(iconposition_dict["name"]);
			id.x = iconposition_dict["xpos"].asInt();
			id.y = iconposition_dict["ypos"].asInt();
			iconDetailsList.push_back(id);
		}
	}

	return iconDetailsList;
}


//This function forcefully pulls the icon listing and positions out of Explorer.exe for icons on the Desktop
bool WindowsSystem::GetIconPositions(vector<iconDetails> &icons)
{
	bool rc = false;
	StrList searchPath;
	StrList activeFileList;

	vector<iconDetails> iconDetailsList = winOS->Is64BitWindows() ? GetIconPositionsHelper64() : GetIconPositionsHelper();


	// For now, we do not get the icon positions for non-desktop working
	// directories
	if (native(scnManager->getWorkingDirectory()) != winOS->GetSystemPath(DesktopDirectory))
		return rc;

	// DOnt bother iterating when no icons exist
	if (iconDetailsList.size() > 0)
	{
		// Get a Listing of all files in the directories we are using
		searchPath.push_back(native(scnManager->getWorkingDirectory()));
		if (searchPath.front() == GetSystemPath(DesktopDirectory))
			searchPath.push_back(GetSystemPath(AllUsersDesktopDir));

		// Loop through the two places where these Icons can exist
		StrList::const_iterator searchPathIter = searchPath.begin();
		while (searchPathIter != searchPath.end())
		{
			//path P(searchPath[i], native);
			//directory_iterator end_itr;
			StrList dirListing = fsManager->getDirectoryContents(*searchPathIter);

			//for (directory_iterator itr(P); itr != end_itr ; ++itr)
			for (int i = 0; i < dirListing.size(); i++)
			{
				activeFileList.push_back(dirListing[i]);
			}

			searchPathIter++;
		}


		for_each(iconDetails iconBuffer, iconDetailsList)
		{

			// ***HACK*** This checks to see if the item is a file or not
			bool exitFlag = false;
			QString filePath;

			if (GetIconTypeFromFileName(iconBuffer.iconName) != -1)
			{
				// This is a VRTUAL folder
				exitFlag = true;
				iconBuffer.VIRTUALFolder = true;
			}else{
				// Check against a directory Listing
				for (int j = 0; !exitFlag && j < activeFileList.size(); j++)
				{
					QFileInfo info(activeFileList[j]);
					QString bName = info.baseName(), lName = info.fileName();

					if (lName == iconBuffer.iconName || bName == iconBuffer.iconName)
					{
						// Save the file path and remove it form the activeListing.
						iconBuffer.iconName = activeFileList[j];
						activeFileList.erase(activeFileList.begin() + j);
						exitFlag = true;
					}
				}

				// The Previous Step didnt work, try the basename search
				if (!exitFlag)
				{
					// Check against a directory Listing
					for (int j = 0; !exitFlag && j < activeFileList.size(); j++)
					{
						QFileInfo activeInfo(activeFileList[j]);
						QFileInfo iconInfo(iconBuffer.iconName);
						QString truncFileName, truncExplorerFileName;

						bool isHidden = (fsManager->getFileAttributes(activeFileList[j]) & Hidden) > 0;
						if (!isHidden || GLOBAL(settings).LoadHiddenFiles)
						{
							// ***HACK*** Try to get the name of the file without the extension
							try
							{
								truncFileName = activeInfo.baseName();
								truncExplorerFileName = iconInfo.baseName();
							}
							catch(std::exception &)
							{
								consoleWrite(QString("The following File could not be BaseNamed: \n\t%1\n").arg(activeFileList[j]));
							}

							// Chop off the absolute path and extions and compare it to the icon name
							if (truncFileName == truncExplorerFileName)
							{
								// Save the file path and remove it form the activeListing.
								iconBuffer.iconName = activeFileList[j];
								activeFileList.erase(activeFileList.begin() + j);
								exitFlag = true;
							}
						}
					}
				}

				// The previous step didnt work, try a brute force method
				if (!exitFlag)
				{
					// Check against a directory Listing
					for (int j = 0; !exitFlag && j < activeFileList.size(); j++)
					{
						exitFlag = true;

						for (int k = 0; k < activeFileList[j].size() && k < iconBuffer.iconName.size(); k++)
						{
							if (activeFileList[j][k] != iconBuffer.iconName[k])
							{
								exitFlag = false;
							}
						}

						if (exitFlag)
						{
							// Save the file path and remove it form the activeListing.
							iconBuffer.iconName = activeFileList[j];
							activeFileList.erase(activeFileList.begin() + j);
						}
					}
				}

				// If the previous attempt failed, try adding it in if the file exists on disk
				// If the icon name is empty, we're probably on a 64-bit system and unable to read
				// any of the icon names. The code immediately above randomly assigned file paths
				// to icons positions, but there may be a few "extra" icons-- virtual icons -- which don't
				// exist in the filesystem. We keep them as empty strings so that later on we can recognize 
				// them and do some post-processing
				if (!exitFlag && iconBuffer.iconName.isEmpty())
				{
					//iconBuffer.iconName = ""; // keep the name as an empty string
					exitFlag = true;
				}
				else if (!exitFlag && !iconBuffer.iconName.isEmpty())
				{
					searchPathIter = searchPath.begin();
					while (searchPathIter != searchPath.end()) 
					{
						QFileInfo iconPath = make_file(*searchPathIter, iconBuffer.iconName);
						if (fsManager->getFileAttributes(native(iconPath))) {
							// This file Exists on Disk, so add it in.
							iconBuffer.iconName = native(iconPath);
							exitFlag = true;
						}
						searchPathIter++;
					}
				}
			}

			if (exitFlag)
			{
				icons.push_back(iconBuffer);
				rc = true;
			}else{
				// I have no idea where this file is.
				consoleWrite(QString("The file [%1] Cannot be found.\n").arg(iconBuffer.iconName));
			}
		}
	}
		

	timerAnchor = timeGetTime();

	return rc;
}

QString	WindowsSystem::GetUIDFromVirtualIcon(int icon)
{
	// these values correspond to special icons on the desktop
	switch (icon)
	{
	case MyComputer:
		return QT_NT("{20D04FE0-3AEA-1069-A2D8-08002B30309D}");
	case RecycleBin:
		{
			if (IsWindowsVersionGreaterThanOrEqualTo(WindowsVista))
				return QT_NT("{645FF040-5081-101B-9F08-00AA002F954E}");
			else
				assert(false);
			break;
		}
	case NetworkNeighborhood:
		{
			if (IsWindowsVersionGreaterThanOrEqualTo(WindowsVista))
				return QT_NT("{F02C1A0D-BE21-4350-88B0-7367FC96EF3C}");
			else
				return QT_NT("{208D2C60-3AEA-1069-A2D7-08002B30309D}");
		}
	case MyDocuments:
		{
			if (IsWindowsVersionGreaterThanOrEqualTo(WindowsVista))
				return QT_NT("{59031a47-3f72-44a7-89c5-5595fe6b30ee}");
			else
				return QT_NT("{450D8FBA-AD25-11D0-98A8-0800361B1103}");
		}
	case InternetExplorer:
		return QT_NT("{871C5380-42A0-1069-A2EA-08002B30309D}");
	case ControlPanel:
		{
			if (IsWindowsVersionGreaterThanOrEqualTo(WindowsVista))
				return QT_NT("{5399E694-6CE5-4D6C-8FCE-1D8870FDCBA0}");
			else
				assert(false);
			break;
		}
	default:
		break;
	}
	return QString();
}

void WindowsSystem::GetIconAvailability(int icon, bool& visibleOut)
{
	const QString visibleDesktopIconsRegPath(QT_NT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\HideDesktopIcons\\NewStartPanel"));
	HKEY regKey = NULL;

	if (ERROR_SUCCESS == 
		RegOpenKeyEx(HKEY_CURRENT_USER, (LPCWSTR) visibleDesktopIconsRegPath.utf16(), NULL, KEY_READ, &regKey))
	{
		DWORD valueType = REG_DWORD;
		DWORD valueBuffer = 0;
		DWORD valueBufferSize = sizeof(DWORD);

		// get the value name for the specific icon
		QString valueName = GetUIDFromVirtualIcon(icon);
		assert(!valueName.isEmpty());

		// query the value
		if (ERROR_SUCCESS == 
			RegQueryValueEx(regKey, (LPCWSTR) valueName.utf16(), NULL, &valueType,(LPBYTE) &valueBuffer, &valueBufferSize))
		{
			// if buffer is 1, then the icon is hidden, else
			// if buffer is 0, then the icon is visible
			visibleOut = (valueBuffer == 0);
		} else { 
			//Key does not yet exist for this icon, this means user has not toggled it. Recycle bin is on by default, the others are off by default.
			visibleOut = (icon == RecycleBin);
		}

		// close the key
		RegCloseKey(regKey);
	} else { 
		//The HideDesktopIcons key does not exist (fresh user, has not yet toggled desktop icons)
		visibleOut = (icon == RecycleBin); //Recycle bin is on by default, the others are off by default
	}
}

bool WindowsSystem::SetIconAvailability(int icon, bool visible)
{
	const QString visibleDesktopIconsRegPath(QT_NT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\HideDesktopIcons\\NewStartPanel"));
	HKEY regKey = NULL;
	bool result = false;

	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, (LPCWSTR) visibleDesktopIconsRegPath.utf16(), NULL, KEY_WRITE, &regKey))
	{
		DWORD isHidden = (visible ? 0 : 1);

		// get the value name for the specific icon
		QString valueName = GetUIDFromVirtualIcon(icon);
		assert(!valueName.isEmpty());

		// set the value
		if (ERROR_SUCCESS == 
			RegSetValueEx(regKey, (LPCWSTR) valueName.utf16(), NULL, REG_DWORD, (const BYTE *) &isHidden, sizeof(isHidden)))
		{
			result = true;
		}

		// close the key
		RegCloseKey(regKey);
	}

	return result;
}

HWND WindowsSystem::FindListView()
{
	HWND progmanHwnd = 0;
	HWND desktopViewHwnd = NULL;
	HWND listViewHwnd = NULL;

	// First find the main window of program that houses the desktop.
	progmanHwnd = FindWindow(NULL, L"Program Manager");

	if (progmanHwnd)
	{
		// Then get the desktop window
		desktopViewHwnd = FindWindowEx(progmanHwnd, NULL, L"SHELLDLL_DefView", NULL);
		
		if (desktopViewHwnd)
		{
			// Finally get the handle to the listview on the desktop.
			listViewHwnd = FindWindowEx(desktopViewHwnd, NULL, L"SysListView32", NULL);
		}
	}

	return listViewHwnd;
}

HANDLE WindowsSystem::FindExplorerProcess(HWND slaveHwnd)
{ 
	HANDLE proc;
	DWORD explorerPid;
	
	// Get the PID based on a HWND. This is the good stuff. You wouldn't believe the long and difficult function I had to write before I heard of this simple API call.
	GetWindowThreadProcessId(slaveHwnd, &explorerPid);
	
	// Get a process handle which we need for the shared memory functions.
	proc = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION, FALSE, explorerPid);
	
	return proc;
}

QString pGetFileNameFromIcon(int clsid)
{
	QString name;

	LPITEMIDLIST pidl;
	if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, clsid, &pidl)))
	{
		SHFILEINFO fileInfo = {0};
		if (SUCCEEDED(SHGetFileInfo((LPCTSTR) pidl, 
			-1,
			&fileInfo, 
			sizeof(fileInfo), 
			SHGFI_PIDL | SHGFI_DISPLAYNAME)))
		{
			name = QString::fromUtf16((const ushort *) fileInfo.szDisplayName);
		}
		
		// Release the PIDL
		CoTaskMemFree(pidl);
	}

	return name;
}

QString WindowsSystem::GetFileNameFromIcon(int specialIcon)
{
	if (!_virtualIconTypeNameMap.contains(specialIcon))
		_virtualIconTypeNameMap.insert(specialIcon, pGetFileNameFromIcon(specialIcon));
	return _virtualIconTypeNameMap[specialIcon];
}

int WindowsSystem::GetIconTypeFromFileName(QString filename)
{
	if (_virtualIconNameTypeMap.empty())
	{
		_virtualIconNameTypeMap.insert(pGetFileNameFromIcon(ControlPanel), ControlPanel);
		_virtualIconNameTypeMap.insert(pGetFileNameFromIcon(InternetExplorer), InternetExplorer);
		_virtualIconNameTypeMap.insert(pGetFileNameFromIcon(DesktopDirectory), DesktopDirectory);
		_virtualIconNameTypeMap.insert(pGetFileNameFromIcon(MyComputer), MyComputer);
		_virtualIconNameTypeMap.insert(pGetFileNameFromIcon(MyDocuments), MyDocuments);
		_virtualIconNameTypeMap.insert(pGetFileNameFromIcon(NetworkNeighborhood), NetworkNeighborhood);
		_virtualIconNameTypeMap.insert(pGetFileNameFromIcon(RecycleBin), RecycleBin);
		_virtualIconNameTypeMap.insert(pGetFileNameFromIcon(Desktop), Desktop);
	}

	int type = -1;
	if (_virtualIconNameTypeMap.contains(filename))
		type = _virtualIconNameTypeMap[filename];
	return type;
}

void WindowsSystem::GetMousePosition(int &x, int &y)
{
	x = mouseManager->primaryTouchX;
	y = mouseManager->primaryTouchY;
	return;
}

int WindowsSystem::GetWindowWidth()
{
	RECT clientRect;
	GetClientRect(windowHwnd, &clientRect);
	// try and catch out of bounds cases?
	if ((clientRect.right - clientRect.left) > 10240)
		return 0;
	return clientRect.right - clientRect.left;
}

int WindowsSystem::GetWindowHeight()
{	
	RECT clientRect;
	GetClientRect(windowHwnd, &clientRect);
	// try and catch out of bounds cases?
	if ((clientRect.bottom - clientRect.top) > 10240)
		return 0;
	return clientRect.bottom - clientRect.top;
}

int WindowsSystem::GetMonitorWidth()
{
	MONITORINFOEX mi = monitors[GLOBAL(settings).monitor];
	return (mi.rcWork.right - mi.rcWork.left);
}

int WindowsSystem::GetMonitorHeight()
{
	MONITORINFOEX mi = monitors[GLOBAL(settings).monitor];
	return (mi.rcWork.bottom - mi.rcWork.top);
}

int WindowsSystem::GetWindowDPIX()
{
	return GetDeviceCaps(GetDC(winOS->GetWindowsHandle()), LOGPIXELSX);
}

int WindowsSystem::GetWindowDPIY()
{
	return GetDeviceCaps(GetDC(winOS->GetWindowsHandle()), LOGPIXELSY);
}

int WindowsSystem::GetWindowBpp()
{
	return GetDeviceCaps(GetDC(winOS->GetWindowsHandle()), BITSPIXEL);
}

void WindowsSystem::ExitBumpTop()
{
	exitFlag = true;
	evtManager->setExitFlag(true);
	evtManager->forceExitAfterTimeout(1500);
	SetConditionalFlag(ThreadSafeProcessing, true);
	DestroyBTWindow();
}

void WindowsSystem::AsyncExitBumpTop()
{
	exitFlag = true;
	evtManager->setAsyncExitFlag(true);
	evtManager->setExitFlag(true);
}

void WindowsSystem::SetWindowState(WindowState newWindowState)
{
	const LONG borderedWStyle = WS_CAPTION | WS_SYSMENU | WS_SIZEBOX | WS_MAXIMIZEBOX | WS_MINIMIZEBOX;
	const LONG borderedWExStyle = WS_EX_APPWINDOW;
	const LONG noBorderedWExStyle = WS_EX_TOOLWINDOW;

	LONG wStyle = GetWindowLong(windowHwnd, GWL_STYLE);
	LONG wExStyle = GetWindowLong(windowHwnd, GWL_EXSTYLE);

	if (windowState == ShellExt)
	{
		assert(false); // shell extension windows shouldn't be changing state
		return;
	}
	
	switch(newWindowState)
	{
	case ShellExt:
		assert(false); // unimplemented; no need for this
		break;
	case Windowed:
		if (windowState == WorkArea) 
		{
			SetParent(windowHwnd, NULL); // make top-level window
			winOS->desktopLock->unlock();
		}
		// enable the bordered window
		SetWindowLong(windowHwnd, GWL_STYLE, ((wStyle | borderedWStyle) & ~WS_POPUP) & ~WS_CHILD);
		SetWindowLong(windowHwnd, GWL_EXSTYLE, ((wExStyle | borderedWExStyle) & ~noBorderedWExStyle));
		SetWindowPos(windowHwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW); // flush the changes

		SetWindowPos(windowHwnd, NULL, 
			prevWindowedRect.left, prevWindowedRect.top,	// prevWindowedRect is set in the constructor
			prevWindowedRect.right - prevWindowedRect.left,	// to the default window size
			prevWindowedRect.bottom - prevWindowedRect.top, 
			SWP_NOZORDER);

		// if we're in "show desktop" mode, we won't see our window unless we activate it
		SetForegroundWindow(windowHwnd);		
		windowState = Windowed;
		break;
	case FullWindow:
		if (windowState == WorkArea) 
		{
			SetParent(windowHwnd, NULL); // make top-level window
			winOS->desktopLock->unlock();
		}
		// enable the bordered window
		SetWindowLong(windowHwnd, GWL_STYLE, ((wStyle | borderedWStyle) & ~WS_POPUP) & ~WS_CHILD);
		SetWindowLong(windowHwnd, GWL_EXSTYLE, ((wExStyle | borderedWExStyle) & ~noBorderedWExStyle));
		SetWindowPos(windowHwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW); // flush the changes

		ShowWindow(windowHwnd, SW_SHOWMAXIMIZED);

		// if we're in "show desktop" mode, we won't see our window unless we activate it
		SetForegroundWindow(windowHwnd);
		windowState = FullWindow;
		break;
	case FullScreen:
		// disable the bordered window
		if (windowState == WorkArea) 
		{
			SetParent(windowHwnd, NULL); // make top-level window
			winOS->desktopLock->unlock();
		}
		SetWindowLong(windowHwnd, GWL_STYLE, ((wStyle & ~borderedWStyle) & ~WS_CHILD) | WS_POPUP );
		SetWindowLong(windowHwnd, GWL_EXSTYLE, ((wExStyle | noBorderedWExStyle) & ~borderedWExStyle));
		SetWindowPos(windowHwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED); // flush the changes
		windowState = FullScreen;
		ResizeWindow();
		
		// if we're in "show desktop" mode, we won't see our window unless we activate it
		SetForegroundWindow(windowHwnd);

		break;
	case WorkArea:	
		{
			if (!winOS->desktopLock)
				winOS->desktopLock = new DesktopLock("desktop.test.lock");

			if (winOS->desktopLock->tryLock()) // make sure that we're the only instance of bumptop on the desktop
			{
				if (windowState == Windowed || windowState == FullScreen || windowState == WorkArea)
				{
					// disable the bordered window 
					SetWindowLong(windowHwnd, GWL_STYLE, (wStyle & ~borderedWStyle) | WS_CHILD | WS_POPUP);
					SetWindowLong(windowHwnd, GWL_EXSTYLE, (wExStyle & ~noBorderedWExStyle) & ~borderedWExStyle);
					SetWindowPos(windowHwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW); // flush the changes
					// make this a child of the desktop
					this->attachToDesktopWindow();
				}
				windowState = WorkArea;
				ResizeWindow();
			}
		}
		break;
	default:
		assert(false); // shouldn't reach this state
		break;
	}


}

void WindowsSystem::ToggleFullScreen()
{ 

	switch (windowState)
	{
	case Windowed:	
		SetWindowState(WorkArea);
		break;
	case FullWindow:
		SetWindowState(WorkArea);
		break;
	case WorkArea:
		skipFullScreenWindowState = true;
		if (skipFullScreenWindowState)
			SetWindowState(Windowed);
		else
			SetWindowState(FullScreen);
		
		break;
	case FullScreen:
		SetWindowState(Windowed);
		break;
	default:
		SetWindowState(WorkArea);
		break;
	};
	timerAnchor = timeGetTime();
}

void WindowsSystem::ToggleWindowMode()
{
	ToggleFullScreen();
}

// Bumptop is a child window of the Desktop, so this
// effectively shows BumpTop
void WindowsSystem::ToggleShowWindowsDesktop()
{	
	IShellDispatch4 *iDispatch;
	if (SUCCEEDED(CoCreateInstance(CLSID_Shell, NULL, CLSCTX_INPROC_SERVER, IID_IDispatch, (void**) &iDispatch)))
	{
		iDispatch->ToggleDesktop();
		iDispatch->Release();
	}
}

LPITEMIDLIST WindowsSystem::GetPidlFromName(int SystemLocation)
{
	LPITEMIDLIST TempPidl = NULL;

	// Retrieve the Desktop path
	SHGetSpecialFolderLocation(NULL, SystemLocation, &TempPidl);

	return TempPidl;
}

IShellFolder2 * WindowsSystem::GetShellFolderFromAbsDirPath( QString p )
{
	// Assumes given path is a directory

	LPITEMIDLIST pidl = NULL;
	IShellFolder2 * psfFolder = NULL;
	
	// namespace extension root (desktop) for parsing path
	LPSHELLFOLDER psfDesktop = NULL;
	if (FAILED(SHGetDesktopFolder(&psfDesktop)))
		return psfFolder;

	// parse path for absolute pidl, and connect to target folder
	if (FAILED(psfDesktop->ParseDisplayName(NULL, NULL, (LPWSTR) p.utf16(), NULL, &pidl, NULL)))
		return psfFolder;
	
	// now get the parent shell folder
	if (FAILED(psfDesktop->BindToObject(pidl, NULL, IID_IShellFolder2, (void**)&psfFolder)))
		return psfFolder;

	// cleanup
	CoTaskMemFree(pidl);
	psfDesktop->Release(); // no longer required

	return psfFolder;
}


LPITEMIDLIST WindowsSystem::GetAbsolutePidlFromAbsFilePath( QString p )
{
	return getAbsolutePidlFromAbsFilePath((LPWSTR) p.utf16());
}

LPITEMIDLIST WindowsSystem::GetRelativePidlFromAbsFilePath( QString p)
{
	LPITEMIDLIST pidl = NULL;
	LPITEMIDLIST pidlRelative = NULL;
	
	// namespace extension root (desktop) for parsing path
	LPSHELLFOLDER psfDesktop = NULL;
	if (FAILED(SHGetDesktopFolder(&psfDesktop)))
		return pidl;

	// parse path for absolute PIDL
	if (FAILED(psfDesktop->ParseDisplayName(NULL, NULL, (LPWSTR) p.utf16(), NULL, &pidl, NULL)))
		return pidl;

	// get the relative pidl, do not need to free pidlTemp
	pidlRelative = ILClone(ILFindLastID(pidl));

	// cleanup
	CoTaskMemFree(pidl);
	psfDesktop->Release();

	return pidlRelative;
}

void WindowsSystem::GetWorkArea(int &r, int &l, int &t, int &b)
{
	RECT workAreaRect;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &workAreaRect, 0);
	r = workAreaRect.right;
	l = workAreaRect.left;
	t = workAreaRect.top;
	b = workAreaRect.bottom;
}

void WindowsSystem::GetMainMonitorCoords(int &x, int &y)
{	
	// Return the buffer east and north of the primary monitor
	x = GLOBAL(windowXOffset);
	y = GLOBAL(windowYOffset);
}

BOOL CALLBACK WindowsSystem::EnumAndRefreshMonitors(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{	
	MONITORINFOEX mi;
	mi.cbSize = sizeof(MONITORINFOEX);

	// Get the size of the monitor
	GetMonitorInfo(hMonitor, &mi);

	// subtract the size of the monitor if its above the primary
	if (mi.rcMonitor.top < 0)
	{
		GLOBAL(windowYOffset) += mi.rcMonitor.top;
	}

	// Subtract the location of the monitor if its to the left
	if (mi.rcMonitor.left < 0)
	{
		GLOBAL(windowXOffset) += mi.rcMonitor.left;
	}

	QString name = QString::fromUtf16((const ushort *) mi.szDevice);
	name.replace("\\", "");
	name.replace(".", "");
	winOS->monitors[name] = mi;
	return TRUE;
}

bool WindowsSystem::GetConditionalFlag(int flag)
{
	return conditionalFlags & flag ? true : false;
}

void WindowsSystem::SetConditionalFlag(int flag, bool value)
{
	if (value)
	{
		// Turn this flag on
		if (!(GetConditionalFlag(flag)))
		{
			conditionalFlags |= flag;
		}
	}else{
		// Turn flag off
		if (GetConditionalFlag(flag))
		{
			conditionalFlags &= ~flag;
		}
	}
}

long GetFileSize(QString file)
{
	HANDLE hFile = ::CreateFile((LPCWSTR) file.utf16(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile)
	{
		// Get the file size
		LARGE_INTEGER fSize;
		fSize.LowPart = 0;
		GetFileSizeEx(hFile, &fSize);

		CloseHandle(hFile);
		return fSize.LowPart;
	}
	return 0;
}

// returns whether check for GDI renderer should be bypassed - for internal VM testing
bool WindowsSystem::ignoreGDI()
{
	return _ignoreGDI;
}

// sets whether check for GDI renderer should be bypassed - for internal VM testing
void WindowsSystem::ignoreGDI(bool value)
{
	_ignoreGDI = value;
}

// This function does an iterative event processing on all the file system changes. This is done because the File system
// events were being fire via callback, which would run a function while half way through another function. Events are
// non-blocking and therefore were being fired while in the middle of functions.
// This function checks the current SVN revision against the one that is included in this build
bool WindowsSystem::InstallUpdate()
{
	// skip this check if we are building for a hw manufacturers demo

	// Clear the contents of the version files
	QFileInfo versionFilePath = make_file(GetUpdateDirectory(), "version.txt");
	QFileInfo descFilePath = make_file(GetUpdateDirectory(), "desc.txt");
	
	// Find out the file size
	if (exists(versionFilePath) &&
		exists(descFilePath))
	{
		QString versionFileStr = read_file_utf8(native(versionFilePath));
		int svnRevision = atoi(SVN_VERSION_NUMBER);
		int updateRevision = versionFileStr.toInt();

		if (updateRevision > 0 && 
			updateRevision > svnRevision)
		{
			QString descriptionFileStr = read_file_utf8(native(descFilePath));
			QString msg = QString(WinOSStr->getString("NewBumpTopAvailable"))
				.arg(svnRevision).arg(updateRevision).arg(descriptionFileStr);

			SetConditionalFlag(ThreadSafeProcessing, true);
			//Use default message box in the case of < Vista, use the dialog manager otherwise
			int rc;
			if(!winOS->IsWindowsVersionGreaterThanOrEqualTo(WindowsVista))
				rc = MessageBox(GetForegroundWindow(), (LPCWSTR) msg.utf16(), (LPCTSTR) QT_TR_NOOP("A New BumpTop!").utf16(), MB_YESNO);
			else {
				dlgManager->setCaption(msg);
				if(dlgManager->promptDialog(DialogUpdateBumptop)) {
					rc = IDYES;
				} else {
					rc = IDNO;
				}
				dlgManager->clearState();
			}

			SetConditionalFlag(ThreadSafeProcessing, false);

			if (rc == IDYES)
			{
				/* NOTE: disable the screenshot mechanism for now
				// prompt to send the image
				if (statsManager->hasScreenshot())
				{
					dlgManager->setCaption("Send us a screenshot");
					dlgManager->setImage(statsManager->getScreenshot());
					dlgManager->setChecked(false);
					if (dlgManager->promptDialog(DialogUpdateBumptop) &&
						dlgManager->getChecked())
					{							
						dlgManager->clearState();
						dlgManager->setPrompt("Uploading Screenshot now.");
						dlgManager->promptDialog(DialogCaptionOnly);

						statsManager->uploadScreenshots();

						dlgManager->clearState();
					}
				}
				*/

				UpdateInstaller u(native(make_file(winOS->GetUpdateDirectory(), _updater->getDownloadedInstallerFile())));
				u.runInstaller();
				return true;
			}
			else 
			{
				if (statsManager->hasScreenshot())
					statsManager->clearScreenshots();
			}
		}
	}

	return false;
}

void WindowsSystem::checkForUpdate()
{
	if (_updater->isManualUpdateOnly())
	{
		if (_updaterThread)
		{
			_updaterThread->detach();
			SAFE_DELETE(_updaterThread);
		}
		_updaterThread = new boost::thread(boost::bind<void>(&Updater::run, _updater));
	}
	else
		_ASSERT(false); // If auto update, the Update item in system tray menu should be disabled and not call this
}

void WindowsSystem::GetLargeIconSupport(bool forceLowResOnVista)
{
	SHFILEINFO info = {0};
	HRESULT hr;
	HRESULT (WINAPI* pfnGetImageList)(int, REFIID, void**) = NULL;
	HMODULE hLib;
	GUID imageListID = {0x46EB5926, 0x582E, 0x4017, {0x9F, 0xDF, 0xE8, 0x99, 0x8D, 0xAA, 0x09, 0x50}};	

	// Load up Shell32.dll
	hLib = GetModuleHandle(L"shell32");

	if (hLib)
	{
		// Attempt to get the address of SHGetImageList
		(FARPROC&) pfnGetImageList = GetProcAddress(hLib, "SHGetImageList");

		if (!pfnGetImageList)
		{
			// If we wailed to aquire an address for SHGetImageList, try using Ordinal 727
			(FARPROC&) pfnGetImageList = GetProcAddress(hLib, MAKEINTRESOURCEA(727));  // see Q316931
		}

		if (pfnGetImageList)
		{
			// Get the 48x48 system image list. 
			// 0x4 == SHIL_JUMBO
			hr = pfnGetImageList(((IsWindowsVersionGreaterThanOrEqualTo(WindowsVista) && !forceLowResOnVista) ? 0x4 : 0x2), imageListID, (void **) &largeIcons);
			if (SUCCEEDED(hr))
			{
				largeIconSupport = true;
			}
		}
	}
}

QString WindowsSystem::GetUniqueID()
{
	return QString("bumptop://texture_id_%1").arg(uniqueIdCounter++);
}


QString WindowsSystem::GetBuildNumber()
{
	// Return the build number
	return QString(SVN_VERSION_NUMBER);
}

BumpTopEdition WindowsSystem::GetBumpTopEdition() { return _bumptopEdition = Standard; }

QString WindowsSystem::BumpTopEditionName( BumpTopEdition edition )
{
	if (edition == VIP)
		return QT_NT("VIP");
	else
		return QT_NT("Standard");
}

void WindowsSystem::SplashScreenIntro(bool startSplashScreen)
{
	if (startSplashScreen)
	{
		if (!_splashScreen)
		{
			QFileInfo splashImageFile = make_file(GetTexturesDirectory(), "splash.jpg");
			QPixmap splashScreenImage = QPixmap(native(splashImageFile));
			_splashScreen = new QSplashScreen(splashScreenImage);
			_splashScreen->show();
			LOG(native(splashImageFile));
		}
	}
	else
	{
		if (_splashScreen)
		{
			_splashScreen->hide();
			SAFE_DELETE(_splashScreen);
		}

		if (scnManager->isInSandboxMode)
			SetWindowState(Windowed);
		else if(!scnManager->isShellExtension) 
			SetWindowState(postSplashscreenWindowState);
	}
}

void WindowsSystem::SetDropPoint(const POINT & point, unsigned int expectedCount)
{
	dropHandler.SetLastDropPoint(point, expectedCount);
	numDroppedFilesFromLastDrag += expectedCount;
}

Vec3 WindowsSystem::GetDropPoint(int& numDroppedFilesRemainingOut, float yAxis)
{	
	bool forceCenter = true;
	if (numDroppedFilesFromLastDrag > 0)
	{
		forceCenter = false;
		--numDroppedFilesFromLastDrag;
		numDroppedFilesRemainingOut = numDroppedFilesFromLastDrag;
	}

	Vec3 v, w, pt, dir;
	Ray r;
	NxPlane floor = NxPlane(Vec3(0,0,0), Vec3(0,1,0));
	NxF32 dist;

	// Project down to the plane
	POINT dropPoint;
	dropHandler.GetLastDropPoint(&dropPoint);
	if (dropPoint.x < 0 || dropPoint.y < 0)
		forceCenter = true;
	window2world(((dropPoint.x == -1) || forceCenter) ? GetWindowWidth() / 2 : dropPoint.x, 
				 ((dropPoint.y == -1) || forceCenter) ? GetWindowHeight() / 2 : dropPoint.y, v, w);
	if (v.isFinite() && w.isFinite())
	{
		dir = w-v;
		dir.normalize();
		r = Ray(v, dir);
		NxRayPlaneIntersect(r, floor, dist, pt);

		// Point on the floor
		pt = Vec3(pt.x, yAxis, pt.z);

		Vec3List wallsPos = GLOBAL(WallsPos);
		if (!wallsPos.empty())
		{
			// Readjust for walls
			if (pt.x > wallsPos[3].x) pt.x = wallsPos[3].x - 20;
			if (pt.x < wallsPos[2].x) pt.x = wallsPos[2].x + 20;
			if (pt.z > wallsPos[0].z) pt.z = wallsPos[0].z - 20;
			if (pt.z < wallsPos[1].z) pt.z = wallsPos[1].z + 20;
		}
	}
	else
	{
		pt = Vec3(0, yAxis, 0);
	}

	return pt;
}

void WindowsSystem::SetLastDropPointFromDrag(unsigned int numDroppedFiles)
{
	numDroppedFilesFromLastDrag = numDroppedFiles;
}

void WindowsSystem::StartDragOnCurrentSelection()
{	
	if ((mouseManager->mouseButtons & MouseButtonLeft) &&
		sel->getPickedActor()) 
	{
		// Release the mouse capture
		ReleaseCapture();

		// get the current selection
		FileSystemActor * data = NULL;
		vector<BumpObject *> selectedObjs = sel->getBumpObjects();
		int expectedSize = 0;
		
		if (!selectedObjs.empty()) 
		{
			vector<QString> fileList;

			// handle plain actors
			for (int i = 0; i < selectedObjs.size(); ++i) 
			{
				if (selectedObjs[i]->getObjectType() == ObjectType(BumpPile, SoftPile)) 
				{
					Pile *p = (Pile *) selectedObjs[i];

					expectedSize += p->getNumItems();

					for (int j = 0; j < p->getNumItems(); ++j) 
					{
						data = GetFileSystemActor((*p)[j]);
						if (data && exists(data->getFullPath())) 
							fileList.push_back(data->getFullPath());
					}

				}else if (selectedObjs[i]->getObjectType() == ObjectType(BumpPile, HardPile)) 
				{
					// Move entire hard pile folder and not just its contents
					FileSystemPile *p = (FileSystemPile *) selectedObjs[i];
					fileList.push_back(p->getPileOwnerPath());

					++expectedSize;

				} else if (selectedObjs[i]->isBumpObjectType(BumpActor)) 
				{
					Actor * actor = ((Actor *) selectedObjs[i])->getActorToMimic();
					++expectedSize;

					// skip drives and logical volumes
					if (actor->isActorType(FileSystem) &&
						((FileSystemActor *)actor)->isFileSystemType(LogicalVolume))
						continue;
					// skip web actors (for now, in the future, we may want to copy the shortcut 
					// to the clipboard or something
					if (actor->isActorType(Webpage))
						continue;

					// handle actor
					if (actor->getObjectToMimic()) 
						data = (FileSystemActor *) actor->getObjectToMimic();					
					else if (actor->isActorType(FileSystem))
						data = GetFileSystemActor(selectedObjs[i]);
					
					if (data)
					{						
						if (data && exists(data->getFullPath())) 
							fileList.push_back(data->getFullPath());
					}
				}
			}

			// check if there are mixed parent paths. these are not supported at
			// the moment, so just show a dialog box
			bool hasMixedParentDirs = false;
			QDir expectedBranchPath;
			for (int i = 0; i < fileList.size(); ++i)
			{
				QFileInfo filePath(fileList[i]);
				if (!empty(expectedBranchPath) &&
					native(expectedBranchPath) != native(filePath.dir()))
				{
					MessageClearPolicy clearPolicy;
						clearPolicy.setTimeout(4);
					scnManager->messages()->addMessage(new Message("StartDragOnCurrentSelection", QT_TR_NOOP("We currently do not support dragging from two different sources.\n"
						"Try and drag items from the hard piles separately"), Message::Warning, clearPolicy));
					hasMixedParentDirs = true;
					break;
				}
				expectedBranchPath = filePath.dir();
			}

			bool initiatedDrag = false;
			DWORD resultEffect = 0;
			if (!fileList.empty() && !hasMixedParentDirs)
			{
				// initiate dragging allowing copy, move and link
				initiatedDrag = true;
				resultEffect = dragHandler.InitiateDrag(fileList, DropMove | DropCopy | DropLink);

				// notify the user if items in the pile were left behind
				if (resultEffect  && fileList.size() < expectedSize)
				{
					::MessageBox(GetForegroundWindow(), (LPCWSTR)QT_TR_NOOP("Could not copy all of the items in the pile").utf16(), (LPCWSTR)QT_TR_NOOP("Copy Error").utf16(), MB_OK);
				}
			}

			
			SHORT leftButtonState = GetSystemMetrics(SM_SWAPBUTTON) ? GetAsyncKeyState(VK_RBUTTON) : GetAsyncKeyState(VK_LBUTTON);

			
			// Deal with the results of the drag, if the item was dropped...
			if (initiatedDrag && 
				(resultEffect != DROPEFFECT_NONE || !(leftButtonState & 0x8000) )) 
				// if the result of the drag was nothing, but you let go as well, it counts as a drop
			{
				// reset the drag state of each object
				vector<BumpObject *>::iterator iter = GLOBAL(initialDragObjects).begin();
				while (iter != GLOBAL(initialDragObjects).end())
				{
					// end the drag
					BumpObject * obj = *iter;
					obj->onDragEnd();
					iter++;
				}
				
				// animate back to the initial position if failed
				// restore the previous locations if the drag does not remove the item from the desktop
				vector<BumpObject *> objList = sel->getBumpObjects();

				for (uint i = 0; i < objList.size(); i++)
				{
					if (objList[i]->getObjectType() == ObjectType(BumpActor, FileSystem))
					{
						FileSystemActor *fsData = (FileSystemActor *) objList[i];

						if (fsManager->isValidFileName(fsData->getFullPath()))
						{
							fsData->restoreStateFromBeforeDrag(fsData);
							sel->setPickedActor(NULL);
							sel->clear();
						}
					}
				}
				
				GLOBAL(initialDragObjects).clear();
			}
		}
	}
}

bool WindowsSystem::IsFullscreen() const
{
	// XXX: should just get window state
	return (windowState != Windowed && windowState != FullWindow);
}

bool IsValidKey(DWORD vkCode)
{
	// alpha numeric
	if (vkCode >= '0' && vkCode <= 'z')
		return true;

	// common keys
	if (vkCode >= KeySpace && vkCode <= KeyDown)
		return true;

	// numpad
	if (vkCode >= KeyNumpad0 && vkCode <= KeyNumpad9)
		return true;

	// 
	if (vkCode >= KeyPlus && vkCode <= KeyPeriod)
		return true;

	// whitespace
	switch (vkCode)
	{
	case KeyBackspace:
	case KeyEnter:
	case KeyInsert:
	case KeyDelete:

	case KeyShift:
	case KeyLeftShift:
	case KeyRightShift:
		return true;

	default:
		break;
	}

	// should we do F-keys outside of the ones bound to hotkeys?

	return false;
}

bool WindowsSystem::IsGrandfatherForegroundWindow()
{
	return GetForegroundWindow() == GetAncestor(GetAncestor(winOS->GetWindowsHandle(), GA_PARENT), GA_PARENT);
}


bool WindowsSystem::IsWindowTopMost()
{
	// workaround since top most window is not always a top-level window
	// we will check all the windows above bumptop and if we find another top-level
	// above ours, then we will bring bumptop to the front
	bool isTopMost = true;
	
	HWND windowHwnd = NULL;
	if (winOS->GetWindowState() == WorkArea)
	{
		windowHwnd = GetAncestor(GetAncestor(winOS->GetWindowsHandle(), GA_PARENT), GA_PARENT);
	}
	else
	{
		windowHwnd = winOS->GetWindowsHandle();
	}

	// NOTE: GetWindowRect sometimes pads the values it returns, so take 
	// the intersection of that rect with the working area of the monitor 
	// that we are currently on
	RECT btr;
	GetWindowRect(windowHwnd, &btr);
	HMONITOR btrMon = MonitorFromRect(&btr, MONITOR_DEFAULTTONEAREST);
	MONITORINFOEX mi;
	mi.cbSize = sizeof(MONITORINFOEX);
	GetMonitorInfo(btrMon, &mi);
	IntersectRect(&btr, &btr, &mi.rcWork);

	HWND prevWnd = GetNextWindow(windowHwnd, GW_HWNDPREV);
	while (prevWnd && isTopMost) 
	{
		// we allow top most floating windows (such as winamp or DUmeter) to sit above bumptop
		// so don't do the intersection test with these windows
		bool isTopMostWindow = (GetWindowLong(prevWnd, GWL_EXSTYLE) & WS_EX_TOPMOST);
		bool isWindowFramed = (GetWindowLong(prevWnd, GWL_STYLE) & WS_THICKFRAME);
		if (IsWindowVisible(prevWnd) && isWindowFramed && !isTopMostWindow) 
		{		
			TCHAR className[MAX_PATH];
			GetClassName(prevWnd, className, MAX_PATH);

			// the task bar will always be above ours, so don't do the intersection test
			// with the task bar window
			if (lstrcmpi(className, _T("Shell_TrayWnd")) != 0) 
			{			
				// otherwise, check if the window rects intersect
				RECT wr, finalIntersect;
				GetWindowRect(prevWnd, &wr);
				
				// ensure window intersections
				if (TRUE == IntersectRect(&finalIntersect, &btr, &wr)) {
					isTopMost = false;
				}
			}
		}
		prevWnd = GetNextWindow(prevWnd, GW_HWNDPREV);
	}

	return isTopMost;
}

void WindowsSystem::BringWindowToForeground()
{
	HWND btWindow = winOS->GetWindowsHandle();

	// workaround as SetForegroundWindow only works from the 
	// active process
	DWORD fgTid = GetWindowThreadProcessId(GetForegroundWindow(), 0);
	DWORD btTid = GetWindowThreadProcessId(btWindow, 0);
	if (btTid != fgTid)
		AttachThreadInput(btTid, fgTid, TRUE);
	
	SetWindowPos(btWindow, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	SetFocus(btWindow);

	// unattach 
	if (btTid != fgTid)
		AttachThreadInput(btTid, fgTid, FALSE);

	// Flyouts are not children of the BumpTop window, but we want them to
	// behave as if they are. In order to do so, we need to put them above
	// the BumpTop window when we bring it to the foreground.
	QWidgetList widgets = QApplication::topLevelWidgets();
	for (int i = 0; i < widgets.size(); i++)
	{
		Flyout *flyout = NULL;
		if (widgets[i]->isVisible() && (flyout = dynamic_cast<Flyout*>(widgets[i])))
			flyout->setWindowZOrder();
	}
}

void WindowsSystem::SetFocusOnWindow()
{
	HWND btWindow = winOS->GetWindowsHandle();

	// workaround as SetFocus only works from the 
	// active process
	DWORD fgTid   = GetWindowThreadProcessId(GetForegroundWindow(), 0);
	DWORD btTid = GetWindowThreadProcessId(btWindow, 0);
	if (btTid != fgTid)
		AttachThreadInput(btTid, fgTid, TRUE);

	SetFocus(btWindow);

	// unattach 
	if (btTid != fgTid)
		AttachThreadInput(btTid, fgTid, FALSE);
}

void WindowsSystem::ResizeWindow()
{
	RECT monRect;
	RECT workAreaRect;
	RECT clientRect;
	if (windowState == FullScreen || windowState == WorkArea || windowState == FullWindow) 
	{
		monRect = monitors[GLOBAL(settings).monitor].rcMonitor;
		workAreaRect = monitors[GLOBAL(settings).monitor].rcWork;

		GetWindowRect(windowHwnd, &clientRect);
	}

	switch (windowState)
	{
	case ShellExt:
		break;
	case FullScreen:
		if (clientRect.bottom != monRect.bottom ||
			clientRect.left != monRect.left ||
			clientRect.right != monRect.right ||
			clientRect.top != monRect.top)
		{
			SetWindowPos(windowHwnd, NULL, 
				monRect.left, 
				monRect.right, 
				monRect.right - monRect.left, 
				monRect.bottom - monRect.top, 
				SWP_NOZORDER | SWP_NOACTIVATE);
		}
		break;
	case WorkArea:
		if (clientRect.bottom != workAreaRect.bottom ||
			clientRect.left != workAreaRect.left ||
			clientRect.right != workAreaRect.right ||
			clientRect.top != workAreaRect.top)
		{
			winOS->SetAbsoluteWindowPos(NULL, 
				workAreaRect.left, 
				workAreaRect.top, 
				workAreaRect.right - workAreaRect.left, 
				workAreaRect.bottom - workAreaRect.top, 
				SWP_NOZORDER | SWP_NOACTIVATE);
		}
		break;
	case FullWindow:
		// nothing to do here
		break;
	case Windowed:
		// nothing to do here
		break;
	default:
		assert(false); // shouldn't reach this state
	}
}

void WindowsSystem::DestroyBTWindow()
{
	rndrManager->destroy();

	if (windowHwnd)
	{
		DestroyWindow(windowHwnd);
		windowHwnd = NULL;
	}

	if (topLevelWindowHwnd)
	{
		DestroyWindow(topLevelWindowHwnd);
		topLevelWindowHwnd = NULL;
	}

	LOG(QString_NT("Windows destroyed"));
}

bool WindowsSystem::CreateBTWindow()
{
	// create an alternate top level window which also listens for top level events
	topLevelWindowHwnd = CreateWindowEx(0,
			WINDOW_NAME, TOPLEVEL_WINDOW_NAME, WS_POPUP,
			0, 0, 16, 16,
			NULL, NULL, hInstance, NULL);

	// create the window
	if (scnManager->isShellExtension)
	{
		windowHwnd = CreateWindowEx(0,
			WINDOW_NAME, WINDOW_NAME, WS_CHILD | WS_VISIBLE,
			parentRect.left,
			parentRect.top,
			parentRect.right - parentRect.left,
			parentRect.bottom - parentRect.top,
			parentHwnd, NULL, hInstance, NULL);
		windowState = ShellExt;
	}
	else
	{
		QString windowTitle = QString::fromUtf16((const ushort *) WINDOW_NAME);
		if (scnManager->isBumpPhotoMode)
		{
			windowTitle = "BumpPhoto";
			windowState = FullWindow;
		}
		else
		{
			windowState = WorkArea;
		}

		// splashscreen window (will eventually turn into regular bumptop window)
		windowHwnd = CreateWindowEx((GLOBAL(settings).hideInTaskBar ? WS_EX_TOOLWINDOW : WS_EX_APPWINDOW),
			WINDOW_NAME, (LPCWSTR) windowTitle.utf16(), WS_SYSMENU | WS_POPUP | WS_CLIPCHILDREN,
			0,0,0,0,
			NULL, NULL, hInstance, NULL);

	}

	return windowHwnd != NULL ? true : false;
}

// If we're the child of the desktop and running on multiple monitors, our coordinate system is 
// now relative to the whole desktop area; 0,0 is no longer the upper right corner of the primary
// monitor. This function corrects for that.
bool WindowsSystem::SetAbsoluteWindowPos(HWND hWndInsertAfter,
								 int X,
								 int Y,
								 int cx,
								 int cy,
								 UINT uFlags)
{
	HWND parentHwnd = GetAncestor(windowHwnd, GA_PARENT);
	if (parentHwnd != NULL)
	{
		RECT parentRect;
		GetWindowRect(parentHwnd, &parentRect);
		X = X - parentRect.left;
		Y = Y - parentRect.top;
	}
	return SetWindowPos(windowHwnd, hWndInsertAfter, X, Y, cx, cy, uFlags) == TRUE ? true : false;
}

HWND WindowsSystem::GetShellExtProxyWindow()
{
	if (scnManager->isShellExtension)
	{
		return shellExtProxyHwnd;
	}
	return NULL;
}

void WindowsSystem::ShellExtBrowseToChild( QString filePath )
{
	if (!scnManager->isShellExtension)
		return;

	// NOTE: the window message keys passed into RegisterWindowMessage MUST match
	// those defined in ../../BtShExt/BtShExt/BTExplorerUtil.h
	uint browseToChildMsg = RegisterWindowMessage(L"BtShExt_browseChild");
	COPYDATASTRUCT cpData;
	cpData.dwData = browseToChildMsg;
	QByteArray utf8str = filePath.toUtf8();
	cpData.lpData = (LPVOID) utf8str.constData();
	cpData.cbData = utf8str.size() + 1;	// +1 for ending null char
	SendMessage(winOS->GetShellExtProxyWindow(), 
		WM_COPYDATA, 
		(WPARAM)(HWND) winOS->GetWindowsHandle(), 
		(LPARAM) (LPVOID) &cpData);
}

void WindowsSystem::ShellExtUpdateStatusBar()
{
	if (!scnManager->isShellExtension)
		return;

	// count the number of files and piles in the scene
	int fileCount = 0;
	int pileCount = scnManager->getPiles().size();
	vector<BumpObject *> objects = scnManager->getBumpObjects();
	for (int i = 0; i < objects.size(); ++i)
	{
		if (dynamic_cast<FileSystemActor *>(objects[i]))
		{
			++fileCount;
		}
	}
	int counts[2];
		counts[0] = fileCount;
		counts[1] = pileCount;
	
	// NOTE: the window message keys passed into RegisterWindowMessage MUST match
	// those defined in ../../BtShExt/BtShExt/BTExplorerUtil.h
	uint updateStatusBarMsg = RegisterWindowMessage(L"BtShExt_updateStatusBar");
	COPYDATASTRUCT cpData;
	cpData.dwData = updateStatusBarMsg;
	cpData.lpData = (LPVOID) counts;
	cpData.cbData = sizeof(int) * 2;
	SendMessage(winOS->GetShellExtProxyWindow(), 
		WM_COPYDATA, 
		(WPARAM)(HWND) winOS->GetWindowsHandle(), 
		(LPARAM) (LPVOID) &cpData);
}

bool WindowsSystem::RenameSoftPile(Pile * softPile)
{
	// ensure it's a soft pile
	if (!softPile->getPileType() == SoftPile)
		return false;

	QString text = softPile->getText();
	QString prompt;
	if (text.isEmpty())
		prompt = WinOSStr->getString("RenamePileTo").arg(QT_TR_NOOP("The Nameless Pile"));
	else
		prompt = WinOSStr->getString("RenamePileTo").arg(text);

	dlgManager->clearState();
	dlgManager->setCaption(QT_TR_NOOP("Rename Pile"));
	dlgManager->setText(text);
	dlgManager->setTextSelection(0, text.length());
	dlgManager->setPrompt(prompt);
	if (dlgManager->promptDialog(DialogInput))
	{
		QString newName = dlgManager->getText();
		softPile->setText(newName);
		textManager->forceUpdate();
	}

	return false;
}

bool WindowsSystem::RenameFile(FileSystemActor *actor, QString newName)
{
	
	// Check to see if the file is not VIRTUAL and then commence a rename
	if (!actor->isFileSystemType(Virtual))
	{
		
			
		// ensure a valid name was given
		if (isValidFilename(newName))
		{
			// valid name found, so go ahead and rename it
			if (actor->isFileSystemType(LogicalVolume))
			{
				SetVolumeLabel((LPCWSTR) actor->getFullPath().utf16(), (LPCWSTR) newName.utf16());
				actor->setText(newName);
				return true;
			}
			else
			{
				fsManager->renameFile(actor, newName, true);
				return true;
			}
			
		}
		else
		{
			// notify the user of the error, and reprompt
			printUnique("Key_RenameSelection", BtUtilStr->getString("ErrorRenameIllegalChar"));	
			return false;
		}			
		
	}

	return true;

}

WindowState WindowsSystem::GetWindowState() const
{
	return windowState;
}

HINSTANCE WindowsSystem::GetInstanceHandle() const
{
	return hInstance;
}

bool WindowsSystem::IsKeyDown(KeyboardValues key)
{
	return (GetAsyncKeyState(key) & 0x8000) > 0 ? true : false;
}

bool WindowsSystem::IsButtonDown(MouseButtons button)
{
	bool swapped = GetSystemMetrics(SM_SWAPBUTTON) > 0;
	int vbutton;
	switch (button)
	{
	case MouseButtonLeft:
		vbutton = swapped ? VK_RBUTTON : VK_LBUTTON;
		break;
	case MouseButtonRight:
		vbutton = swapped ? VK_LBUTTON : VK_RBUTTON;
		break;
	case MouseButtonMiddle:
		vbutton = VK_MBUTTON;
		break;
	default:
		return false;
	}
	return (GetAsyncKeyState(vbutton) & 0x8000) > 0 ? true : false;
}

DWORD WindowsSystem::getRegistryDwordValue(QString valueName, bool& valueExistsOut)
{
	return getRegistryDirectDwordValue(QString::fromUtf16((const ushort *) BT_REGISTRY_KEY), valueName, valueExistsOut);
}

DWORD WindowsSystem::getRegistryDirectDwordValue(QString path, QString valueName, bool& valueExistsOut)
{
	DWORD value = 0;
	DWORD dwValue;
	DWORD dwType = REG_DWORD;
	DWORD dwSize = sizeof(dwValue);
	HKEY hKey;
	valueExistsOut = false;
	LONG returnStatus = RegOpenKeyEx(HKEY_CURRENT_USER, (LPCWSTR) path.utf16(), 0L, KEY_READ, &hKey);
	if (returnStatus == ERROR_SUCCESS)
	{
		returnStatus = RegQueryValueEx(hKey, (LPCWSTR) valueName.utf16(), NULL, &dwType, (LPBYTE)&dwValue, &dwSize);
		if (returnStatus == ERROR_SUCCESS)
		{
			value = dwValue;
			valueExistsOut = true;
		}
	}
	RegCloseKey(hKey);
	return value;
}


DWORD WindowsSystem::getRegistryDwordValue(QString valueName)
{
	bool tmp = false;
	return getRegistryDwordValue(valueName, tmp);
}

void WindowsSystem::setRegistryDwordValue( QString valueName, DWORD value )
{
	HKEY hKey;
	// Creates the key if it doesn't exist. If it does, it simply opens it.
	LONG returnStatus = RegCreateKeyEx(HKEY_CURRENT_USER, BT_REGISTRY_KEY, 0L, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL);
	if (returnStatus == ERROR_SUCCESS)
	{
		returnStatus = RegSetValueEx(hKey, (LPCWSTR) valueName.utf16(), 0, REG_DWORD, (LPBYTE)&value, sizeof(value));
	}
	RegCloseKey(hKey);
}

QString WindowsSystem::getRegistryStringValue(QString valueName)
{
	QString value;
	TCHAR lszValue[MAX_PATH];
	HKEY hKey;
	DWORD dwType = REG_SZ;
	DWORD dwSize = MAX_PATH;
	LONG returnStatus = RegOpenKeyEx(HKEY_CURRENT_USER, BT_REGISTRY_KEY, 0L, KEY_READ, &hKey);
	if (returnStatus == ERROR_SUCCESS)
	{
		returnStatus = RegQueryValueEx(hKey, (LPCWSTR) valueName.utf16(), NULL, &dwType, (LPBYTE) &lszValue, &dwSize);
		if (returnStatus == ERROR_SUCCESS)
		{
			value = QString::fromUtf16((const ushort *) lszValue);
		}
	}
	RegCloseKey(hKey);
	return value;
}

void WindowsSystem::setRegistryStringValue(QString valueName, QString value)
{
	HKEY hKey;
	// Creates the key if it doesn't exist. If it does, it simply opens it.
	LONG returnStatus = RegCreateKeyEx(HKEY_CURRENT_USER, BT_REGISTRY_KEY, 0L, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL);
	if (returnStatus == ERROR_SUCCESS)
	{
		returnStatus = RegSetValueEx(hKey, (LPCWSTR) valueName.utf16(), 0, REG_SZ, (LPBYTE) value.utf16(), 2 * (value.size() + 1));
	}
	RegCloseKey(hKey);
}

QString WindowsSystem::GetGUID()
{
	// check the settings for the GUID if one exists
	if (GLOBAL(settings).guid.isEmpty())
	{
		GUID_LOG("guid is empty, generating a new one");
		QFileInfo settingsPath = make_file(winOS->GetDataDirectory(), "settings.json");
		GLOBAL(settings).guid = generateUniqueGUID();
		GLOBAL(settings).save(settingsPath);
		GUID_LOG("guid is now " + GLOBAL(settings).guid);
	}
	return GLOBAL(settings).guid;
}

void WindowsSystem::startAccumulatingInteractionTimer()
{
	// reset/start the timer
	if (!interactionTimeOutTimer)
		// first event
		accumulatedInterationTime.restart();
	else
		// another event
		KillTimer(windowHwnd, interactionTimeOutTimer);
	interactionTimeOutTimer = SetTimer(windowHwnd, INTERACTION_TIMERCALLBACK, 500, NULL);
}

void WindowsSystem::stopAccumulatingInteractionTimer()
{
	if (interactionTimeOutTimer)
	{
		KillTimer(windowHwnd, interactionTimeOutTimer);
		interactionTimeOutTimer = NULL;	
	}
	accumulatedInterationTime.restart();
}

unsigned int WindowsSystem::GetTime() const
{
	return timeGetTime();
}

void WindowsSystem::attachToDesktopWindow()
{
	if (IsWindowsVersionGreaterThanOrEqualTo(Windows7))
	{
		// EnumProc will handle attaching the bumptop window to the desktop
		::EnumWindows(EnumProc, (LPARAM)this);
	}
	else // All other versions of windows have the desktop under the Progman window
	{
		HWND hWnd = FindWindowEx(FindWindow(L"Progman", NULL), NULL, L"SHELLDLL_DefView", NULL);
		SetParent(windowHwnd, hWnd);
	}
}


// Static method that is called when executing EnumWindows
// This method is used to set the main BumpTop window as a child
// of the Desktop Window
BOOL CALLBACK WindowsSystem::EnumProc(HWND hwnd, LPARAM lp)
{
	HWND shellWin = FindWindowEx(hwnd, NULL, L"SHELLDLL_DefView", NULL);
	if (shellWin)
	{
		// Set the parent of the bumptop window to be the desktop
		SetParent(winOS->GetWindowsHandle(), shellWin);
		// Return false to stop enumerating windows
		return false;
	}
	// We didn't find the window so keep enumerating
	return true;
}

void WindowsSystem::detatchFromDesktopWindow()
{
	// get the window position
	RECT windowRect;
	GetWindowRect(windowHwnd, &windowRect);
	// detach from current parent
	SetParent(windowHwnd, NULL);
	// restore the window position after detaching
	SetWindowPos(windowHwnd, NULL, windowRect.left, windowRect.top, 0, 0, 
		SWP_NOSIZE | SWP_NOZORDER | SWP_NOSENDCHANGING | SWP_NOACTIVATE);
} 

bool WindowsSystem::isAttachedToDesktopWindow()
{
	// If BumpTop is attached to the desktop window, this means it is the grandchild
	// of the ProgMan in the window hierarchy. There is another possibility-- when you
	// hit Start-D or "Show Desktop", windows moves the desktop to be the child of 
	// some window with classname "WorkerW." If Bumptop's grandfather has either the name
	// ProgMan or WorkerW, then we are successfully attached as the desktop window
	HWND grandfather = GetAncestor(GetAncestor(winOS->GetWindowsHandle(), GA_PARENT), GA_PARENT);

	TCHAR buffer[MAX_PATH+1];
	GetClassName(grandfather, buffer, MAX_PATH);

	QString bufferStr = QString::fromUtf16((const ushort *) buffer);
	bool retValue = bufferStr == "Progman" || 
					bufferStr == "WorkerW";

	return retValue;

}

bool WindowsSystem::IsWindowsVersion(WindowsOSVersion version)
{
	// Object used to store OS version information
	OSVERSIONINFO osvi = GetOSVersionInfo();

	// 5 - 2k, xp, 2003
	// 6 - vista
	// 6.1 - win7 beta
	switch (version)
	{
	case Windows7:
		return osvi.dwMajorVersion == 6 && osvi.dwMinorVersion > 0;
	case WindowsVista:
		return osvi.dwMajorVersion == 6;
	case WindowsXP:
		return osvi.dwMajorVersion <= 5;
	default:
		return false;
	}
}

bool WindowsSystem::IsWindowsVersionGreaterThanOrEqualTo(WindowsOSVersion version)
{
	// Object used to store OS version information
	OSVERSIONINFO osvi = GetOSVersionInfo();

	// 5 - 2k, xp, 2003
	// 6 - vista
	// 6.1 - win7 beta
	switch (version)
	{
	case Windows7:
		return osvi.dwMajorVersion >= 6 && osvi.dwMinorVersion > 0;
	case WindowsVista:
		return osvi.dwMajorVersion >= 6;
	case WindowsXP:
		return osvi.dwMajorVersion >= 5;
	default:
		return false;
	}
}

OSVERSIONINFO WindowsSystem::GetOSVersionInfo()
{
	// Object used to store OS version information
	OSVERSIONINFO osvi;
	// Allocate memory for osvi
	ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	// Get the OS Version information
	GetVersionEx(&osvi);
	return osvi;
}

HBITMAP WindowsSystem::ExtractThumbnail_Vista(QString texturePath, IThumbnailCache ** ptcOut, IShellItem ** psiOut, ISharedBitmap ** psbOut)
{
	// ensure valid outptrs
	if (!ptcOut) return NULL; else (*ptcOut) = NULL;
	if (!psiOut) return NULL; else (*psiOut) = NULL;
	if (!psbOut) return NULL; else (*psbOut) = NULL;

	// try and create an instance of the IThumbnailCache interface
	HRESULT hr;
	HBITMAP hbitmap;
	bool bitmapExtracted = false;	
	hr = CoCreateInstance(CLSID_LocalThumbnailCache, NULL, CLSCTX_INPROC_SERVER, IID_IThumbnailCache, (LPVOID *) &(*ptcOut));

	if (SUCCEEDED(hr))
	{
		// try and get the IShellItem interface for the specified item
		HRESULT (WINAPI* pfnSHCreateItemFromParsingName)(PCWSTR, IBindCtx *, REFIID, void **) = NULL;
		HMODULE hLib = GetModuleHandle(L"shell32");

		if (hLib)
		{
			(FARPROC&) pfnSHCreateItemFromParsingName = GetProcAddress(hLib, "SHCreateItemFromParsingName");

			if (pfnSHCreateItemFromParsingName)
			{

				GUID shellItemID = {0x7e9fb0d3, 0x919f, 0x4307, {0xab, 0x2e, 0x9b,0x18, 0x60, 0x31, 0x0c, 0x93}};
				hr = pfnSHCreateItemFromParsingName((LPCWSTR) texturePath.utf16(), NULL, shellItemID, (void **) &(*psiOut));

				if (SUCCEEDED(hr))
				{
					// try and extract the thumbnail from the item
					WTS_CACHEFLAGS cacheFlags;
					WTS_THUMBNAILID thumbnailId;

					// NOTE: using WTS_EXTRACT: Explorer extracts the thumbnail if it is not cached
					hr = (*ptcOut)->GetThumbnail((*psiOut), 256, WTS_EXTRACT, 
						&(*psbOut), &cacheFlags, &thumbnailId);

					if (SUCCEEDED(hr))
					{
						// try and extract the HBITMAP from the ISharedBitmap and load it into open gl
						if (SUCCEEDED((*psbOut)->GetSharedBitmap(&hbitmap)))
						{
							return hbitmap;
						}
						SAFE_RELEASE(*psbOut);
					}
					SAFE_RELEASE(*psiOut);
				}

			}
		}
		SAFE_RELEASE(*ptcOut);
	}
	return NULL;
}

void WindowsSystem::RelaunchBumpTop()
{	
	// get the location of bumptop to relaunch
	QString appPath = winOS->GetExecutablePath().absoluteFilePath();
	
	setRegistryDwordValue("RelaunchBumpTopWindow", 1);
	setRegistryDwordValue("ShutdownIncomplete", 1);	

	// Unlock the desktop lock so the new instance of BumpTop can 
	// load up without deferring to this dying instance.
	if (winOS->desktopLock && winOS->desktopLock->isLocked())
		winOS->desktopLock->unlock();

	// relaunch bumptop after exiting
	fsManager->launchFile(appPath);
}



bool WindowsSystem::isTopMostDesktopWindowChild()
{
	return isAttachedToDesktopWindow() && GetNextWindow(GetWindowsHandle(), GW_HWNDPREV) == NULL;
}

void WindowsSystem::makeTopWindow()
{
	SetWindowPos(winOS->GetWindowsHandle(), HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}

bool WindowsSystem::Is64BitWindows()
{
	QString value;
	TCHAR lszValue[MAX_PATH];
	HKEY hKey;
	DWORD dwType = REG_SZ;
	DWORD dwSize = MAX_PATH;

	LONG returnStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0L, KEY_READ, &hKey);
	if (returnStatus == ERROR_SUCCESS)
	{
		returnStatus = RegQueryValueEx(hKey, L"Identifier", NULL, &dwType, (LPBYTE)&lszValue, &dwSize);
		if (returnStatus == ERROR_SUCCESS)
		{
			value = QString::fromUtf16((const ushort *) lszValue);
		}
	}
	RegCloseKey(hKey);
	return !value.isEmpty() && !value.contains("x86");
}

void WindowsSystem::MoveToNextMonitor()
{
	if (monitors.size() == 1)
		return;

	QHash<QString, MONITORINFOEX>::iterator iter = monitors.begin();
	while (iter != monitors.end())
	{
		QString name = iter.key();
		iter++;
		if (name == GLOBAL(settings).monitor)
		{	
			if (iter == monitors.end())
				GLOBAL(settings).monitor = (monitors.begin()).key();		// loop around to first item
			else
				GLOBAL(settings).monitor = iter.key();					// get the "next" item
			break;
		}
	}

	windowState = WorkArea;
	winOS->detatchFromDesktopWindow();
	winOS->ResizeWindow();
	winOS->attachToDesktopWindow();
	SetFocus(winOS->GetWindowsHandle());
	
	SaveSettingsFile();

	printUnique("MoveToNextMonitor", QString(WinOSStr->getString("MoveToMonitor")).arg(GLOBAL(settings).monitor));
}

bool WindowsSystem::IsFileInUse(QString file)
{	
	HANDLE hTestFile = CreateFile((LPCTSTR) file.utf16(), GENERIC_READ, 
		FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	bool inUse = (hTestFile == INVALID_HANDLE_VALUE);
	CloseHandle(hTestFile);
	return inUse;
}

QFileInfo WindowsSystem::CreateDxDiagLog()
{
	// try and send the user's dxdiag info
	QString dxDiag = native(make_file(winOS->GetSystemDirectory(), "dxdiag.exe"));
	QString dxDiagOut = native(make_file(winOS->GetDataDirectory(), "dxdiag.txt"));
	QString dxArguments = "/t " + dxDiagOut;

	QFileInfo dxDiagOutPath(dxDiagOut);

	STARTUPINFO         si;
	PROCESS_INFORMATION pi;

	ZeroMemory (&si, sizeof(si));
	si.cb=sizeof (si);
	BOOL success = CreateProcess((LPCWSTR) dxDiag.utf16(),
		(LPWSTR) dxArguments.utf16(),
		NULL,
		NULL,
		FALSE,
		CREATE_NO_WINDOW,
		NULL,
		NULL,
		&si,
		&pi);
	WaitForSingleObject(pi.hProcess, INFINITE);

	return QFileInfo(native(dxDiagOutPath));
}


vector<QString> WindowsSystem::GetGraphicsCardIDs()
{
	QFileInfo dxDiagFile = winOS->CreateDxDiagLog();
	QString params;
	vector<QString> videoCardHardwareID;

	if (exists(dxDiagFile))
	{
		QString dxDiag = read_file_utf8(native(dxDiagFile));
		

		// vendor id
		QRegExp filter("Vendor ID: ([^\n\r]*)");
		filter.setCaseSensitivity(Qt::CaseInsensitive);
		if (filter.indexIn(dxDiag) > -1)
			videoCardHardwareID.push_back(filter.cap(1).trimmed());

		// device id
		filter.setPattern("Device ID: ([^\n\r]*)");
		filter.setCaseSensitivity(Qt::CaseInsensitive);
		if (filter.indexIn(dxDiag) > -1)
			videoCardHardwareID.push_back(filter.cap(1).trimmed());

		// subsys id
		filter.setPattern("SubSys ID: ([^\n\r]*)");
		filter.setCaseSensitivity(Qt::CaseInsensitive);
		if (filter.indexIn(dxDiag) > -1)
			videoCardHardwareID.push_back(filter.cap(1).trimmed());

		// revision id
		filter.setPattern("Revision ID: ([^\n\r]*)");
		filter.setCaseSensitivity(Qt::CaseInsensitive);
		if (filter.indexIn(dxDiag) > -1)
			videoCardHardwareID.push_back(filter.cap(1).trimmed());
	}


	return videoCardHardwareID;
}

void WindowsSystem::FailOnDrivers()
{
	// If we plan on skipping animations or rendering don't bother failing on drivers
	if (scnManager->skipAnimations || scnManager->disableRendering)
		return;

	winOS->ExitBumpTop();
	ExitBumptop();
	dlgManager->clearState();
	if (dlgManager->promptDialog(DialogUpdateGraphicsDrivers))
	{
		QUrl driversUrl("http://bumptop.com/drivers#video_cards");
		QFileInfo dxDiagFile = winOS->CreateDxDiagLog();
		QString params;
		vector<QString> videoHardwareID = WindowsSystem::GetGraphicsCardIDs();
		QString hardwareID = "";
		for(int x = 0; x < videoHardwareID.size()-1; x++) { //-1 because we do not want the revision number, which is the last element in the vector
			hardwareID += videoHardwareID.at(x).mid(2,-1); //Chop off the first two characters (0x????) -> (????)
		}
		if (exists(dxDiagFile))
		{
			QString dxDiag = read_file_utf8(native(dxDiagFile));

			// os
			QRegExp filter("Operating System:([^\n\r]*)");
			filter.setCaseSensitivity(Qt::CaseInsensitive);
			if (filter.indexIn(dxDiag) > -1)
				driversUrl.addQueryItem("os", filter.cap(1).trimmed());

			// card type
			filter.setPattern("Card name:([^\n\r]*)");
			if (filter.indexIn(dxDiag) > -1)
				driversUrl.addQueryItem("card", filter.cap(1).trimmed());

			// manufacturer
			filter.setPattern("Manufacturer:([^\n\r]*)");
			if (filter.indexIn(dxDiag) > -1)
				driversUrl.addQueryItem("manufacturer", filter.cap(1).trimmed());

			// driver version
			filter.setPattern("Driver Version:([^\n\r]*)");
			if (filter.indexIn(dxDiag) > -1)
				driversUrl.addQueryItem("driver_version", filter.cap(1).trimmed());

			// driver hardware ID
			driversUrl.addQueryItem("video_hardware_id", hardwareID);
		}	

		fsManager->launchFile(driversUrl.toString());
	}
	exit(0);
}

void WindowsSystem::FailOnBpp()
{
	winOS->ExitBumpTop();
	ExitBumptop();
	{
		::MessageBox(winOS->GetWindowsHandle(), (LPCWSTR)QT_TR_NOOP("Please set your monitor color quality to 'Highest (32 bit)'"
			"in\nyour Display Properties before running BumpTop.").utf16(), (LPCWSTR)QT_TR_NOOP("Invalid Color Depth!").utf16(), MB_OK | MB_ICONERROR);
	}	
	exit(0);
}

SettingsAppMessageHandler* WindowsSystem::GetSettingsAppMessageHandler()
{
	return _settingsAppHandler;
}

bool WindowsSystem::QueryRecycleBin()
{
	OleInitialize(NULL);
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	if (winOS->GetSystemPath(DesktopDirectory) == native(scnManager->getWorkingDirectory()))
	{		
		HKEY recycleBinKey;
		TCHAR defaultIconPath[MAX_PATH];
		TCHAR fullIconPath[MAX_PATH];
		DWORD dwType = REG_SZ;
		DWORD dwSize = MAX_PATH;

		LONG returnStatus = RegOpenKeyEx(HKEY_CURRENT_USER, QT_NT(L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\CLSID\\{645FF040-5081-101B-9F08-00AA002F954E}\\DefaultIcon"), 0, KEY_QUERY_VALUE, &recycleBinKey);

		if (returnStatus != ERROR_SUCCESS)
			return false;
		
		while (_recyclerThread)
		{
			_recyclerThread->markAsSafeToJoin(false);
	
			returnStatus = RegQueryValueEx(recycleBinKey, NULL, NULL, &dwType, (LPBYTE) &defaultIconPath, &dwSize) == ERROR_SUCCESS &&
						   RegQueryValueEx(recycleBinKey, QT_NT(L"full"), NULL, &dwType, (LPBYTE) &fullIconPath, &dwSize) == ERROR_SUCCESS;
			
			if (_memberMutex.tryLock() && returnStatus)
			{
				bool isRecycleBinFull = _isRecycleBinFull;
				if (_tcscmp(defaultIconPath, fullIconPath) == 0)
					_isRecycleBinFull = true;
				else
					_isRecycleBinFull = false;
				
				_recycleBinStateChanged = (_isRecycleBinFull != isRecycleBinFull) ? true : false;
				_memberMutex.unlock();
			}
			
			_recyclerThread->markAsSafeToJoin(true);
			Sleep(5000);
		}
		RegCloseKey(recycleBinKey);
	}

	CoUninitialize();
	OleUninitialize();

	return true;
}

void WindowsSystem::onPowerResume()
{
	if (_recyclerThread)
		_recyclerThread->reRun();
}

void WindowsSystem::onPowerSuspend()
{
	if (_recyclerThread)
		_recyclerThread->join(1000);
}

QString WindowsSystem::GetLocaleLanguage()
{
	if (GLOBAL(settings).languageOverride.isEmpty())
	{
		// XXX: check out GetUserDefaultUILanguage()
		WORD id = PRIMARYLANGID(GetUserDefaultUILanguage());
		TCHAR langBuffer[128] = {0};
		GetLocaleInfo(id, LOCALE_SISO639LANGNAME, langBuffer, sizeof(langBuffer));
		TCHAR countryBuffer[128] = {0};
		GetLocaleInfo(id, LOCALE_SISO3166CTRYNAME, countryBuffer, sizeof(countryBuffer));
		QString code = QString(QT_NT("%1_%2"))
			.arg(QString::fromUtf16((const ushort *) langBuffer))
			.arg(QString::fromUtf16((const ushort *) countryBuffer))
			.toLower();
		// map to simplified chinese for now if one of the other chinese variations
		if (code.startsWith("zh"))
		{
			if (code != "zh_cn")
				code = QT_NT("zh_tw");
		}
		else
		{
			// trim the code
			if (code.contains("_"))
				code = code.mid(0, code.indexOf("_"));
		}
		return code;
	}
	else
		return GLOBAL(settings).languageOverride;
}

Windows7Multitouch *WindowsSystem::GetWindows7Multitouch()
{
	return windows7Multitouch;
}

LibraryManager* WindowsSystem::GetLibraryManager()
{
	return libraryManager;
}

bool WindowsSystem::GetShortPathName( QString fullPathName, QString &shortPathName )
{
	TCHAR shortFilePath[MAX_PATH + 1];
	// GetShortPathName will return 0 if it fails
	int success = ::GetShortPathName((LPCWSTR) fullPathName.utf16(), (LPWSTR) &shortFilePath, MAX_PATH + 1);	
	if (success)
	{
		shortPathName = QString::fromUtf16((const ushort *) shortFilePath);
		return true;
	}
	return false;
}

// Returns the absolute path of the HTML source for the shared folder widget.
QString WindowsSystem::GetSharingWidgetPath()
{
	QDir widgetDir = parent(winOS->GetTestsDirectory()) / "Sharing";
	return widgetDir.absoluteFilePath(QT_NT("sharedFolder.html"));
}

WindowsOSStrings::WindowsOSStrings()
{
	_strs.insert("BumpTopErrorSafe", QT_TR_NOOP("BumpTop experienced an error.\n\nPlease help us fix this issue by describing what you were doing before the crash and if possible, how to reproduce it."));
	_strs.insert("BumpTopError", QT_TR_NOOP("Boom! BumpTop experienced an error.\n\nPlease help us fix this issue by describing what you were doing before the crash and if possible, how to reproduce it.  Thanks!"));
	_strs.insert("ErrorConnectBumpHQ", QT_TR_NOOP("We couldn't connect to Bump HQ.\nTo manually report this bug please email us at feedback@bumptop.com and attach the following file:\n%1\n\nThanks you for your help. Bump On!"));
	_strs.insert("ErrorInfoSent", QT_TR_NOOP("Thanks for your help, the error information was sent.\n Bump On!"));
	_strs.insert("NewBumpTopAvailable", QT_TR_NOOP("A new version of BumpTop is available! (Yours: %1, Newest: %2)\n\n%3\n\nWe've automatically downloaded the update for you, would you like to update now?"));
	_strs.insert("RenameTo", QT_TR_NOOP("Rename file (%1) to:"));
	_strs.insert("RenamePileTo", QT_TR_NOOP("Rename pile (%1) to:"));
	_strs.insert("RenameToAgain", QT_TR_NOOP("Rename file to (Characters including %1 are not allowed):"));
	_strs.insert("MoveToMonitor", QT_TR_NOOP("Moving to monitor '%1'"));
	_strs.insert("ExistingUserPrompt", QT_TR_NOOP("As a Thank You for beta testing you get a discount on BumpTop Pro, only $19.  Otherwise on May 1, 2009 you'll be switched to BumpTop Free."));
	_strs.insert("ExistingUserProButton", QT_TR_NOOP("Get Pro Discount"));
	_strs.insert("ExistingUserFreeButton", QT_TR_NOOP("Decide Later"));
	_strs.insert("NewUserPrompt", QT_TR_NOOP("What version of BumpTop would you like to use?"));
	_strs.insert("NewUserProButton", QT_TR_NOOP("BumpTop Pro"));
	_strs.insert("NewUserFreeButton", QT_TR_NOOP("BumpTop Free"));
	_strs.insert("AuthFlickrUser", QT_TR_NOOP("Did you allow BumpTop to access your photos?"));
}

QString WindowsOSStrings::getString( QString key )
{
	QHash<QString, QString>::const_iterator iter = _strs.find(key);
	if (iter != _strs.end())
		return iter.value();
	assert(false);
	return QString();
}
