#ifndef NX_COLLISION_NXSHAPE
#define NX_COLLISION_NXSHAPE
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/
#include "Nxp.h"
#include "NxPhysicsSDK.h"
#include "NxAllocateable.h"

class NxBounds3;
class NxBoxShape;
class NxBoxShapeOffCenter;
class NxPlaneShape;
class NxSphereShape;
class NxCapsuleShape;
class NxCollisionSpace;
class NxTriangleMeshShape;
class NxActor;

enum NxShapeType
	{
	NX_SHAPE_PLANE,		//!< A physical plane
	NX_SHAPE_SPHERE,	//!< A physical sphere
	NX_SHAPE_BOX,		//!< A physical box (OBB)
	NX_SHAPE_CAPSULE,	//!< A physical capsule (LSS)
	NX_SHAPE_MESH,		//!< A physical mesh
	NX_SHAPE_COMPOUND,	//!< internal use only!
	NX_SHAPE_COUNT,

	NX_SHAPE_FORCE_DWORD = 0x7fffffff
	};

enum NxShapeFlag
	{
	NX_TRIGGER_ON_ENTER	= (1<<0),		//!< Trigger callback will be called when a shape enters the trigger volume.
	NX_TRIGGER_ON_LEAVE	= (1<<1),		//!< Trigger callback will be called after a shape leaves the trigger volume.
	NX_TRIGGER_ON_STAY	= (1<<2),		//!< Trigger callback will be called while a shape is intersecting the trigger volume.
	NX_TRIGGER_ENABLE	= NX_TRIGGER_ON_ENTER|NX_TRIGGER_ON_LEAVE|NX_TRIGGER_ON_STAY,

	NX_SF_VISUALIZATION	= (1<<3),		//!< Enable debug renderer for this shape
	};

typedef NxShapeFlag	NxTriggerFlag;		//!< For compatibility with previous SDK versions before 2.1.1

/**
Abstract base class for the various collision shapes.
An instance of a subclass can be created by calling the createShape() method of the NxActor class,
or by adding the shape descriptors into the NxActorDesc class before creating the actor.

Note: in order to avoid a naming conflict, downcast operators are isTYPE(), while up casts are getTYPE().
*/
class NxShape
	{
	protected:
	NX_INLINE					NxShape() : userData(NULL)
											{}
	virtual						~NxShape()	{}

	public:

//@SG{

//@SE{

	/**
	Retrieves the actor which this shape is associated with.
	*/
	virtual NxActor & getActor() const = 0;

	/**
	Sets which collision group this shape is part of. Default group is 0. Maximum possible group is 31.
	Collision groups are sets of shapes which may or may not be set
	to collision detect with each other, by NxPhysicsSDK::setGroupCollisionFlag()
	NxCollisionGroup is an integer between 0 and 31.
	*/
	virtual void setGroup(NxCollisionGroup) = 0;

	/**
	Retrieves the value set with the above call.
	NxCollisionGroup is an integer between 0 and 31.
	*/
	virtual NxCollisionGroup getGroup() const = 0;

//@SE}

//@SE{

	/**
	Returns a world space AABB enclosing this shape.
	*/
	virtual void getWorldBounds(NxBounds3& dest) const = 0;	

//@SE}

//@SE{

	/**
	The shape may be turned into a trigger by setting one or more of the
	above TriggerFlag-s to true. A trigger shape will not collide
	with other shapes. Instead, if a shape enters the trigger's volume, 
	a trigger event will be sent to the user via the NxUserTriggerReport::onTrigger method.
	You can set a NxUserTriggerReport object with NxScene::setUserTriggerReport().

	Since version 2.1.1 this is also used to setup generic (non-trigger) flags.
	*/
	virtual	void	setFlag(NxShapeFlag flag, bool value) = 0;

	/**
	Retrieves a flag.
	*/
	virtual	NX_BOOL	getFlag(NxShapeFlag flag) const = 0;

//@SE}

//@SE{

	/**
	The setLocal*() methods set the pose of the shape in actor space, i.e. relative
	to the actor they are owned by.
	This transformation is identity by default.
	*/
	virtual		void setLocalPose(const NxMat34&)				= 0;
	virtual		void setLocalPosition(const NxVec3&)			= 0;
	virtual		void setLocalOrientation(const NxMat33&)		= 0;

	/**
	The getLocal*() methods retrieve the pose of the shape in actor space, i.e. relative
	to the actor they are owned by.
	This transformation is identity by default.
	\deprecated { theGet*(x) methods are deprecated.  Use the new convention x = get*() instead. }
	*/
	virtual		void getLocalPose(NxMat34&)				const	= 0;
	virtual		void getLocalPosition(NxVec3&)			const	= 0;
	virtual		void getLocalOrientation(NxMat33&)		const	= 0;

#ifndef DOXYGEN
	virtual		NxMat34		getLocalPoseVal()			const	= 0;
	virtual		NxVec3		getLocalPositionVal()		const	= 0;
	virtual		NxMat33		getLocalOrientationVal()	const	= 0;
#endif
	NX_INLINE	NxMat34		getLocalPose()				const	{ return getLocalPoseVal();			}
	NX_INLINE	NxVec3		getLocalPosition()			const	{ return getLocalPositionVal();		}
	NX_INLINE	NxMat33		getLocalOrientation()		const	{ return getLocalOrientationVal();	}
//@SE}

//@SE{

	/**
	The setGlobal() calls are convenience methods which transform the passed parameter
	into the current local space of the actor and then call setLocalPose().
	*/
	virtual		void setGlobalPose(const NxMat34&)				= 0;
	virtual		void setGlobalPosition(const NxVec3&)			= 0;
	virtual		void setGlobalOrientation(const NxMat33&)		= 0;

	/**
	The getGlobal*() methods retrieve the shape's current world space pose. This is 
	the local pose multiplied by the actor's current global pose.
	\deprecated { theGet*(x) methods are deprecated.  Use the new convention x = get*() instead. }
	*/
	virtual		void getGlobalPose(NxMat34&)			const	= 0;
	virtual		void getGlobalPosition(NxVec3&)			const	= 0;
	virtual		void getGlobalOrientation(NxMat33&)		const	= 0;

	/**
	The get*Val() methods works just like the get*() methods, except they return the 
	desired value instead of copying it to a destination variable.
	*/
#ifndef DOXYGEN
	virtual		NxMat34		getGlobalPoseVal()			const	= 0;
	virtual		NxVec3		getGlobalPositionVal()		const	= 0;
	virtual		NxMat33		getGlobalOrientationVal()	const	= 0;
#endif
	NX_INLINE	NxMat34		getGlobalPose()				const	{ return getGlobalPoseVal();		}
	NX_INLINE	NxVec3		getGlobalPosition()			const	{ return getGlobalPositionVal();	}
	NX_INLINE	NxMat33		getGlobalOrientation()		const	{ return getGlobalOrientationVal();	}

//@SE}

//@SE{
	/**
	Assigns a material index to the shape.  The material index should
	have been provided by NxPhysicsSDK::addMaterial(), or a similar call.
	If the material index is invalid, it will still be recorded, but 
	the default material (at index 0) will effectively be used for simulation.
	*/
	virtual void				setMaterial(NxMaterialIndex)	= 0;

	/**
	Retrieves the material index currently assigned to the shape.
	*/
	virtual NxMaterialIndex		getMaterial() const				= 0;
//@SE}

//@SE{

	/**
	returns the type of shape.
	*/
	virtual NxShapeType  getType() const = 0;

	/**
	Type casting operator. The result may be cast to the desired subclass type.
	*/
	virtual void* is(NxShapeType) const = 0;

	/**
	attempts to perform an downcast to the type returned. Returns 0 if this object is not of the appropriate type.
	*/
	NX_INLINE NxPlaneShape*			isPlane()			{ return (NxPlaneShape*)		is(NX_SHAPE_PLANE);			}
	/**
	attempts to perform an downcast to the type returned. Returns 0 if this object is not of the appropriate type.
	*/
	NX_INLINE NxSphereShape*		isSphere()			{ return (NxSphereShape*)		is(NX_SHAPE_SPHERE);		}
	/**
	attempts to perform an downcast to the type returned. Returns 0 if this object is not of the appropriate type.
	*/
	NX_INLINE NxBoxShape*			isBox()				{ return (NxBoxShape*)			is(NX_SHAPE_BOX);			}
	/**
	attempts to perform an downcast to the type returned. Returns 0 if this object is not of the appropriate type.
	*/
	NX_INLINE NxCapsuleShape*		isCapsule()			{ return (NxCapsuleShape*)		is(NX_SHAPE_CAPSULE);		}
	/**
	attempts to perform an downcast to the type returned. Returns 0 if this object is not of the appropriate type.
	*/
	NX_INLINE NxTriangleMeshShape*	isTriangleMesh()	{ return (NxTriangleMeshShape*)	is(NX_SHAPE_MESH);			}
//@SE}

//@SE{
	/**
	Sets a name string for the object that can be retrieved with getName().  This is for debugging and is not used
	by the SDK.  The string is not copied by the SDK, only the pointer is stored.
	*/
	virtual	void			setName(const char*)		= 0;

	/**
	retrieves the name string set with setName().
	*/
	virtual	const char*		getName()			const	= 0;

//@SE}

//@SG}
	void*				userData;	//!< user can assign this to whatever, usually to create a 1:1 relationship with a user object.
	};
#endif
