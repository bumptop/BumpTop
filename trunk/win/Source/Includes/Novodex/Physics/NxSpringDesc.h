#ifndef NX_PHYSICS_NXSPRINGDESC
#define NX_PHYSICS_NXSPRINGDESC
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

/**
Describes a joint spring. The spring is implicitly integrated, so even high spring and damper
coefficients should be robust.
*/
class NxSpringDesc
	{
	public:
	NxReal spring;		//!< spring coefficient
	NxReal damper;		//!< damper coefficient
	NxReal targetValue;	//!< target value (angle/position) of spring where the spring force is zero.

	NX_INLINE NxSpringDesc();
	NX_INLINE NxSpringDesc(NxReal spring, NxReal damper = 0, NxReal targetValue = 0);
	NX_INLINE void setToDefault();
	NX_INLINE bool isValid() const;
	};

NX_INLINE NxSpringDesc::NxSpringDesc()
	{
	setToDefault();
	}

NX_INLINE NxSpringDesc::NxSpringDesc(NxReal s, NxReal d, NxReal t)
	{
	spring = s;
	damper = d;
	targetValue = t;
	}

NX_INLINE void NxSpringDesc::setToDefault()
	{
	spring = 0;
	damper = 0;
	targetValue = 0;
	}

NX_INLINE bool NxSpringDesc::isValid() const
	{
	return (spring >= 0 && damper >= 0);
	}

#endif
