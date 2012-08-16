#ifndef NX_FOUNDATION_NXALLOCATEABLE
#define NX_FOUNDATION_NXALLOCATEABLE
/*----------------------------------------------------------------------------*\
|
|								NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/
#include "NxUserAllocator.h"
#include "NxFoundationSDK.h"

NX_C_EXPORT NXF_DLL_EXPORT NxUserAllocator * nxFoundationSDKAllocator;	//the SDK defines this.

/**
Subclasses of this base class automatically take part in user memory management
*/
class NxAllocateable
	{
	public:
	NX_INLINE void* operator new( size_t size );
	NX_INLINE void* operator new( size_t size, const char * fileName, int line );
	NX_INLINE void* operator new[]( size_t size );
	NX_INLINE void* operator new[]( size_t size, const char * fileName, int line );
	NX_INLINE void  operator delete( void* p );
	NX_INLINE void  operator delete( void* p, const char *, int);
	NX_INLINE void  operator delete[](void* p);
	NX_INLINE void  operator delete[](void* p, const char *, int);

	};

NX_INLINE void* NxAllocateable::operator new( size_t size )	
	{	
	void * mem = nxFoundationSDKAllocator->malloc(size);	
	return mem;	
	}

NX_INLINE void* NxAllocateable::operator new( size_t size, const char * fileName, int line )	
	{	
	void * mem = nxFoundationSDKAllocator->mallocDEBUG(size, fileName, line);	
	return mem;	
	}

NX_INLINE void* NxAllocateable::operator new[]( size_t size )	
	{	
	void * mem = nxFoundationSDKAllocator->malloc(size);	
	return mem;	
	}

NX_INLINE void* NxAllocateable::operator new[]( size_t size, const char * fileName, int line )	
	{	
	void * mem = nxFoundationSDKAllocator->mallocDEBUG(size, fileName, line);	
	return mem;	
	}

NX_INLINE void NxAllocateable::operator delete( void* p )	
	{	
	nxFoundationSDKAllocator->free(p);	
	}

NX_INLINE void NxAllocateable::operator delete( void* p, const char *, int)	
	{	
	nxFoundationSDKAllocator->free(p);	
	}

NX_INLINE void NxAllocateable::operator delete[]( void* p )	
	{	
	nxFoundationSDKAllocator->free(p);	
	}

NX_INLINE void NxAllocateable::operator delete[]( void* p, const char *, int)	
	{	
	nxFoundationSDKAllocator->free(p);	
	}

#endif