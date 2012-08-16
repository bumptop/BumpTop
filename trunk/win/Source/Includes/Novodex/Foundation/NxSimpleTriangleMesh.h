#ifndef NX_FOUNDATION_NXSIMPLETRIANGLEMESH
#define NX_FOUNDATION_NXSIMPLETRIANGLEMESH
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

/**
Enum with flag values to be used in NxSimpleTriangleMesh::flags.
*/
enum NxMeshFlags
	{
	/**
	The Nx libraries assume that the face normal of a triangle with vertices [a,b,c] can be computed as:
	edge1 = b-a
	edge2 = c-a
	face_normal = edge1 x edge2.

	Note: this is the same as counterclockwise winding in a right handed graphics coordinate system.

	If this does not match the winding order for your triangles, raise the below flag.
	*/
	NX_MF_FLIPNORMALS		=	1 << 0,
	NX_MF_16_BIT_INDICES	=	1 << 1,	//<! Denotes the use of 16-bit vertex indices
	NX_MF_CONVEX			=	1 << 2,	//<! Denotes that the mesh is convex.
	NX_MF_COMPUTE_CONVEX	=	1 << 3,	//<! Automatically recomputes the hull from the vertices
	};

typedef NxVec3 NxPoint;

/**
A structure describing a triangle mesh.
*/
class NxSimpleTriangleMesh
	{
	public:
	NxU32 numVertices;			//!< Number of vertices.
	NxU32 numTriangles;			//!< Number of triangles.
	NxU32 pointStrideBytes;		//!< Offset between vertex points in bytes.
	NxU32 triangleStrideBytes;	//!< Offset between triangles in bytes.

	/**
	Pointer to first vertex point. Caller may add pointStrideBytes bytes to the pointer to access the next point.
	*/
	const void* points;

	/**
	Pointer to first triangle. Caller may add triangleStrideBytes bytes to the pointer to access the next triangle.
	These are triplets of 0 based indices:
	vert0 vert1 vert2
	vert0 vert1 vert2
	vert0 vert1 vert2
	...

	where vertex is either a 32 or 16 bit unsigned integer. There are numTriangles*3 indices.

	This is declared as a void pointer because it is actually either an NxU16 or a NxU32 pointer.
	*/
	const void* triangles;

	/**
	Flags bits, combined from values of the enum ::NxMeshFlags
	*/
	NxU32 flags;

	/**
	constructor sets to default.
	*/
	NX_INLINE NxSimpleTriangleMesh();	
	/**
	(re)sets the structure to the default.	
	*/
	NX_INLINE void setToDefault();
	/**
	returns true if the current settings are valid
	*/
	NX_INLINE bool isValid() const;
	};


NX_INLINE NxSimpleTriangleMesh::NxSimpleTriangleMesh()
	{
	setToDefault();
	}

NX_INLINE void NxSimpleTriangleMesh::setToDefault()
	{
	numVertices			= 0;
	numTriangles		= 0;
	pointStrideBytes	= 0;
	triangleStrideBytes	= 0;
	points				= NULL;
	triangles			= NULL;
	flags				= 0;
	}

NX_INLINE bool NxSimpleTriangleMesh::isValid() const
	{
	// Check geometry
	if(numVertices < 3 ||	//at least 1 trig's worth of points
		(numVertices > 0xffff && flags & NX_MF_16_BIT_INDICES))
		return false;
	if(!points)
		return false;
	if(pointStrideBytes < sizeof(NxPoint))	//should be at least one point's worth of data
		return false;

	// Check topology
	// The triangles pointer is not mandatory:
	// - for a non-convex mesh, the geometry can be non-indexed, only defined by a set of numVertices points.
	//   In that case the number of triangles is implicitely equals to numVertices/3.
	// - for a convex mesh, the vertex cloud is enough to define the convex hull.
	if(triangles)
		{
		// Indexed mesh
		if(numTriangles < 2)	//some algos require at least 2 trigs
			return false;
		if(flags & NX_MF_16_BIT_INDICES)
			{
			if((triangleStrideBytes < sizeof(NxU16)*3))
				return false;
			}
		else
			{
			if((triangleStrideBytes < sizeof(NxU32)*3))
				return false;
			}
		}
	else
		{
		// Non-indexed mesh or convex mesh
		if(flags & NX_MF_CONVEX)
			{
			// Convex mesh => we can compute the hull from the vertices
			if(!(flags & NX_MF_COMPUTE_CONVEX))
				return false;	// If the mesh is convex and we're not allowed to compute the hull,
								// you have to provide it completely (geometry & topology).
			}
		else
			{
			// Non-indexed mesh => we must ensure the geometry defines an implicit number of triangles
			if(numVertices%3)	// i.e. numVertices can't be divided by 3
				return false;
			}
		}
	return true;
	}

#endif
