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

#ifndef _BT_STACK_ICON_
#define _BT_STACK_ICON_

// -----------------------------------------------------------------------------

#include "BT_MouseEventHandler.h"
#include "BT_Renderable.h"

class Pile;

// -----------------------------------------------------------------------------

class MakeStackIcon : public Renderable, public MouseEventHandler
{
	Q_DECLARE_TR_FUNCTIONS(MakeStackIcon)
		
	// Actions to perform when target crossed
	bool		enabled;
	bool		active;
	Vec3		position;
	int			numTimesCrossed;
	float		size;
	float		alphaInc;
	bool		prevPointEntered;
	Pile		*vertStackPile;
	Vec2		lastMousePt;
	boost::function<void(Pile*)> _onCrossedHandler;

	// Private Actions
	void		doCross();
	void		undoCross();

public:

	MakeStackIcon();
	~MakeStackIcon();

	// Actions
	void		update();
	void		reset();
	void		onRender(uint flags = RenderSideless);
	bool		checkForCross(Vec3 currPoint, Vec3 prevPoint);
	void		onMouseMove(Vec2 &pt);

	// Getters
	bool		isEnabled();
	Vec3		getPosition() const;
	bool		isActive() const;

	// Setters
	void		setEnable(bool e);
	Vec3		getCorner(int i);
	void		setPosition(Vec3 val);
	void		setActive(bool val);
	void		setOnCrossedHandler(boost::function<void(Pile*)> onCrossedHandler);
};

// -----------------------------------------------------------------------------

#else
	class MakeStackIcon;
#endif