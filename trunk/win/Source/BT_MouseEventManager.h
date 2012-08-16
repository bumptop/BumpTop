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

#ifndef _BT_MOUSE_EVENT_MANAGER_
#define _BT_MOUSE_EVENT_MANAGER_

// -----------------------------------------------------------------------------

#include "BT_Singleton.h"
#include "BT_MousePointer.h"

class Selection;

//5/14/2008
//Multitouch
//==========
//
//Multitouch exists in BumpTop. It can be used in two ways: using multiple mice, 
//and using the SmartBoard Multitouch API. The framework has been written so that
//other multitouch input devices can be added relatively easily.
//
//Although multitouch works, it's currently a hack designed to touch as little of 
//the old  mouse code as possible.
//
//The general framework is in MouseEventManager.
//
//The two device-specific frameworks are in BT_MultipleMice.cpp and BT_SmartBoard.cpp.
//Look at those as a model for how to add a new input framework.
//
//How Multitouch in BT generally works
//====================================
//
//The multitouch framework was created over the old mouse framework by a hack; 
//basically, the first touch on the display is routed to the old code, and any
//subsequent touches are routed to a more limited multitouch handler.
//
//Each mouse event is attached to an associated MousePointer object. This object
//is agnostic to the input device; a pointer could be a mouse, or a touch from 
//a finger on a SmartBoard. When MultipleMice mode is turned off, there is a 
//default MousePointer object created for the standard Windows mouse that is
//passed in with all events.
//
//For each additional touch, a selection object is created which includes all the
//objects associated with that touch. When the touch point moves, that selection
//updates to follow it; this is done by calling update on each of the additional
//selections. 
//
//In BumpTop.cpp, we call mouseManager->update in the motion callback and in the
//timer callback.
//
//Multitouch scaling and zooming/panning
//===================
//
//Scaling is implemented in BT_Actor.cpp: Actors get MouseDown and MouseUp calls
//and remember their touches. When there is more than one touch, the actor updates
//its size on each mousemove call using setDims based on how far apart the touches 
//are.
//
//Multitouch zooming/panning is implemented in the MouseEventManager. In the
//beginning of OnMouseEvent, we check to see if the touch is on the floor, and if
//so, we put it in the panTouches array. There is special logic implemented there
//for a touch on a floor; in particular, when there are two touches on the floor
//and one of the the touches moves, the code zooms in / out and pans appropriately
//using cam->onMouseMove and cam->updateContinuousScrollPoint.

// -----------------------------------------------------------------------------

class MouseEventHandler;

// -----------------------------------------------------------------------------

class MouseEventManager
{
	vector<MouseEventHandler *> handlerList;

	// We hack multitouch in the following way: when no mouse button on any mouse is pressed, and then
	// you get a MouseDown event, that mouse becomes the "primary touch." This touch is handled using
	// the normal BumpTop code. When you get a MouseUp event, that mouse ceases to be a primary touch.
	//
	// When you get a second MouseDown event, from another mouse, this is now treated as an "additional
	// touch." Additional touches are much more limited; we handle them separately, and all they do 
	// (for now) is drag the objects
	MousePointer *primaryTouch;
	hash_map<MousePointer*,Selection*> additionalTouches;

	vector<MousePointer*> panTouches;
	float initialPinchDist;
	float pinchScaleFactor;

	// qt forwards
	Qt::MouseButtons _buttons;
	QMouseEvent * _prevMouseDown;

	// Singleton
	friend class Singleton<MouseEventManager>;
	MouseEventManager();

public:
	~MouseEventManager();

	// FIX: DONT DO THIS - USE SETTERS/GETTERS!!!!
	int lastMouseButtons; // messy but old code requires it; refers to the primary touch
	int mouseButtons; // messy but old code requires it; refers to the primary touch
	int primaryTouchX;
	int primaryTouchY;

	// Event router
	void onMouseEvent(UINT message, int x, int y, short mouseWheelDelta, MousePointer* pointer);

	// Events
	void onPrimaryMouseDown(Vec2 &pt, MouseButtons button);
	void onPrimaryMouseUp(Vec2 &pt, MouseButtons button);
	void onPrimaryMouseMove(Vec2 &pt);

	void onAdditionalMouseDown(Vec2 &pt, MouseButtons button, MousePointer* pointer);
	void onAdditionalMouseUp(Vec2 &pt, MouseButtons button, MousePointer* pointer);
	void onAdditionalMouseMove(Vec2 &pt, MousePointer* pointer);

	void update();

	// Event Handler Registration
	bool addHandler(MouseEventHandler *eventHandler);
	bool removeHandler(MouseEventHandler *eventHandler);

	MousePointer *getPrimaryTouch();
};

// -----------------------------------------------------------------------------

#define mouseManager Singleton<MouseEventManager>::getInstance()

// -----------------------------------------------------------------------------

#else
	class MouseEventManager;
#endif