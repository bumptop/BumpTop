#ifndef NX_FOUNDATION_NXUSER_ALLOCATOR_ACCESS
#define NX_FOUNDATION_NXUSER_ALLOCATOR_ACCESS
/*----------------------------------------------------------------------------*\
|
|								NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/
#include "NxUserAllocator.h"
#include "Nx.h"

#include <stdlib.h>
#ifdef WIN32
	#include <crtdbg.h>
#endif
/**
Allocator, which is used to access the global NxUserAllocator instance 
(used for dynamic data types template instantiation).
*/
class NxUserAllocatorAccess
	{
	public:
		/**
		Allocates size bytes of memory.
		*/
		NX_INLINE void * malloc(size_t size)
			{
			return nxFoundationSDKAllocator->malloc(size);
			}

		/**
		Allocates size bytes of memory.
		Same as above, but with extra debug info fields.
		*/
		NX_INLINE void * mallocDEBUG(size_t size, const char * fileName, int line)
			{
			return nxFoundationSDKAllocator->mallocDEBUG(size, fileName, line);
			}

		/**
		Resizes the memory block previously allocated with malloc() or
		realloc() to be size() bytes, and returns the possibly moved memory.
		*/
		NX_INLINE void * realloc(void * memory, size_t size)
			{
			return nxFoundationSDKAllocator->realloc(memory, size);
			}

		/**
		Frees the memory previously allocated by malloc() or realloc().
		*/
		NX_INLINE void free(void * memory)
			{
			nxFoundationSDKAllocator->free(memory);
			}

		void checkDEBUG(void)
		{
			nxFoundationSDKAllocator->checkDEBUG();
		}
	};

#endif





