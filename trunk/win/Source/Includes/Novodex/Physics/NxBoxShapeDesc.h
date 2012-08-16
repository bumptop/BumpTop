#ifndef NX_COLLISION_NXBOXSHAPEDESC
#define NX_COLLISION_NXBOXSHAPEDESC
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

#include "NxShapeDesc.h"

/**
Descriptor class for NxBoxShape.
*/
class NxBoxShapeDesc : public NxShapeDesc
	{
	public:
	/** 
	Dimensions of the box. The dimensions are the 'radii' of the box, meaning 1/2 extents in x dimension, 
	1/2 extents in y dimension, 1/2 extents in z dimension. All three must be positive.
	*/
	NxVec3	dimensions;			

	/**
	constructor sets to default.
	*/
	NX_INLINE					NxBoxShapeDesc();	
	/**
	(re)sets the structure to the default.	
	*/
	NX_INLINE virtual	void	setToDefault();
	/**
	returns true if the current settings are valid
	*/
	NX_INLINE virtual	bool	isValid() const;
	};

NX_INLINE NxBoxShapeDesc::NxBoxShapeDesc() : NxShapeDesc(NX_SHAPE_BOX)	//constructor sets to default
	{
	setToDefault();
	}

NX_INLINE void NxBoxShapeDesc::setToDefault()
	{
	NxShapeDesc::setToDefault();
	dimensions.zero();
	}

NX_INLINE bool NxBoxShapeDesc::isValid() const
	{
	if(!dimensions.isFinite())			return false;
	if(dimensions.x<0.0f)				return false;
	if(dimensions.y<0.0f)				return false;
	if(dimensions.z<0.0f)				return false;
	return NxShapeDesc::isValid();
	}

#endif
