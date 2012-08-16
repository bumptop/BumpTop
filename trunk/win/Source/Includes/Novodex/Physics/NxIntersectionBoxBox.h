#ifndef NX_INTERSECTION_BOX_BOX
#define NX_INTERSECTION_BOX_BOX

/*----------------------------------------------------------------------------*\
|
|								NovodeX Technology
|
|								 www.novodex.com
|
\*----------------------------------------------------------------------------*/

#include "Nxp.h"
#include "NxBox.h"

	/**
	Boolean intersection test between two OBBs. Uses the separating axis theorem. Disabling 'full_test' only performs 6 axis tests out of 15.
	*/
	NX_C_EXPORT NXP_DLL_EXPORT bool NX_CALL_CONV NxBoxBoxIntersect(	const NxVec3& extents0, const NxVec3& center0, const NxMat33& rotation0,
														const NxVec3& extents1, const NxVec3& center1, const NxMat33& rotation1,
														bool fullTest);

	/**
	Boolean intersection test between two OBBs. Uses the separating axis theorem. Disabling 'full_test' only performs 6 axis tests out of 15.
	*/
	NX_INLINE bool NxBoxBoxIntersect(const NxBox& obb0, const NxBox& obb1, bool fullTest=true)
		{
		return NxBoxBoxIntersect(
			obb0.extents, obb0.center, obb0.rot,
			obb1.extents, obb1.center, obb1.rot,
			fullTest);
		}

	enum NxSepAxis
	{
		NX_SEP_AXIS_OVERLAP,

		NX_SEP_AXIS_A0,
		NX_SEP_AXIS_A1,
		NX_SEP_AXIS_A2,

		NX_SEP_AXIS_B0,
		NX_SEP_AXIS_B1,
		NX_SEP_AXIS_B2,

		NX_SEP_AXIS_A0_CROSS_B0,
		NX_SEP_AXIS_A0_CROSS_B1,
		NX_SEP_AXIS_A0_CROSS_B2,

		NX_SEP_AXIS_A1_CROSS_B0,
		NX_SEP_AXIS_A1_CROSS_B1,
		NX_SEP_AXIS_A1_CROSS_B2,

		NX_SEP_AXIS_A2_CROSS_B0,
		NX_SEP_AXIS_A2_CROSS_B1,
		NX_SEP_AXIS_A2_CROSS_B2,

		NX_SEP_AXIS_FORCE_DWORD	= 0x7fffffff
	};

	/**
	Computes the separating axis between two OBBs.
	*/
	NX_C_EXPORT NXP_DLL_EXPORT NxSepAxis NX_CALL_CONV NxSeparatingAxis(	const NxVec3& extents0, const NxVec3& center0, const NxMat33& rotation0,
															const NxVec3& extents1, const NxVec3& center1, const NxMat33& rotation1,
															bool fullTest=true);

	/**
	Computes the separating axis between two OBBs.
	*/
	NX_INLINE NxSepAxis NxSeparatingAxis(const NxBox& obb0, const NxBox& obb1, bool fullTest=true)
	{
		return NxSeparatingAxis(
			obb0.extents, obb0.center, obb0.rot,
			obb1.extents, obb1.center, obb1.rot,
			fullTest);
	}
#endif
