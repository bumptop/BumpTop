#ifndef NX_COLLISION_NXPLANESHAPEDESC
#define NX_COLLISION_NXPLANESHAPEDESC
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

#include "NxShapeDesc.h"

/**
Descriptor class for NxPlaneShape.
See also the class NxPlane.
*/
class NxPlaneShapeDesc : public NxShapeDesc
	{
	public:
	NxVec3		normal;			//!< Plane normal. (unit length!)
	NxReal		d;				//!< The distance from the origin.

	/**
	constructor sets to default.
	*/
	NX_INLINE					NxPlaneShapeDesc();	
	/**
	(re)sets the structure to the default.	
	*/
	NX_INLINE virtual	void	setToDefault();
	/**
	returns true if the current settings are valid
	*/
	NX_INLINE virtual	bool	isValid() const;
	};

NX_INLINE NxPlaneShapeDesc::NxPlaneShapeDesc() : NxShapeDesc(NX_SHAPE_PLANE)	//constructor sets to default
	{
	setToDefault();
	}

NX_INLINE void NxPlaneShapeDesc::setToDefault()
	{
	NxShapeDesc::setToDefault();
	// default ground plane
	normal.set(NxReal(0.0),NxReal(1.0),NxReal(0.0));
	d=NxReal(0.0);
	}

NX_INLINE bool NxPlaneShapeDesc::isValid() const
	{
	if(!normal.isFinite())		return false;
	if(!NxMath::isFinite(d))	return false;
	return NxShapeDesc::isValid();
	}

#endif
