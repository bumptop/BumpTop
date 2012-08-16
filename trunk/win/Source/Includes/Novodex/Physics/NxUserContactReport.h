#ifndef NX_COLLISION_NXUSERCONTACTREPORT
#define NX_COLLISION_NXUSERCONTACTREPORT
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com

\*----------------------------------------------------------------------------*/

#include "Nxp.h"
#include "NxShape.h"
#include "NxContactStreamIterator.h"

class NxActor;

enum NxContactPairFlag
	{
	NX_IGNORE_PAIR				= (1<<0),	//!< Disable contact generation for this pair

	NX_NOTIFY_ON_START_TOUCH	= (1<<1),	//!< Pair callback will be called when the pair starts to be in contact
	NX_NOTIFY_ON_END_TOUCH		= (1<<2),	//!< Pair callback will be called when the pair stops to be in contact
	NX_NOTIFY_ON_TOUCH			= (1<<3),	//!< Pair callback will keep getting called while the pair is in contact
	NX_NOTIFY_ON_IMPACT			= (1<<4),	//!< [Not yet implemented] pair callback will be called when it may be appropriate for the pair to play an impact sound
	NX_NOTIFY_ON_ROLL			= (1<<5),	//!< [Not yet implemented] pair callback will be called when the pair is in contact and rolling.
	NX_NOTIFY_ON_SLIDE			= (1<<6),	//!< [Not yet implemented] pair callback will be called when the pair is in contact and sliding (and not rolling).

	NX_NOTIFY_DISABLE			= 0,		//!< Pair will be ignored and onPairCreated will never be called again.
	};


/**
An instance of this class is passed to NxUserContactReport::onContactNotify().
It contains a contact stream which may be parsed using the class NxContactStreamIterator.
You may also use the setPairNotify() method to change the behavior of the pair.
*/
class NxContactPair
	{
	public:
	NX_INLINE	NxContactPair() : stream(NULL)	{}

	/**
	Call this to change the pair behavior that you previously specified with the return 
	value of NxUserContactReport::onPairCreated().
	The parameter is a combination of the flags in NxContactPairFlags.
	*/
	virtual void setPairNotify(NxU32) = 0;

	NxActor*				actors[2];			//!< the two actors that make up the pair.
	NxConstContactStream	stream;				//!< use this to create stream iter.
	NxVec3					sumNormalForce;		//!< the total contact normal force that was applied for this pair, to maintain nonpenetration constraints.
	NxVec3					sumFrictionForce;	//!< the total tangential force that was applied for this pair.
	};

/**
The user needs to implement this interface class in order to be notified when
certain contact events occur. Once you pass an instance of this class to 
NxScene::setUserContactReport(), its  onPairCreated() method will be called for
each pair of actors which comes into close proximity. From then on you may 
request additional contact information, or you may suppress contact between the
two shapes.
*/
class NxUserContactReport
	{
	public:
 	/**
	Called for a pair in contact. The events parameter is a combination of:

	NX_NOTIFY_ON_START_TOUCH,
	NX_NOTIFY_ON_END_TOUCH,
	NX_NOTIFY_ON_TOUCH,
	NX_NOTIFY_ON_IMPACT,	//unimplemented!
	NX_NOTIFY_ON_ROLL,		//unimplemented!
	NX_NOTIFY_ON_SLIDE,		//unimplemented!

	See the documentation of NxContactPairFlag for an explanation of each. You request which events 
	are reported with the return value of onPairCreated(), or by calling the appropriate method
	of the passed pair. Do not keep a reference to the passed object, as it will be invalid after
	this function returns.
	*/
	virtual void  onContactNotify(NxContactPair& pair, NxU32 events) = 0;
	};

/**
The user needs to implement this interface class in order to be notified when trigger events
occur. Once you pass an instance of this class to NxScene::setUserTriggerReport(), shapes
which have been marked as triggers using NxShape::setFlag(NxShape::TriggerFlag) will call the
onTrigger() method when their trigger status changes.
*/
class NxUserTriggerReport
	{
	public:
	/**
	Called when a trigger shape reports a trigger event. triggerShape is the shape that has been
	marked as a trigger, the otherShape is the shape causing the trigger event, and status is the
	type of trigger event.
	*/
	virtual void onTrigger(NxShape& triggerShape, NxShape& otherShape, NxTriggerFlag status) = 0;
	};

#endif
