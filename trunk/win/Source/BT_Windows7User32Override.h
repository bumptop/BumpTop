#ifndef _BT_WINDOWS7_USER32_OVERRIDE_
#define _BT_WINDOWS7_USER32_OVERRIDE_

// we need these definitions since we are building against the winxp platform
// under which these are not defined in the windows sdk.
#if WINVER < 0x0601
// WM_TOUCH event
#define WM_TOUCH                    0x0240

// RegisterTouchWindow flag values
#define TWF_FINETOUCH       (0x00000001)
#define TWF_WANTPALM        (0x00000002)

// Touch input flag values (TOUCHINPUT.dwFlags)
#define TOUCHEVENTF_MOVE            0x0001
#define TOUCHEVENTF_DOWN            0x0002
#define TOUCHEVENTF_UP              0x0004
#define TOUCHEVENTF_INRANGE         0x0008
#define TOUCHEVENTF_PRIMARY         0x0010
#define TOUCHEVENTF_NOCOALESCE      0x0020
#define TOUCHEVENTF_PEN             0x0040
#define TOUCHEVENTF_PALM            0x0080

// Conversion of touch input coordinates to pixels
#define TOUCH_COORD_TO_PIXEL(l)         ((l) / 100)

// Touch input mask values (TOUCHINPUT.dwMask)
#define TOUCHINPUTMASKF_TIMEFROMSYSTEM  0x0001  // the dwTime field contains a system generated value
#define TOUCHINPUTMASKF_EXTRAINFO       0x0002  // the dwExtraInfo field is valid
#define TOUCHINPUTMASKF_CONTACTAREA     0x0004  // the cxContact and cyContact fields are valid

// From http://msdn.microsoft.com/en-us/library/ms699430%28VS.85%29.aspx
#define WM_TABLET_QUERY_SYSTEM_GESTURE_STATUS 716
#define SYSTEM_GESTURE_STATUS_NOHOLD 0x0001 // Disable press-and-hold right click

// Touch input handle
DECLARE_HANDLE(HTOUCHINPUT);
typedef struct tagTOUCHINPUT {
	LONG x;
	LONG y;
	HANDLE hSource;
	DWORD dwID;
	DWORD dwFlags;
	DWORD dwMask;
	DWORD dwTime;
	ULONG_PTR dwExtraInfo;
	DWORD cxContact;
	DWORD cyContact;
} TOUCHINPUT, *PTOUCHINPUT;
typedef TOUCHINPUT const * PCTOUCHINPUT;

DECLARE_HANDLE(HGESTUREMETRICS);
typedef struct tagGESTUREMETRICS {
	UINT   cbSize; 
	DWORD  dwID;
	DWORD  dwTimeout; 
	DWORD  dwDistanceTolerance; 
	DWORD  dwAngularTolerance;
	DWORD  dwExtraInfo;
} GESTUREMETRICS, *LPGESTUREMETRICS;

// HPOWERNOTIFY
#if !defined(_HPOWERNOTIFY_DEF_)
#define _HPOWERNOTIFY_DEF_
	typedef  PVOID           HPOWERNOTIFY;
	typedef  HPOWERNOTIFY   *PHPOWERNOTIFY;
#endif

WINUSERAPI
HPOWERNOTIFY
WINAPI
RegisterPowerSettingNotification(
    IN HANDLE hRecipient,
    IN LPCGUID PowerSettingGuid,
    IN DWORD Flags
    );

WINUSERAPI
BOOL
WINAPI
UnregisterPowerSettingNotification(
    IN HPOWERNOTIFY Handle
    );

// PBT_POWERSETTINGCHANGE
#ifndef PBT_POWERSETTINGCHANGE
#define PBT_POWERSETTINGCHANGE          0x8013
typedef struct {
    GUID PowerSetting;
    DWORD DataLength;
    UCHAR Data[1];
} POWERBROADCAST_SETTING, *PPOWERBROADCAST_SETTING;
#endif // PBT_POWERSETTINGCHANGE

typedef enum {
  KF_FLAG_CREATE                = 0x00008000,
  KF_FLAG_DONT_VERIFY           = 0x00004000,
  KF_FLAG_DONT_UNEXPAND         = 0x00002000,
  KF_FLAG_NO_ALIAS              = 0x00001000,
  KF_FLAG_INIT                  = 0x00000800,
  KF_FLAG_DEFAULT_PATH          = 0x00000400,
  KF_FLAG_NOT_PARENT_RELATIVE   = 0x00000200,
  KF_FLAG_SIMPLE_IDLIST         = 0x00000100,
  KF_FLAG_ALIAS_ONLY            = 0x80000000 
} KNOWN_FOLDER_FLAG;

__inline HRESULT SHLoadLibraryFromItem(__in IShellItem *psiLibrary, __in DWORD grfMode, __in REFIID riid, __deref_out void **ppv)
{
    *ppv = NULL;
    IShellLibrary *plib;
    HRESULT hr = CoCreateInstance(CLSID_ShellLibrary, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&plib));
    if (SUCCEEDED(hr))
    {
        hr = plib->LoadLibraryFromItem(psiLibrary, grfMode);
        if (SUCCEEDED(hr))
        {
            hr = plib->QueryInterface(riid, ppv);
        }
        plib->Release();
    }
    return hr;
}
#endif

typedef BOOL (__stdcall *RegisterTouchWindowSignature)(HWND, ULONG);
typedef BOOL (__stdcall *UnregisterTouchWindowSignature)(HWND);
typedef BOOL (__stdcall *CloseTouchInputHandleSignature)(HTOUCHINPUT);
typedef BOOL (__stdcall *GetTouchInputInfoSignature)(HTOUCHINPUT, UINT, PTOUCHINPUT, int);
typedef BOOL (__stdcall *TKGetGestureMetricsSignature)(LPGESTUREMETRICS);

typedef BOOL (__stdcall *ShutdownBlockReasonCreateSignature)(HWND, LPCWSTR);
typedef BOOL (__stdcall *ShutdownBlockReasonDestroySignature)(HWND);
typedef BOOL (__stdcall *ShutdownBlockReasonQuerySignature)(HWND, LPWSTR, DWORD*);

typedef HPOWERNOTIFY (__stdcall *RegisterPowerSettingNotificationSignature)(HANDLE, LPCGUID, DWORD);
typedef BOOL (__stdcall *UnregisterPowerSettingNotificationSignature)(HPOWERNOTIFY);

typedef HRESULT (__stdcall *SHGetKnownFolderItemSignature)(REFKNOWNFOLDERID rfid, KNOWN_FOLDER_FLAG flags, HANDLE hToken, REFIID riid, void **ppv);
typedef HRESULT (__stdcall *SHGetKnownFolderIDListSignature)(REFKNOWNFOLDERID rfid, DWORD dwFlags, HANDLE hToken, PIDLIST_ABSOLUTE *ppidl);
typedef HRESULT (__stdcall *SHCreateItemFromParsingNameSignature)(PCWSTR pszPath, IBindCtx *pbc, REFIID riid, void **ppv);
typedef HRESULT (__stdcall *SHGetIDListFromObjectSignature)(IUnknown *punk, PIDLIST_ABSOLUTE *ppidl);
typedef HRESULT (__stdcall *SHGetNameFromIDListSignature)(PCIDLIST_ABSOLUTE pidl, SIGDN sigdnName, PWSTR *ppszName);
typedef HRESULT (__stdcall *SHCreateItemFromIDListSignature)(PCIDLIST_ABSOLUTE pidl, REFIID riid, void **ppv);
#endif // _BT_WINDOWS7_USER32_OVERRIDE_