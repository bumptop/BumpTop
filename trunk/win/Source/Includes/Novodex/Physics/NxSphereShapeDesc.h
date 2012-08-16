#ifndef NX_COLLISION_NXSPHERESHAPEDESC
#define NX_COLLISION_NXSPHERESHAPEDESC
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

#include "NxShapeDesc.h"

/**
Descriptor class for NxSphereShape.
*/
class NxSphereShapeDesc : public NxShapeDesc
	{
	public:
	NxReal		radius;			//!< radius of shape. Must be positive.

	/**
	constructor sets to default.
	*/
	NX_INLINE					NxSphereShapeDesc();	
	/**
	(re)sets the structure to the default.	
	*/
	NX_INLINE virtual	void	setToDefault();
	/**
	returns true if the current settings are valid
	*/
	NX_INLINE virtual	bool	isValid() const;
	};

NX_INLINE NxSphereShapeDesc::NxSphereShapeDesc() : NxShapeDesc(NX_SHAPE_SPHERE)	//constructor sets to default
	{
	setToDefault();
	}

NX_INLINE void NxSphereShapeDesc::setToDefault()
	{
	NxShapeDesc::setToDefault();
	radius = 0.0f;
	}

NX_INLINE bool NxSphereShapeDesc::isValid() const
	{
	if(!NxMath::isFinite(radius))	return false;
	if(radius<=0.0f)				return false;
	return NxShapeDesc::isValid();
	}

#endif
