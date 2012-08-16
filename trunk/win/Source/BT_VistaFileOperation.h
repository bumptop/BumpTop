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

#ifndef BT_IFILE_OPERATION_PROGRESS_SINK
#define BT_IFILE_OPERATION_PROGRESS_SINK

#pragma warning(disable : 4995)

#define WIN32_LEAN_AND_MEAN
#include <propsys.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <ShObjIdl.h>
#include "Qt/qstring.h"
#include "Qt/qvector.h"
#include "Qt/qfile.h"
#include "Qt/qfileinfo.h"
#include "Qt/qdir.h"
#include <assert.h>
#include "BT_WindowsHelper.h"

// Found these by inspection. Can't seem to find any system defined flags
#define RESULT_FILE_COLLISION 0x27000B
#define RESULT_FILE_MOVED 0x270008
#define RESULT_FILE_NOT_MOVED 0x270005
#define RESULT_FILE_CANCELED -0x7FD90000

enum FileOperationType
{
	Move,
	Copy,
	Delete,
	Rename
};

class VistaFileOperation : public IFileOperationProgressSink
{
private:
	bool _comInitialized;
	long   _cRef;
	HMODULE _hMod;
	SHCreateItemFromParsingNameSignature pSHCreateItemFromParsingName;
	SHCreateShellItemArrayFromIDListsSignature pSHCreateShellItemArrayFromIDLists;

	bool performedChecks;
	int totalFiles;
	QList<IShellItem*> itemsWithCollisions;
	QList<FileOperationResult> results;
	QList<FileOperationResult>::iterator resultPosition;

	// Private methods
	bool createShellItemArrayFromPaths(QList<LPWSTR>& paths, IShellItemArray** shellItemArray);
	bool basicFileOperation(FileOperationType operationType, QList<LPWSTR>& files, LPWSTR newName, LPWSTR destFolder, QList<FileOperationResult>& results);
	bool moveFilesOperation(IFileOperation* pfo, IShellItemArray* itemArray, LPWSTR destFolder);
	bool copyFilesOperation(IFileOperation* pfo, IShellItemArray* itemArray, LPWSTR destFolder);
	bool renameFilesOperation(IFileOperation* pfo, IShellItemArray* itemArray, LPWSTR newName);
	bool deleteFilesOperation(IFileOperation* pfo, IShellItemArray* itemArray);
	
public:
	VistaFileOperation();
	~VistaFileOperation();
	
	// File Operation methods
	bool moveFiles(QList<LPWSTR>& src, LPWSTR destFolder, QList<FileOperationResult>& results);
	bool copyFiles(QList<LPWSTR>& src, LPWSTR destFolder, QList<FileOperationResult>& results);
	bool renameFiles(QList<LPWSTR>& files, LPWSTR newName, QList<FileOperationResult>& results);
	bool deleteFiles(QList<LPWSTR>& files, QList<FileOperationResult>& results);
	
	// IUnknown
	IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv);
	IFACEMETHODIMP_(ULONG) AddRef();
	IFACEMETHODIMP_(ULONG) Release();

	// IFileOperationProgressSink
	IFACEMETHODIMP StartOperations();
	IFACEMETHODIMP FinishOperations(HRESULT hrResult);
	IFACEMETHODIMP PreRenameItem(DWORD dwFlags, IShellItem *psiItem, LPCWSTR pwszNewName);
	IFACEMETHODIMP PostRenameItem(DWORD dwFlags, IShellItem *psiItem, LPCWSTR pwszNewName, HRESULT hrRename, IShellItem *psiNewlyCreated);
	IFACEMETHODIMP PreMoveItem(DWORD dwFlags, IShellItem *psiItem, IShellItem *psiDestinationFolder, LPCWSTR pwszNewName);
	IFACEMETHODIMP PostMoveItem(DWORD dwFlags, IShellItem *psiItem,	IShellItem *psiDestinationFolder, LPCWSTR pwszNewName, HRESULT hrMove, IShellItem *psiNewlyCreated);
	IFACEMETHODIMP PreCopyItem(DWORD dwFlags, IShellItem *psiItem, IShellItem *psiDestinationFolder, LPCWSTR pwszNewName);
	IFACEMETHODIMP PostCopyItem(DWORD dwFlags, IShellItem *psiItem,	IShellItem *psiDestinationFolder, LPCWSTR pwszNewName, HRESULT hrCopy, IShellItem *psiNewlyCreated);
	IFACEMETHODIMP PreDeleteItem(DWORD dwFlags, IShellItem *psiItem);
	IFACEMETHODIMP PostDeleteItem(DWORD dwFlags, IShellItem *psiItem, HRESULT hrDelete, IShellItem *psiNewlyCreated);
	IFACEMETHODIMP PreNewItem(DWORD dwFlags, IShellItem *psiDestinationFolder, LPCWSTR pszNewName); 
	IFACEMETHODIMP PostNewItem(DWORD dwFlags, IShellItem *psiDestinationFolder, LPCWSTR pszNewName, LPCWSTR pszTemplateName, DWORD dwFileAttributes, HRESULT hrNew, IShellItem *psiNewItem);
	IFACEMETHODIMP UpdateProgress(UINT iWorkTotal, UINT iWorkSoFar);
	IFACEMETHODIMP ResetTimer();
	IFACEMETHODIMP PauseTimer();
	IFACEMETHODIMP ResumeTimer();
};

#endif // BT_IFILE_OPERATION_PROGRESS_SINK