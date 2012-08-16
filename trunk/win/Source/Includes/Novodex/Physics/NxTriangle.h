#ifndef NX_COLLISION_NXTRIANGLE
#define NX_COLLISION_NXTRIANGLE
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/
#include "Nxp.h"

	#define	NX_INV3		0.33333333333333333333f		//!< 1/3

/**
Triangle class.
*/
	class NxTriangle
	{
		public:
		//! Constructor
		NX_INLINE			NxTriangle()
			{
			}
		//! Constructor
		NX_INLINE			NxTriangle(const NxVec3& p0, const NxVec3& p1, const NxVec3& p2)
			{
				verts[0] = p0;
				verts[1] = p1;
				verts[2] = p2;
			}
		//! Copy constructor
		NX_INLINE			NxTriangle(const NxTriangle& triangle)
			{
				verts[0] = triangle.verts[0];
				verts[1] = triangle.verts[1];
				verts[2] = triangle.verts[2];
			}
		//! Destructor
		NX_INLINE			~NxTriangle()
			{
			}

		//! Vertices
				NxVec3		verts[3];

		NX_INLINE	void	center(NxVec3& center) const
			{
				center = (verts[0] + verts[1] + verts[2])*NX_INV3;
			}

		NX_INLINE	void	normal(NxVec3& _normal) const
			{
				_normal = (verts[1]-verts[0])^(verts[2]-verts[0]);
				_normal.normalize();
			}

		//! Makes a fat triangle
		NX_INLINE	void	inflate(float fatCoeff, bool constantBorder)
			{
			// Compute triangle center
			NxVec3 triangleCenter;
			center(triangleCenter);

			// Don't normalize?
			// Normalize => add a constant border, regardless of triangle size
			// Don't => add more to big triangles
			for(unsigned i=0;i<3;i++)
				{
				NxVec3 v = verts[i] - triangleCenter;

				if(constantBorder)	v.normalize();

				verts[i] += v * fatCoeff;
				}
			}
	};


#endif