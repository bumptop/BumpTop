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
#include "BT_MenuAction.h"
#include "BT_LibrariesMenuActionCustomizer.h"
#include "BT_LibrarySubMenuActionCustomizer.h"
#include "BT_WindowsOS.h"
#include "BT_LibraryManager.h"
#include "BT_Library.h"
#include "BT_MarkingMenu.h"
#include "BT_Selection.h"

LibrariesMenuActionCustomizer::LibrariesMenuActionCustomizer()
{

}

void LibrariesMenuActionCustomizer::update( MenuAction *action )
{
	if (winOS->GetLibraryManager())
	{
		clearSubMenus(action);
		QListIterator< QSharedPointer<Library> > libIt(winOS->GetLibraryManager()->getLibraries());
		while (libIt.hasNext())
		{
			QSharedPointer<Library> library = libIt.next();
			action->subMenu.push_back(MenuAction(KeyCombo(), true, library->getName(), QT_TR_NOOP("Switch to this library"), 1.0f, 100.0f, Ellipsis, Nothing, NULL, NULL, new LibrarySubMenuActionCustomizer(library)));
		}
	}
}

void LibrariesMenuActionCustomizer::clearSubMenus(MenuAction *action)
{
	vector<MenuAction>::iterator it = action->subMenu.begin();
	while (it != action->subMenu.end())
	{
		MenuAction& menuAction = (*it);
		if (menuAction.getCustomAction())
			delete menuAction.getCustomAction();
		it++;
	}
	action->subMenu.clear();
}