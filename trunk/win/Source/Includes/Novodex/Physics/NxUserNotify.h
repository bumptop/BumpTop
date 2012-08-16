#ifndef NX_PHYSICS_NXUSERNOTIFY
#define NX_PHYSICS_NXUSERNOTIFY
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

#include "Nxp.h"

/**
 An interface class that the user can implement in order to receive simulation events.
*/
class NxUserNotify
	{
	public:
	/**
	This is called when a breakable joint breaks.  The user should not release
	the joint inside this call!  Instead, if the user would like to have the joint
	released and no longer holds any referenced to it, he should return true.  
	In this case the joint will be released by the system.  Otherwise the user should return false.
	*/
	virtual bool onJointBreak(NxReal breakingForce, NxJoint & brokenJoint) = 0;
	};
#endif
