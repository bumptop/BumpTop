#ifndef __3DMATH_H__
#define __3DMATH_H__
/*-------------------------------------------------------------*\
|					Sun Commander source file					|
|																|
|																|
|																|
| Copyright (C) 1999 Adam Moravanszky							|
|																|
| This program is free software; you can redistribute it and/or |
| modify it under the terms of the GNU General Public License   |
| Version 2 as published by the Free Software Foundation.       |
|                                                               |
\*-------------------------------------------------------------*/
/*-------------------------------*\
| Vector, Matrix, and Quaternion classes
|
| Vec3, Quat based on code by Robert Stephen Rodgers
| uses 'flo' as base type
\*-------------------------------*/
/*-Revision----------------------\
| At: 11/10/99 4:52:08 PM
| Added Mat33::Orthogonalize() 
| uses GramSchmidt algorithm, based 
| on Maple code `Copyright (c) 1997 by Erich Kaltofen`
\-------------------------------*/
/*-Revision----------------------\
| At: 11/15/99 8:55:00 PM
| Changed Orthogonalize to 
| Orthonormalize.
\-------------------------------*/
/*-Revision----------------------\
| At: 11/15/99 8:59:32 PM
| Changed all methods which took 
| pointer params to take reference
| params.  Oh dear.
\-------------------------------*/
/*-Revision----------------------\
| At: 4/2/00 5:35:15 PM
| Added assignment to/from
| float array.
|
\-------------------------------*/

#include <math.h>
#include <Smart.h>
#include <stdlib.h>

#define M3D_EPSILON		0.0005f

inline flo frand()//between 0 and 1
	{
	return (flo)rand()/((flo)RAND_MAX+1);
	}

class clVector3D
{

		bool epsilonEquals(const flo left, const flo right, const flo epsilon) const;
	public: 
			flo x,y,z;

		clVector3D() {}
		clVector3D(const flo x, const flo y, const flo z);
		clVector3D(const clVector3D& v);

		flo  GetElem(int i);						//get/set element at index i=0..2
		void SetElem(int i, flo elem);
		void Get(flo &);
		inline float & operator[](unsigned index) const;


		clVector3D& operator=(const clVector3D& v);
		clVector3D& operator=(const flo & v);
		bool operator==(const clVector3D& v) const;

		clVector3D operator*(const flo f) const;			// scalar multiple of this vector
		clVector3D& operator*=(const flo f);				
		clVector3D operator+(const clVector3D& v) const;		// sum of the two vectors
		clVector3D& operator+=(const clVector3D& v);
		clVector3D operator-(const clVector3D& v) const;		// signed difference of the two vectors
		clVector3D& operator-=(const clVector3D& v);
		void Add(const clVector3D * v);						//this += v;
		bool isZero() const;
		void Zero();
		void Set(const flo x, const flo y, const flo z);
		
		clVector3D Cross(const clVector3D& v) const;	// returns = this Cross v
		void Cross(const clVector3D& left, const clVector3D& right); // this = left Cross right
		flo Dot(const clVector3D& v) const;		// Dot = this . v
		flo Magnitude() const;				// Magnitude of this vector
		flo MagnitudeSq() const;				// Magnitude squared of this vector
		flo Angle(const clVector3D& v) const;		// Angle between this & v
		void Normalize();					// makes this vector a unit vector


};

//short names
typedef clVector3D		Vec3;

class clMatrix3x3; 
typedef clMatrix3x3		Mat33;

class clQuaternion  
{
	public:
		static flo deg_to_rad(const flo src);  // convert degrees to radians
		static flo rad_to_deg(const flo src);  // convert radians to degrees
		static flo dot4(const flo v1[4], const flo v2[4]); // 4-vector Dot product
		static bool epsilonEquals(const flo leflo, const flo right, const flo epsilon);


		flo w,x,y,z;

		// various ways of creating a new clQuaternionernion
		////////////////////////////////////////////////
		clQuaternion() {}										// default constructor -- contents UNDEFINED
		clQuaternion(const clQuaternion& s);					// copy constructor
		clQuaternion(const clVector3D& v);						// copy constructor, assumes w=0 
		clQuaternion(const flo Angle, Vec3 & axis)				// creates a clQuaternion from an Angle axis -- note that if Angle > 360 the resulting rotation is Angle mod 360
			{
			fromAngleAxis(Angle, axis);
			}
		// clQuaternion functions
		////////////////////////////////////////////////
		void Set(const flo w,const  flo x,const  flo y,const  flo z);
		void Zero();
		void fromAngleAxis(const flo Angle, Vec3 & axis);			// set the clQuaternion by Angle-axis (see AA constructor)
		void fromEulerAngles(flo roll, flo pitch, flo yaw);
		void clQuaternion::fromMatrix(Mat33 &m);					// set the clQuaternion from a rotation matrix
		void Normalize();											// convert this clQuaternion to a unit clQuaternionernion
		void fromclQuaternion(const clQuaternion& q);				// set the clQuaternion to another clQuaternion
		void fromclQuaterniondata(const flo clQuaternion_wxyz[4]);	// directly set the clQuaternion data -- a normalization is performed
		void getAngleAxis(flo& Angle, flo axis[3]) const;			// fetches the Angle/axis given by the clQuaternion
		void multiply(const clQuaternion& q);						// this = this * q		
		void multiply(const clQuaternion& leflo, const clQuaternion& right);		// this = leflo * right
		void slerp(const flo t, const clQuaternion& leflo, const clQuaternion& right); // this = slerp(t, leflo, right)
		bool sameAs(const clQuaternion& q) const;
		void Rotate(clVector3D & cloVector);						//rotates passed vec by rot expressed by quaternion.  overwrites arg ith the result.
		void InverseRotate(clVector3D & cloVector);				//rotates passed vec by opposite of rot expressed by quaternion.  overwrites arg ith the result.
		flo Magnitude();											//this is actually magnitude squared.

		// semantic sugar -- these wrap the above functions
		///////////////////////////////////////////////////
		clQuaternion& operator*= (const clQuaternion& r_h_s);
		clQuaternion& operator+= (const clQuaternion& r_h_s);
		clQuaternion& operator-= (const clQuaternion& r_h_s);
		clQuaternion  operator+  (const clQuaternion& r_h_s) const;
		clQuaternion  operator-  (const clQuaternion& r_h_s) const;

		clQuaternion& operator*= (const flo Scale);
		clQuaternion  operator*  (const clQuaternion& r_h_s) const;
		clVector3D    operator^  (const clQuaternion& r_h_s) const;//same as normal quat rot, but casts itself into a vector.  (doesn't compute w term)
		clQuaternion  operator*  (const clVector3D& v) const;//implicitly extends vector by a 0 w element.
		clQuaternion  operator*  (const flo Scale) const;
		bool  operator== (const clQuaternion& r_h_s) const;
		clQuaternion& operator=  (const clQuaternion& r_h_s);
		clQuaternion& operator=  (const clVector3D& v);		//implicitly extends vector by a 0 w element.
		void Random()
			{
			x = 2.0f*frand() - 1;
			y = 2.0f*frand() - 1;
			z = 2.0f*frand() - 1;
			w = 2.0f*frand() - 1;
			Normalize();
			}

		

};


class clMatrix4x4
	{
	public:
	union
		{
		flo M44[4][4];						//column major order [x-column][y-row]
		flo M16[16];						//[x = 2][y = 1] == [9]
		};
											/*
											 0 4 8 12
											 1 5 9 13 
											 2 6 0 14
											 3 7 1 15
											*/

	clMatrix4x4();							//init to identity
	clMatrix4x4(clQuaternion & Quat);			//create from quat
	void ID();								//init to identity

	void FromQuat(clQuaternion & Quat);		//sets itself to rotmatrix based on quat passed
	void Set(clMatrix4x4 & clopMatrix);		//This = Matrix
	void Set(flo tx, flo ty, flo tz, flo roty, flo rotz, flo rotx,clMatrix4x4 * inverse = 0);	//sets matrix to product: rot_x(rotx)*rot_z(rotz)*rot_y(roty)*translate(txyz).  angles in radians.
																								//the computation of the result's inverse is inexpensive, and will be provided in the last arg if its not zero.


	void SetProjection(float tan_half_fovHorizontal, float tan_half_fovVertical, float zNear, float zFar ); //args are to be tan(?/2)
	void SetTranslation(const clVector3D & clpVector);	//sets 3 elems responsible for translation
	void SetTranslationNegative(const clVector3D & cloVector);	//same as above, but sets vars to -clopVector.
	void Copy3x3(clMatrix4x4 & cloMatrix);			//copies the 3x3 part of the passed matrix to the 3x3 part of this matrix. (sets a rotation)
	void Copy3x3Transposed(clMatrix4x4 & cloMatrix);//same as above, but transposes 3x3 part as it copies.  (sets the inverse of a rotation)
	void Copy3x3Transposed(clMatrix3x3 & cloMatrix);

	void TranslateNegative(clVector3D & cloVector);//this = this * TranslationMx(-clopVector);
													//assumes all but 3x3 part is 0, except lower right corner = 1
	clVector3D Rotate(const clVector3D & cloVector) const;		//uses only 3x3 part of matrix to transform a vector.
	clVector3D RotateTransposed(const clVector3D & cloVector) const;		//uses only transposed 3x3 part of matrix to transform a vector.

	void Invert3x4(bool scaling = true); 				//set scaling to false if this matrix is a product T*R, else it is a T*R*S.  Scaling must be uniform!!

	/* not yet implemented:
	void Scale(clVector3D * clopVector);	//same as multiplying with corresp scaling matrix.  overwrites itself with the result
	void Mult(clMatrix4x4 * clopMatrix);	//This = This * Matrix
	void Add(clMatrix4x4 * clopMatrix);		//overwrites itself with the result
	void Subtract(clMatrix4x4 * clopMatrix);//overwrites itself with the result
	clMatrix4x4 * Copy(void);				//creates a copy obj, returns ptr to it, caller gets to own it.
	*/										//other possibilities:  Norm, Determinant, Invert.
											//make special cases for mult which assume 0001 for last row.
	clMatrix4x4  operator*  (const clMatrix4x4& r_h_s) const;
	clVector3D   operator*  (const clVector3D & r_h_s) const;	//assumes 4th rows are identity.
	clVector3D   multHDiv   (const clVector3D & r,bool onlyDivXY = false) const;
	};



class clMatrix3x3
	{
	public:
	union
		{
		flo M33[3][3];						//column major order [x-column][y-row]
		flo M9[9];							//[x = 2][y = 1] == [7]
		};

	clMatrix3x3();							//init to identity
	void ID();								//init to identity
	void FromQuat(clQuaternion & Quat);		//sets itself to rotmatrix based on quat passed
	void Orthonormalize();					//Gram-Schmidt orthogonalization to correct numerical drift, plus column normalization
	void GetColumn(int i,Vec3 & col);		//get/set column vector at i = 0..2
	void SetColumn(int i,Vec3 & col);

	clMatrix3x3& operator=  (const clMatrix3x3& r_h_s);
	clMatrix3x3& operator=  (const flo * f9);
	clMatrix3x3  operator*  (const clMatrix3x3& r_h_s) const;
	clVector3D   operator*  (const clVector3D & r_h_s) const;
	clMatrix3x3  operator%  (const clMatrix3x3& r_h_s) const;	// multiplies by transpose of r_h_s

	void vecTransMult(const clVector3D & r_h_s, clVector3D & result) const;	// performs dest = rhs^t * this
	};

//short names
typedef clMatrix4x4		Mat44;

typedef clMatrix4x4		Mat4;
typedef clMatrix3x3		Mat3;

typedef clQuaternion	Quat;

/*
use example:

	clVec3	Position;
	clMat44 Orientation;
	clMat44 Model2World;


	SetPosition(Vec3 * p)
		{
		Position = *p;	//copy data

		Model2World.Set(Orientation);
		Model2World.SetTranslation(Position);
		}

	Translate(Vec3 * dp)
		{
		Position.Add(p);	//copy data

		Model2World.Set(Orientation);
		Model2World.SetTranslation(Position);
		}

	SetOrientation(Quat * q)
		{
		Orientation.FromQuat(q);
		Model2World.Copy3x3(Orientation);
		}

*/



  
inline clVector3D ::clVector3D(const flo x, const flo y, const flo z)
{
Set(x,y,z);
}

inline void clVector3D ::Set(const flo x, const flo y, const flo z)
	{
	this->x = x;
	this->y = y;
	this->z = z; 
	}

inline void Vec3::Get(flo &f)
	{
	f = x;
	(&f)[1] = y;
	(&f)[2] = z;
	}

inline flo  Vec3::GetElem(int i)
	{
	switch(i) 
		{
		case 0:
			return x;
		case 1:
			return y;
		default:
			return z;
		};
	}
inline void Vec3::SetElem(int i, flo elem)
	{
	switch(i) 
		{
		case 0:
			x=elem; break;
		case 1:
			y=elem; break;
		default:
			z=elem;
		};
	}


inline float & Vec3::operator[](unsigned index) const
	{ 
	return ((float *)&x)[index];
	}

  
inline clVector3D::clVector3D(const clVector3D& v)
{
	x = v.x;
	y = v.y;
	z = v.z; 
}


  
inline clVector3D & clVector3D ::operator=(const clVector3D& v)
{
	x = v.x;
	y = v.y;
	z = v.z;
	return *this;
}

inline clVector3D& clVector3D::operator=(const flo & v)
	{
	x = v;
	y = (&v)[1];
	z = (&v)[2];
	return *this;
	}

  
inline bool clVector3D ::operator==(const clVector3D& v) const
{
	return 
			epsilonEquals(x, v.x, (flo) M3D_EPSILON) &&
			epsilonEquals(y, v.y, (flo) M3D_EPSILON) &&
			epsilonEquals(z, v.z, (flo) M3D_EPSILON);
}


  
inline bool clVector3D ::epsilonEquals(const flo left, const flo right, const flo epsilon) const
{
	const flo diff = (flo)fabs(left - right);
	if(diff < epsilon) return true;
	return false;
}



  
inline clVector3D  clVector3D ::operator*(const flo f) const 
{ 
	clVector3D t(x * f, y * f, z * f); 
	return t; 
}




  
inline clVector3D & clVector3D ::operator*=(const flo f)
{
	x *= f; y *= f;  z *= f; 
	return *this;
}




  
inline clVector3D  clVector3D ::operator+(const clVector3D& v) const 
{ 
	clVector3D t(x + v.x,
			y + v.y,
			z + v.z);  
	return t;
}



  
inline void  clVector3D ::Add(const clVector3D * v) 
{ 
	x += v->x; 
	y += v->y; 
	z += v->z; 
}



  
inline clVector3D  clVector3D ::operator-(const clVector3D& v) const 
{ 
	clVector3D t(x - v.x,y - v.y,z - v.z);  
	return t;
}



  
inline clVector3D &  clVector3D ::operator-=(const clVector3D& v) 
	{ 
	x -= v.x; 
	y -= v.y; 
	z -= v.z; 
	return *this; 
	}

inline clVector3D  & clVector3D ::operator+=(const clVector3D& v) 
	{ 
	x += v.x; 
	y += v.y; 
	z += v.z; 
	return *this; 
	}



inline bool clVector3D ::isZero() const
	{
	return 
		epsilonEquals(x, 0, (flo) M3D_EPSILON) &&
		epsilonEquals(y, 0, (flo) M3D_EPSILON) &&
		epsilonEquals(z, 0, (flo) M3D_EPSILON);
	}

inline void clVector3D ::Zero()
	{
	x = y = z = 0.0f;
	}




inline clVector3D  clVector3D ::Cross(const clVector3D& v) const
	{
	clVector3D t(
		(y * v.z) - (z * v.y),
		(z * v.x) - (x * v.z),
		(x * v.y) - (y * v.x)
		);
	return t;
	}




inline void clVector3D ::Cross(const clVector3D& left, const clVector3D& right)
	{
	assert(&left != this && &right != this);//can't do this in place!
	x = (left.y * right.z) - (left.z * right.y);
	y = (left.z * right.x) - (left.x * right.z);
	z = (left.x * right.y) - (left.y * right.x);
	}




inline flo clVector3D ::Dot(const clVector3D& v) const
	{
	return x*v.x + y*v.y + z*v.z; 
	}





inline flo clVector3D ::Magnitude() const
	{
	return (flo)sqrt(Dot(*this));
	}

inline flo clVector3D ::MagnitudeSq() const
	{
	return Dot(*this);
	}



inline flo clVector3D ::Angle(const clVector3D& v) const
	{
	const flo divis = Magnitude() * v.Magnitude();
	return (flo)acos( Dot(v) / divis); 
	}



  
inline void clVector3D ::Normalize()
{
	flo m = Magnitude(); 
	if (m)
		{
		const flo il = (flo) 1.0 / m;
		x *= il;
		y *= il;
		z *= il;
		}
}

inline void clQuaternion ::Set(const flo w, const flo x, const flo y, const flo z)
	{
	this->w = w;
	this->x = x;
	this->y = y;
	this->z = z; 
	}

inline clQuaternion::clQuaternion(const clQuaternion& s)
{
	w = s.w;
	x = s.x;
	y = s.y;
	z = s.z;
}

inline clQuaternion::clQuaternion(const clVector3D& v)
{
	w = 0.0f;
	x = v.x;
	y = v.y;
	z = v.z;
}

inline void clQuaternion::Zero()
	{
	w = 1.0f;
	x = y = z = 0.0f;
	}

inline void clQuaternion::fromclQuaterniondata(const flo q[4])
{
	w = q[0];
	x = q[1];
	y = q[2];
	z = q[3];
	Normalize();
}




inline void clQuaternion::fromclQuaternion(const clQuaternion& q)
{
	w = q.w;
	x = q.x;
	y = q.y;
	z = q.z;
}




inline flo clQuaternion::deg_to_rad(const flo src)  
{
	return (flo)0.01745329251994329547 * src;
}




inline flo clQuaternion::rad_to_deg(const flo src) 
{
	return (flo)57.29577951308232286465 * src;
}



inline void clQuaternion::Normalize()
	{
	// a clQuaternionernion can be converted to a unit clQuaternionernion by 
	// dividing each sub element by the clQuaternion's Magnitude 
	// (unit clQuaternions satisfy w^2 + x^2 + y^2 + z^2 = 1) 

/*	const flo wsqd = w * w;
	const flo xsqd = x * x;
	const flo ysqd = y * y;
	const flo zsqd = z * z;

   wsqd + xsqd + ysqd + zsqd 
*/	const flo mag = (flo) sqrt(Magnitude());
	if (mag)
		{
		const flo i_magnitude = (flo) 1.0 / mag;
		
		w = w * i_magnitude;
		x = x * i_magnitude;
		y = y * i_magnitude;
		z = z * i_magnitude;
		}
	}




inline void clQuaternion::multiply(const clQuaternion& q)
	{
	flo T[4]; //working clQuaternionernion
	T[0] =w*q.w - q.x*x - y*q.y - q.z*z;
	T[1] =w*q.x + q.w*x + y*q.z - q.y*z;
	T[2] =w*q.y + q.w*y + z*q.x - q.z*x;
	z =w*q.z + q.w*z + x*q.y - q.x*y;

	w = T[0];
	x = T[1];
	y = T[2];
	}

inline flo clQuaternion::Magnitude()
	{
	return x*x + y*y + z*z + w*w;
	}


inline void clQuaternion::multiply(const clQuaternion& leflo, const clQuaternion& right)
	{
	w =leflo.w*right.w - leflo.x*right.x - leflo.y*right.y - leflo.z*right.z;
	x =leflo.w*right.x + right.w*leflo.x + leflo.y*right.z - right.y*leflo.z;
	y =leflo.w*right.y + right.w*leflo.y + leflo.z*right.x - right.z*leflo.x;
	z =leflo.w*right.z + right.w*leflo.z + leflo.x*right.y - right.x*leflo.y;
	}

inline void clQuaternion::Rotate(clVector3D & cloVector)	//rotates passed vec by rot expressed by quaternion.  overwrites arg ith the result.
	{
	flo msq = 1.0f/Magnitude();
	clQuaternion MyInverse;
	MyInverse.w = w*msq;
	MyInverse.x = -x*msq;
	MyInverse.y = -y*msq;
	MyInverse.z = -z*msq;

	cloVector = (*this * cloVector) ^ MyInverse;
	
	}

inline void clQuaternion::InverseRotate(clVector3D & cloVector)	
	//rotates passed vec by rot opposite to that expressed by quaternion.  overwrites arg ith the result.
	{
	flo msq = 1.0f/Magnitude();
	clQuaternion MyInverse;
	MyInverse.w = w*msq;
	MyInverse.x = -x*msq;
	MyInverse.y = -y*msq;
	MyInverse.z = -z*msq;

	cloVector = (MyInverse * cloVector) ^ (*this);
	}


inline clQuaternion clQuaternion::operator*(const clVector3D & Vec) const
	{
	clQuaternion a;
	a.w =this->w*0.0f - this->x*Vec.x - this->y*Vec.y - this->z*Vec.z;
	a.x =this->w*Vec.x + 0.0f*this->x + this->y*Vec.z - Vec.y*this->z;
	a.y =this->w*Vec.y + 0.0f*this->y + this->z*Vec.x - Vec.z*this->x;
	a.z =this->w*Vec.z + 0.0f*this->z + this->x*Vec.y - Vec.x*this->y;
	return a;
	}

inline void clQuaternion::getAngleAxis(flo& Angle, flo axis[3]) const
{
/*	Angle = (flo) acos((flo)w) * 2.0f;
	const flo i_sin_over = 1.0f / (flo)sin((flo)Angle/2.0f);
	axis[0] = x * i_sin_over;
	axis[1] = y * i_sin_over;
	axis[2] = z * i_sin_over;
	Angle = (flo) rad_to_deg(Angle);
*/



	//return axis and angle of rotation of quaternion
    Angle = (flo)acos(w) * 2.0f;
    flo sa = (flo)sqrt(1.0f - w*w);
	if (sa)
		{
		axis[0]=x/sa; axis[1]=y/sa; axis[2]=z/sa;
		Angle = (flo) rad_to_deg(Angle);
		}
	else
		axis[0]=axis[1]=axis[2]=0.0f;
	}




inline void clQuaternion::fromAngleAxis(const flo Angle, Vec3 & axis)
{
	x = axis.x;
	y = axis.y;
	z = axis.z;

	// required: Normalize the axis

	const flo i_length = (flo) 1.0f / (flo) sqrt( x*x + y*y + z*z );
	
	x = x * i_length;
	y = y * i_length;
	z = z * i_length;

	// now make a clQuaternionernion out of it
	flo Half = deg_to_rad((flo)Angle / 2.0f);

	w = (flo) cos(Half);//this used to be w/o deg to rad.
	const flo sin_theta_over_two = (flo) sin( Half );
	x = x * sin_theta_over_two;
	y = y * sin_theta_over_two;
	z = z * sin_theta_over_two;
}

inline void clQuaternion::fromEulerAngles(flo roll, flo pitch, flo yaw)
//(double heading, double attitude, double bank) 
	{    
	// Assuming the angles are in radians.    
	float c1 = cosf(pitch/2);	//around x axis
	float s1 = sinf(pitch/2);    
	float c2 = cosf(yaw/2);		//around y axis
	float s2 = sinf(yaw/2);    
	float c3 = cosf(roll/2);    
	float s3 = sinf(roll/2);	//around z axis



	x = c3 * c2 * s1 - s3 * s2 * c1 ;

	y = c3 * s2 * c1 + s3 * c2 * s1 ;

	z = s3 * c2 * c1 - c3 * s2 * s1 ;

	w = c3 * c2 * c1 + s3 * s2 * s1 ;








/*

	float c1 = cosf(roll);    
	float s1 = sinf(roll);    
	float c2 = cosf(pitch);    
	float s2 = sinf(pitch);    
	float c3 = cosf(yaw);    
	float s3 = sinf(yaw);

    w = sqrtf(1.0f + c1 * c2 + c1*c3 + s1 * s2 * s3 + c2*c3) / 2.0f;
	float w4 = (4.0f * w);
	x = (c2 * s3 + c1 * s3 -s1 * s2 * c3) / w4 ;    
	y = (s1 * s3 + c1 * s2 * c3 +s2) / w4 ;    
	z = (s1 * c2 + s1 * c3 - c1 * s2 * s3) / w4 ;
	*/
  }


inline void clQuaternion::fromMatrix(Mat33 &m)
    {
    flo tr, s;
    tr = m.M33[0][0] + m.M33[1][1] + m.M33[2][2];
    if(tr >= 0)
		{
		s = (flo)sqrt(tr +1);
		w = 0.5f * s;
		s = 0.5f / s;
		x = (m.M33[1][2] - m.M33[2][1]) * s;
		y = (m.M33[2][0] - m.M33[0][2]) * s;
		z = (m.M33[0][1] - m.M33[1][0]) * s;
		}
    else
		{
		int i = 0; 
		if (m.M33[1][1] > m.M33[0][0])
			i = 1; 
		if(m.M33[2][2] > m.M33[i][i])
			i=2; 
		switch (i)
			{
			case 0:
				s   = (flo)sqrt((m.M33[0][0] - (m.M33[1][1] + m.M33[2][2])) + 1);
				x = 0.5f * s;
				s   = 0.5f / s;
				y = (m.M33[1][0] + m.M33[0][1]) * s; 
				z = (m.M33[0][2] + m.M33[2][0]) * s;
				w = (m.M33[1][2] - m.M33[2][1]) * s;
				break;
			case 1:
				s   = (flo)sqrt((m.M33[1][1] - (m.M33[2][2] + m.M33[0][0])) + 1);
				y = 0.5f * s;
				s   = 0.5f / s;
				z = (m.M33[2][1] + m.M33[1][2]) * s;
				x = (m.M33[1][0] + m.M33[0][1]) * s;
				w = (m.M33[2][0] - m.M33[0][2]) * s;
				break;
			case 2:
				s   = (flo)sqrt((m.M33[2][2] - (m.M33[0][0] + m.M33[1][1])) + 1);
				z = 0.5f * s;
				s   = 0.5f / s;
				x = (m.M33[0][2] + m.M33[2][0]) * s;
				y = (m.M33[2][1] + m.M33[1][2]) * s;
				w = (m.M33[0][1] - m.M33[1][0]) * s;
			}
		}
	}



inline void clQuaternion::slerp(const flo t, const clQuaternion& leflo, const clQuaternion& right)
{
	// the slerp of a pair of unit clQuaternionerions is the weighted
	// interpolation between them, where the interpolation weight is
	// given by t = [0, 1.0]
	//
	// the trick to slerping is that we find the Angle between the two
	// clQuaternions by treating them as a pair of four vectors and getting the
	// cosine [as the Dot product].  
	//
	// then the slerp between two clQuaternionernions A and B is:
	//
	//       A * (upper_weight) + B * (lower_weight) 
	//		
	//	where the weights are the sines of the t-weighted Angle
	//	divided by the sine of the Angle. 
	//
	// the resulting clQuaternionernion is also a unit clQuaternionernion.


	// find the Angle between the two clQuaternions by treating 
	// them as 4-length vectors -- V1.V2 = cos(theta) 
	const flo cosine = dot4(&leflo.w, &right.w); 
	const flo Angle = (flo)acos((flo)cosine); // Angle is in radians.. 

	flo lower_weight;
	flo upper_weight;

	// set up our weights
	if(epsilonEquals(Angle, 0.0f, M3D_EPSILON))
	{
		// numerically unstable when Angle is close to 0, lerp instead
		lower_weight = t;
		upper_weight = 1.0f - t;
	}
	else
	{
		// this will also fall apart if w approaches k*pi/2 for k = [1, 2, ...]
		const flo i_sin_angle = 1.0f / (flo)sin(Angle);
		lower_weight = (flo)sin(Angle - Angle*t) * i_sin_angle;
		upper_weight = (flo)sin(Angle * t) * i_sin_angle;
	}

	w = (leflo.w * (lower_weight)) + (right.w * (upper_weight));
	x = (leflo.x * (lower_weight)) + (right.x * (upper_weight));
	y = (leflo.y * (lower_weight)) + (right.y * (upper_weight));
	z = (leflo.z * (lower_weight)) + (right.z * (upper_weight));
}




inline flo clQuaternion::dot4(const flo v1[4], const flo v2[4]) 
{
	return 
		  (v1[0] * v2[0]) 
		+ (v1[1] * v2[1]) 
		+ (v1[2] * v2[2]) 
		+ (v1[3] * v2[3]);
}



inline bool clQuaternion::epsilonEquals(const flo leflo, const flo right, const flo epsilon) 
{
	const flo diff = (flo)fabs(leflo - right);
	if(diff < epsilon) return true;
	return false;
}




inline clQuaternion& clQuaternion::operator*=(const clQuaternion& r_h_s)
	{
	multiply(r_h_s);
	return *this;
	}

inline clQuaternion& clQuaternion::operator*= (const flo S)
	{
	x*=S;
	y*=S;
	z*=S;
	w*=S;
	return *this;
	}

inline clQuaternion& clQuaternion::operator+= (const clQuaternion& r_h_s)
	{
	x+=r_h_s.x;
	y+=r_h_s.y;
	z+=r_h_s.z;
	w+=r_h_s.w;
	return *this;
	}
inline clQuaternion& clQuaternion::operator-= (const clQuaternion& r_h_s)
	{
	x-=r_h_s.x;
	y-=r_h_s.y;
	z-=r_h_s.z;
	w-=r_h_s.w;
	return *this;
	}

inline clQuaternion clQuaternion::operator+(const clQuaternion& r_h_s) const
	{
	clQuaternion a;
	a.x=this->x+r_h_s.x;
	a.y=this->y+r_h_s.y;
	a.z=this->z+r_h_s.z;
	a.w=this->w+r_h_s.w;
	return a;
	}
inline clQuaternion clQuaternion::operator-(const clQuaternion& r_h_s) const
	{
	clQuaternion a;
	a.x=this->x-r_h_s.x;
	a.y=this->y-r_h_s.y;
	a.z=this->z-r_h_s.z;
	a.w=this->w-r_h_s.w;
	return a;
	}

inline clQuaternion clQuaternion::operator*(const flo S) const
	{
	clQuaternion a;
	a.x=this->x*S;
	a.y=this->y*S;
	a.z=this->z*S;
	a.w=this->w*S;
	return a;
	}


inline clQuaternion clQuaternion::operator*(const clQuaternion& r_h_s) const
	{
	clQuaternion a;
	a.multiply(*this, r_h_s);
	return a;
	}

inline clVector3D clQuaternion::operator^(const clQuaternion& right) const
	{
	clVector3D a;
	a.x =this->w*right.x + right.w*this->x + this->y*right.z - right.y*this->z;
	a.y =this->w*right.y + right.w*this->y + this->z*right.x - right.z*this->x;
	a.z =this->w*right.z + right.w*this->z + this->x*right.y - right.x*this->y;
	return a;
	}



inline bool clQuaternion::operator==(const clQuaternion& r_h_s) const
	{
	return sameAs(r_h_s);
	}




inline clQuaternion& clQuaternion::operator=(const clQuaternion& r_h_s)
	{
	fromclQuaternion(r_h_s);
	return *this;
	}

inline clQuaternion& clQuaternion::operator=(const clVector3D& v)
	{
	w = 0.0f;
	x = v.x;
	y = v.y;
	z = v.z;
	return *this;
	}



inline bool clQuaternion::sameAs(const clQuaternion& r_h_s) const
{
	return	(w == r_h_s.w) && 
			(x == r_h_s.x) && 
			(y == r_h_s.y) && 
			(z == r_h_s.z);
}



//---------------------------------------------------------------
//---------------------------------------------------------------

inline clMatrix4x4::clMatrix4x4()							//init to identity
	{
	ID();
	}
inline void clMatrix4x4::ID()								//init to identity
	{
	M44[0][0] = M44[1][1] = M44[2][2] = M44[3][3] = 1.0f;



				M44[1][0] = M44[2][0] = M44[3][0] = 
	M44[0][1] =				M44[2][1] = M44[3][1] = 
	M44[0][2] = M44[1][2] =				M44[3][2] = 
	M44[0][3] = M44[1][3] = M44[2][3] = 

	0.0f;
	}

inline clMatrix4x4::clMatrix4x4(clQuaternion & q)			//create from quat
	{
	FromQuat(q);
	}

inline void clMatrix4x4::FromQuat(clQuaternion & q)		//sets itself to rotmatrix based on quat passed
	{
	const flo w = q.w;
	const flo x = q.x;
	const flo y = q.y;
	const flo z = q.z;

	M16[  0 ] = 1.0f - y*y*2.0f - z*z*2.0f;
	M16[  1 ] = x*y*2.0f + w*z*2.0f;	
	M16[  2 ] = x*z*2.0f - w*y*2.0f;	
	M16[  3 ] = 0.0f;	

	M16[  4 ] = x*y*2.0f - w*z*2.0f;	
	M16[  5 ] = 1.0f - x*x*2.0f - z*z*2.0f;	
	M16[  6 ] = y*z*2.0f + w*x*2.0f;	
	M16[  7 ] = 0.0f;	
	
	M16[  8 ] = x*z*2.0f + w*y*2.0f;	
	M16[  9 ] = y*z*2.0f - w*x*2.0f;	
	M16[ 10 ] = 1.0f - x*x*2.0f - y*y*2.0f;	
	M16[ 11 ] = 0.0f;	
	
	M16[ 12 ] = 0.0f;	
	M16[ 13 ] = 0.0f;	
	M16[ 14 ] = 0.0f;	
	M16[ 15 ] = 1.0f;	
	}

inline void clMatrix4x4::Set(clMatrix4x4 & cloMatrix)		//This = Matrix
	{
	for (int n=0; n<16; n++)
		M16[n] = cloMatrix.M16[n];
	}

inline void clMatrix4x4::Set(flo tx, flo ty, flo tz, flo roty, flo rotz, flo rotx, Mat4 * inverse)	//sets matrix to product: rot_x(rotx)*rot_z(rotz)*rot_y(roty)*translate(txyz).  angles in radians.
	{
	flo sinx = sinf(rotx); flo cosx = cosf(rotx);
	flo siny = sinf(roty); flo cosy = cosf(roty);
	flo sinz = sinf(rotz); flo cosz = cosf(rotz);

	//below code was generated by maple from a certain worksheet...
	
	flo t1 = cosz*cosy;
	flo t3 = cosz*siny;
	flo t9 = cosx*sinz;
	flo t13 = cosx*cosz;
	flo t22 = sinx*sinz;
	flo t27 = sinx*cosz;
	flo t12 = t9*cosy+sinx*siny;
	flo t17 = t9*siny-1*sinx*cosy;
	flo t26 = t22*cosy-1*cosx*siny;
	flo t30 = t22*siny+cosx*cosy;

	M44[0][0] = t1;
	M44[0][1] = t12;
	M44[0][2] = t26;
	M44[0][3] = 0;

	M44[1][0] = -1*sinz;
	M44[1][1] = t13;
	M44[1][2] = t27;
	M44[1][3] = 0;

	M44[2][0] = t3;
	M44[2][1] = t17;
	M44[2][2] = t30;
	M44[2][3] = 0;

	M44[3][0] = t1*tx-1*sinz*ty+t3*tz;
	M44[3][1] = t12*tx+t13*ty+t17*tz;
	M44[3][2] = t26*tx+t27*ty+t30*tz;
	M44[3][3] = 1;


	if (inverse)
		{
		//negate the angles: sin(-x) = -sin(x)  cos(-x) == cos(x) 
		sinx*=-1;
		siny*=-1;
		sinz*=-1;
		//below code also generated by maple (we also set the negative of txyz below)
		flo t2 = cosy*sinz;
		flo t15 = siny*sinz;

		inverse->M44[0][0] = cosy*cosz;
		inverse->M44[0][1] = sinz; 
		inverse->M44[0][2] = -1*siny*cosz;
		inverse->M44[0][3] = 0; 

		inverse->M44[1][0] = -1*t2*cosx + siny*sinx;
		inverse->M44[1][1] = cosz*cosx;
		inverse->M44[1][2] = t15*cosx + cosy*sinx;
		inverse->M44[1][3] = 0;

		inverse->M44[2][0] = t2*sinx + siny*cosx; 
		inverse->M44[2][1] = -1*cosz*sinx;
		inverse->M44[2][2] = -1*t15*sinx + cosy*cosx;
		inverse->M44[2][3] = 0;

		inverse->M44[3][0] = -tx;
		inverse->M44[3][1] = -ty;
		inverse->M44[3][2] = -tz;
		inverse->M44[3][3] = 1;
		}
	}


inline void clMatrix4x4::SetProjection(float tan_half_fovHorizontal, float tan_half_fovVertical, float zNear, float zFar )
	{
	float xmin, xmax, ymin, ymax;
	float one_deltax, one_deltay, one_deltaz, doubleznear;	//note:  I don't believe this produces a 'w friendly' matrix as requested by Direct3D for fog and depth buffering.  Oh well.

	xmax = zNear * tan_half_fovHorizontal;//tanf( fovHorizontal/2 );
	xmin = -xmax;

	ymax = zNear * tan_half_fovVertical;//tanf ( fovVertical/2 ); //ymax = xmax / aspect;
	ymin = -ymax;


	doubleznear = 2 * zNear;
	one_deltax = 1 / (xmax - xmin);
	one_deltay = 1 / (ymax - ymin);
	one_deltaz = 1 / (zFar - zNear);

	M16[0] = (float)(doubleznear * one_deltax);
	M16[1] = 0.f;
	M16[2] = 0.f;
	M16[3] = 0.f;

	M16[4] = 0.f;
	M16[5] = (float)(doubleznear * one_deltay);
	M16[6] = 0.f;
	M16[7] = 0.f;

	M16[8] = (float)((xmax + xmin) * one_deltax);
	M16[9] = (float)((ymax + ymin) * one_deltay);
	M16[10] =(float)(-(zFar + zNear) * one_deltaz);
	M16[11] = -1.0f;

	M16[12] = 0.f;
	M16[13] = 0.f;
	M16[14] = (float)(-(zFar * doubleznear) * one_deltaz);
	M16[15] = 0;
	}

inline void clMatrix4x4::SetTranslation(const clVector3D & v)
	{
	M16[ 12 ] = v.x;
	M16[ 13 ] = v.y;	
	M16[ 14 ] = v.z;	
	}

inline void clMatrix4x4::SetTranslationNegative(const clVector3D & v)	//same as above, but sets vars to -clopVector.
	{
	M16[ 12 ] = -v.x;
	M16[ 13 ] = -v.y;	
	M16[ 14 ] = -v.z;	
	}

inline void clMatrix4x4::TranslateNegative(clVector3D & v)//this = this * TranslationMx(-clopVector);
	{																//assumes all but 3x3 part is 0, except lower right corner = 1
	M44[3][0] = -M44[0][0] * v.x	-M44[1][0] * v.y		-M44[2][0] * v.z;
	M44[3][1] = -M44[0][1] * v.x	-M44[1][1] * v.y		-M44[2][1] * v.z;
	M44[3][2] = -M44[0][2] * v.x	-M44[1][2] * v.y		-M44[2][2] * v.z;

	}

inline void clMatrix4x4::Copy3x3(clMatrix4x4 & m)	//copies the 3x3 part of the passed matrix to the 3x3 part of this matrix.
	{
	M16[  0 ] =m.M16[0];
	M16[  1 ] =m.M16[1];
	M16[  2 ] =m.M16[2];

	M16[  4 ] =m.M16[4];
	M16[  5 ] =m.M16[5];
	M16[  6 ] =m.M16[6];
	
	M16[  8 ] =m.M16[8];
	M16[  9 ] =m.M16[9];
	M16[ 10 ] =m.M16[10];
	}

inline void clMatrix4x4::Copy3x3Transposed(clMatrix4x4 & m)	//same as above, but transposes 3x3 part as it copies.  (sets the inverse of a rotation)
	{
	M16[  0 ] =m.M16[0];
	M16[  1 ] =m.M16[4];
	M16[  2 ] =m.M16[8];

	M16[  4 ] =m.M16[1];
	M16[  5 ] =m.M16[5];
	M16[  6 ] =m.M16[9];
	
	M16[  8 ] =m.M16[2];
	M16[  9 ] =m.M16[6];
	M16[ 10 ] =m.M16[10];
	}

inline void clMatrix4x4::Copy3x3Transposed(clMatrix3x3 & m)	//same as above, but transposes 3x3 part as it copies.  (sets the inverse of a rotation)
	{
	M16[  0 ] =m.M9[0];
	M16[  1 ] =m.M9[3];
	M16[  2 ] =m.M9[6];

	M16[  4 ] =m.M9[1];
	M16[  5 ] =m.M9[4];
	M16[  6 ] =m.M9[7];
	
	M16[  8 ] =m.M9[2];
	M16[  9 ] =m.M9[5];
	M16[ 10 ] =m.M9[8];
	}

inline clVector3D clMatrix4x4::Rotate(const clVector3D & r) const		
	{
	clVector3D a;

	a.x = M44[0][0] * r.x + M44[1][0] * r.y + M44[2][0] * r.z;
	a.y = M44[0][1] * r.x + M44[1][1] * r.y + M44[2][1] * r.z; 
	a.z = M44[0][2] * r.x + M44[1][2] * r.y + M44[2][2] * r.z; 

	return a;
	}

inline clVector3D clMatrix4x4::RotateTransposed(const clVector3D & r) const		
	{
	clVector3D a;

	a.x = M44[0][0] * r.x + M44[0][1] * r.y + M44[0][2] * r.z;
	a.y = M44[1][0] * r.x + M44[1][1] * r.y + M44[1][2] * r.z; 
	a.z = M44[2][0] * r.x + M44[2][1] * r.y + M44[2][2] * r.z; 

	return a;
	}


inline clMatrix4x4  clMatrix4x4::operator*  (const clMatrix4x4& r) const
	{
	clMatrix4x4 a;	//temp result of multiplication

	a.M44[0][0] = M44[0][0] * r.M44[0][0] + M44[1][0] * r.M44[0][1] + M44[2][0] * r.M44[0][2] + M44[3][0] * r.M44[0][3];
	a.M44[0][1] = M44[0][1] * r.M44[0][0] + M44[1][1] * r.M44[0][1] + M44[2][1] * r.M44[0][2] + M44[3][1] * r.M44[0][3]; 
	a.M44[0][2] = M44[0][2] * r.M44[0][0] + M44[1][2] * r.M44[0][1] + M44[2][2] * r.M44[0][2] + M44[3][2] * r.M44[0][3];
	a.M44[0][3] = M44[0][3] * r.M44[0][0] + M44[1][3] * r.M44[0][1] + M44[2][3] * r.M44[0][2] + M44[3][3] * r.M44[0][3];

	a.M44[1][0] = M44[0][0] * r.M44[1][0] + M44[1][0] * r.M44[1][1] + M44[2][0] * r.M44[1][2] + M44[3][0] * r.M44[1][3];
	a.M44[1][1] = M44[0][1] * r.M44[1][0] + M44[1][1] * r.M44[1][1] + M44[2][1] * r.M44[1][2] + M44[3][1] * r.M44[1][3];
	a.M44[1][2] = M44[0][2] * r.M44[1][0] + M44[1][2] * r.M44[1][1] + M44[2][2] * r.M44[1][2] + M44[3][2] * r.M44[1][3]; 
	a.M44[1][3] = M44[0][3] * r.M44[1][0] + M44[1][3] * r.M44[1][1] + M44[2][3] * r.M44[1][2] + M44[3][3] * r.M44[1][3];

	a.M44[2][0] = M44[0][0] * r.M44[2][0] + M44[1][0] * r.M44[2][1] + M44[2][0] * r.M44[2][2] + M44[3][0] * r.M44[2][3]; 
	a.M44[2][1] = M44[0][1] * r.M44[2][0] + M44[1][1] * r.M44[2][1] + M44[2][1] * r.M44[2][2] + M44[3][1] * r.M44[2][3]; 
	a.M44[2][2] = M44[0][2] * r.M44[2][0] + M44[1][2] * r.M44[2][1] + M44[2][2] * r.M44[2][2] + M44[3][2] * r.M44[2][3]; 
	a.M44[2][3] = M44[0][3] * r.M44[2][0] + M44[1][3] * r.M44[2][1] + M44[2][3] * r.M44[2][2] + M44[3][3] * r.M44[2][3];

	a.M44[3][0] = M44[0][0] * r.M44[3][0] + M44[1][0] * r.M44[3][1] + M44[2][0] * r.M44[3][2] + M44[3][0] * r.M44[3][3];
	a.M44[3][1] = M44[0][1] * r.M44[3][0] + M44[1][1] * r.M44[3][1] + M44[2][1] * r.M44[3][2] + M44[3][1] * r.M44[3][3];
	a.M44[3][2] = M44[0][2] * r.M44[3][0] + M44[1][2] * r.M44[3][1] + M44[2][2] * r.M44[3][2] + M44[3][2] * r.M44[3][3];
	a.M44[3][3] = M44[0][3] * r.M44[3][0] + M44[1][3] * r.M44[3][1] + M44[2][3] * r.M44[3][2] + M44[3][3] * r.M44[3][3];

	return a;
	}

inline clVector3D   clMatrix4x4::operator*  (const clVector3D & r) const //implicitly extends vector to 1x4
	{
	clVector3D a;	//temp result of multiplication
	//a.multiply(*this, r_h_s):

	a.x = M44[0][0] * r.x + M44[1][0] * r.y + M44[2][0] * r.z + M44[3][0];
	a.y = M44[0][1] * r.x + M44[1][1] * r.y + M44[2][1] * r.z + M44[3][1]; 
	a.z = M44[0][2] * r.x + M44[1][2] * r.y + M44[2][2] * r.z + M44[3][2]; 

	return a;
	}

inline clVector3D   clMatrix4x4::multHDiv  (const clVector3D & r, bool onlyDivXY) const		// r.w is implicitly 1, output w is explicit, before it gets divided away to go back to regular 3D space.
	{
	clVector3D a;	//temp result of multiplication
	float a_w;

	a.x = M44[0][0] * r.x + M44[1][0] * r.y + M44[2][0] * r.z + M44[3][0];
	a.y = M44[0][1] * r.x + M44[1][1] * r.y + M44[2][1] * r.z + M44[3][1]; 
	a.z = M44[0][2] * r.x + M44[1][2] * r.y + M44[2][2] * r.z + M44[3][2]; 
	a_w = M44[0][3] * r.x + M44[1][3] * r.y + M44[2][3] * r.z + M44[3][3]; 

//	assert(a_w != 0);	//I now have code where it is desired behavior to return infinity.

	a_w = (float)(1.0/fabs(a_w));	//the fabs is not needed for simple projection, but for clipping I don't want the negative-z region to be flipped.

	a.x *= a_w;			
	a.y *= a_w;
	if (!onlyDivXY)
		a.z *= a_w;
	return a;
	}


inline void clMatrix4x4::Invert3x4(bool scaling)
	{
	//these are the inverted parts:
	//note: this only supports uniform scaling, else it gets difficult.
	float scale = 1.0f;
	/*
	clMatrix4x4 rotation;
	clMatrix4x4 translation;
	//pick apart the matrix into its three pieces

	if (scaling)
		{
		//find out what the scaling is - magnitude of column vectors
		scale = ((Vec3 *)&(M16[0]))->Magnitude();

		//invert the scale
		scale = 1/scale;

		// get rid of the scale
		(*((Vec3 *)&(M16[0]))) *= scale;
		(*((Vec3 *)&(M16[4]))) *= scale;
		(*((Vec3 *)&(M16[8]))) *= scale;
		}

	// set the rotation
	rotation.Copy3x3Transposed(*this);

	if (scaling)
		{
		// reapply the scale
		(*((Vec3 *)&(rotation.M16[0]))) *= scale;
		(*((Vec3 *)&(rotation.M16[4]))) *= scale;
		(*((Vec3 *)&(rotation.M16[8]))) *= scale;
		}
	// set the translation
	translation.SetTranslationNegative((*((Vec3 *)&(M16[12]))));

	// compute the product:
	*this = rotation * translation;
	*/

	//the same thing optimized, in place: 
	clMatrix4x4 a;
	if (scaling)
		{
		//find out what the scaling is - magnitude of column vectors
		scale = ((Vec3 *)&(M16[0]))->Magnitude();

		//invert the scale
		scale = 1/scale;

		scale = scale * scale;
		}

	a.M44[0][0] = M44[0][0]*scale;
	a.M44[0][1] = M44[1][0]*scale; 
	a.M44[0][2] = M44[2][0]*scale;
//	a.M44[0][3] = 0;

	a.M44[1][0] = M44[0][1]*scale;
	a.M44[1][1] = M44[1][1]*scale;
	a.M44[1][2] = M44[2][1]*scale; 
//	a.M44[1][3] = 0; 

	a.M44[2][0] = M44[0][2]*scale; 
	a.M44[2][1] = M44[1][2]*scale; 
	a.M44[2][2] = M44[2][2]*scale; 
//	a.M44[2][3] = 0; 

	SetTranslation(a.Rotate(Vec3(-M44[3][0], -M44[3][1], -M44[3][2])));
	Copy3x3(a);
//	a.M44[3][3] = 1; 



	}

//---------------------------------------------------------------
//---------------------------------------------------------------

inline clMatrix3x3::clMatrix3x3()							//init to identity
	{
	ID();
	}

inline void clMatrix3x3::ID()								//init to identity
	{
	M33[0][0] = M33[1][1] = M33[2][2] = 1.0f;

				M33[1][0] = M33[2][0] = 
	M33[0][1] =				M33[2][1] = 
	M33[0][2] = M33[1][2] =				

	0.0f;
	}

inline clMatrix3x3& clMatrix3x3::operator=  (const clMatrix3x3& r_h_s)
	{
	for (int n=0; n<9; n++)	//copy that to this matrix
	M9[n] = r_h_s.M9[n];
	return *this;
	}

inline clMatrix3x3& clMatrix3x3::operator=  (const flo * f9)
	{
	for (int n=0; n<9; n++)	//copy that to this matrix
	M9[n] = f9[n];
	return *this;
	}


inline clMatrix3x3  clMatrix3x3::operator*  (const clMatrix3x3& r) const
	{
	clMatrix3x3 a;	//temp result of multiplication
	//a.multiply(*this, r_h_s):

	//cols of a:
	a.M33[0][0] = M33[0][0] * r.M33[0][0] + M33[1][0] * r.M33[0][1] + M33[2][0] * r.M33[0][2];
	a.M33[0][1] = M33[0][1] * r.M33[0][0] + M33[1][1] * r.M33[0][1] + M33[2][1] * r.M33[0][2]; 
	a.M33[0][2] = M33[0][2] * r.M33[0][0] + M33[1][2] * r.M33[0][1] + M33[2][2] * r.M33[0][2]; 

	a.M33[1][0] = M33[0][0] * r.M33[1][0] + M33[1][0] * r.M33[1][1] + M33[2][0] * r.M33[1][2]; 
	a.M33[1][1] = M33[0][1] * r.M33[1][0] + M33[1][1] * r.M33[1][1] + M33[2][1] * r.M33[1][2]; 
	a.M33[1][2] = M33[0][2] * r.M33[1][0] + M33[1][2] * r.M33[1][1] + M33[2][2] * r.M33[1][2]; 

	a.M33[2][0] = M33[0][0] * r.M33[2][0] + M33[1][0] * r.M33[2][1] + M33[2][0] * r.M33[2][2]; 
	a.M33[2][1] = M33[0][1] * r.M33[2][0] + M33[1][1] * r.M33[2][1] + M33[2][1] * r.M33[2][2]; 
	a.M33[2][2] = M33[0][2] * r.M33[2][0] + M33[1][2] * r.M33[2][1] + M33[2][2] * r.M33[2][2]; 

	return a;
	}
inline clMatrix3x3  clMatrix3x3::operator%  (const clMatrix3x3& r) const
	{//mulitplies by transpose of r_h_s
	clMatrix3x3 a;	//temp result of multiplication

	//cols of a:
	a.M33[0][0] = M33[0][0] * r.M33[0][0] + M33[1][0] * r.M33[1][0] + M33[2][0] * r.M33[2][0];
	a.M33[0][1] = M33[0][1] * r.M33[0][0] + M33[1][1] * r.M33[1][0] + M33[2][1] * r.M33[2][0]; 
	a.M33[0][2] = M33[0][2] * r.M33[0][0] + M33[1][2] * r.M33[1][0] + M33[2][2] * r.M33[2][0]; 

	a.M33[1][0] = M33[0][0] * r.M33[0][1] + M33[1][0] * r.M33[1][1] + M33[2][0] * r.M33[2][1]; 
	a.M33[1][1] = M33[0][1] * r.M33[0][1] + M33[1][1] * r.M33[1][1] + M33[2][1] * r.M33[2][1]; 
	a.M33[1][2] = M33[0][2] * r.M33[0][1] + M33[1][2] * r.M33[1][1] + M33[2][2] * r.M33[2][1]; 

	a.M33[2][0] = M33[0][0] * r.M33[0][2] + M33[1][0] * r.M33[1][2] + M33[2][0] * r.M33[2][2]; 
	a.M33[2][1] = M33[0][1] * r.M33[0][2] + M33[1][1] * r.M33[1][2] + M33[2][1] * r.M33[2][2]; 
	a.M33[2][2] = M33[0][2] * r.M33[0][2] + M33[1][2] * r.M33[1][2] + M33[2][2] * r.M33[2][2]; 

	return a;
	}

inline clVector3D   clMatrix3x3::operator*  (const clVector3D & r) const
	{
	clVector3D a;	//temp result of multiplication
	//a.multiply(*this, r_h_s):

	a.x = M33[0][0] * r.x + M33[1][0] * r.y + M33[2][0] * r.z;
	a.y = M33[0][1] * r.x + M33[1][1] * r.y + M33[2][1] * r.z; 
	a.z = M33[0][2] * r.x + M33[1][2] * r.y + M33[2][2] * r.z; 

	return a;
	}

inline void clMatrix3x3::vecTransMult(const clVector3D & r, clVector3D & result) const	// performs dest = rhs^t * this
	{
	result.x = M33[0][0] * r.x + M33[0][1] * r.y + M33[0][2] * r.z;
	result.y = M33[1][0] * r.x + M33[1][1] * r.y + M33[1][2] * r.z; 
	result.z = M33[2][0] * r.x + M33[2][1] * r.y + M33[2][2] * r.z; 
	}


inline void clMatrix3x3::FromQuat(clQuaternion & q)		//sets itself to rotmatrix based on quat passed
	{
	const flo w = q.w;
	const flo x = q.x;
	const flo y = q.y;
	const flo z = q.z;

	M9[  0 ] = 1.0f - y*y*2.0f - z*z*2.0f;
	M9[  1 ] = x*y*2.0f + w*z*2.0f;	
	M9[  2 ] = x*z*2.0f - w*y*2.0f;	

	M9[  3 ] = x*y*2.0f - w*z*2.0f;	
	M9[  4 ] = 1.0f - x*x*2.0f - z*z*2.0f;	
	M9[  5 ] = y*z*2.0f + w*x*2.0f;	
	
	M9[  6 ] = x*z*2.0f + w*y*2.0f;	
	M9[  7 ] = y*z*2.0f - w*x*2.0f;	
	M9[  8 ] = 1.0f - x*x*2.0f - y*y*2.0f;	
	
	}

inline void Mat33::GetColumn(int i,Vec3 & col)
	{
	col.x = M33[i][0];
	col.y = M33[i][1];
	col.z = M33[i][2];
	}
inline void  Mat33::SetColumn(int i,Vec3 & col)
	{
	M33[i][0] = col.x;
	M33[i][1] = col.y;
	M33[i][2] = col.z;
	}

inline void Mat33::Orthonormalize()  //uses GramSchmidt algorithm to orthogonalize, based on Maple code `Copyright (c) 1997 by Erich Kaltofen`
									// then normalize
	{
	Vec3 w,t1,t2,t3;
	flo norm_sq;

    const flo m=3;			//m := linalg[rowdim](A);
    const flo n=3;			//n := linalg[coldim](A);
	int i, j, k = 0;				//k := 0;


    Mat33 v = *this;				//v := linalg[col](A, 1 .. n); -- 3 column vectors indexable
    Vec3 norm_u_sq;
																//# orthogonalize v[i]
    for (i=0; i<n; i++)//for i to n do
		{
        v.GetColumn(i,w);		//i-th column
        for (j=0; j<k; j++)									//# pull w along projection of v[i] with u[j]
			{
			this->GetColumn(j,t1);
			this->GetColumn(j,t2);
			v.GetColumn(i,t3);
			Vec3 temp = (t2 * (1.0f/norm_u_sq.GetElem(j)));
			Vec3 temp2 = temp  * t3.Dot( t1 );
			w -= temp;	
			}
																//        # compute norm of orthogonalized v[i]
      norm_sq = w.Dot(w);

		if (norm_sq != 0.0f) 
			{													//           # linearly independent new orthogonal vector 
																//       # add to list of u and norm_u_sq
			this->SetColumn(i,w);									//u = [op(u), evalm(w)];
            norm_u_sq.SetElem(i,norm_sq);						//norm_u_sq = [op(norm_u_sq), norm_sq];
            k ++;
			}
		}
	

	Vec3 temp;													//may want to do this in-place -- dunno if optimizer does this for me
	for (i=0; i<3; i++)
		{
		GetColumn(i,temp);
		temp.Normalize();
		SetColumn(i,temp);
		}

	}

#endif //__3DMATH_H__

