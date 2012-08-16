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
#include "BT_ChooseVersionDialog.h"
#include "BT_CrashDialog.h"
#include "BT_DialogManager.h"
#include "BT_FileSystemManager.h"
#include "BT_PhotoFrameDialog.h"
#include "BT_QtUtil.h"
#include "BT_RenderManager.h"
#include "BT_SettingsAppMessageHandler.h"
#include "BT_SceneManager.h"
#include "BT_ThankYouDialog.h"
#include "BT_Util.h"
#include "BT_WindowsOS.h"
#include "TwitterClient.h"

//---------------------------------------------------------------------------------------------------------

// Sets the controls of a dialog to the same appearance as in 96 DPI to match the background picture,
// which is not scaled based on DPI.
void Win32Dialog::AdjustDialogDPI(PWND dialog, int targetClientW, int targetClientH)
{
	WINDOWINFO winfo = {0};
	winfo.cbSize = sizeof(WINDOWINFO);

	int dpiX = GetDeviceCaps(GetDC(dialog), LOGPIXELSX);
	int dpiY = GetDeviceCaps(GetDC(dialog), LOGPIXELSY);

	GetWindowInfo(dialog, &winfo);

	int x = winfo.rcWindow.left;
	int y = winfo.rcWindow.top;
	int w = (winfo.rcWindow.right - winfo.rcWindow.left);
	int h = (winfo.rcWindow.bottom - winfo.rcWindow.top);

	int cw = winfo.rcClient.right - winfo.rcClient.left;
	int ch = winfo.rcClient.bottom - winfo.rcClient.top;
	
	int bw = w - cw,  bh = h - ch; //border width, height
	
	// Scale the dialog window to relative the same size as in 96 DPI (standard).
	// The scaling the slightly off, even though in MSDN article the scaling is simply 96 / DPI
	// So we find the exact scaling using hard coded dialog box sizes
	
	dpiScaleX = targetClientW / float(cw);  
	dpiScaleY = targetClientH / float(ch); 
		
	w = bw + cw * dpiScaleX;
	h = bh + ch * dpiScaleY;
	
	SetWindowPos(dialog, NULL, x, y, w, h, SWP_NOZORDER);
}

// Sets the controls of a dialog to the same appearance as in 96 DPI to match the background picture,
// which is not scaled based on DPI.
void Win32Dialog::AdjustControlDPI(PWND owner, PWND control)
{
	WINDOWINFO winfo = {0};
	winfo.cbSize = sizeof(WINDOWINFO);

	int ownerCX = 0, ownerCY = 0;

	GetWindowInfo(owner, &winfo);
	ownerCX = winfo.rcClient.left;
	ownerCY = winfo.rcClient.top;

	GetWindowInfo(control, &winfo);

	// Scale the dialog window to relative the same size as in 96 DPI (standard).
	// The scaling the slightly off, even though in MSDN article the scaling is simply 96 / DPI
	// So we use calculated scale from the hard coded dialog size
	int x = (winfo.rcWindow.left - ownerCX) * dpiScaleX;
	int y = (winfo.rcWindow.top - ownerCY) * dpiScaleY;
	int w = (winfo.rcWindow.right - winfo.rcWindow.left) * dpiScaleX;
	int h = (winfo.rcWindow.bottom - winfo.rcWindow.top) * dpiScaleY;

	SetWindowPos(control, NULL, x, y, w, h, SWP_NOZORDER);
}

bool Win32Dialog::onDestroy()
{
	return true;
}

bool Win32Dialog::closeDialog(HWND hwnd, int returnCode)
{
	onDestroy();
	EndDialog(hwnd, returnCode);
	return true;
}

bool Win32Dialog::onCommand(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	return true;
}

bool Win32Dialog::onMouseDown(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	return true;
}
bool Win32Dialog::onMouseUp(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	return true;
}

bool Win32Dialog::onMouseLeave(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	return true;
}

bool Win32Dialog::onMouseMove(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	return true;
}

//---------------------------------------------------------------------------------------------------------
struct RichTextString
{
	TCHAR * str;
	unsigned int len;
};

static DWORD CALLBACK StreamQFileContentsCallback(DWORD dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
	QFile * file = (QFile *) dwCookie;
	*pcb = file->read((char *) pbBuff, cb);
	return 0;
}
//---------------------------------------------------------------------------------------------------------

// static inits
WNDPROC DialogManager::prevRichEditCtrlWndProc = NULL;

DialogManager::DialogManager()
: _comctrlModule(NULL)
, _pTaskDialogIndirect(NULL)
{
	expiredChooseVersionDialog = false;	

	// create the default complex dialogs
	complexDialogs.insert(make_pair(DialogPhotoFrame, new PhotoFrameDialog));
	complexDialogs.insert(make_pair(DialogCrash, new CrashDialog));
	complexDialogs.insert(make_pair(DialogChooseVersion, new ChooseVersionDialog));
	complexDialogs.insert(make_pair(DialogThankYou, new ThankYouDialog));

	// try and load the task dialog functions from the comctrl api
	_comctrlModule = LoadLibrary(_T("comctl32.dll"));
	if (_comctrlModule)
	{
		_pTaskDialogIndirect = (TaskDialogIndirect) GetProcAddress(_comctrlModule, "TaskDialogIndirect");
	}
}

DialogManager::~DialogManager()
{
	if (_comctrlModule)
		FreeLibrary(_comctrlModule);

	// free all the remaining complex dialogs
	ComplexDialogsContainer::iterator iter = complexDialogs.begin();
	while (iter != complexDialogs.end())
	{
 		delete iter->second;
		iter++;
	}
}

bool DialogManager::clearState()
{
	// In the case of Novodex Crashes, this clears the Message Box
	if (dialogType == DialogCaptionOnly && hDlg)
	{
		EndDialog(hDlg, NULL);
	}

	// Clear All state
	hDlg = NULL;
	text.clear();
	subText.clear();
	caption = "BumpTop";
	prompt.clear();
	emailAddress.clear();
	setHasParent(true);
	setPressedAuthorizeManually(false);
	setPressedFree(false);
	setExpiredChooseVersionDialog(false);

	return true;
}

bool DialogManager::promptDialog(DialogType boxType)
{
	if(scnManager->skipAllPromptDialogs)
		return true;

	bool rc = false;

	// Save the type of Dialog we will be creating
	winOS->SetConditionalFlag(ThreadSafeProcessing, true);
	dialogType = boxType;

	if (boxType >= DialogOK && boxType <= DialogYesNo)
	{
		rc = promptStock();
	}
	else if (!vistaPromptDialogOverride(boxType, rc))
	{
		if (boxType != DialogDirectoryListing)
		{
			rc = promptCustom();
		}else{
			// Directory Listing Box
			rc = promptDirListing();			
		}
	}

	// Turn off ThreadSafe Processing
	winOS->SetConditionalFlag(ThreadSafeProcessing, false);
	return rc;
}

void DialogManager::setPrompt(QString newPrompt)
{
	// Set a prompt (If Applicable)
	prompt = newPrompt;
}

bool DialogManager::promptStock()
{
	int rcVal;

	// This case is for stock MessageBox Dialogs
	rcVal = MessageBox(winOS->GetWindowsHandle(), (LPCTSTR) prompt.utf16(),(LPCTSTR) caption.utf16(), (UINT) dialogType | MB_ICONINFORMATION | MB_TOPMOST);

	// Return false only if the user selected a negative button (Cancel, No, etc)
	return (rcVal != IDCANCEL && rcVal != IDNO);
}

bool DialogManager::promptCustom()
{
	int x, y, w, h, l, r, t, b, rc;
	RECT rect;
	MSG uMsg;

	// Choose the function to call that takes care of the processing
	if (dialogType == DialogCaptionOnly) 
	{
		// Create Modeless DIalog
		hDlg = CreateDialog(winOS->GetInstanceHandle(), MAKEINTRESOURCE(dialogType), getHasParent() ? winOS->GetWindowsHandle() : NULL, NULL);

		// Modeless Dialogs don't have their own processing function, we setup everything here
		SetDlgItemText(hDlg, IDC_MODELESS_PROMPT, (LPCWSTR) prompt.utf16());
		winOS->GetWorkArea(r, l, t, b);
		GetWindowRect(hDlg, &rect);
		x = r - l;
		y = b - t; 		
		w = rect.right - rect.left;
		h = rect.bottom - rect.top;

		SetWindowPos(hDlg, GetTopWindow(NULL), (x / 2) - w / 2, (y / 2) - h / 2, w, h, SWP_SHOWWINDOW | SWP_FRAMECHANGED);
		ShowWindow(hDlg, SW_SHOW);

		// This forcefully updates the windows Rendering of our modeless box
		while (PeekMessage(&uMsg, NULL, 0, 0, PM_REMOVE) == TRUE)
		{
			TranslateMessage(&uMsg);
			DispatchMessage(&uMsg);
		}

		return true;

	}else{
		// This case is for our own custom Dialog Boxes
		rc = DialogBox(winOS->GetInstanceHandle(), MAKEINTRESOURCE(dialogType), getHasParent() ? winOS->GetWindowsHandle() : NULL, (DLGPROC) dlgProc);

		return (rc != 0);
	}
}

bool DialogManager::promptDirListing()
{
	TCHAR szFile[260];
	BROWSEINFO bi = { 0 };
	LPITEMIDLIST pidl;
	bool rc = false;;

	bi.lpszTitle = (LPCTSTR) caption.utf16();
	bi.pszDisplayName = szFile;
	bi.ulFlags = BIF_USENEWUI | BIF_EDITBOX | BIF_NEWDIALOGSTYLE | BIF_BROWSEINCLUDEURLS | BIF_SHAREABLE | BIF_UAHINT | BIF_BROWSEINCLUDEFILES;

	// Get the PIDL of the name selected
	pidl = SHBrowseForFolder(&bi);

	if (pidl)
	{
		// Retrieve the filename
		SHGetPathFromIDList(pidl, szFile);
		CoTaskMemFree(pidl);

		// Save the file Path selected and exit
		setText(QString::fromUtf16((const ushort *) szFile));
		rc = true;
	}

	return rc;
}

void DialogManager::setCaption(QString newCaption)
{
	// Set a caption for the dialog Box (If Applicable)
	caption = newCaption;
}

void DialogManager::setText(QString in)
{
	// Set the default text
	text = in;
	setTextSelection(0, -1);
}

void DialogManager::setTextSelection(int start, int end)
{
	selText.x = start;
	selText.y = end;
}

void DialogManager::setSubText(QString in)
{
	// Set the sub text
	subText = in;
	setSubTextSelection(0, -1);
}

void DialogManager::setSubTextSelection(int start, int end)
{
	selSubText.x = start;
	selSubText.y = end;
}

void DialogManager::setType(QString t)
{
	type = t;
}

void DialogManager::setEmail(QString newEmail)
{
	// Set a new email
	emailAddress = newEmail;
}

void DialogManager::setImage( QString filePath )
{
	// Set a new image to be displayed
	image = filePath;
}

void DialogManager::setChecked( bool c )
{
	checked = c;
}

void DialogManager::setHasParent(bool hasParent)
{
	this->hasParent = hasParent;
}


QString DialogManager::getText() const
{
	// Return what the user entered
	return text;
}

POINT DialogManager::getTextSelection() const
{
	return selText;
}

QString DialogManager::getSubText() const
{
	// Return what the user entered in the second field
	return subText;
}

POINT DialogManager::getSubTextSelection() const
{
	return selSubText;
}

QString DialogManager::getType() const
{
	return type;
}

QString DialogManager::getEmail() const
{
	// Return the email that was entered
	return emailAddress;
}

QString DialogManager::getCaption() const
{
	// Return the caption of the window
	return caption;
}

QString DialogManager::getPrompt() const
{
	// Return the prompt
	return prompt;
}

DialogType DialogManager::getDialogType() const
{
	// Return the last used Dialog Type
	return dialogType;
}

QString DialogManager::getImage() const
{
	// Return the image displayed
	return image;
}
bool DialogManager::getChecked() const
{
	return checked;
}

bool DialogManager::getHasParent()
{
	return this->hasParent;
}


LRESULT DialogManager::dlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	uint maxChar = 65536;
	TCHAR buf[65536];
	DialogType currentDialogType = dlgManager->getDialogType();
	Win32Dialog * complexDialog = dlgManager->getComplexDialog(currentDialogType);

	switch (msg)
	{
		case WM_INITDIALOG:
			// Init Dialog
			if (!dlgManager->getCaption().isEmpty())
				SetWindowText(hwnd, (LPCTSTR) dlgManager->getCaption().utf16());

			// center the dialog
			{
				int l, r, t, b;
				winOS->GetWorkArea(r, l, t, b);
				int windowWidth = r - l;
				int windowHeight = b - t;
				RECT dialogRect;
				GetWindowRect(hwnd, &dialogRect);
				SetWindowPos(hwnd, NULL, (windowWidth - (dialogRect.right - dialogRect.left))/2, 
					(windowHeight - (dialogRect.bottom - dialogRect.top))/2, 0, 0, 
					SWP_NOSIZE | SWP_NOZORDER);
			}

			switch (currentDialogType)
			{
				case DialogAuthorizationFailed:
					SetWindowText(hwnd, (LPCTSTR) QT_TR_NOOP("Help us help you!").utf16());
					SetDlgItemText(hwnd, IDC_PROMPT, (LPCTSTR) QT_TR_NOOP("You may be running an altered version of BumpTop!\n\nPlease consider purchasing a BumpTop Pro license so that we can continue innovating on the best 3D physics-based desktop available.").utf16());
					SetDlgItemText(hwnd, IDOK, (LPCTSTR) QT_TR_NOOP("Buy BumpTop Pro License...").utf16());
					SetDlgItemText(hwnd, IDCANCEL, (LPCTSTR) QT_TR_NOOP("Continue running free version").utf16());
					break;
				case DialogAuthorizationChoose:
					{
						QString prompt = dlgManager->getPrompt();
						QString proButtonText = dlgManager->getText();
						QString freeButtonText = dlgManager->getSubText();
						SetWindowText(hwnd, (LPCTSTR) QT_TR_NOOP("Choose BumpTop Version").utf16());
						SetDlgItemText(hwnd, IDC_QUESTION, (LPCTSTR) prompt.utf16());
						SetDlgItemText(hwnd, IDOK, (LPCTSTR) proButtonText.utf16());
						SetDlgItemText(hwnd, IDNO, (LPCTSTR) freeButtonText.utf16());
						SetDlgItemText(hwnd, IDC_LEARNMORE, (LPCTSTR) QT_TR_NOOP("Why go Pro?").utf16());

						HBITMAP bmp = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_AUTH_HEADER));
						HWND pictureCtrl = GetDlgItem(hwnd, IDC_AUTH_HEADER);
						SendMessage(pictureCtrl, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM) bmp);
					}
					break;
				case DialogTwitterAuth:
					SetWindowText(hwnd, (LPCTSTR) QT_TR_NOOP("Twitter Integration Login").utf16());
					SetDlgItemText(hwnd, IDC_PROMPT, (LPCTSTR) QT_TR_NOOP("We are redirecting you to the Twitter application authorization page.  Please enter your PIN below after you have logged in.").utf16());
					SetDlgItemText(hwnd, IDOK, (LPCTSTR) QT_TR_NOOP("Start Twittering!").utf16());
					SetDlgItemText(hwnd, IDCANCEL, (LPCTSTR) QT_TR_NOOP("Cancel").utf16());
					break;
				case DialogFacebookAuth:
					SetWindowText(hwnd, (LPCTSTR) QT_TR_NOOP("Facebook Photo Uploader Login").utf16());
					SetDlgItemText(hwnd, IDC_PROMPT, (LPCTSTR) QT_TR_NOOP("We are redirecting you to the BumpTop's Facebook Photo Uploader login page.  Please continue only after you have logged in.\n\n(You only have to login once!)").utf16());
					SetDlgItemText(hwnd, IDOK, (LPCTSTR) QT_TR_NOOP("Start uploading photos to Facebook!").utf16());
					SetDlgItemText(hwnd, IDCANCEL, (LPCTSTR) QT_TR_NOOP("Cancel").utf16());
					break;
				case DialogFacebookConfirm:
					SetWindowText(hwnd, (LPCTSTR) QT_TR_NOOP("Upload to Facebook?").utf16());
					SetDlgItemText(hwnd, IDC_PROMPT, (LPCTSTR) QT_TR_NOOP("Upload these photo(s) to Facebook?").utf16());
					SetDlgItemText(hwnd, IDOK, (LPCTSTR) QT_TR_NOOP("Upload!").utf16());
					SetDlgItemText(hwnd, IDCANCEL, (LPCTSTR) QT_TR_NOOP("Cancel").utf16());
					break;
				case DialogDeauthorizeConfirm:
					SetWindowText(hwnd, (LPCTSTR) QT_TR_NOOP("BumpTop Settings").utf16());
					SetDlgItemText(hwnd, IDC_PROMPT, (LPCTSTR) QT_TR_NOOP("Are you sure you want to deauthorize your pro key?").utf16());
					SetDlgItemText(hwnd, IDOK, (LPCTSTR) QT_TR_NOOP("Yes, deauthorize").utf16());
					SetDlgItemText(hwnd, IDCANCEL, (LPCTSTR) QT_TR_NOOP("No, do not deauthorize").utf16());
					break;
				case DialogUpdateMessage:
					SetWindowText(hwnd, (LPCTSTR) QT_TR_NOOP("BumpTop Notification").utf16());
					SetDlgItemText(hwnd, IDC_PROMPT, (LPCTSTR) dlgManager->getPrompt().utf16());
					SetDlgItemText(hwnd, IDOK, (LPCTSTR) QT_TR_NOOP("Visit BumpTop for more information...").utf16());
					SetDlgItemText(hwnd, IDCANCEL, (LPCTSTR) QT_TR_NOOP("Close").utf16());
					break;
				case DialogChangeIcon:
					SetWindowText(hwnd, (LPCTSTR) QT_TR_NOOP("Change Icon?").utf16());
					SetDlgItemText(hwnd, IDOK, (LPCTSTR) QT_TR_NOOP("Change Icon...").utf16());
					SetDlgItemText(hwnd, IDCANCEL, (LPCTSTR) QT_TR_NOOP("Clear changes").utf16());
					SetFocus(GetDlgItem(hwnd, IDOK));
					break;
				case DialogFeedback:
					SetWindowText(hwnd, (LPCTSTR) QT_TR_NOOP("BumpTop Feedback Reporter").utf16());
					SendMessage(GetDlgItem(hwnd, IDC_FEEDBACK_TYPE), CB_ADDSTRING, NULL, (LPARAM) QT_TR_NOOP("General feedback").utf16());
					SendMessage(GetDlgItem(hwnd, IDC_FEEDBACK_TYPE), CB_ADDSTRING, NULL, (LPARAM) QT_TR_NOOP("Bug/Problem (give as much info as possible)").utf16());
					SendMessage(GetDlgItem(hwnd, IDC_FEEDBACK_TYPE), CB_ADDSTRING, NULL, (LPARAM) QT_TR_NOOP("Feature request").utf16());
					SendMessage(GetDlgItem(hwnd, IDC_FEEDBACK_TYPE), CB_SETCURSEL, 0, NULL);
					SetDlgItemText(hwnd, IDC_FEEDBACK_EMAIL, (LPCTSTR) dlgManager->getEmail().utf16());
					SetDlgItemText(hwnd, IDC_FEEDBACK_MESSAGE, (LPCTSTR) dlgManager->getText().utf16());
					SetDlgItemText(hwnd, IDC_CRASHBOX_PROMPT, (LPCTSTR) QT_TR_NOOP("We want your feedback!\nLet us know what you think about BumpTop and how we can improve the experience.").utf16());
					SetDlgItemText(hwnd, IDC_FEEDBACK_LABEL, (LPCTSTR) QT_TR_NOOP("Type of feedback:").utf16());
					SetDlgItemText(hwnd, IDC_FEEDBACK_LABEL2, (LPCTSTR) QT_TR_NOOP("Email Address:").utf16());
					SetDlgItemText(hwnd, IDC_FEEDBACK_LABEL3, (LPCTSTR) QT_TR_NOOP("Message:").utf16());
					SetDlgItemText(hwnd, IDC_STATIC, (LPCTSTR) QT_TR_NOOP("If this is a bug, help us to locate and fix the problem by explaining the problem in detail, listing the steps taken to produce the problem, and what you expected to happen instead.").utf16());
					SetDlgItemText(hwnd, IDOK, (LPCTSTR) QT_TR_NOOP("Send Feedback").utf16());
					SetDlgItemText(hwnd, IDCANCEL, (LPCTSTR) QT_TR_NOOP("Cancel").utf16());
					SetFocus(GetDlgItem(hwnd, IDC_FEEDBACK_MESSAGE));
					break;
				case DialogBreakPile:
					SetWindowText(hwnd, (LPCTSTR) QT_TR_NOOP("Break Folder Pile?").utf16());
					SetDlgItemText(hwnd, IDC_PROMPT, (LPCTSTR) dlgManager->getPrompt().utf16());
					SetDlgItemText(hwnd, IDNO, (LPCTSTR) QT_TR_NOOP("Move files").utf16());
					SetDlgItemText(hwnd, IDOK, (LPCTSTR) QT_TR_NOOP("Move files and break pile").utf16());
					SetDlgItemText(hwnd, IDCANCEL, (LPCTSTR) QT_TR_NOOP("Cancel").utf16());
					SetFocus(GetDlgItem(hwnd, IDOK));
					break;

				case DialogInputAuth:
					{
						SetDlgItemText(hwnd, IDC_BUYPROKEY, (LPCTSTR) QT_TR_NOOP("Go Pro").utf16());
						SetDlgItemText(hwnd, IDC_AUTHORIZEMANUALLY, (LPCTSTR) QT_TR_NOOP("Manual").utf16());
						SetDlgItemText(hwnd, IDC_PROXYSETTINGS, (LPCTSTR) QT_TR_NOOP("Proxy").utf16());

						HBITMAP bmp = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_AUTH_HEADER));
						HWND pictureCtrl = GetDlgItem(hwnd, IDC_AUTH_HEADER);
						SendMessage(pictureCtrl, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM) bmp);
					}
				case DialogInput:
				case DialogInput2:
					{
						// Regular Dialog Box
						QString caption = dlgManager->getCaption();
						if (caption.isEmpty())
							caption = QT_TR_NOOP("BumpTop");
						SetWindowText(hwnd, (LPCTSTR) caption.utf16());
						SetDlgItemText(hwnd, IDC_INPUTBOX_PROMPT, (LPCTSTR) dlgManager->getPrompt().utf16());
						SetDlgItemText(hwnd, IDC_INPUTBOX_TEXT, (LPCTSTR) dlgManager->getText().utf16());
						SetDlgItemText(hwnd, IDOK, (LPCTSTR) QT_TR_NOOP("Ok").utf16());
						SetDlgItemText(hwnd, IDCANCEL, (LPCTSTR) QT_TR_NOOP("Cancel").utf16());
						SetFocus(GetDlgItem(hwnd, IDC_INPUTBOX_TEXT));

						if (dlgManager->getText().size() > 0)
						{
							POINT selection = dlgManager->getTextSelection();
							SendMessage(GetDlgItem(hwnd, IDC_INPUTBOX_TEXT), EM_SETSEL, selection.x, MAKELPARAM(selection.y, 0));
						}
					}
					break;

				case DialogInputBrowse:
					// Dialog with Browse Button
					SetWindowText(hwnd, (LPCTSTR) QT_TR_NOOP("BumpTop").utf16());
					SetDlgItemText(hwnd, IDC_INPUTBOX_BROWSE_PROMPT, (LPCTSTR) dlgManager->getPrompt().utf16());
					SetDlgItemText(hwnd, IDC_INPUTBOX_BROWSE_TEXT, (LPCTSTR) dlgManager->getText().utf16());
					SetDlgItemText(hwnd, IDOK, (LPCTSTR) QT_TR_NOOP("Ok").utf16());
					SetDlgItemText(hwnd, IDCANCEL, (LPCTSTR) QT_TR_NOOP("Cancel").utf16());
					SetFocus(GetDlgItem(hwnd, IDC_INPUTBOX_BROWSE_TEXT));
					break;

				case DialogThemeConflict:
					// Theme Conflict dialog			
					SetWindowText(hwnd, (LPCTSTR) QT_TR_NOOP("BumpTop Theme Conflict").utf16());
					SetDlgItemText(hwnd, IDC_THEME_CONFLICT_PROMPT, (LPCTSTR) dlgManager->getPrompt().utf16());
					SetDlgItemText(hwnd, IDOK, (LPCTSTR) QT_TR_NOOP("Merge").utf16());
					SetDlgItemText(hwnd, IDNO, (LPCTSTR) QT_TR_NOOP("Replace with Default").utf16());
					SetDlgItemText(hwnd, IDCANCEL, (LPCTSTR) QT_TR_NOOP("Quit").utf16());
					break;

				case DialogCaptionOnly:
					SetWindowText(hwnd, (LPCTSTR) QT_TR_NOOP("BumpTop").utf16());
					break;

				case DialogUpdateBumptop:
					// set the strings
					SetWindowText(hwnd, (LPCTSTR) QT_TR_NOOP("New Update Available!").utf16());
					SetDlgItemText(hwnd, IDC_UPDATE_SEND_SCREENSHOT, (LPCTSTR) QT_TR_NOOP("Submit a screenshot of my Desktop to help improve BumpTop.  It will be kept private.").utf16());
					SetDlgItemText(hwnd, IDOK, (LPCTSTR) QT_TR_NOOP("Install").utf16());
					SetDlgItemText(hwnd, IDCANCEL, (LPCTSTR) QT_TR_NOOP("Cancel").utf16());

					// Update bumptop dialog
					SendMessage(GetDlgItem(hwnd, IDC_UPDATE_SEND_SCREENSHOT), BM_SETCHECK, (dlgManager->getChecked() ? BST_CHECKED : BST_UNCHECKED), 0);
					{
						HBITMAP bitmap;
						ULONG_PTR gdiplusToken;
						Gdiplus::GdiplusStartupInput gdiplusStartupInput;
						if (Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL) == Gdiplus::Ok)
						{					
							QString imagePath = dlgManager->getImage();
							Gdiplus::Bitmap gdiBitmap((LPCWSTR) imagePath.utf16(), FALSE);
							gdiBitmap.GetHBITMAP(Gdiplus::Color(), &bitmap);
							if (bitmap)
							{
								SendMessage(GetDlgItem(hwnd, IDC_UPDATE_SCREENSHOT), STM_SETIMAGE, IMAGE_BITMAP, (LPARAM) bitmap);
								SetWindowPos(GetDlgItem(hwnd, IDC_UPDATE_SCREENSHOT), NULL, 0, 0, 234, 150, SWP_NOZORDER | SWP_NOMOVE);
							}
						}
						Gdiplus::GdiplusShutdown(gdiplusToken);
					}
					SetFocus(GetDlgItem(hwnd, IDC_UPDATE_SEND_SCREENSHOT));
					break;
				case DialogEula:
					{
						SetWindowText(hwnd, (LPCTSTR) QT_TR_NOOP("End User License Agreement").utf16());
						SetDlgItemText(hwnd, IDOK, (LPCTSTR) QT_TR_NOOP("&Accept").utf16());
						SetDlgItemText(hwnd, IDCANCEL, (LPCTSTR) QT_TR_NOOP("&Decline").utf16());

						// enable word wrapping
						SendMessage(GetDlgItem(hwnd, IDC_TEXTBOX), EM_SETWORDWRAPMODE, WBF_WORDBREAK, 0);
						SendMessage(GetDlgItem(hwnd, IDC_TEXTBOX), EM_SETTARGETDEVICE, 0, 0);

						// stream the localized eula text into the control
						QString rtfFilePath = dlgManager->getText();
						QFile rtfFile(rtfFilePath);
						rtfFile.open(QFile::ReadOnly);
						
						EDITSTREAM es = {0};
						es.dwCookie = (DWORD) &rtfFile;
						es.pfnCallback = StreamQFileContentsCallback; 
						::SendMessage(GetDlgItem(hwnd, IDC_TEXTBOX), EM_STREAMIN, SF_RTF, (LPARAM) &es);

						rtfFile.close();
					}
					break;

				case DialogTwitterTweet:
					{						
						// NOTE: the prompt is _appended_ to the prompt here
						QString caption = dlgManager->getCaption();
						QString prompt = dlgManager->getPrompt();
						SetWindowText(hwnd, (LPCTSTR) (caption.isEmpty() ? QT_TR_NOOP("Update Twitter") : caption).utf16());
						SetDlgItemText(hwnd, IDOK, (LPCTSTR) QT_TR_NOOP("&Update").utf16());
						SetDlgItemText(hwnd, IDCANCEL, (LPCTSTR) QT_TR_NOOP("Cancel").utf16());
						SetDlgItemText(hwnd, IDC_TWITTER_LABEL, (LPCTSTR) (prompt.isEmpty() ? QT_TR_NOOP("What are you doing?") : prompt).utf16());
						SetDlgItemText(hwnd, IDC_TWITTER_CHARCOUNT, (LPCTSTR) QT_TR_NOOP("%1 characters left").arg(TWITTER_MAX_CHARS).utf16());

						// enable word wrapping
						SendMessage(GetDlgItem(hwnd, IDC_TEXTBOX), EM_SETWORDWRAPMODE, WBF_WORDBREAK, 0);
						SendMessage(GetDlgItem(hwnd, IDC_TEXTBOX), EM_SETTARGETDEVICE, 0, 0);

						// hook the wnd proc
						prevRichEditCtrlWndProc = (WNDPROC) SetWindowLong(GetDlgItem(hwnd, IDC_TEXTBOX), GWL_WNDPROC, (LONG) richEditCtrlProc);

						// i18n
						QString msg = dlgManager->getText();
						SETTEXTEX text = {0};
						text.flags = ST_DEFAULT;
						text.codepage = 1200;		// unicode code page
						SendMessage(GetDlgItem(hwnd, IDC_TEXTBOX), EM_SETTEXTEX, (WPARAM) &text, (LPARAM) msg.utf16());

						// font size
						CHARFORMAT2 charFormat;
						ZeroMemory(&charFormat, sizeof(CHARFORMAT2));
						charFormat.cbSize = sizeof(CHARFORMAT2);
						charFormat.dwMask = CFM_SIZE;
						charFormat.yHeight = 220;
						SendMessage(GetDlgItem(hwnd, IDC_TEXTBOX), EM_SETCHARFORMAT, SCF_ALL, (LPARAM) &charFormat);

						// move the caret to the beginning
						CHARRANGE range = {0};
						SendMessage(GetDlgItem(hwnd, IDC_TEXTBOX), EM_EXSETSEL, 0, (LPARAM) &range);

						SetFocus(GetDlgItem(hwnd, IDC_TEXTBOX));
					}
					break;
				case DialogTwitterLogin:
					SetWindowText(hwnd, (LPCTSTR) QT_TR_NOOP("Login to Twitter").utf16());
					SetDlgItemText(hwnd, IDC_TWITTER_LOGIN_USERNAME_LABEL, (LPCTSTR) QT_TR_NOOP("Username").utf16());
					SetDlgItemText(hwnd, IDC_TWITTER_LOGIN_PASSWORD_LABEL, (LPCTSTR) QT_TR_NOOP("Password").utf16());
					SetDlgItemText(hwnd, IDC_TWITTER_LOGIN_LABEL, (LPCTSTR) QT_TR_NOOP("Please login below to send tweets and upload photos to your Twitter!").utf16());
					SetDlgItemText(hwnd, IDOK, (LPCTSTR) QT_TR_NOOP("Login to Twitter").utf16());
					SetDlgItemText(hwnd, IDCANCEL, (LPCTSTR) QT_TR_NOOP("Cancel").utf16());
					SetFocus(GetDlgItem(hwnd, IDC_TWITTER_LOGIN_USERNAME));
					break;

				case DialogUpdateGraphicsDrivers:
					// set the strings
					SetWindowText(hwnd, (LPCTSTR) QT_TR_NOOP("Error Initializing Hardware Acceleration").utf16());
					SetDlgItemText(hwnd, IDC_PROMPT, (LPCTSTR) QT_TR_NOOP("BumpTop is unable to initialize 3D hardware acceleration.  Please update to the latest drivers for your video card and relaunch BumpTop.").utf16());
					SetDlgItemText(hwnd, IDOK, (LPCTSTR) QT_TR_NOOP("Download latest graphics drivers...").utf16());
					SetDlgItemText(hwnd, IDCANCEL, (LPCTSTR) QT_TR_NOOP("Exit BumpTop").utf16());
					break;

				default:
					if (complexDialog)
					{
						if (currentDialogType == DialogChooseVersion)
						{
							if (dlgManager->getExpiredChooseVersionDialog())
								complexDialog->onCommand(NULL, MAKEWPARAM(TRUE, 0), 0);
							else
								complexDialog->onCommand(NULL, MAKEWPARAM(FALSE, 0), 0);
						}		
						// init the photo frame
						complexDialog->onInit(hwnd);
					}
					else
					{
						return FALSE;
					}
					break;
			}
			break;

		case WM_COMMAND:
			{
				// first try and defer to the active complex dialog to handle the command
				if (complexDialog)
				{
					if (complexDialog->onCommand(hwnd, wParam, lParam))
					{
						return TRUE;
					}
				}

				// otherwise, just continue with the default handling process
				// Process Buttons
				switch (LOWORD(wParam))
				{

				case IDOK:
					memset(buf, NULL, maxChar);

					switch (currentDialogType)
					{
					case DialogFeedback:
						// get the type
						{
							DWORD selType = SendMessage(GetDlgItem(hwnd, IDC_FEEDBACK_TYPE), CB_GETCURSEL, NULL, NULL);
							SendMessage(GetDlgItem(hwnd, IDC_FEEDBACK_TYPE), CB_GETLBTEXT, (WPARAM) selType, (LPARAM) buf);
							dlgManager->setType(QString::fromUtf16((const ushort *) buf));
						}

						// get the email
						memset(buf, 0, maxChar);
						GetWindowText(GetDlgItem(hwnd, IDC_FEEDBACK_EMAIL), buf, maxChar);
						dlgManager->setEmail(QString::fromUtf16((const ushort *) buf));

						// get the message
						memset(buf, 0, maxChar);
						GetWindowText(GetDlgItem(hwnd, IDC_FEEDBACK_MESSAGE), buf, maxChar);
						dlgManager->setText(QString::fromUtf16((const ushort *) buf));
						break;
					case DialogInput:
					case DialogInput2:
					case DialogInputAuth:
						// Get entered Text
						GetWindowText(GetDlgItem(hwnd, IDC_INPUTBOX_TEXT), buf, maxChar);
						dlgManager->setText(QString::fromUtf16((const ushort *) buf));
						break;

					case DialogInputBrowse:
						// Get entered Text
						GetWindowText(GetDlgItem(hwnd, IDC_INPUTBOX_BROWSE_TEXT), buf, maxChar);
						dlgManager->setText(QString::fromUtf16((const ushort *) buf));
						break;

					case DialogThemeConflict:
						dlgManager->setText("merge");
						break;

					case DialogBreakPile:
						dlgManager->setText("break");
						break;

					case DialogTwitterTweet:
						{
							// i18n
							GETTEXTEX text = {0};
							text.cb = maxChar;
							text.flags = GT_USECRLF;
							text.codepage = 1200;		// unicode code page
							text.lpDefaultChar = "?";
							DWORD result = SendMessage(GetDlgItem(hwnd, IDC_TEXTBOX), EM_GETTEXTEX, (WPARAM) &text, (LPARAM) buf);
							dlgManager->setText(QString::fromUtf16((const ushort *) buf));
						}
						break;
					case DialogTwitterLogin:
						{
							GetWindowText(GetDlgItem(hwnd, IDC_TWITTER_LOGIN_USERNAME), buf, maxChar);
							dlgManager->setText(QString::fromUtf16((const ushort *) buf));
							GetWindowText(GetDlgItem(hwnd, IDC_TWITTER_LOGIN_PASSWORD), buf, maxChar);
							dlgManager->setSubText(QString::fromUtf16((const ushort *) buf));
						}
						break;
					case DialogTwitterAuth:
						{
							// Get entered pin
							GetWindowText(GetDlgItem(hwnd, IDC_PIN_TEXT), buf, maxChar);
							dlgManager->setText(QString::fromUtf16((const ushort *) buf));
						}
						break;
					}

					// Kill the Dialog
					EndDialog(hwnd, 1);
					break;

				case IDC_BROWSE:
					// Use the windows Folder Browse Dialog
					if (dlgManager->promptDirListing())
					{
						SetDlgItemText(hwnd, IDC_INPUTBOX_BROWSE_TEXT, (LPCTSTR) dlgManager->getText().utf16());
					}
					break;

				case IDC_UPDATE_SEND_SCREENSHOT:
					// the checkbox is clicked
					dlgManager->setChecked(BST_CHECKED == SendMessage(GetDlgItem(hwnd, IDC_UPDATE_SEND_SCREENSHOT), BM_GETCHECK, 0, 0));
					break;

				case IDNO:
					switch (currentDialogType)
					{
					case DialogAuthorizationChoose:
						dlgManager->setPressedFree(true);
						break;
					case DialogThemeConflict:
						dlgManager->setText("replace");
						break;
					case DialogBreakPile:
						dlgManager->setText("move");
						break;
					default:
						break;
					}
					EndDialog(hwnd, 1);
					break;

				case IDCANCEL:
					// Just Kill the Dialog
					EndDialog(hwnd, 0);
					break;

				case IDC_BUYPROKEY:
					if (currentDialogType == DialogInputAuth)
						launchBumpTopProPage("settingsGoProButton");
					break;

				case IDC_PROXYSETTINGS:
					if (!winOS->GetSettingsAppMessageHandler()->showProxySettings())
						Key_ShowSettingsDialog();
					break;

				case IDC_AUTHORIZEMANUALLY:
					dlgManager->setPressedAuthorizeManually(true);
					GetWindowText(GetDlgItem(hwnd, IDC_INPUTBOX_TEXT), buf, maxChar);
					dlgManager->setText(QString::fromUtf16((const ushort *) buf));
					EndDialog(hwnd, 0);
					return TRUE;

				case IDC_LEARNMORE:
					// launch the bumptop pro website
					launchBumpTopProPage("whyGoProButton");
					break;
				}
			}

			return TRUE;
			break;

		case WM_CLOSE:
			if (complexDialog)
				complexDialog->closeDialog(hwnd, 0);
		case WM_DESTROY:		
			break;

	}

	return FALSE;
}

Win32Dialog * DialogManager::getComplexDialog( DialogType dialogType )
{
	if (complexDialogs.find(dialogType) != complexDialogs.end())
	{
		return complexDialogs[dialogType];
	}
	return NULL;
}

LRESULT DialogManager::richEditCtrlProc( HWND ctrl, UINT msg, WPARAM wParam, LPARAM lParam )
{
	bool updateKeyCount = false;
	switch (msg)
	{
	case WM_KEYDOWN:
		if (wParam == VK_RETURN)
		{
			HWND parent = ::GetParent(ctrl);

			bool isCtrlDown = (GetAsyncKeyState(KeyControl) & 0x8000);
			if (isCtrlDown)
			{
				// finish the dialog
				::SendMessage(parent, WM_COMMAND, MAKEWPARAM(IDOK, EN_UPDATE), (LPARAM) ctrl);
				return FALSE;
			}
		}
		else
		{
			updateKeyCount = true;
		}
		break;
	case WM_KEYUP:
		if (wParam != VK_RETURN)
			updateKeyCount = true;
		break;
	}

	// update the key count
	if (updateKeyCount)
	{
		// call the callback first
		LRESULT result = CallWindowProc(prevRichEditCtrlWndProc, ctrl, msg, wParam, lParam);

		// check if we can update the character count
		HWND parent = ::GetParent(ctrl);
		uint maxChar = 65536;
		TCHAR buf[65536] = {0};

		// i18n
		GETTEXTEX text = {0};
		text.cb = maxChar;
		text.flags = GT_USECRLF;
		text.codepage = 1200;		// unicode code page
		text.lpDefaultChar = "?";
		SendMessage(ctrl, EM_GETTEXTEX, (WPARAM) &text, (LPARAM) buf);
		int charCount = QString::fromUtf16((const ushort *) buf).size();
		if (charCount >= 0)
		{
			// update the count
			QString label = QString("%1 characters left").arg(TWITTER_MAX_CHARS - charCount);
			if (charCount > 140)
				label = QString("Too many characters! (%1 extra)").arg(charCount - TWITTER_MAX_CHARS);
			SetDlgItemText(parent, IDC_TWITTER_CHARCOUNT, (LPCTSTR) label.utf16());
		}

		return result;
	}

	// call the control's own window procedure
	return CallWindowProc(prevRichEditCtrlWndProc, ctrl, msg, wParam, lParam);
}

void DialogManager::setPressedAuthorizeManually( bool val )
{
	pressedAuthorizeManually = val;
}

bool DialogManager::getPressedAuthorizeManually()
{
	return pressedAuthorizeManually;
}

void DialogManager::setPressedFree( bool val )
{
	pressedFree = val;
}

bool DialogManager::getPressedFree()
{
	return pressedFree;
}

void DialogManager::setExpiredChooseVersionDialog(bool val)
{
	expiredChooseVersionDialog = val;
}

bool DialogManager::getExpiredChooseVersionDialog() const
{
	return expiredChooseVersionDialog;
}

bool DialogManager::vistaPromptDialogOverride( DialogType type, bool& successOut )
{
	if (!_pTaskDialogIndirect)
		return false;

	successOut = false;
	switch (type)
	{
	case DialogUpdateBumptop:
		{
			int nButtonPressed                  = 0;
			QString title = QT_TR_NOOP("New Update Available!");
			QString instruction = dlgManager->getCaption();
			QString ok = QT_TR_NOOP("Update Now");
			QString cancel = QT_TR_NOOP("No Thanks");
			TASKDIALOGCONFIG config             = {0};
			TASKDIALOG_BUTTON buttons[2];
			buttons[0].nButtonID = IDOK;
			buttons[0].pszButtonText = (PCWSTR) ok.utf16();
			buttons[1].nButtonID = IDCANCEL;
			buttons[1].pszButtonText = (PCWSTR) cancel.utf16();

			config.cbSize                       = sizeof(config);
			config.hwndParent					= getHasParent() ? winOS->GetWindowsHandle() : NULL;
			config.hInstance                    = winOS->GetInstanceHandle();
			config.dwFlags						= TDF_USE_COMMAND_LINKS | TDF_POSITION_RELATIVE_TO_WINDOW;
			config.dwCommonButtons				= TDCBF_CANCEL_BUTTON;
			config.pszWindowTitle				= (PCWSTR) title.utf16();
			config.pszMainInstruction           = (PCWSTR) instruction.utf16();
			config.pButtons                     = buttons;
			config.cButtons                     = 2;
			config.nDefaultButton				= IDOK;

			_pTaskDialogIndirect(&config, &nButtonPressed, NULL, NULL);
			switch (nButtonPressed)
			{
			case IDOK:
				successOut = true;
			default:
				break; // should never happen
			}
			return true;
		}
	case DialogUpdateGraphicsDrivers:
		{
			int nButtonPressed                  = 0;
			QString title = QT_TR_NOOP("Update your drivers!");
			QString instruction = QT_TR_NOOP("Unable to initialize 3D hardware acceleration");
#if DISABLE_PHONING
			QString content = QT_TR_NOOP("Please update to the latest drivers for your video card and relaunch BumpTop.");
#else
			QString content = QT_TR_NOOP("Please update to the latest drivers for your video card and relaunch BumpTop.\n\n<A HREF=\"http://bumptop.com/support/drivers.php\">More information</A>");
#endif
			QString ok = QT_TR_NOOP("Download latest graphics drivers");
			TASKDIALOGCONFIG config             = {0};
			TASKDIALOG_BUTTON buttons;
				buttons.nButtonID = IDOK;
				buttons.pszButtonText = (PCWSTR) ok.utf16();			

			config.cbSize                       = sizeof(config);
			config.hwndParent					= getHasParent() ? winOS->GetWindowsHandle() : NULL;
			config.hInstance                    = winOS->GetInstanceHandle();
#if DISABLE_PHONING
			config.dwFlags						= TDF_POSITION_RELATIVE_TO_WINDOW;
#else
			config.dwFlags						= TDF_ENABLE_HYPERLINKS | TDF_USE_COMMAND_LINKS | TDF_POSITION_RELATIVE_TO_WINDOW;
#endif
			config.dwCommonButtons				= TDCBF_CLOSE_BUTTON;
			config.pszMainIcon                  = TD_WARNING_ICON;
			config.pszWindowTitle				= (PCWSTR) title.utf16();
			config.pszMainInstruction           = (PCWSTR) instruction.utf16();
			config.pszContent                   = (PCWSTR) content.utf16();
#ifndef DISABLE_PHONING
			config.pButtons                     = &buttons;
			config.cButtons                     = 1;
#endif
			config.nDefaultButton				= IDOK;
			config.pfCallback					= (PFTASKDIALOGCALLBACK) &vistaTaskDialogCallbackProc;

			_pTaskDialogIndirect(&config, &nButtonPressed, NULL, NULL);
			switch (nButtonPressed)
			{
			case IDOK:
				successOut = true;
			default:
				break; // should never happen
			}

			return true;
		}
	case DialogCaptionOnly:
		{
			int nButtonPressed                  = 0;
			TASKDIALOGCONFIG config             = {0};
			config.cbSize                       = sizeof(config);
			config.hwndParent					= getHasParent() ? winOS->GetWindowsHandle() : NULL;
			config.hInstance                    = winOS->GetInstanceHandle();
			config.dwFlags						= TDF_POSITION_RELATIVE_TO_WINDOW;
			config.pszWindowTitle				= (PCWSTR) caption.utf16();
			config.pszMainInstruction           = (PCWSTR) prompt.utf16();

			_pTaskDialogIndirect(&config, &nButtonPressed, NULL, NULL);
			successOut = true;
			return true;
		}
	case DialogFacebookConfirm:
		{
			int nButtonPressed                  = 0;
			QString title = QT_TR_NOOP("BumpTop");
			QString instruction = QT_TR_NOOP("Upload to Facebook?");
			QString content = QT_TR_NOOP("Do you want to upload and share these photos on your Facebook account?");
			QString ok = QT_TR_NOOP("Upload these photos to Facebook");
			TASKDIALOGCONFIG config             = {0};			
			TASKDIALOG_BUTTON buttons;
				buttons.nButtonID = IDOK;
				buttons.pszButtonText = (PCWSTR) ok.utf16();
			
			config.cbSize                       = sizeof(config);
			config.hwndParent					= getHasParent() ? winOS->GetWindowsHandle() : NULL;
			config.hInstance                    = winOS->GetInstanceHandle();
			config.dwFlags						= TDF_USE_COMMAND_LINKS | TDF_POSITION_RELATIVE_TO_WINDOW;
			config.dwCommonButtons				= TDCBF_CANCEL_BUTTON;
			config.pszWindowTitle				= (PCWSTR) title.utf16();
			config.pszMainInstruction           = (PCWSTR) instruction.utf16();
			config.pszContent                   = (PCWSTR) content.utf16();
			config.pButtons                     = &buttons;
			config.cButtons                     = 1;
			config.nDefaultButton				= IDOK;

			_pTaskDialogIndirect(&config, &nButtonPressed, NULL, NULL);
			switch (nButtonPressed)
			{
			case IDOK:
				successOut = true;
			default:
				break; // should never happen
			}
			return true;
		}
	case DialogFacebookAuth:
		{
			int nButtonPressed                  = 0;
			QString title = QT_TR_NOOP("BumpTop");
			QString instruction = QT_TR_NOOP("Setup Facebook Photo integration");
			QString content = QT_TR_NOOP("We are now redirecting you to the BumpTop's Facebook application login page.");
			QString ok = QT_TR_NOOP("Start uploading photos to Facebook!\nEnsure that you have approved BumpTop's\nFacebook application before continuing");
			TASKDIALOGCONFIG config             = {0};
			TASKDIALOG_BUTTON buttons;
				buttons.nButtonID = IDOK;
				buttons.pszButtonText = (PCWSTR) ok.utf16();			

			config.cbSize                       = sizeof(config);
			config.hwndParent					= getHasParent() ? winOS->GetWindowsHandle() : NULL;
			config.hInstance                    = winOS->GetInstanceHandle();
			config.dwFlags						= TDF_USE_COMMAND_LINKS | TDF_POSITION_RELATIVE_TO_WINDOW;
			config.dwCommonButtons				= TDCBF_CANCEL_BUTTON;
			config.pszWindowTitle				= (PCWSTR) title.utf16();
			config.pszMainInstruction           = (PCWSTR) instruction.utf16();
			config.pszContent                   = (PCWSTR) content.utf16();
			config.pButtons                     = &buttons;
			config.cButtons                     = 1;
			config.nDefaultButton				= IDOK;

			_pTaskDialogIndirect(&config, &nButtonPressed, NULL, NULL);
			switch (nButtonPressed)
			{
			case IDOK:
				successOut = true;
			default:
				break; // should never happen
			}
			return true;
		}
	case DialogDeauthorizeConfirm:
		{
			int nButtonPressed                  = 0;
			QString title = QT_TR_NOOP("BumpTop");
			QString instruction = QT_TR_NOOP("Deauthorize BumpTop Pro key?");
			QString content = QT_TR_NOOP("Deauthorizing your Pro key will disable Pro-specific features on this copy of BumpTop on next startup.  This also allows another copy of BumpTop Free to be authorized in the future.");
			QString ok = QT_TR_NOOP("Deauthorize my BumpTop Pro key\nThis will deauthorize Pro features on this\ncopy of BumpTop");
			TASKDIALOGCONFIG config             = {0};
			TASKDIALOG_BUTTON buttons;
				buttons.nButtonID = IDOK;
				buttons.pszButtonText = (PCWSTR) ok.utf16();

			config.cbSize                       = sizeof(config);
			config.hwndParent					= getHasParent() ? winOS->GetWindowsHandle() : NULL;
			config.hInstance                    = winOS->GetInstanceHandle();
			config.dwFlags						= TDF_USE_COMMAND_LINKS | TDF_POSITION_RELATIVE_TO_WINDOW;
			config.dwCommonButtons				= TDCBF_CANCEL_BUTTON;
			config.pszWindowTitle				= (PCWSTR) title.utf16();
			config.pszMainInstruction           = (PCWSTR) instruction.utf16();
			config.pszContent                   = (PCWSTR) content.utf16();
			config.pButtons                     = &buttons;
			config.cButtons                     = 1;
			config.nDefaultButton				= IDOK;

			_pTaskDialogIndirect(&config, &nButtonPressed, NULL, NULL);
			switch (nButtonPressed)
			{
			case IDOK:
				successOut = true;
			default:
				break; // should never happen
			}
			return true;
		}
	case DialogChangeIcon:
		{
			int nButtonPressed                  = 0;
			QString title = QT_TR_NOOP("BumpTop");
			QString instruction = QT_TR_NOOP("Override file icon?");
			QString ok = QT_TR_NOOP("Change icon...\nSelect the image to override this file's icon with");
			QString cancel = QT_TR_NOOP("Restore default icon\nRestore the original file's icon");
			TASKDIALOGCONFIG config             = {0};
			TASKDIALOG_BUTTON buttons[2];
				buttons[0].nButtonID = IDOK;
				buttons[0].pszButtonText = (PCWSTR) ok.utf16();
				buttons[1].nButtonID = IDCANCEL;
				buttons[1].pszButtonText = (PCWSTR) cancel.utf16();

			config.cbSize                       = sizeof(config);
			config.hwndParent					= getHasParent() ? winOS->GetWindowsHandle() : NULL;
			config.hInstance                    = winOS->GetInstanceHandle();
			config.dwFlags						= TDF_USE_COMMAND_LINKS | TDF_POSITION_RELATIVE_TO_WINDOW;
			config.dwCommonButtons				= TDCBF_CANCEL_BUTTON;
			config.pszWindowTitle				= (PCWSTR) title.utf16();
			config.pszMainInstruction           = (PCWSTR) instruction.utf16();
			config.pButtons                     = buttons;
			config.cButtons                     = 2;
			config.nDefaultButton				= IDOK;

			_pTaskDialogIndirect(&config, &nButtonPressed, NULL, NULL);
			switch (nButtonPressed)
			{
			case IDOK:
				successOut = true;
			default:
				break; // should never happen
			}
			return true;
		}
	case DialogBreakPile:
		{
			int nButtonPressed                  = 0;
			QString title = QT_TR_NOOP("BumpTop");
			QString instruction = QT_TR_NOOP("Break this Folder Pile?");
			QString content = QT_TR_NOOP("Breaking this Folder Pile will move all files from the folder onto the desktop.");
			QString ok = QT_TR_NOOP("Break Pile and Move files\nMove all files out of the pile and onto the desktop.");
			TASKDIALOGCONFIG config             = {0};
			TASKDIALOG_BUTTON buttons;
				buttons.nButtonID = IDOK;
				buttons.pszButtonText = (PCWSTR) ok.utf16();
			
			config.cbSize                       = sizeof(config);
			config.hwndParent					= getHasParent() ? winOS->GetWindowsHandle() : NULL;
			config.hInstance                    = winOS->GetInstanceHandle();
			config.dwFlags						= TDF_USE_COMMAND_LINKS | TDF_POSITION_RELATIVE_TO_WINDOW;
			config.dwCommonButtons				= TDCBF_CANCEL_BUTTON;
			config.pszWindowTitle				= (PCWSTR) title.utf16();
			config.pszMainInstruction           = (PCWSTR) instruction.utf16();
			config.pszContent					= (PCWSTR) content.utf16();
			config.pButtons                     = &buttons;
			config.cButtons                     = 1;
			config.nDefaultButton				= IDOK;

			_pTaskDialogIndirect(&config, &nButtonPressed, NULL, NULL);
			switch (nButtonPressed)
			{
			case IDOK:
				successOut = true;
			default:
				break; // should never happen
			}
			return true;
		}
	case DialogAuthorizationFailed:
		{
			int nButtonPressed                  = 0;
			QString title = QT_TR_NOOP("BumpTop");
			QString instruction = QT_TR_NOOP("Altered BumpTop executable detected!");
			QString content = QT_TR_NOOP("You may be running an altered version of BumpTop!\n\nPlease consider purchasing a BumpTop Pro license so that we can continue innovating on the best 3D physics-based desktop available.");
			QString ok = QT_TR_NOOP("Buy BumpTop Pro License\nEnjoy additional features and support by purchasing\na Pro license!");
			QString cancel = QT_TR_NOOP("Continue using BumpTop Free");
			TASKDIALOGCONFIG config             = {0};
			TASKDIALOG_BUTTON buttons[2];
				buttons[0].nButtonID = IDOK;
				buttons[0].pszButtonText = (PCWSTR) ok.utf16();
				buttons[1].nButtonID = IDCANCEL;
				buttons[1].pszButtonText = (PCWSTR) cancel.utf16();
			
			config.cbSize                       = sizeof(config);
			config.hwndParent					= getHasParent() ? winOS->GetWindowsHandle() : NULL;
			config.hInstance                    = winOS->GetInstanceHandle();
			config.dwFlags						= TDF_USE_COMMAND_LINKS | TDF_POSITION_RELATIVE_TO_WINDOW;
			config.pszMainIcon                  = TD_WARNING_ICON;
			config.dwCommonButtons				= TDCBF_CLOSE_BUTTON;
			config.pszWindowTitle				= (PCWSTR) title.utf16();
			config.pszMainInstruction           = (PCWSTR) instruction.utf16();
			config.pszContent					= (PCWSTR) content.utf16();
			config.pButtons                     = buttons;
			config.cButtons                     = 2;
			config.nDefaultButton				= IDOK;

			_pTaskDialogIndirect(&config, &nButtonPressed, NULL, NULL);
			switch (nButtonPressed)
			{
			case IDOK:
				successOut = true;
			default:
				break; // should never happen
			}
			return true;
		}
	default:
		break;
	}
	return false;
}

HRESULT CALLBACK DialogManager::vistaTaskDialogCallbackProc( HWND hwnd, UINT uNotification, WPARAM wParam, LPARAM lParam, LONG_PTR dwRefData )
{
	switch (dlgManager->getDialogType())
	{
	case DialogUpdateGraphicsDrivers:
		if (uNotification == TDN_HYPERLINK_CLICKED)
		{
			QString url = QString::fromUtf16((ushort *) lParam);
			fsManager->launchFileAsync(url);
		}
		break;
	default:
		break;
	}
	return S_OK;
}