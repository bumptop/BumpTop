#ifndef NXPOOL_H
#define NXPOOL_H
/*----------------------------------------------------------------------------*\
|
|								NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

#include "NxArray.h"

template<class Element, int ElementSize>
class NxPool
{
	typedef NxArraySDK<Element*> ElementPtrArray;

public:
	NxPool(NxU32 initial, NxU32 increment = 0)
	{   
		mIncrement = increment;
		mCount = 0;
		mCapacity = 0;
		allocSlab(initial);
	}

	~NxPool()
	{
		for(NxU32 i=0;i<mSlabArray.size();i++)
			nxFoundationSDKAllocator->free(mSlabArray[i]);
	}


    Element *get()
    {
        Element *element = 0;

        if(mCount==mCapacity && mIncrement)            
            allocSlab(mIncrement);

        if(mCount < mCapacity)
        {
            element = mContents[mCount];
            mIDToContents[element->hwid] = mCount++;
        }
		printf("Acquired %lp\n",element);

        return element;
    }

    void put(Element *element)
    {
		printf("Releasing %lp\n",element);
        if(mIDToContents[element->hwid]!=0xffffffff)
        {
            NxU32 elementPos = mIDToContents[element->hwid];
            /* swap the released element with the last element & decrement content counter */
            Element *replacement = mContents[--mCount];
            mContents[mCount] = element;
            mContents[elementPos] = replacement;

            /* update the ID to contents arrays */
            mIDToContents[replacement->hwid] = elementPos;
            mIDToContents[element->hwid] = 0xffffffff;
        }
    }

    const NxArray<Element *> contents() 
    {
        return mContents;
    }
    
    NxU32 count() 
    {
        return mCount;
    }

private:
    void allocSlab(NxU32 count)
    {
        char *mem = (char *)
            nxFoundationSDKAllocator->malloc(count * ElementSize);
        if(!mem)
            return;
        
        mSlabArray.pushBack(mem);
        mContents.reserve(mCapacity+count);
        mIDToContents.reserve(mCapacity+count);

        for(NxU32 i=0;i<count;i++)
        {
            Element *e = (Element *)(mem + i * ElementSize);
            e->hwid = mCapacity+i;
            mIDToContents.pushBack(0xffffffff);
            mContents.pushBack(e);
        }

        mCapacity+=count;
    }

    NxU32                 mIncrement;     /* resize quantum */
    NxU32                 mCapacity;      /* current size */
    NxU32                 mCount;         /* current number of elements */

    NxArraySDK<char *> mSlabArray;        /* array of allocated slabs */ 

    ElementPtrArray mContents;            /* array of elements. The first mCount are in use, 
                                             the rest are free for allocation */

    NxArraySDK<NxU32>     mIDToContents;  /* maps IDs to the mContents array in order to get O(1) removal. 
                                             Only required for things actually allocated, so use 0xffffffff 
                                             for 'not allocated' and thereby catch double-deletion. */
};


#endif
