#ifndef NX_INTERSECTION_SEGMENT_BOX
#define NX_INTERSECTION_SEGMENT_BOX

#include "Nxp.h"
#include "NxBox.h"

class NxRay;

//namespace NxCollision
//{
	/**
	Segment-AABB intersection test. Also computes intersection point.
	*/
	NX_C_EXPORT NXP_DLL_EXPORT bool NX_CALL_CONV NxSegmentBoxIntersect(const NxVec3& p1, const NxVec3& p2,const NxVec3& bbox_min,const NxVec3& bbox_max, NxVec3& intercept);

	/**
	Ray-AABB intersection test. Also computes intersection point.
	*/
	NX_C_EXPORT NXP_DLL_EXPORT bool NX_CALL_CONV NxRayAABBIntersect(const NxVec3& min, const NxVec3& max, const NxVec3& origin, const NxVec3& dir, NxVec3& coord);

	/**
	Boolean segment-OBB intersection test. Based on separating axis theorem.
	*/
	NX_C_EXPORT NXP_DLL_EXPORT bool NX_CALL_CONV NxSegmentOBBIntersect(const NxVec3& p0, const NxVec3& p1, const NxVec3& center, const NxVec3& extents, const NxMat33& rot);

	/**
	Boolean segment-AABB intersection test. Based on separating axis theorem.
	*/
	NX_C_EXPORT NXP_DLL_EXPORT bool NX_CALL_CONV NxSegmentAABBIntersect(const NxVec3& p0, const NxVec3& p1, const NxVec3& min, const NxVec3& max);

	/**
	Boolean ray-OBB intersection test. Based on separating axis theorem.
	*/
	NX_C_EXPORT NXP_DLL_EXPORT bool NX_CALL_CONV NxRayOBBIntersect(const NxRay& ray, const NxVec3& center, const NxVec3& extents, const NxMat33& rot);
//}

#endif
