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
#include "BT_PopupMenuActionCustomizer.h"
#include "BT_SceneManager.h"
#include "BT_MenuAction.h"
#include "BT_MarkingMenu.h"
#include "BT_FileSystemManager.h"
#include "BT_EllipsisMenu.h"
#include "BT_WindowsOS.h"

PopupMenuActionCustomizer::PopupMenuActionCustomizer()
{

}

void PopupMenuActionCustomizer::update( MenuAction *action )
{

}

void PopupMenuActionCustomizer::execute( MenuAction *action )
{
	ContextMenu menu;
	if (menu.hasActiveMenu())
		return;

	if (!_onInvokeCommandHandler.empty())
		menu.setOnInvokeCommandHandler(_onInvokeCommandHandler);

	//Build a vector out of the submenu elements
	vector<MenuAction *> displayItems;

	for(int x = 0; x < action->subMenu.size(); x++) {
		displayItems.push_back(&action->subMenu[x]);
	}

	float xoffset = 0;
	float yoffset = 0;
	float sliceRadius = markingMenu->getRadius();
	uint pos_flags = 0;

	switch((int)action->getDegreePreference()) {
		case North:
			yoffset = -sliceRadius;
			pos_flags = TPM_BOTTOMALIGN | TPM_CENTERALIGN;
			break;
		case East:
			xoffset = sliceRadius;
			pos_flags = TPM_LEFTALIGN;
			break;
		case South:
			yoffset = sliceRadius;
			pos_flags = TPM_TOPALIGN | TPM_CENTERALIGN;
			break;
		case West:
			xoffset = -sliceRadius;
			pos_flags = TPM_RIGHTALIGN;
			break;
		case NorthEast:
			xoffset = sliceRadius;
			yoffset = -sliceRadius;
			pos_flags = TPM_BOTTOMALIGN | TPM_LEFTALIGN;
			break;
		case SouthEast:
			xoffset = sliceRadius;
			yoffset = sliceRadius;
			pos_flags = TPM_TOPALIGN | TPM_LEFTALIGN;
			break;
		case SouthWest:
			xoffset = -sliceRadius;
			yoffset = sliceRadius;
			pos_flags = TPM_TOPALIGN | TPM_RIGHTALIGN;
			break;
		case NorthWest:
			xoffset = -sliceRadius;
			yoffset = -sliceRadius;
			pos_flags = TPM_BOTTOMALIGN | TPM_RIGHTALIGN;
			break;
	}

	POINT p;
	p.x = markingMenu->getCentroid().x + xoffset;
	p.y = markingMenu->getCentroid().y + yoffset;

	POINT windowOffset = {0, 0}; // Convert window coord to screen coord
	if (ClientToScreen(winOS->GetWindowsHandle(), &windowOffset))
	{
		p.x += windowOffset.x;
		p.y += windowOffset.y;
	}
	
	HMENU contextMenu = CreatePopupMenu();
	menu.showMenu(displayItems, p, false, pos_flags);
}