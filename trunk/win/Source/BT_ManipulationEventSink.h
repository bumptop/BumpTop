// Much of this code is directly from MSDN "Adding Manipulation Support in Unmanaged Code"
// http://msdn.microsoft.com/en-us/library/dd371408(VS.85).aspx
// It is licensed under Microsoft Limited Public License, 
// a copy of which is available in LICENCES.txt

#ifndef BT_MANIPULATION_EVENT_SINK
#define BT_MANIPULATION_EVENT_SINK

#include "BT_Manipulation.h"

class ManipulationEventSink : _IManipulationEvents
{
private:
	Manipulation m_manipulationData;
	
	int m_cRefCount;
	int m_cStartedEventCount;
	int m_cDeltaEventCount;
	int m_cCompletedEventCount;

	IManipulationProcessor *m_pManip;

	IConnectionPoint * m_spConnection;

public:
	ManipulationEventSink(IManipulationProcessor *manip);
	~ManipulationEventSink();

	const Manipulation& getManipulationData() const;

	//////////////////////////////
	// IManipulationEvents methods
	//////////////////////////////
	virtual HRESULT STDMETHODCALLTYPE ManipulationStarted( 
		/* [in] */ FLOAT x,
		/* [in] */ FLOAT y);

	virtual HRESULT STDMETHODCALLTYPE ManipulationDelta( 
		/* [in] */ FLOAT x,
		/* [in] */ FLOAT y,
		/* [in] */ FLOAT translationDeltaX,
		/* [in] */ FLOAT translationDeltaY,
		/* [in] */ FLOAT scaleDelta,
		/* [in] */ FLOAT expansionDelta,
		/* [in] */ FLOAT rotationDelta,
		/* [in] */ FLOAT cumulativeTranslationX,
		/* [in] */ FLOAT cumulativeTranslationY,
		/* [in] */ FLOAT cumulativeScale,
		/* [in] */ FLOAT cumulativeExpansion,
		/* [in] */ FLOAT cumulativeRotation);

	virtual HRESULT STDMETHODCALLTYPE ManipulationCompleted( 
		/* [in] */ FLOAT x,
		/* [in] */ FLOAT y,
		/* [in] */ FLOAT cumulativeTranslationX,
		/* [in] */ FLOAT cumulativeTranslationY,
		/* [in] */ FLOAT cumulativeScale,
		/* [in] */ FLOAT cumulativeExpansion,
		/* [in] */ FLOAT cumulativeRotation);

	////////////////////////////////////////////////////////////
	// IUnknown methods
	////////////////////////////////////////////////////////////
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);
	STDMETHOD(QueryInterface)(REFIID riid, LPVOID *ppvObj);
};

#endif /*BT_MANIPULATION_EVENT_SINK*/