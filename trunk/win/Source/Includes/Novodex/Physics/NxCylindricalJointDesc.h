#ifndef NX_PHYSICS_NXSLIDINGJOINTDESC
#define NX_PHYSICS_NXSLIDINGJOINTDESC
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
Desc class for sliding joint.
*/
class NxCylindricalJointDesc : public NxJointDesc
	{
	public:
	/**
	constructor sets to default.
	*/
	NX_INLINE NxCylindricalJointDesc();	
	/**
	(re)sets the structure to the default.	
	*/
	NX_INLINE void setToDefault();
	/**
	returns true if the current settings are valid
	*/
	NX_INLINE bool isValid() const;

	};

NX_INLINE NxCylindricalJointDesc::NxCylindricalJointDesc() : NxJointDesc(NX_JOINT_CYLINDRICAL)	//constructor sets to default
	{
	setToDefault();
	}

NX_INLINE void NxCylindricalJointDesc::setToDefault()
	{
	NxJointDesc::setToDefault();
	}

NX_INLINE bool NxCylindricalJointDesc::isValid() const
	{
	return NxJointDesc::isValid();
	}

#endif
