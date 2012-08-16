#ifndef NX_COLLISION_NXCAPSULESHAPEDESC
#define NX_COLLISION_NXCAPSULESHAPEDESC
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

#include "NxShapeDesc.h"

/**
Descriptor class for NxCapsuleShape.
*/
class NxCapsuleShapeDesc : public NxShapeDesc
	{
	public:
	NxReal		radius;			//!< radius of the capsule's hemispherical ends and its trunk.
	NxReal		height;			//!< the distance between the two hemispherical ends of the capsule. The height is along the capsule's Y axis. 

	/**
	constructor sets to default.
	*/
	NX_INLINE					NxCapsuleShapeDesc();	
	/**
	(re)sets the structure to the default.	
	*/
	NX_INLINE virtual	void	setToDefault();
	/**
	returns true if the current settings are valid
	*/
	NX_INLINE virtual	bool	isValid() const;
	};

NX_INLINE NxCapsuleShapeDesc::NxCapsuleShapeDesc() : NxShapeDesc(NX_SHAPE_CAPSULE)	//constructor sets to default
	{
	setToDefault();
	}

NX_INLINE void NxCapsuleShapeDesc::setToDefault()
	{
	NxShapeDesc::setToDefault();
	radius = 0.0f;
	height = 0.0f;
	}

NX_INLINE bool NxCapsuleShapeDesc::isValid() const
	{
	if(!NxMath::isFinite(radius))	return false;
	if(radius<=0.0f)				return false;
	if(!NxMath::isFinite(height))	return false;
	if(height<=0.0f)				return false;
	return NxShapeDesc::isValid();
	}

#endif
