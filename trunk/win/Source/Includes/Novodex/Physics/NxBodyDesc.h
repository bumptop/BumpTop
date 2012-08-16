#ifndef NX_PHYSICS_NXBODYDESC
#define NX_PHYSICS_NXBODYDESC
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

	enum NxBodyFlag
		{
		NX_BF_DISABLE_GRAVITY	= (1<<0),	//!< set if gravity should not be applied on this body
		
		/**	
		Enable/disable freezing for this body/actor. A frozen actor becomes temporarily static.
		Note: this is an experimental feature which doesn't always work on actors which have joints 
		connected to them.
		*/
		NX_BF_FROZEN_POS_X		= (1<<1),
		NX_BF_FROZEN_POS_Y		= (1<<2),
		NX_BF_FROZEN_POS_Z		= (1<<3),
		NX_BF_FROZEN_ROT_X		= (1<<4),
		NX_BF_FROZEN_ROT_Y		= (1<<5),
		NX_BF_FROZEN_ROT_Z		= (1<<6),
		NX_BF_FROZEN_POS		= NX_BF_FROZEN_POS_X|NX_BF_FROZEN_POS_Y|NX_BF_FROZEN_POS_Z,
		NX_BF_FROZEN_ROT		= NX_BF_FROZEN_ROT_X|NX_BF_FROZEN_ROT_Y|NX_BF_FROZEN_ROT_Z,
		NX_BF_FROZEN			= NX_BF_FROZEN_POS|NX_BF_FROZEN_ROT,


		/**
		Enables kinematic mode for the actor.  Kinematic actors are special dynamic actors that are not 
		influenced by forces (such as gravity), and have no momentum.  They appear to have infinite
		mass and can be moved around the world using the moveGlobal*() methods.  They will push 
		regular dynamic actors out of the way.
		
		Currently they will not collide with static or other kinematic objects.  This will change in a later version.
		Note that if a dynamic actor is squished between a kinematic and a static or two kinematics, then it will
		have no choice but to get pressed into one of them.  Later we will make it possible to have the kinematic
		motion be blocked in this case.

		Kinematic actors are great for moving platforms or characters, where direct motion control is desired.

		You can not connect Reduced joints to kinematic actors.  Lagrange joints work ok if the platform
		is moving with a relatively low, uniform velocity.
		*/
		NX_BF_KINEMATIC			= (1<<7),
		NX_BF_VISUALIZATION		= (1<<8),		//!< Enable debug renderer for this body
		};

/**
Descriptor for the optional rigid body dynamic state of NxActor.
*/
class NxBodyDesc
	{
	public:
	NxMat34		massLocalPose;		  //!< position and orientation of the center of mass
	NxVec3		massSpaceInertia;	  //!< Diagonal mass space inertia tensor in body space (all zeros to let the system compute it)
	NxReal		mass;				  //!< Mass of body
	NxVec3		linearVelocity;		  //!< Initial linear velocity
	NxVec3		angularVelocity;	  //!< Initial angular velocity
	NxVec3		initialForce;		  //!< Initial force
	NxVec3		initialTorque;		  //!< Initial torque
	NxReal		wakeUpCounter;		  //!< Initial wake-up counter
	NxReal		linearDamping;		  //!< Linear damping
	NxReal		angularDamping;		  //!< Angular damping
	NxReal		maxAngularVelocity;	  //!< Max. allowed angular velocity (negative values to use default)
	NxU32		flags;				  //!< Combination of body flags

	NxReal		sleepLinearVelocity;  //!< maximum linear velocity at which body can go to sleep. If negative, the global default will be used.
	NxReal		sleepAngularVelocity; //!< maximum angular velocity at which body can go to sleep.  If negative, the global default will be used.

	NxU32		solverIterationCount; //!< solver accuracy setting when dealing with this body.
	/**
	constructor sets to default, mass == 0 (an immediate call to isValid() 
	will return false). 
	*/
	NX_INLINE NxBodyDesc();	
	/**
	(re)sets the structure to the default, mass == 0 (an immediate call to isValid() 
	will return false). 	
	*/
	NX_INLINE void setToDefault();
	/**
	returns true if the current settings are valid
	*/
	NX_INLINE bool isValid() const;
	};

NX_INLINE NxBodyDesc::NxBodyDesc()	//constructor sets to default
	{
	setToDefault();
	}

NX_INLINE void NxBodyDesc::setToDefault()
	{
	massLocalPose			.id();
	massSpaceInertia		.zero();
	linearVelocity			.setNotUsed();
	angularVelocity			.setNotUsed();
	initialForce			.setNotUsed();
	initialTorque			.setNotUsed();
	wakeUpCounter			= 20.0f*0.02f;
	mass					= 0.0f;
	linearDamping			= 0.0f;
	angularDamping			= 0.05f;
	maxAngularVelocity		= -1.0f;
	flags					= NX_BF_VISUALIZATION;
	sleepLinearVelocity		= -1.0;
	sleepAngularVelocity	= -1.0;

	solverIterationCount    = 4;
	}

NX_INLINE bool NxBodyDesc::isValid() const
	{
	if(mass<0.0f)		//no negative masses plz.
		return false;
	if(!massLocalPose.isFinite())
		return false;
	return true;
	}

#endif
