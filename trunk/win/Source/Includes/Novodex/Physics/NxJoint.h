#ifndef NX_PHYSICS_NXJOINT
#define NX_PHYSICS_NXJOINT
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/
#include "Nxp.h"

class NxActor;
class NxRevoluteJoint;
class NxPointInPlaneJoint;
class NxPointOnLineJoint;
class NxPrismaticJoint;
class NxCylindricalJoint;
class NxSphericalJoint;

enum NxJointType
	{
	NX_JOINT_PRISMATIC,			//!< Permits a single translational degree of freedom.
	NX_JOINT_REVOLUTE,			//!< Also known as a hinge joint, permits one rotational degree of freedom.
	NX_JOINT_CYLINDRICAL,		//!< Formerly known as a sliding joint, permits one translational and one rotational degree of freedom.
	NX_JOINT_SPHERICAL,			//!< Also known as a ball or ball and socket joint.
	NX_JOINT_POINT_ON_LINE,		//!< A point on one actor is constrained to stay on a line on another.
	NX_JOINT_POINT_IN_PLANE,	//!< A point on one actor is constrained to stay on a plane on another.

	NX_JOINT_COUNT,				//!< Just to track the number of available enum values. Not a joint type.
	NX_JOINT_FORCE_DWORD = 0x7fffffff
	};

enum NxJointMethod
	{
	NX_JM_LAGRANGE,
	NX_JM_REDUCED
	};

enum NxJointState
	{
	NX_JS_UNBOUND,
	NX_JS_SIMULATING,
	NX_JS_BROKEN
	};


/**
 Abstract base class for the different types of joints.
 All joints are used to connect two bodies, or a body and the environment.

 A NULL body represents the environment. Whenever the below comments mention two bodies,
 one of them may always be the environment.
*/
class NxJoint
	{
	protected:
	NX_INLINE					NxJoint() : userData(NULL)
											{}
	virtual						~NxJoint()	{}

	public:

	/**
	retrieves the Actors involved.
	*/
	virtual void getActors(NxActor** actor1, NxActor** actor2) = 0;

	/**
	sets the point where the two bodies are attached, specified in global coordinates.
	set this after setting the bodies of the joint.
	*/
	virtual void setGlobalAnchor(const NxVec3 &) = 0;

	/**
	retrieves the joint anchor.
	\deprecated { theGet*(x) methods are deprecated.  Use the new convention x = get*() instead. }
	*/
	virtual void getGlobalAnchor(NxVec3 &) const = 0;

	/**
	Sets the direction of the joint's primary axis, specified in global coordinates.
	The direction vector should be normalized to unit length.
	This is only used if the sphere joint has a twist limit on it.
	*/
	virtual void setGlobalAxis(const NxVec3 &) = 0;

	/**
	Retrieves the joint axis.
	\deprecated { theGet*(x) methods are deprecated.  Use the new convention x = get*() instead. }
	*/
	virtual void getGlobalAxis(NxVec3 &) const = 0;
#ifndef DOXYGEN
	virtual		NxVec3	getGlobalAnchorVal() const = 0;
	virtual		NxVec3	getGlobalAxisVal() const = 0;
#endif
	NX_INLINE	NxVec3	getGlobalAnchor()	const {return getGlobalAnchorVal();}
	NX_INLINE	NxVec3	getGlobalAxis()		const {return getGlobalAxisVal();}

	/**
	Requests a type of simulation method for this joint. It can be either
	JM_LAGRANGE or JM_REDUCED.
	The request may not always be satisfied. 
	The JM_LAGRANGE method is faster but less accurate. It supports breakable 
	joints, cyclic configurations, but not (yet) damping.

	The JM_REDUCED method is slower (but still linear time); 
	JM_REDUCED does not yet support breakable joints or cyclic configurations.

	Some joint types are supported only in one or the other configuration.

	By default, joints try to be created in JM_REDUCED mode.
	To query the current mode, retrieve the joint flags with getFlags() and 
	inspect the bits for 
	*/
	virtual void requestMethod(NxJointMethod) = 0;

	/**
	Retrieves the current simulation method. As long as the state of the joint
	is JS_UNBOUND, the return value is only the last method request. The actual
	sim method is only determined when the joint is bound the first time the 
	simulation is advanced after creating or changing the joint, and the state becomes
	JS_SIMULATING.
	*/
	virtual NxJointMethod getMethod() = 0;

	/**
	Returns the state of the joint.
	Joints are created in the JS_UNBOUND state. Making certain changes to the simulation or the joint 
	can also make joints become unbound.
	Unbound joints are automatically bound the next time Scene::run() is called, and this changes their
	state to JS_SIMULATING. JS_BROKEN means that a breakable joint has broken due to a large force
	or one of its bodies has been deleted. In either case the joint was removed from the simulation, 
	so it should be released by the user to free up its memory.
	*/
	virtual NxJointState getState() = 0;
	
	/**
	Sets the maximum force magnitude that the joint is able
	to withstand without breaking. If the joint force rises above
	this threshold, the joint breaks, and becomes disabled. Additionally,
	the jointBreakNotify() method of the scene's user notify callback will be called.
	(You can set this with NxScene::setUserNotify()).

	There are two values, one for linear forces, and one for angular forces.

	Both force values are NX_MAX_REAL by default. This setting makes the joint unbreakable. 
	The values should always be nonnegative.
	
	Only JM_LAGRANGE joints support breakability at the moment. You can force
	JM_LAGRANGE mode using requestMethod().
	*/
	virtual void setBreakable(NxReal maxForce, NxReal maxTorque) = 0;

	/**
	Retrieves the max forces of a breakable joint. See setBreakable().
	*/
	virtual void getBreakable(NxReal & maxForce, NxReal & maxTorque) = 0;

	//!limits:
	/**
	sets the limit point. The point is specified in the global coordinate frame.

	All types of joints may be limited with the same system:
	You may elect a point attached to one of the two bodies to act as the limit point.
	You may also specify several planes attached to the other body.

	The points and planes move together with the body they are attached to.

	The simulation then makes certain that the pair of bodies only move relative to eachother 
	so that the limit point stays on the positive side of all limit planes.

	the default limit point is (0,0,0) in the local frame of body2.
	Calling this deletes all existing limit planes.
	*/

	virtual void setLimitPoint(const NxVec3 & point, bool pointIsOnBody2 = true) = 0;

	/**
	retrieves the global space limit point. Returns true if the point is fixed
	on body2.
	*/
	virtual bool getLimitPoint(NxVec3 & worldLimitPoint) = 0;

	/**
	adds a limit plane. Both of the parameters are given in global coordinates.
	see setLimitPoint() for the meaning of limit planes. the plane is affixed to the
	body that does not have the limit point.

	the normal of the plane points toward the positive side of the plane, and thus toward the
	limit point. If the normal points away from the limit point at the time of this call, the
	method returns false and the limit plane is ignored.
	*/
	virtual bool addLimitPlane(const NxVec3 & normal, const NxVec3 & pointInPlane) = 0;

	/**
	deletes all limit planes added. Invalidates limit plane iterator (see below)!
	*/
	virtual void purgeLimitPlanes() = 0;

	/**
	restarts the iteration. Call before starting
	to iterate. This method may be used together with
	the below two methods to enumerate the limit planes.
	This iterator becomes invalid when planes
	are added or removed, or the plane iterator mechanism is
	invoked on another joint.
	*/
	virtual void resetLimitPlaneIterator() = 0;

	/**
	Returns true until the iterator reaches the end of the
	sequence. Adding or removing elements does not reset
	the iterator.
	*/
	virtual bool hasMoreLimitPlanes() = 0;

	/**
	returns the next element pointed to by the iterator, and
	increments the iterator.

	returns true if this limit plane is satisfied. (should always be true)
	places the global frame plane equation (consisting of normal and d, the 4th
	element) coefficients in the argument references.
	*/
	virtual bool getNextLimitPlane(NxVec3 & planeNormal, NxReal & planeD) = 0;


	/**
	returns the type of joint.
	*/
	virtual NxJointType  getType() const = 0;

	/**
	Type casting operator. The result may be cast to the desired subclass type.
	*/
	virtual void* is(NxJointType) const = 0;

	/**
	attempts to perform an downcast to the type returned. Returns 0 if this object is not of the appropriate type.
	*/
	virtual NxRevoluteJoint* isRevoluteJoint() = 0;

	/**
	attempts to perform an downcast to the type returned. Returns 0 if this object is not of the appropriate type.
	*/
	virtual NxPointInPlaneJoint* isPointInPlaneJoint() = 0;

	/**
	attempts to perform an downcast to the type returned. Returns 0 if this object is not of the appropriate type.
	*/
	virtual NxPointOnLineJoint* isPointOnLineJoint() = 0;

	/**
	attempts to perform an downcast to the type returned. Returns 0 if this object is not of the appropriate type.
	*/
	virtual NxPrismaticJoint* isPrismaticJoint() = 0;

	/**
	attempts to perform an downcast to the type returned. Returns 0 if this object is not of the appropriate type.
	*/
	virtual NxCylindricalJoint* isCylindricalJoint() = 0;

	/**
	attempts to perform an downcast to the type returned. Returns 0 if this object is not of the appropriate type.
	*/
	virtual NxSphericalJoint* isSphericalJoint() = 0;

	/**
	Sets a name string for the object that can be retrieved with getName().  This is for debugging and is not used
	by the SDK.  The string is not copied by the SDK, only the pointer is stored.
	*/
	virtual	void			setName(const char*)		= 0;

	/**
	retrieves the name string set with setName().
	*/
	virtual	const char*		getName()			const	= 0;

	void*			userData;	//!< user can assign this to whatever, usually to create a 1:1 relationship with a user object.
	};
#endif
