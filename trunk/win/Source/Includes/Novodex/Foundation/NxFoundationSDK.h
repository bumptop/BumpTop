#ifndef NX_FOUNDATION_NXFOUNDATIONSDK
#define NX_FOUNDATION_NXFOUNDATIONSDK
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

#include "Nxf.h"
#include "VersionNumber.h"

class NxUserOutputStream;
class NxUserAllocator;
//class NxUserDynamicMesh;
class NxProfilingZone;
class NxDebugRenderable;
class NxUserDebugRenderer;
class NxDebugRenderable;

/**
Foundation SDK singleton class.

You need to have an instance of this class to instance the higher level SDKs.
*/
class NxFoundationSDK
	{
	public:
	/**
	Destroys the instance it is called on.

	Use this release method to destroy an instance of this class.  Be sure
	to not keep a reference to this object after calling release.
	*/
	virtual	void release() = 0;

	/**
	Sets an error stream provided by the user.

	After an error stream has been set, human readable error messages 
	will be inserted into it.  
	*/
	virtual void setErrorStream(NxUserOutputStream *) = 0;

	/**
	retrieves error stream
	*/
	virtual NxUserOutputStream * getErrorStream() = 0;

	/**
	retrieves information about the last (most recent) error that has occurred, and then
	resets both the last error code to NXE_NO_ERROR.
	*/
	virtual NxErrorCode getLastError() = 0;

	/**
	retrieves information about the first error that has occurred since the last call to
	getLastError() or getFirstError(), and then	this error code to NXE_NO_ERROR.
	*/
	virtual NxErrorCode getFirstError() = 0;

	/**
	retrieves the current allocator.  
	*/
	virtual NxUserAllocator & getAllocator() = 0;

	/**
	creates a profiling zone.  At the moment this is not needed by the user.
	*/
	virtual NxProfilingZone * createProfilingZone(const char * x) = 0;

	/**
	creates a debug render context.
	*/
	virtual NxDebugRenderable* createDebugRenderable() = 0;

	/**
	releases a debug context.
	*/
	virtual void releaseDebugRenderable(NxDebugRenderable*& data) = 0;

	/**
	renders all of the debug renderables using the passed renderer.
	*/
	virtual void renderDebugData(const NxUserDebugRenderer& renderer) const = 0;
	};
/**
Pass the constant NX_FOUNDATION_SDK_VERSION to the NxCreateFoundationSDK function.  
This is to ensure that the application is using the same header version as the
library was built with.
*/
#define NX_FOUNDATION_SDK_VERSION ((   NX_SDK_VERSION_MAJOR   <<24)+(NX_SDK_VERSION_MINOR    <<16)+(NX_SDK_VERSION_BUGFIX    <<8) + 0)
//2.1.1 Automatic scheme via VersionNumber.h on July 9, 2004.
//2.1.0 (new scheme: major.minor.build.configCode) on May 12, 2004.  ConfigCode can be i.e. 32 vs. 64 bit.
//2.2 on Friday Feb 13, 2004
//2.1 on Jan 20, 2004


/**
Creates an instance of this class.
Pass the constant NX_FOUNDATION_SDK_VERSION as the argument.
Because the class is a singleton class, multiple calls return the same object.

The second argument is an optional stream object into which error messages will be logged.

You may set an optional allocator class which  will be use to allocate all memory. 
Note: Once you set an allocator, you cannot clear it,
because objects allocated with a particular allocator will need to be
deallocated with the same.

If this returns 0, you are most likely have a mismatch between your library DLL and SDK headers.
*/
NX_C_EXPORT NXF_DLL_EXPORT NxFoundationSDK * NX_CALL_CONV NxCreateFoundationSDK(NxU32 sdkVersion, NxUserOutputStream * errorStream = 0, NxUserAllocator * allocator = 0);

#ifdef DEBUG_DETERMINISM
	NX_C_EXPORT NXF_DLL_EXPORT void NX_CALL_CONV initRecorder(const char* filename);
	NX_C_EXPORT NXF_DLL_EXPORT void NX_CALL_CONV closeRecorder(const char* filename);
	NX_C_EXPORT NXF_DLL_EXPORT void NX_CALL_CONV recordString(const char* str);
	NX_C_EXPORT NXF_DLL_EXPORT void NX_CALL_CONV recordFloat(float f);
	NX_C_EXPORT NXF_DLL_EXPORT void NX_CALL_CONV recordDword(int d);
	NX_C_EXPORT NXF_DLL_EXPORT void NX_CALL_CONV recordPos(const NxVec3& p);
	NX_C_EXPORT NXF_DLL_EXPORT void NX_CALL_CONV recordQuat(const NxQuat& q);
	NX_C_EXPORT NXF_DLL_EXPORT void NX_CALL_CONV recordMatrix(const NxMat33& o);
#endif

#endif
