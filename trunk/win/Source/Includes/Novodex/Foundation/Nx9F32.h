#ifndef NX_FOUNDATION_NX9F32
#define NX_FOUNDATION_NX9F32
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

#include "Nxf.h"

//the file name of this header is legacy due to pain of renaming file in repository.

class Nx9Real
	{
	
	public:
        struct S
			{
#ifndef TRANSPOSED_MAT33
			NxReal        _11, _12, _13;
			NxReal        _21, _22, _23;
			NxReal        _31, _32, _33;
#else
			NxReal        _11, _21, _31;
			NxReal        _12, _22, _32;
			NxReal        _13, _23, _33;
#endif
			};
	
    union 
		{
		S s;
		NxReal m[3][3];
		};
	};


#endif
