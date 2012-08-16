#ifndef NX_PHYSICS_NXPRISMATICJOINTDESC
#define NX_PHYSICS_NXPRISMATICJOINTDESC
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
Desc class for prismatic joint.
*/
class NxPrismaticJointDesc : public NxJointDesc
	{
	public:
	/**
	constructor sets to default.
	*/
	NX_INLINE NxPrismaticJointDesc();	
	/**
	(re)sets the structure to the default.	
	*/
	NX_INLINE void setToDefault();
	/**
	returns true if the current settings are valid
	*/
	NX_INLINE bool isValid() const;

	};

NX_INLINE NxPrismaticJointDesc::NxPrismaticJointDesc() : NxJointDesc(NX_JOINT_PRISMATIC)	//constructor sets to default
	{
	setToDefault();
	}

NX_INLINE void NxPrismaticJointDesc::setToDefault()
	{
	NxJointDesc::setToDefault();
	}

NX_INLINE bool NxPrismaticJointDesc::isValid() const
	{
	return NxJointDesc::isValid();
	}

#endif
