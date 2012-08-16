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
#include "BT_LibraryOverlay.h"
#include "BT_WindowsOS.h"
#include "BT_SceneManager.h"
#include "BT_OverlayComponent.h"
#include "BT_ThemeManager.h"
#include "BT_DropDownMenu.h"
#include "BT_LibraryManager.h"
#include "BT_Util.h"

LibraryMenuControl::LibraryMenuControl()
: _unHoveredAlpha(0.3f)
, _dropDownMenu(NULL)
, _selectedLibrary(NULL)
, _needsUpdate(true)
{}

LibraryMenuControl::~LibraryMenuControl()
{
	destroy();
}

// This method creates the control for the first time. It should only be called
// on creation
void LibraryMenuControl::init(QSharedPointer<Library>& library)
{
	destroy();

	if (!winOS->GetLibraryManager())
		return;
	
	getStyle().setFloating(true);
	
	_dropDownMenu = new DropDownMenu();
	
	LibraryMenuItem* toSelect = NULL;
	const QList< QSharedPointer<Library> >& libraries = winOS->GetLibraryManager()->getLibraries();
	QListIterator< QSharedPointer<Library> > iter(libraries);
	while (iter.hasNext())
	{
		QSharedPointer<Library> lib = iter.next();
		LibraryMenuItem* item = new LibraryMenuItem(lib);
		if (lib->getHashKey().startsWith(QT_NT("usr_")) && library != lib)
			item->setDeleteable(true);
		_dropDownMenu->addItem(item);
		if (lib->getHashKey() == library->getHashKey())
			toSelect = item;
	}
	
	BrowseFolderMenuItem* browseFolder = new BrowseFolderMenuItem(QString("More places..."));
	_dropDownMenu->addItem(browseFolder);

	if (toSelect)
		_dropDownMenu->select(toSelect);

	this->addItem(_dropDownMenu);
	scnManager->registerOverlay(this);
	fadeOut(_unHoveredAlpha);
}

// This method is called just before a render
void LibraryMenuControl::update()
{
	if (!_needsUpdate)
		return;

	init(_selectedLibrary);

	_needsUpdate = false;
}

void LibraryMenuControl::destroy()
{
	scnManager->unregisterOverlay(this);
	this->clearItems();
	_dropDownMenu = NULL;
}

void LibraryMenuControl::fadeIn(float maxAlpha)
{
	if (!_dropDownMenu)
		return;

	float alpha = _dropDownMenu->getStyle().getAlpha();
	if (alpha < maxAlpha)
		_dropDownMenu->setAlphaAnim(alpha, maxAlpha, 5);
}

void LibraryMenuControl::fadeOut(float minAlpha)
{
	if (!_dropDownMenu || _dropDownMenu->isOpen())
		return;

	float alpha = _dropDownMenu->getStyle().getAlpha();
	if (alpha > minAlpha)
		_dropDownMenu->setAlphaAnim(alpha, minAlpha, 15);
}

void LibraryMenuControl::show(QSharedPointer<Library>& library)
{
	_selectedLibrary = library;
	_needsUpdate = true;
	scnManager->registerOverlay(this);

	// Initialization is deffered till we draw
}

void LibraryMenuControl::onRender(const Vec3 &offset, const Bounds & renderingBounds)
{
	update();
	AbsoluteOverlayLayout::onRender(offset, renderingBounds);
}

void LibraryMenuControl::hide()
{
	fadeOut();
}

bool LibraryMenuControl::onMouseDown(MouseOverlayEvent& mouseEvent)
{
	if (_dropDownMenu && !intersects(mouseEvent.getAbsolutePosition(), _dropDownMenu->getBounds()))
	{
		_dropDownMenu->slideClosed();
		fadeOut(_unHoveredAlpha);
	}
	return AbsoluteOverlayLayout::onMouseDown(mouseEvent);
}

bool LibraryMenuControl::onMouseUp(MouseOverlayEvent& mouseEvent)
{
	return false;
}

bool LibraryMenuControl::onMouseMove(MouseOverlayEvent& mouseEvent)
{
	if (_dropDownMenu)
	{
		Vec3 pos = mouseEvent.getAbsolutePosition() - getPosition();
		if (intersects(pos, _dropDownMenu->getBounds()))
		{
			if (!_dropDownMenu->wasMouseLastHovered())
			{
				_dropDownMenu->setMouseLastHovered(true);
				fadeIn();
			}
		}
		else if (_dropDownMenu->wasMouseLastHovered())
		{
			_dropDownMenu->setMouseLastHovered(false);
			fadeOut(_unHoveredAlpha);
		}
	}
	
	return AbsoluteOverlayLayout::onMouseMove(mouseEvent);
}

void LibraryMenuControl::onSize(Vec3& newSize)
{
	if (_dropDownMenu)
		_dropDownMenu->reLayout();
	AbsoluteOverlayLayout::onSize(newSize);
}

/*	------------------------
	    LibraryMenuItem    
	------------------------ */

LibraryMenuItem::LibraryMenuItem()
: _library(NULL)
, _closeButton(NULL)
{}

LibraryMenuItem::LibraryMenuItem(QSharedPointer<Library> &library)
{
	init(library);	
}

LibraryMenuItem::~LibraryMenuItem()
{}

void LibraryMenuItem::init(QSharedPointer<Library> &library)
{
	_library = library;
	_closeButton = NULL;

	QString fontFamily = themeManager->getValueAsFontFamilyName(QT_NT("ui.message.font.family"),QT_NT(""));
	int fontSize = themeManager->getValueAsInt(QT_NT("ui.message.font.size"), 14);
	
	TextOverlay* text = new TextOverlay(_library->getName());
	text->getStyle().setAlpha(1.0f);
	text->setFont(FontDescription(fontFamily, fontSize));
	text->getStyle().setBackgroundColor(ColorVal(0, 0, 0, 0));
	text->getStyle().setPadding(LeftRightEdges, 0.0f);
	text->getStyle().setPadding(TopBottomEdges, 0.0f);

	float height = (float)text->getTextBuffer().getActualSize().height();
	ImageOverlay* icon = new ImageOverlay(_library->getIconTextureKey());
	icon->getStyle().setScaledDimensions(Vec3(height, height, 0.0f));

	this->addItem(icon);
	this->addItem(text);
}

void LibraryMenuItem::setDeleteable(bool value)
{
	if (_closeButton && value)
		return;
	if (!_closeButton && !value)
		return;

	if (value)
	{
		_closeButton = new ImageOverlay(QT_NT("widget.close"));
		_closeButton->getStyle().setScaledDimensions(Vec3(20.0f, 20.0f, 0.0f));

		this->insertItem(_closeButton, 0);
	}
	else
	{
		this->deleteItem(_closeButton);
		_closeButton = NULL;
	}
}

bool LibraryMenuItem::isDeleteable() const
{
	return _closeButton;
}

void LibraryMenuItem::onSelect()
{
	if (_library)
	{
		SwitchToLibrary(_library);
	}
}

bool LibraryMenuItem::shouldClose(MouseOverlayEvent& mouseEvent)
{
	if (_closeButton)
	{
		Vec3 pos = mouseEvent.getPosition();
		return intersects(pos, _closeButton->getBounds());
	}
	return false;
}

void LibraryMenuItem::onClose()
{
	setAlphaAnim(getStyle().getAlpha(), 0.0f, 15, (FinishedCallBack) RemoveLayoutItemAfterAnim, getParent());
	if (winOS->GetLibraryManager())
	{
		RemoveLibrary(_library);
		GLOBAL(settings).otherLibraries = winOS->GetLibraryManager()->getFolderLibraryDirectories();
		winOS->SaveSettingsFile();
	}
}

/*	------------------------
	    BrowseFolderMenuItem    
	------------------------ */

BrowseFolderMenuItem::BrowseFolderMenuItem()
{}

BrowseFolderMenuItem::BrowseFolderMenuItem(QString& caption)
{
	init(caption);
}

BrowseFolderMenuItem::~BrowseFolderMenuItem()
{}

void BrowseFolderMenuItem::init(QString& caption)
{
	QString fontFamily = themeManager->getValueAsFontFamilyName(QT_NT("ui.message.font.family"),QT_NT(""));
	int fontSize = themeManager->getValueAsInt(QT_NT("ui.message.font.size"), 14);
	
	TextOverlay* text = new TextOverlay(caption);
	text->getStyle().setAlpha(1.0f);
	text->setFont(FontDescription(fontFamily, fontSize));
	text->getStyle().setBackgroundColor(ColorVal(0, 0, 0, 0));
	text->getStyle().setPadding(LeftRightEdges, 0.0f);
	text->getStyle().setPadding(TopBottomEdges, 0.0f);

	this->addItem(text);
}

void BrowseFolderMenuItem::onSelect()
{
	if (SUCCEEDED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)))
	{
		DropDownMenu* dropDownMenu = dynamic_cast<DropDownMenu*>(getParent());
		if (dropDownMenu)
		{
			BROWSEINFO browseInfo;
			ZeroMemory(&browseInfo, sizeof(browseInfo));
			browseInfo.hwndOwner = winOS->GetWindowsHandle();
			browseInfo.ulFlags = BIF_USENEWUI | BIF_RETURNONLYFSDIRS;
			PIDLIST_ABSOLUTE pidl = SHBrowseForFolder(&browseInfo);
			if (pidl)
			{	
				if (winOS->GetLibraryManager())
				{
					QSharedPointer<Library> library = winOS->GetLibraryManager()->addFolderAsLibrary(pidl);
					if (library)
					{
						LOG(QString("Library Directory: %1\n").arg(library->getFolderPaths().front()));
						
						// Save the newly created library
						QList<QString> otherLibs = winOS->GetLibraryManager()->getFolderLibraryDirectories();
						GLOBAL(settings).otherLibraries = otherLibs;
						winOS->SaveSettingsFile();
						
						LibraryMenuItem* item = new LibraryMenuItem(library);
						dropDownMenu->select(item);
						SwitchToLibrary(library);
					}
				}
				CoTaskMemFree(pidl);
			}
			else
			{
				// Reselect the last selected object, since this selection failed
				dropDownMenu->select(dropDownMenu->items().front());
			}
		}
		CoUninitialize();
	}
}