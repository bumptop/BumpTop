#ifndef NX_PHYSICS_NXHINGEJOINT
#define NX_PHYSICS_NXHINGEJOINT
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

#include "Nxp.h"

class NxJoint;
#include "NxRevoluteJointDesc.h"

static const NxReal NX_NO_LOW_LIMIT  = -16;
static const NxReal NX_NO_HIGH_LIMIT =  16;


/**
 A hinge joint removes all but a single rotational degree of freedom from two objects.
 The axis along which the two bodies may rotate is specified with a point and a direction
 vector.
*/
class NxRevoluteJoint
	{
	public:
	/**
	use this for changing a significant number of joint parameters at once.
	Use the set() methods for changing only a single property at once.
	*/
	virtual void loadFromDesc(const NxRevoluteJointDesc&) = 0;

	/**
	writes all of the object's attributes to the desc struct  
	*/
	virtual void saveToDesc(NxRevoluteJointDesc&) = 0;

	/**
	Sets angular joint limits.
	If either of these limits are set, any planar limits in NxJoint are ignored.
	The limits are angles defined the same way as the values getAngle() returns.
	The following has to hold:
	- Pi < lowAngle < highAngle < Pi
	Both limits are disabled by default.

	Also sets coefficients of restitutions for the low and high angular limits.
	These settings are only used if valid limits are set using setLimits().
	These restitution coefficients work the same way as for contacts.

	The coefficient of restitution determines whether a collision with the joint limit is
	completely elastic (like pool balls, restitution = 1, no energy is lost in the collision),
	completely inelastic (like putty, restitution = 0, no rebound after collision) or
	somewhere in between. The default is 0 for both.

	This automatically enables the limit.
	*/
	virtual void setLimits(const NxJointLimitPairDesc &) = 0;

	/**
	Retrieves the joint limits. Returns true if it is enabled.

	Also returns the limit restitutions.
	*/
	virtual bool getLimits(NxJointLimitPairDesc &) = 0;

	/**
	sets motor parameters for the joint. The motor rotates the bodies relative to each other
	along the hinge axis. The motor has these parameters:
	
	velTarget - the relative velocity the motor is trying to achieve. The motor will only be able
				to reach this velocity if the maxForce is sufficiently large. If the joint is 
				spinning faster than this velocity, the motor will actually try to brake. If you set this
				to infinity then the motor will keep speeding up, unless there is some sort of resistance
				on the attached bodies. The sign of this variable determines the rotation direction,
				with positive values going the same way as positive joint angles.
				Default is infinity.
	maxForce -  the maximum force (torque in this case) the motor can exert. Zero disables the motor.
				Default is 0, should be >= 0. Setting this to a very large value if velTarget is also 
				very large may not be a good idea.
	freeSpin -  if this flag is set, and if the joint is spinning faster than velTarget, then neither
				braking nor additional acceleration will result.
				default: false.

	This automatically enables the motor.
	*/
	virtual void setMotor(const NxMotorDesc &) = 0;
	/**
	reads back the motor parameters. Returns true if it is enabled.
	*/
	virtual bool getMotor(NxMotorDesc &) = 0;

	/**
	sets spring parameters. The spring is implicitly integrated so no instability should result for arbitrary
	spring and damping constants. Using these settings together with a motor is not possible -- the motor will have
	priority and the spring settings are ignored.
	If you would like to simulate your motor's internal friction, do this by altering the motor parameters directly.

	spring - The rotational spring acts along the hinge axis and tries to force
				the joint angle to zero. A setting of zero disables the spring. Default is 0, should be >= 0.
	damper - Damping coefficient; acts against the hinge's angular velocity. A setting of zero disables
				the damping. The default is 0, should be >= 0.
	targetValue - The angle at which the spring is relaxed. In [-Pi,Pi]. Default is 0.

	This automatically enables the spring.
	*/
	virtual void setSpring(const NxSpringDesc &) = 0;

	/**
	retrieves spring settings. Returns true if it is enabled.
	*/
	virtual bool getSpring(NxSpringDesc &) = 0;

	/**
	retrieves the current hinge angle. The relative orientation of the bodies
	is stored when the joint is created, or when setAxis() or setAnchor() is called.
	This initial orientation returns an angle of zero, and joint angles are measured
	relative to this pose.
	The angle is in the range [-Pi, Pi], with positive angles CCW around the axis, measured
	from body2 to body1.
	*/
	virtual NxReal getAngle() = 0;

	/**
	retrieves the hinge angle's rate of change (angular velocity).
	It is the angular velocity of body1 minus body2 projected along the axis.
	*/
	virtual NxReal getVelocity() = 0;

	/**
	sets the flags to enable/disable the spring/motor/limit.	This is a combination of the ::NxRevoluteJointFlag bits.
	*/
	virtual void setFlags(NxU32 flags) = 0;

	/**
	returns the current flag settings.
	*/
	virtual NxU32 getFlags() = 0;

	/**
	sets the joint projection mode.
	*/
	virtual void setProjectionMode(NxJointProjectionMode projectionMode) = 0;

	/**
	returns the current flag settings.
	*/
	virtual NxJointProjectionMode getProjectionMode() = 0;
	/**
	This class is internally a subclass of NxJoint. This operator
	is automatically used to perform an upcast.
	*/
	virtual operator NxJoint &() = 0;

	/**
	This class is internally a subclass of NxJoint. Use this
	method to perform an upcast.
	*/
	virtual NxJoint & getJoint() = 0;
	};
#endif