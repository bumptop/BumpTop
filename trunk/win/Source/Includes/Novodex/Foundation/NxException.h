#ifndef NX_FOUNDATION_NXEXCEPTION
#define NX_FOUNDATION_NXEXCEPTION
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

#include "Nx.h"
/**
 Objects of this class are optionally thrown by some classes as part of the error reporting mechanism.
*/
class NxException
	{
	public:
	virtual NxErrorCode getErrorCode() = 0;
	virtual const char * getFile() = 0;
	virtual int getLine() = 0;
	};
#endif