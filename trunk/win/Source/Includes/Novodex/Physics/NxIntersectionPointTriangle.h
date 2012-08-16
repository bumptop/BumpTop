#ifndef NX_INTERSECTION_POINT_TRIANGLE
#define NX_INTERSECTION_POINT_TRIANGLE

#include "Nxp.h"

//namespace NxCollision {

	/**
	Point-in-triangle test. We use the edges as parameters in case the user has access to edges directly
	This is actually a "point-in-prism" test since it returns true as long as the point is bound by the edge planes.
	*/
	NX_INLINE NX_BOOL NxPointTriangleIntersect(const NxVec3& p, const NxVec3& p0, const NxVec3& edge10, const NxVec3& edge20)
		{ 
		NxF32 a = edge10|edge10;
		NxF32 b = edge10|edge20;
		NxF32 c = edge20|edge20;
		NxF32 ac_bb = (a*c)-(b*b);

		NxVec3 vp = p - p0;

		NxF32 d = vp|edge10;
		NxF32 e = vp|edge20;

		NxF32 x = (d*c) - (e*b);
		NxF32 y = (e*a) - (d*b);
		NxF32 z = x + y - ac_bb;

		// Same as: if(x>0.0f && y>0.0f && z<0.0f)	return TRUE;
		//			else							return FALSE;
		return (( NX_IR(z) & ~(NX_IR(x)|NX_IR(y)) ) & NX_SIGN_BITMASK);
		}

	/**
	Dedicated 2D version of previous test
	*/
	NX_INLINE NX_BOOL NxPointTriangleIntersect2D(
		NxF32 px, NxF32 pz,
		NxF32 p0x, NxF32 p0z,
		NxF32 e10x, NxF32 e10z,
		NxF32 e20x, NxF32 e20z)
		{ 
		NxF32 a = e10x*e10x + e10z*e10z;
		NxF32 b = e10x*e20x + e10z*e20z;
		NxF32 c = e20x*e20x + e20z*e20z;
		NxF32 ac_bb = (a*c)-(b*b);

		NxF32 vpx = px - p0x;
		NxF32 vpz = pz - p0z;

		NxF32 d = vpx*e10x + vpz*e10z;
		NxF32 e = vpx*e20x + vpz*e20z;

		NxF32 x = (d*c) - (e*b);
		NxF32 y = (e*a) - (d*b);
		NxF32 z = x + y - ac_bb;

		// Same as: if(x>0.0f && y>0.0f && z<0.0f)	return TRUE;
		//			else							return FALSE;
		return (( NX_IR(z) & ~(NX_IR(x)|NX_IR(y)) ) & NX_SIGN_BITMASK);
		}

//}

#endif
