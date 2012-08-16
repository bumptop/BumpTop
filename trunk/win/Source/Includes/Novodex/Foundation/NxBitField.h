#ifndef NX_FOUNDATION_NXBitField
#define NX_FOUNDATION_NXBitField
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

#include "Nx.h"

/**
 BitField class, for efficient storage of flags, and sub-byte width
 enums.  #bits can hypothetically be changed by changing the integer type of the flags member var.
 Previously this used to be a template class but it was too painful to maintain for what is probably zero benefit.
*/
class NxBitField
	{
	public:
	/**
	this could be bool, but an integer type is more efficient.  In any case, Flag variables
	should either be 1 or 0.
	*/
	typedef NxU32 IntType;	//currently I hardcode this to U32, it used to be templatized.  If we need more sizes later we can try to put the template back, its in the repository.

	typedef NxU32 Flag;
	typedef NxU32 Field;
	typedef NxU32 Mask;
	typedef NxU32 Shift;

	class FlagRef
		{
		public:
		NX_INLINE FlagRef(NxBitField & x, NxU32 index) : bitField(x), bitIndex(index)	{	}
		NX_INLINE const FlagRef & operator=(Flag f)
			{
			bitField.setFlag(bitIndex, f);
			return *this;
			}
		NX_INLINE operator Flag()
			{
			return bitField.getFlag(bitIndex);
			}
		private:

		NxBitField & bitField;
		NxU32 bitIndex;
		};



	//construction and assignment:
	//! default constructor leaves uninitialized.
	NX_INLINE NxBitField() { }
	NX_INLINE NxBitField(IntType);
	NX_INLINE NxBitField(const NxBitField &);
	NX_INLINE operator IntType()	{	return bitField;	}

	NX_INLINE const NxBitField & operator=(const NxBitField &);
	NX_INLINE const NxBitField & operator=(IntType);
	
	//!manipulating a single bit using a bit index.  The smallest bitIndex is 0.
	NX_INLINE void setFlag(NxU32 bitIndex, Flag value);
	NX_INLINE void raiseFlag(NxU32 bitIndex);
	NX_INLINE void lowerFlag(NxU32 bitIndex);
	NX_INLINE Flag getFlag(NxU32 bitIndex) const;


	//manipulating a single bit, using a single bit bit-mask
	NX_INLINE void setFlagMask(Mask mask, Flag value);
	NX_INLINE void raiseFlagMask(Mask mask);
	NX_INLINE void lowerFlagMask(Mask mask);
	NX_INLINE Flag getFlagMask(Mask mask) const;


	/**
	manipulating a set of bits: 
	shift is the lsb of the field
	mask is the value of all the raised flags in the field.

	Example: if the bits 4,5,6 of the bit field are being used,
	then shift is 4 and mask is (1<<4)|(1<<5)|(1<<6) == 1110000b = 112
	*/
	NX_INLINE Field getField(Shift shift, Mask mask) const;
	NX_INLINE void setField(Shift shift, Mask mask, Field field);
	NX_INLINE void clearField(Mask mask);

	//! more operators
	NX_INLINE FlagRef operator[](NxU32 bitIndex);
	//!statics

	static NX_INLINE Mask rangeToDenseMask(NxU32 lowIndex, NxU32 highIndex);
	static NX_INLINE Shift maskToShift(Mask mask); 


	IntType bitField;
	};

typedef NxBitField NxBitField32;



NX_INLINE NxBitField::NxBitField(IntType v)
	{
	bitField = v;
	}


NX_INLINE NxBitField::NxBitField(const NxBitField & r)
	{
	bitField = r.bitField;
	}


NX_INLINE void NxBitField::setFlag(NxU32 bitIndex, Flag value)
	{
	if (value)
		raiseFlag(bitIndex);
	else
		lowerFlag(bitIndex);
	}


NX_INLINE void NxBitField::raiseFlag(NxU32 bitIndex)
	{
	bitField |= (1 << bitIndex);
	}


NX_INLINE void NxBitField::lowerFlag(NxU32 bitIndex)
	{
	bitField &= ~(1 << bitIndex);
	}


NX_INLINE NxBitField::Flag NxBitField::getFlag(NxU32 bitIndex) const
	{
	return (bitField & (1 << bitIndex)) >> bitIndex;
	}



NX_INLINE void NxBitField::setFlagMask(Mask mask, Flag value)
	{
	if (value)
		raiseFlagMask(mask);
	else
		lowerFlagMask(mask);
	}


NX_INLINE void NxBitField::raiseFlagMask(Mask mask)
	{
	bitField |= mask;
	}


NX_INLINE void NxBitField::lowerFlagMask(Mask mask)
	{
	bitField &= ~mask;
	}


NX_INLINE NxBitField::Flag NxBitField::getFlagMask(Mask mask) const
	{
	return (bitField & mask) != 0;
	}


NX_INLINE NxBitField::Field NxBitField::getField(Shift shift, Mask mask) const
	{
	return ((bitField & mask) >>  shift);
	}


NX_INLINE void NxBitField::setField(Shift shift, Mask mask, Field field)
	{
	clearField(mask);
	bitField |= field << shift;
	}


NX_INLINE void  NxBitField::clearField(Mask mask)
	{
	bitField &= ~mask;
	}


NX_INLINE NxBitField::FlagRef NxBitField::operator[](NxU32 bitIndex)
	{
	return FlagRef(*this, bitIndex);
	}


NX_INLINE const NxBitField & NxBitField::operator=(const NxBitField &x)
	{
	bitField = x.bitField;
	return *this;
	}


NX_INLINE const NxBitField & NxBitField::operator=(IntType x)
	{
	bitField = x;
	return *this;
	}

#endif
