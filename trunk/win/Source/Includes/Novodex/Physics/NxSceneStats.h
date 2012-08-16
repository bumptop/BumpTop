#ifndef NX_SCENE_STATS
#define NX_SCENE_STATS

/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

#define PROFILE_LEVEL 1 // used to designation how detailed the profiling on this build should be.

class NxSceneStats
{
public:

	NxI32	numContacts;
	NxI32   maxContacts;
	NxI32   numActors;
	NxI32   numJoints;
	NxI32   numAwake;
	NxI32   numAsleep;
	NxI32   numStaticShapes;

	NxSceneStats(void)
	{
		reset();
	}

	void reset(void)
	{
		numContacts = 0;
		maxContacts = 0;
		numAwake = 0;
		numAsleep = 0;
		numActors = 0;
		numJoints = 0;
		numStaticShapes = 0;
	}


};


#endif
