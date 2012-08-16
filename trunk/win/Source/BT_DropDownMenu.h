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

#ifndef _BT_DROPDOWNMENU_
#define _BT_DROPDOWNMENU_

#include "BT_Common.h"
#include "BT_OverlayComponent.h"
#include "BT_OverlayEvent.h"

class DropDownMenuItem;

class DropDownMenu : public VerticalOverlayLayout
{
private:
	bool _opened;
	int _openAnimSteps;
	int _closeAnimSteps;
	OverlayComponent* _componentSelectedReady;

	ColorVal _backgroundColor;
	ColorVal _selectedBackgroundColor;
	ColorVal _mouseOverBackgroundColor;

private:
	void init();

public:
	DropDownMenu();
	virtual ~DropDownMenu();

	bool select(OverlayComponent* component);

	void open();
	void close();
	bool isOpen() const;
	
	void slideOpen();
	void slideClosed();

	void maximizeComponentWidths();

	virtual void reLayout();
	virtual Vec3 getPreferredDimensions();
	
	virtual void onRender(const Vec3& offset, const Bounds& renderingBounds);
	virtual bool onMouseDown(MouseOverlayEvent& mouseEvent);
	virtual bool onMouseMove(MouseOverlayEvent& mouseEvent);
};

class DropDownMenuItem : public HorizontalOverlayLayout
{
public:
	DropDownMenuItem();
	virtual ~DropDownMenuItem();

	virtual bool onMouseDown(MouseOverlayEvent& mouseEvent);
	virtual bool onMouseUp(MouseOverlayEvent& mouseEvent);
	virtual bool onMouseMove(MouseOverlayEvent& mouseEvent);
	virtual void onSelect() = 0;
	virtual void onClose();
	virtual bool shouldClose(MouseOverlayEvent& mouseEvent);
};

#endif /*_BT_DROPDOWNMENU_*/