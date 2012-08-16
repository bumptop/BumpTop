#ifndef NX_FOUNDATION_NXALLOCATOR_DEFAULT
#define NX_FOUNDATION_NXALLOCATOR_DEFAULT
/*----------------------------------------------------------------------------*\
|
|								NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

#include "Nx.h"

#include <stdlib.h>

#ifdef WIN32
	#include <crtdbg.h>
#endif
/**
Default memory allocator using standard C malloc / free / realloc.
*/
class NxAllocatorDefault	
	{
	public:
		/**
		Allocates size bytes of memory.

		Compatible with the standard C malloc().
		*/
		NX_INLINE void * malloc(size_t size)
			{
			return ::malloc(size);
			}

		/**
		Allocates size bytes of memory.

		Same as above, but with extra debug info fields.
		*/
		NX_INLINE void * mallocDEBUG(size_t size, const char * fileName, int line)
			{
#ifdef _DEBUG
	#ifdef WIN32
			return ::_malloc_dbg(size, _NORMAL_BLOCK, fileName, line);
	#else
	// TODO: insert a Linux Debugger Function
			return ::malloc(size);
	#endif
#else
			NX_ASSERT(0);//Don't use debug malloc for release mode code!
			return 0;
#endif
			}

		/**
		Resizes the memory block previously allocated with malloc() or
		realloc() to be size() bytes, and returns the possibly moved memory.

		Compatible with the standard C realloc().
		*/
		NX_INLINE void * realloc(void * memory, size_t size)
			{
			return ::realloc(memory,size);
			}

		/**
		Frees the memory previously allocated by malloc() or realloc().

		Compatible with the standard C free().
		*/
		NX_INLINE void free(void * memory)
			{
			::free(memory);
			}

		NX_INLINE void check(void * /*memory*/)
			{
			}
	};

#endif





