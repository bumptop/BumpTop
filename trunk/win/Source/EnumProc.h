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

#pragma once

#ifndef _BT_ENUM_
#define _BT_ENUM_

// -----------------------------------------------------------------------------

class CWindowIterator 
{
protected:

	HWND* m_hwnds;		// array of hwnds for this PID
	DWORD m_nAlloc;		// size of array
	DWORD m_count;		// number of HWNDs found
	DWORD m_current;	// current HWND

	static BOOL CALLBACK EnumProc(HWND hwnd, LPARAM lp);
	virtual BOOL OnEnumProc(HWND hwnd);
	virtual BOOL OnWindow(HWND /*hwnd*/) { return TRUE; }

public:

	CWindowIterator(DWORD nAlloc=1024);
	~CWindowIterator();
	
	DWORD GetCount() { return m_count; }
	HWND First();
	HWND Next() { return m_hwnds && m_current < m_count ? m_hwnds[m_current++] : NULL; }
};

bool IsTopLevelWindow(HWND hWnd);

// -----------------------------------------------------------------------------

#else
	class CWindowIterator;
#endif
