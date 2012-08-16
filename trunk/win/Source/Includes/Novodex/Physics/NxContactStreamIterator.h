#ifndef NX_PHYSICS_NXCONTACTSTREAMITERATOR
#define NX_PHYSICS_NXCONTACTSTREAMITERATOR
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/
#include "Nxp.h"

typedef NxU32 * NxContactStream;
typedef const NxU32 * NxConstContactStream;

enum NxShapePairStreamFlags
	{
	NX_SF_HAS_MATS_PER_POINT = 1 << 0	//used when we have materials per triangle in a mesh.  In this case the extData field is used after the point separation value.
	};

/**
NxContactStreamIterator is for iterating through packed contact streams.

The user code to use this iterator looks like this:
void MyUserContactInfo::onContactNotify(NxPair & pair, NxU32 events)
	{
	NxContactStreamIterator i(pair.stream);
	//user can call getNumPairs() here
	while(i.goNextPair())
		{
		//user can also call getShape() and getNumPatches() here
		while(i.goNextPatch())
			{
			//user can also call getPatchNormal() and getNumPoints() here
			while(i.goNextPoint())
				{
				//user can also call getPoint() and getSeparation() here
				}
			}
		}
	}

Note:  It is NOT OK to skip any of the iteration calls.  For example, you may NOT put a break or a continue
statement in any of the above blocks, short of completely aborting the iteration and deleting the 
NxContactStreamIterator object.
*/

class NxContactStreamIterator
	{
	public:
	/**
	Starts the iteration, and returns the number of pairs.
	*/
	NX_INLINE NxContactStreamIterator(NxConstContactStream stream);

	//iteration:
	NX_INLINE bool goNextPair();
	NX_INLINE bool goNextPatch();
	NX_INLINE bool goNextPoint();

	//accessors:
	NX_INLINE NxU32 getNumPairs();	//may be called at any time

	NX_INLINE NxShape * getShape(NxU32 shapeIndex);	//may be called after goNextPair() returned true. ShapeIndex is 0 or 1.
	NX_INLINE NxU32 getNumPatches();//may be called after goNextPair() returned true

	NX_INLINE const NxVec3 & getPatchNormal();	//may be called after goNextPatch() returned true
	NX_INLINE NxU32 getNumPoints();//may be called after goNextPatch() returned true

	NX_INLINE const NxVec3 & getPoint();//may be called after goNextPoint() returned true
	NX_INLINE NxF32 getSeparation();//may be called after goNextPoint() returned true

	NX_INLINE NxU32 getExtData();
	private:
	//iterator variables -- are only initialized by stream iterator calls:
	//Note: structs are used so that we can leave the iterators vars on the stack as they were
	//and the user's iteration code doesn't have to change when we add members.

	NxU32 numPairs;
	//current pair properties:
	NxShape * shapes[2];
	NxU16 shapeFlags;
	NxU16 numPatches;
	//current patch properties:
	const NxVec3 * patchNormal;
	NxU32  numPoints;
	//current point properties:
	const NxVec3 * point;
	NxReal separation;
	NxU32 extData;			//only exists if (shapeFlags & NX_SF_HAS_MATS_PER_POINT)

	NxConstContactStream stream;
	};

NX_INLINE NxContactStreamIterator::NxContactStreamIterator(NxConstContactStream s)
	{
	stream = s;
	numPairs = stream ? *stream++ : 0;
	}

NX_INLINE NxU32 NxContactStreamIterator::getNumPairs()
	{
	return numPairs;
	}

NX_INLINE NxShape * NxContactStreamIterator::getShape(NxU32 shapeIndex)
	{
	return shapes[shapeIndex];
	}

NX_INLINE NxU32 NxContactStreamIterator::getNumPatches()
	{
	return numPatches;
	}

NX_INLINE const NxVec3 & NxContactStreamIterator::getPatchNormal()
	{
	return *patchNormal;
	}

NX_INLINE NxU32 NxContactStreamIterator::getNumPoints()
	{
	return numPoints;
	}

NX_INLINE const NxVec3 & NxContactStreamIterator::getPoint()
	{
	return *point;
	}

NX_INLINE NxF32 NxContactStreamIterator::getSeparation()
	{
	return separation;
	}

NX_INLINE NxU32 NxContactStreamIterator::getExtData()
	{
	return extData;
	}

NX_INLINE bool NxContactStreamIterator::goNextPair()
	{
	if (numPairs--)
		{
#ifdef NX32
		shapes[0] = (NxShape*)*stream++;
		shapes[1] = (NxShape*)*stream++;
#else
		NxU64 low = (NxU64)*stream++;
		NxU64 high = (NxU64)*stream++;
		NxU64 bits = low|(high<<32);
		shapes[0] = (NxShape*)bits;
		low = (NxU64)*stream++;
		high = (NxU64)*stream++;
		bits = low|(high<<32);
		shapes[1] = (NxShape*)bits;
#endif
		NxU32 t = *stream++;
		numPatches = (NxU16) (t & 0xffff);
		shapeFlags = (NxU16) (t >> 16);

		return true;
		}
	else
		return false;
	}

NX_INLINE bool NxContactStreamIterator::goNextPatch()
	{
	if (numPatches--)
		{
		patchNormal = reinterpret_cast<const NxVec3 *>(stream);
		stream += 3;
		numPoints = *stream++;
		return true;
		}
	else
		return false;
	}

NX_INLINE bool NxContactStreamIterator::goNextPoint()
	{
	if (numPoints--)
		{
		point = reinterpret_cast<const NxVec3 *>(stream);
		stream += 3;
		NxU32 bind = *stream++;
		separation = NX_FR(bind);
		if (shapeFlags & NX_SF_HAS_MATS_PER_POINT)
			extData = *stream++;
		else
			extData = 0xffffffff;	//means that there is no ext data.

		//bind = *stream++;
		//materialIDs[0] = bind & 0xffff;
		//materialIDs[1] = bind >> 16;

		return true;
		}
	else
		return false;
	}


#endif
