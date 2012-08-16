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

#ifndef BT_OGREVIEW_H
#define BT_OGREVIEW_H

// forward declarations
namespace Ogre
{
	class Camera;
	class Root;
	class RenderSystem;
	class RenderWindow;
	class SceneManager;
	class Viewport;
}

// ogre view
class OgreView
{
	Ogre::Root * _root;
	Ogre::SceneManager * _sceneManager;
	Ogre::RenderSystem * _renderSystem;
	Ogre::RenderWindow * _renderWindow;

	Ogre::Camera * _camera;
	Ogre::Viewport * _viewport;

private:
	bool initRenderSystem(/* WINDOW */);
	bool initRenderWindow(/* WINDOW */);
	bool initCamera();
	bool initResources();
	bool initLights();

public:
	OgreView();
	~OgreView();

	bool initialize(/* WINDOW */);
	bool renderOneFrame();
};

#endif // BT_OGREVIEW_H