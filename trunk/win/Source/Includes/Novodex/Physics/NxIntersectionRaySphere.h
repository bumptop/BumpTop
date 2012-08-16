#ifndef NX_INTERSECTION_RAY_SPHERE
#define NX_INTERSECTION_RAY_SPHERE

#include "Nxp.h"

//namespace NxCollision
//{
	/**
	Rat-sphere intersection test. Returns true if the ray intersects the sphere, and the impact point if needed.
	*/
	NX_C_EXPORT NXP_DLL_EXPORT bool NX_CALL_CONV NxRaySphereIntersect(const NxVec3& origin, const NxVec3& dir, const NxVec3& center, NxF32 radius, NxVec3* coord=NULL);
//}

#endif
