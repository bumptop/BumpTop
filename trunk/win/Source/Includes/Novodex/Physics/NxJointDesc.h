#ifndef NX_PHYSICS_NXJOINTDESC
#define NX_PHYSICS_NXJOINTDESC
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

#include "Nxp.h"
#include "NxJoint.h"
#include "NxActor.h"


enum NxJointFlag
    {
    NX_JF_COLLISION_ENABLED	= (1<<0),	//!< Raised if collision detection should be enabled between the jointed parts.
    NX_JF_VISUALIZATION		= (1<<1),	//!< Enable debug renderer for this joint
    };

enum NxJointProjectionMode
	{
	NX_JPM_NONE  = 0,				//!< don't project this joint
	NX_JPM_POINT_MINDIST = 1,		//!< this is the only projection method right now 
	//there are expected to be more modes later
	};

/**
Descriptor class for the NxJoint class. Joint descriptors for all
the different joint types are derived from this class.
*/
class NxJointDesc
	{
	protected:
	const NxJointType type;			//!< The type of joint. This is set by the ctor of the derived class.
	public:	
	NxActor * actor[2];				//!< The two actors connected by the joint. The actors must be in the same scene as this joint. At least one of the two pointers must be a dynamic actor. One of the two may be NULL to indicate the world frame.  Neither may be a static actor!

	NxVec3 localNormal[2];			//!< X axis of joint space, in actor[i]'s space, orthogonal to localAxis[i]
	NxVec3 localAxis[2];			//!< Z axis of joint space, in actor[i]'s space. This is the primary axis of the joint.
	NxVec3 localAnchor[2];			//!< Attachment point of joint in actor[i]'s space

	NxJointMethod method;			//!< Joint method: determines how the joint is simulated. Default is NX_JM_REDUCED. See NxJoint::requestMethod() for details.
	NxReal maxForce;				//!< Maximum linear force that the joint can withstand before breaking, must be positive. Default: +inf. Only the method NX_JM_LAGRANGE joints support breakability at the moment.
	NxReal maxTorque;				//!< Maximum angular force (torque) that the joint can withstand before breaking, must be positive. Default: +inf. Only the method NX_JM_LAGRANGE joints support breakability at the moment.
	void* userData;					//!< Will be copied to NxJoint::userData.
	const char* name;				//!< Possible debug name.  The string is not copied by the SDK, only the pointer is stored.

	NxU32 jointFlags;				//!< This is a combination of the bits defined by ::NxJointFlag .


	NX_INLINE virtual		~NxJointDesc();
	/**
	(re)sets the structure to the default.	
	*/
	NX_INLINE virtual void setToDefault();
	/**
	returns true if the current settings are valid
	*/
	NX_INLINE virtual bool isValid() const;


	/**
	sets the members localAnchor[0,1] by transforming the passed world space
	vector into actor1 resp. actor2's local space. The actor pointers must already be set!
	*/
	NX_INLINE void setGlobalAnchor(const NxVec3 & wsAnchor);

	/**
	sets the members localAxis[0,1] by transforming the passed world space
	vector into actor1 resp. actor2's local space, and finding arbitrary orthogonals for localNormal[0,1].
	The actor pointers must already be set!
	*/
	NX_INLINE void setGlobalAxis(const NxVec3 & wsAxis);

	NX_INLINE NxJointType	getType()	const	{ return type; }
	protected:
	/**
	constructor sets to default.
	*/
	NX_INLINE NxJointDesc(NxJointType t);	
	};

NX_C_EXPORT NXP_DLL_EXPORT void NX_CALL_CONV NxJointDesc_SetGlobalAnchor(NxJointDesc & dis, const NxVec3 & wsAnchor);
NX_C_EXPORT NXP_DLL_EXPORT void NX_CALL_CONV NxJointDesc_SetGlobalAxis(NxJointDesc & dis, const NxVec3 & wsAxis);


NX_INLINE NxJointDesc::NxJointDesc(NxJointType t) : type(t)	//constructor sets to default
	{
	setToDefault();
	}

NX_INLINE NxJointDesc::~NxJointDesc()
	{
	}

NX_INLINE void NxJointDesc::setToDefault()
	{
	for (int i=0; i<2; i++)
		{
		actor[i] = 0;
		localAxis[i].set(0,0,1);
		localNormal[i].set(1,0,0);
		localAnchor[i].zero();
		}

	method		= NX_JM_REDUCED;
	maxForce	= NX_MAX_REAL;
	maxTorque	= NX_MAX_REAL;
	userData	= NULL;
	name		= NULL;
	jointFlags	= NX_JF_VISUALIZATION;
	}

NX_INLINE bool NxJointDesc::isValid() const
	{
	if (!(actor[0] || actor[1]))
		return false;
	//non-null pointers must be dynamic:
	if (actor[0] && ! actor[0]->isDynamic())
		return false;
	if (actor[1] && ! actor[1]->isDynamic())
		return false;

	if (type >= NX_JOINT_COUNT)
		return false;
	for (int i=0; i<2; i++)
		{
		if (localAxis[i].magnitudeSquared() < 0.9f) return false;
		if (localNormal[i].magnitudeSquared() < 0.9f) return false;
		//check orthogonal pairs
		if (localAxis[i].dot(localNormal[i]) > 0.1f) return false;
		}
	if (maxForce <= 0)
		return false;
	if (maxTorque <= 0)
		return false;

	return true;
	}

NX_INLINE void NxJointDesc::setGlobalAnchor(const NxVec3 & wsAnchor)
	{
	NxJointDesc_SetGlobalAnchor(*this, wsAnchor);
	}

NX_INLINE void NxJointDesc::setGlobalAxis(const NxVec3 & wsAxis)
	{
	NxJointDesc_SetGlobalAxis(*this, wsAxis);
	}

#endif