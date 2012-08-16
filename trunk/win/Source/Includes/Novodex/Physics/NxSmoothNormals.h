#ifndef NX_COLLISION_NXSMOOTHNORMALS
#define NX_COLLISION_NXSMOOTHNORMALS
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/
#include "Nxp.h"

	/**
	Builds smooth vertex normals over a mesh.
	- "smooth" because smoothing groups are not supported here
	- takes angles into account for correct cube normals computation
	*/
	NX_C_EXPORT NXP_DLL_EXPORT bool NX_CALL_CONV NxBuildSmoothNormals(
		NxU32 nbTris,			//!< Number of triangles
		NxU32 nbVerts,			//!< Number of vertices
		const NxVec3* verts,	//!< Array of vertices
		const NxU32* dFaces,	//!< Array of dword triangle indices, or null
		const NxU16* wFaces,	//!< Array of word triangle indices, or null
		NxVec3* normals,		//!< Array of computed normals (assumes nbVerts vectors)
		bool flip=false			//!< Flips the normals or not
		);
#endif