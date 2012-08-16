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

NxActor *NxActorWrapper::getActor()
{
	// Return the Actor that represents this NxActorWrapper
	return actor;
}

NxWrapperTypes NxActorWrapper::getUserDataType()
{
	return userDataType;
}

Mat34 NxActorWrapper::getGlobalPose() const
{
	return actor->getGlobalPose();
}

Vec3 NxActorWrapper::getGlobalPosition() const
{
	return actor->getGlobalPosition();
}

Mat33 NxActorWrapper::getGlobalOrientation() const
{
	return actor->getGlobalOrientation();
}

Quat NxActorWrapper::getGlobalOrientationQuat() const
{
	return actor->getGlobalOrientationQuat();
}

NxReal NxActorWrapper::getMass() const
{
	return actor->getMass();
}

bool NxActorWrapper::isDynamic() const
{
	return actor->isDynamic();
}

bool NxActorWrapper::isSleeping() const
{
	return actor->isSleeping();
}

const Mat34 &NxActorWrapper::getGlobalPoseReference() const
{
	return actor->getGlobalPoseReference();
}

NxU32 NxActorWrapper::getNbShapes() const
{
	return actor->getNbShapes();
}

NxShape **NxActorWrapper::getShapes() const
{
	return actor->getShapes();
}

Vec3 NxActorWrapper::getLinearVelocity() const
{
	return actor->getLinearVelocity();
}

Vec3 NxActorWrapper::getAngularVelocity() const
{
	return actor->getAngularVelocity();
}

float NxActorWrapper::getDensity() const
{
	return _density;
}

Vec3 NxActorWrapper::getPointVelocity(const Vec3 &pt) const
{
	return actor->getPointVelocityVal(pt);
}

void NxActorWrapper::setGlobalPose(const Mat34 &m)
{
	actor->setGlobalPose(m);
}

void NxActorWrapper::setGlobalPosition(const Vec3 &v)
{
	actor->setGlobalPosition(v);
}

void NxActorWrapper::setGlobalOrientation(const Mat33 &m)
{
	actor->setGlobalOrientation(m);
}

void NxActorWrapper::setGlobalOrientationQuat(const Quat &q)
{
	actor->setGlobalOrientationQuat(q);
}

void NxActorWrapper::setMass(NxReal r)
{
	actor->setMass(r);
}

void NxActorWrapper::setLinearVelocity(const Vec3 &v)
{
	actor->setLinearVelocity(v);
}

void NxActorWrapper::setAngularVelocity(const Vec3 &v)
{
	actor->setAngularVelocity(v);
}

void NxActorWrapper::wakeUp(NxF32 wakeCounterValue)
{
	actor->wakeUp(wakeCounterValue);
}

void NxActorWrapper::putToSleep()
{
	actor->putToSleep();
}

void NxActorWrapper::raiseActorFlag(NxActorFlag f)
{
	actor->raiseActorFlag(f);
}

void NxActorWrapper::clearActorFlag(NxActorFlag f)
{
	actor->clearActorFlag(f);
}

bool NxActorWrapper::readActorFlag(NxActorFlag f) const
{
	return actor->readActorFlag(f);
}

void NxActorWrapper::raiseBodyFlag(NxBodyFlag f)
{
	actor->raiseBodyFlag(f);
}

void NxActorWrapper::clearBodyFlag(NxBodyFlag f)
{
	actor->clearBodyFlag(f);
}

bool NxActorWrapper::readBodyFlag(NxBodyFlag f) const
{
	return actor->readBodyFlag(f);
}

void NxActorWrapper::addForce(const Vec3& force)
{
	actor->addForce(force);
}

void NxActorWrapper::addForceAtLocalPos(const Vec3& force, const Vec3& pos)
{
	actor->addForceAtLocalPos(force, pos);
}

void NxActorWrapper::addForceAtPos(const Vec3& force, const Vec3& pos)
{
	actor->addForceAtPos(force, pos);
}

void NxActorWrapper::updateMassFromShapes(NxReal density, NxReal totalMass)
{
	actor->updateMassFromShapes(density, totalMass);
}

void NxActorWrapper::setLinearMomentum(const NxVec3 &m)
{
	actor->setLinearMomentum(m);
}

void NxActorWrapper::setAngularMomentum(const NxVec3 &m)
{
	actor->setAngularMomentum(m);
}

void NxActorWrapper::setForce(const NxVec3 &f)
{
	actor->setForce(f);
}

void NxActorWrapper::setTorque(const NxVec3 &t)
{
	actor->setTorque(t);
}

bool NxActorWrapper::isFrozen()
{
	return readBodyFlag(NX_BF_FROZEN);
}

bool NxActorWrapper::isCollidable()
{
	// if NX_AF_DISABLE_COLLISION is /not/ set, then it is rotatable
	return !readActorFlag(NX_AF_DISABLE_COLLISION);
}

bool NxActorWrapper::isRotatable()
{
	// if NX_BF_FROZEN_ROT is /not/ set, then it is rotatable
	return !readBodyFlag(NX_BF_FROZEN_ROT);
}

bool NxActorWrapper::isGravityEnabled()
{
	// if NX_BF_DISABLE_GRAVITY is /not/ set, then it is rotatable
	return !readBodyFlag(NX_BF_DISABLE_GRAVITY);
}

void NxActorWrapper::setRotation(bool enableRot)
{
	// Enable Rotation
	if (enableRot)
	{
		clearBodyFlag(NX_BF_FROZEN_ROT);
	}else{
		raiseBodyFlag(NX_BF_FROZEN_ROT);
	}
}

void NxActorWrapper::setGravity(bool enableGrav)
{
	// Enable/Disable Gravity
	if (enableGrav)
	{
		clearBodyFlag(NX_BF_DISABLE_GRAVITY);
	}else{
		raiseBodyFlag(NX_BF_DISABLE_GRAVITY);
	}
}

void NxActorWrapper::setCollisions(bool enableCollision)
{
	// Enable/Disable Collisions
	if (enableCollision)
	{
		clearActorFlag(NX_AF_DISABLE_COLLISION);
	}else{
		raiseActorFlag(NX_AF_DISABLE_COLLISION);
	}
}

void NxActorWrapper::setFrozen(bool frozen)
{
	// Freeze the Actor
	if (frozen)
	{
		raiseBodyFlag(NX_BF_FROZEN);
	}else{
		clearBodyFlag(NX_BF_FROZEN);
	}
}

void NxActorWrapper::setMovementOnAxis(Axis axis, bool enable)
{
	NxBodyFlag axisType;

	if (axis == XAxis) axisType = NX_BF_FROZEN_POS_X;
	if (axis == YAxis) axisType = NX_BF_FROZEN_POS_Y;
	if (axis == ZAxis) axisType = NX_BF_FROZEN_POS_Z;

	if (enable)
		clearBodyFlag(axisType);
	else
		raiseBodyFlag(axisType);
}

bool NxActorWrapper::isAxisFrozen(Axis axis)
{
	NxBodyFlag axisType;

	if (axis == XAxis) axisType = NX_BF_FROZEN_POS_X;
	if (axis == YAxis) axisType = NX_BF_FROZEN_POS_Y;
	if (axis == ZAxis) axisType = NX_BF_FROZEN_POS_Z;

	return readBodyFlag(axisType);
}

void NxActorWrapper::stopAllMotion()
{
	// Stops all movement
	setLinearMomentum(Vec3(0.0f));
	setLinearVelocity(Vec3(0.0f));
	setAngularMomentum(Vec3(0.0f));
	setAngularVelocity(Vec3(0.0f));
	setForce(Vec3(0.0f));
	setTorque(Vec3(0.0f));
}

Bounds NxActorWrapper::getBoundingBox()
{
	Bounds bounds;
	Box box;

	// Find out the Actor's bounding box
	actor->getShapes()[0]->isBox()->getWorldOBB(box);

	// NOTE: we could use minAABBvalues, but this is used in a tight loop
	//		 so we're just going to expand them out
	// bounds.set(minAABBvalues(box), maxAABBvalues(box));
	bounds.boundsOfOBB(box.rot, box.center, box.extents);
	return bounds;
}

// Get a plane containing the "front" face of the actor -- i.e., the face
// that points up when items are on the floor, or towards the center of the
// desktop when items are on the wall
NxPlane NxActorWrapper::getFrontFacePlane() const
{
	// We subtract the Z value because in actor coordinates, the positive Z is
	//  towards the back of the actor, and the negative Z is towards the front
	Vec3 adjustment(0, 0, -((NxActorWrapper *)this)->getDims().z);

	Mat33 ori = getGlobalOrientation();
	Vec3 pointOnPlane = getGlobalPosition() + (ori * adjustment);
	Vec3 normal = ori * Vec3(0, 0, -1);
	return NxPlane(pointOnPlane, normal);
}

