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
#include "BT_DropDownMenu.h"
#include "BT_RenderManager.h"
#include "BT_WindowsOS.h"
#include "BT_Util.h"

#ifdef DXRENDER
	#include "BT_DXRender.h"
#endif

DropDownMenu::DropDownMenu()
: _opened(false)
, _openAnimSteps(10)
, _closeAnimSteps(5)
, _componentSelectedReady(NULL)
, _backgroundColor(200, 30, 30, 30)
, _selectedBackgroundColor(255, 0, 0, 0)
, _mouseOverBackgroundColor(255, 87, 126, 187)
{
	init();
}

DropDownMenu::~DropDownMenu()
{}

void DropDownMenu::init()
{
	_opened = false;
	OverlayStyle& style = this->getStyle();
	style.setBackgroundColor(_backgroundColor);
	style.setPadding(LeftRightEdges, 0.0f);
	style.setPadding(TopBottomEdges, 0.0f);
	style.setOverflow(OverlayStyle::Hidden);
}

bool DropDownMenu::select(OverlayComponent *component)
{
	// Select this component as the active menu item
	// and deselect the previously selected one

	// Set the previously selected component's background color
	OverlayComponent* oldComponent = items().front();
	oldComponent->getStyle().setBackgroundColor(ColorVal(0, 0, 0, 0));
	Vec3 prefDims = oldComponent->getPreferredDimensions();			
	oldComponent->setSize(prefDims);
	
	// Set the newly selected component's background color
	component->getStyle().setBackgroundColor(_selectedBackgroundColor);
	Vec3 componentSize = component->getSize();
	componentSize.x = getSize().x;
	component->setSize(componentSize);

	// Mark the component as needing to be moved in the
	// component list. We cannot move it here, since this
	// is called from within an iteration over the list
	// of components and if the list is modified,
	// the iterator will be invalidated
	_componentSelectedReady = component;

	slideClosed();
	return true;
}

void DropDownMenu::open()
{
	if (_opened)
		return;

	_opened = true;
	
	Vec3 openSize = getPreferredDimensions();
	setSize(openSize);
	rndrManager->invalidateRenderer();
	maximizeComponentWidths();
}

void DropDownMenu::close()
{
	if (!_opened)
		return;

	_opened = false;

	if (items().size() > 0)
	{
		Vec3 closedSize = getPreferredDimensions();
		setSize(closedSize);
		rndrManager->invalidateRenderer();
	}
}

bool DropDownMenu::isOpen() const
{
	return _opened;
}

void DropDownMenu::slideOpen()
{
	if (_opened)
		return;

	_opened = true;
	
	killAnimation();

	Vec3 currentSize = getSize();
	Vec3 openSize = getPreferredDimensions();
	setSizeAnim(currentSize, openSize, _openAnimSteps);
	maximizeComponentWidths();
}

void DropDownMenu::slideClosed()
{
	if (!_opened)
		return;

	_opened = false;

	killAnimation();

	if (items().size() > 0)
	{
		Vec3 currentSize = getSize();
		Vec3 closedSize = getPreferredDimensions();
		setSizeAnim(currentSize, closedSize, _closeAnimSteps);
	}
}

void DropDownMenu::maximizeComponentWidths()
{
	float maxWidth = getPreferredDimensions().x;
	
	const vector<OverlayComponent*>& components = items();
	vector<OverlayComponent*>::const_iterator iter = components.begin();
	while (iter != components.end())
	{
		OverlayComponent* component = *iter;
		Vec3 size = component->getSize();
		size.x = maxWidth;
		component->setSize(size);
		iter++;
	}
}

Vec3 DropDownMenu::getPreferredDimensions()
{
	if (isAnimating(SizeAnim))
		return getSize();

	if (isOpen())
		return VerticalOverlayLayout::getPreferredDimensions();
	
	Vec3 dims(0,0,0);
	const vector<OverlayComponent *>& components = items();
	vector<OverlayComponent *>::const_iterator iter = components.begin();

	// get the dimensions of the bounds of the first component
	if (components.size() < 1)
		return dims;

	// Find the largest width
	while (iter != components.end())
	{
		dims.x = NxMath::max(dims.x, (*iter)->getPreferredDimensions().x);
		iter++;
	}
	
	// add the current padding to the dimension bounds and the first item's height and the largest
	// item's width
	dims.y = components.front()->getPreferredDimensions().y + getStyle().getPadding(TopBottomEdges);
	dims.x += getStyle().getPadding(LeftRightEdges);
	return dims;
}

// Positions the components inside the Drop down menu from top to bottom, unlike VerticalOverlayLayout,
// which does it from the bottom up.
void DropDownMenu::reLayout()
{
	Vec3 size = getSize();
	getStyle().setOffset(Vec3(10.0f, winOS->GetWindowHeight() - size.y - 10.0f, 0.0f));
	setPosition(Vec3(10.0f, winOS->GetWindowHeight() - size.y - 10.0f, 0.0f));
	
	const vector<OverlayComponent *>& items = OverlayLayout::items();

	// set the position of each of the items
	Vec3 finalPos(0.0f);
	finalPos.y = size.y - getStyle().getPadding(TopEdge);
	Vec3 itemDims;
	vector<OverlayComponent *>::const_iterator iter = items.begin();
	while (iter != items.end())
	{
		OverlayComponent* item = *iter;
		
		// The backgrounds need to be updated in case an item is removed from the menu,
		// causing all items to shrink. This is expensive though, and since this function
		// is hit on each Animation tick, we'd better not update when we're animating
		if (!isAnimating())
			item->markBackgroundAsDirty();
		itemDims = item->getPreferredDimensions();
		finalPos.y -= itemDims.y;
		item->setPosition(finalPos);
		finalPos.y -= getStyle().getSpacing();
		iter++;
	}
}

void DropDownMenu::onRender(const Vec3& offset, const Bounds& renderingBounds)
{
	if (_componentSelectedReady)
	{
		// Move the component to the bottom
		removeItem(_componentSelectedReady);
		insertItem(_componentSelectedReady, 0);

		// Upon insertion, an alpha animation is applied to the component
		// this is undesired in this case, as we are simply repositioning
		// the component so it is removed
		_componentSelectedReady->killAnimation();
		reLayout();
		_componentSelectedReady = NULL;
	}
	dxr->device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	dxr->device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	dxr->device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
	VerticalOverlayLayout::onRender(offset, renderingBounds);
	dxr->device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	dxr->device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	dxr->device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
}

bool DropDownMenu::onMouseDown(MouseOverlayEvent &mouseEvent)
{
	if (!isOpen())
	{
		slideOpen();
		return true;
	}
	else if (mouseEvent.getButton() == MouseButtonRight)
	{
		slideClosed();
		return true;
	}
	return VerticalOverlayLayout::onMouseDown(mouseEvent);
}

bool DropDownMenu::onMouseMove(MouseOverlayEvent &mouseEvent)
{
	const vector<OverlayComponent*> menuItems = items();
	vector<OverlayComponent*>::const_iterator iter = menuItems.begin();
	while (iter != menuItems.end())
	{
		OverlayComponent* item = *iter;
		Vec3 pos = mouseEvent.getAbsolutePosition() - getPosition();
		if (intersects(pos, item->getBounds()))
		{
			if (!item->wasMouseLastHovered())
			{
				if (isOpen())
				{
					// Hover
					if (items().front() != item)
					{
						item->getStyle().setBackgroundColor(_mouseOverBackgroundColor);
					}
					item->markBackgroundAsDirty();
					rndrManager->invalidateRenderer();
				}
				item->setMouseLastHovered(true);
			}
			
			item->onMouseMove(mouseEvent);
		}
		else if (item->wasMouseLastHovered())
		{
			// Unhover
			if (items().front() != item)
			{
				// Skip coloring the selected item
				item->getStyle().setBackgroundColor(ColorVal(0, 0, 0, 0));	
			}
			item->setMouseLastHovered(false);
			item->markBackgroundAsDirty();
			rndrManager->invalidateRenderer();
		}
		iter++;
	}
	return false;
}

/*	------------------------
	    DropDownMenuItem    
	------------------------ */

DropDownMenuItem::DropDownMenuItem()
{}

DropDownMenuItem::~DropDownMenuItem()
{}

bool DropDownMenuItem::onMouseDown(MouseOverlayEvent& mouseEvent)
{
	DropDownMenu* dropDownMenuParent = dynamic_cast<DropDownMenu*>(getParent());
	if (shouldClose(mouseEvent))
	{
		onClose();
		return true;
	}

	if (dropDownMenuParent)
		dropDownMenuParent->select(this);
	onSelect();
	return true;
}

bool DropDownMenuItem::onMouseUp(MouseOverlayEvent& mouseEvent)
{
	return true;
}

bool DropDownMenuItem::onMouseMove(MouseOverlayEvent& mouseEvent)
{
	return true;
}

void DropDownMenuItem::onClose()
{}

bool DropDownMenuItem::shouldClose(MouseOverlayEvent& mouseEvent)
{
	return false;
}