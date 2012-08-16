#ifndef NX_PHYSICS_NXMOTORDESC
#define NX_PHYSICS_NXMOTORDESC
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

/**
Describes a joint motor.
*/
class NxMotorDesc
	{
	public:
/**
	The relative velocity the motor is trying to achieve. The motor will only be able
	to reach this velocity if the maxForce is sufficiently large. If the joint is 
	spinning faster than this velocity, the motor will actually try to brake. If you set this
	to infinity then the motor will keep speeding up, unless there is some sort of resistance
	on the attached bodies. The sign of this variable determines the rotation direction,
	with positive values going the same way as positive joint angles.
	Default is infinity.
*/
	NxReal velTarget;	//target velocity of motor
/**
	The maximum force (torque in this case) the motor can exert. Zero disables the motor.
	Default is 0, should be >= 0. Setting this to a very large value if velTarget is also 
	very large may not be a good idea.
*/
	NxReal maxForce;	//maximum motor force/torque
/**
	If true, motor will not brake when it spins faster than velTarget
	default: false.
*/
	NX_BOOL freeSpin;

	NX_INLINE NxMotorDesc();
	NX_INLINE NxMotorDesc(NxReal velTarget, NxReal maxForce = 0, NX_BOOL freeSpin = 0);
	NX_INLINE void setToDefault();
	NX_INLINE bool isValid() const;
	};

NX_INLINE NxMotorDesc::NxMotorDesc()
	{
	setToDefault();
	}

NX_INLINE NxMotorDesc::NxMotorDesc(NxReal v, NxReal m, NX_BOOL f)
	{
	velTarget = v;
	maxForce = m;
	freeSpin = f;
	}

NX_INLINE void NxMotorDesc::setToDefault()
	{
	velTarget = NX_MAX_REAL;
	maxForce = 0;
	freeSpin = 0;
	}

NX_INLINE bool NxMotorDesc::isValid() const
	{
	return (maxForce >= 0);
	}

#endif
