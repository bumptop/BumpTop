#ifndef NX_FOUNDATION_NXPLANE
#define NX_FOUNDATION_NXPLANE
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

#include "Nxf.h"
#include "NxVec3.h"
#include "NxMat34.h"

class NxPlane
	{
	public:
	/**
	Constructor
	*/
	NX_INLINE NxPlane()
		{
		}

	/**
	Constructor from a normal and a distance
	*/
	NX_INLINE NxPlane(NxF32 nx, NxF32 ny, NxF32 nz, NxF32 d)
		{
		set(nx, ny, nz, d);
		}

	/**
	Constructor from a point on the plane and a normal
	*/
	NX_INLINE NxPlane(const NxVec3& p, const NxVec3& n)
		{
		set(p, n);
		}

	/**
	Constructor from three points
	*/
	NX_INLINE NxPlane(const NxVec3& p0, const NxVec3& p1, const NxVec3& p2)
		{
		set(p0, p1, p2);
		}

	/**
	Constructor from a normal and a distance
	*/
	NX_INLINE NxPlane(const NxVec3& _n, NxF32 _d) : normal(_n), d(_d)
		{
		}

	/**
	Copy constructor
	*/
	NX_INLINE NxPlane(const NxPlane& plane) : normal(plane.normal), d(plane.d)
		{
		}

	/**
	Destructor
	*/
	NX_INLINE ~NxPlane()
		{
		}

	NX_INLINE NxPlane& zero()
		{
		normal.zero();
		d = 0.0f;
		return *this;
		}

	NX_INLINE NxPlane& set(NxF32 nx, NxF32 ny, NxF32 nz, NxF32 _d)
		{
		normal.set(nx, ny, nz);
		d = _d;
		return *this;
		}

	NX_INLINE NxPlane& set(const NxVec3& _normal, NxF32 _d)
		{
		normal = _normal;
		d = _d;
		return *this;
		}

	NX_INLINE NxPlane& set(const NxVec3& p, const NxVec3& _n)
		{
		normal = _n;
		d = - p.dot(_n);
		return *this;
		}

	/**
	 Computes the plane equation from 3 points.
	 */
	NxPlane& set(const NxVec3& p0, const NxVec3& p1, const NxVec3& p2)
		{
		NxVec3 Edge0 = p1 - p0;
		NxVec3 Edge1 = p2 - p0;

		normal = Edge0.cross(Edge1);
		normal.normalize();

		d = -p0.dot(normal);

		return	*this;
		}

	NX_INLINE NxF32 distance(const NxVec3& p) const
		{
		return p.dot(normal) + d;
		}

	NX_INLINE bool belongs(const NxVec3& p) const
		{
		return fabsf(distance(p)) < (1.0e-7f);
		}

	/**
	projects p into the plane
	*/
	NX_INLINE NxVec3 project(const NxVec3 & p) const
		{
		return p + normal * distance(p);
		}

	/**
	find an arbitrary point in the plane
	*/
	NX_INLINE NxVec3 pointInPlane() const
		{
		return normal * d;
		}

	NX_INLINE void normalize()
		{
			NxF32 Denom = 1.0f / normal.magnitude();
			normal.x	*= Denom;
			normal.y	*= Denom;
			normal.z	*= Denom;
			d			*= Denom;
		}

	NX_INLINE operator NxVec3() const
		{
		return normal;
		}

	NxVec3	normal;		//!< The normal to the plane
	NxF32	d;			//!< The distance from the origin
	};

	/**
	 Transforms a plane by a 4x4 matrix.
	 \param		transformed	[out] transformed plane
	 \param		plane		[in] source plane
	 \param		transform	[in] transform matrix
	 \warning	the plane normal must be unit-length
	 */
	NX_INLINE void TransformPlane(NxPlane& transformed, const NxPlane& plane, const NxMat34& transform)
	{
		// Rotate the normal using the rotation part of the 4x4 matrix
		transform.M.multiplyByTranspose(plane.normal, transformed.normal);

		// Compute new d
		transformed.d = plane.d - transform.t.dot(transformed.normal);
	}
#endif
