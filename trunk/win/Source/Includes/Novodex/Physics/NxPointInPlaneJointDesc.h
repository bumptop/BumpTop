#ifndef NX_PHYSICS_NXPOINTINPLANEJOINTDESC
#define NX_PHYSICS_NXPOINTINPLANEJOINTDESC
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

#include "Nxp.h"
#include "NxJointDesc.h"
/**
Desc class for point-in-plane joint.
*/
class NxPointInPlaneJointDesc : public NxJointDesc
	{
	public:
	/**
	constructor sets to default.
	*/
	NX_INLINE NxPointInPlaneJointDesc();	
	/**
	(re)sets the structure to the default.	
	*/
	NX_INLINE void setToDefault();
	/**
	returns true if the current settings are valid
	*/
	NX_INLINE bool isValid() const;

	};

NX_INLINE NxPointInPlaneJointDesc::NxPointInPlaneJointDesc() : NxJointDesc(NX_JOINT_POINT_IN_PLANE)	//constructor sets to default
	{
	setToDefault();
	}

NX_INLINE void NxPointInPlaneJointDesc::setToDefault()
	{
	NxJointDesc::setToDefault();
	}

NX_INLINE bool NxPointInPlaneJointDesc::isValid() const
	{
	return NxJointDesc::isValid();
	}

#endif
