#ifndef NX_PHYSICS_NXSPRINGANDDAMPEREFFECTORDESC
#define NX_PHYSICS_NXSPRINGANDDAMPEREFFECTORDESC
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

#include "Nxp.h"

/**
Desc class for NxSpringAndDamperEffector.  TODO: This is not yet/anymore used for anything useful!!
*/
class NxSpringAndDamperEffectorDesc
	{
	public:

	/**
	constructor sets to default.
	*/
	NX_INLINE NxSpringAndDamperEffectorDesc();	
	/**
	(re)sets the structure to the default.	
	*/
	NX_INLINE void setToDefault();
	/**
	returns true if the current settings are valid
	*/
	NX_INLINE bool isValid() const;

	};

NX_INLINE NxSpringAndDamperEffectorDesc::NxSpringAndDamperEffectorDesc()	//constructor sets to default
	{
	setToDefault();
	}

NX_INLINE void NxSpringAndDamperEffectorDesc::setToDefault()
	{
	}

NX_INLINE bool NxSpringAndDamperEffectorDesc::isValid() const
	{
	return true;
	}

#endif
