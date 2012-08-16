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
#include "BT_Camera.h"
#include "BT_LassoMenu.h"
#include "BT_Macros.h"
#include "BT_MouseEventHandler.h"
#include "BT_MouseEventManager.h"
#include "BT_OverlayComponent.h"
#include "BT_Pile.h"
#include "BT_RaycastReports.h"
#include "BT_SceneManager.h"
#include "BT_Selection.h"
#include "BT_Util.h"
#include "BT_WebActor.h"
#include "BT_WindowsOS.h"
#include "Bumptop.h"


// -----------------------------------------------------------------------------

MouseEventManager::MouseEventManager()
{	
	mouseButtons = 0;
	lastMouseButtons = 0;
	primaryTouch = NULL;
	primaryTouchX = 0;
	primaryTouchY = 0;
	_prevMouseDown = NULL;
}

MouseEventManager::~MouseEventManager()
{
}

void MouseEventManager::onMouseEvent(UINT message, int x, int y, short mouseWheelDelta, MousePointer* pointer)
{
	assert(pointer != NULL);
		
	// in case we didn't get the mouse up event
	if (primaryTouch != NULL && 
		!primaryTouch->isMouseLeftDown() &&
		!primaryTouch->isMouseMiddleDown() &&
		!primaryTouch->isMouseRightDown())
		primaryTouch = NULL;

	// if we don't have a primary pointer, and if this pointer isn't already down in 
	// "additional touch" mode, use the normal BumpTop mouse handlers
	if ((primaryTouch == NULL || primaryTouch == pointer) &&
		 additionalTouches.find(pointer) == additionalTouches.end()) 
	{
		primaryTouch = pointer;
		primaryTouchX = x;
		primaryTouchY = y;

		switch(message)
		{
		case WM_LBUTTONUP:
			GLOBAL(mouseUpTriggered) = true;
			if (mouseButtons & MouseButtonLeft) mouseButtons &= ~MouseButtonLeft;
			onPrimaryMouseUp(Vec2(float(x), float(y), 0.0f), MouseButtonLeft);
			break;
		case WM_RBUTTONUP:
			// Right Mouse Up
			if (mouseButtons & MouseButtonRight) mouseButtons &= ~MouseButtonRight;
			onPrimaryMouseUp(Vec2(float(x), float(y), 0.0f), MouseButtonRight);
			break;
		case WM_MBUTTONUP:
			// Middle Mouse Up
			if (mouseButtons & MouseButtonMiddle) mouseButtons &= ~MouseButtonMiddle;
			onPrimaryMouseUp(Vec2(float(x), float(y), 0.0f), MouseButtonMiddle);
			break;
		case WM_LBUTTONDOWN:
			// Left Mouse Down
			mouseButtons |= MouseButtonLeft;
			onPrimaryMouseDown(Vec2(float(x), float(y), 0.0f), MouseButtonLeft);
			break;
		case WM_RBUTTONDOWN:
			// Right Mouse Down
			mouseButtons |= MouseButtonRight;
			onPrimaryMouseDown(Vec2(float(x), float(y), 0.0f), MouseButtonRight);
			break;
		case WM_MBUTTONDOWN:
			// Middle Mouse Down
			mouseButtons |= MouseButtonMiddle;
			onPrimaryMouseDown(Vec2(float(x), float(y), 0.0f), MouseButtonMiddle);
			break;
		case WM_MOUSEWHEEL:
			// Process mouse wheel movement
			if (mouseWheelDelta > 0)
			{
				// Remove the scroll event because it does not have a mouse Up event [mvj ??]
				//button |= MouseButtonScrollUp;
				onPrimaryMouseUp(Vec2(float(x), float(y), mouseWheelDelta), MouseButtonScrollUp);
			}else{
				// Remove the scroll event because it does not have a mouse Up event [mvj ??]
				//button |= MouseButtonScrollDn;
				onPrimaryMouseUp(Vec2(float(x), float(y), mouseWheelDelta), MouseButtonScrollDn);
			}
			break;
		case WM_MOUSEMOVE:
			onPrimaryMouseMove(Vec2(float(x), float(y), 0.0f));
			break;
		default:
			MessageBox(NULL, L"unknown message", L"err", MB_OK | MB_ICONERROR);
			break;
		}

		if (pointer->isMouseLeftDown() || pointer->isMouseMiddleDown() || pointer->isMouseRightDown())
		{
			primaryTouch = pointer;
		}
		else
		{
			primaryTouch = NULL;
		}



		// Remove the scroll event because it does not have a mouse Up event
		if (mouseButtons & MouseButtonScrollUp)
		{
			mouseButtons &= ~MouseButtonScrollUp;
		}else if (mouseButtons & MouseButtonScrollDn)
		{
			mouseButtons &= ~MouseButtonScrollDn;
		}
	}
	else // if there is already a touch, we use the handlers for additional touches 
	{
		switch(message)
		{
		case WM_LBUTTONUP:
			onAdditionalMouseUp(Vec2(float(x), float(y), 0.0f), MouseButtonLeft, pointer);
			break;
		case WM_RBUTTONUP:
			onAdditionalMouseUp(Vec2(float(x), float(y), 0.0f), MouseButtonRight, pointer);
			break;
		case WM_MBUTTONUP:
			onAdditionalMouseUp(Vec2(float(x), float(y), 0.0f), MouseButtonMiddle, pointer);
			break;
		case WM_LBUTTONDOWN:
			onAdditionalMouseDown(Vec2(float(x), float(y), 0.0f), MouseButtonLeft, pointer);
			break;
		case WM_RBUTTONDOWN:
			onAdditionalMouseDown(Vec2(float(x), float(y), 0.0f), MouseButtonRight, pointer);
			break;
		case WM_MBUTTONDOWN:
			onAdditionalMouseDown(Vec2(float(x), float(y), 0.0f), MouseButtonMiddle, pointer);
			break;
		case WM_MOUSEWHEEL:
			// we don't do anything here now
			break;
		}
	}
}

void MouseEventManager::onPrimaryMouseDown(Vec2 &pt, MouseButtons button)
{
	// don't handle MouseDown events if the camera is moving or a WebActor is zoomed in
	if (cam->isAnimating())
		return;
	else if (WebActor::hasFocusedWebActor())
	{
		WebActor::zoomOutFocusedWebActor();
		return;
	}

#ifdef ENABLE_WEBKIT
	vector<BumpObject *> objs = scnManager->getBumpObjects(ObjectType(BumpActor, Webpage));
	for (int i = 0; i < objs.size(); ++i)
	{
		WebActor * actor = (WebActor *) objs[i];
		if (!actor->isFocused() && sel->getPickedActor() && (actor != sel->getPickedActor()))
			continue;

		Qt::KeyboardModifiers modifiers = 0;
		if (winOS->IsKeyDown(KeyControl))
			modifiers |= Qt::ControlModifier;
		if (winOS->IsKeyDown(KeyShift))
			modifiers |= Qt::ShiftModifier;
		if (winOS->IsKeyDown(KeyAlt))
			modifiers |= Qt::AltModifier;

		Qt::MouseButton b = Qt::NoButton;
		if (button == MouseButtonLeft)
			b = Qt::LeftButton;
		if (button == MouseButtonRight)
			b = Qt::RightButton;
		if (button == MouseButtonMiddle)
			b = Qt::MidButton;

		// we defer the mouse event to mouse up since we don't want to pass in mouse events when the
		// object is dragged in non-focused mode
		QMouseEvent * evt = new QMouseEvent(QEvent::MouseButtonPress, QPoint(pt.x, pt.y), b, _buttons, modifiers);
		if (actor->isFocused())
		{
			if (actor->onMouseEvent(evt))
			{
				_buttons |= b;
				return;
			}
		}
		else
		{
			_buttons |= b;
			SAFE_DELETE(_prevMouseDown);
			_prevMouseDown = evt;
		}
	}
#endif

	// For funsies :)
#ifndef _DEBUG
	if (GLOBAL(settings).enableDebugKeys)
#endif
	{	
		if (winOS->IsKeyDown(KeyAlt) && winOS->IsKeyDown(KeyControl) && winOS->IsKeyDown(KeyLeftShift))
		{
			ForcePush(pt);
			return;
		}
	}

	// get focus (Windows does this automatically for you normally, but we are running
	// into cases where BumpTop, on startup, will not get keyboard focus even if you
	// click on it)
	winOS->SetFocusOnWindow();

	// pass the event onto the handlers
	for (uint i = 0; i < handlerList.size(); i++)
	{
		// NOTE: special case, leave the lasso menu handler till after the overlays
		if (handlerList[i] != lassoMenu)
		{
			if (handlerList[i]->onMouseDown(pt, button))
				return;
		}
	}

	// let the overlays intercept the event
	vector<OverlayLayout *> overlays = scnManager->getOverlays();
	for (int i = 0; i < overlays.size(); ++i)
	{
		Vec3 dims = overlays[i]->getSize();
		Vec3 point(pt.x, dims.y - pt.y, 0.0f);
		if (overlays[i]->onMouseDown(MouseOverlayEvent(overlays[i], point, point, button)))
			return;
	}

	// don't lasso or handle mouse events (right-click menu) while viewing photo frames
	if (cam->inSlideshow())
	{
		// only skip non-leftmouse buttons, since we want the user to still
		// be able to swipe to move to the next image
		if (button & ~MouseButtonLeft)
			return;
	}

	// NOTE: see above, we leave the lasso menu handling for mouse down until after the 
	// overlays
	lassoMenu->onMouseDown(pt, button);
	
	// REFACTOR: get rid of this
	MouseCallback(button, GLUT_DOWN, int(pt.x), int(pt.y));
}

void MouseEventManager::onPrimaryMouseUp(Vec2 &pt, MouseButtons button)
{
	// don't handle MouseUp events if the camera is moving or a WebActor is zoomed in
	if (cam->isAnimating())
		return;
	else if (WebActor::hasFocusedWebActor())
	{
		WebActor::zoomOutFocusedWebActor();
		return;
	}

#ifdef ENABLE_WEBKIT
	vector<BumpObject *> objs = scnManager->getBumpObjects(ObjectType(BumpActor, Webpage));
	for (int i = 0; i < objs.size(); ++i)
	{
		WebActor * actor = (WebActor *) objs[i];
		if (!actor->isFocused() && !sel->isInSelection(actor))
			continue;
		if (actor->isParentType(BumpPile) && ((Pile *) actor->getParent())->getActiveLeafItem() == actor)
			continue;

		if (button == MouseButtonScrollDn ||
			button == MouseButtonScrollUp)
		{
			if (actor->isFocused() || sel->getSize() == 1)
			{
				bool vertical = true;
				QPoint pos(pt.x, pt.y);
				Qt::KeyboardModifiers modifiers = 0;
				if (winOS->IsKeyDown(KeyControl))
					modifiers |= Qt::ControlModifier;
				if (winOS->IsKeyDown(KeyShift))
				{
					modifiers |= Qt::ShiftModifier;
					vertical = false;
				}
				if (winOS->IsKeyDown(KeyAlt))
					modifiers |= Qt::AltModifier;

				QWheelEvent * evt = new QWheelEvent(pos, pos, pt.z, _buttons, modifiers, (vertical ? Qt::Vertical : Qt::Horizontal));
				actor->onWheelEvent(evt);
				return;
			}
		}
		else if ((int)_buttons)
		{
			Qt::KeyboardModifiers modifiers = 0;
			if (winOS->IsKeyDown(KeyControl))
				modifiers |= Qt::ControlModifier;
			if (winOS->IsKeyDown(KeyShift))
				modifiers |= Qt::ShiftModifier;
			if (winOS->IsKeyDown(KeyAlt))
				modifiers |= Qt::AltModifier;

			Qt::MouseButton b = Qt::NoButton;
			if (button == MouseButtonLeft)
				b = Qt::LeftButton;
			if (button == MouseButtonRight)
				b = Qt::RightButton;
			if (button == MouseButtonMiddle)
				b = Qt::MidButton;

			QMouseEvent * evt = new QMouseEvent(QEvent::MouseButtonRelease, QPoint(pt.x, pt.y), b, _buttons, modifiers);
			if (_prevMouseDown)
			{
				// send the saved mouse down event first if necessary
				actor->onMouseEvent(_prevMouseDown);
				_prevMouseDown = NULL;
			}
			actor->onMouseEvent(evt);
			_buttons &= ~b;
			if (actor->isFocused())
				return;
		}
	}
#endif

	// let the overlays intercept the event
	vector<OverlayLayout *> overlays = scnManager->getOverlays();
	for (int i = 0; i < overlays.size(); ++i)
	{
		Vec3 dims = overlays[i]->getSize();
		Vec3 point(pt.x, dims.y - pt.y, 0.0f);
		if (overlays[i]->onMouseUp(MouseOverlayEvent(overlays[i], point, point, button)))
			return;
	}

	// REFACTOR: get rid of this
	MouseCallback(button, GLUT_UP, int(pt.x), int(pt.y));

	for (uint i = 0; i < handlerList.size(); i++)
	{
		// Send all handlers a mouse up
		handlerList[i]->onMouseUp(pt, button);
	}
}

void MouseEventManager::onPrimaryMouseMove(Vec2 &pt)
{
#ifdef ENABLE_WEBKIT
	vector<BumpObject *> objs = scnManager->getBumpObjects(ObjectType(BumpActor, Webpage));
	for (int i = 0; i < objs.size(); ++i)
	{
		// disable clicking on the widget
		WebActor * actor = (WebActor *) objs[i];

		Qt::KeyboardModifiers modifiers = 0;
		if (winOS->IsKeyDown(KeyControl))
			modifiers |= Qt::ControlModifier;
		if (winOS->IsKeyDown(KeyShift))
			modifiers |= Qt::ShiftModifier;
		if (winOS->IsKeyDown(KeyAlt))
			modifiers |= Qt::AltModifier;
		
		if (actor->isFocused())
		{
			bool handled = false;

			QMouseEvent * evt = new QMouseEvent(QEvent::MouseMove, QPoint(pt.x, pt.y), Qt::NoButton, _buttons, modifiers);
			if (actor->onMouseEvent(evt))
				handled = true;

			// ensure that the buttons are actually down still 
			if (_buttons.testFlag(Qt::LeftButton) && !winOS->IsButtonDown(MouseButtonLeft))
			{
				onPrimaryMouseUp(pt, MouseButtonLeft);
				handled = true;
			}
			if (_buttons.testFlag(Qt::MidButton) && !winOS->IsButtonDown(MouseButtonMiddle))
			{
				onPrimaryMouseUp(pt, MouseButtonMiddle);
				handled = true;
			}
			if (_buttons.testFlag(Qt::RightButton) && !winOS->IsButtonDown(MouseButtonRight))
			{
				onPrimaryMouseUp(pt, MouseButtonRight);
				handled = true;
			}
			if (handled)
				return;
		}
		else
		{
			SAFE_DELETE(_prevMouseDown);
			_buttons = 0;
		}
	}
#endif

	// let the overlays intercept the event
	vector<OverlayLayout *> overlays = scnManager->getOverlays();
	for (int i = 0; i < overlays.size(); ++i)
	{
		Vec3 dims = overlays[i]->getSize();
		Vec3 point(pt.x, dims.y - pt.y, 0.0f);
		if (overlays[i]->onMouseMove(MouseOverlayEvent(overlays[i], point, point, 0)))
			return;
	}

	for (uint i = 0; i < handlerList.size(); i++)
	{
		// Send all handlers a mouse Down
		handlerList[i]->onMouseMove(pt);
	}

	// REFACTOR: get rid of this
	MotionCallback(int(pt.x), int(pt.y));
}

bool MouseEventManager::addHandler(MouseEventHandler *eventHandler)
{
	// Add this handler to the list
	handlerList.push_back(eventHandler);

	return true;
}

bool MouseEventManager::removeHandler(MouseEventHandler *eventHandler)
{
	// Search through all the items and remove the requested handler
	for (uint i = 0; i < handlerList.size(); i++)
	{
		if (handlerList[i] == eventHandler)
		{
			// Remove form the handler list
			handlerList.erase(handlerList.begin() + i);
			return true;
		}
	}

	// Event Handler not found!
	return false;
}


void MouseEventManager::onAdditionalMouseDown(Vec2 &pt, MouseButtons button, MousePointer* pointer)
{
	tuple<NxActorWrapper*, BumpObject*, Vec3> t = pick(int(pt.x), int(pt.y));
	BumpObject *pickedObject = t.get<1>();
	Vec3 stabPointActorSpace = t.get<2>();


	if (pickedObject != NULL && (pickedObject->isBumpObjectType(BumpActor) || pickedObject->isBumpObjectType(BumpPile)))
	{
		Selection *selection = new Selection();
		selection->add(pickedObject);
		selection->setPickedActor(pickedObject);
		selection->setStabPointActorSpace(stabPointActorSpace);
		pickedObject->onDragBegin();
		if (pickedObject->isBumpObjectType(BumpActor)) ((Actor*)pickedObject)->onTouchDown(pt, pointer);


		additionalTouches[pointer] = selection;
	}
}

void MouseEventManager::onAdditionalMouseUp(Vec2 &pt, MouseButtons button, MousePointer* pointer)
{
	if (additionalTouches.find(pointer) != additionalTouches.end())
	{
		Selection *selection = additionalTouches[pointer];
		selection->getPickedActor()->onDragEnd();
		if (selection->getPickedActor()->isBumpObjectType(BumpActor)) ((Actor*)selection->getPickedActor())->onTouchUp(pt, pointer);
		additionalTouches.erase(pointer);
		delete selection;
	}

}

void MouseEventManager::onAdditionalMouseMove(Vec2 &pt, MousePointer* pointer)
{
	if (additionalTouches.find(pointer) != additionalTouches.end())
	{
		Selection *selection = additionalTouches[pointer];
		selection->update();
	}
}


void MouseEventManager::update()
{
	for (hash_map<MousePointer*,Selection*>::iterator it = mouseManager->additionalTouches.begin();
		it != mouseManager->additionalTouches.end();
		it++)
	{
		MousePointer *mp = it->first;
		Selection * selection = it->second;

		POINT p;
		p.x = int(mp->getX());
		p.y = int(mp->getY());
		ScreenToClient(winOS->GetWindowsHandle(), &p);
		selection->update();
	}
}

MousePointer* MouseEventManager::getPrimaryTouch()
{
	return primaryTouch;
}