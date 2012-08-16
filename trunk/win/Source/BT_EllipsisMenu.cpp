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
#include "BT_EllipsisMenu.h"
#include "BT_FileSystemManager.h"
#include "BT_FileSystemPile.h"
#include "BT_MarkingMenu.h"
#include "BT_MenuAction.h"
#include "BT_SceneManager.h"
#include "BT_Selection.h"
#include "BT_Util.h"
#include "BT_WindowsOS.h"

// static initializations
HMENU ContextMenu::_currentHmenu = NULL;
HMENU ContextMenu::_prevHmenu = NULL;
QPolygon ContextMenu::_currentHmenuRegion;
bool ContextMenu::_unionMenuRectOnNextEnterIdle = false;
bool ContextMenu::_enableTracking = false;

ContextMenu::ContextMenu()
{
	contextMenu = NULL;
	shellFolder = NULL;
	pidlArray = NULL;
	menuType = 0;
	numItems = 0;
} 

ContextMenu::~ContextMenu()
{
	// Safely release memory
	SAFE_DELETE_ARRAY(pidlArray);
	SAFE_RELEASE(shellFolder);
}

// This functions determines which version of IContextMenu is available for 
// those objects(always the highest one) and returns that interface.
LRESULT ContextMenu::HookWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	/* debug
	consoleWrite(QString("%1 - %2\n").arg(GetMessageName(message)).arg((int)hWnd));
	consoleWrite("\n");
	*/

	switch(message)
	{ 
	case WM_MENUCHAR:	// only supported by IContextMenu3
		if (GLOBAL(g_IContext3))
		{
			LRESULT lResult = 0;
			GLOBAL(g_IContext3)->HandleMenuMsg2(message, wParam, lParam, &lResult);
			return lResult;
		}
		break;

	case WM_DRAWITEM:
	case WM_MEASUREITEM:
		if (wParam) 
			break; // if wParam != 0 then the message is not menu-related

	case WM_INITMENUPOPUP:
		{
			_unionMenuRectOnNextEnterIdle = true;
			_prevHmenu = (HMENU) wParam;
			if (GLOBAL(g_IContext2))
			{
				return GLOBAL(g_IContext2)->HandleMenuMsg(message, wParam, lParam);
			}
			else if (GLOBAL(g_IContext3))
			{
				LRESULT lResult = 0;
				GLOBAL(g_IContext3)->HandleMenuMsg2(message, wParam, lParam, &lResult);
				return lResult;
			}		
		}
		break;

	case WM_ENTERIDLE:
		if (_unionMenuRectOnNextEnterIdle)
		{
			if (_currentHmenu && _enableTracking &&
				winOS->IsWindowsVersionGreaterThanOrEqualTo(Windows7))
			{
				// union the rects
				RECT r = {0};
				RECT tmp = {0};
				HMENU hmenu = (HMENU) _prevHmenu;
				for (int i = 0; i < GetMenuItemCount(hmenu); i++)
				{
					GetMenuItemRect(NULL, hmenu, i, &tmp);
					UnionRect(&r, &r, &tmp);
				}
				InflateRect(&r, 10, 10);	// buffer
				_currentHmenuRegion = _currentHmenuRegion.united(QPolygon(QRect(QPoint(r.left, r.top), QPoint(r.right, r.bottom))));
				/* debug
				QRect r2 = _currentHmenuRegion.boundingRect();
				consoleWrite(QString("Union: %1 %2 %3 %4\n").arg(r2.left()).arg(r2.top()).arg(r2.right()).arg(r2.bottom()));
				*/
				_prevHmenu = NULL;
			}
			_unionMenuRectOnNextEnterIdle = false;
		}
		break;

	case WM_INPUT:
		{
			if (_currentHmenu && _enableTracking &&
				winOS->IsWindowsVersionGreaterThanOrEqualTo(Windows7))
			{
				if (!_unionMenuRectOnNextEnterIdle)
				{
					// end the menu if the mouse is not inside the menu bounds
					POINT p;
					GetCursorPos(&p);
					// if (!PtInRect(&_currentHmenuRegion, p))
					if (!_currentHmenuRegion.containsPoint(QPoint(p.x, p.y), Qt::WindingFill))
					{
						if (!markingMenu->isHittingMenuWithCallback(Vec3(p.x, p.y, 0.0f), Key_MoreOptions))
						{
						 	EndMenu();
						}
					}
				}

				/* debug
				POINT p;
				GetCursorPos(&p);
				consoleWrite(QString("P: %1 %2\n").arg(p.x).arg(p.y));
				*/
			}
			/* debug
			evtManager->processEvents();
			*/
		}
		break;

	case WM_UNINITMENUPOPUP:
		if (_currentHmenu && _enableTracking &&
			winOS->IsWindowsVersionGreaterThanOrEqualTo(Windows7))
		{
			// intersection of the rects
			RECT r = {0};
			RECT tmp = {0};
			HMENU hmenu = (HMENU) wParam;
			for (int i = 0; i < GetMenuItemCount(hmenu); i++)
			{
				GetMenuItemRect(NULL, hmenu, i, &tmp);
				UnionRect(&r, &r, &tmp);
			}
			InflateRect(&r, 10, 10);	// buffer
			_currentHmenuRegion = _currentHmenuRegion.subtracted(QPolygon(QRect(QPoint(r.left, r.top), QPoint(r.right, r.bottom))));
			/* debug 
			QRect r2 = _currentHmenuRegion.boundingRect();
			consoleWrite(QString("Intersect: %1 %2 %3 %4\n").arg(r2.left()).arg(r2.top()).arg(r2.right()).arg(r2.bottom()));
			*/
		}
		break;

	case WM_CONTEXTMENU:
		// ignore this message for now since we know the context menu has already
		// been shown
		return 0;

	default:
		break;
	}

	return ::CallWindowProc(GLOBAL(OldWndProc) , hWnd, message, wParam, lParam);
}

// Displays an explorer-style right-click context menu
// displayNativeMenu is used to indicate whether the default items from 
// the explorer context menu should be appended to the menu.
// pos_flags is used to determine the anchoring point of the submenu (TPM_CENTERALIGN/etc)
void ContextMenu::showMenu(vector<MenuAction *> &list, POINT p, bool displayNativeMenu, uint pos_flags)
{
	LOG_LINE_REACHED();
	
	HMENU menu;
	vector<FileSystemActor *> fsList = getSelectedItems();

	menuType = 0;

	// Check if we need a context menu for items or the desktop
	if (!fsList.empty())
	{
		// ensure that we want to display the menu for all the items
		for (int i = 0; i < fsList.size() && displayNativeMenu; ++i)
		{
			displayNativeMenu = displayNativeMenu && fsList[i]->allowNativeContextMenu();
		}

		// Other loose Items
		if (displayNativeMenu)
			menu = getMenu(fsList);
		else
			menu = CreatePopupMenu();
	}
	else if ((sel->getSize() == 0) && (displayNativeMenu))
	{
		// User clicked on the background (Working Dir)
		menu = getMenu(native(scnManager->getWorkingDirectory()));
	}
	else
	{
		// Soft Piles and Logical Actors
		menu = CreatePopupMenu();
	}

	// Append the menu action items to our list
	appendMenuActions(menu, list);

	
	removeAdjacentMenuSeparators(menu);

	// Show the menu
	invokeMenu(menu, p, pos_flags);

	// Delete the menu
	DestroyMenu(menu);
}

BOOL tryTrackPopupMenu(
    __in HMENU hMenu,
    __in UINT uFlags,
    __in int x,
    __in int y,
    __in int nReserved,
    __in HWND hWnd,
    __in_opt CONST RECT *prcRect)
{
	__try
	{
		return TrackPopupMenu(hMenu, uFlags, x, y, nReserved, hWnd, prcRect);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		logger->logException(__FILE__, __LINE__, GetExceptionCode());
		return FALSE;
	}
}

// Draws the explorer-style context menu to the screen
void ContextMenu::invokeMenu(HMENU menu, POINT pt, uint pos_flags)
{

	uint idCommand;

	// Sanity Check
	if (!menu) return;

	// Subclass window to handle menu related messages in ContextMenu 
	if (menuType > 1)
	{
		if (menuType == 2)
		{
			contextMenu->QueryInterface(IID_IContextMenu2, (LPVOID *) &GLOBAL(g_IContext2));
		}else{ 
			contextMenu->QueryInterface(IID_IContextMenu3, (LPVOID *) &GLOBAL(g_IContext3));
		}
	}

	// Invoke the menu
	GLOBAL(OldWndProc) = (WNDPROC) SetWindowLong(winOS->GetWindowsHandle(), GWL_WNDPROC,(DWORD) HookWndProc);
	_currentHmenu = menu;
	_prevHmenu = NULL;
	_currentHmenuRegion = QPolygon();
	_unionMenuRectOnNextEnterIdle = false;
	idCommand = tryTrackPopupMenu(menu, TPM_RETURNCMD | pos_flags | TPM_VERPOSANIMATION, pt.x, pt.y, NULL, winOS->GetWindowsHandle(), NULL);
	_unionMenuRectOnNextEnterIdle = false;
	_prevHmenu = NULL;
	_currentHmenu = NULL;
	SetWindowLong(winOS->GetWindowsHandle(), GWL_WNDPROC,(DWORD) GLOBAL(OldWndProc));

	// See if there was a command chosen
	if (idCommand >= MIN_ID && idCommand <= MAX_ID)
	{
		invokeCommand(contextMenu, idCommand - MIN_ID);
	}

	// destroy the marking menu since we have made a selection
	if (winOS->IsWindowsVersionGreaterThanOrEqualTo(Windows7))
		markingMenu->destroy();

	SAFE_RELEASE(GLOBAL(g_IContext2));
	SAFE_RELEASE(GLOBAL(g_IContext3));

	// Release the context Menu when were done
	SAFE_RELEASE(contextMenu);
}

void ContextMenu::invokeCommand(LPCONTEXTMENU pContextMenu, UINT idCommand)
{
	CMINVOKECOMMANDINFO cmi = {0};
	HRESULT hr;
	if (idCommand < DISPLAY_SCREEN_RESOLUTION_ID)
	{
		// Set it all up
		cmi.cbSize = sizeof(CMINVOKECOMMANDINFO);
		cmi.lpVerb =(LPSTR) MAKEINTRESOURCE(idCommand);
		cmi.nShow = SW_SHOWNORMAL;

		// See how many shortcuts are currently on the desktop
		int numShortcutsBeforeInvokeCommand = scnManager->getFileSystemActors(DeadLink, false).size();

		// Invoke the specific command
		hr = pContextMenu->InvokeCommand(&cmi);
		
		// Update the file system watcher to see if any new files were created
		fsManager->update();

		// Check if a new shortcut has been added to the desktop
		vector<FileSystemActor *> shortcutsAfterInvokingCommand = scnManager->getFileSystemActors(DeadLink, false);

		// If a new shortcut was added launch "New Shortcut" prompt dialog
		if (winOS->IsWindowsVersionGreaterThanOrEqualTo(WindowsVista) && 
				shortcutsAfterInvokingCommand.size() > numShortcutsBeforeInvokeCommand)
		{
			// Use the last item added to the list of getFileSystemActors
			int index = shortcutsAfterInvokingCommand.size() - 1;
			QString filePath = shortcutsAfterInvokingCommand[index]->getFullPath();

			// Create a path without spaces
			// The new shortcut prompt requires a path to the new shortcut file, and the path 
			// cannot contain spaces. Enclosing the path in ""'s doesn't work either
			// So instead we will call GetShortPathName to get a short path name
			// that we can pass to rundll32.exe
			QString shortFilePathQS("");
			winOS->GetShortPathName(filePath, shortFilePathQS);

			// Launch rundll32.exe to bring up the new shortcut prompt	
			fsManager->launchFile("rundll32.exe", "appwiz.cpl,NewLinkHere " + native(shortFilePathQS), "", false, false);
		}
	}else{
		// Handle the display properties
		if (native(scnManager->getWorkingDirectory()) == winOS->GetSystemPath(DesktopDirectory) &&
			idCommand == DISPLAY_PROPERTIES_ID)
		{
			if (winOS->IsWindowsVersion(WindowsXP))
			{
				// Launch the display properties
				fsManager->launchFileAsync("Rundll32.exe", "shell32.dll,Control_RunDLL Desk.cpl", "", false, false);
			}
			else if (winOS->IsWindowsVersionGreaterThanOrEqualTo(WindowsVista))
			{
				// Launch the personalization applet
				// http://msdn.microsoft.com/en-us/library/cc144191(VS.85).aspx
				fsManager->launchFileAsync("control.exe", "/name Microsoft.Personalization", "", false, false);
			}

			if (!_onInvokeCommandHandler.empty())
				_onInvokeCommandHandler();

			return;
		}
		else if (idCommand == DISPLAY_GADGETS_ID)
		{
			fsManager->launchFileAsync("sidebar.exe", " /showGadgets", "", false, false);
		}
		else if (idCommand == DISPLAY_SCREEN_RESOLUTION_ID)
		{
			fsManager->launchFileAsync("control.exe", "desk.cpl,,@settings", "", false, false);	
		}

		// Handle Menu Actions
		if (idCommand >= MENU_ACTION_ID_MIN)
		{
			actionList[idCommand]->execute();
		}
	}

	if (!_onInvokeCommandHandler.empty())
		_onInvokeCommandHandler();
}

void ContextMenu::removeBumpThisFolder(HMENU menu)
{
	removeMenuItem(menu, "&Bump This Folder");
}

void ContextMenu::removeMenuItem( HMENU menu, QString itemName )
{
	int i;

	// Sanity check
	if (!menu) return;

	// Find the bump this folder item
	i = getMenuItemIndex(menu, itemName);
	if (i != -1)
	{
		// If the menu name matches the BT shell ext context menu string
		// then delete it
		DeleteMenu(menu, i, MF_BYPOSITION);
	}

}


void ContextMenu::removeAdjacentMenuSeparators(HMENU menu)
{
	// Loop through each item in menu
	bool prevItemIsSeparator = false;


	for (int i = 0; i < GetMenuItemCount(menu);)
	{
		bool currentItemIsSeparator = (0 != (GetMenuState(menu, i, MF_BYPOSITION) & MF_SEPARATOR));
		// If this is a separator, and the previous item was a 
		// separator, remove this item
		if (prevItemIsSeparator && currentItemIsSeparator)
		{
			DeleteMenu(menu, i, MF_BYPOSITION);
		}
		else
		{
			i++;
		}
		prevItemIsSeparator = currentItemIsSeparator;

	}

}

HMENU ContextMenu::createMenuFromPIDL()
{

	HMENU hMenu = NULL;

	// Sanity Check
	if (contextMenu) SAFE_RELEASE(contextMenu);

	// Can we get the Menu from this
	if (getContextMenu((void **) &contextMenu, menuType))
	{
		hMenu = CreatePopupMenu();
	
		// Apply the Explorer menu to our menu
		contextMenu->QueryContextMenu(hMenu, GetMenuItemCount(hMenu), MIN_ID, MAX_ID, CMF_EXPLORE);

		// Remove the "Bump This Folder" option
		removeBumpThisFolder(hMenu);
		removeMenuItem(hMenu, "&View");
		removeMenuItem(hMenu, "S&ort By");
		removeMenuItem(hMenu, "Grou&p By");
		removeMenuItem(hMenu, "Stac&k By");
		removeMenuItem(hMenu, "R&efresh");
		removeMenuItem(hMenu, "Arrange &Icons By");
	}

	return hMenu;
}

void ContextMenu::appendMenuActions(HMENU hMenu, vector<MenuAction *> menuActionList)
{
	bool useSeparator;

	actionList.clear();
	useSeparator = GetMenuItemCount(hMenu) == 0 ? false : true;

	//Unique id for menu elements
	uint menuId = 0;

	for (uint i = 0; i < menuActionList.size(); i++)
	{
		QString pStr = menuActionList[i]->getLabel();

		if(menuActionList[i]->isSubMenu()) {

			HMENU sub = CreatePopupMenu();

			MENUITEMINFO itemInfo;
				itemInfo.cbSize = sizeof(MENUITEMINFO);
				itemInfo.fMask = MIIM_ID | MIIM_STRING | MIIM_SUBMENU;
				itemInfo.wID = MENU_ACTION_ID_MIN + menuId + 1;
				itemInfo.fType = MFT_STRING;
				itemInfo.cch = pStr.size();
				itemInfo.dwTypeData = (LPWSTR) pStr.utf16();
				itemInfo.hSubMenu = sub;
				InsertMenuItem(hMenu, menuId, MF_BYPOSITION, &itemInfo);
				actionList[MENU_ACTION_ID_MIN + menuId] = menuActionList[i];
				menuId++;

			for(uint x = 0; x < menuActionList[i]->subMenu.size(); x++) {
				MENUITEMINFO itemInfo;
					itemInfo.cbSize = sizeof(MENUITEMINFO);
					itemInfo.fMask = MIIM_ID | MIIM_STRING;
					if(menuActionList[i]->subMenu[x].isCheckBox()) {
						//For checkable items
						itemInfo.fMask |= MIIM_STATE;

						if(menuActionList[i]->subMenu[x].checkConditionMet())
							itemInfo.fState = MFS_CHECKED;
						else
							itemInfo.fState = MFS_UNCHECKED;
					}
					itemInfo.wID = MENU_ACTION_ID_MIN + menuId + 1;
					itemInfo.fType = MFT_STRING;
					itemInfo.cch = menuActionList[i]->subMenu[x].getLabel().size();
					itemInfo.dwTypeData = (LPWSTR) menuActionList[i]->subMenu[x].getLabel().utf16();
				InsertMenuItem(sub, menuId, MF_BYPOSITION, &itemInfo);
				actionList[MENU_ACTION_ID_MIN + menuId] = &menuActionList[i]->subMenu[x];
				menuId++;
			}
			continue;
		}

		if (menuActionList[i]->getHotKey().subKeys.key > 0)
		{
			QString sStr = menuActionList[i]->getHotKey().toString();

			// Align everything to the side, using a tab
			pStr.append("\t");
			pStr.append("(&");
			pStr.append(sStr);
			pStr.append(")");
		}
		// Plug it into the menu
		MENUITEMINFO itemInfo;
			itemInfo.cbSize = sizeof(MENUITEMINFO);
			itemInfo.fMask = MIIM_ID | MIIM_STRING;
			if(menuActionList[i]->isCheckBox()) {
				//For checkable items
				itemInfo.fMask |= MIIM_STATE;
				//checkConditionMet is a boolean callback to the function that determines
				//if this item should be checked when the menu is rendered
				if(menuActionList[i]->checkConditionMet())
					itemInfo.fState = MFS_CHECKED;
				else
					itemInfo.fState = MFS_UNCHECKED;
			}
			itemInfo.wID = MENU_ACTION_ID_MIN + menuId + 1;
			itemInfo.fType = MFT_STRING;
			itemInfo.cch = pStr.size();
			itemInfo.dwTypeData = (LPWSTR) pStr.utf16();
		InsertMenuItem(hMenu, menuId, MF_BYPOSITION, &itemInfo);
		actionList[MENU_ACTION_ID_MIN + menuId] = menuActionList[i];
		menuId++;
	}

	if (useSeparator)
	{
		// Add in a separator
		// menuActionList.size() was originally used to indicate the ID of the separator
		// but with the addition of the submenu logic, it's not reliable so menuId is used
		// in its place
		InsertMenu(hMenu, menuId, MF_BYPOSITION | MF_SEPARATOR, NULL, L"");
		menuId++;
	}
}

vector<FileSystemActor *> ContextMenu::getSelectedItems()
{
	vector<FileSystemActor *> fsList;
	vector<BumpObject *> objList = sel->getBumpObjects();

	// Grab only the filesystem actors or hard piles from the list
	for (uint i = 0; i < objList.size(); i++)
	{
		if (objList[i]->getObjectType() == ObjectType(BumpActor, FileSystem))
		{	
			fsList.push_back((FileSystemActor *) objList[i]);
		}else if (objList[i]->getObjectType() == ObjectType(BumpPile, HardPile))
		{
			FileSystemPile *fsPile = (FileSystemPile *) objList[i];
			fsList.push_back(fsPile->getOwner());
		}else if (objList[i]->getObjectType() == ObjectType(BumpPile, SoftPile))
		{
			Pile *pile = (Pile *) objList[i];

			for (uint j = 0; j < pile->getNumItems(); j++)
			{
				if ((*pile)[j]->getObjectType() == ObjectType(BumpActor, FileSystem))
				{
					fsList.push_back((FileSystemActor *) (*pile)[j]);
				}
			}
		}
	}

	return fsList;
}
BOOL ContextMenu::getContextMenu(void **ppContextMenu, int &iMenuType)
{
	*ppContextMenu = NULL;
	LPCONTEXTMENU icm1 = NULL;

	// First we retrieve the normal IContextMenu interface(every object should have it)
	if (shellFolder) 
	{	
		if (!numItems)
		{
			// get IContextMenu of the shellFolder itself through its parent 
			LPITEMIDLIST pidl = winOS->GetAbsolutePidlFromAbsFilePath(native(scnManager->getWorkingDirectory()));
			LPSHELLFOLDER parentFolder = NULL;
			LPITEMIDLIST lastPidl = NULL;
			SHBindToParent(pidl, IID_IShellFolder, (void**)&parentFolder, (LPCITEMIDLIST*)&lastPidl);
			parentFolder->GetUIObjectOf(NULL, 1, (LPCITEMIDLIST*)&lastPidl, IID_IContextMenu, NULL, (void**)&icm1);
			parentFolder->Release();
			ILFree(pidl);
		}
		else // try and use the old way to get the menu items
			shellFolder->GetUIObjectOf(NULL, numItems, (LPCITEMIDLIST *) pidlArray, IID_IContextMenu, NULL, (LPVOID *) &icm1);	

		if (icm1)
		{	
			// Since we got an IContextMenu interface we can now obtain the higher version interfaces via that
			if (icm1->QueryInterface(IID_IContextMenu3, ppContextMenu) == NOERROR)
			{
				iMenuType = 3;
			}else if (icm1->QueryInterface(IID_IContextMenu2, ppContextMenu) == NOERROR)
			{
				iMenuType = 2;
			}

			if (*ppContextMenu) 
			{
				// We can now release version 1 interface, cause we got a higher one
				icm1->Release(); 
			}else{
				// Redirect to version 1 interface	
				iMenuType = 1;
				*ppContextMenu = icm1;
			}
		}else{
			return FALSE;
		}
	}else{
		return FALSE;
	}	

	return TRUE;
}

int ContextMenu::getMenuItemIndex(HMENU menu, QString menuString)
{
	// Loop through each item and see which item matches the query
	for (int i = 0; i < GetMenuItemCount(menu); i++)
	{
		TCHAR menuName[MAX_PATH + 1];
		GetMenuString(menu, i, menuName, MAX_PATH, MF_BYPOSITION);

		// Compare the strings
		if (menuString.compare(QString::fromUtf16((const ushort *) menuName), Qt::CaseInsensitive) == 0)
			return i;
	}

	return -1;
}

HMENU ContextMenu::getMenu(vector<FileSystemActor *> objList)
{
	numItems = objList.size();
	HMENU menu = NULL;
	LPITEMIDLIST pidlItem = NULL;

	pidlArray = new LPITEMIDLIST[numItems + 1];	

	// If its in ShExt mode, use the desktop click as a click on the folder
	if (scnManager->isShellExtension && objList.empty())
	{
		QString wdStr = native(scnManager->getWorkingDirectory());
		if (wdStr.size() > 3)
		{
			shellFolder = winOS->GetShellFolderFromAbsDirPath(wdStr);		
		}
		else
		{
			return getMenu(wdStr);
		}
	}
	else
	{
		shellFolder = winOS->GetShellFolderFromAbsDirPath(native(parent(objList[0]->getFullPath())));
	}

	// Create an ID list of files that are being selected
	set<QString> sourceDirs;
	for (uint i = 0; i < objList.size(); i++)
	{
		if (objList[i]->isFileSystemType(Virtual))
		{ 
			pidlArray[i] = winOS->GetPidlFromName(winOS->GetIconTypeFromFileName(objList[i]->getFullPath()));
			SAFE_RELEASE(shellFolder);
			sourceDirs.insert(native(scnManager->getWorkingDirectory()));
		}else{
			pidlArray[i] = winOS->GetRelativePidlFromAbsFilePath(objList[i]->getFullPath());		
			sourceDirs.insert(native(parent(objList[i]->getFullPath())));	
		}
	}

	assert(!sourceDirs.empty());
	
	// If we don't have a shell folder selected, find one (Virtual Folders Only)
	if (!shellFolder)
	{
		SHBindToParent((LPCITEMIDLIST) pidlArray[0], IID_IShellFolder2, (void **) &shellFolder, (LPCITEMIDLIST *) &pidlItem);	
	}

	// Construct the menu
	menu = createMenuFromPIDL();

	// remove the last item if it is a separator
	removeLastSeparator(menu);

	return menu;
}

HMENU ContextMenu::getMenu(QString dir)
{
	HMENU menu = NULL;

	shellFolder = winOS->GetShellFolderFromAbsDirPath(dir);
	// Construct a menu using the dir as a shell Folder (Returns NEW submenu)

	menu = createMenuFromPIDL();	

	// remove the last item if it is a separator
	removeLastSeparator(menu);

	// Replace the Properties item on the menu with Display Options
	if (native(scnManager->getWorkingDirectory()) == winOS->GetSystemPath(DesktopDirectory))
	{
		// replace the properties in xp
		int i = getMenuItemIndex(menu, QT_TR_NOOP("P&roperties"));
		if (i > -1)
		{
			RemoveMenu(menu, i, MF_BYPOSITION);
			if (winOS->IsWindowsVersion(WindowsXP))
			{
				AppendMenu(menu, MF_STRING, DISPLAY_PROPERTIES_ID + 1, (LPCWSTR) QT_TR_NOOP("P&roperties").utf16());
			}			
		}

		// add the additional items in vista+ to the end of the menu
		if (winOS->IsWindowsVersionGreaterThanOrEqualTo(WindowsVista))
		{
			AppendMenu(menu, MF_STRING, DISPLAY_SCREEN_RESOLUTION_ID + 1, (LPCWSTR) QT_TR_NOOP("&Screen Resolution").utf16());
			AppendMenu(menu, MF_STRING, DISPLAY_GADGETS_ID + 1, (LPCWSTR) QT_TR_NOOP("&Gadgets").utf16());
			AppendMenu(menu, MF_STRING, DISPLAY_PROPERTIES_ID + 1, (LPCWSTR) QT_TR_NOOP("&Personalize").utf16());

			/*
			HICON hIcon = (HICON) LoadImage(winOS->GetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON2), IMAGE_ICON, 16, 16, 0);
			HBITMAP hBitmap;
			ICONINFO iconinfo = {0};
			GetIconInfo(hIcon, &iconinfo);
			DestroyIcon(hIcon);
			hBitmap = iconinfo.hbmColor;
			SetMenuItemBitmaps(menu, i, MF_BYPOSITION, hBitmap, hBitmap);				
			*/
		}
	}

	return menu;
}

void ContextMenu::removeLastSeparator(HMENU menu)
{
	int numItems = GetMenuItemCount(menu);
	if (numItems)
	{
		if (GetMenuState(menu, numItems-1, MF_BYPOSITION) & MF_SEPARATOR)
		{
			RemoveMenu(menu, numItems-1, MF_BYPOSITION);
		}
	}
}

void ContextMenu::setOnInvokeCommandHandler( boost::function<void()> onInvokeCommandHandler )
{
	_onInvokeCommandHandler = onInvokeCommandHandler;
}

bool ContextMenu::hasActiveMenu()
{
	return ContextMenu::_currentHmenu != NULL;
}

void ContextMenu::setTrackingEnabled(bool enabled)
{
	ContextMenu::_enableTracking = enabled;
}