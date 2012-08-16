#ifndef NX_PHYSICS_NX_SCENE
#define NX_PHYSICS_NX_SCENE
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

#include "Nxp.h"
#include "NxUserRaycastReport.h"
#include "NxSceneDesc.h"
#include "NxSceneStats.h"
#include "NxArray.h"

class NxActor;
class NxActorDesc;
class NxJoint;
class NxJointDesc;
class NxEffector;
class NxShape;
class NxUserNotify;
class NxUserTriggerReport;
class NxUserContactReport;
class NxSpringAndDamperEffector;
class NxSpringAndDamperEffectorDesc;
class NxRay;
class NxBounds3;
class NxTriangle;

/**
Struct used by NxScene::getPairFlagArray().
The high bit of each 32 bit	flag field denotes whether a pair is a shape or an actor pair.
The flags are defined by the enum NxContactPairFlag.
*/
struct NxPairFlag
	{
	void*	objects[2];
	NxU32	flags;

	NX_INLINE NxU32 isActorPair() const { return flags & 0x80000000;	}
	};


enum NxStandardFences { NX_FENCE_RUN_FINISHED, /*NX_SYNC_RAYCAST_FINISHED,*/ NX_NUM_STANDARD_FENCES };


/**
 Scene interface class. A scene is a collection of bodies, constraints, and
 effectors which can interact. The scene simulates the behavior of these objects
 over time. Several scenes may exist at the same time, but each body, constraint,
 or effector object is specific to a scene -- they may not be shared.

 For example, attempting to create a joint in one scene and then using it to attach
 bodies from a different scene results in undefined behavior.
*/
class NxScene
	{
	protected:
										NxScene()	{}
	virtual								~NxScene()	{}

	public:
//@SG{

//@SE{
	/**
	Sets a constant gravity for the entire scene.
	*/
	virtual void						setGravity(const NxVec3&) = 0;

	/**
	Retrieves the current gravity setting.
	*/
	virtual void						getGravity(NxVec3&) = 0;

//@SE}

//@SE{

	/**
	Creates an actor in this scene. NxActorDesc::isValid() must return true and
	the NxActorDesc must provide at least one shape, otherwise returns NULL.
	*/
	virtual NxActor*					createActor(const NxActorDesc&) = 0;

	/**
	Deletes the specified actor. The actor must be in this scene.
	Do not keep a reference to the deleted instance.

	Note: deleting a static actor will not wake up any sleeping objects that were
	sitting on it.  Use a kinematic actor instead to get this behavior.
	*/
	virtual void						releaseActor(NxActor&) = 0;

//@SE}

//@SE{

	/**
	Creates a joint. The joint type depends on the type of joint desc passed in.
	*/
	virtual NxJoint *					createJoint(const NxJointDesc &) = 0;

	/**
	Deletes the specified joint. The joint must be in this scene.
	Do not keep a reference to the deleted instance.
	*/
	virtual void						releaseJoint(NxJoint &) = 0;

//@SE}

//@SE{

	/**
	Creates an instance of the returned class.
	*/
	virtual NxSpringAndDamperEffector*	createSpringAndDamperEffector(const NxSpringAndDamperEffectorDesc&) = 0;

	/**
	Deletes the effector passed.
	Do not keep a reference to the deleted instance.
	*/
	virtual void						releaseEffector(NxEffector&) = 0;

//@SE}

//@SE{
#ifdef USE_NEW_PAIR_FLAGS
	/**
	Sets the pair flags for the given pair of actors.  The pair flags are a combination of bits
	defined by ::NxContactPairFlag.  Calling this on an actor that has no shape(s) has no effect.
	The two actor references must not reference the same actor.
	*/
	virtual void						setActorPairFlags(NxActor&, NxActor&, NxU32 nxContactPairFlag) = 0;

	/**
	Retrieves the pair flags for the given pair of actors.  The pair flags are a combination of bits
	defined by ::NxContactPairFlag.  If no pair record is found, zero is returned.
	The two actor references must not reference the same actor.
	*/
	virtual NxU32						getActorPairFlags(NxActor&, NxActor&) const = 0;

	/**
	similar to setPairFlags(), but for a pair of shapes.  The only supported flag is NX_IGNORE_PAIR!
	The two shape references must not reference the same shape.
	*/
	virtual	void						setShapePairFlags(NxShape&, NxShape&, NxU32 nxContactPairFlag) = 0;

	/**
	similar to getPairFlags(), but for a pair of shapes.  The only supported flag is NX_IGNORE_PAIR!
	The two shape references must not reference the same shape.
	*/
	virtual	NxU32						getShapePairFlags(NxShape&, NxShape&) const = 0;

	/**
	Returns the number of pairs for which pairFlags are defined.  (This includes actor and shape pairs.)
	*/
	virtual NxU32						getNbPairs() const = 0;

	/**
	Writes the pair data.  The high bit of each 32 bit
	flag field denotes whether a pair is a shape or an actor pair.  numPairs is the number of pairs the buffer can hold.
	The user is responsible for allocating and deallocating the buffer.  
	*/
	virtual bool						getPairFlagArray(NxPairFlag* userArray, NxU32 numPairs) const = 0;
#else
	/**
	enables or disables collision detection between a pair of actors. Initially all pairs are enabled.

	Collision detection between two shapes a and b occurs if: 
	NxPhysicsSDK::getGroupCollisionFlag(a->getGroup(), b->getGroup()) && isEnabledPair(a->getActor(),b->getActor()) is true.

	Note: a and b may not refer to the same shape.
	*/
	virtual void					enablePair(NxActor&, NxActor&, bool enable) = 0;

	/**
	Queries the value set by the above call.
	*/
	virtual bool					isEnabledPair(NxActor&, NxActor&) = 0;

	/**
	similar to enablePair(), but for a pair of shapes.
	*/
	virtual	void					enableShapePair(NxShape& s0, NxShape& s1, bool enable) = 0;
#endif
//@SE}

//@SE{

	/**
	Returns the number of actors.
	*/
	virtual	NxU32					getNbActors()		const	= 0;

	/**
	Returns an array of actor pointers with size getNbActors().
	*/
	virtual	NxActor**				getActors()					= 0;

//@SE}

//@SE{

   	/**
   	Returns the number of static shapes.
   	*/
 	virtual	NxU32					getNbStaticShapes()		const	= 0;
   
   	/**
   	Returns an array of NxShape pointers with size getNbStaticShapes().
   	*/
  	virtual	NxShape**				getStaticShapes() = 0;
   
//@SE}

//@SE{
	/**
	Flush the scene's command queue for processing.
	Flushes any eventually buffered commands so that they get executed.
	Ensures that commands buffered in the system will continue to make forward progress until completion.
	*/
	virtual void					flushStream() = 0;

	/**
	Advances the simulation by an elapsedTime time.  If elapsedTime is large, it is internally
	subdivided according to parameters provided with the setTiming() method.

	Calls to startRun() must pair with calls to finishRun():
	Each finishRun() invocation corresponds to exactly one startRun()
	invocation; calling finishRun() twice without an intervening finishRun()
	or finishRun() twice without an intervening startRun() causes an error
	condition.

	scene->startRun();
	...do some processing until physics is computed...
	scene->finishRun();
	...now results of run may be retrieved.

	Applications should not modify physics objects between calls to
	startRun() and finishRun();
	*/
	virtual void					startRun(NxF32 elapsedTime) = 0;

	/**
	Writes the new simulation state computed by the
	last startRun() commands into the physics objects.

	At this point callbacks will run generated by contacts,
	triggers, and ray casts processed during the run().

	Each finishRun() invocation corresponds to exactly one startRun()
	invocation; calling finishRun() twice without an intervening finishRun()
	or finishRun() twice without an intervening startRun() causes an error
	condition.

	This method stalls until the results of the run are available.
	*/
	virtual void					finishRun() = 0;

	/**
	Sets simulation timing parameters used in startRun(elapsedTime).

	If elapsedTime is large, it is internally subdivided into up to
	maxIter integration steps no larger than maxTimestep. The timestep method of
	TIMESTEP_FIXED is strongly preferred for stable, reproducible simulation.
	*/
	virtual void					setTiming(NxF32 maxTimestep=1.0f/60.0f, NxU32 maxIter=8, NxTimeStepMethod method=NX_TIMESTEP_FIXED) = 0;

	/**
	Retrieves simulation timing parameters.
	*/
	virtual void					getTiming(NxF32 & maxTimestep, NxU32 & maxIter, NxTimeStepMethod & method) const = 0;

	/**
	\deprecated { use setTiming()/startRun()/finishRun() instead }
	Advances the simulation by an elapsedTime time.  If elapsedTime is large, it is internally
	subdivided into up to maxIter integration steps no larger than maxTimestep. The timestep method of
	TIMESTEP_FIXED is strongly preferred for stable, reproducible simulation.
	*/
	virtual void					runFor(NxF32 elapsedTime, NxF32 maxTimestep=1.0f/60.0f, NxU32 maxIter=8, NxTimeStepMethod method=NX_TIMESTEP_FIXED) = 0;

	/**
	Call this if you want this scene to generate visualization data. You can have this data get rendered into a buffer
	using PhysicsSDK::visualize(buffer).
	*/
	virtual	void					visualize() = 0;

	/**
	Call this method to retrieve statistics about the current scene.
	*/
	virtual	NxSceneStats *	getSceneStats() = 0;

//@SE}

//@SE{

	/**
	Sets a user notify object which receives special simulation events when they occur.
	*/
	virtual void					setUserNotify(NxUserNotify* callback) = 0;

	/**
	Retrieves the userNotify pointer set with setUserNotify().
	*/
	virtual NxUserNotify*			getUserNotify() const = 0;

//@SE}

//@SE{

	/**
	Sets a trigger report object which receives collision trigger events.
	*/
	virtual	void					setUserTriggerReport(NxUserTriggerReport* callback) = 0;

	/**
	Retrieves the callback pointer set with setUserTriggerReport().
	*/
	virtual	NxUserTriggerReport*	getUserTriggerReport() const = 0;

//@SE}

//@SE{

	/**
	Sets a contact report object which receives collision contact events.
	*/
	virtual	void					setUserContactReport(NxUserContactReport* callback) = 0;

	/**
	Retrieves the callback pointer set with setUserContactReport().
	*/
	virtual	NxUserContactReport*	getUserContactReport() const = 0;

//@SE}

//@SE{

	//! Raycasting

	/**
	Returns true if any axis aligned boundix box enclosing a shape of type shapeType is intersected by the ray.
	Note: Make certain that the direction vector of NxRay is normalized.
	*/
	virtual bool					raycastAnyBounds		(const NxRay& worldRay, NxShapesType shapesType, NxU32 groups=0xffffffff) const = 0;

	/**
	Returns true if any shape of type ShapeType is intersected by the ray.
	Note: Make certain that the direction vector of NxRay is normalized.
	*/
	virtual bool					raycastAnyShape			(const NxRay& worldRay, NxShapesType shapesType, NxU32 groups=0xffffffff) const = 0;

	/**
	Calls the report's hitCallback() method for all the axis aligned bounding boxes enclosing shapes
	of type ShapeType intersected by the ray. The point of impact is provided as a parameter to hitCallback().
	Returns the number of shapes hit.
	Note: Make certain that the direction vector of NxRay is normalized.
	*/
	virtual NxU32					raycastAllBounds		(const NxRay& worldRay, NxUserRaycastReport& report, NxShapesType shapesType, NxU32 groups=0xffffffff) const = 0;

	/**
	Calls the report's hitCallback() method for all the shapes of type ShapeType intersected by the ray.
	Returns the number of shapes hit. The point of impact is provided as a parameter to hitCallback().
	Note: Make certain that the direction vector of NxRay is normalized.
	*/
	virtual NxU32					raycastAllShapes		(const NxRay& worldRay, NxUserRaycastReport& report, NxShapesType shapesType, NxU32 groups=0xffffffff) const = 0;

	/**
	Returns the first axis aligned boundix box enclosing a shape of type shapeType that is hit along the ray. The world space intersectin point,
	and the distance along the ray are also provided.
	Note: Make certain that the direction vector of NxRay is normalized.
	*/
	virtual NxShape*				raycastClosestBounds	(const NxRay& worldRay, NxShapesType shapeType, NxRaycastHit& hit, NxU32 groups=0xffffffff) const = 0;

	/**
	Returns the first shape of type shapeType that is hit along the ray. The world space intersectin point,
	and the distance along the ray are also provided.
	Note: Make certain that the direction vector of NxRay is normalized.
	*/
	virtual NxShape*				raycastClosestShape		(const NxRay& worldRay, NxShapesType shapeType, NxRaycastHit& hit, NxU32 groups=0xffffffff) const = 0;

//@SE}

//@SE{

	virtual	NxU32					overlapAABBTriangles	(const NxBounds3& worldBounds, NxArraySDK<NxTriangle>& worldTriangles) const = 0;

//@SE}

//@SE{
	/**
	Returns true if a fence point has completed.

	\param block If this parameter is true the check will stall
	until the synchronization point is reached.

	Between the issue of a NxScene::startRun() and NxScene::finishRun(), this
	method allows the application to check whether the NxScene::finishRun()
	operation will stall.  Returns true if there will be no stall.

	*/
    bool wait(NxStandardFences, bool block);

//@SE}


	void * userData;	//!< user can assign this to whatever, usually to create a 1:1 relationship with a user object.
//@SG}




	};

#endif
