#ifndef NX_PHYSICS_NXJOINTLIMITPAIRDESC
#define NX_PHYSICS_NXJOINTLIMITPAIRDESC
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/
#include "NxJointLimitDesc.h"

/**
Describes a pair of joint limits
*/
class NxJointLimitPairDesc
	{
	public:
	NxJointLimitDesc low;		//!< The low limit (smaller value)
	NxJointLimitDesc high;		//!< the high limit (larger value)

	NX_INLINE NxJointLimitPairDesc();
	NX_INLINE void setToDefault();
	NX_INLINE bool isValid() const;
	};

NX_INLINE NxJointLimitPairDesc::NxJointLimitPairDesc()
	{
	setToDefault();
	}

NX_INLINE void NxJointLimitPairDesc::setToDefault()
	{
	//nothing
	}

NX_INLINE bool NxJointLimitPairDesc::isValid() const
	{
	return (low.isValid() && high.isValid() && low.value <= high.value);
	}

#endif
