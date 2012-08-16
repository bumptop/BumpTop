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
#include "BT_Camera.h"
#include "BT_OgreUtil.h"
#include "BT_Util.h"
#include "BT_WindowsOS.h"


Ogre::Vector3 OgreUtil::fromVec3( const Vec3& vec )
{
	return Ogre::Vector3(vec.x, vec.y, vec.z);
}

Vec3 OgreUtil::toVec3( const Ogre::Vector3& vec )
{
	return Vec3(vec.x, vec.y, vec.z);
}

Vec3 OgreUtil::toVec3( const Ogre::Vector2& vec )
{
	return Vec3(vec.x, vec.y, 0.0f);
}

Ray OgreUtil::toRay( const Ogre::Ray& ray )
{
	return Ray(toVec3(ray.getOrigin()), toVec3(ray.getDirection()));
}

Ogre::Ray OgreUtil::window2World( int x, int y )
{
#if PRE_OGRE_MIGRATION
	return Ogre::Ray();
#else
	int width = winOS->GetWindowWidth();
	int height = winOS->GetWindowHeight();

	// get the ray at 0,0
	Ogre::Ray ray;
	Ogre::Camera * camera = cam->getOgreCamera();
	camera->getCameraToViewportRay(x/(float)width, y/(float)height, &ray);
	return ray;
#endif
}

Ogre::Vector2 OgreUtil::world2Window( const Ogre::Vector3& position )
{
#if PRE_OGRE_MIGRATION
	return Ogre::Vector2();
#else
	Ogre::Camera * camera = cam->getOgreCamera();
	Ogre::Vector3 hcsPosition = camera->getProjectionMatrix() * camera->getViewMatrix() * position;
	return Ogre::Vector2(0.5 - hcsPosition.x * 0.5f, hcsPosition.y * 0.5f + 0.5);
#endif
}

Ogre::PixelFormat OgreUtil::fromImageFormat( unsigned int devilImageFormat, bool isDXT )
{
	switch (devilImageFormat)
	{
	case IL_RGB:
		return isDXT ? Ogre::PF_DXT1 : Ogre::PF_BYTE_RGB;
	case IL_RGBA:
		return isDXT ? Ogre::PF_DXT5 : Ogre::PF_BYTE_RGBA;
	case IL_BGR:
		return isDXT ? Ogre::PF_DXT1 : Ogre::PF_BYTE_BGR;
	case IL_BGRA:
		return isDXT ? Ogre::PF_DXT5 : Ogre::PF_BYTE_BGRA;
	};
	return Ogre::PF_UNKNOWN;
}

Ogre::MaterialPtr OgreUtil::createVertexColoredMaterial( const std::string& name )
{
	if (!Ogre::MaterialManager::getSingleton().resourceExists(name))
	{
		Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().create(
			name, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		material->getTechnique(0)->getPass(0)->setLightingEnabled(false);
		material->getTechnique(0)->getPass(0)->setCullingMode(Ogre::CULL_NONE);
		material->setSceneBlending(Ogre::SBF_SOURCE_ALPHA, Ogre::SBF_ONE_MINUS_SOURCE_ALPHA);
		material->getTechnique(0)->getPass(0)->setVertexColourTracking(Ogre::TVC_AMBIENT);
		material->getTechnique(0)->getPass(0)->setDepthWriteEnabled(false);
		return material;
	}
	else
	{
		return Ogre::MaterialManager::getSingleton().getByName(name);
	}
}
#endif