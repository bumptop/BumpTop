/*******************************************************************
//		*** COPYRIGHT NOTICE ****
//
//	Copyright 2001. Wes Jones (wesj@hotmail.com)
//
//	This code is free for use under the following conditions:
//
//		You may not claim authorship of this code.
//		You may not sell or distrubute this code without author's permission.
//		*Author's permission has been obtained to distribute this code with BumpTop's source code.
//		You are not permitted to sell this code in it's compiled, non-compiled, executable, or any other form. 
//		Executable code excepted in the case that it is not sold separately from an application which 
//		uses this it. This means you can use this code for your applications as you see fit, but this code may not be sold in any form
//		to others for use in their applications.
//		This copyright notice may not be removed.
//		
//
//		If this code was NOT obtained by downloading it from www.codeproject.com,
//		or given to you by a friend or coworker, please tell me, & let me know how you got it. There a plenty of lazy bastards that 
//		collect source code from the internet, and then sell it as part of 
//		a 'Programmer's Library'.  Using this code for such a purpose is stricly prohibited.
//
//		If you'd like to pay me to turn this into an ActiveX/COM object so you 
//		can use it in a Visual Basic application, feel free to contact me with an offer,
//		and I will create it for you. Otherwise, here is the source code, and you may make your own
//		ActiveX/COM object, providing that it is not sold separately.
//
//  No guarantees or warranties are expressed or implied. 
//	This code may contain bugs.
//	Warning: May contain matter. If this should come into contact with anti-matter, a violent explosion may occur.
*******************************************************************/

// DelayedDirectoryChangeHandler.h: interface for the CDelayedDirectoryChangeHandler2 class.
//
//////////////////////////////////////////////////////////////////////
//
//	You needn't worry about the classes in this file.
//	they are implementation classes used to help CDirectoryChangeWatcher work.
//
//

#pragma once

#ifndef _BT_DIRECTORY_CHANGE_HANDLER_
#define _BT_DIRECTORY_CHANGE_HANDLER_

// -----------------------------------------------------------------------------

#include "DirectoryChanges.h"

// -----------------------------------------------------------------------------

class CDirectoryChangeWatcher;
class CDirectoryChangeHandler;
class CDirChangeNotification;
class CDelayedDirectoryChangeHandler;
class CDelayedNotificationWindow;
class CDelayedNotificationThread;

// -----------------------------------------------------------------------------

typedef BOOL (STDAPICALLTYPE * FUNC_PatternMatchSpec)(LPCTSTR pszFile, LPCTSTR pszSpec);

/*******************************************************************
	The classes in this file implement methods to ensure that file change 
	notifications are fired in a thread other than the worker thread used
	by CDirectoryChangeWatcher.

	Dispatching the notifications in to a different thread improves the performance
	of CDirectoryChangeWatcher so that it can process more notifications faster
	and notifications aren't 'lost'.


	There are two methods of dispatching functions to another thread.

		1)  One is to use the message pump associated w/ the main thread by posting notifications
			to a hidden window. This is implemented w/ the class CDelayedNotificationWindow.

		2)	The other is to create a worker thread that implements a message pump.  This is 
			implemented w/ the class CDelayedNotificationThread.


	If your app uses a GUI then it has a already has message pump.  
	You can make sure that CDelayedNotificationWindow is used in this case.
	The advantage here being that there is one less worker thread used in your program.

	If your app is a command line app or otherwise doesn't have a GUI,
	then you will want to make sure that you are using the CDelayedNotificationThread
	to dispatch notifications to another thread.

	This is determined by a flag passed is passed to the constructor of CDirecotryChangeWatcher

********************************************************************/

class CDelayedNotifier
//
//	Abstract base class for ensuring notifications are fired in a thread 
//
//
{
public:
	virtual ~CDelayedNotifier(){}
	virtual void PostNotification(CDirChangeNotification * pNotification) = 0;

};

class CDelayedNotificationWindow : public CDelayedNotifier
//
//	A class that implements a
//	there will always be only one of the actual windows 
//	in existance. 
//
{
public:
		CDelayedNotificationWindow(){  AddRef(); }
		virtual ~CDelayedNotificationWindow(){ Release(); }
		

		void PostNotification(CDirChangeNotification * pNotification);
private:
		long AddRef();		//	the window handle is reference counted
		long Release();		//

		static long s_RefCnt;
		static HWND s_hWnd; //there's only one window no matter how many instances of this class there are.... this means that all notifications are handled by the same thread.
		static BOOL s_bRegisterWindow;

		BOOL RegisterWindowClass(LPCTSTR szClassName);
		BOOL CreateNotificationWindow();
};

class CDelayedNotificationThread : public CDelayedNotifier
//
//	Class that implements a worker thread w/ a message pump.
//	CDirectoryChangeWatcher posts notifications to this thread, where they are dispatched.
//	This thread executes CDirectoryChangeHandler notifications.
//
{
public:
	CDelayedNotificationThread()
		:m_hThreadStartEvent(NULL)
	{ 
		m_hThreadStartEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
		ASSERT(m_hThreadStartEvent);
		AddRef(); 
	}
	virtual ~CDelayedNotificationThread()
	{ 
		Release(); 
		if(m_hThreadStartEvent) 
			CloseHandle(m_hThreadStartEvent), m_hThreadStartEvent = NULL;
	}

	void PostNotification(CDirChangeNotification * pNotification);

private:
	long AddRef();					// The thread handle is reference
	long Release();					// counted so that only one thread is used
									// so that there's only one worker thread(performing this functino)
									// no matter how many directories are being watched
	static long		s_RefCnt;		//
	static HANDLE	s_hThread;		//	
	static DWORD	s_dwThreadID;	//  
										
	static UINT __stdcall ThreadFunc(LPVOID lpvThis);

	bool StartThread();
	bool StopThread();

	BOOL WaitForThreadStartup(){ return WaitForSingleObject(m_hThreadStartEvent, INFINITE) == WAIT_OBJECT_0; };
	BOOL SignalThreadStartup(){ return SetEvent(m_hThreadStartEvent) ; }

	HANDLE m_hThreadStartEvent;//signals that the worker thread has started. this fixes a bug condition.
		
};

class CDirChangeNotification
//
//	 A class to help dispatch the change notifications to the main thread.
//
//	 This class holds the data in memory until the notification can be dispatched.(ie: this is the time between when the notification is posted, and the clients notification code is called).
//
//
{
private:
	CDirChangeNotification();//not implemented
public:
	explicit CDirChangeNotification(CDelayedDirectoryChangeHandler * pDelayedHandler, DWORD dwPartialPathOffset);
	~CDirChangeNotification();

	//
	//
	void PostonFileAdded(LPCTSTR szFileName);
	void PostonFileRemoved(LPCTSTR szFileName);
	void PostonFileNameChanged(LPCTSTR szOldName, LPCTSTR szNewName);
	void PostonFileModified(LPCTSTR szFileName);
	void PostonReadDirectoryChangesError(DWORD dwError, LPCTSTR szDirectoryName);
	void PostonWatchStarted(DWORD dwError, LPCTSTR szDirectoryName);
	void PostonWatchStopped(LPCTSTR szDirectoryName);

	void DispatchNotificationFunction();


	enum eFunctionToDispatch{	eFunctionNotDefined = -1,
								eonFileAdded		= FILE_ACTION_ADDED, 
								eonFileRemoved		= FILE_ACTION_REMOVED, 
								eonFileModified	= FILE_ACTION_MODIFIED,
								eonFileNameChanged	= FILE_ACTION_RENAMED_OLD_NAME,
								eonReadDirectoryChangesError,
								eonWatchStarted,
								eonWatchStopped
	};	
protected:
	void PostNotification();
	
private:
	friend class CDelayedDirectoryChangeHandler;
	CDelayedDirectoryChangeHandler * m_pDelayedHandler;

	//
	//	Members to help implement DispatchNotificationFunction
	//
	//

	eFunctionToDispatch m_eFunctionToDispatch;
	//Notification Data:
	TCHAR *	m_szFileName1;//<-- is the szFileName parameter to onFileAdded(),onFileRemoved,onFileModified(), and is szOldFileName to onFileNameChanged(). Is also strDirectoryName to onReadDirectoryChangesError(), onWatchStarted(), and onWatchStopped()
	TCHAR *	m_szFileName2;//<-- is the szNewFileName parameter to onFileNameChanged()
	DWORD m_dwError;	  //<-- is the dwError parameter to onWatchStarted(), and onReadDirectoryChangesError()
	//

	DWORD m_dwPartialPathOffset;//helps support FILTERS_CHECK_PARTIAL_PATH...not passed to any functions other than may be used during tests in CDelayedDirectoryChangeHandler::NotifyClientOfFileChange()


	friend class CDirChangeNotification;
	friend class CDirectoryChangeWatcher;
	friend DWORD GetPathOffsetBasedOnFilterFlags(CDirChangeNotification*,DWORD);//a friend function
};

//////////////////////////////////////////////////////////////////////////
//
//	This class makes it so that a file change notification is executed in the
//	context of the main thread, and not the worker thread.
//
//
//	It works by creating a hidden window.  When it receieves a notification
//	via one of the onFilexxx() functions, a message is posted to this window.
//	when the message is handled, the notification is fired again in the context
//	of the main thread, or whichever thread that called CDirectoryChangeWatcher::WatchDirectory()
//
//
/////////////////////////////////////////////////////////////////////////////
//	Note this code wants to use PathMatchSpec()
//	which is only supported on WINNT 4.0 w/ Internet Explorer 4.0 and above.
//	PathMatchSpec is fully supported on Win2000/XP.
//
//	For the case of WINNT 4.0 w/out IE 4.0, we'll use a simpler function.
//	some functionality is lost, but such is the price.
//

class CDelayedDirectoryChangeHandler : public CDirectoryChangeHandler
//
//	Decorates an instance of a CDirectoryChangeHandler object.
//	Intercepts notification function calls and posts them to 
//	another thread through a method implemented by a class derived from 
//	CDelayedNotifier
//	
//
//	This class implements dispatching the notifications to a thread
//	other than CDirectoryChangeWatcher::MonitorDirectoryChanges()
//
//	Also supports the include and exclude filters for each directory
//
{
private:
	CDelayedDirectoryChangeHandler();//not implemented.
public:
	CDelayedDirectoryChangeHandler(CDirectoryChangeHandler * pRealHandler, bool bAppHasGUI, LPCTSTR szIncludeFilter, LPCTSTR szExcludeFilter, DWORD dwFilterFlags);
	virtual ~CDelayedDirectoryChangeHandler();

	
	CDirectoryChangeHandler * GetRealChangeHandler()const { return m_pRealHandler; }
	CDirectoryChangeHandler * & GetRealChangeHandler(){ return m_pRealHandler; }//FYI: PCLint will give a warning that this exposes a private/protected member& defeats encapsulation.  

	void PostNotification(CDirChangeNotification * pNotification);
	void DispatchNotificationFunction(CDirChangeNotification * pNotification);


//protected:
	//These functions are called when the directory to watch has had a change made to it
	void onFileAdded(LPCTSTR strFileName);
	void onFileRemoved(LPCTSTR strFileName);
	void onFileModified(LPCTSTR strFileName);
	void onFileNameChanged(LPCTSTR strOldFileName, LPCTSTR strNewFileName);
	void onReadDirectoryChangesError(DWORD dwError, const CString & strDirectoryName);

	void onWatchStarted(DWORD dwError, const CString & strDirectoryName);
	void onWatchStopped(const CString & strDirectoryName);
	

	void SetChangedDirectoryName(const CString & strChangedDirName);
	const std::basic_string<TCHAR> & GetChangedDirectoryName()const;

	BOOL WaitForOnWatchStoppedDispatched();//see comments in .cpp


	bool NotifyClientOfFileChange(CDirChangeNotification * pNot);

	bool IncludeThisNotification(LPCTSTR szFileName);	//	based on file name.
	bool ExcludeThisNotification(LPCTSTR szFileName);	//	Allows us to filter notifications
														//
	
	

	CDirChangeNotification * GetNotificationObject();
	void DisposeOfNotification(CDirChangeNotification * pNotification);

	CDelayedNotifier * m_pDelayNotifier;
	CDirectoryChangeHandler * m_pRealHandler;	

						// m_bAppHasGUI: 
						//   This flag, if set to true, indicates that the app has a message
	bool m_bAppHasGUI;	//	 pump, and that functions are dispatched to the main thread.
						//   Otherwise, functions are dispatched to a separate worker thread.
						//
	DWORD m_dwFilterFlags;

	DWORD m_dwPartialPathOffset; //helps support FILTERS_CHECK_PARTIAL_PATH
	void SetPartialPathOffset(const CString & strWatchedDirName);

	friend class CDirectoryChangeWatcher;
	friend class CDirWatchInfo;

private:
	HANDLE m_hWatchStoppedDispatchedEvent;//supports WaitForOnWatchStoppedDispatched()

	TCHAR * m_szIncludeFilter;		//	Supports the include
	TCHAR * m_szExcludeFilter;		//	& exclude filters

	//
	//	Load PathMatchSpec dynamically because it's only supported if IE 4.0 or greater is
	//	installed.
	static HMODULE s_hShlwapi_dll;//for the PathMatchSpec() function
	static BOOL s_bShlwapi_dllExists;//if on NT4.0 w/out IE 4.0 or greater, this'll be false
	static long s_nRefCnt_hShlwapi;
	static FUNC_PatternMatchSpec s_fpPatternMatchSpec;

	BOOL _PathMatchSpec(LPCTSTR szPath, LPCTSTR szPattern);
	BOOL InitializePathMatchFunc(LPCTSTR szIncludeFilter, LPCTSTR szExcludeFilter);
	BOOL InitializePatterns(LPCTSTR szIncludeFilter, LPCTSTR szExcludeFilter);
	void UninitializePathMatchFunc();

	bool UsesRealPathMatchSpec() const;//are we using PathMatchSpec() or wildcmp()?

	//note: if the PathMatchSpec function isn't found, wildcmp() is used instead.
	//
	//	to support multiple file specs separated by a semi-colon,
	//	the include and exclude filters that are passed into the 
	//	the constructor are parsed into separate strings
	//	which are all checked in a loop.
	//
	int m_nNumIncludeFilterSpecs;
	int m_nNumExcludeFilterSpecs;


};



#else
	class CDirectoryChangeWatcher;
	class CDirectoryChangeHandler;
	class CDirChangeNotification;
	class CDelayedDirectoryChangeHandler;
	class CDelayedNotificationWindow;
	class CDelayedNotificationThread;
	class CDirectoryChangeHandler;
	class CDelayedNotifier;
#endif