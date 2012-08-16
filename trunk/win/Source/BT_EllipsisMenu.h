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

#ifndef _BT_CONTEXT_MENU_
#define _BT_CONTEXT_MENU_

// -----------------------------------------------------------------------------

class FileSystemActor;
class MenuAction;

// -----------------------------------------------------------------------------

#define MIN_ID							1
#define MAX_ID							10000
#define DISPLAY_SCREEN_RESOLUTION_ID	1022
#define DISPLAY_GADGETS_ID				1023
#define DISPLAY_PROPERTIES_ID			1024
#define MENU_ACTION_ID_MIN				1025

// -----------------------------------------------------------------------------

class ContextMenu  
{
	Q_DECLARE_TR_FUNCTIONS(ContextMenu)

		// BumpTop Functionality
	hash_map<uint, MenuAction *> actionList;

	// Explorer Functionality
	IContextMenu *	contextMenu;
	int				menuType;
	IShellFolder2	*shellFolder;
	LPITEMIDLIST	*pidlArray;	
	uint			numItems;

	// need to be accessed inside the hook wndproc
	static HMENU	_currentHmenu;
	static HMENU	_prevHmenu;
	static QPolygon	_currentHmenuRegion;
	static bool		_enableTracking;
	static bool		_unionMenuRectOnNextEnterIdle;

	// native integration
	boost::function<void()> _onInvokeCommandHandler;
private:
	// Private Actions
	void			invokeCommand(LPCONTEXTMENU pContextMenu, UINT idCommand);
	static LRESULT	HookWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	void			removeBumpThisFolder(HMENU menu);
	void			removeMenuItem(HMENU menu, QString itemName);
	void			removeAdjacentMenuSeparators(HMENU menu);
	HMENU			createMenuFromPIDL();
	void			invokeMenu(HMENU menu, POINT pt, uint pos_flags = TPM_CENTERALIGN);
	void			appendMenuActions(HMENU hMenu, vector<MenuAction *> menuActionList);
	void			removeLastSeparator(HMENU hMenu);

	// Private Getters
	BOOL			getContextMenu(void **ppContextMenu, int &iMenuType);
	void			showMenu(HMENU menu);
	HMENU			getMenu(vector<FileSystemActor *> objList);
	HMENU			getMenu(QString dir);
	int				getMenuItemIndex(HMENU menu, QString menuString);
	vector<FileSystemActor *> getSelectedItems();

public:

	ContextMenu();
	~ContextMenu();

	// Actions
	void			showMenu(vector<MenuAction *> &list, POINT p, bool displayNativeMenu = true, uint pos_flags = TPM_CENTERALIGN);
	void			setOnInvokeCommandHandler(boost::function<void()> onInvokeCommand);
	void			setTrackingEnabled(bool enabled);
	bool			hasActiveMenu();

};

// -----------------------------------------------------------------------------

#else
class ContextMenu;
#endif