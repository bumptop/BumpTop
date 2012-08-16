#ifndef NX_PHYSICS_NXSPRINGANDDAMPEREFFECTOR
#define NX_PHYSICS_NXSPRINGANDDAMPEREFFECTOR
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

#include "Nxp.h"

class NxActor;
class NxEffector;

/**
 Represents a spring and damper element, which exerts a force between two bodies,
 proportional to the relative positions and the relative velocities of the bodies.
*/
class NxSpringAndDamperEffector
	{
	public:

	/**
	Sets the two bodies which are connected by the element. You may set one of the bodies
	to NULL to signify that the effect is between a body and the static environment.

	Setting both of the bodies to NULL is invalid.

	Each body parameter is followed by a point defined in the global coordinate frame, which will
	move with the respective body. This is the point where the respective end of the spring and damper
	element is attached to the body.
	*/
	virtual void setBodies(NxActor* body1, const NxVec3  & global1, NxActor* body2, const NxVec3  & global2) = 0;

	/**
	Sets the properties of the linear spring. 
	
	The first three parameters are stretch distances between the end points:

	distRelaxed is the distance at which the spring is relaxed, and there is no spring force applied.
	distCompressSaturate is the distance at which the repulsive spring force magnitude no longer increases, but stays constant.
	distStretchSaturate is the distance at which the attractive spring force magnitude no longer increases, but stays constant.

	the following has to hold: 0 <= distCompressSaturate <= distRelaxed <= distStretchSaturate

	The last two parameters are maximal force magnitudes:

	maxCompressForce is the force applied when distCompressSaturate is reached. The force ramps up linearly until this value, starting at zero at distRelaxed.
	maxStretchForce  is the force applied when distStretchSaturate  is reached. The force ramps up linearly until this value, starting at zero at distRelaxed.

	set maxCompressForce to zero to disable the compress phase.
	set maxStretchForce  to zero to disable the stretch phase.

	the following has to hold: 0 <= maxCompressForce, 0 <= maxStretchForce.

	*/
	virtual void setLinearSpring(NxReal distCompressSaturate, NxReal distRelaxed, NxReal distStretchSaturate, NxReal maxCompressForce, NxReal maxStretchForce) = 0;

	/**
	Retrieves the spring properties.
	*/
	virtual void getLinearSpring(NxReal & distCompressSaturate, NxReal & distRelaxed, NxReal & distStretchSaturate, NxReal & maxCompressForce, NxReal & maxStretchForce) = 0;

	/**
	Sets the properties of the linear damper.

	The first two parameters are relative body velocities:

	velCompressSaturate is the negative (compression direction) velocity where the damping force magnitude no longer increases, but stays constant.
	velStretchSaturate  is the positive (stretch direction) velocity where the the damping force magnitude no longer increases, but stays constant.

	the following has to hold: velCompressSaturate <= 0 <= velStretchSaturate; 

	The last two parameters are maximal force magnitudes:
	maxCompressForce is the force applied when velCompressSaturate is reached. The force ramps up linearly until this value, starting at zero at vrel == 0.
	maxStretchForce  is the force applied when velStretchSaturate  is reached. The force ramps up linearly until this value, starting at zero at vrel == 0.

	the following has to hold: 0 <= maxCompressForce; 0 <= maxStretchForce.
	*/
	virtual void setLinearDamper(NxReal velCompressSaturate, NxReal velStretchSaturate, NxReal maxCompressForce, NxReal maxStretchForce) = 0;

	/**
	Retrieves the damper properties.
	*/
	virtual void getLinearDamper(NxReal & velCompressSaturate, NxReal & velStretchSaturate, NxReal & maxCompressForce, NxReal & maxStretchForce) = 0;
	
	/**
	This class is internally a subclass of NxEffector. This operator
	is automatically used to perform an upcast.
	*/
	virtual operator NxEffector &() = 0;

	/**
	This class is internally a subclass of NxEffector. Use this
	method to perform an upcast.
	*/
	virtual NxEffector & getEffector() = 0;
	};
#endif