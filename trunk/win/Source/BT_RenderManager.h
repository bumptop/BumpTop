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

#ifndef _RENDER_MANAGER_
#define _RENDER_MANAGER_

// -----------------------------------------------------------------------------

#include "BT_Renderable.h"
#include "BT_Singleton.h"
#include "BT_ThemeManager.h"

// -----------------------------------------------------------------------------

typedef BOOL (APIENTRY *PFNWGLSWAPINTERVALFARPROC)( int );

// -----------------------------------------------------------------------------

class RenderManager : public ThemeEventHandler
{
	Q_DECLARE_TR_FUNCTIONS(RenderManager)

	weak_ptr<ThemeManager> themeManagerRef;

	// OpenGL Context
	HWND					hWnd;
#ifdef DXRENDER
#else
	HDC						hDeviceContext;
	HGLRC					hRenderContext;
	PIXELFORMATDESCRIPTOR	pfd;
	int						pixelFormat;
#endif
	bool					multiSamplingEnabled;
	bool					vSyncEnabled;
	bool					renderIsNeeded;
	ColorVal				hoverColor;
	ColorVal				selColor;
	ColorVal				freshColor;

	// Objects to Render
	vector<Renderable *>	renderList;

	// light 0 positions
	float _light0Position[4];
	float _light0Direction[4];

	// Singleton
	friend class Singleton<RenderManager>;
	RenderManager();

	// Private Actions
	bool initVSync();
	bool initMultiSample();
	bool initSceneRendering();

	vector<Renderable *> sortRenderOrder(vector<Renderable *> &curList);
	void renderObjects(vector<Renderable *> &curList);

	void prepareRenderShadows();
	void renderShadows(vector<Renderable *> &curList);
	void finalizeRenderShadows();

	void prepareRenderSelectionHighlight();
	void renderSelectionHighlight(vector<Renderable *> &curList);
	void finalizeRenderSelectionHighlight();

public:

	~RenderManager();

	// Actions
	bool initGL();
	void destroy();
	bool addObject(Renderable *obj);
	bool removeObject(Renderable *obj);
	void moveObjectToIndex(Renderable * obj, unsigned int index);
	void invalidateRenderer();
	inline void swapBuffers();
	bool setVSync(bool enable);

	// Events
	void onRender();
	void onSize(int width, int height);
	virtual void onThemeChanged();

	// Getters
	unsigned int getRenderListSize();
	int getIndex(Renderable *obj);
	bool isRenderRequired();
	bool isMultisamplingEnabled() const;
	bool getVSync() const { return vSyncEnabled; }

	// Setters
	void setMultisamplingEnabled(bool state);
};

// -----------------------------------------------------------------------------

#define rndrManager Singleton<RenderManager>::getInstance()

// -----------------------------------------------------------------------------

#include "BT_RenderManager.inl"

// -----------------------------------------------------------------------------

#else
	class RenderManager;
#endif