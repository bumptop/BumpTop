#ifndef NX_FOUNDATION_NXVOLUMEINTEGRATION
#define NX_FOUNDATION_NXVOLUMEINTEGRATION
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

#include "Nx.h"
#include "NxVec3.h"
#include "NxMat33.h"

class NxSimpleTriangleMesh;

/**
Data structure used by NxComputeVolumeIntegrals.
*/
struct NxIntegrals
	{
	NxVec3 COM;					//!< Center of mass
	NxF64 mass;						//!< Total mass
	NxF64 inertiaTensor[3][3];		//!< Intertia tensor (mass matrix) relative to the origin
	NxF64 COMInertiaTensor[3][3];	//!< Intertia tensor (mass matrix) relative to the COM

	void getInertia(NxMat33& inertia)
	{
		for(NxU32 j=0;j<3;j++)
		{
			for(NxU32 i=0;i<3;i++)
			{
				inertia(i,j) = (NxF32)COMInertiaTensor[i][j];
			}
		}
	}

	void getOriginInertia(NxMat33& inertia)
	{
		for(NxU32 j=0;j<3;j++)
		{
			for(NxU32 i=0;i<3;i++)
			{
				inertia(i,j) = (NxF32)inertiaTensor[i][j];
			}
		}
	}
	};

/**
Used to compute the inertia tensor of a triangle mesh.
*/
NX_C_EXPORT NXF_DLL_EXPORT bool NX_CALL_CONV NxComputeVolumeIntegrals(const NxSimpleTriangleMesh& mesh, NxReal density, NxIntegrals& integrals);

#endif
