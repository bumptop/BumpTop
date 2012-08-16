#ifndef NX_PHYSICS_NXJOINTLIMITDESC
#define NX_PHYSICS_NXJOINTLIMITDESC
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

/**
Describes a joint limit.
*/
class NxJointLimitDesc
	{
	public:
	NxReal value;		//!< the angle / position beyond which the limit is active. Which side the limit restricts depends on whether this is a high or low limit.
	NxReal restitution;	//!< limit bounce
	NxReal hardness;	//!< [not yet implemented!] limit can be made softer by setting this to less than 1.

	NX_INLINE NxJointLimitDesc();
	NX_INLINE void setToDefault();
	NX_INLINE bool isValid() const;
	};

NX_INLINE NxJointLimitDesc::NxJointLimitDesc()
	{
	setToDefault();
	}

NX_INLINE void NxJointLimitDesc::setToDefault()
	{
	value = 0;
	restitution = 0;
	hardness = 1;
	}

NX_INLINE bool NxJointLimitDesc::isValid() const
	{
	return (restitution >= 0 && restitution <= 1 && hardness >= 0 && hardness <= 1);
	}

#endif
