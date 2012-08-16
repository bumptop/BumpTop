#ifndef NX_FOUNDATION_NxMat33T
#define NX_FOUNDATION_NxMat33T
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

#include "NxVec3.h"
#include "NxQuat.h"

/**
 The idea of the matrix/vector classes is to partition them into two parts:
 One is the data structure which may have different formatting (3x3, 3x4, 4x4),
 row or column major.  The other is a template class which has all the operators
 but is storage format independent.

 This way it should be easier to change formats depending on what is faster/slower
 on a particular platform.

 Design issue: We use nameless struct/unions here.
 Design issue: this used to be implemented with a template.  This had no benefit
 but it added syntactic complexity.  Currently we just use a typedef and a preprocessor switch 
 to change between different memory layouts.

 The matrix math in this class is storage format (row/col major) independent as far
 as the user is concerned.
 When the user wants to get/set raw data, he needs to specify what order the data is
 coming in.  
 
*/

enum NxMatrixType
	{
	NX_ZERO_MATRIX,
	NX_IDENTITY_MATRIX
	};

#include "Nx9F32.h"				//change this if changing type below, to Nx9F32, Nx12F32, Nx16F32		

typedef Nx9Real Mat33DataType;	//takes Nx9Real, Nx12Real, Nx16Real

class NxMat33
	{
	public:
	NX_INLINE NxMat33();
	NX_INLINE NxMat33(NxMatrixType type);

	NX_INLINE NxMat33(const NxMat33&);
	NX_INLINE NxMat33(const NxQuat &);
	NX_INLINE ~NxMat33();
	NX_INLINE const NxMat33& operator=(const NxMat33 &src);

	// Access elements

	//low level data access, single or double precision, with eventual translation:
	//for dense 9 element data
	NX_INLINE void setRowMajor(const NxF32 *);
	NX_INLINE void setColumnMajor(const NxF32 *);
	NX_INLINE void getRowMajor(NxF32 *) const;
	NX_INLINE void getColumnMajor(NxF32 *) const;

	NX_INLINE void setRowMajor(const NxF64 *);
	NX_INLINE void setColumnMajor(const NxF64 *);
	NX_INLINE void getRowMajor(NxF64 *) const;
	NX_INLINE void getColumnMajor(NxF64 *) const;


	//for loose 4-padded data.
	NX_INLINE void setRowMajorStride4(const NxF32 *);
	NX_INLINE void setColumnMajorStride4(const NxF32 *);
	NX_INLINE void getRowMajorStride4(NxF32 *) const;
	NX_INLINE void getColumnMajorStride4(NxF32 *) const;

	NX_INLINE void setRowMajorStride4(const NxF64 *);
	NX_INLINE void setColumnMajorStride4(const NxF64 *);
	NX_INLINE void getRowMajorStride4(NxF64 *) const;
	NX_INLINE void getColumnMajorStride4(NxF64 *) const;


	NX_INLINE void setRow(int row, const NxVec3 &);
	NX_INLINE void setColumn(int col, const NxVec3 &);
	NX_INLINE void getRow(int row, NxVec3 &) const;
	NX_INLINE void getColumn(int col, NxVec3 &) const;

	//element access:
    NX_INLINE NxReal & operator()(int row, int col);
    NX_INLINE const NxReal & operator() (int row, int col) const;

	/**
	returns true for identity matrix
	*/
	NX_INLINE bool isIdentity() const;

	/**
	returns true for zero matrix
	*/
	NX_INLINE bool isZero() const;

	/**
	returns true if all elems are finite (not NAN or INF, etc.)
	*/
	NX_INLINE bool isFinite() const;

	//create special matrices:

	/**
	sets this matrix to the zero matrix.
	*/
	NX_INLINE void zero();

	/**
	sets this matrix to the identity matrix.
	*/
	NX_INLINE void id();

	/**
	this = -this
	*/
	NX_INLINE void setNegative();

	/**
	sets this matrix to the diagonal matrix.
	*/
	NX_INLINE void diagonal(const NxVec3 &vec);

	/**
	sets this matrix to the star matrix,
	so that star(v) * x = v.cross(x) .
	*/
	NX_INLINE void star(const NxVec3 &vec);


	NX_INLINE void fromQuat(const NxQuat &);
	NX_INLINE void toQuat(NxQuat &) const;

	//modifications:

	NX_INLINE const NxMat33 &operator +=(const NxMat33 &);
	NX_INLINE const NxMat33 &operator -=(const NxMat33 &);
	NX_INLINE const NxMat33 &operator *=(NxReal);
	NX_INLINE const NxMat33 &operator /=(NxReal);

	/**
	Gram-Schmidt orthogonalization to correct numerical drift, plus column normalization
	Caution: I believe the current implementation does not work right!
	*/
	NX_INLINE void orthonormalize();


	/**
	returns determinant
	*/
	NX_INLINE NxReal determinant();

	/**
	assigns inverse to dest.  Returns false if singular (i.e. if no inverse exists),
	setting dest to identity.
	*/
	NX_INLINE bool getInverse(NxMat33& dest) const;

	/**
	this = transpose(other)
	this == other is OK.
	*/
	NX_INLINE void setTransposed(const NxMat33& other);

	/**
	this = transpose(this)
	*/
	NX_INLINE void setTransposed();

	/**
	this = this * [ d.x 0 0; 0 d.y 0; 0 0 d.z];
	*/
	NX_INLINE void multiplyDiagonal(const NxVec3 &d);

	/**
	*/
	NX_INLINE void multiplyDiagonalTranspose(const NxVec3 &d);

	/**
	dst = this * [ d.x 0 0; 0 d.y 0; 0 0 d.z];
	*/
	NX_INLINE void multiplyDiagonal(const NxVec3 &d, NxMat33 &dst) const;

	/**
	dst = this * [ d.x 0 0; 0 d.y 0; 0 0 d.z];
	*/
	NX_INLINE void multiplyDiagonalTranspose(const NxVec3 &d, NxMat33 &dst) const;

	/**
	dst = this * src
	*/
	NX_INLINE void multiply(const NxVec3 &src, NxVec3 &dst) const;
	/**
	dst = transpose(this) * src
	*/
	NX_INLINE void multiplyByTranspose(const NxVec3 &src, NxVec3 &dst) const;

	/**
	this = a + b
	*/
	NX_INLINE void  add(const NxMat33 & a, const NxMat33 & b);
	/***
	this = a - b
	*/
	NX_INLINE void  subtract(const NxMat33 &a, const NxMat33 &b);
	/**
	this = s * a;
	*/
	NX_INLINE void  multiply(NxReal s,  const NxMat33 & a);
	/**
	this = left * right
	*/
	NX_INLINE void multiply(const NxMat33& left, const NxMat33& right);
	/**
	this = transpose(left) * right
	note: multiplyWithTransposed is faster.
	*/
	NX_INLINE void multiplyTransposeLeft(const NxMat33& left, const NxMat33& right);
	/**
	this = left * transpose(right)
	note: faster than multiplyByTranspose.
	*/
	NX_INLINE void multiplyTransposeRight(const NxMat33& left, const NxMat33& right);

	/**
	this = left * transpose(right)
	*/
	NX_INLINE void multiplyTransposeRight(const NxVec3 &left, const NxVec3 &right);


	//overloaded multiply, and transposed-multiply ops:

	/**
	returns transpose(this)*src
	*/
	NX_INLINE NxVec3 operator%  (const NxVec3 & src) const;
	/**
	matrix vector product
	*/
	NX_INLINE NxVec3 operator*  (const NxVec3 & src) const;
	/**
	matrix product
	*/
	NX_INLINE NxMat33&	operator*= (const NxMat33& mat);
	/**
	matrix difference
	*/
	NX_INLINE NxMat33	operator-  (const NxMat33& mat)	const;
	/**
	matrix addition
	*/
	NX_INLINE NxMat33	operator+  (const NxMat33& mat)	const;
	/**
	matrix product
	*/
	NX_INLINE NxMat33	operator*  (const NxMat33& mat)	const;
	/**
	matrix scalar product
	*/
	NX_INLINE NxMat33	operator*  (float s)				const;

	private:
	Mat33DataType data;
	};


NX_INLINE NxMat33::NxMat33()
	{
	}


NX_INLINE NxMat33::NxMat33(NxMatrixType type)
	{
		switch(type)
		{
			case NX_ZERO_MATRIX:		zero();	break;
			case NX_IDENTITY_MATRIX:	id();	break;
		}
	}


NX_INLINE NxMat33::NxMat33(const NxMat33& a)
	{
	data = a.data;
	}


NX_INLINE NxMat33::NxMat33(const NxQuat &q)
	{
	fromQuat(q);
	}



NX_INLINE NxMat33::~NxMat33()
	{
	//nothing
	}


NX_INLINE const NxMat33& NxMat33::operator=(const NxMat33 &a)
	{
	data = a.data;
	return *this;
	}


NX_INLINE void NxMat33::setRowMajor(const NxF32* d)
	{
	//we are also row major, so this is a direct copy
	data.s._11 = (NxReal)d[0];
	data.s._12 = (NxReal)d[1];
	data.s._13 = (NxReal)d[2];

	data.s._21 = (NxReal)d[3];
	data.s._22 = (NxReal)d[4];
	data.s._23 = (NxReal)d[5];

	data.s._31 = (NxReal)d[6];
	data.s._32 = (NxReal)d[7];
	data.s._33 = (NxReal)d[8];
	}


NX_INLINE void NxMat33::setColumnMajor(const NxF32* d)
	{
	//we are column major, so copy transposed.
	data.s._11 = (NxReal)d[0];
	data.s._12 = (NxReal)d[3];
	data.s._13 = (NxReal)d[6];

	data.s._21 = (NxReal)d[1];
	data.s._22 = (NxReal)d[4];
	data.s._23 = (NxReal)d[7];

	data.s._31 = (NxReal)d[2];
	data.s._32 = (NxReal)d[5];
	data.s._33 = (NxReal)d[8];
	}


NX_INLINE void NxMat33::getRowMajor(NxF32* d) const
	{
	//we are also row major, so this is a direct copy
	d[0] = (NxF32)data.s._11;
	d[1] = (NxF32)data.s._12;
	d[2] = (NxF32)data.s._13;

	d[3] = (NxF32)data.s._21;
	d[4] = (NxF32)data.s._22;
	d[5] = (NxF32)data.s._23;

	d[6] = (NxF32)data.s._31;
	d[7] = (NxF32)data.s._32;
	d[8] = (NxF32)data.s._33;
	}


NX_INLINE void NxMat33::getColumnMajor(NxF32* d) const
	{
	//we are column major, so copy transposed.
	d[0] = (NxF32)data.s._11;
	d[3] = (NxF32)data.s._12;
	d[6] = (NxF32)data.s._13;

	d[1] = (NxF32)data.s._21;
	d[4] = (NxF32)data.s._22;
	d[7] = (NxF32)data.s._23;

	d[2] = (NxF32)data.s._31;
	d[5] = (NxF32)data.s._32;
	d[8] = (NxF32)data.s._33;
	}


NX_INLINE void NxMat33::setRowMajorStride4(const NxF32* d)
	{
	//we are also row major, so this is a direct copy
	//however we've got to skip every 4th element.
	data.s._11 = (NxReal)d[0];
	data.s._12 = (NxReal)d[1];
	data.s._13 = (NxReal)d[2];

	data.s._21 = (NxReal)d[4];
	data.s._22 = (NxReal)d[5];
	data.s._23 = (NxReal)d[6];

	data.s._31 = (NxReal)d[8];
	data.s._32 = (NxReal)d[9];
	data.s._33 = (NxReal)d[10];
	}


NX_INLINE void NxMat33::setColumnMajorStride4(const NxF32* d)
	{
	//we are column major, so copy transposed.
	//however we've got to skip every 4th element.
	data.s._11 = (NxReal)d[0];
	data.s._12 = (NxReal)d[4];
	data.s._13 = (NxReal)d[8];

	data.s._21 = (NxReal)d[1];
	data.s._22 = (NxReal)d[5];
	data.s._23 = (NxReal)d[9];

	data.s._31 = (NxReal)d[2];
	data.s._32 = (NxReal)d[6];
	data.s._33 = (NxReal)d[10];
	}


NX_INLINE void NxMat33::getRowMajorStride4(NxF32* d) const
	{
	//we are also row major, so this is a direct copy
	//however we've got to skip every 4th element.
	d[0] = (NxF32)data.s._11;
	d[1] = (NxF32)data.s._12;
	d[2] = (NxF32)data.s._13;

	d[4] = (NxF32)data.s._21;
	d[5] = (NxF32)data.s._22;
	d[6] = (NxF32)data.s._23;

	d[8] = (NxF32)data.s._31;
	d[9] = (NxF32)data.s._32;
	d[10]= (NxF32)data.s._33;
	}


NX_INLINE void NxMat33::getColumnMajorStride4(NxF32* d) const
	{
	//we are column major, so copy transposed.
	//however we've got to skip every 4th element.
	d[0] = (NxF32)data.s._11;
	d[4] = (NxF32)data.s._12;
	d[8] = (NxF32)data.s._13;

	d[1] = (NxF32)data.s._21;
	d[5] = (NxF32)data.s._22;
	d[9] = (NxF32)data.s._23;

	d[2] = (NxF32)data.s._31;
	d[6] = (NxF32)data.s._32;
	d[10]= (NxF32)data.s._33;
	}


NX_INLINE void NxMat33::setRowMajor(const NxF64*d)
	{
	//we are also row major, so this is a direct copy
	data.s._11 = (NxReal)d[0];
	data.s._12 = (NxReal)d[1];
	data.s._13 = (NxReal)d[2];

	data.s._21 = (NxReal)d[3];
	data.s._22 = (NxReal)d[4];
	data.s._23 = (NxReal)d[5];

	data.s._31 = (NxReal)d[6];
	data.s._32 = (NxReal)d[7];
	data.s._33 = (NxReal)d[8];
	}


NX_INLINE void NxMat33::setColumnMajor(const NxF64*d)
	{
	//we are column major, so copy transposed.
	data.s._11 = (NxReal)d[0];
	data.s._12 = (NxReal)d[3];
	data.s._13 = (NxReal)d[6];

	data.s._21 = (NxReal)d[1];
	data.s._22 = (NxReal)d[4];
	data.s._23 = (NxReal)d[7];

	data.s._31 = (NxReal)d[2];
	data.s._32 = (NxReal)d[5];
	data.s._33 = (NxReal)d[8];
	}


NX_INLINE void NxMat33::getRowMajor(NxF64*d) const
	{
	//we are also row major, so this is a direct copy
	d[0] = (NxF64)data.s._11;
	d[1] = (NxF64)data.s._12;
	d[2] = (NxF64)data.s._13;

	d[3] = (NxF64)data.s._21;
	d[4] = (NxF64)data.s._22;
	d[5] = (NxF64)data.s._23;

	d[6] = (NxF64)data.s._31;
	d[7] = (NxF64)data.s._32;
	d[8] = (NxF64)data.s._33;
	}


NX_INLINE void NxMat33::getColumnMajor(NxF64*d) const
	{
	//we are column major, so copy transposed.
	d[0] = (NxF64)data.s._11;
	d[3] = (NxF64)data.s._12;
	d[6] = (NxF64)data.s._13;

	d[1] = (NxF64)data.s._21;
	d[4] = (NxF64)data.s._22;
	d[7] = (NxF64)data.s._23;

	d[2] = (NxF64)data.s._31;
	d[5] = (NxF64)data.s._32;
	d[8] = (NxF64)data.s._33;
	}


NX_INLINE void NxMat33::setRowMajorStride4(const NxF64*d)
	{
	//we are also row major, so this is a direct copy
	//however we've got to skip every 4th element.
	data.s._11 = (NxReal)d[0];
	data.s._12 = (NxReal)d[1];
	data.s._13 = (NxReal)d[2];

	data.s._21 = (NxReal)d[4];
	data.s._22 = (NxReal)d[5];
	data.s._23 = (NxReal)d[6];

	data.s._31 = (NxReal)d[8];
	data.s._32 = (NxReal)d[9];
	data.s._33 = (NxReal)d[10];
	}


NX_INLINE void NxMat33::setColumnMajorStride4(const NxF64*d)
	{
	//we are column major, so copy transposed.
	//however we've got to skip every 4th element.
	data.s._11 = (NxReal)d[0];
	data.s._12 = (NxReal)d[4];
	data.s._13 = (NxReal)d[8];

	data.s._21 = (NxReal)d[1];
	data.s._22 = (NxReal)d[5];
	data.s._23 = (NxReal)d[9];

	data.s._31 = (NxReal)d[2];
	data.s._32 = (NxReal)d[6];
	data.s._33 = (NxReal)d[10];
	}


NX_INLINE void NxMat33::getRowMajorStride4(NxF64*d) const
	{
	//we are also row major, so this is a direct copy
	//however we've got to skip every 4th element.
	d[0] = (NxF64)data.s._11;
	d[1] = (NxF64)data.s._12;
	d[2] = (NxF64)data.s._13;

	d[4] = (NxF64)data.s._21;
	d[5] = (NxF64)data.s._22;
	d[6] = (NxF64)data.s._23;

	d[8] = (NxF64)data.s._31;
	d[9] = (NxF64)data.s._32;
	d[10]= (NxF64)data.s._33;
	}


NX_INLINE void NxMat33::getColumnMajorStride4(NxF64*d) const

	{
	//we are column major, so copy transposed.
	//however we've got to skip every 4th element.
	d[0] = (NxF64)data.s._11;
	d[4] = (NxF64)data.s._12;
	d[8] = (NxF64)data.s._13;

	d[1] = (NxF64)data.s._21;
	d[5] = (NxF64)data.s._22;
	d[9] = (NxF64)data.s._23;

	d[2] = (NxF64)data.s._31;
	d[6] = (NxF64)data.s._32;
	d[10]= (NxF64)data.s._33;
	}


NX_INLINE void NxMat33::setRow(int row, const NxVec3 & v)
	{
#ifndef TRANSPOSED_MAT33
	data.m[row][0] = v.x;
	data.m[row][1] = v.y;
	data.m[row][2] = v.z;
#else
	data.m[0][row] = v.x;
	data.m[1][row] = v.y;
	data.m[2][row] = v.z;
#endif
	}


NX_INLINE void NxMat33::setColumn(int col, const NxVec3 & v)
	{
#ifndef TRANSPOSED_MAT33
	data.m[0][col] = v.x;
	data.m[1][col] = v.y;
	data.m[2][col] = v.z;
#else
	data.m[col][0] = v.x;
	data.m[col][1] = v.y;
	data.m[col][2] = v.z;
#endif
	}


NX_INLINE void NxMat33::getRow(int row, NxVec3 & v) const
	{
#ifndef TRANSPOSED_MAT33
	v.x = data.m[row][0];
	v.y = data.m[row][1];
	v.z = data.m[row][2];
#else
	v.x = data.m[0][row];
	v.y = data.m[1][row];
	v.z = data.m[2][row];
#endif
	}


NX_INLINE void NxMat33::getColumn(int col, NxVec3 & v) const
	{
#ifndef TRANSPOSED_MAT33
	v.x = data.m[0][col];
	v.y = data.m[1][col];
	v.z = data.m[2][col];
#else
	v.x = data.m[col][0];
	v.y = data.m[col][1];
	v.z = data.m[col][2];
#endif
	}


NX_INLINE NxReal & NxMat33::operator()(int row, int col)
	{
#ifndef TRANSPOSED_MAT33
	return data.m[row][col];
#else
	return data.m[col][row];
#endif
	}


NX_INLINE const NxReal & NxMat33::operator() (int row, int col) const
	{
#ifndef TRANSPOSED_MAT33
	return data.m[row][col];
#else
	return data.m[col][row];
#endif
	}

//const methods


NX_INLINE bool NxMat33::isIdentity() const
	{
	if(NX_IR(data.s._11)!=NX_IEEE_1_0)	return false;
	if(NX_IR(data.s._12))				return false;
	if(NX_IR(data.s._13))				return false;

	if(NX_IR(data.s._21))				return false;
	if(NX_IR(data.s._22)!=NX_IEEE_1_0)	return false;
	if(NX_IR(data.s._23))				return false;

	if(NX_IR(data.s._31))				return false;
	if(NX_IR(data.s._32))				return false;
	if(NX_IR(data.s._33)!=NX_IEEE_1_0)	return false;
	return true;
	}


NX_INLINE bool NxMat33::isZero() const
	{
	if(NX_IR(data.s._11))	return false;
	if(NX_IR(data.s._12))	return false;
	if(NX_IR(data.s._13))	return false;

	if(NX_IR(data.s._21))	return false;
	if(NX_IR(data.s._22))	return false;
	if(NX_IR(data.s._23))	return false;

	if(NX_IR(data.s._31))	return false;
	if(NX_IR(data.s._32))	return false;
	if(NX_IR(data.s._33))	return false;
	return true;
	}


NX_INLINE bool NxMat33::isFinite() const
	{
	return NxMath::isFinite(data.s._11)
	&& NxMath::isFinite(data.s._12)
	&& NxMath::isFinite(data.s._13)

	&& NxMath::isFinite(data.s._21)
	&& NxMath::isFinite(data.s._22)
	&& NxMath::isFinite(data.s._23)

	&& NxMath::isFinite(data.s._31)
	&& NxMath::isFinite(data.s._32)
	&& NxMath::isFinite(data.s._33);
	}



NX_INLINE void NxMat33::zero()
	{
	data.s._11 = NxReal(0.0);
	data.s._12 = NxReal(0.0);
	data.s._13 = NxReal(0.0);

	data.s._21 = NxReal(0.0);
	data.s._22 = NxReal(0.0);
	data.s._23 = NxReal(0.0);

	data.s._31 = NxReal(0.0);
	data.s._32 = NxReal(0.0);
	data.s._33 = NxReal(0.0);
	}


NX_INLINE void NxMat33::id()
	{
	data.s._11 = NxReal(1.0);
	data.s._12 = NxReal(0.0);
	data.s._13 = NxReal(0.0);

	data.s._21 = NxReal(0.0);
	data.s._22 = NxReal(1.0);
	data.s._23 = NxReal(0.0);

	data.s._31 = NxReal(0.0);
	data.s._32 = NxReal(0.0);
	data.s._33 = NxReal(1.0);
	}


NX_INLINE void NxMat33::setNegative()
	{
	data.s._11 = -data.s._11;
	data.s._12 = -data.s._12;
	data.s._13 = -data.s._13;

	data.s._21 = -data.s._21;
	data.s._22 = -data.s._22;
	data.s._23 = -data.s._23;

	data.s._31 = -data.s._31;
	data.s._32 = -data.s._32;
	data.s._33 = -data.s._33;
	}


NX_INLINE void NxMat33::diagonal(const NxVec3 &v)
	{
	data.s._11 = v.x;
	data.s._12 = NxReal(0.0);
	data.s._13 = NxReal(0.0);

	data.s._21 = NxReal(0.0);
	data.s._22 = v.y;
	data.s._23 = NxReal(0.0);

	data.s._31 = NxReal(0.0);
	data.s._32 = NxReal(0.0);
	data.s._33 = v.z;
	}


NX_INLINE void NxMat33::star(const NxVec3 &v)
	{
	data.s._11 = NxReal(0.0);	data.s._12 =-v.z;	data.s._13 = v.y;
	data.s._21 = v.z;	data.s._22 = NxReal(0.0);	data.s._23 =-v.x;
	data.s._31 =-v.y;	data.s._32 = v.x;	data.s._33 = NxReal(0.0);
	}


NX_INLINE void NxMat33::fromQuat(const NxQuat & q)
	{
	const NxReal w = q.w;
	const NxReal x = q.x;
	const NxReal y = q.y;
	const NxReal z = q.z;

	data.s._11 = NxReal(1.0) - y*y*NxReal(2.0) - z*z*NxReal(2.0);
	data.s._12 = x*y*NxReal(2.0) - w*z*NxReal(2.0);	
	data.s._13 = x*z*NxReal(2.0) + w*y*NxReal(2.0);	

	data.s._21 = x*y*NxReal(2.0) + w*z*NxReal(2.0);	
	data.s._22 = NxReal(1.0) - x*x*NxReal(2.0) - z*z*NxReal(2.0);	
	data.s._23 = y*z*NxReal(2.0) - w*x*NxReal(2.0);	
	
	data.s._31 = x*z*NxReal(2.0) - w*y*NxReal(2.0);	
	data.s._32 = y*z*NxReal(2.0) + w*x*NxReal(2.0);	
	data.s._33 = NxReal(1.0) - x*x*NxReal(2.0) - y*y*NxReal(2.0);	
	}


NX_INLINE void NxMat33::toQuat(NxQuat & q) const					// set the NxQuat from a rotation matrix
	{
    NxReal tr, s;
    tr = data.s._11 + data.s._22 + data.s._33;
    if(tr >= 0)
		{
		s = (NxReal)NxMath::sqrt(tr +1);
		q.w = NxReal(0.5) * s;
		s = NxReal(0.5) / s;
		q.x = ((*this)(2,1) - (*this)(1,2)) * s;
		q.y = ((*this)(0,2) - (*this)(2,0)) * s;
		q.z = ((*this)(1,0) - (*this)(0,1)) * s;
		}
    else
		{
		int i = 0; 
		if (data.s._22 > data.s._11)
			i = 1; 
		if(data.s._33 > (*this)(i,i))
			i=2; 
		switch (i)
			{
			case 0:
				s = (NxReal)NxMath::sqrt((data.s._11 - (data.s._22 + data.s._33)) + 1);
				q.x = NxReal(0.5) * s;
				s = NxReal(0.5) / s;
				q.y = ((*this)(0,1) + (*this)(1,0)) * s; 
				q.z = ((*this)(2,0) + (*this)(0,2)) * s;
				q.w = ((*this)(2,1) - (*this)(1,2)) * s;
				break;
			case 1:
				s = (NxReal)NxMath::sqrt((data.s._22 - (data.s._33 + data.s._11)) + 1);
				q.y = NxReal(0.5) * s;
				s = NxReal(0.5) / s;
				q.z = ((*this)(1,2) + (*this)(2,1)) * s;
				q.x = ((*this)(0,1) + (*this)(1,0)) * s;
				q.w = ((*this)(0,2) - (*this)(2,0)) * s;
				break;
			case 2:
				s = (NxReal)NxMath::sqrt((data.s._33 - (data.s._11 + data.s._22)) + 1);
				q.z = NxReal(0.5) * s;
				s = NxReal(0.5) / s;
				q.x = ((*this)(2,0) + (*this)(0,2)) * s;
				q.y = ((*this)(1,2) + (*this)(2,1)) * s;
				q.w = ((*this)(1,0) - (*this)(0,1)) * s;
			}
		}
	}
/*

NX_INLINE void NxMat33::orthonormalize()	//Gram-Schmidt orthogonalization to correct numerical drift, plus column normalization
	{
	//TODO: This is buggy!
	NxVec3 w,t1,t2,t3;
	NxReal norm_sq;

    const NxReal m=3;			//m := linalg[rowdim](A);
    const NxReal n=3;			//n := linalg[coldim](A);
	int i, j, k = 0;				//k := 0;


    Mat33d v = *this;				//v := linalg[col](A, 1 .. n); -- 3 column vectors indexable
    NxVec3 norm_u_sq;
																//# orthogonalize v[i]
    for (i=0; i<n; i++)//for i to n do
		{
        v.getColumn(i,w);		//i-th column
        for (j=0; j<k; j++)									//# pull w along projection of v[i] with u[j]
			{
			this->getColumn(j,t1);
			this->getColumn(j,t2);
			v.getColumn(i,t3);
			NxVec3 temp = (t2 * (NxReal(1.0)/norm_u_sq[j]));
			NxVec3 temp2 = temp  * t3.dot( t1 );
			w -= temp;	
			}
																//        # compute norm of orthogonalized v[i]
      norm_sq = w.Dot(w);

		if (norm_sq != NxReal(0.0)) 
			{													//           # linearly independent new orthogonal vector 
																//       # add to list of u and norm_u_sq
			this->SetColumn(i,w);									//u = [op(u), evalm(w)];
            norm_u_sq[i] = norm_sq;						//norm_u_sq = [op(norm_u_sq), norm_sq];
            k ++;
			}
		}
	

	NxVec3 temp;													//may want to do this in-place -- dunno if optimizer does this for me
	for (i=0; i<3; i++)
		{
		getColumn(i,temp);
		temp.normalize();
		setColumn(i,temp);
		}
	}
	*/


NX_INLINE void NxMat33::setTransposed(const NxMat33& other)
	{
	//gotta special case in-place case
	if (this != &other)
		{
		data.s._11 = other.data.s._11;
		data.s._12 = other.data.s._21;
		data.s._13 = other.data.s._31;

		data.s._21 = other.data.s._12;
		data.s._22 = other.data.s._22;
		data.s._23 = other.data.s._32;

		data.s._31 = other.data.s._13;
		data.s._32 = other.data.s._23;
		data.s._33 = other.data.s._33;
		}
	else
		{
		NxReal tx, ty, tz;
		tx = data.s._21;	data.s._21 = other.data.s._12;	data.s._12 = tx;
		ty = data.s._31;	data.s._31 = other.data.s._13;	data.s._13 = ty;
		tz = data.s._32;	data.s._32 = other.data.s._23;	data.s._23 = tz;
		}
	}


NX_INLINE void NxMat33::setTransposed()
	{
		NX_Swap(data.s._12, data.s._21);
		NX_Swap(data.s._23, data.s._32);
		NX_Swap(data.s._13, data.s._31);
	}


NX_INLINE void NxMat33::multiplyDiagonal(const NxVec3 &d)
	{
	data.s._11 *= d.x;
	data.s._12 *= d.y;
	data.s._13 *= d.z;

	data.s._21 *= d.x;
	data.s._22 *= d.y;
	data.s._23 *= d.z;

	data.s._31 *= d.x;
	data.s._32 *= d.y;
	data.s._33 *= d.z;
	}


NX_INLINE void NxMat33::multiplyDiagonalTranspose(const NxVec3 &d)
	{
	data.s._11 *= d.x;
	data.s._12 *= d.x;
	data.s._13 *= d.x;

	data.s._21 *= d.y;
	data.s._22 *= d.y;
	data.s._23 *= d.y;

	data.s._31 *= d.z;
	data.s._32 *= d.z;
	data.s._33 *= d.z;
	}


NX_INLINE void NxMat33::multiplyDiagonal(const NxVec3 &d, NxMat33& dst) const
	{
	dst.data.s._11 = data.s._11 * d.x;
	dst.data.s._12 = data.s._12 * d.y;
	dst.data.s._13 = data.s._13 * d.z;

	dst.data.s._21 = data.s._21 * d.x;
	dst.data.s._22 = data.s._22 * d.y;
	dst.data.s._23 = data.s._23 * d.z;

	dst.data.s._31 = data.s._31 * d.x;
	dst.data.s._32 = data.s._32 * d.y;
	dst.data.s._33 = data.s._33 * d.z;
	}


NX_INLINE void NxMat33::multiplyDiagonalTranspose(const NxVec3 &d, NxMat33& dst) const
	{
	dst.data.s._11 = data.s._11 * d.x;
	dst.data.s._12 = data.s._21 * d.y;
	dst.data.s._13 = data.s._31 * d.z;

	dst.data.s._21 = data.s._12 * d.x;
	dst.data.s._22 = data.s._22 * d.y;
	dst.data.s._23 = data.s._32 * d.z;

	dst.data.s._31 = data.s._13 * d.x;
	dst.data.s._32 = data.s._23 * d.y;
	dst.data.s._33 = data.s._33 * d.z;
	}


NX_INLINE void NxMat33::multiply(const NxVec3 &src, NxVec3 &dst) const
	{
	NxReal x,y,z;	//so it works if src == dst
	x = data.s._11 * src.x + data.s._12 * src.y + data.s._13 * src.z;
	y = data.s._21 * src.x + data.s._22 * src.y + data.s._23 * src.z;
	z = data.s._31 * src.x + data.s._32 * src.y + data.s._33 * src.z;

	dst.x = x;
	dst.y = y;
	dst.z = z;	
	}


NX_INLINE void NxMat33::multiplyByTranspose(const NxVec3 &src, NxVec3 &dst) const
	{
	NxReal x,y,z;	//so it works if src == dst
	x = data.s._11 * src.x + data.s._21 * src.y + data.s._31 * src.z;
	y = data.s._12 * src.x + data.s._22 * src.y + data.s._32 * src.z;
	z = data.s._13 * src.x + data.s._23 * src.y + data.s._33 * src.z;

	dst.x = x;
	dst.y = y;
	dst.z = z;	
	}


NX_INLINE void NxMat33::add(const NxMat33 & a, const NxMat33 & b)
	{
	data.s._11 = a.data.s._11 + b.data.s._11;
	data.s._12 = a.data.s._12 + b.data.s._12;
	data.s._13 = a.data.s._13 + b.data.s._13;

	data.s._21 = a.data.s._21 + b.data.s._21;
	data.s._22 = a.data.s._22 + b.data.s._22;
	data.s._23 = a.data.s._23 + b.data.s._23;

	data.s._31 = a.data.s._31 + b.data.s._31;
	data.s._32 = a.data.s._32 + b.data.s._32;
	data.s._33 = a.data.s._33 + b.data.s._33;
	}


NX_INLINE void NxMat33::subtract(const NxMat33 &a, const NxMat33 &b)
	{
	data.s._11 = a.data.s._11 - b.data.s._11;
	data.s._12 = a.data.s._12 - b.data.s._12;
	data.s._13 = a.data.s._13 - b.data.s._13;

	data.s._21 = a.data.s._21 - b.data.s._21;
	data.s._22 = a.data.s._22 - b.data.s._22;
	data.s._23 = a.data.s._23 - b.data.s._23;

	data.s._31 = a.data.s._31 - b.data.s._31;
	data.s._32 = a.data.s._32 - b.data.s._32;
	data.s._33 = a.data.s._33 - b.data.s._33;
	}


NX_INLINE void NxMat33::multiply(NxReal d,  const NxMat33 & a)
	{
	data.s._11 = a.data.s._11 * d;
	data.s._12 = a.data.s._12 * d;
	data.s._13 = a.data.s._13 * d;

	data.s._21 = a.data.s._21 * d;
	data.s._22 = a.data.s._22 * d;
	data.s._23 = a.data.s._23 * d;

	data.s._31 = a.data.s._31 * d;
	data.s._32 = a.data.s._32 * d;
	data.s._33 = a.data.s._33 * d;
	}


NX_INLINE void NxMat33::multiply(const NxMat33& left, const NxMat33& right)
	{
	NxReal a,b,c, d,e,f, g,h,i;
	//note: temps needed so that x.multiply(x,y) works OK.
	a =left.data.s._11 * right.data.s._11 +left.data.s._12 * right.data.s._21 +left.data.s._13 * right.data.s._31;
	b =left.data.s._11 * right.data.s._12 +left.data.s._12 * right.data.s._22 +left.data.s._13 * right.data.s._32;
	c =left.data.s._11 * right.data.s._13 +left.data.s._12 * right.data.s._23 +left.data.s._13 * right.data.s._33;

	d =left.data.s._21 * right.data.s._11 +left.data.s._22 * right.data.s._21 +left.data.s._23 * right.data.s._31;
	e =left.data.s._21 * right.data.s._12 +left.data.s._22 * right.data.s._22 +left.data.s._23 * right.data.s._32;
	f =left.data.s._21 * right.data.s._13 +left.data.s._22 * right.data.s._23 +left.data.s._23 * right.data.s._33;

	g =left.data.s._31 * right.data.s._11 +left.data.s._32 * right.data.s._21 +left.data.s._33 * right.data.s._31;
	h =left.data.s._31 * right.data.s._12 +left.data.s._32 * right.data.s._22 +left.data.s._33 * right.data.s._32;
	i =left.data.s._31 * right.data.s._13 +left.data.s._32 * right.data.s._23 +left.data.s._33 * right.data.s._33;


	data.s._11 = a;
	data.s._12 = b;
	data.s._13 = c;

	data.s._21 = d;
	data.s._22 = e;
	data.s._23 = f;

	data.s._31 = g;
	data.s._32 = h;
	data.s._33 = i;
	}


NX_INLINE void NxMat33::multiplyTransposeLeft(const NxMat33& left, const NxMat33& right)
	{
	NxReal a,b,c, d,e,f, g,h,i;
	//note: temps needed so that x.multiply(x,y) works OK.
	a =left.data.s._11 * right.data.s._11 +left.data.s._21 * right.data.s._21 +left.data.s._31 * right.data.s._31;
	b =left.data.s._11 * right.data.s._12 +left.data.s._21 * right.data.s._22 +left.data.s._31 * right.data.s._32;
	c =left.data.s._11 * right.data.s._13 +left.data.s._21 * right.data.s._23 +left.data.s._31 * right.data.s._33;

	d =left.data.s._12 * right.data.s._11 +left.data.s._22 * right.data.s._21 +left.data.s._32 * right.data.s._31;
	e =left.data.s._12 * right.data.s._12 +left.data.s._22 * right.data.s._22 +left.data.s._32 * right.data.s._32;
	f =left.data.s._12 * right.data.s._13 +left.data.s._22 * right.data.s._23 +left.data.s._32 * right.data.s._33;

	g =left.data.s._13 * right.data.s._11 +left.data.s._23 * right.data.s._21 +left.data.s._33 * right.data.s._31;
	h =left.data.s._13 * right.data.s._12 +left.data.s._23 * right.data.s._22 +left.data.s._33 * right.data.s._32;
	i =left.data.s._13 * right.data.s._13 +left.data.s._23 * right.data.s._23 +left.data.s._33 * right.data.s._33;

	data.s._11 = a;
	data.s._12 = b;
	data.s._13 = c;

	data.s._21 = d;
	data.s._22 = e;
	data.s._23 = f;

	data.s._31 = g;
	data.s._32 = h;
	data.s._33 = i;
	}


NX_INLINE void NxMat33::multiplyTransposeRight(const NxMat33& left, const NxMat33& right)
	{
	NxReal a,b,c, d,e,f, g,h,i;
	//note: temps needed so that x.multiply(x,y) works OK.
	a =left.data.s._11 * right.data.s._11 +left.data.s._12 * right.data.s._12 +left.data.s._13 * right.data.s._13;
	b =left.data.s._11 * right.data.s._21 +left.data.s._12 * right.data.s._22 +left.data.s._13 * right.data.s._23;
	c =left.data.s._11 * right.data.s._31 +left.data.s._12 * right.data.s._32 +left.data.s._13 * right.data.s._33;

	d =left.data.s._21 * right.data.s._11 +left.data.s._22 * right.data.s._12 +left.data.s._23 * right.data.s._13;
	e =left.data.s._21 * right.data.s._21 +left.data.s._22 * right.data.s._22 +left.data.s._23 * right.data.s._23;
	f =left.data.s._21 * right.data.s._31 +left.data.s._22 * right.data.s._32 +left.data.s._23 * right.data.s._33;

	g =left.data.s._31 * right.data.s._11 +left.data.s._32 * right.data.s._12 +left.data.s._33 * right.data.s._13;
	h =left.data.s._31 * right.data.s._21 +left.data.s._32 * right.data.s._22 +left.data.s._33 * right.data.s._23;
	i =left.data.s._31 * right.data.s._31 +left.data.s._32 * right.data.s._32 +left.data.s._33 * right.data.s._33;

	data.s._11 = a;
	data.s._12 = b;
	data.s._13 = c;

	data.s._21 = d;
	data.s._22 = e;
	data.s._23 = f;

	data.s._31 = g;
	data.s._32 = h;
	data.s._33 = i;
	}


NX_INLINE void NxMat33::multiplyTransposeRight(const NxVec3 &left, const NxVec3 &right)
	{
	data.s._11 = left.x * right.x;
	data.s._12 = left.x * right.y;
	data.s._13 = left.x * right.z;

	data.s._21 = left.y * right.x;
	data.s._22 = left.y * right.y;
	data.s._23 = left.y * right.z;

	data.s._31 = left.z * right.x;
	data.s._32 = left.z * right.y;
	data.s._33 = left.z * right.z;
	}


NX_INLINE NxVec3  NxMat33::operator%(const NxVec3 & src) const
	{
	NxVec3 dest;
	this->multiplyByTranspose(src, dest);
	return dest;
	}


NX_INLINE NxVec3  NxMat33::operator*(const NxVec3 & src) const
	{
	NxVec3 dest;
	this->multiply(src, dest);
	return dest;
	}


NX_INLINE const NxMat33 &NxMat33::operator +=(const NxMat33 &d)
	{
	data.s._11 += d.data.s._11;
	data.s._12 += d.data.s._12;
	data.s._13 += d.data.s._13;

	data.s._21 += d.data.s._21;
	data.s._22 += d.data.s._22;
	data.s._23 += d.data.s._23;

	data.s._31 += d.data.s._31;
	data.s._32 += d.data.s._32;
	data.s._33 += d.data.s._33;
	return *this;
	}


NX_INLINE const NxMat33 &NxMat33::operator -=(const NxMat33 &d)
	{
	data.s._11 -= d.data.s._11;
	data.s._12 -= d.data.s._12;
	data.s._13 -= d.data.s._13;

	data.s._21 -= d.data.s._21;
	data.s._22 -= d.data.s._22;
	data.s._23 -= d.data.s._23;

	data.s._31 -= d.data.s._31;
	data.s._32 -= d.data.s._32;
	data.s._33 -= d.data.s._33;
	return *this;
	}


NX_INLINE const NxMat33 &NxMat33::operator *=(NxReal f)
	{
	data.s._11 *= f;
	data.s._12 *= f;
	data.s._13 *= f;

	data.s._21 *= f;
	data.s._22 *= f;
	data.s._23 *= f;

	data.s._31 *= f;
	data.s._32 *= f;
	data.s._33 *= f;
	return *this;
	}


NX_INLINE const NxMat33 &NxMat33::operator /=(NxReal x)
	{
	NxReal f = NxReal(1.0) / x;
	data.s._11 *= f;
	data.s._12 *= f;
	data.s._13 *= f;

	data.s._21 *= f;
	data.s._22 *= f;
	data.s._23 *= f;

	data.s._31 *= f;
	data.s._32 *= f;
	data.s._33 *= f;
	return *this;
	}


NX_INLINE NxReal NxMat33::determinant()
	{
	return  data.s._11*data.s._22*data.s._33 + data.s._12*data.s._23*data.s._31 + data.s._13*data.s._21*data.s._32 
		  - data.s._13*data.s._22*data.s._31 - data.s._12*data.s._21*data.s._33 - data.s._11*data.s._23*data.s._32;
	}


bool NxMat33::getInverse(NxMat33& dest) const
	{
	NxReal b00,b01,b02,b10,b11,b12,b20,b21,b22;

	b00 = data.s._22*data.s._33-data.s._23*data.s._32;	b01 = data.s._13*data.s._32-data.s._12*data.s._33;	b02 = data.s._12*data.s._23-data.s._13*data.s._22;
	b10 = data.s._23*data.s._31-data.s._21*data.s._33;	b11 = data.s._11*data.s._33-data.s._13*data.s._31;	b12 = data.s._13*data.s._21-data.s._11*data.s._23;
	b20 = data.s._21*data.s._32-data.s._22*data.s._31;	b21 = data.s._12*data.s._31-data.s._11*data.s._32;	b22 = data.s._11*data.s._22-data.s._12*data.s._21;
	


	/*
	compute determinant: 
	NxReal d =   a00*a11*a22 + a01*a12*a20 + a02*a10*a21	- a02*a11*a20 - a01*a10*a22 - a00*a12*a21;
				0				1			2			3				4			5

	this is a subset of the multiplies done above:

	NxReal d = b00*a00				+		b01*a10						 + b02 * a20;
	NxReal d = (a11*a22-a12*a21)*a00 +		(a02*a21-a01*a22)*a10		 + (a01*a12-a02*a11) * a20;

	NxReal d = a11*a22*a00-a12*a21*a00 +		a02*a21*a10-a01*a22*a10		 + a01*a12*a20-a02*a11*a20;
			0			5					2			4					1			3
	*/

	NxReal d = b00*data.s._11		+		b01*data.s._21				 + b02 * data.s._31;
	
	if (d == NxReal(0.0))		//singular?
		{
		dest.id();
		return false;
		}
	
	d = NxReal(1.0)/d;

	//only do assignment at the end, in case dest == this:


	dest.data.s._11 = b00*d; dest.data.s._12 = b01*d; dest.data.s._13 = b02*d;
	dest.data.s._21 = b10*d; dest.data.s._22 = b11*d; dest.data.s._23 = b12*d;
	dest.data.s._31 = b20*d; dest.data.s._32 = b21*d; dest.data.s._33 = b22*d;

	return true;
	}


NX_INLINE NxMat33&	NxMat33::operator*= (const NxMat33& mat)
	{
	this->multiply(*this, mat);
	return *this;
	}


NX_INLINE NxMat33	NxMat33::operator-  (const NxMat33& mat)	const
	{
	NxMat33 temp;
	temp.subtract(*this, mat);
	return temp;
	}


NX_INLINE NxMat33	NxMat33::operator+  (const NxMat33& mat)	const
	{
	NxMat33 temp;
	temp.add(*this, mat);
	return temp;
	}


NX_INLINE NxMat33	NxMat33::operator*  (const NxMat33& mat)	const
	{
	NxMat33 temp;
	temp.multiply(*this, mat);
	return temp;
	}


NX_INLINE NxMat33	NxMat33::operator*  (float s)			const
	{
	NxMat33 temp;
	temp.multiply(s, *this);
	return temp;
	}

#endif
