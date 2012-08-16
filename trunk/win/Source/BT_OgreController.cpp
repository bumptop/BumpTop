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
#include "BT_NxActorWrapper.h"
#include "BT_OgreController.h"
#include "BT_OgreEventListener.h"
#include "BT_RenderManager.h"
#include "BT_SceneManager.h"
#include "BT_Util.h"
#include "BT_WindowsOS.h"

#define PLUGINS_CFG "plugins.cfg"
#define DISPLAY_CFG "display.cfg"
#define RENDER_LOG "render.log"


OgreStaticObjects::OgreStaticObjects()
{}

void OgreStaticObjects::init()
{
	initializeFloor();
	initializeWalls();
}

void OgreStaticObjects::initializeFloor()
{
	vector<NxActorWrapper *> walls = GLOBAL(Walls);
	assert(walls.size() > 0);
	Ogre::ManualObject * obj = NULL;
	Vec3 mainWorkspaceDims(abs(GLOBAL(WallsPos)[2].x - GLOBAL(WallsPos)[3].x) - (getDimensions(GLOBAL(Walls)[3]).z + getDimensions(GLOBAL(Walls)[2]).z),
		abs(GLOBAL(WallsPos)[0].z - GLOBAL(WallsPos)[1].z) - (getDimensions(GLOBAL(Walls)[0]).z + getDimensions(GLOBAL(Walls)[1]).z),
		0);
	mainWorkspaceDims += Vec3(2.0f);
	mainWorkspaceDims.z = 1.0f;

	// desktop floor
	{
		obj = new Ogre::ManualObject("DesktopFloor");
		obj->begin("default");
		obj->position(1, 0, -1);
		obj->position(-1, 0, -1);
		obj->position(-1, 0, 1);
		obj->position(1, 0, 1);

		obj->quad(0, 1, 2, 3);
		obj->end();
		Ogre::SceneNode * node = rndrManager->getOgreSceneManager()->getRootSceneNode()->createChildSceneNode();
		node->setPosition(0, 0, 0);
		node->setScale(mainWorkspaceDims.x / 2.0f, mainWorkspaceDims.z, mainWorkspaceDims.y / 2.0f);
		node->attachObject(obj);
		
		// save the reference
		_floors.push_back(node);
		_floorObjects.push_back(obj);
	}
}

void OgreStaticObjects::initializeWalls()
{
	// [top, bottom, right, left] walls
	vector<NxActorWrapper *> walls = GLOBAL(Walls);
	vector<Vec3> wallsPos = GLOBAL(WallsPos);
	assert(walls.size() > 0);
	Ogre::ManualObject * obj = NULL;

	float topUVMargin = 0.025f;
	float topMargin = 2.0f;

	// top wall
	{
		Vec3 pos = walls[0]->getGlobalPosition();
		Vec3 dims = walls[0]->getDims();
		pos.z -= dims.z;

		obj = new Ogre::ManualObject("TopWall");
		obj->begin("default");
		obj->position(1, 1, 0);
		obj->position(1, -1, 0);
		obj->position(-1, -1, 0);
		obj->position(-1, 1, 0);

		obj->quad(0, 1, 2, 3);
		obj->end();
		Ogre::SceneNode * node = rndrManager->getOgreSceneManager()->getRootSceneNode()->createChildSceneNode();
		node->setPosition(pos.x, pos.y, pos.z);
		node->setScale(dims.x, dims.y, dims.z);
		node->attachObject(obj);

		// save the reference
		_walls.push_back(node);
		_wallObjects.push_back(obj);
	}

	// bottom wall
	{
		Vec3 pos = walls[1]->getGlobalPosition();
		Vec3 dims = walls[1]->getDims();
		pos.z += dims.z;

		obj = new Ogre::ManualObject("BottomWall");
		obj->begin("default");
		obj->position(-1, 1, 0);
		obj->position(-1, -1, 0);
		obj->position(1, -1, 0);
		obj->position(1, 1, 0);

		obj->quad(0, 1, 2, 3);
		obj->end();
		Ogre::SceneNode * node = rndrManager->getOgreSceneManager()->getRootSceneNode()->createChildSceneNode();
		node->setPosition(pos.x, pos.y, pos.z);
		node->setScale(dims.x, dims.y, dims.z);
		node->attachObject(obj);

		// save the reference
		_walls.push_back(node);
		_wallObjects.push_back(obj);
	}

	// right wall
	{
		Vec3 pos = walls[2]->getGlobalPosition();
		Vec3 dims = walls[2]->getDims();
		pos.x += dims.z;

		obj = new Ogre::ManualObject("RightWall");
		obj->begin("default");
		obj->position(0, 1, 1);
		obj->position(0, -1, 1);
		obj->position(0, -1, -1);
		obj->position(0, 1, -1);

		obj->quad(0, 1, 2, 3);
		obj->end();
		Ogre::SceneNode * node = rndrManager->getOgreSceneManager()->getRootSceneNode()->createChildSceneNode();
		node->setPosition(pos.x, pos.y, pos.z);
		node->setScale(dims.z, dims.y, dims.x);
		node->attachObject(obj);

		// save the reference
		_walls.push_back(node);
		_wallObjects.push_back(obj);
	}

	// left wall
	{
		Vec3 pos = walls[3]->getGlobalPosition();
		Vec3 dims = walls[3]->getDims();
		pos.x -= dims.z;

		obj = new Ogre::ManualObject("RightWall");
		obj->begin("default");
		obj->position(0, 1, -1);
		obj->position(0, -1, -1);
		obj->position(0, -1, 1);
		obj->position(0, 1, 1);

		obj->quad(0, 1, 2, 3);
		obj->end();
		Ogre::SceneNode * node = rndrManager->getOgreSceneManager()->getRootSceneNode()->createChildSceneNode();
		node->setPosition(pos.x, pos.y, pos.z);
		node->setScale(dims.z, dims.y, dims.x);
		node->attachObject(obj);

		// save the reference
		_walls.push_back(node);
		_wallObjects.push_back(obj);
	}
}

void OgreStaticObjects::onOgreUpdate()
{

}

void OgreStaticObjects::onOgreRender()
{

}

void OgreStaticObjects::onOgrePostRender()
{

}

void OgreStaticObjects::onOgreTextureChange(const QString& texId, const Vec3& newDims, bool loadSucceeded)
{
	if ("floor.desktop" == texId)
	{
		// load the ao pass
		_floorObjects[0]->setMaterialName(0, stdString(texId));
	}
	else if ("wall.top" == texId)
	{
		_wallObjects[0]->setMaterialName(0, stdString(texId));
	}
	else if ("wall.bottom" == texId)
	{
		_wallObjects[1]->setMaterialName(0, stdString(texId));
	}
	else if ("wall.right" == texId)
	{
		_wallObjects[2]->setMaterialName(0, stdString(texId));
	}
	else if ("wall.left" == texId)
	{
		_wallObjects[3]->setMaterialName(0, stdString(texId));
	}
}

OgreController::OgreController()
: _ogreRoot(NULL)
{}

OgreController::~OgreController()
{
	SAFE_DELETE(_ogreStaticObjects);
}

bool OgreController::init()
{
	_ogreRoot = new Ogre::Root(PLUGINS_CFG, DISPLAY_CFG, RENDER_LOG);
	
	// register the static objects
	_ogreStaticObjects = new OgreStaticObjects();
	addOgreEventListener(_ogreStaticObjects);

	// initialize the overlay panels, etc.
	Ogre::OverlayManager& overlayManager = Ogre::OverlayManager::getSingleton();
	Ogre::Overlay * tmpOverlay;
	tmpOverlay = overlayManager.create("overlay.primary");
	tmpOverlay->show();
	_ogreOverlays.insert("overlay.primary", tmpOverlay);

	return true;
}

bool OgreController::initRenderSystem(HWND window)
{
	return rndrManager->initRenderSystem(window);
}

bool OgreController::initStaticObjects()
{
	_ogreStaticObjects->init();

	return true;
}

void OgreController::addOgreEventListener( OgreEventListener * listener )
{
	if (!_listeners.contains(listener))
	{
		_listeners.append(listener);
	}
	else
		assert(false);
}

/*
bool OgreController::containsOgreEventListener( OgreEventListener * listener )
{
	return _listeners.contains(listener);
}

const QList<OgreEventListener *>& OgreController::getOgreEventListeners() const
{
	return _listeners;
}
*/

void OgreController::removeOgreEventListener( OgreEventListener * listener )
{
	if (_listeners.contains(listener))
	{
		_listeners.removeAt(_listeners.indexOf(listener));
	}
	else
		assert(false);
}

void OgreController::timerUpdate( int ms )
{
	QListIterator<OgreEventListener *> iter(_listeners);
	while (iter.hasNext())
	{
		iter.next()->onOgreUpdate();
	}
}

void OgreController::renderUpdate()
{
	QListIterator<OgreEventListener *> iter(_listeners);
	while (iter.hasNext())
	{
		iter.next()->onOgreRender();
	}
}

void OgreController::postRenderUpdate()
{
	QListIterator<OgreEventListener *> iter(_listeners);
	while (iter.hasNext())
	{
		iter.next()->onOgrePostRender();
	}
}

void OgreController::textureChanged(const QString& texId, const Vec3& newDims, bool loadSucceeded)
{
	QListIterator<OgreEventListener *> iter(_listeners);
	while (iter.hasNext())
	{
		iter.next()->onOgreTextureChange(texId, newDims, loadSucceeded);
	}
}

Ogre::Overlay * OgreController::getOverlayLayer() const
{
	return _ogreOverlays["overlay.primary"];
}
#endif