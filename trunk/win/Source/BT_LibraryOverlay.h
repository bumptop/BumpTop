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

#ifndef _BT_LIBRARYOVERLAY_
#define _BT_LIBRARYOVERLAY_

#include "BT_Common.h"
#include "BT_OverlayEvent.h"
#include "BT_OverlayComponent.h"
#include "BT_Library.h"
#include "BT_DropDownMenu.h"

class LibraryMenuControl : public AbsoluteOverlayLayout
{
	Q_DECLARE_TR_FUNCTIONS(LibraryMenuControl)

	DropDownMenu* _dropDownMenu;
	QSharedPointer<Library> _selectedLibrary;
	float _unHoveredAlpha;
	bool _needsUpdate;

private:
	void init(QSharedPointer<Library>& library);
	void update();
	void destroy();

	// animation
	void fadeIn(float maxAlpha = 1.0f);
	void fadeOut(float minAlpha = 0.0f);

public:
	LibraryMenuControl();
	virtual ~LibraryMenuControl();

	void show(QSharedPointer<Library>& library);
	void hide();

	// event handlers
	virtual bool onMouseDown(MouseOverlayEvent& mouseEvent);
	virtual bool onMouseUp(MouseOverlayEvent& mouseEvent);
	virtual bool onMouseMove(MouseOverlayEvent& mouseEvent);
	virtual void onSize(Vec3& newSize);
	virtual void onRender(const Vec3& offset, const Bounds & renderingBounds);
};

class LibraryMenuItem : public DropDownMenuItem
{
private:
	QSharedPointer<Library> _library;
	ImageOverlay* _closeButton;

private:
	virtual void onSelect();

public:
	LibraryMenuItem();
	LibraryMenuItem(QSharedPointer<Library>& library);
	virtual ~LibraryMenuItem();
	void init(QSharedPointer<Library>& library);
	
	virtual bool shouldClose(MouseOverlayEvent& mouseEvent);
	virtual void onClose();

	void setDeleteable(bool value);
	bool isDeleteable() const;
};

class BrowseFolderMenuItem : public DropDownMenuItem
{
private:
	virtual void onSelect();

public:
	BrowseFolderMenuItem();
	BrowseFolderMenuItem(QString& caption);
	virtual ~BrowseFolderMenuItem();
	void init(QString& caption);
};
#endif /*_BT_LIBRARYOVERLAY_ */