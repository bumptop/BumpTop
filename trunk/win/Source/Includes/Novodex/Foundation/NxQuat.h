#ifndef NX_FOUNDATION_NxQuatT
#define NX_FOUNDATION_NxQuatT
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

#include "Nxf.h"
#include "NxVec3.h"

/**
 This is a quaternion class.  The stored data is not public because the storage
 order of the four elements is not standardized between applications and we don't
 want to be tripped up by assumptions.

 no direct data access because storage order is ambiguous.
 no 4 float ctor because storage order is ambiguous.

  T = data type for quat data
  NxReal = data type for float data

*/

class NxQuat
	{
	public:
	/**
	Default constructor, does not do any initialization.
	*/
	NX_INLINE NxQuat();

	/**
	Copy constructor.
	*/
	NX_INLINE NxQuat(const NxQuat&);

	/**
	copies xyz elements, sets w to zero.
	*/
	NX_INLINE NxQuat(const NxVec3& v);

	/**
	creates from angle-axis representation.
	note that if Angle > 360 the resulting rotation is Angle mod 360.
	Angle is in degrees.
	*/
	NX_INLINE NxQuat(const NxReal angle, const NxVec3 & axis);

	NX_INLINE void id();

	//setting:
	NX_INLINE void setWXYZ(NxReal w, NxReal x, NxReal y, NxReal z);
	NX_INLINE void setXYZW(NxReal x, NxReal y, NxReal z, NxReal w);
	NX_INLINE void setWXYZ(const NxReal *);
	NX_INLINE void setXYZW(const NxReal *);

	NX_INLINE NxQuat& operator=  (const NxQuat&);

	/**
	implicitly extends vector by a 0 w element.
	*/
	NX_INLINE NxQuat& operator=  (const NxVec3&);

	NX_INLINE void setx(const NxReal& d);
	NX_INLINE void sety(const NxReal& d);
	NX_INLINE void setz(const NxReal& d);
	NX_INLINE void setw(const NxReal& d);

	NX_INLINE void getWXYZ(NxF32 *) const;
	NX_INLINE void getXYZW(NxF32 *) const;

	NX_INLINE void getWXYZ(NxF64 *) const;
	NX_INLINE void getXYZW(NxF64 *) const;

	/**
	returns true if all elems are finite (not NAN or INF, etc.)
	*/
	NX_INLINE bool isFinite() const;

	/**
	sets to the quat [0,0,0,1]
	*/
	NX_INLINE void zero();

	/**
	creates a random unit quaternion.
	*/
	NX_INLINE void random();
	/**
	creates from angle-axis representation.
	note that if Angle > 360 the resulting rotation is Angle mod 360.
	Angle is in degrees.
	*/
	NX_INLINE void fromAngleAxis(NxReal angle, const NxVec3 & axis);

	/**
	fetches the Angle/axis given by the NxQuat.  Angle is in degrees.
	*/
	NX_INLINE void getAngleAxis(NxReal& Angle, NxVec3 & axis) const;

	/**
	gets the angle between this quat and the identity quaternion.
	*/
	NX_INLINE NxReal getAngle() const;

	/**
	gets the angle between this quat and the argument
	*/
	NX_INLINE NxReal getAngle(const NxQuat &) const;

	/**
	this is the squared 4D vector length, should be 1 for unit quaternions.
	*/
	NX_INLINE NxReal magnitudeSquared() const;

	/**
	returns the scalar product of 
	this and other.
	*/
	NX_INLINE NxReal dot(const NxQuat &) const;

	//modifiers:
	/**
	maps to the closest unit quaternion.
	*/
	NX_INLINE void normalize();

	/*
	assigns its own conjugate to itself.
	Note: for unit quats, this is the inverse.
	*/
	NX_INLINE void conjugate();

	/**
	this = a * b
	should not be called with this as a or b.
	*/
	NX_INLINE void multiply(const NxQuat& a, const NxQuat& b);

	/**
	this = a * v
	v is interpreted as quat [xyz0]
	should not be called with this as a.
	*/
	NX_INLINE void multiply(const NxQuat& a, const NxVec3& v);

	/**
	this = slerp(t, a, b)
	*/
	NX_INLINE void slerp(const NxReal t, const NxQuat& a, const NxQuat& b);
	/**
	rotates passed vec by rot expressed by unit quaternion.  overwrites arg ith the result.
	*/
	NX_INLINE void rotate(NxVec3 &) const;
	/**
	rotates passed vec by opposite of rot expressed by unit quaternion.  overwrites arg ith the result.
	*/
	NX_INLINE void inverseRotate(NxVec3 &) const;

	/**
	negates all the elements of the quat.  q and -q represent the same rotation.
	*/
	NX_INLINE void negate();

	NX_INLINE NxQuat& operator*= (const NxQuat&);
	NX_INLINE NxQuat& operator+= (const NxQuat&);
	NX_INLINE NxQuat& operator-= (const NxQuat&);
	NX_INLINE NxQuat& operator*= (const NxReal);

    NxReal x,y,z,w;

    /* 
	ops we decided not to implement:
	bool  operator== (const NxQuat&) const;
	NxQuat  operator+  (const NxQuat& r_h_s) const;
	NxQuat  operator-  (const NxQuat& r_h_s) const;
	NxQuat  operator*  (const NxQuat& r_h_s) const;
	NxVec3  operator^  (const NxQuat& r_h_s) const;//same as normal quat rot, but casts itself into a vector.  (doesn't compute w term)
	NxQuat  operator*  (const NxVec3& v) const;//implicitly extends vector by a 0 w element.
	NxQuat  operator*  (const NxReal Scale) const;
	*/

	friend class NxMat33;
	};




NX_INLINE NxQuat::NxQuat()
	{
	//nothing
	}


NX_INLINE NxQuat::NxQuat(const NxQuat& q) : x(q.x), y(q.y), z(q.z), w(q.w)
	{
	}


NX_INLINE NxQuat::NxQuat(const NxVec3& v)						// copy constructor, assumes w=0 
	{
	x = v.x;
	y = v.y;
	z = v.z;
	w = NxReal(0.0);
	}


NX_INLINE NxQuat::NxQuat(const NxReal angle, const NxVec3 & axis)				// creates a NxQuat from an Angle axis -- note that if Angle > 360 the resulting rotation is Angle mod 360
	{
	fromAngleAxis(angle,axis);
	}


NX_INLINE void NxQuat::id()
	{
	x = NxReal(0);
	y = NxReal(0);
	z = NxReal(0);
	w = NxReal(1);
	}


NX_INLINE void NxQuat::setWXYZ(NxReal sw, NxReal sx, NxReal sy, NxReal sz)
	{
	x = sx;
	y = sy;
	z = sz;
	w = sw;
	}


NX_INLINE void NxQuat::setXYZW(NxReal sx, NxReal sy, NxReal sz, NxReal sw)
	{
	x = sx;
	y = sy;
	z = sz;
	w = sw;
	}


NX_INLINE void NxQuat::setWXYZ(const NxReal * d)
	{
	x = d[1];
	y = d[2];
	z = d[3];
	w = d[0];
	}


NX_INLINE void NxQuat::setXYZW(const NxReal * d)
	{
	x = d[0];
	y = d[1];
	z = d[2];
	w = d[3];
	}


NX_INLINE void NxQuat::getWXYZ(NxF32 *d) const
	{
	d[1] = (NxF32)x;
	d[2] = (NxF32)y;
	d[3] = (NxF32)z;
	d[0] = (NxF32)w;
	}


NX_INLINE void NxQuat::getXYZW(NxF32 *d) const
	{
	d[0] = (NxF32)x;
	d[1] = (NxF32)y;
	d[2] = (NxF32)z;
	d[3] = (NxF32)w;
	}


NX_INLINE void NxQuat::getWXYZ(NxF64 *d) const
	{
	d[1] = (NxF64)x;
	d[2] = (NxF64)y;
	d[3] = (NxF64)z;
	d[0] = (NxF64)w;
	}


NX_INLINE void NxQuat::getXYZW(NxF64 *d) const
	{
	d[0] = (NxF64)x;
	d[1] = (NxF64)y;
	d[2] = (NxF64)z;
	d[3] = (NxF64)w;
	}

//const methods
 
NX_INLINE bool NxQuat::isFinite() const
	{
	return NxMath::isFinite(x) 
		&& NxMath::isFinite(y) 
		&& NxMath::isFinite(z)
		&& NxMath::isFinite(w);
	}



NX_INLINE void NxQuat::zero()
	{
	x = NxReal(0.0);
	y = NxReal(0.0);
	z = NxReal(0.0);
	w = NxReal(1.0);
	}


NX_INLINE void NxQuat::negate()
	{
	x = -x;
	y = -y;
	z = -z;
	w = -w;
	}


NX_INLINE void NxQuat::random()
	{
	x = NxMath::rand(NxReal(0.0),NxReal(1.0));
	y = NxMath::rand(NxReal(0.0),NxReal(1.0));
	z = NxMath::rand(NxReal(0.0),NxReal(1.0));
	w = NxMath::rand(NxReal(0.0),NxReal(1.0));
	normalize();
	}


NX_INLINE void NxQuat::fromAngleAxis(NxReal Angle, const NxVec3 & axis)			// set the NxQuat by Angle-axis (see AA constructor)
	{
	x = axis.x;
	y = axis.y;
	z = axis.z;

	// required: Normalize the axis

	const NxReal i_length =  NxReal(1.0) / NxMath::sqrt( x*x + y*y + z*z );
	
	x = x * i_length;
	y = y * i_length;
	z = z * i_length;

	// now make a clQuaternionernion out of it
	NxReal Half = NxMath::degToRad(Angle * NxReal(0.5));

	w = NxMath::cos(Half);//this used to be w/o deg to rad.
	const NxReal sin_theta_over_two = NxMath::sin(Half );
	x = x * sin_theta_over_two;
	y = y * sin_theta_over_two;
	z = z * sin_theta_over_two;
	}


NX_INLINE void NxQuat::setx(const NxReal& d) 
	{ 
	x = d;
	}


NX_INLINE void NxQuat::sety(const NxReal& d) 
	{ 
	y = d;
	}


NX_INLINE void NxQuat::setz(const NxReal& d) 
	{ 
	z = d;
	}


NX_INLINE void NxQuat::setw(const NxReal& d) 
	{ 
	w = d;
	}


NX_INLINE void NxQuat::getAngleAxis(NxReal& angle, NxVec3 & axis) const
	{
	//return axis and angle of rotation of quaternion
    angle = NxMath::acos(w) * NxReal(2.0);		//this is getAngle()
    NxReal sa = NxMath::sqrt(NxReal(1.0) - w*w);
	if (sa)
		{
		axis.set(x/sa,y/sa,z/sa);
		angle = NxMath::radToDeg(angle);
		}
	else
		axis.zero();

	}

/**
gets the angle between this quat and the identity quaternion.
*/

NX_INLINE NxReal NxQuat::getAngle() const
	{
	return NxMath::acos(w) * NxReal(2.0);
	}

/**
gets the angle between this quat and the argument
*/

NX_INLINE NxReal NxQuat::getAngle(const NxQuat & q) const
	{
	return NxMath::acos(dot(q)) * NxReal(2.0);
	}


NX_INLINE NxReal NxQuat::magnitudeSquared() const

//modifyers:
	{
	return x*x + y*y + z*z + w*w;
	}


NX_INLINE NxReal NxQuat::dot(const NxQuat &v) const
	{
	return x * v.x + y * v.y + z * v.z  + w * v.w;
	}


NX_INLINE void NxQuat::normalize()											// convert this NxQuat to a unit clQuaternionernion
	{
	const NxReal mag = NxMath::sqrt(magnitudeSquared());
	if (mag)
		{
		const NxReal imag = NxReal(1.0) / mag;
		
		x *= imag;
		y *= imag;
		z *= imag;
		w *= imag;
		}
	}


NX_INLINE void NxQuat::conjugate()											// convert this NxQuat to a unit clQuaternionernion
	{
	x = -x;
	y = -y;
	z = -z;
	}


NX_INLINE void NxQuat::multiply(const NxQuat& left, const NxQuat& right)		// this = a * b
	{
	w =left.w*right.w - left.x*right.x - left.y*right.y - left.z*right.z;
	x =left.w*right.x + right.w*left.x + left.y*right.z - right.y*left.z;
	y =left.w*right.y + right.w*left.y + left.z*right.x - right.z*left.x;
	z =left.w*right.z + right.w*left.z + left.x*right.y - right.x*left.y;
	}


NX_INLINE void NxQuat::multiply(const NxQuat& left, const NxVec3& right)		// this = a * b
	{
	w = - left.x*right.x - left.y*right.y - left.z *right.z;
	x =   left.w*right.x + left.y*right.z - right.y*left.z;
	y =   left.w*right.y + left.z*right.x - right.z*left.x;
	z =   left.w*right.z + left.x*right.y - right.x*left.y;
	}


NX_INLINE void NxQuat::slerp(const NxReal t, const NxQuat& left, const NxQuat& right) // this = slerp(t, a, b)
	{
	NxReal cosine = 
		left.x * right.x + 
		left.y * right.y + 
		left.z * right.z + 
		left.w * right.w;		//this is left.dot(right)

	NxQuat wrappedRight;

	NxReal sign = NxReal(1);
	if (cosine < 0)
		{
		cosine = - cosine;
		sign = NxReal(-1);
		}

	NxReal lower_weight=0;
	NxReal upper_weight=0;
#ifdef OLD_ONE
	const NxReal angle = NxMath::acos(cosine); // half angle is in radians.. //this is left.getAngle(right)/2
	
	
	// set up our weights
	if(NxMath::equals(angle, NxReal(0), NxReal(0.0005)))
		{
		// numerically unstable when angle is close to 0, lerp instead
		lower_weight = t;
		upper_weight = NxReal(1) - t;
		}
	else
		{
		// this will also fall apart if w approaches k*pi/2 for k = [1, 2, ...]
		const NxReal i_sin_angle = NxReal(1) / NxMath::sin(angle);


#else


		NxReal Sin = NxReal(1) - cosine*cosine;
		
#define QUAT_EPSILON	(NxReal(1.0e-8f))
		if(Sin>=QUAT_EPSILON*QUAT_EPSILON)	
			{
			Sin = NxMath::sqrt(Sin);
			const NxReal angle = NxMath::atan2(Sin, cosine);
			const NxReal i_sin_angle = NxReal(1) / Sin;



#endif
		lower_weight = NxMath::sin(angle*(NxReal(1)-t)) * i_sin_angle;
		upper_weight = NxMath::sin(angle * t) * i_sin_angle * sign;
		}
	w = (left.w * (lower_weight)) + (right.w * (upper_weight));
	x = (left.x * (lower_weight)) + (right.x * (upper_weight));
	y = (left.y * (lower_weight)) + (right.y * (upper_weight));
	z = (left.z * (lower_weight)) + (right.z * (upper_weight));
	}


NX_INLINE void NxQuat::rotate(NxVec3 & v) const						//rotates passed vec by rot expressed by quaternion.  overwrites arg ith the result.
	{
	//NxReal msq = NxReal(1.0)/magnitudeSquared();	//assume unit quat!
	NxQuat myInverse;
	myInverse.x = -x;//*msq;
	myInverse.y = -y;//*msq;
	myInverse.z = -z;//*msq;
	myInverse.w =  w;//*msq;

	//v = ((*this) * v) ^ myInverse;

	NxQuat left;
	left.multiply(*this,v);
	v.x =left.w*myInverse.x + myInverse.w*left.x + left.y*myInverse.z - myInverse.y*left.z;
	v.y =left.w*myInverse.y + myInverse.w*left.y + left.z*myInverse.x - myInverse.z*left.x;
	v.z =left.w*myInverse.z + myInverse.w*left.z + left.x*myInverse.y - myInverse.x*left.y;
	}


NX_INLINE void NxQuat::inverseRotate(NxVec3 & v) const				//rotates passed vec by opposite of rot expressed by quaternion.  overwrites arg ith the result.
	{
	//NxReal msq = NxReal(1.0)/magnitudeSquared();	//assume unit quat!
	NxQuat myInverse;
	myInverse.x = -x;//*msq;
	myInverse.y = -y;//*msq;
	myInverse.z = -z;//*msq;
	myInverse.w =  w;//*msq;

	//v = (myInverse * v) ^ (*this);
	NxQuat left;
	left.multiply(myInverse,v);
	v.x =left.w*x + w*left.x + left.y*z - y*left.z;
	v.y =left.w*y + w*left.y + left.z*x - z*left.x;
	v.z =left.w*z + w*left.z + left.x*y - x*left.y;
	}


NX_INLINE NxQuat& NxQuat::operator=  (const NxQuat& q)
	{
	x = q.x;
	y = q.y;
	z = q.z;
	w = q.w;
	return *this;
	}

#if 0
NX_INLINE NxQuat& NxQuat::operator=  (const NxVec3& v)		//implicitly extends vector by a 0 w element.
	{
	x = v.x;
	y = v.y;
	z = v.z;
	w = NxReal(1.0);
	return *this;
	}
#endif

NX_INLINE NxQuat& NxQuat::operator*= (const NxQuat& q)
	{
	NxReal xx[4]; //working Quaternion
	xx[0] = w*q.w - q.x*x - y*q.y - q.z*z;
	xx[1] = w*q.x + q.w*x + y*q.z - q.y*z;
	xx[2] = w*q.y + q.w*y + z*q.x - q.z*x;
	z=w*q.z + q.w*z + x*q.y - q.x*y;

	w = xx[0];
	x = xx[1];
	y = xx[2];
	return *this;
	}


NX_INLINE NxQuat& NxQuat::operator+= (const NxQuat& q)
	{
	x+=q.x;
	y+=q.y;
	z+=q.z;
	w+=q.w;
	return *this;
	}


NX_INLINE NxQuat& NxQuat::operator-= (const NxQuat& q)
	{
	x-=q.x;
	y-=q.y;
	z-=q.z;
	w-=q.w;
	return *this;
	}


NX_INLINE NxQuat& NxQuat::operator*= (const NxReal s)
	{
	x*=s;
	y*=s;
	z*=s;
	w*=s;
	return *this;
	}
#endif
