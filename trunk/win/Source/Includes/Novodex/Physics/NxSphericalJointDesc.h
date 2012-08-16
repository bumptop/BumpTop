#ifndef NX_PHYSICS_NXSPHEREJOINTDESC
#define NX_PHYSICS_NXSPHEREJOINTDESC
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

#include "Nxp.h"
#include "NxJointLimitPairDesc.h"
#include "NxSpringDesc.h"
#include "NxJointDesc.h"

class NxActor;

enum NxSphericalJointFlag
	{
	NX_SJF_TWIST_LIMIT_ENABLED = 1 << 0,//!< true if the twist limit is enabled
	NX_SJF_SWING_LIMIT_ENABLED = 1 << 1,//!< true if the swing limit is enabled
	NX_SJF_TWIST_SPRING_ENABLED= 1 << 2,//!< true if the twist spring is enabled
	NX_SJF_SWING_SPRING_ENABLED= 1 << 3,//!< true if the swing spring is enabled
	NX_SJF_JOINT_SPRING_ENABLED= 1 << 4,//!< true if the joint spring is enabled
	};

/**
Desc class for sphere joint.
*/
class NxSphericalJointDesc : public NxJointDesc
	{
	public:
	NxVec3 swingAxis;		//!<swing limit axis defined in the joint space of actor 0 ([localNormal[0], localAxis[0]^localNormal[0],localAxis[0]])
/**
	projectionDistance: if flags.projectionMode is 1, the joint gets artificially projected together when it drifts more than this distance. Sometimes it is not possible to project (for example when the joints form a cycle)
	Should be nonnegative. However, it may be a bad idea to always project to a very small or zero distance because the solver *needs* some error in order to produce correct motion.
*/
	NxReal projectionDistance;	
	//limits:
	NxJointLimitPairDesc twistLimit;		//!< limits rotation around twist axis
	NxJointLimitDesc swingLimit;			//!< limits swing of twist axis
	//spring + damper:
	NxSpringDesc	 twistSpring;			//!< spring that works against twisting
	NxSpringDesc	 swingSpring;			//!< spring that works against swinging
	NxSpringDesc	 jointSpring;			//!< spring that lets the joint get pulled apart

	NxU32 flags;							//!< This is a combination of the bits defined by ::NxSphericalJointFlag . 
	NxJointProjectionMode projectionMode;	//!< use this to enable joint projection

	/**
	constructor sets to default.
	*/
	NX_INLINE NxSphericalJointDesc();	
	/**
	(re)sets the structure to the default.	
	*/
	NX_INLINE void setToDefault(bool fromCtor = false);
	/**
	returns true if the current settings are valid
	*/
	NX_INLINE bool isValid() const;
	};

NX_INLINE NxSphericalJointDesc::NxSphericalJointDesc() : NxJointDesc(NX_JOINT_SPHERICAL)	//constructor sets to default
	{
	setToDefault(true);
	}

NX_INLINE void NxSphericalJointDesc::setToDefault(bool fromCtor)
	{
	NxJointDesc::setToDefault();

	swingAxis.set(0,0,1);

	if (!fromCtor)
		{
		//this is redundant if we're being called from the ctor:
		twistLimit.setToDefault();
		swingLimit.setToDefault();
		twistSpring.setToDefault();
		swingSpring.setToDefault();
		jointSpring.setToDefault();
		}

	projectionDistance = 1.0f;

	flags = 0;
	projectionMode = NX_JPM_NONE;
	}

NX_INLINE bool NxSphericalJointDesc::isValid() const
	{
	//check unit vectors
	if (swingAxis.magnitudeSquared() < 0.9f) return false;
	if (projectionDistance < 0.0f) return false;

	if (!twistLimit.isValid()) return false;
	if (!swingLimit.isValid()) return false;
	if (!swingSpring.isValid()) return false;
	if (!twistSpring.isValid()) return false;
	if (!jointSpring.isValid()) return false;

	return NxJointDesc::isValid();
	}

#endif
