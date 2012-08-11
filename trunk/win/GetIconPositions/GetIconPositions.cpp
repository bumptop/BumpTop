// Copyright 2012 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// grab_icon_positions.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <vector>
using namespace std;

// for concatenating qdirectories like in boost::filesystem
QDir operator/(const QDir& lhs, const QDir& rhs) 
{ 
	return QDir(lhs.absolutePath() + QDir::separator() + rhs.absolutePath()); 
}
QDir operator/(const QDir& lhs, const QString& rhs) 
{ 
	return QDir(lhs.absolutePath() + QDir::separator() + rhs); 
}
QDir& operator/=(QDir& lhs, const QDir& rhs) 
{ 
	lhs.setPath((lhs/rhs).absolutePath()); 
	return lhs;
}

// native qstring representation of a qdir/qfileinfo
QString native(const QDir& dir)
{
	return QDir::toNativeSeparators(dir.absolutePath());
}
QString native(const QFile& f)
{
	return QDir::toNativeSeparators(QFileInfo(f).absoluteFilePath());
}
QString native(const QFileInfo& p)
{
	return QDir::toNativeSeparators(p.absoluteFilePath());
}

// convenience functions to check file existance
bool exists(QString path)
{
#ifdef WIN32
	return (TRUE == PathFileExists((LPCTSTR) path.utf16()));
#else
	return QFileInfo(path).exists();
#endif
}
bool exists( QFile path )
{
	return exists(native(path));
}
bool exists( QFileInfo path )
{
	return exists(native(path));
}
bool exists( QDir path )
{
	return exists(native(path));
}

// creates a new directory with the given QDir
void create_directory(const QDir& dir)
{
	QDir().mkpath(dir.absolutePath());
}

static BOOL CALLBACK EnumProc(HWND hwnd, LPARAM lp)
{
	int maxChar = 1024;
	TCHAR buf[1024] = {0};
	if (GetClassName(hwnd, buf, maxChar))
	{
		QString className = QString::fromUtf16((const ushort *) buf);
		if (className == "Progman" ||
			className == "WorkerW")
		{
			HWND defViewHwnd = FindWindowEx(hwnd, NULL, L"SHELLDLL_DefView", NULL);
			if (defViewHwnd)
			{
				HWND listHwnd = FindWindowEx(defViewHwnd, NULL, L"SysListView32", NULL);
				if (listHwnd)
				{
					HWND * listViewHwnd = (HWND *) lp;
					*listViewHwnd = listHwnd;
					return FALSE;
				}
			}
		}
	}
	return TRUE;
}

HWND FindListView()
{
	HWND progmanHwnd = 0;
	HWND desktopViewHwnd = NULL;
	HWND listViewHwnd = NULL;

	// First find the main window of program that houses the desktop.
	vector<QString> windows;
		windows.push_back("Progman");
		windows.push_back("WorkerW");
	for (int i = 0; i < windows.size(); ++i)
	{
		progmanHwnd = FindWindow((LPCTSTR) windows[i].utf16(), NULL);
		if (progmanHwnd)
		{
			// Then get the desktop window
			desktopViewHwnd = FindWindowEx(progmanHwnd, NULL, L"SHELLDLL_DefView", NULL);

			if (desktopViewHwnd)
			{
				// Finally get the handle to the listview on the desktop.
				listViewHwnd = FindWindowEx(desktopViewHwnd, NULL, L"SysListView32", NULL);
				return listViewHwnd;
			}
		}
	}
	return 0;
}

HANDLE FindExplorerProcess(HWND slaveHwnd)
{ 
	HANDLE proc;
	DWORD explorerPid;

	// Get the PID based on a HWND. This is the good stuff. You wouldn't believe the long and difficult function I had to write before I heard of this simple API call.
	GetWindowThreadProcessId(slaveHwnd, &explorerPid);

	// Get a process handle which we need for the shared memory functions.
	proc = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION, FALSE, explorerPid);

	return proc;
}

QString GetSystemPath(int SystemLocation)
{
	SHELLEXECUTEINFO ExecInfo = {0};
	QString DesktopPath;
	LPITEMIDLIST TempPidl;
	TCHAR Path[MAX_PATH];

	// Retrieve the Desktop path
	if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, SystemLocation, &TempPidl)))
	{
		// Save the path into a String only when not executing path
		SHGetPathFromIDList(TempPidl, Path);
		DesktopPath = QString::fromUtf16((const ushort *) Path);

		// Release the PIDL
		CoTaskMemFree(TempPidl);
	}

	return DesktopPath;
}

QDir GetUserApplicationDataPath()
{
	QDir appPath = GetSystemPath(CSIDL_APPDATA);
	QDir companyPath = appPath / "Bump Technologies, Inc";
	QDir productName = companyPath / "BumpTop";

	// ensure company path exists	
	if (!exists(companyPath))
		create_directory(companyPath);

	// ensure product path exists
	if (!exists(productName))
		create_directory(productName);

	return productName;
}


int _tmain(int argc, _TCHAR* argv[])
{
	HWND listViewHwnd = 0;
	HANDLE explorer = 0;
	unsigned long iconCount = 0;
	void* iconPos = 0;
	void* iconLabel = 0;
	TCHAR* iconNameBuffer = 0;
	bool error = false;
	LRESULT result = 0;
	POINT iconPoint;
	LVITEM iconListLabel;
	TCHAR buffer[sizeof(TCHAR) * (MAX_PATH+1)];
	bool rc = false;
	
	/* for debugging
	if (argc > 1)
	{
		while (true)
		{
			if (IsDebuggerPresent())
			{
				__debugbreak();
				break;
			}

			Sleep(1000);
		}
	}
	*/

	QFile file(QFileInfo(GetUserApplicationDataPath(), "iconpositions.json").absoluteFilePath());	
	if (!file.open(QFile::WriteOnly | QFile::Truncate))
		return 1;

	QTextStream icon_positions_file(&file);
	icon_positions_file.setCodec("UTF-8");
	icon_positions_file << QString("[") << endl;

	// Get the HWND of the listview
	::EnumWindows(EnumProc, (LPARAM) &listViewHwnd);

	// Get the total number of icons on the desktop
	iconCount = (unsigned long) SendMessage(listViewHwnd, LVM_GETITEMCOUNT, 0, 0);

	// DOnt bother iterating when no icons exist
	if (iconCount > 0)
	{
		// Get the PID of the process that houses the listview, i.e.: Explorer.exe
		explorer = FindExplorerProcess(listViewHwnd);

		// Here we allocate the shared memory buffers to use in our little IPC.
		iconPos = VirtualAllocEx(explorer, NULL, sizeof(POINT), MEM_COMMIT, PAGE_READWRITE);
		iconLabel = VirtualAllocEx(explorer, NULL, sizeof(LVITEM), MEM_COMMIT, PAGE_READWRITE);
		iconNameBuffer = (TCHAR *) VirtualAllocEx(explorer, NULL, sizeof(sizeof(TCHAR) * (MAX_PATH + 1)), MEM_COMMIT, PAGE_READWRITE);


		for (unsigned int i = 0; i < iconCount; i++)
		{
			// First we get the icon position.
			result = SendMessage(listViewHwnd, LVM_GETITEMPOSITION, i, (LPARAM) iconPos);

			if (result) 
			{
				// Get the data from the shared memory
				ReadProcessMemory(explorer, iconPos, &iconPoint, sizeof(POINT), NULL);


				// Set stuff up to retrieve the label of the icon.
				iconListLabel.iSubItem = 0;
				iconListLabel.cchTextMax = MAX_PATH;
				iconListLabel.mask = LVIF_TEXT;
				iconListLabel.pszText = (LPTSTR) iconNameBuffer;


				// Request the label.
				// Write the list source
				WriteProcessMemory(explorer, iconLabel, &iconListLabel, sizeof(LVITEM), NULL);

				// this causes lots of badness on 64-bit windows. since we're trying to pass
				// pointers from a 32-bit process to a 64-bit one, something is getting messed
				// up. explorer memory usage balloons by hundreds of megabytes.
				// plus, it doesn't work anyway. -mvj
				result = SendMessage(listViewHwnd, LVM_GETITEMTEXT, i, (LPARAM) iconLabel);


			
				if (SUCCEEDED(result))
				{

					ReadProcessMemory(explorer, iconNameBuffer, &buffer, sizeof(buffer), NULL);

					icon_positions_file << "\t{" << endl;
					icon_positions_file << "\t\"name\" : \"" << QString::fromUtf16((const ushort *) buffer) << "\"," << endl;
					icon_positions_file << "\t\"xpos\" : " << iconPoint.x << "," << endl;
					icon_positions_file << "\t\"ypos\" : " << iconPoint.y << endl;
					icon_positions_file << "\t}," << endl;

					//iconBuffer->x = iconPoint.x;
					//iconBuffer->y = iconPoint.y;
					//iconBuffer->iconName = string(buffer);
					//iconBuffer->index = i;

					
				}
			}
		}
	}
	icon_positions_file << "]" << endl;
	file.close();
	return 0;
}

