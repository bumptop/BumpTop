#ifndef NX_INTERSECTION_RAY_TRIANGLE
#define NX_INTERSECTION_RAY_TRIANGLE

#include "Nxp.h"

//namespace NxCollision
//{
	/**
	Ray-triangle intersection test. Returns impact distance (d) as well as barycentric coordinates (u,v) of impact point.
	Use NxComputeBarycentricPoint() in Foundation to compute the impact point out of barycentric coordinates.
	The test performs back face culling or not according to 'cull'.
	*/
	NX_C_EXPORT NXP_DLL_EXPORT bool NX_CALL_CONV NxRayTriIntersect(const NxVec3& orig, const NxVec3& dir, const NxVec3& vert0, const NxVec3& vert1, const NxVec3& vert2, float& t, float& u, float& v, bool cull);
//}

#endif
