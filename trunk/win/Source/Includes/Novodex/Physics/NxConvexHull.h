#ifndef NX_PHYSICS_NXCONVEXHULL
#define NX_PHYSICS_NXCONVEXHULL
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

#include "Nxp.h"

#include "NxArray.h"
class NxPlane;

	/**
	Computes
	*/
	NX_C_EXPORT NXP_DLL_EXPORT bool NxCreateHullFromPlanes(NxArraySDK<NxVec3>& vertices, NxU32 nbPlanes, const NxPlane* planes);

#endif