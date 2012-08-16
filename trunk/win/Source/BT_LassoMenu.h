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

#ifndef _BT_LASSO_MENU_
#define _BT_LASSO_MENU_

// -----------------------------------------------------------------------------

#include "BT_Singleton.h"
#include "BT_MouseEventHandler.h"
#include "BT_ColorVal.h"
#include "BT_StackIcon.h"
#include "BT_ThemeManager.h"
#include "BT_MousePointer.h"

// -----------------------------------------------------------------------------

class LassoMenu : public MouseEventHandler, 
				  public ThemeEventHandler
{
	weak_ptr<ThemeManager> themeManagerRef;

	QList<Vec3>		actualLassoPoints;		// This is what the User Sees
	Vec3List		boundingLassoPoints;	// This is used for PointInPolygon Calculations
	Bounds			boundingLassoBounds;
	ColorVal		lassoColor;				// Lasso Color
	MakeStackIcon	stackIcon;				// StackIcon class
	float			innerCircleSize;		// The circle that comes up when the lasso is first started
	bool			hasLeftInnerCircle;		// A Trigger for leaving and entering the inner circle
	bool			innerCircle;			// Toggle the drawing of the inner circle
	Vec3			centroid;
	uint			lastLassoPtsAmt;		// Used in raycasting the latest points

private:
	// Private Actions
	void		determineSelectedActors();
	void		updateBoundingLasso(Vec3 newPt);
	void		drawInnerCircle();

	// Singleton
	friend class Singleton<LassoMenu>;
	LassoMenu();

public:

	~LassoMenu();

	// Actions
	void		init();
	void		update();
	void		disableInnerCircle();
	void		reset();

	// Events
	bool		onMouseUp(Vec3 &pt, MouseButtons button);
	bool		onMouseDown(Vec3 &pt, MouseButtons button);
	void		onMouseMove(Vec3 &pt);
	void		onRender(uint flags = RenderSideless);
	virtual void onThemeChanged();

	// Getters
	MakeStackIcon& getStackIcon();
	float		getInnerCircleSize() const;
	ColorVal	getLassoColor() const;
	Vec3		getCentroid() const;
	float		getCurvatureRatio() const;
	bool		isTailVectorPointingToStackIcon(float delta) const;
	bool		isVisible() const;

	// Setters
	void		setLassoColor(ColorVal &newColor);
};

// -----------------------------------------------------------------------------

#define lassoMenu Singleton<LassoMenu>::getInstance()

// -----------------------------------------------------------------------------

#else
	class LassoMenu;
#endif