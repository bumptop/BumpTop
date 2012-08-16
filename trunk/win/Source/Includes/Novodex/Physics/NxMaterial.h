#ifndef NX_PHYSICS_NXMATERIAL
#define NX_PHYSICS_NXMATERIAL
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

enum NxMaterialFlag
	{
	/**
	Flag to enable anisotropic friction computation. 

	For a pair of actors, anisotropic friction is used only if at least one of the two actors' materials are anisotropic.
	The anisotropic friction parameters for the pair are taken from the material which is more anisotropic (i.e. the difference
	between its two dynamic friction coefficients is greater).

	The anisotropy direction of the chosen material is transformed to world space:

	dirOfAnisotropyWS = actor2world * dirOfAnisotropy

	Next, the directions of anisotropy in one or more contact planes (i.e. orthogonal to the contact normal) have to be determined. 
	The two directions are:

	uAxis = (dirOfAnisotropyWS ^ contactNormal).normalize()
	vAxis = contactNormal ^ uAxis

	This way [uAxis, contactNormal, vAxis] forms a basis.

	It may happen, however, that (dirOfAnisotropyWS | contactNormal).magnitude() == 1 
	and then (dirOfAnisotropyWS ^ contactNormal) has zero length. This happens when 
	the contactNormal is coincident to the direction of anisotropy. In this case we perform isotropic friction. 
	*/
	NX_MF_ANISOTROPIC = 1 << 0,
	/**
	[Not yet implemented]

	Flag to enable moving surface computations.

	When set, points on the surface of the object are assumed to be moving in the direction dirOfAnisotropy with
	speed speedOfMotion. As this is only intended for tangential motion at a contact, the normal component of this 
	motion at a contact point is ignored. 
	*/
	NX_MF_MOVING_SURFACE = 1 << 1,

	//Note: Bits 16-31 are reserved for internal use!
	};

/**
[Not yet implemented]

Flag that determines the combine mode. When two actors come in contact with eachother, they each have
materials with various coefficients, but we only need a single set of coefficients for the pair.

Physics doesn't have any inherent combinations because the coefficients are determined empirically on a case by case
basis. However, simulating this with a pairwise lookup table is often impractical.

For this reason the following combine behaviors are available:

CM_MIN = 0,
CM_MULTIPLY = 1,
CM_AVERAGE = 2,
CM_MAX = 3,
CM_TABLE = 4,

The effective combine mode for the pair is max(material0.combineMode, material1.combineMode).
*/
enum NxCombineMode
	{
	CM_MIN = 0,
	CM_MULTIPLY = 1,
	CM_AVERAGE = 2,
	CM_MAX = 3,
	CM_TABLE = 4,
	CM_N_VALUES = 5,	//this a sentinel to denote the number of possible values.  We assert that the variable's value is smaller than this.
	CM_PAD_32 = 0xffffffff 
	};


/**
Descriptor-like user side class for describing surface properties.
*/
class NxMaterial
	{
	public:
	/**
	coefficient of dynamic friction -- should be in [0, 1] and also be less or equal to staticFriction.
	if flags.anisotropic is set, then this value is used for the primary direction of anisotropy (U axis)
	*/
	NxReal	dynamicFriction;
	/**
	coefficient of static friction -- should be in [0, +inf]
	if flags.anisotropic is set, then this value is used for the primary direction of anisotropy (U axis)
	*/
	NxReal	staticFriction;
	NxReal	spinFriction;		//!< [Not yet implemented]
	NxReal	rollFriction;		//!< [Not yet implemented]

	NxReal	restitution;		//!< coefficient of restitution

	/**
	anisotropic dynamic friction coefficient for along the secondary (V) axis of anisotropy. 
	This is only used if flags.anisotropic is set.
	*/
	NxReal dynamicFrictionV;
	/**
	anisotropic static  friction coefficient for along the secondary (V) axis of anisotropy. 
	This is only used if flags.anisotropic is set.
	*/
	NxReal staticFrictionV;
	/**
	actor space direction (unit vector) of anisotropy.
	This is only used if flags.anisotropic is set.
	*/
	NxVec3 dirOfAnisotropy;
	/**
	actor space direction (unit vector) of surface motion.
	This is only used if flags.movingSurface is set.
	*/
	NxVec3 dirOfMotion;
	/**
	The current speed of surface motion in the direction of dirOfMotion.
	This is only used if flags.movingSurface is set.
	*/
	NxReal speedOfMotion;

	/**
	Flags, a combination of the bits defined by the enum ::NxMaterialFlag . 
	*/
	NxU32 flags;

	/**
	[Not yet implemented]
	Friction combine mode.  See the enum ::NxCombineMode .
	*/
	NxCombineMode frictionCombineMode;

	/**
	constructor sets to default.
	*/
	NX_INLINE NxMaterial();	
	/**
	(re)sets the structure to the default.	
	*/
	NX_INLINE void setToDefault();
	/**
	returns true if the current settings are valid
	*/
	NX_INLINE bool isValid() const;
	};

NX_INLINE NxMaterial::NxMaterial()
	{
	setToDefault();
	}

NX_INLINE	void NxMaterial::setToDefault()
	{
	dynamicFriction	= 0.0f;
	staticFriction	= 0.0f;
	spinFriction	= 0.0f;
	rollFriction	= 0.0f;
	restitution		= 0.0f;


	dynamicFrictionV= 0.0f;
	staticFrictionV = 0.0f;

	dirOfAnisotropy.set(1,0,0);
	dirOfMotion.set(1,0,0);
	speedOfMotion = 0.0f;
	flags = 0;
	frictionCombineMode = CM_AVERAGE;
	}

NX_INLINE	bool NxMaterial::isValid()	const
	{
	if(dynamicFriction < 0.0f || dynamicFriction > 1.0f) 
		return false;
	if(staticFriction < 0.0f) 
		return false;
	if(restitution < 0.0f || restitution > 1.0f) 
		return false;

	if (flags & NX_MF_ANISOTROPIC)
		{
		NxReal ad = dirOfAnisotropy.magnitudeSquared();
		if (ad < 0.98f || ad > 1.03f)
			return false;
		if(dynamicFrictionV < 0.0f || dynamicFrictionV > 1.0f) 
			return false;
		if(staticFrictionV < 0.0f) 
			return false;
		}
	if (flags & NX_MF_MOVING_SURFACE)
		{
		NxReal md = dirOfMotion.magnitudeSquared();
		if (md < 0.98f || md > 1.03f)
			return false;
		}
	if (frictionCombineMode >= CM_N_VALUES)
		return false;

	return true;
	}

#endif
