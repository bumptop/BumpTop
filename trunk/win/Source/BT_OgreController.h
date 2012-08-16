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

#ifndef BT_OGRECONTROLLER_H
#define BT_OGRECONTROLLER_H

#include "BT_Singleton.h"
#include "BT_OgreEventListener.h"

#if PRE_OGRE_MIGRATION
#else
class OgreStaticObjects : public OgreEventListener
{
	QList<Ogre::SceneNode *> _walls;
	QList<Ogre::ManualObject *> _wallObjects;
	QList<Ogre::SceneNode *> _floors;
	QList<Ogre::ManualObject *> _floorObjects;

private:
	void initializeFloor();
	void initializeWalls();

public:
	OgreStaticObjects();

	void init();

	// ogre events for static objects
	virtual void onOgreUpdate();
	virtual void onOgreRender();
	virtual void onOgrePostRender();
	virtual void onOgreTextureChange(const QString& texId, const Vec3& newDims, bool loadSucceeded);
};

class OgreController
{
	// NOTE: listeners are not managed by the ogre controller
	QList<OgreEventListener *> _listeners;
	Ogre::Root * _ogreRoot;
	QHash<QString, Ogre::Overlay *> _ogreOverlays;

	// static ogre objects
	OgreStaticObjects * _ogreStaticObjects;

private:
	friend class Singleton<OgreController>;
	OgreController();

public:
	~OgreController();

	bool init();
	bool initRenderSystem(HWND window);
	bool initStaticObjects();

	void addOgreEventListener(OgreEventListener * listener);
	void removeOgreEventListener(OgreEventListener * listener);
	/*
	bool containsOgreEventListener(OgreEventListener * listener);
	const QList<OgreEventListener *>& getOgreEventListeners() const;
	*/

	Ogre::Overlay * getOverlayLayer() const;

	void timerUpdate(int ms);
	void renderUpdate();
	void postRenderUpdate();
	void textureChanged(const QString& texId, const Vec3& newDims, bool loadSucceeded);
};

// convenience define
#define ogreController Singleton<OgreController>::getInstance()
#endif

#endif // BT_OGRECONTROLLER_H