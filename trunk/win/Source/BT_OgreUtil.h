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

#ifndef BT_OGREUTIL_H
#define BT_OGREUTIL_H

#if PRE_OGRE_MIGRATION
#else
class OgreUtil
{
public:
	static Ogre::Vector3 fromVec3(const Vec3& vec);
	static Vec3 toVec3(const Ogre::Vector3& vec);
	static Vec3 toVec3(const Ogre::Vector2& vec);
	static Ray toRay(const Ogre::Ray& ray);
	
	static Ogre::Ray window2World(int x, int y);
	static Ogre::Vector2 world2Window(const Ogre::Vector3& position);

	static Ogre::PixelFormat fromImageFormat(unsigned int devilImageFormat, bool isDXT);

	static Ogre::MaterialPtr createVertexColoredMaterial(const std::string& name);

	template<typename T>
	static Ogre::SharedPtr<T> asPtr(const Ogre::ResourcePtr& ptr)
	{
		return Ogre::SharedPtr<T>(static_cast<T*>(ptr.getPointer()));
	}
};
#endif

#endif // BT_OGREUTIL_H