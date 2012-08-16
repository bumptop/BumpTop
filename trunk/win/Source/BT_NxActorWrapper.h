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

#ifndef _NX_ACTOR_
#define _NX_ACTOR_

// -----------------------------------------------------------------------------

#define PILE_SPACE_OTHER_NUMBER	9
#define PILE_SPACE_GROUP_NUMBER	8
#define PILE_SPACE_FLOOR_NUMBER	7
#define TEMPORARY_GROUP_NUMBER	6
#define TEXT_ACTOR_GROUP_NUMBER	5
#define ISOLATED_GROUP_NUMBER	4
#define WALL_GROUP_NUMBER		3
#define FLOOR_GROUP_NUMBER		2
#define DYNAMIC_GROUP_NUMBER	1

// -----------------------------------------------------------------------------

enum Axis
{
	XAxis,
	YAxis,
	ZAxis,
};

// -----------------------------------------------------------------------------

enum NxWrapperTypes
{
	NoUserData,
	UserDataAvailable,
};

// -----------------------------------------------------------------------------

class NxActorWrapper
{
protected:
	float _density;
	NxWrapperTypes userDataType;
	NxActor *actor;
	NxScene *scene;

private:
	void init(bool staticActor, bool pushIntoSceneManagerActiveList);

	// Used to check if the actor has changes requiring rendering.
	// No need to initialize since we want them to be different to cause rendering on initialization.
	Vec3 oldPosition, oldSize; 
	Quat oldOrientation;
	unsigned int consecutiveNoChange; // after some consecutive no changes, set velocities to zero to save CPU from physics

public:

	// Wrapper ctors for custom actors and scene
	NxActorWrapper(NxScene * customScene);
	NxActorWrapper(NxActor * existingActor, NxScene * customScene);
	NxActorWrapper(bool staticActor = false);
	virtual ~NxActorWrapper();

	// Getters
	inline NxActor		*getActor();
	inline NxWrapperTypes getUserDataType();
	inline bool			isFrozen();
	inline bool			isCollidable();
	inline bool			isRotatable();
	inline bool			isGravityEnabled();
	inline bool			isAxisFrozen(Axis axis);
	bool				isMoving();
	unsigned int		isRequiringRender(); // Returns 0 if render is not required; need to be called for every object after each physics tick
	inline Bounds		getBoundingBox();
	Bounds				getScreenBoundingBox();
	Box					getBox();
	Vec3				getDims();

	// Setters
	inline void			setMovementOnAxis(Axis axis, bool enable);
	inline void			setFrozen(bool frozen);
	inline void			setRotation(bool enableRot);
	inline void			setGravity(bool enableGrav);
	inline void			setCollisions(bool enableCollision);
	void				setDims(const Vec3 &s);

	// Novodex Wrapper Getters
	inline Mat34		getGlobalPose() const;
	inline Vec3			getGlobalPosition() const;
	inline Mat33		getGlobalOrientation() const;
	inline Quat			getGlobalOrientationQuat() const;
	inline NxReal		getMass() const;
	inline Vec3			getLinearVelocity() const;
	inline Vec3			getAngularVelocity() const;
	inline float		getDensity() const;
	inline bool			isDynamic() const;
	inline bool			isSleeping() const;
	inline const Mat34	&getGlobalPoseReference() const;
	inline NxU32		getNbShapes() const;
	inline NxShape		**getShapes() const;
	inline Vec3			getPointVelocity(const Vec3 &pt) const;

	// Novodex Wrapper Setters
	inline void			setGlobalPose(const Mat34 &m);
	inline void			setGlobalPosition(const Vec3 &v);
	inline void			setGlobalOrientation(const Mat33 &m);
	inline void			setGlobalOrientationQuat(const Quat &q);
	inline void			setMass(NxReal r);
	inline void			setLinearVelocity(const Vec3 &v);
	inline void			setAngularVelocity(const Vec3 &v);
	inline void			setLinearMomentum(const NxVec3&);
	inline void			setAngularMomentum(const NxVec3&);
	inline void			setForce(const NxVec3&);
	inline void			setTorque(const NxVec3&);

	// Novodex Wrapper Functionality
	inline void			wakeUp(NxF32 wakeCounterValue = NX_NUM_SLEEP_FRAMES);
	inline void			putToSleep();
	inline void			raiseActorFlag(NxActorFlag f);
	inline void			clearActorFlag(NxActorFlag f);
	inline bool			readActorFlag(NxActorFlag f) const;
	inline void			raiseBodyFlag(NxBodyFlag f);
	inline void			clearBodyFlag(NxBodyFlag f);
	inline bool			readBodyFlag(NxBodyFlag f) const;
	inline void			addForce(const Vec3& force);
	inline void			addForceAtLocalPos(const Vec3& force, const Vec3& pos);
	inline void			addForceAtPos(const Vec3& force, const Vec3& pos);
	inline void			updateMassFromShapes(NxReal density, NxReal totalMass);
	inline void			stopAllMotion();

	// Useful helpers
	inline NxPlane		getFrontFacePlane() const;
};

// -----------------------------------------------------------------------------

#include "BT_NxActorWrapper.inl"

// -----------------------------------------------------------------------------

#else
	class NxActorWrapper;
#endif