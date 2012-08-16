#ifndef NX_PHYSICS_NXPOINTONLINEJOINTDESC
#define NX_PHYSICS_NXPOINTONLINEJOINTDESC
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
Desc class for point-on-line joint.
*/
class NxPointOnLineJointDesc : public NxJointDesc
	{
	public:
	/**
	constructor sets to default.
	*/
	NX_INLINE NxPointOnLineJointDesc();	
	/**
	(re)sets the structure to the default.	
	*/
	NX_INLINE void setToDefault();
	/**
	returns true if the current settings are valid
	*/
	NX_INLINE bool isValid() const;

	};

NX_INLINE NxPointOnLineJointDesc::NxPointOnLineJointDesc() : NxJointDesc(NX_JOINT_POINT_ON_LINE)	//constructor sets to default
	{
	setToDefault();
	}

NX_INLINE void NxPointOnLineJointDesc::setToDefault()
	{
	NxJointDesc::setToDefault();
	}

NX_INLINE bool NxPointOnLineJointDesc::isValid() const
	{
	return NxJointDesc::isValid();
	}

#endif
