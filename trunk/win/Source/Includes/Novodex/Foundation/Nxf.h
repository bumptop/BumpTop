#ifndef NX_FOUNDATION_NXF
#define NX_FOUNDATION_NXF
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/
#include "Nx.h"

/**
Simple types of the foundation library. By changing these the computation 
precision of this library only can be changed.  Note: of course the 
library needs to be rebuilt for these changes to take effect!

By default, the 32 bit version of the SDK is compiled. In order to change the precision
you can define NX_FOUNDATION_USE_F64 in the SDK makefiles. In practice you should keep
the 32 bit configuration, as the Physics SDK works with this precision. 
*/

#ifndef NX_FOUNDATION_USE_F64
//#include "Nx4F32.h"
//#include "Nx9F32.h"

typedef NxF32 NxReal;
//typedef NxVec3T<NxF32> NxVec3;
//typedef NxQuatT<Nx4F32, NxF32> NxQuat;
//typedef NxMat33T<Nx9F32, Nx4F32, NxF32> NxMat33;
//typedef NxMat34T<Nx9F32, Nx4F32, NxF32> NxMat34;

#define NxPi	NxPiF32		
#define NxHalfPi NxHalfPiF32	
#define NxTwoPi NxTwoPiF32	
#define NxInvPi NxInvPiF32	

#define	NX_MAX_REAL			NX_MAX_F32
#define	NX_MIN_REAL			NX_MIN_F32
#define NX_EPS_REAL			NX_EPS_F32

#else
//#include "Nx4F64.h"
//#include "Nx9F64.h"

typedef NxF64 NxReal;
//typedef NxVec3T<NxF64> NxVec3;
//typedef NxQuatT<Nx4F64, NxF64> NxQuat;
//typedef NxMat33T<Nx9F64, Nx4F64, NxF64> NxMat33;
//typedef NxMat34T<Nx9F64, Nx4F64, NxF64> NxMat34;

#define NxPi	NxPiF64		
#define NxHalfPi NxHalfPiF64
#define NxTwoPi NxTwoPiF64
#define NxInvPi NxInvPiF64	

#define	NX_MAX_REAL			NX_MAX_F64
#define	NX_MIN_REAL			NX_MIN_F64
#define NX_EPS_REAL			NX_EPS_F64
#endif

//#define NxfBounds3 NxBounds3		//legacy code support, may want to replace them all eventually.
#endif