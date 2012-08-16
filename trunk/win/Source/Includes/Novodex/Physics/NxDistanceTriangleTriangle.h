#ifndef NX_COLLISION_NXDISTANCETRIANGLETRIANGLE
#define NX_COLLISION_NXDISTANCETRIANGLETRIANGLE
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/
#include "Nxp.h"

//namespace NxCollision {

	/**
	Computes the distance between two triangles tri0 and tri1. Also returns closest points cp0 and cp1 on each triangle.
	Returned distance value is the distance between cp0 and cp1.
	*/
	NX_C_EXPORT NXP_DLL_EXPORT NxF32 NX_CALL_CONV NxTriangleTriangleDist(NxVec3& cp0, NxVec3& cp1, const NxVec3 tri0[3], const NxVec3 tri1[3]);
//}

#endif
