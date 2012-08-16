#ifndef NX_PHYSICS_NXEFFECTOR
#define NX_PHYSICS_NXEFFECTOR
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

#include "Nxp.h"

class NxSpringAndDamperEffector;

/**
 An effector is a class that gets called before each tick of the
 scene. At this point it may apply any permissible effect
 to the objects. 
*/
class NxEffector
	{
	public:	
	/**
	Attempts to perform an downcast to the type returned. Returns 0 if this object is not of the appropriate type.
	*/
	virtual NxSpringAndDamperEffector * isSpringAndDamperEffector() = 0;

	void * userData;	//!< user can assign this to whatever, usually to create a 1:1 relationship with a user object.
	};
#endif