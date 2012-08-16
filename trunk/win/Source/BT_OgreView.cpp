// Copyright 2011 Google Inc. All Rights Reserved.
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

#if PRE_OGRE_MIGRATION
#else
#include "BT_OgreView.h"

#include "BT_Util.h"
#include "BT_QtUtil.h"
#include "BT_WindowsOS.h"

#define RESOURCES_CFG "resources.cfg"
#define PLUGINS_CFG "plugins.cfg"
#define DISPLAY_CFG "display.cfg"
#define RENDER_LOG "render.log"

/*
struct OgreViewParams
{
float width, height;
}
*/

OgreView::OgreView()
: _root(NULL)
, _renderSystem(NULL)
{
	_root = new Ogre::Root(PLUGINS_CFG, DISPLAY_CFG, RENDER_LOG);
}

OgreView::~OgreView()
{
	delete _root;
}

bool OgreView::initialize(/* WINDOW */)
{
	// intitialize the render system
	if (!initRenderSystem())
	{
		// FAIL
		return false;
	}

	// initialize the root
	_root->initialise(false);

	// initialize the render window
	initRenderWindow();

	// create the scene manager
	_sceneManager = _root->createSceneManager(Ogre::ST_GENERIC);

	// initialize the camera
	initCamera();

	// initialize all resource
	initResources();

	// initialize lighting
	initLights();

	_root->startRendering();

	return true;
}

bool OgreView::renderOneFrame()
{
	return _root->renderOneFrame();
}

bool OgreView::initRenderSystem()
{
#ifdef _DEBUG
	const char * directxRenderSystem = "RenderSystem_Direct3D9_d";
	const char * openglRenderSystem = "RenderSystem_GL_d";
#else
	const char * directxRenderSystem = "RenderSystem_Direct3D9";
	const char * openglRenderSystem = "RenderSystem_GL";
#endif

	bool usingDirectX = true;

	// try and load each of the render systems
	try
	{
		_root->loadPlugin(directxRenderSystem);
	}
	catch (...)
	{
		try
		{
			_root->loadPlugin(openglRenderSystem);
			usingDirectX = false;
		}
		catch (...)
		{
			return false;
		}
	}

	// grab the first render system
	Ogre::RenderSystemList * renderSystems = _root->getAvailableRenderers();
	if (!renderSystems->empty())
	{
		// set the render system
		_renderSystem = renderSystems->front();
		_root->setRenderSystem(_renderSystem);

		// disable full screen
		_renderSystem->setConfigOption("Full Screen", "No");

		/*
		// set the dimensions of the window
		int monWidth = winOS->GetMonitorWidth();
		int monHeight = winOS->GetMonitorHeight();
		int bpp = winOS->GetWindowBpp();
		if (usingDirectX)
		{
		_renderSystem->setConfigOption("Video Mode", QString("%1 x %2 @ %3-bit colour").arg(monWidth).arg(monHeight).arg(bpp));
		}
		else
		{
		_renderSystem->setConfigOption("Colour Depth", QString("%1").arg(bpp));
		_renderSystem->setConfigOption("Video Mode", QString("%1 x %2").arg(monWidth).arg(monHeight));
		}

		// set FSAA
		// set vsync
		*/

		// list the available configuration properties, their current values, and the 
		// possible values
		Ogre::ConfigOptionMap& configOptions = _renderSystem->getConfigOptions();
		Ogre::ConfigOptionMap::iterator iter = configOptions.begin();
		while (iter != configOptions.end())
		{
			Ogre::_ConfigOption& opt = iter->second;
			QString possibleValStrs;
			for (int i = 0; i < opt.possibleValues.size(); ++i)
				possibleValStrs += qstring(opt.possibleValues[i]) + ", ";
			QString str = QString("  %1: %2 [%3]\n")
				.arg(qstring(opt.name))
				.arg(qstring(opt.currentValue))
				.arg(possibleValStrs);
			consoleWrite(str);
			iter++;
		}

		return true;
	}
	return false;
}

bool OgreView::initRenderWindow( /* WINDOW */ )
{
	// create the render window (MUST happen before resource initialization)
	Ogre::NameValuePairList opts;
	// opts["parentWindowHandle"] = Ogre::StringConverter::toString(HWND);
	_renderWindow = _root->createRenderWindow(
		"BumpTop",
		800, 600,
		false, &opts);

	return true;
}

bool OgreView::initCamera()
{
	// setup the default camera
	_camera = _sceneManager->createCamera("DefaultCamera");

	// setup the viewport
	_viewport = _renderWindow->addViewport(_camera);
	_viewport->setBackgroundColour(Ogre::ColourValue(0.1f, 0.1f, 0.125f, 1.0f));
	return true;
}

bool OgreView::initResources()
{
	// load the resource config file
	Ogre::ConfigFile cf;
	cf.load(RESOURCES_CFG);

	// go through each section and add the resource locations
	Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();
	std::string secName, typeName, archName;
	while (seci.hasMoreElements())
	{
		secName = seci.peekNextKey();
		Ogre::ConfigFile::SettingsMultiMap * settings = seci.getNext();
		Ogre::ConfigFile::SettingsMultiMap::iterator i;
		for (i = settings->begin(); i != settings->end(); ++i)
		{
			typeName = i->first;
			archName = i->second;
			Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
				archName, typeName, secName);
		}
	}

	Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
	Ogre::MaterialManager::getSingleton().initialise();
	return true;
}

bool OgreView::initLights()
{
	// Set ambient light
	_sceneManager->setAmbientLight(Ogre::ColourValue(0.5, 0.5, 0.5));

	Ogre::Light * light = _sceneManager->createLight("MainLight");
	Ogre::SceneNode * lightNode = _sceneManager->getRootSceneNode()->createChildSceneNode();
	lightNode->createChildSceneNode(Ogre::Vector3(0,40,0))->attachObject(light);

	return true;
}

#endif