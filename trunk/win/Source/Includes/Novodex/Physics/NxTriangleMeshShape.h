#ifndef NX_COLLISION_NXTRIANGLEMESHSHAPE
#define NX_COLLISION_NXTRIANGLEMESHSHAPE
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

#include "Nxp.h"

class NxShape;
class NxTriangleMeshShapeDesc;
class NxTriangleMesh;

enum NxMeshShapeFlag
	{
	/** 
	Select between "normal" or "smooth" sphere-mesh contact generation routines. Default: 1
	The 'normal' algorithm assumes that the mesh is composed from flat triangles. 
	When a ball rolls along the mesh surface, it will experience small, sudden changes 
	in velocity as it rolls from one triangle to the next.  The smooth algorithm, on the other hand, 
	assumes that the triangles are just an approximation of a surface that is smooth.  
	It uses the Gouraud algorithm to smooth the triangles' vertex normals (which in this 
	case are particularly important).  This way the rolling sphere's velocity will change 
	smoothly over time, instead of suddenly.  We recommend this algorithm for simulating car wheels on a terrain.
	*/
	NX_MESH_SMOOTH_SPHERE_COLLISIONS	= (1<<0),		
	};

/**
This class is a shape instance of a triangle mesh object of type NxTriangleMesh.

Each shape is owned by an actor that it is attached to.

An instance can be created by calling the createShape() method of the NxActor object
that should own it, with a NxTriangleMeshShapeDesc object as the parameter, or by adding the 
shape descriptor into the NxActorDesc class before creating the actor.

The shape is deleted by calling NxActor::releaseShape() on the owning actor.
*/

class NxTriangleMeshShape
	{
	public:

	/**
	This class is internally a subclass of NxShape. Use this
	method to perform an upcast.
	*/
	virtual NxShape& getShape() = 0;
	virtual const NxShape& getShape() const = 0;

	/**
	This class is internally a subclass of NxShape. This operator
	is automatically used to perform an upcast.
	*/
	virtual operator NxShape &() = 0;

	virtual	bool				saveToDesc(NxTriangleMeshShapeDesc&)	const = 0;

	/**
	Retrieves the triangle mesh data associated with this instance.
	*/
	virtual	NxTriangleMesh&		getTriangleMesh() = 0;

	};
#endif
