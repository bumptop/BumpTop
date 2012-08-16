#ifndef NX_FOUNDATION_NXUSERDEBUGRENDERER
#define NX_FOUNDATION_NXUSERDEBUGRENDERER
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/
class NxDebugRenderable;
/**
 User Debug Renderer class.  The user should subclass this to implement
 rendering functionaltiy.
*/

class NxUserDebugRenderer
	{
	public:
	virtual void renderData(const NxDebugRenderable& data) const = 0;
	};
#endif