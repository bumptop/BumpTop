#ifndef NX_COLLISION_NXPMAP
#define NX_COLLISION_NXPMAP
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/
/**
PMap data structure for mesh collision detection.  Used by the functions
NxCreatePMap and NxReleasePMap.  
This structure can be assigned to NxTriangleMeshDesc::pmap or passed to NxTriangleMesh::loadPMap().
*/
class NxPMap
	{
	public:
	NxU32		dataSize;	//!< size of data buffer in bytes
	void*		data;		//!< data buffer that stores the PMap information.
	};

	/**
	Creates a PMap from a triangle mesh. A PMap is an optional data structure which makes mesh-mesh collision 
	detection more robust at the cost of higher	memory consumption.
	
	This structure can then be assigned to NxTriangleMeshDesc::pmap or passed to NxTriangleMesh::loadPMap().

	You may wish to store the PMap on disc (just write the above data block to a file of your choice) after
	computing it because the creation process can be quite expensive. Then you won't have to create it the next time
	you need it.
	*/
	NX_C_EXPORT NXP_DLL_EXPORT bool NX_CALL_CONV NxCreatePMap(NxPMap& pmap, const NxTriangleMesh& mesh, NxU32 density, NxUserOutputStream* outputStream = NULL);

	/**
	Releases PMap previously created with NxCreatePMap. You should not call this on PMap data you have loaded from
	disc yourself. Don't release a PMap while it is being used by a NxTriangleMesh object.
	*/
	NX_C_EXPORT NXP_DLL_EXPORT bool NX_CALL_CONV NxReleasePMap(NxPMap& pmap);

#endif
