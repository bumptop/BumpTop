#ifndef NX_PHYSICS_NXPOINTONLINEJOINT
#define NX_PHYSICS_NXPOINTONLINEJOINT
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

#include "Nxp.h"

class NxJoint;
class NxPointOnLineJointDesc;

/**
 A point on line joint constrains a point on one body to only move along
 a line attached to another body.

 The starting point of the point is defined as the anchor point. The line
 through this point is specified by its direction (axis) vector.
*/
class NxPointOnLineJoint
	{
	public:
	/**
	use this for changing a significant number of joint parameters at once.
	Use the set() methods for changing only a single property at once.
	*/
	virtual void loadFromDesc(const NxPointOnLineJointDesc&) = 0;

	/**
	writes all of the object's attributes to the desc struct  
	*/
	virtual void saveToDesc(NxPointOnLineJointDesc&) = 0;

	/**
	This class is internally a subclass of NxJoint. This operator
	is automatically used to perform an upcast.
	*/
	virtual operator NxJoint &() = 0;

	/**
	This class is internally a subclass of NxJoint. Use this
	method to perform an upcast.
	*/
	virtual NxJoint & getJoint() = 0;
	};
#endif