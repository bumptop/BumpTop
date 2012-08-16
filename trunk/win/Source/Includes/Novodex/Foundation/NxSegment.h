#ifndef NX_FOUNDATION_NXSEGMENT
#define NX_FOUNDATION_NXSEGMENT
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

#include "Nxf.h"
#include "NxVec3.h"


class NxSegment;
NX_C_EXPORT NXF_DLL_EXPORT NxF32 NX_CALL_CONV NxComputeSquareDistance(const NxSegment& seg, const NxVec3& point, NxF32* t);

class NxSegment
	{
	public:
	/**
	Constructor
	*/
	NX_INLINE NxSegment()
		{
		}

	/**
	Constructor
	*/
	NX_INLINE NxSegment(const NxVec3& _p0, const NxVec3& _p1) : p0(_p0), p1(_p1)
		{
		}

	/**
	Copy constructor
	*/
	NX_INLINE NxSegment(const NxSegment& seg) : p0(seg.p0), p1(seg.p1)
		{
		}

	/**
	Destructor
	*/
	NX_INLINE ~NxSegment()
		{
		}

	NX_INLINE const NxVec3& getOrigin() const
		{
		return p0;
		}

	NX_INLINE NxVec3 computeDirection() const
		{
		return p1 - p0;
		}

	NX_INLINE void computeDirection(NxVec3& dir) const
		{
		dir = p1 - p0;
		}

	NX_INLINE NxF32 computeLength() const
		{
		return p1.distance(p0);
		}

	NX_INLINE NxF32 computeSquareLength() const
		{
		return p1.distanceSquared(p0);
		}

	NX_INLINE void setOriginDirection(const NxVec3& origin, const NxVec3& direction)
		{
		p0 = p1 = origin;
		p1 += direction;
		}

	/**
	Computes a point on the segment
	\param		pt	[out] point on segment
	\param		t	[in] point's parameter [t=0 => pt = mP0, t=1 => pt = mP1]
	 */
	NX_INLINE void computePoint(NxVec3& pt, NxF32 t) const
		{
		pt = p0 + t * (p1 - p0);
		}

	NX_INLINE NxF32 squareDistance(const NxVec3& point, NxF32* t=NULL) const
		{
		return NxComputeSquareDistance(*this, point, t);
		}

	NX_INLINE NxF32 distance(const NxVec3& point, NxF32* t=NULL) const
		{
		return sqrtf(squareDistance(point, t));
		}

	NxVec3	p0;		//!< Start of segment
	NxVec3	p1;		//!< End of segment
	};

#endif
