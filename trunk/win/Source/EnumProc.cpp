// http://msdn.microsoft.com/en-us/magazine/cc301495.aspx
// MSDN Magazine sample code is licensed under Microsoft Limited Public License,
// a copy of which is available in LICENCES.txt

////////////////////////////////////////////////////////////////
// MSDN Magazine -- July 2002
// If this code works, it was written by Paul DiLascia.
// If not, I don't know who wrote it.
// Compiles with Visual Studio 6.0 and Visual Studio .NET
// Runs in Windows XP and probably Windows 2000 too.
//

#include "BT_Common.h"
#include "EnumProc.h"

CWindowIterator::CWindowIterator(DWORD nAlloc)
{
	assert(nAlloc>0);
	m_current = m_count = 0;
	m_hwnds = new HWND [nAlloc];
	m_nAlloc = nAlloc;
}

CWindowIterator::~CWindowIterator()
{
	delete [] m_hwnds;
}

//////////////////
// Move to first top-level window.
// Stuff entire array and return the first HWND.
//
HWND CWindowIterator::First()
{
	::EnumWindows(EnumProc, (LPARAM)this);
	m_current = 0;
	return Next();
}

//////////////////
// Static enumerator passes to VRTUAL fn.
//
BOOL CALLBACK CWindowIterator::EnumProc(HWND hwnd, LPARAM lp)
{
	return ((CWindowIterator*)lp)->OnEnumProc(hwnd);
}

//////////////////
// Virtual enumerator proc: add HWND to array if OnWindow is TRUE.
//
BOOL CWindowIterator::OnEnumProc(HWND hwnd)
{
	if (m_count < m_nAlloc)
		m_hwnds[m_count++] = hwnd;

	return TRUE; // keep looking
}



//////// Tells you if a window would be present in the task bar (written by BumpTop authors)

bool IsTopLevelWindow(HWND hWnd)
{
	WINDOWINFO wi = {0};

	assert(GetWindowInfo(hWnd, &wi));



	bool WS_VISIBLE_isSet = (wi.dwStyle & WS_VISIBLE) != 0;
	bool WS_EX_NOACTIVATE_isSet = (wi.dwExStyle & WS_EX_NOACTIVATE) != 0;
	bool WS_EX_TOOLWINDOW_isSet = (wi.dwExStyle & WS_EX_TOOLWINDOW) != 0;
	bool WS_EX_APPWINDOW_isSet = (wi.dwExStyle & WS_EX_APPWINDOW) != 0;
	bool WS_POPUP_isSet = (wi.dwStyle & WS_POPUP) != 0;
	bool WS_CHILD_isSet = (wi.dwStyle & WS_CHILD) != 0;


	if (!WS_VISIBLE_isSet
		|| ((WS_EX_NOACTIVATE_isSet || WS_EX_TOOLWINDOW_isSet) && !WS_EX_APPWINDOW_isSet)
		|| hWnd == 0
		|| WS_POPUP_isSet
		|| WS_CHILD_isSet
		)
	{
		return false;
	}
	else
	{
		return true;
	}

}
