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

#ifdef BUMPTOP
#pragma once
#pragma warning(disable : 4995)
#pragma warning(disable : 4996)
#pragma warning(disable : 4273)
#pragma warning(disable : 4985)
#pragma warning(disable : 4800)

// Win32 Headers
#ifdef WIN32
	#define WIN32_LEAN_AND_MEAN

	// WINVER indicates the minimum OS that we support (WindowsXP)
	#define WINVER 0x0501
	#define _WIN32_WINNT WINVER

	#include <signal.h>
	#include <resource.h>
	#include <WinSock2.h>
	#include <windows.h>
	#include <Commctrl.h>
	#include <commoncontrols.h>
	#include <Commdlg.h>
	#include <Richedit.h>
	#include <urlmon.h>
	#include <commctrl.h>
	#include <shellapi.h>
	#include <shlobj.h>
	#include <shlwapi.h>
	#include <windowsx.h>
	#include <mmsystem.h>
	#include <dbghelp.h>
	#include <gdiplus.h>
	#include <thumbcache.h>
	#include "WinSpool.h"
	#include <winhttp.h>
	#include <tpcshrd.h>
	#include <tabflicks.h>
	#include <process.h>
	#include "psapi.h"
	#include "msi.h"
	#include "dbt.h"
	#include "mlang.h"
	#include "usp10.h"
	#include "WTSapi32.h"

	#include <manipulations_i.c>
	#include <manipulations.h>
	#ifdef BTDEBUG
		#include <crtdbg.h>
	#endif

	// for smbios
	#include <comdef.h>
	#include <Wbemidl.h>
	# pragma comment(lib, "wbemuuid.lib")

	#include "VC_Manifest.h"
#endif

// System (Un)Defines
#undef random
#undef min
#undef max

// Common Header Includes
#ifdef DXRENDER
#ifdef BTDEBUG
#define D3D_DEBUG_INFO
#endif

#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "d3d9.lib")
#include <d3d9.h>
#include <d3dx9core.h>
#else
#include <glew.h>
#include "wglew.h"
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <vector>
#include <deque>
#include <map>
#include <list>
#include <queue>
#include <stack>
#include <fstream>
#include <iostream>
#include <tchar.h>
#include <strsafe.h>
#include <hash_map>
#include <NxPhysics.h>
#include <functional>
#include <algorithm>
#include <cstdio>
#include <fcntl.h>
#include <io.h>
#include <math.h>
#include <IL/il.h>
#include <IL/ilu.h>
#include <IL/ilut.h>
#include <cstdlib>
#include <ctime>
#include <Iphlpapi.h>
#include <boost/config.hpp>
#include <boost/functional/hash.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/program_options.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <json/value.h>
#include <json/reader.h>
#include <json/writer.h>

//NEWMAT 10
#include "newmat10/newmat.h"
#include "newmat10/newmatio.h"
#include "newmat10/myexcept.h"

#if TWITTER_OAUTH
	#include <openssl/hmac.h>
	#include <openssl/evp.h>
	#include <openssl/x509.h>
	#include <openssl/x509v3.h>
	#include <openssl/ssl.h>
#endif
#include <gtest/gtest.h> // googletest
#include "EnumProc.h"

#ifndef DXRENDER
#include "ShapeVis.h"
#endif

#if ENABLE_WEBKIT
	#ifdef BTDEBUG
		#pragma comment(lib, "QtGuid4.lib")
		#pragma comment(lib, "QtWebKitd4.lib")
		#pragma comment(lib, "QtNetworkd4.lib")
	#else
		#pragma comment(lib, "QtGui4.lib")
		#pragma comment(lib, "QtWebKit4.lib")
		#pragma comment(lib, "QtNetwork4.lib")
	#endif

	#ifdef QT_NO_OPENSSL
		#error "BumpTop web functionality requires Qt with SSL support"
	#endif
#endif

// qt headers
#include "QtCore/QtCore"
#include "QtGui/QtGui"
#include "QtWebKit/QtWebKit"

// Namespaces
using namespace std;
using namespace stdext;
using namespace boost;
using namespace boost::serialization;
using namespace boost::archive;
using namespace boost::program_options;

// Typedefs
typedef unsigned int				uint;
typedef unsigned char				byte;
typedef NxVec3						Vec3;
typedef NxVec3						Vec2;
typedef NxMat33						Mat33;
typedef NxMat34						Mat34;
typedef NxQuat						Quat;
typedef NxRay						Ray;
typedef NxBox						Box;
typedef NxBounds3					Bounds;
typedef vector<QString>				StrList;
typedef vector<NxVec3>				Vec3List;
typedef vector<Mat33>				Mat33List;
typedef vector<Mat34>				Mat34List;
typedef void						(* VoidCallback)(void);
typedef bool						(* BoolCallback)(void);

// Special SDK Enablers
//#define SMART_SUPPORT

// Preprocessor Macros
#define SAFE_RELEASE(p)				{ if (p) { (p)->Release(); (p) = NULL; } }
#define SAFE_DELETE(p)				{ if (p) { delete (p);     (p) = NULL; } }
#define SAFE_DELETE_ARRAY(p)		{ if (p) { delete [] (p);  (p) = NULL; } }
#define WINDOW_NAME					L"BumpTop"
#define TOPLEVEL_WINDOW_NAME		L"BumpTopTopLevel"
#define ICON_PATH_CONCAT			";"
#define TIMER_ID_NUMBER				1
#define TIMER_CALLBACK_STEP			15
#define BT_REGULAR_INPUT_BOX		1
#define BT_CRASH_REPORT_BOX			2
#define BT_TIMERCALLBACK			1
#define WINDOW_TIMERCALLBACK		2
#define STATS_TIMERCALLBACK			3
#define INTERACTION_TIMERCALLBACK	4
#define SCREENSHOT_TIMERCALLBACK	5
#define CHECK_FOR_TOGGLE_DESKTOP_TIMERCALLBACK	6
#define REAUTHORIZATION_TIMERCALLBACK 7
#define VIRTUALICONSYNC_TIMERCALLBACK 8
#define MAX_PATHS					32768

// Translation
#include "BT_Singleton.h"
class Translations
{
	Q_DECLARE_TR_FUNCTIONS(Translations)

private:
	QTranslator * _translator;
	QTranslator * _multiTouchTranslator;
	QTranslator * _webTranslator;

private:
	friend class Singleton<Translations>;
	Translations();

public:
	QString translate(const char * context, const char * str);
	QString translateWeb(const char * context, const char * str);
};

#define MAKE_STRING_A(x) # x

//don't translate
#define QT_NT(str) str
typedef QString QString_NT;

#undef QT_TR_NOOP
#define QT_TR_NOOP(str) (Singleton<Translations>::getInstance()->translate(trContext(), str))

#undef QT_TRANSLATE_NOOP
#define QT_TRANSLATE_NOOP(scope, str) (Singleton<Translations>::getInstance()->translate(scope, str))

#define Q_DECLARE_TR_CONTEXT(context) \
public: static inline char * trContext() \
		{ return #context; } \
private:

//Logging
#include "BT_Logger.h"

// Functional Macros
#define for_each						BOOST_FOREACH

typedef HWND PWND;
#endif // BUMPTOP
