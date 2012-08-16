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

#ifndef _BT_RENDERABLE_
#define _BT_RENDERABLE_

// -----------------------------------------------------------------------------

enum RenderSpace
{
	ScreenSpace,
	WorldSpace,
};

// -----------------------------------------------------------------------------

enum RenderFlags
{
	RenderSideless	= (1 << 0),
	RenderSides		= (1 << 1),
	RenderCrumpled	= (1 << 2),
	RenderPeeled	= (1 << 3),
	RenderSkipModelViewChange = (1 << 4),
	RenderIgnoreDepth = (1 << 5),
	RenderCustomDisplayList = (1 << 6),
};

// -----------------------------------------------------------------------------

class Renderable
{
protected:

	bool enabled;
	float alpha;
	RenderSpace renderType;

public:

	Renderable();
	virtual ~Renderable();

	// Actions
	virtual void onRender(uint flags = RenderSideless) = 0;
#ifdef DXRENDER
	virtual void onRelease(){}
#endif
	// Getters
	inline float getAlpha() const;
	inline RenderSpace getRenderType() const;
	inline bool isEnabled() const;

	// Setters
	inline void setEnabled(bool val);
	inline virtual void setAlpha(float val);

};

// -----------------------------------------------------------------------------

#include "BT_Renderable.inl"

// -----------------------------------------------------------------------------

#else
	class Renderable;
#endif