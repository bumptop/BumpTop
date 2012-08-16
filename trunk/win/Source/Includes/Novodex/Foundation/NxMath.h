#ifndef NX_FOUNDATION_NXMATH
#define NX_FOUNDATION_NXMATH
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/
#include <math.h>
#include <float.h>
#include <stdlib.h>	//for rand()

#include "Nx.h"
#include "NxFPU.h"

//constants
static const NxF64 NxPiF64		= 3.141592653589793;
static const NxF64 NxHalfPiF64	= 1.57079632679489661923;
static const NxF64 NxTwoPiF64	= 6.28318530717958647692;
static const NxF64 NxInvPiF64	= 0.31830988618379067154;
//we can get bad range checks if we use double prec consts to check single prec results.
static const NxF32 NxPiF32		= 3.141592653589793f;
static const NxF32 NxHalfPiF32	= 1.57079632679489661923f;
static const NxF32 NxTwoPiF32	= 6.28318530717958647692f;
static const NxF32 NxInvPiF32	= 0.31830988618379067154f;


#if defined(min) || defined(max)
#error Error: min or max is #defined, probably in <windows.h>.  Put #define NOMINMAX before including windows.h to suppress windows global min,max macros.
#endif

/**
Static class with stateless scalar math routines.
*/
class NxMath
	{
	public:
		//!type conversion and rounding
		/**
		returns true if the two numbers are within eps of each other.
		*/
		NX_INLINE static bool equals(NxF32,NxF32,NxF32 eps);
		NX_INLINE static bool equals(NxF64,NxF64,NxF64 eps);
		/**
		The floor function returns a floating-point value representing the largest integer that is less than or equal to x.
		*/
		NX_INLINE static NxF32 floor(NxF32);
		NX_INLINE static NxF64 floor(NxF64);
		/**
		The ceil function returns a double value representing the smallest integer that is greater than or equal to x. 
		*/
		NX_INLINE static NxF32 ceil(NxF32);
		NX_INLINE static NxF64 ceil(NxF64);
		/**
		Truncates the float to an integer.
		*/
		NX_INLINE static NxI32 trunc(NxF32);
		NX_INLINE static NxI32 trunc(NxF64);
		/**
		abs returns the absolute value of its argument. 
		*/
		NX_INLINE static NxF32 abs(NxF32);
		NX_INLINE static NxF64 abs(NxF64);
		NX_INLINE static NxI32 abs(NxI32);
		/**
		sign returns the sign of its argument. The sign of zero is undefined.
		*/
		NX_INLINE static NxF32 sign(NxF32);
		NX_INLINE static NxF64 sign(NxF64);
		NX_INLINE static NxI32 sign(NxI32);
		/**
		The return value is the greater of the two specified values. 
		*/
		NX_INLINE static NxF32 max(NxF32,NxF32);
		NX_INLINE static NxF64 max(NxF64,NxF64);
		NX_INLINE static NxI32 max(NxI32,NxI32);
		NX_INLINE static NxU32 max(NxU32,NxU32);
		/**
		The return value is the lesser of the two specified values. 
		*/
		NX_INLINE static NxF32 min(NxF32,NxF32);
		NX_INLINE static NxF64 min(NxF64,NxF64);
		NX_INLINE static NxI32 min(NxI32,NxI32);
		NX_INLINE static NxU32 min(NxU32,NxU32);
		/**
		mod returns the floating-point remainder of x / y. If the value of y is 0.0, mod returns a quiet NaN.
		*/
		NX_INLINE static NxF32 mod(NxF32 x, NxF32 y);
		NX_INLINE static NxF64 mod(NxF64 x, NxF64 y);

		/**
		Clamps v to the range [hi,lo]
		*/
		NX_INLINE static NxF32 clamp(NxF32 v, NxF32 hi, NxF32 low);
		NX_INLINE static NxF64 clamp(NxF64 v, NxF64 hi, NxF64 low);
		NX_INLINE static NxU32 clamp(NxU32 v, NxU32 hi, NxU32 low);
		NX_INLINE static NxI32 clamp(NxI32 v, NxI32 hi, NxI32 low);
		//!powers
		/**
		Square root.
		*/
		NX_INLINE static NxF32 sqrt(NxF32);
		NX_INLINE static NxF64 sqrt(NxF64);
		/**
		Calculates x raised to the power of y.
		*/
		NX_INLINE static NxF32 pow(NxF32 x, NxF32 y);
		NX_INLINE static NxF64 pow(NxF64 x, NxF64 y);
		/**
		Calculates e^n
		*/
		NX_INLINE static NxF32 exp(NxF32);
		NX_INLINE static NxF64 exp(NxF64);
		/**
		Calculates logarithms.
		*/
		NX_INLINE static NxF32 logE(NxF32);
		NX_INLINE static NxF64 logE(NxF64);
		NX_INLINE static NxF32 log2(NxF32);
		NX_INLINE static NxF64 log2(NxF64);
		NX_INLINE static NxF32 log10(NxF32);
		NX_INLINE static NxF64 log10(NxF64);
		//!trigonometry -- all angles are in radians.
		/**
		Converts degrees to radians.
		*/
		NX_INLINE static NxF32 degToRad(NxF32);
		NX_INLINE static NxF64 degToRad(NxF64);
		/**
		Converts radians to degrees.
		*/
		NX_INLINE static NxF32 radToDeg(NxF32);
		NX_INLINE static NxF64 radToDeg(NxF64);

		/**
		Sine of an angle.
		*/
		NX_INLINE static NxF32 sin(NxF32);
		NX_INLINE static NxF64 sin(NxF64);
		/**
		Cosine of an angle.
		*/
		NX_INLINE static NxF32 cos(NxF32);
		NX_INLINE static NxF64 cos(NxF64);
		/**
		Computes both the sin and cos.
		*/
		NX_INLINE static void sinCos(NxF32, NxF32 & sin, NxF32 & cos);
		NX_INLINE static void sinCos(NxF64, NxF64 & sin, NxF64 & cos);
		/**
		Tangent of an angle.
		*/
		NX_INLINE static NxF32 tan(NxF32);
		NX_INLINE static NxF64 tan(NxF64);
		/**
		Arcsine.
		Returns angle between -PI/2 and PI/2 in radians
		*/
		NX_INLINE static NxF32 asin(NxF32);
		NX_INLINE static NxF64 asin(NxF64);
		/**
		Arccosine.
		Returns angle between 0 and PI in radians
		*/
		NX_INLINE static NxF32 acos(NxF32);
		NX_INLINE static NxF64 acos(NxF64);
		/**
		ArcTangent.
		Returns angle between -PI/2 and PI/2 in radians
		*/
		NX_INLINE static NxF32 atan(NxF32);
		NX_INLINE static NxF64 atan(NxF64);
		/**
		Arctangent of (x/y) with correct sign.
		Returns angle between -PI and PI in radians
		*/
		NX_INLINE static NxF32 atan2(NxF32 x, NxF32 y);
		NX_INLINE static NxF64 atan2(NxF64 x, NxF64 y);
		//random numbers
		/**
		uniform random number in [a,b]
		*/
		NX_INLINE static NxF32 rand(NxF32 a,NxF32 b);
		NX_INLINE static NxI32 rand(NxI32 a,NxI32 b);

		/**
		hashing: hashes an array of n 32 bit values
		to a 32 bit value.  Because the output bits
		are uniformly distributed, the caller may mask
		off some of the bits to index into a hash table
		smaller than 2^32.
		*/
		NX_INLINE static NxU32 hash(NxU32 * array, NxU32 n);

		NX_INLINE static int hash32(int);

		/**
		returns true if the passed number is a finite floating point number as opposed
		to INF, NAN, etc.
		*/
		NX_INLINE static bool isFinite(NxF32 x);
		NX_INLINE static bool isFinite(NxF64 x);
	};

/*
Many of these are just implemented as NX_INLINE calls to the C lib right now,
but later we could replace some of them with some approximations or more
clever stuff.
*/
NX_INLINE bool NxMath::equals(NxF32 a,NxF32 b,NxF32 eps)
	{
	const NxF32 diff = NxMath::abs(a - b);
	return (diff < eps);
	}

NX_INLINE bool NxMath::equals(NxF64 a,NxF64 b,NxF64 eps)
	{
	const NxF64 diff = NxMath::abs(a - b);
	return (diff < eps);
	}

NX_INLINE NxF32 NxMath::floor(NxF32 a)
	{
	return ::floorf(a);
	}

NX_INLINE NxF64 NxMath::floor(NxF64 a)
	{
	return ::floor(a);
	}

NX_INLINE NxF32 NxMath::ceil(NxF32 a)
	{
	return ::ceilf(a);
	}

NX_INLINE NxF64 NxMath::ceil(NxF64 a)
	{
	return ::ceil(a);
	}

NX_INLINE NxI32 NxMath::trunc(NxF32 a)
	{
	return (NxI32) a;	// ### PT: this actually depends on FPU settings
	}

NX_INLINE NxI32 NxMath::trunc(NxF64 a)
	{
	return (NxI32) a;	// ### PT: this actually depends on FPU settings
	}

NX_INLINE NxF32 NxMath::abs(NxF32 a)
	{
	return ::fabsf(a);
	}

NX_INLINE NxF64 NxMath::abs(NxF64 a)
	{
	return ::fabs(a);
	}

NX_INLINE NxI32 NxMath::abs(NxI32 a)
	{
	return ::abs(a);
	}

NX_INLINE NxF32 NxMath::sign(NxF32 a)
	{
	return (a >= 0.0f) ? 1.0f : -1.0f;
	}

NX_INLINE NxF64 NxMath::sign(NxF64 a)
	{
	return (a >= 0.0) ? 1.0 : -1.0;
	}

NX_INLINE NxI32 NxMath::sign(NxI32 a)
	{
	return (a >= 0) ? 1 : -1;
	}

NX_INLINE NxF32 NxMath::max(NxF32 a,NxF32 b)
	{
	return (a < b) ? b : a;
	}

NX_INLINE NxF64 NxMath::max(NxF64 a,NxF64 b)
	{
	return (a < b) ? b : a;
	}

NX_INLINE NxI32 NxMath::max(NxI32 a,NxI32 b)
	{
	return (a < b) ? b : a;
	}

NX_INLINE NxU32 NxMath::max(NxU32 a,NxU32 b)
	{
	return (a < b) ? b : a;
	}

NX_INLINE NxF32 NxMath::min(NxF32 a,NxF32 b)
	{
	return (a < b) ? a : b;
	}

NX_INLINE NxF64 NxMath::min(NxF64 a,NxF64 b)
	{
	return (a < b) ? a : b;
	}

NX_INLINE NxI32 NxMath::min(NxI32 a,NxI32 b)
	{
	return (a < b) ? a : b;
	}

NX_INLINE NxU32 NxMath::min(NxU32 a,NxU32 b)
	{
	return (a < b) ? a : b;
	}

NX_INLINE NxF32 NxMath::mod(NxF32 x, NxF32 y)
	{
	return (NxF32)::fmod(x,y);
	}

NX_INLINE NxF64 NxMath::mod(NxF64 x, NxF64 y)
	{
	return ::fmod(x,y);
	}

NX_INLINE NxF32 NxMath::clamp(NxF32 v, NxF32 hi, NxF32 low)
	{
	if (v > hi) 
		return hi;
	else if (v < low) 
		return low;
	else
		return v;
	}

NX_INLINE NxF64 NxMath::clamp(NxF64 v, NxF64 hi, NxF64 low)
	{
	if (v > hi) 
		return hi;
	else if (v < low) 
		return low;
	else
		return v;
	}

NX_INLINE NxU32 NxMath::clamp(NxU32 v, NxU32 hi, NxU32 low)
	{
	if (v > hi) 
		return hi;
	else if (v < low) 
		return low;
	else
		return v;
	}

NX_INLINE NxI32 NxMath::clamp(NxI32 v, NxI32 hi, NxI32 low)
	{
	if (v > hi) 
		return hi;
	else if (v < low) 
		return low;
	else
		return v;
	}

NX_INLINE NxF32 NxMath::sqrt(NxF32 a)
	{
	return ::sqrtf(a);
	}

NX_INLINE NxF64 NxMath::sqrt(NxF64 a)
	{
	return ::sqrt(a);
	}

NX_INLINE NxF32 NxMath::pow(NxF32 x, NxF32 y)
	{
	return ::powf(x,y);
	}

NX_INLINE NxF64 NxMath::pow(NxF64 x, NxF64 y)
	{
	return ::pow(x,y);
	}

NX_INLINE NxF32 NxMath::exp(NxF32 a)
	{
	return ::expf(a);
	}

NX_INLINE NxF64 NxMath::exp(NxF64 a)
	{
	return ::exp(a);
	}

NX_INLINE NxF32 NxMath::logE(NxF32 a)
	{
	return ::logf(a);
	}

NX_INLINE NxF64 NxMath::logE(NxF64 a)
	{
	return ::log(a);
	}

NX_INLINE NxF32 NxMath::log2(NxF32 a)
	{
	const NxF32 ln2 = (NxF32)0.693147180559945309417;
    return ::logf(a) / ln2;
	}

NX_INLINE NxF64 NxMath::log2(NxF64 a)
	{
	const NxF64 ln2 = (NxF64)0.693147180559945309417;
    return ::log(a) / ln2;
	}

NX_INLINE NxF32 NxMath::log10(NxF32 a)
	{
	return (NxF32)::log10(a);
	}

NX_INLINE NxF64 NxMath::log10(NxF64 a)
	{
	return ::log10(a);
	}

NX_INLINE NxF32 NxMath::degToRad(NxF32 a)
	{
	return (NxF32)0.01745329251994329547 * a;
	}

NX_INLINE NxF64 NxMath::degToRad(NxF64 a)
	{
	return (NxF64)0.01745329251994329547 * a;
	}

NX_INLINE NxF32 NxMath::radToDeg(NxF32 a)
	{
	return (NxF32)57.29577951308232286465 * a;
	}

NX_INLINE NxF64 NxMath::radToDeg(NxF64 a)
	{
	return (NxF64)57.29577951308232286465 * a;
	}

NX_INLINE NxF32 NxMath::sin(NxF32 a)
	{
	return ::sinf(a);
	}

NX_INLINE NxF64 NxMath::sin(NxF64 a)
	{
	return ::sin(a);
	}

NX_INLINE NxF32 NxMath::cos(NxF32 a)
	{
	return ::cosf(a);
	}

NX_INLINE NxF64 NxMath::cos(NxF64 a)
	{
	return ::cos(a);
	}

NX_INLINE void NxMath::sinCos(NxF32 a, NxF32 & s, NxF32 & c)
	{
	s = ::sinf(a);
	c = ::cosf(a);
	}

NX_INLINE void NxMath::sinCos(NxF64 a, NxF64 & s, NxF64 & c)
	{
	s = ::sin(a);
	c = ::cos(a);
	}

NX_INLINE NxF32 NxMath::tan(NxF32 a)
	{
	return ::tanf(a);
	}

NX_INLINE NxF64 NxMath::tan(NxF64 a)
	{
	return ::tan(a);
	}

NX_INLINE NxF32 NxMath::asin(NxF32 f)
	{
	// Take care of FPU inaccuracies
	if(f>=1.0f)	return (NxF32)NxHalfPiF32;
	if(f<=-1.0f)return -(NxF32)NxHalfPiF32;
				return ::asinf(f);
	}

NX_INLINE NxF64 NxMath::asin(NxF64 f)
	{
	// Take care of FPU inaccuracies
	if(f>=1.0)	return (NxF32)NxHalfPiF64;
	if(f<=-1.0)	return -(NxF32)NxHalfPiF64;
				return ::asin(f);
	}

NX_INLINE NxF32 NxMath::acos(NxF32 f)
	{
	// Take care of FPU inaccuracies
	if(f>=1.0f)	return 0.0f;
	if(f<=-1.0f)return (NxF32)NxPiF32;
				return ::acosf(f);
	}

NX_INLINE NxF64 NxMath::acos(NxF64 f)
	{
	// Take care of FPU inaccuracies
	if(f>=1.0)	return 0.0;
	if(f<=-1.0)	return (NxF64)NxPiF64;
				return ::acos(f);
	}

NX_INLINE NxF32 NxMath::atan(NxF32 a)
	{
	return ::atanf(a);
	}

NX_INLINE NxF64 NxMath::atan(NxF64 a)
	{
	return ::atan(a);
	}

NX_INLINE NxF32 NxMath::atan2(NxF32 x, NxF32 y)
	{
	return ::atan2f(x,y);
	}

NX_INLINE NxF64 NxMath::atan2(NxF64 x, NxF64 y)
	{
	return ::atan2(x,y);
	}

NX_INLINE NxF32 NxMath::rand(NxF32 a,NxF32 b)
	{
	const NxF32 r = (NxF32)::rand()/((NxF32)RAND_MAX+1);
	return r*(b-a) + a;
	}

NX_INLINE NxI32 NxMath::rand(NxI32 a,NxI32 b)
	{
	return a + ( ((NxI32)::rand() * (b - a)) /((NxI32)RAND_MAX) );
	}

/*
--------------------------------------------------------------------
lookup2.c, by Bob Jenkins, December 1996, Public Domain.
hash(), hash2(), hash3, and mix() are externally useful functions.
Routines to test the hash are included if SELF_TEST is defined.
You can use this free for any purpose.  It has no warranty.
--------------------------------------------------------------------
--------------------------------------------------------------------
mix -- mix 3 32-bit values reversibly.
For every delta with one or two bit set, and the deltas of all three
  high bits or all three low bits, whether the original value of a,b,c
  is almost all zero or is uniformly distributed,
* If mix() is run forward or backward, at least 32 bits in a,b,c
  have at least 1/4 probability of changing.
* If mix() is run forward, every bit of c will change between 1/3 and
  2/3 of the time.  (Well, 22/100 and 78/100 for some 2-bit deltas.)
mix() was built out of 36 single-cycle latency instructions in a 
  structure that could supported 2x parallelism, like so:
      a -= b; 
      a -= c; x = (c>>13);
      b -= c; a ^= x;
      b -= a; x = (a<<8);
      c -= a; b ^= x;
      c -= b; x = (b>>13);
      ...
  Unfortunately, superscalar Pentiums and Sparcs can't take advantage 
  of that parallelism.  They've also turned some of those single-cycle
  latency instructions into multi-cycle latency instructions.  Still,
  this is the fastest good hash I could find.  There were about 2^^68
  to choose from.  I only looked at a billion or so.
--------------------------------------------------------------------
*/
#define mix(a,b,c) \
{ \
  a -= b; a -= c; a ^= (c>>13); \
  b -= c; b -= a; b ^= (a<<8); \
  c -= a; c -= b; c ^= (b>>13); \
  a -= b; a -= c; a ^= (c>>12);  \
  b -= c; b -= a; b ^= (a<<16); \
  c -= a; c -= b; c ^= (b>>5); \
  a -= b; a -= c; a ^= (c>>3);  \
  b -= c; b -= a; b ^= (a<<10); \
  c -= a; c -= b; c ^= (b>>15); \
}

/*
--------------------------------------------------------------------
 This works on all machines.  hash2() is identical to hash() on 
 little-endian machines, except that the length has to be measured
 in ub4s instead of bytes.  It is much faster than hash().  It 
 requires
 -- that the key be an array of ub4's, and
 -- that all your machines have the same endianness, and
 -- that the length be the number of ub4's in the key
--------------------------------------------------------------------
*/
NX_INLINE NxU32 NxMath::hash(  NxU32 *k, NxU32 length)
//register ub4 *k;        /* the key */
//register ub4  length;   /* the length of the key, in ub4s */
	{
	NxU32 a,b,c,len;

	/* Set up the internal state */
	len = length;
	a = b = 0x9e3779b9;  /* the golden ratio; an arbitrary value */
	c = 0;           /* the previous hash value */

	/*---------------------------------------- handle most of the key */
	while (len >= 3)
	{
	  a += k[0];
	  b += k[1];
	  c += k[2];
	  mix(a,b,c);
	  k += 3; len -= 3;
	}

	/*-------------------------------------- handle the last 2 ub4's */
	c += length;
	switch(len)              /* all the case statements fall through */
	{
	 /* c is reserved for the length */
	case 2 : b+=k[1];
	case 1 : a+=k[0];
	 /* case 0: nothing left to add */
	}
	mix(a,b,c);
	/*-------------------------------------------- report the result */
	return c;
	}

NX_INLINE int NxMath::hash32(int key)
	{
	key += ~(key << 15);
	key ^=  (key >> 10);
	key +=  (key << 3);
	key ^=  (key >> 6);
	key += ~(key << 11);
	key ^=  (key >> 16);
	return key;
	}


NX_INLINE bool NxMath::isFinite(NxF32 f)
	{
	#if defined(_MSC_VER)
	return (0 == ((_FPCLASS_SNAN | _FPCLASS_QNAN | _FPCLASS_NINF | _FPCLASS_PINF) & _fpclass(f) ));
	#else
	return true;
	#endif
	
	}

NX_INLINE bool NxMath::isFinite(NxF64 f)
	{
	#if defined(_MSC_VER)
	return (0 == ((_FPCLASS_SNAN | _FPCLASS_QNAN | _FPCLASS_NINF | _FPCLASS_PINF) & _fpclass(f) ));
	#else
	return true;
	#endif
	}


#endif
