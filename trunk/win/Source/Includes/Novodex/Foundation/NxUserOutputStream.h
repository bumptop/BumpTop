#ifndef NX_FOUNDATION_NXUSEROUTPUTSTREAM
#define NX_FOUNDATION_NXUSEROUTPUTSTREAM
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

#include "Nx.h"

enum NxAssertResponse
	{
	NX_AR_CONTINUE,			//!continue execution
	NX_AR_IGNORE,			//!continue and don't report this assert from now on
	NX_AR_BREAKPOINT		//!trigger a breakpoint
	};

/**
 User defined interface class.  Used by the library to emit debug information.
*/
class NxUserOutputStream
	{
	public:

	/**
	Reports an error code.
	*/
	virtual void reportError(NxErrorCode, const char * message, const char *file, int line) = 0;
	/**
	Reports an assertion violation.  The user should return 
	*/
	virtual NxAssertResponse reportAssertViolation(const char * message, const char *file, int line) = 0;
	/**
	Simply prints some debug text
	*/
	virtual void print(const char * message) = 0;
	};
#endif