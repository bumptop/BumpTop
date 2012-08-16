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

#ifndef BT_OGREEVENTLISTENER_H
#define BT_OGREEVENTLISTENER_H

/*
* An implementable interface that a class can implement to handle specific 
* OGRE events (timer update, render, post render, texture loaded, etc.)
*/
class OgreEventListener
{
public:
	// used to sync bumptop/ogre objects/state after a timer update
	virtual void onOgreUpdate();

	// used to sync bumptop/ogre objects/state for a render
	virtual void onOgreRender();

	// used to sync bumptop/ogre objects/state post-render
	virtual void onOgrePostRender();

	// used to sync bumptop/ogre objects/state on texture loaded
	virtual void onOgreTextureChange(const QString& texId, const Vec3& newDims, bool loadSucceeded);
};

#endif // BT_OGREEVENTLISTENER_H