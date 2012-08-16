#ifndef NX_INTERSECTION_SWEPT_SPHERES
#define NX_INTERSECTION_SWEPT_SPHERES

#include "Nxp.h"
#include "NxSphere.h"

	/**
	Sphere-sphere sweep test. Returns true if spheres intersect during their linear motion along provided velocity vectors.
	*/
	NX_C_EXPORT NXP_DLL_EXPORT bool NX_CALL_CONV NxSweptSpheresIntersect(	const NxSphere& sphere0, const NxVec3& velocity0,
																const NxSphere& sphere1, const NxVec3& velocity1);

#endif



