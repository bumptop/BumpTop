#ifndef NX_FOUNDATION_NXSIMPLETYPES
#define NX_FOUNDATION_NXSIMPLETYPES
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

// Platform specific types:
//Design note: Its OK to use int for general loop variables and temps.

#ifdef WIN32
	typedef __int64				NxI64;
	typedef signed int			NxI32;
	typedef signed short		NxI16;
	typedef signed char			NxI8;

	typedef unsigned __int64	NxU64;
	typedef unsigned int		NxU32;
	typedef unsigned short		NxU16;
	typedef unsigned char		NxU8;

	typedef float				NxF32;
	typedef double				NxF64;
		
#elif LINUX
	typedef long long				NxI64;
	typedef signed int			NxI32;
	typedef signed short		NxI16;
	typedef signed char			NxI8;

	typedef unsigned long long	NxU64;
	typedef unsigned int		NxU32;
	typedef unsigned short		NxU16;
	typedef unsigned char		NxU8;

	typedef float				NxF32;
	typedef double				NxF64;
#else
	#error Unknown platform!
#endif

	NX_COMPILE_TIME_ASSERT(sizeof(bool)==1);	// ...otherwise things might fail with VC++ 4.2 !
	NX_COMPILE_TIME_ASSERT(sizeof(NxI8)==1);
	NX_COMPILE_TIME_ASSERT(sizeof(NxU8)==1);
	NX_COMPILE_TIME_ASSERT(sizeof(NxI16)==2);
	NX_COMPILE_TIME_ASSERT(sizeof(NxU16)==2);
	NX_COMPILE_TIME_ASSERT(sizeof(NxI32)==4);
	NX_COMPILE_TIME_ASSERT(sizeof(NxU32)==4);
	NX_COMPILE_TIME_ASSERT(sizeof(NxI64)==8);
	NX_COMPILE_TIME_ASSERT(sizeof(NxU64)==8);

	// Type ranges
	#define	NX_MAX_I8			0x7f			//max possible sbyte value
	#define	NX_MIN_I8			0x80			//min possible sbyte value
	#define	NX_MAX_U8			0xff			//max possible ubyte value
	#define	NX_MIN_U8			0x00			//min possible ubyte value
	#define	NX_MAX_I16			0x7fff			//max possible sword value
	#define	NX_MIN_I16			0x8000			//min possible sword value
	#define	NX_MAX_U16			0xffff			//max possible uword value
	#define	NX_MIN_U16			0x0000			//min possible uword value
	#define	NX_MAX_I32			0x7fffffff		//max possible sdword value
	#define	NX_MIN_I32			0x80000000		//min possible sdword value
	#define	NX_MAX_U32			0xffffffff		//max possible udword value
	#define	NX_MIN_U32			0x00000000		//min possible udword value
	#define	NX_MAX_F32			FLT_MAX			//max possible float value
	#define	NX_MIN_F32			(-FLT_MAX)		//min possible float value
	#define	NX_MAX_F64			DBL_MAX			//max possible double value
	#define	NX_MIN_F64			(-DBL_MAX)		//min possible double value

	#define NX_EPS_F32			FLT_EPSILON		//smallest number not zero
	#define NX_EPS_F64			DBL_EPSILON		//smallest number not zero

	#define NX_IEEE_1_0			0x3f800000		//integer representation of 1.0
	#define NX_IEEE_255_0		0x437f0000		//integer representation of 255.0
	#define NX_IEEE_MAX_F32		0x7f7fffff		//integer representation of MAX_NXFLOAT
	#define NX_IEEE_MIN_F32		0xff7fffff		//integer representation of MIN_NXFLOAT

	typedef int	NX_BOOL;
	#define NX_FALSE			0
	#define NX_TRUE				1

	#define	NX_MIN(a, b)		((a) < (b) ? (a) : (b))			//Returns the min value between a and b
	#define	NX_MAX(a, b)		((a) > (b) ? (a) : (b))			//Returns the max value between a and b

#endif
