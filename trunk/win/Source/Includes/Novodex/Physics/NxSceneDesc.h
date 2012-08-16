#ifndef NX_PHYSICS_NXSCENEDESC
#define NX_PHYSICS_NXSCENEDESC
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

enum NxBroadPhaseType
	{
		NX_BROADPHASE_QUADRATIC,	//!< No acceleration structure (O(n^2)) [debug purpose]
		NX_BROADPHASE_FULL,			//!< Brute-force algorithm, performing the full job each time (O(n))
		NX_BROADPHASE_COHERENT,		//!< Incremental algorithm, using temporal coherence (O(n))

		NX_BROADPHASE_FORCE_DWORD = 0x7fffffff
	};

enum NxTimeStepMethod
{
	NX_TIMESTEP_FIXED,
	NX_TIMESTEP_VARIABLE,
	NX_NUM_TIMESTEP_METHODS
};

class NxUserNotify;
class NxUserTriggerReport;
class NxUserContactReport;

/**
Descriptor class for scenes.
*/

class NxSceneDesc
	{
	public:
	NxBroadPhaseType		broadPhase;			//!< Broad phase algorithm
	NxVec3					gravity;			//!< Gravity vector
	NxUserNotify*			userNotify;
	NxUserTriggerReport*	userTriggerReport;
	NxUserContactReport*	userContactReport;
	NxF32					maxTimestep;		//!< Maximum integration time step size
	NxU32					maxIter;			//!< Maximum number of substeps to take
	NxTimeStepMethod		timeStepMethod;				//!< integration method
	bool					groundPlane;		//!< Default ground plane
	bool					collisionDetection;
	void *					userData;			//!< Will be copied to NxShape::userData
	// TODO: PT: maybe also expected number of actors

	/**
	constructor sets to default (no gravity, no ground plane, collision detection on).
	*/
	NX_INLINE NxSceneDesc();	
	/**
	(re)sets the structure to the default (no gravity, no ground plane, collision detection on).	
	*/
	NX_INLINE void setToDefault();
	/**
	returns true if the current settings are valid (returns always true).
	*/
	NX_INLINE bool isValid() const;
	};

NX_INLINE NxSceneDesc::NxSceneDesc()	//constructor sets to default
	{
	setToDefault();
	}

NX_INLINE void NxSceneDesc::setToDefault()
	{
	broadPhase			= NX_BROADPHASE_COHERENT;
	gravity.zero();
	userNotify			= NULL;
	userTriggerReport	= NULL;
	userContactReport	= NULL;

	maxTimestep			= 1.0f/60.0f;
	maxIter				= 8;
	timeStepMethod		= NX_TIMESTEP_FIXED;

	groundPlane			= false;
	collisionDetection	= true;
	userData			= NULL;
	}

NX_INLINE bool NxSceneDesc::isValid() const
	{
	if (maxTimestep <= 0 || maxIter < 1 || timeStepMethod > NX_NUM_TIMESTEP_METHODS)
		return false;
	return true;
	}

#endif
