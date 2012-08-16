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
#include "BT_MenuActionManager.h"
#include "BT_KeyCombo.h"
#include "BT_ObjectType.h"
#include "BT_SceneManager.h"
#include "BT_Selection.h"
#include "BT_Pile.h"
#include "BT_Util.h"
#include "BT_FileSystemActor.h"
#include "BT_MarkingMenu.h"
#include "BT_WindowsOS.h"
#include "BT_Authorization.h"
#include "BT_StickyNoteMenuActionCustomizer.h"
#include "BT_LeafForwardMenuActionCustomizer.h"
#include "BT_PileBreakMenuActionCustomizer.h"
#include "BT_PopupMenuActionCustomizer.h"
#include "BT_LibrariesMenuActionCustomizer.h"

MenuActionManager::MenuActionManager()
: themeManagerRef(themeManager)
{
	themeManager->registerThemeEventHandler(this);
}

MenuActionManager::~MenuActionManager()
{
	if (shared_ptr<ThemeManager> tm = themeManagerRef.lock())
		tm->unregisterThemeEventHandler(this);
}

void MenuActionManager::init()
{
	menuActions.push_back(MenuAction(KeyCombo(MouseButtonRight, false, true), true, QT_TR_NOOP("More"),					QT_TR_NOOP("More Options..."),												1.0f, 100.0f, South,	AnySelection | Nothing, NULL, Key_MoreOptions));
	
#ifdef WIN7LIBRARIES
	if (winOS->IsWindowsVersionGreaterThanOrEqualTo(Windows7))
	{
		// Libraries menu option
		//menuActions.push_back(MenuAction(KeyCombo(), true,						QT_TR_NOOP("Libraries"),				QT_TR_NOOP("Switch to a different Library"),								1.0f, 100.0f, West, Nothing, NULL, NULL, new LibrariesMenuActionCustomizer()));
	}
#endif

	menuActions.push_back(MenuAction(KeyCombo('P', true), true,					QT_TR_NOOP("Sticky\nNote"),				QT_TR_NOOP("Leave yourself reminders on Bumpable Sticky Notes"),			1.0f, 100.0f, West,		Nothing, NULL, NULL, new StickyNoteMenuActionCustomizer()));
	menuActions.push_back(MenuAction(KeyCombo('N', true, true, false), true,	QT_TR_NOOP("New Folder..."),			QT_TR_NOOP("Create a new directory"),										1.0f, 100.0f, Ellipsis,	Nothing, NULL, Key_CreateNewDirectory));

	MenuAction widgetSubMenu(KeyCombo(), true,									QT_TR_NOOP("Widgets"),					QT_TR_NOOP("Add Widgets to your BumpTop"),									1.0f, 100.0f, North, Nothing, NULL, NULL, new PopupMenuActionCustomizer());
		//BumpWidgets sub menu
		widgetSubMenu.subMenu.push_back(MenuAction(KeyCombo(), true,			QT_TR_NOOP("Add a Photo Frame..."),		QT_TR_NOOP("Creates a new photo frame."),									1.0f, 100.0f, Ellipsis,	Nothing, NULL, Key_CreateNewPhotoFrame));
		//widgetSubMenu.subMenu.push_back(MenuAction(KeyCombo(), true,			QT_TR_NOOP("Add a Web Widget..."),		QT_TR_NOOP("Creates a new web widget."),									1.0f, 100.0f, Ellipsis,	Nothing, NULL, Key_CreateWebActor));
		widgetSubMenu.subMenu.push_back(MenuAction(KeyCombo(), true,			QT_TR_NOOP("Twitter"),					QT_TR_NOOP("Twitter Widget"),												1.0f, 100.0f, Ellipsis,	Nothing, NULL, Key_ToggleCustomTwitterActor, NULL, false, true, Key_IsTwitterWidgetEnabled));
		widgetSubMenu.subMenu.push_back(MenuAction(KeyCombo(), true,			QT_TR_NOOP("Printer"),					QT_TR_NOOP("Printer Widget"),												1.0f, 100.0f, Ellipsis,	Nothing, NULL, Key_ToggleCustomPrinterActor, NULL, false, true, Key_IsPrinterWidgetEnabled));
		widgetSubMenu.subMenu.push_back(MenuAction(KeyCombo(), true,			QT_TR_NOOP("Email"),					QT_TR_NOOP("Email Widget"),													1.0f, 100.0f, Ellipsis,	Nothing, NULL, Key_ToggleCustomEmailActor, NULL, false, true, Key_IsEmailWidgetEnabled));
		widgetSubMenu.subMenu.push_back(MenuAction(KeyCombo(), true,			QT_TR_NOOP("Sticky Notes"),					QT_TR_NOOP("Email Widget"),													1.0f, 100.0f, Ellipsis,	Nothing, NULL, Key_ToggleCustomStickyNotePadActor, NULL, false, true, Key_IsStickyNotePadWidgetEnabled));
		widgetSubMenu.subMenu.push_back(MenuAction(KeyCombo(), true,			QT_TR_NOOP("Facebook"),					QT_TR_NOOP("Facebook Widget"),												1.0f, 100.0f, Ellipsis,	Nothing, NULL, Key_ToggleFacebookWidget, NULL, false, true, Key_IsFacebookWidgetEnabled));
		if (exists(winOS->GetSharingWidgetPath()))
			widgetSubMenu.subMenu.push_back(MenuAction(KeyCombo(), true,		QT_TR_NOOP("Sharing"),					QT_TR_NOOP("Sharing Widget"),												1.0f, 100.0f, Ellipsis,	Nothing, NULL, Key_ToggleSharingWidget, NULL, false, true, Key_IsSharingWidgetEnabled));
	menuActions.push_back(widgetSubMenu);

	MenuAction pileBySubMenu(KeyCombo(), true,									QT_TR_NOOP("Pile by"),					QT_TR_NOOP("Sort the items on your desktop"),								1.0f, 100.0f, East, Nothing | MultipleFreeItems | MultipleFullPile, NULL, NULL, new PopupMenuActionCustomizer());
		//Pile By sub menu
		pileBySubMenu.subMenu.push_back(MenuAction(KeyCombo('T', true), true,	QT_TR_NOOP("Pile by Type"),				QT_TR_NOOP("Sorts all Items on the desktop into Piles by Type."),			1.0f, 100.0f, Ellipsis, Nothing, NULL, Key_SortIntoPilesByType));
		pileBySubMenu.subMenu.push_back(MenuAction(KeyCombo(), true,			QT_TR_NOOP("Pile by Time"),				QT_TR_NOOP("Sorts all Items on the desktop into Piles by modify Time."),	1.0f, 100.0f, Ellipsis,	Nothing, NULL, Key_SortIntoPilesByTime));
	menuActions.push_back(pileBySubMenu);

	menuActions.push_back(MenuAction(KeyCombo('R', true), true,			QT_TR_NOOP("Grid"),						QT_TR_NOOP("Lays out the selected items into a grid."),								1.0f, 100.0f, NorthWest,MultipleFreeItems, NULL, Key_GridLayoutSelection));	
	menuActions.push_back(MenuAction(KeyCombo('Q', true), true,			QT_TR_NOOP("Create\nPile"),				QT_TR_NOOP("Creates a Piles from the current selection."),							1.0f, 100.0f, North,	MultipleFreeItems | MultipleFullPile, NULL, Key_MakePile));
	// NOTE: that we have a special case check for Expand To Pile in MenuAction::isSelectionValid()
	menuActions.push_back(MenuAction(KeyCombo('E', true), true,			QT_TR_NOOP("Expand to\nPile"),			QT_TR_NOOP("Expands all Folders in the current selection into Piles."),				1.0f, 100.0f, North,	UseOnlyThisType | SingleFreeItem, ObjectType(BumpActor, FileSystem, Folder), Key_ExpandToPile));
	menuActions.push_back(MenuAction(KeyCombo('E', true), true,			QT_TR_NOOP("Stack"),					QT_TR_NOOP("Collapses the current view into a stacked Pile state."),				1.0f, 100.0f, NorthWest,UseOnlyThisType | SingleFullPile, ObjectType(BumpPile, HardPile, Grid), Key_StackPile));
	menuActions.push_back(MenuAction(KeyCombo('E', true), true,			QT_TR_NOOP("Stack"),					QT_TR_NOOP("Collapses the current view into a stacked Pile state."),				1.0f, 100.0f, NorthWest,UseOnlyThisType | SingleFullPile, ObjectType(BumpPile, SoftPile, Grid), Key_StackPile));
	menuActions.push_back(MenuAction(KeyCombo('D', true), true,			QT_TR_NOOP("     Folderize"),			QT_TR_NOOP("Moves all pile contents into a new folder."),							1.0f, 100.0f, NorthEast,UseOnlyThisType | AnyFullPile, NULL, Key_Folderize));
	//menuActions.push_back(MenuAction(KeyCombo('Z', true), true,		QT_TR_NOOP("Undo"),						QT_TR_NOOP("Reverts Changes made on the Desktop."),									1.0f, 100.0f, West,		Nothing, NULL, NULL, undoManager));
	//menuActions.push_back(MenuAction(KeyCombo('Y', true), true,		QT_TR_NOOP("Redo"),						QT_TR_NOOP("Returns the state of the Desktop before Undo was Triggered."),			1.0f, 100.0f, East,		Nothing, NULL, NULL, undoManager));
	//menuActions.push_back(MenuAction(KeyCombo('F', true), true,		QT_TR_NOOP("Search"),					QT_TR_NOOP("Searches the filenames on the desktop for the substring provided."),	1.0f, 100.0f, North,	Nothing, NULL, Key_SearchSubString));
	menuActions.push_back(MenuAction(KeyCombo('G', true), true,			QT_TR_NOOP("Grow"),						QT_TR_NOOP("Enlarges the Piles in the current selection."),							1.0f, 100.0f, Ellipsis,	AnyFullPile, NULL, Key_Grow));
	menuActions.push_back(MenuAction(KeyCombo('S', true), true,			QT_TR_NOOP("Shrink"),					QT_TR_NOOP("Shrinks the Piles in the current selection."),							1.0f, 100.0f, Ellipsis,	AnyFullPile, NULL, Key_Shrink));
	menuActions.push_back(MenuAction(KeyCombo(KeyComma, true), true,	QT_TR_NOOP("BumpTop Settings..."),		QT_TR_NOOP("Bring up BumpTop's Settings dialog."),									1.0f, 100.0f, Ellipsis,	Nothing, NULL, Key_ShowSettingsDialog));
	menuActions.push_back(MenuAction(KeyCombo(), true,					QT_TR_NOOP("BumpTop Themes..."),		QT_TR_NOOP("Themes..."),															1.0f, 100.0f, Ellipsis,	Nothing, NULL, Key_ShowSettingsOpenThemesTab));
	menuActions.push_back(MenuAction(KeyCombo(), true,					QT_TR_NOOP("Open Explorer Window Here"),QT_TR_NOOP("Opens an Explorer window for this folder"),								1.0f, 100.0f, Ellipsis,	Nothing, NULL, Key_LaunchExplorerWindowOnWorkingDirectory));
	menuActions.push_back(MenuAction(KeyCombo(), true,					QT_TR_NOOP("Open Command Window Here"),	QT_TR_NOOP("Opens a Command window for this folder"),								1.0f, 100.0f, Ellipsis,	Nothing, NULL, Key_LaunchCommandWindowOnWorkingDirectory));
	menuActions.push_back(MenuAction(KeyCombo(KeyF2), true,				QT_TR_NOOP("Rename"),					QT_TR_NOOP("Renames the currently selected file."),									1.0f, 100.0f, Ellipsis,	UseOnlyThisType | SingleFreeItem, ObjectType(BumpActor, FileSystem), Key_RenameSelection));
	menuActions.push_back(MenuAction(KeyCombo(KeyF2), true,				QT_TR_NOOP("Rename"),					QT_TR_NOOP("Renames the currently selected file."),									1.0f, 100.0f, Ellipsis,	UseOnlyThisType | SingleFullPile, ObjectType(BumpPile, HardPile, Stack), Key_RenameSelection));
	menuActions.push_back(MenuAction(KeyCombo(KeyF2), true,				QT_TR_NOOP("Rename"),					QT_TR_NOOP("Renames the currently selected file."),									1.0f, 100.0f, Ellipsis,	UseOnlyThisType | SinglePileMemberItem, NULL, Key_RenameSelection));
	menuActions.push_back(MenuAction(KeyCombo('R', true), true,			QT_TR_NOOP("Reset size"),				QT_TR_NOOP("Resizes the selected items to their original size."),					1.0f, 100.0f, Ellipsis,	AnyFreeItems | UseOnlyThisType, ObjectType(BumpActor), Key_ResetSize));
	menuActions.push_back(MenuAction(KeyCombo(), true,					QT_TR_NOOP("Reset size"),				QT_TR_NOOP("Resizes the selected items to their original size."),					1.0f, 100.0f, Ellipsis,	AnySelection | UseOnlyThisType, ObjectType(BumpPile), Key_ResetSize));
	menuActions.push_back(MenuAction(KeyCombo(), true,					QT_TR_NOOP("Change Icon..."),			QT_TR_NOOP("Overrides the icon with a custom image."),								1.0f, 100.0f, Ellipsis,	UseOnlyThisType | SingleFreeItem, ObjectType(BumpActor, ~Webpage), Key_OverrideFileTexture));
	menuActions.push_back(MenuAction(KeyCombo(), true,					QT_TR_NOOP("Show file in Explorer"),	QT_TR_NOOP("Opens a new Explorer window and select this file"),						1.0f, 100.0f, Ellipsis,	SingleFreeItem, ObjectType(BumpActor, FileSystem), Key_LaunchExplorerWithFileSelected));
	menuActions.push_back(MenuAction(KeyCombo(KeyDelete), true,			QT_TR_NOOP("Delete"),					QT_TR_NOOP("Deletes the current selection from your computer."),					1.0f, 100.0f, Ellipsis,	AnySelection, ObjectType(BumpActor, Text, NULL), Key_DeleteSelection));
	menuActions.push_back(MenuAction(KeyCombo(KeyDelete), true,			QT_TR_NOOP("Delete"),					QT_TR_NOOP("Deletes the web widget."),												1.0f, 100.0f, Ellipsis,	UseOnlyThisType | SingleFreeItem, ObjectType(BumpActor, Webpage, NULL), Key_DeleteSelection));
	menuActions.push_back(MenuAction(KeyCombo(), true,					QT_TR_NOOP("Filename Toggle"),			QT_TR_NOOP("Toggles the text label under each Icon."),								1.0f, 100.0f, Ellipsis, UseOnlyThisType | AnyFreeItems, ObjectType(BumpActor, ~Webpage), Key_ToggleSelectionText));
	menuActions.push_back(MenuAction(KeyCombo('N'), true,				QT_TR_NOOP("Name Pile"),				QT_TR_NOOP("Puts the contents of this pile into a directory with a given name."),	1.0f, 100.0f, Ellipsis,	UseOnlyThisType | SingleFullPile, ObjectType(BumpPile, SoftPile, Stack), Key_RenameSelection));
	menuActions.push_back(MenuAction(KeyCombo(KeyRight), true,			QT_TR_NOOP("Slideshow"),				QT_TR_NOOP("Starts a slideshow with the currently selected objects."),				1.0f, 100.0f, Ellipsis,	UseOnlyThisType | AnySelection, ObjectType(BumpActor, FileSystem, Image), Key_ToggleSlideShow));
	menuActions.push_back(MenuAction(KeyCombo('G', true), true,			QT_TR_NOOP("Grow"),						QT_TR_NOOP("Enlarges the Icons in the current selection."),							1.0f, 100.0f, East,		AnyFreeItems | AnyPileMemberItems, NULL, Key_Grow));
	menuActions.push_back(MenuAction(KeyCombo('S', true), true,			QT_TR_NOOP("Shrink"),					QT_TR_NOOP("Shrinks the Icons in the current selection."),							1.0f, 100.0f, West,		AnyFreeItems | AnyPileMemberItems, NULL, Key_Shrink));
	menuActions.push_back(MenuAction(KeyCombo('R', true), true,			QT_TR_NOOP("Grid View"),				QT_TR_NOOP("Spreads the Icons in the current selection into a grid."),				1.0f, 100.0f, Ellipsis,	UseOnlyThisType | MultipleFreeItems, ObjectType(BumpPile, HardPile, Stack), Key_GridView));
	menuActions.push_back(MenuAction(KeyCombo('R', true), true,			QT_TR_NOOP("Grid View"),				QT_TR_NOOP("Spreads the Icons in the current selection into a grid."),				1.0f, 100.0f, Ellipsis,	UseOnlyThisType | MultipleFreeItems, ObjectType(BumpPile, SoftPile, Stack), Key_GridView));
	menuActions.push_back(MenuAction(KeyCombo(MouseButtonLeft, true), true, QT_TR_NOOP("Grid"),					QT_TR_NOOP("Spreads the Icons in the current selection into a grid."),				1.0f, 100.0f, North,	UseOnlyThisType | SingleFullPile, ObjectType(BumpPile, HardPile, Stack), Key_GridView));
	menuActions.push_back(MenuAction(KeyCombo(MouseButtonLeft, true), true, QT_TR_NOOP("Grid"),					QT_TR_NOOP("Spreads the Icons in the current selection into a grid."),				1.0f, 100.0f, North,	UseOnlyThisType | SingleFullPile, ObjectType(BumpPile, SoftPile, Stack), Key_GridView));
	menuActions.push_back(MenuAction(KeyCombo(MouseButtonScrollUp), true, QT_TR_NOOP("Flip Page"),				QT_TR_NOOP("Flips through the pile like a book."),									1.0f, 100.0f, NorthWest,UseOnlyThisType | SingleFullPile, ObjectType(BumpPile, NULL, Stack | Leaf), NULL, new LeafForwardMenuActionCustomizer()));	
	menuActions.push_back(MenuAction(KeyCombo('A', true), true,			QT_TR_NOOP("Fan Out"),					QT_TR_NOOP("Fans out the Pile."),													1.0f, 100.0f, SouthWest,UseOnlyThisType | SingleFullPile, NULL, Key_FanOut));
	menuActions.push_back(MenuAction(KeyCombo('H'), true,				QT_TR_NOOP("Sort By Type"),				QT_TR_NOOP("Sorts all Items in the selection by Type."),							1.0f, 100.0f, Ellipsis,	UseOnlyThisType | SingleFullPile, ObjectType(BumpPile, NULL, ~Leaf), Key_SortByType));
	menuActions.push_back(MenuAction(KeyCombo('Y'), true,				QT_TR_NOOP("Sort By Size"),				QT_TR_NOOP("Sorts all Items in the selection by Size."),							1.0f, 100.0f, Ellipsis,	UseOnlyThisType | SingleFullPile, ObjectType(BumpPile, NULL, ~Leaf)	, Key_SortBySize));
	menuActions.push_back(MenuAction(KeyCombo('M'), true,				QT_TR_NOOP("Sort By Name"),				QT_TR_NOOP("Sorts all Items in the selection by Name."),							1.0f, 100.0f, Ellipsis,	UseOnlyThisType | SingleFullPile, ObjectType(BumpPile, NULL, ~Leaf)	, Key_SortByName));
	menuActions.push_back(MenuAction(KeyCombo('B', true), true,			QT_TR_NOOP("Break Pile"),				QT_TR_NOOP("Breaks the pile into free items."),										1.0f, 100.0f, SouthEast,UseOnlyThisType | AnyFullPile, NULL, NULL, new PileBreakMenuActionCustomizer()));
	menuActions.push_back(MenuAction(KeyCombo('V'), true,				QT_TR_NOOP("Remove from Pile"),			QT_TR_NOOP("Removes the selection from its pile."),									1.0f, 100.0f, SouthWest,UseOnlyThisType | AnyPileMemberItems, NULL, Key_RemoveFromPile));
	menuActions.push_back(MenuAction(KeyCombo(), true,					QT_TR_NOOP("Change Photo Frame"),		QT_TR_NOOP("Changes which photos to show in the photo frame."),								1.0f, 100.0f, Ellipsis,	UseOnlyThisType | SingleFreeItem, ObjectType(BumpActor, FileSystem, PhotoFrame), Key_ModifySelectedPhotoFrameSource));
	menuActions.push_back(MenuAction(KeyCombo(KeyDelete), true,			QT_TR_NOOP("Delete"),					QT_TR_NOOP("Deletes the photo frame."),												1.0f, 100.0f, Ellipsis,	UseOnlyThisType | SingleFreeItem, ObjectType(BumpActor, FileSystem, PhotoFrame), Key_DeleteSelectedPhotoFrameSource));
	menuActions.push_back(MenuAction(KeyCombo(), true,					QT_TR_NOOP("Logout of service"),		QT_TR_NOOP("Logs out of this service."),											1.0f, 100.0f, Ellipsis,	UseOnlyThisType | SingleFreeItem, ObjectType(BumpActor, Custom), Key_LogoutServiceSelection));
	menuActions.push_back(MenuAction(KeyCombo(KeyF4, false, false, true), true, QT_TR_NOOP("Exit BumpTop"),		QT_TR_NOOP("Returns you back to Windows."),											1.0f, 100.0f, Ellipsis,	Nothing, NULL, ExitBumptop));
	menuActions.push_back(MenuAction(KeyCombo(), true,					QT_TR_NOOP("Reload page"),				QT_TR_NOOP("Reloads the web page."),												1.0f, 100.0f, Ellipsis,	UseOnlyThisType | SingleFreeItem, ObjectType(BumpActor, Webpage, NULL), Key_ReloadWebActor));	

	menuActions.push_back(MenuAction(KeyCombo('Q', true), true,				QT_TR_NOOP("Create\nPile"),				QT_TR_NOOP("Converts this bubble cluster into a pile."),							1.0f, 100.0f, North,	AnyBumpCluster, NULL, Key_CreatePileFromCluster));

	// update all the fonts
	onThemeChanged();
}

vector<MenuAction *> MenuActionManager::determineMenuActions(Selection *s)
{
	vector<MenuAction *> newMenuActions;
	uint selectionType = s->getSelectionType();

	// Look for menu actions that are compatible with the selection
	for (uint i = 0; i < menuActions.size(); i++)
	{
		if (menuActions[i].isSelectionValid(selectionType))
		{
			// The selection condition matches this menu action, add it to the menu
			newMenuActions.push_back(&menuActions[i]);
		}
	}

	return newMenuActions;
}

void MenuActionManager::onThemeChanged()
{
	if (menuActions.empty())
	{
		QString fontName = themeManager->getValueAsFontFamilyName("ui.markingMenu.font.family","");
		_primaryTextFont = FontDescription(fontName, themeManager->getValueAsInt("ui.markingMenu.font.size",12));		
		_secondaryTextFont = FontDescription(fontName, themeManager->getValueAsInt("ui.markingMenu.font.subTextSize",10));

		// QT_TEXT_RENDERING TEMP (to accomodate old themes, we bound the font)
		_primaryTextFont.fontSize = NxMath::min(14, _primaryTextFont.fontSize);
		_secondaryTextFont.fontSize = NxMath::min(9, _secondaryTextFont.fontSize);
		// END TEMP
	}

	vector<MenuAction>::iterator iter = menuActions.begin();
	while (iter != menuActions.end())
	{
		MenuAction& action = *iter;
		action.setPrimaryTextFont(fontManager->getFont(_primaryTextFont));
		action.setSecondaryTextFont(fontManager->getFont(_secondaryTextFont));
		iter++;
	}
}

#ifdef DXRENDER
void MenuActionManager::onRelease()
{
	vector<MenuAction>::iterator iter = menuActions.begin();
	while (iter != menuActions.end())
	{
		MenuAction& action = *iter;
		action.onRelease();
		iter++;
	}
}
#endif