#ifndef NX_PHYSICS_NXHINGEJOINTDESC
#define NX_PHYSICS_NXHINGEJOINTDESC
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

#include "Nxp.h"
#include "NxJointDesc.h"
#include "NxJointLimitPairDesc.h"
#include "NxSpringDesc.h"
#include "NxMotorDesc.h"

enum NxRevoluteJointFlag
	{
	NX_RJF_LIMIT_ENABLED = 1 << 0,			//!< true if the limit is enabled
	NX_RJF_MOTOR_ENABLED = 1 << 1,			//!< true if the motor is enabled
	NX_RJF_SPRING_ENABLED = 1 << 2,			//!< true if the spring is enabled.  The spring will only take effect if the motor is disabled.
	};

/**
Desc class for hinge joint.
*/
class NxRevoluteJointDesc : public NxJointDesc
	{
	public:
	NxJointLimitPairDesc limit;				//!< Optional limits for the angular motion of the joint. 
	NxMotorDesc			 motor;				//!< Optional motor.
	NxSpringDesc		 spring;			//!< Optional spring.

/**
	projectionDistance: if projectionMode is NX_JPM_POINT_MINDIST, the joint gets artificially projected together when it drifts more than this distance.  Sometimes it is not possible to project (for example when the joints form a cycle)
	Should be nonnegative.  However, it may be a bad idea to always project to a very small or zero distance because the solver *needs* some error in order to produce correct motion.
*/
	NxReal projectionDistance;	
/**
	same deal as projectionDistance, except this is an angle (in radians) to which angular drift is projected.
*/
	NxReal projectionAngle;

	NxU32 flags;							//!< This is a combination of the bits defined by ::NxRevoluteJointFlag . 
	NxJointProjectionMode projectionMode;	//!< use this to enable joint projection

	/**
	constructor sets to default.
	*/
	NX_INLINE NxRevoluteJointDesc();	
	/**
	(re)sets the structure to the default.	
	*/
	NX_INLINE void setToDefault(bool fromCtor = false);
	/**
	returns true if the current settings are valid
	*/
	NX_INLINE bool isValid() const;

	};

NX_INLINE NxRevoluteJointDesc::NxRevoluteJointDesc() : NxJointDesc(NX_JOINT_REVOLUTE)	//constructor sets to default
	{
	setToDefault(true);
	}

NX_INLINE void NxRevoluteJointDesc::setToDefault(bool fromCtor)
	{
	NxJointDesc::setToDefault();
	projectionDistance = 1.0f;
	projectionAngle = 0.0872f;	//about 5 degrees in radians.

	if (!fromCtor)
		{
		limit.setToDefault();
		motor.setToDefault();
		spring.setToDefault();
		}

	flags = 0;
	projectionMode = NX_JPM_NONE;
	}

NX_INLINE bool NxRevoluteJointDesc::isValid() const
	{
	if (projectionDistance < 0.0f) return false;
	if (projectionAngle < 0.02f) return false;	//if its smaller then current algo gets too close to a singularity.
	

	if (!limit.isValid()) return false;
	if (!motor.isValid()) return false;
	if (!spring.isValid()) return false;


	return NxJointDesc::isValid();
	}

#endif
