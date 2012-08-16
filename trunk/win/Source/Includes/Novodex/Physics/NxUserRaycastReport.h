#ifndef NX_PHYSICS_NXRAYCAST
#define NX_PHYSICS_NXRAYCAST

#include "Nxp.h"
class NxShape;

enum NxShapesType
{
	NX_STATIC_SHAPES		= (1<<0),								//!< Hits static shapes
	NX_DYNAMIC_SHAPES		= (1<<1),								//!< Hits dynamic shapes
	NX_ALL_SHAPES			= NX_STATIC_SHAPES|NX_DYNAMIC_SHAPES	//!< Hits both static & dynamic shapes
};

/**
All members of the NxRaycastHit structure are not always available. For example when the ray hits a sphere,
the faceID member is not computed. Also, when raycasting against bounds (AABBs) instead of actual shapes,
some members are not available either. 

Some members like barycentric coordinates are currently only computed for triangle meshes, but next versions
might provide them in other cases. The client code should check bit flags to make sure returned values are
relevant.
*/
enum NxRaycastBit
{
	NX_RAYCAST_SHAPE		= (1<<0),								//!< "shape" member of NxRaycastHit is valid
	NX_RAYCAST_IMPACT		= (1<<1),								//!< "impact" member of NxRaycastHit is valid
	NX_RAYCAST_FACE_INDEX	= (1<<2),								//!< "faceID" member of NxRaycastHit is valid
	NX_RAYCAST_DISTANCE		= (1<<3),								//!< "distance" member of NxRaycastHit is valid
	NX_RAYCAST_UV			= (1<<4),								//!< "u" and "v" members of NxRaycastHit are valid
};

/**
This structure captures all possible results for a single raycast query.
*/
struct NxRaycastHit
{
	NxShape*	shape;												//!< Touched shape
	NxVec3		impact;												//!< Impact point in world space
	NxU32		faceID;												//!< Index of touched face
	NxF32		distance;											//!< Distance from ray start to impact point
	NxF32		u,v;												//!< Impact barycentric coordinates
	NxU32		flags;												//!< Combination of ::NxRaycastBit, validating above members.
};

/**
The user needs to pass an instance of this class to several of the ray casting routines in
NxScene. Its onHit method will be called for each shape that the ray intersects.
*/
class NxUserRaycastReport
	{
	public:

	/**
	This method is called for each shape hit by the raycast. If onHit returns true, it may be
	called again with the next shape that was stabbed. If it returns false, no further shapes
	are returned, and the raycast is concluded.
	*/
	virtual bool onHit(const NxRaycastHit&) = 0;
	};

#endif
