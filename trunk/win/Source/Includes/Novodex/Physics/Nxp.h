#ifndef NX_PHYSICS_NXP
#define NX_PHYSICS_NXP
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

//this header should be included first thing in all headers in physics/include
#ifndef NXP_DLL_EXPORT
	#if defined NX_PHYSICS_DLL

		#define NXP_DLL_EXPORT __declspec(dllexport)
		#define NXF_DLL_EXPORT __declspec(dllimport)

	#elif defined NX_PHYSICS_STATICLIB

		#define NXP_DLL_EXPORT
		#define NXF_DLL_EXPORT

	#elif defined NX_USE_SDK_DLLS

		#define NXP_DLL_EXPORT __declspec(dllimport)
		#define NXF_DLL_EXPORT __declspec(dllimport)

	#elif defined NX_USE_SDK_STATICLIBS

		#define NXP_DLL_EXPORT
		#define NXF_DLL_EXPORT

	#else

		//#error Please define either NX_USE_SDK_DLLS or NX_USE_SDK_STATICLIBS in your project settings depending on the kind of libraries you use!
		#define NXP_DLL_EXPORT __declspec(dllimport)
		#define NXF_DLL_EXPORT __declspec(dllimport)
				
	#endif
#endif

#include "Nxf.h"
#include "NxVec3.h"
#include "NxQuat.h"
#include "NxMat33.h"
#include "NxMat34.h"

#include "VersionNumber.h"
/**
Pass the constant NX_PHYSICS_SDK_VERSION to the NxCreatePhysicsSDK function. 
This is to ensure that the application is using the same header version as the
library was built with.
*/

#define NX_PHYSICS_SDK_VERSION ((   NX_SDK_VERSION_MAJOR   <<24)+(NX_SDK_VERSION_MINOR    <<16)+(NX_SDK_VERSION_BUGFIX    <<8) + 0)
//2.1.1 Automatic scheme via VersionNumber.h on July 9, 2004.
//2.1.0 (new scheme: major.minor.build.configCode) on May 12, 2004.  ConfigCode can be i.e. 32 vs. 64 bit.
//2.3 on Friday April 2, 2004, starting ag. changes.
//2.2 on Friday Feb 13, 2004
//2.1 on Jan 20, 2004


#define NX_RELEASE(x)	\
	if(x)				\
		{				\
		x->release();	\
		x = NULL;		\
		}

//Note: users can't change these defines as it would need the libraries to be recompiled!
//#define UNDEFINE_ME_BEFORE_SHIPPING
#define USE_SHAPESET
#define USE_SOLVER_CALLBACK
#define USE_BODY_SIGNATURE			// This removes a branch in the main solver loop
#define NEW_SHAPE_INSTANCE_PAIR		// Needed for multiple scenes
#define NEW_BROAD_PHASE
#define LAZY_SIP_REMOVAL
#define USE_NEW_PAIR_FLAGS
#define NX_USE_ADAPTIVE_FORCE
#define MAINTAIN_PREVIOUS_POSE

// Define those two ones to report normal & friction forces in contact report. It's otherwise not needed for the SDK to run...
	#define REPORT_SUM_NORMAL_FORCE
	#define FRICTION_DEBUG_VIS

#define OBSERVABLE_BODY				// can't get rid of them for now, because of BodyEffectors
#define USE_SOLVERBODY
#ifdef UNDEFINE_ME_BEFORE_SHIPPING
#define USE_PROFILER	// WARNING: profiling has a significant speed hit (we now profile very low-level stuff)
#endif

// a bunch of simple defines used in several places:

typedef NxU32 NxCollisionGroup;		// Must be < 32
typedef NxU16 NxMaterialIndex;

struct Triangle32	//TODO: move somewhere else??
	{
	NxU32 v[3];	//vertex indices
	Triangle32() {}
	Triangle32(NxU32 a, NxU32 b, NxU32 c) {v[0] = a; v[1] = b; v[2] = c; }

	};
#endif
