#ifndef NX_FOUNDATION_NXDEBUGRENDERABLE
#define NX_FOUNDATION_NXDEBUGRENDERABLE
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

#include "Nx.h"
#include "NxBox.h"
#include "NxBounds3.h"

/**
Default color values used for debug rendering.
*/
enum NxDebugColor
{
	NX_ARGB_BLACK	= 0xff000000,
	NX_ARGB_RED		= 0xffff0000,
	NX_ARGB_GREEN	= 0xff00ff00,
	NX_ARGB_BLUE	= 0xff0000ff,
	NX_ARGB_YELLOW	= 0xffffff00,
	NX_ARGB_MAGENTA	= 0xffff00ff,
	NX_ARGB_CYAN	= 0xff00ffff,
	NX_ARGB_WHITE	= 0xffffffff,
};

struct NxDebugPoint
{
	NxVec3	p;
	NxU32	color;
};

struct NxDebugLine
{
	NxVec3	p0;
	NxVec3	p1;
	NxU32	color;
};

struct NxDebugTriangle
{
	NxVec3	p0;
	NxVec3	p1;
	NxVec3	p2;
	NxU32	color;
};

/**
This class manages lists of points, lines, and triangles.  They represent visualizations
of SDK objects to help with debugging the user's code.  They are rendered using the class
NxUserDebugRenderer, and by calling NxFoundationSDK::renderDebugData.
The user should not have to instance this class.
*/
class NxDebugRenderable
	{
	public:
	//reading:
	virtual NxU32 getNbPoints() const = 0;
	virtual const NxDebugPoint* getPoints() const = 0;

	virtual NxU32 getNbLines() const = 0;
	virtual const NxDebugLine* getLines() const = 0;

	virtual NxU32 getNbTriangles() const = 0;
	virtual const NxDebugTriangle* getTriangles() const = 0;



	//low level writing
	virtual	void clear() = 0;

	virtual void addPoint(const NxVec3& p, NxU32 color) = 0;
	virtual void addLine(const NxVec3& p0, const NxVec3& p1, NxU32 color) = 0;
	virtual void addTriangle(const NxVec3& p0, const NxVec3& p1, const NxVec3& p2, NxU32 color) = 0;


	//high level writing
	virtual void addOBB(const NxBox& box, NxU32 color=0xffffffff, bool renderFrame=false) = 0;
	virtual void addAABB(const NxBounds3& bounds, NxU32 color=0x00ffff00, bool renderFrame=false) = 0;
	virtual void addArrow(const NxVec3& position, const NxVec3& direction, NxReal length, NxReal scale, NxU32 color=0xffffffff) = 0;
	virtual void addBasis(const NxVec3& position, const NxMat33& columns, const NxVec3& lengths, NxReal scale, NxU32 colors[3] = 0) = 0;
	virtual	void addCircle(NxU32 nbSegments, const NxMat34& matrix, NxU32 color, NxF32 radius, bool semicircle = false) = 0;
	};

#endif
