#ifndef NX_INTERSECTION_SEGMENT_CAPSULE
#define NX_INTERSECTION_SEGMENT_CAPSULE

#include "Nxp.h"
#include "NxCapsule.h"
#include "NxSegment.h"

//namespace NxCollision {

	/**
	Ray-capsule intersection test. Returns number of intersection points (0,1 or 2) and corresponding parameters along the ray.
	*/
	NX_C_EXPORT NXP_DLL_EXPORT  NxU32 NX_CALL_CONV NxRayCapsuleIntersect(const NxVec3& origin, const NxVec3& dir, const NxCapsule& capsule, NxReal t[2]);

	/**
	Segment-capsule intersection test. Returns number of intersection points (0,1 or 2) and corresponding parameters along the ray.
	*/
	NX_INLINE void NxSegmentCapsuleIntersect(const NxSegment& segment, const NxCapsule& capsule, NxU32* nbImpacts, NxReal t[2])
		{
		NxReal s[2];
		NxU32 numISec = NxRayCapsuleIntersect(segment.p0, segment.computeDirection(), capsule,s);

		NxU32 numClip = 0;
		for(NxU32 i = 0; i < numISec; i++)
			{
			if ( 0.0f <= s[i] && s[i] <= 1.0f ) t[numClip++] = s[i];
			}

		*nbImpacts = numClip;
		}
//}

#endif
