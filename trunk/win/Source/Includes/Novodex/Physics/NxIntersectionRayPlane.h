#ifndef NX_INTERSECTION_RAY_PLANE
#define NX_INTERSECTION_RAY_PLANE

#include "Nxp.h"
class NxRay;
class NxPlane;

	/**
	Segment-plane intersection test. Returns distance between v1 and impact point, as well as impact point on plane.
	*/
	NX_C_EXPORT NXP_DLL_EXPORT	void NX_CALL_CONV	NxSegmentPlaneIntersect(const NxVec3& v1, const NxVec3& v2, const NxPlane& plane, NxF32& dist, NxVec3& pointOnPLane);

	/**
	Ray-plane intersection test. Returns distance between ray origin and impact point, as well as impact point on plane.
	*/
	NX_C_EXPORT NXP_DLL_EXPORT	bool NX_CALL_CONV	NxRayPlaneIntersect(const NxRay& ray, const NxPlane& plane, NxF32& dist, NxVec3& pointOnPlane);

#endif
