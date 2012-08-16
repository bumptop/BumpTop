// Much of this code is directly from MSDN "Adding Manipulation Support in Unmanaged Code"
// http://msdn.microsoft.com/en-us/library/dd371408(VS.85).aspx
// It is licensed under Microsoft Limited Public License, 
// a copy of which is available in LICENCES.txt

#include "BT_Common.h"
#include "BT_ManipulationEventSink.h"
#include "BT_Util.h"

// Construct and initialize the event sink.
// This is mostly verbatim from the MSDN article
ManipulationEventSink::ManipulationEventSink(IManipulationProcessor *manip)
{
	// Set initial ref count to 1. Required for managing COM object lifetime.
	m_cRefCount = 1;

	m_pManip = manip;

	m_cStartedEventCount = 0;
	m_cDeltaEventCount = 0;
	m_cCompletedEventCount = 0;
	m_spConnection = NULL;

	HRESULT hr = S_OK;

	// Get the container with the connection points.
	IConnectionPointContainer * spConnectionContainer = NULL;
	hr = manip->QueryInterface(&spConnectionContainer);
	if (spConnectionContainer == NULL){
		// something went wrong, try to gracefully quit
	}

	// Get a connection point.
	hr = spConnectionContainer->FindConnectionPoint(__uuidof(_IManipulationEvents), &m_spConnection);
	if (m_spConnection == NULL){
		// something went wrong, try to gracefully quit
	}

	DWORD dwCookie;

	// Advise.
	hr = m_spConnection->Advise(this, &dwCookie);

	SAFE_RELEASE(spConnectionContainer);
}

ManipulationEventSink::~ManipulationEventSink()
{
	//Cleanup.
	SAFE_RELEASE(m_spConnection);
}

///////////////////////////////////
//Implement IManipulationEvents
///////////////////////////////////

HRESULT STDMETHODCALLTYPE ManipulationEventSink::ManipulationStarted( 
	/* [in] */ FLOAT x,
	/* [in] */ FLOAT y)
{
	m_cStartedEventCount ++;

	//consoleWrite(QString("Manipulation started!\n"));

	m_manipulationData.x = x;
	m_manipulationData.y = y;

	return S_OK;
}

HRESULT STDMETHODCALLTYPE ManipulationEventSink::ManipulationDelta( 
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
	/* [in] */ FLOAT cumulativeRotation)
{
	m_cDeltaEventCount ++;

	// place your code handler here to do any operations based on the manipulation
	//consoleWrite(QString("x=%1 y=%2 translDelta=(%3,%4) scaleD=%5 expD=%6 rotD=%7\n")
	//	.arg(x)
	//	.arg(y)
	//	.arg(cumulativeTranslationX)
	//	.arg(cumulativeTranslationY)
	//	.arg(cumulativeScale)
	//	.arg(cumulativeExpansion)
	//	.arg(cumulativeRotation * (180.0f / 3.141592f)));

	m_manipulationData.cumulativeExpansion = cumulativeExpansion;
	m_manipulationData.cumulativeRotation = cumulativeRotation;
	m_manipulationData.cumulativeScale = cumulativeScale;
	m_manipulationData.cumulativeTranslationX = cumulativeTranslationX;
	m_manipulationData.cumulativeTranslationY = cumulativeTranslationY;
	m_manipulationData.expansionDelta = expansionDelta;
	m_manipulationData.rotationDelta = rotationDelta;
	m_manipulationData.scaleDelta = scaleDelta;
	m_manipulationData.translationDeltaX = translationDeltaX;
	m_manipulationData.translationDeltaY = translationDeltaY;
	m_manipulationData.x = x;
	m_manipulationData.y = y;

	return S_OK;
}

HRESULT STDMETHODCALLTYPE ManipulationEventSink::ManipulationCompleted( 
	/* [in] */ FLOAT x,
	/* [in] */ FLOAT y,
	/* [in] */ FLOAT cumulativeTranslationX,
	/* [in] */ FLOAT cumulativeTranslationY,
	/* [in] */ FLOAT cumulativeScale,
	/* [in] */ FLOAT cumulativeExpansion,
	/* [in] */ FLOAT cumulativeRotation)
{
	m_cCompletedEventCount ++;

	m_manipulationData.cumulativeExpansion = cumulativeExpansion;
	m_manipulationData.cumulativeRotation = cumulativeRotation;
	m_manipulationData.cumulativeScale = cumulativeScale;
	m_manipulationData.cumulativeTranslationX = cumulativeTranslationX;
	m_manipulationData.cumulativeTranslationY = cumulativeTranslationY;
	m_manipulationData.x = x;
	m_manipulationData.y = y;

	//consoleWrite(QString("\nManipulation completed!\n"));

	return S_OK;
}


/////////////////////////////////
//Implement IUnknown
/////////////////////////////////

// Everything below here is verbatim from the MSDN article

ULONG ManipulationEventSink::AddRef(void) 
{
	return ++m_cRefCount;
}

ULONG ManipulationEventSink::Release(void)
{ 
	m_cRefCount --;

	if(0 == m_cRefCount) {
		delete this;
		return 0;
	}

	return m_cRefCount;
}

HRESULT ManipulationEventSink::QueryInterface(REFIID riid, LPVOID *ppvObj) 
{
	if (IID__IManipulationEvents == riid) {
		*ppvObj = (_IManipulationEvents *)(this); AddRef(); return S_OK;
	} else if (IID_IUnknown == riid) {
		*ppvObj = (IUnknown *)(this); AddRef(); return S_OK;
	} else {
		return E_NOINTERFACE;
	}
}

const Manipulation& ManipulationEventSink::getManipulationData() const
{
	return m_manipulationData;
}
