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

#include "BT_Common.h"
#include "BT_NxActorWrapper.h"
#include "BT_SceneManager.h"
#include "BT_UndoStack.h"
#include "BT_Util.h"

NxActorWrapper::NxActorWrapper(NxActor * existingActor, NxScene * customScene)
: actor(existingActor)
, consecutiveNoChange(0)
, scene(customScene)
{
	init(false, false);
}

NxActorWrapper::NxActorWrapper(NxScene * customScene)
: actor(0)
, consecutiveNoChange(0)
, scene(customScene)
{
	init(false, false);
}

NxActorWrapper::NxActorWrapper(bool staticActor)
: actor(0)
, consecutiveNoChange(0)
, scene(scnManager->gScene)
{
	init(staticActor, true);
}


void NxActorWrapper::init( bool staticActor, bool pushIntoSceneManagerActiveList )
{
	if (!actor)
	{
		NxBodyDesc BodyDesc;
		NxBoxShapeDesc BoxDesc;
		NxActorDesc ActorDesc;

		// Create the Actor
		BoxDesc.flags = 0;
		BoxDesc.dimensions = Vec3(1, 1, 1);
		ActorDesc.shapes.pushBack(&BoxDesc);

		if (staticActor)
			ActorDesc.body = NULL;
		else
		{
			BodyDesc.linearDamping = 0.05f;
			BodyDesc.angularDamping = 0.1f;
			ActorDesc.flags = 0;
			ActorDesc.body = &BodyDesc;
		}

		// Actor Properties
		_density = 1.0f;
		ActorDesc.density = _density;
		ActorDesc.globalPose = Mat34(true);

		// Create the Actor
		actor = scene->createActor(ActorDesc);
		actor->getShapes()[0]->setGroup(DYNAMIC_GROUP_NUMBER);
	}

	// Register it with the Scene
	if (pushIntoSceneManagerActiveList)
		scnManager->activeNxActorList.push_back(this);

	userDataType = NoUserData;
}

NxActorWrapper::~NxActorWrapper()
{
	NxShape **shapes;
	NxU32 nShapes;

	// Delete this Actor
	if (actor)
	{
		// Remove it from the Scene
		for (int i = 0; i < scnManager->activeNxActorList.size(); i++)
		{
			if (scnManager->activeNxActorList[i]->getActor() == actor)
			{
				scnManager->activeNxActorList.erase(scnManager->activeNxActorList.begin() + i);
			}
		}

		// Get at the shape of this item
		shapes = actor->getShapes();
		nShapes = actor->getNbShapes();

		// Prune the inactive list to help Novodex process
		while (nShapes--)
		{
			actor->releaseShape(*shapes[nShapes]);
			shapes[nShapes] = NULL;
		}

		// Release the actor
		scene->releaseActor(*actor);
		actor = NULL;
	}
}

Vec3 NxActorWrapper::getDims()
{
	NxShape** shapes;
	NxBoxShape* box;

	if (actor->getNbShapes() > 0)
	{
		shapes = actor->getShapes();
		box = shapes[0]->isBox();

		if (box)
		{
			// Return proper dimensions
			return box->getDimensions();
		}
		else 
		{
			// Actor does not have a Bounding Box?
			return Vec3(0.0f);
		}
	}

	return Vec3(0.0f);
}

void NxActorWrapper::setDims(const Vec3 &s)
{
	NxShape** shapes;
	NxBoxShape* box;

	// Don't allow setting of 0 or negative sizes
	if (s.x <= 0 || s.y <= 0 || s.z <= 0)
	{
		return;
	}

	if (actor->getNbShapes() > 0)
	{
		// Increase the size of the bounding box
		shapes = actor->getShapes();
		box = shapes[0]->isBox();
		box->setDimensions(s);
	}

	// Adjust mass for the new size
	if (actor->isDynamic())
		actor->updateMassFromShapes(getDensity(), 0);	
}

// This function checks weather or not the object is moving. If the object is moving
// then its velocity is ramped down to nothing. This is a fix for idle CPU cycles being
// used when object move very slightly, hardly visible to the user.
bool NxActorWrapper::isMoving()
{
	float linVelDecay = 0.8f; // Percent by which to slow down velocity
	float linVelSlowdownThreshold = 13.0f;

	// See if this actor has a velocity
	// NOTE: Is there a better way of doing this?
	if (getLinearVelocity().magnitudeSquared() > 0.1f || 
		getAngularVelocity().magnitudeSquared() > 0.1f)
	{
		// If the movement is too small, slow it down
		if (getLinearVelocity().magnitudeSquared() < linVelSlowdownThreshold)
		{
			Vec3 newVel = getLinearVelocity() * linVelDecay;
			if (newVel.magnitudeSquared() <= 0.1f)
			{
				setLinearVelocity(Vec3(0.0f));
				return false;
			}
			else 
				setLinearVelocity(newVel);
		}

		return true;
	}

	return false;
}

unsigned int NxActorWrapper::isRequiringRender()
{
	Vec3 position = getGlobalPosition();
	if ((position - oldPosition).magnitudeSquared() > 0.04f)
	{
		oldPosition = position;
		consecutiveNoChange = 0;
		return 1;
	}
	
	Quat orientation = getGlobalOrientationQuat();
	Quat orientationDiff;
	orientationDiff.setXYZW(orientation.x - oldOrientation.x,orientation.y - oldOrientation.y,orientation.z - oldOrientation.z,orientation.w - oldOrientation.w);
	if (orientationDiff.magnitudeSquared() > 0.01f)
	{
		oldOrientation = orientation;
		consecutiveNoChange = 0;
		return 2;
	}

	Vec3 size = getDims();
	if ((size - oldSize).magnitudeSquared() > 0.04f)
	{
		oldSize = size;
		consecutiveNoChange = 0;
		return 3;
	}

	consecutiveNoChange++;
	const unsigned int MINIMUM_CONSECUTIVE_NO_CHANGE_ITERATIONS = 60;
	if (consecutiveNoChange == MINIMUM_CONSECUTIVE_NO_CHANGE_ITERATIONS)
	{
		setLinearVelocity(Vec3(0.0f));
		setAngularVelocity(Vec3(0.0f));
	}
	return 0;
}

Box NxActorWrapper::getBox()
{
	NxShape ** shapes = getShapes();
	NxBoxShape * box = shapes[0]->isBox();
	Box tempBox;

	// Get the OOB of our actor
	box->getWorldOBB(tempBox);

	return tempBox;
}

Bounds NxActorWrapper::getScreenBoundingBox()
{
	// get the bounds of all the points of the bounding box projected into
	// window space
	Bounds bounds;
	Box box = getBox();
	Vec3 points[8];
	box.computePoints(points);
	for (int i = 0; i < 8; ++i)
	{
		bounds.include(WorldToClient(points[i], true, true));
	}
	return bounds;
}