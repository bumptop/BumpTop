#ifndef NX_PHYSICS_NXPHYSICS
#define NX_PHYSICS_NXPHYSICS
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

/**
This is the main include header for the Physics SDK, for users who
want to use a single #include file.

Alternatively, one can instead directly #include a subset of the below files.
*/

#include "NxFoundation.h"		//include the all of the foundation SDK 
//#include "C:\Program Files\NovodeX SDK\SDKs\Foundation\include\NxFoundation.h"		//include the all of the foundation SDK 


//////////////general:

#include "NxScene.h"
#include "NxSceneDesc.h"

#include "NxActor.h"
#include "NxActorDesc.h"

#include "NxMaterial.h"

#include "NxContactStreamIterator.h"

#include "NxUserContactReport.h"
#include "NxUserNotify.h"

#include "NxBodyDesc.h"

#include "NxEffector.h"

#include "NxSpringAndDamperEffector.h"
#include "NxSpringAndDamperEffectorDesc.h"

/////////////joints:

#include "NxJoint.h"

#include "NxJointLimitDesc.h"
#include "NxJointLimitPairDesc.h"
#include "NxMotorDesc.h"
#include "NxSpringDesc.h"

#include "NxPointInPlaneJoint.h"
#include "NxPointInPlaneJointDesc.h"

#include "NxPointOnLineJoint.h"
#include "NxPointOnLineJointDesc.h"

#include "NxRevoluteJoint.h"
#include "NxRevoluteJointDesc.h"

#include "NxPrismaticJoint.h"
#include "NxPrismaticJointDesc.h"

#include "NxCylindricalJoint.h"
#include "NxCylindricalJointDesc.h"

#include "NxSphericalJoint.h"
#include "NxSphericalJointDesc.h"

//////////////shapes:

#include "NxShape.h"
#include "NxShapeDesc.h"

#include "NxBoxShape.h"
#include "NxBoxShapeDesc.h"

#include "NxCapsuleShape.h"
#include "NxCapsuleShapeDesc.h"

#include "NxPlaneShape.h"
#include "NxPlaneShapeDesc.h"

#include "NxSphereShape.h"
#include "NxSphereShapeDesc.h"

#include "NxTriangleMesh.h"
#include "NxTriangleMeshDesc.h"

#include "NxTriangleMeshShape.h"
#include "NxTriangleMeshShapeDesc.h"


//////////////utils:

#include "NxDistanceTriangleTriangle.h"
#include "NxInertiaTensor.h"
#include "NxIntersectionBoxBox.h"
#include "NxIntersectionPointTriangle.h"
#include "NxIntersectionRayPlane.h"
#include "NxIntersectionRaySphere.h"
#include "NxIntersectionRayTriangle.h"
#include "NxIntersectionSegmentBox.h"
#include "NxIntersectionSegmentCapsule.h"
#include "NxIntersectionSweptSpheres.h"
#include "NxPMap.h"
#include "NxUserRaycastReport.h"
#include "NxSmoothNormals.h"
#include "NxConvexHull.h"


#endif
