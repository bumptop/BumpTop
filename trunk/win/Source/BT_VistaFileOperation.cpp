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

#include "BT_VistaFileOperation.h"

VistaFileOperation::VistaFileOperation() : 
	_cRef(1),
	performedChecks(false),
	totalFiles(0),
	pSHCreateItemFromParsingName(NULL),
	pSHCreateShellItemArrayFromIDLists(NULL),
	_hMod(NULL)
{
	HRESULT hr = CoInitialize(NULL);
	_comInitialized = (hr == S_OK);
	_hMod = LoadLibrary(L"Shell32.dll");
	pSHCreateItemFromParsingName = (SHCreateItemFromParsingNameSignature) GetProcAddress(_hMod, "SHCreateItemFromParsingName");
	pSHCreateShellItemArrayFromIDLists = (SHCreateShellItemArrayFromIDListsSignature) GetProcAddress(_hMod, "SHCreateShellItemArrayFromIDLists");
}

VistaFileOperation::~VistaFileOperation()
{
	FreeLibrary(_hMod);

	if (_comInitialized)
		CoUninitialize();
}

bool VistaFileOperation::basicFileOperation(FileOperationType operationType, QList<LPWSTR>& files, LPWSTR newName, LPWSTR destFolder, QList<FileOperationResult>& results)
{
	if (!pSHCreateItemFromParsingName || !pSHCreateShellItemArrayFromIDLists)
		return false;

	results.clear();
	bool succeeded = false;

	IFileOperation *pfo = NULL; 
	HRESULT hr = CoCreateInstance(CLSID_FileOperation, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pfo)); 

	if (SUCCEEDED(hr))
	{
		// Set this class to be our callback
		DWORD dwCookie = 0;
		hr = pfo->Advise(this, &dwCookie);

		if (SUCCEEDED(hr))
		{
			// Parse all our file paths into an IShellItemArray
			IShellItemArray* itemArray;
			if (createShellItemArrayFromPaths(files, &itemArray))
			{	
				// Operation specifics go here
				if (operationType == Move)
					succeeded = moveFilesOperation(pfo, itemArray, destFolder);
				else if (operationType == Copy)
					succeeded = copyFilesOperation(pfo, itemArray, destFolder);
				else if (operationType == Rename)
					succeeded = renameFilesOperation(pfo, itemArray, newName);
				else if (operationType == Delete)
					succeeded = deleteFilesOperation(pfo, itemArray);

				if (succeeded)
				{
					succeeded = false;
					hr = pfo->PerformOperations();
					if (SUCCEEDED(hr))
						succeeded = true;
					
					// Sync the local result list with the callback one.
					// If somehow the file op failed and some files didn't get processed,
					// make sure we fill them with FileOperationResults of Error.
					int resultLen = this->results.size();
					for (int i = 0; i < files.size(); i++)
					{
						if (i >= resultLen)
						{
							FileOperationResult res;
							res.action = Error;
							results.push_back(res);
						}
						else
						{
							results.push_back(this->results[i]);
						}
					}
				}
				itemArray->Release();
			}
			pfo->Unadvise(dwCookie);
		}
		pfo->Release();
	}
	return succeeded;		
}

bool VistaFileOperation::moveFilesOperation(IFileOperation* pfo, IShellItemArray* itemArray, LPWSTR destFolder)
{
	assert(pfo != NULL);
	assert(itemArray != NULL);

	IShellItem* destinationFolder;
	HRESULT hr = pSHCreateItemFromParsingName((PCWSTR) destFolder, NULL, IID_PPV_ARGS(&destinationFolder));
	if (SUCCEEDED(hr))
	{
		hr = pfo->MoveItems(itemArray, destinationFolder);
		destinationFolder->Release();
		return SUCCEEDED(hr);
	}
	return false;
}

bool VistaFileOperation::copyFilesOperation(IFileOperation* pfo, IShellItemArray* itemArray, LPWSTR destFolder)
{
	assert(pfo != NULL);
	assert(itemArray != NULL);

	IShellItem* destinationFolder;
	HRESULT hr = pSHCreateItemFromParsingName((PCWSTR) destFolder, NULL, IID_PPV_ARGS(&destinationFolder));
	if (SUCCEEDED(hr))
	{
		hr = pfo->CopyItems(itemArray, destinationFolder);
		destinationFolder->Release();
		return SUCCEEDED(hr);
	}
	return false;
}

bool VistaFileOperation::renameFilesOperation(IFileOperation* pfo, IShellItemArray* itemArray, LPWSTR newName)
{
	assert(pfo != NULL);
	assert(itemArray != NULL);

	HRESULT hr = pfo->RenameItems(itemArray, (LPCWSTR) newName);
	return SUCCEEDED(hr);
}

bool VistaFileOperation::deleteFilesOperation(IFileOperation* pfo, IShellItemArray* itemArray)
{
	assert(pfo != NULL);
	assert(itemArray != NULL);

	HRESULT hr = pfo->DeleteItems(itemArray);
	return SUCCEEDED(hr);
}

bool VistaFileOperation::createShellItemArrayFromPaths(QList<LPWSTR>& paths, IShellItemArray** shellItemArray)
{
	PCIDLIST_ABSOLUTE_ARRAY pidlArray = new LPCITEMIDLIST[paths.size()];
	for (int i = 0; i < paths.size(); i++)
	{
		LPITEMIDLIST pidl = getAbsolutePidlFromAbsFilePath(paths[i]);
		if (!pidl)
		{
			delete [] pidlArray;
			return false;						
		}
		pidlArray[i] = (LPCITEMIDLIST) pidl;
	}

	HRESULT hr = pSHCreateShellItemArrayFromIDLists(paths.size(), pidlArray, shellItemArray);
	
	delete [] pidlArray;

	if (SUCCEEDED(hr))
		return true;
	
	return false;
}

bool VistaFileOperation::moveFiles(QList<LPWSTR>& src, LPWSTR destFolder, QList<FileOperationResult>& results)
{
	return basicFileOperation(Move, src, NULL, destFolder, results);
}

bool VistaFileOperation::copyFiles(QList<LPWSTR>& src, LPWSTR destFolder, QList<FileOperationResult>& results)
{
	return basicFileOperation(Copy, src, NULL, destFolder, results);
}

bool VistaFileOperation::renameFiles(QList<LPWSTR>& files, LPWSTR newName, QList<FileOperationResult>& results)
{
	return basicFileOperation(Rename, files, newName, NULL, results);
}

bool VistaFileOperation::deleteFiles(QList<LPWSTR>& files, QList<FileOperationResult>& results)
{
	return basicFileOperation(Delete, files, NULL, NULL, results);
}

IFACEMETHODIMP VistaFileOperation::QueryInterface(REFIID riid, void **ppv)
{
	static const QITAB qit[] = 
	{
		QITABENT(VistaFileOperation, IFileOperationProgressSink),
		{0},
	};
	return QISearch(this, qit, riid, ppv);
}

IFACEMETHODIMP_(ULONG) VistaFileOperation::AddRef()
{
	return InterlockedIncrement(&_cRef);
}

IFACEMETHODIMP_(ULONG) VistaFileOperation::Release()
{
	ULONG cRef = InterlockedDecrement(&_cRef);
	if (0 == cRef)
	{
		delete this;
	}
	return cRef;
}

IFACEMETHODIMP VistaFileOperation::StartOperations()
{
	results.clear();
	resultPosition = results.begin();
	itemsWithCollisions.clear();
	performedChecks = false;
	return S_OK;
}

IFACEMETHODIMP VistaFileOperation::FinishOperations( HRESULT hrResult )
{
	return S_OK;
}

IFACEMETHODIMP VistaFileOperation::PreRenameItem( DWORD dwFlags, IShellItem *psiItem, LPCWSTR pwszNewName )
{
	return S_OK;
}

IFACEMETHODIMP VistaFileOperation::PostRenameItem( DWORD dwFlags, IShellItem *psiItem, LPCWSTR pwszNewName, HRESULT hrRename, IShellItem *psiNewlyCreated )
{
	return S_OK;
}

IFACEMETHODIMP VistaFileOperation::PreMoveItem( DWORD dwFlags, IShellItem *psiItem, IShellItem *psiDestinationFolder, LPCWSTR pwszNewName )
{
	return S_OK;
}


// The way the IFileOperationProgressSink callback works (for move operation):
// 
// First, Windows will callback the PreMoveItem function for an item and then will attempt to move that item.
// Next, it calls PostMoveItem for that same item and gives certain parameters, one being the result of the move.
// Once all the items have had their respective PreItemMove and PostMoveItem calls, Windows calls the PreMoveItem
// on the first file that had a collision. Then the FileName collision dialog comes up asking the user what action 
// he/she wants to do for the filename collision. Once the user has selected an option, the PostMoveItem is called
// for that item with a result of either moved, didn't moved, and canceled. If there was a rename, the pwszNewName
// parameter holds the new name of the file. Windows continues this for all the remaining items that had collisions.
//
// NOTE: Windows may do more file operations if it wants to. For instance, if you have an html file with a folder named
// "[name of html file]_files" in the same directory, then if you move either the folder or the file, the other one will
// automatically be moved as well. Thankfully if there is a name collision on the automatically moved item, it just stays
// where it was.
//

IFACEMETHODIMP VistaFileOperation::PostMoveItem( DWORD dwFlags, IShellItem *psiItem, IShellItem *psiDestinationFolder, LPCWSTR pwszNewName, HRESULT hrMove, IShellItem *psiNewlyCreated )
{
	// Check if we have checked all our files for collisions
	if (!performedChecks)
	{
		if (itemsWithCollisions.contains(psiItem))
		{
			// If the item we are processing is in the collision list, that
			// means we've processed it once and that we are finished our
			// first pass.
			resultPosition = results.begin();
			performedChecks = true;
		}
		else if (resultPosition == results.end())
		{
			// Make sure we have room to store this items' results
			FileOperationResult res;
			res.action = Error;
			results.push_back(res);
			if (results.size() > 1)
				resultPosition = results.end() - 1;
			else
				resultPosition = results.begin();
		}
	}
	
	if (performedChecks)
	{
		// This is our second pass, so only work on the files with
		// collisions.
		while (resultPosition != results.end() && (*resultPosition).action != Collision)
			resultPosition++;
		
		if (resultPosition == results.end())
			return S_OK;
	}

	// Deal with different results
	if (hrMove == RESULT_FILE_COLLISION)
	{
		// There was a file name collision. This method will get called again later on
		// with the result of the users selection in the Windows "replace, don't move, rename" 
		// dialog.
		(*resultPosition).action = Collision;
		itemsWithCollisions.push_back(psiItem);
	}
	else if (hrMove == RESULT_FILE_CANCELED)
	{
		(*resultPosition).action = Error;
	}
	else if (hrMove == RESULT_FILE_NOT_MOVED)
	{
		(*resultPosition).action = NotMoved;
	}
	else if (hrMove == RESULT_FILE_MOVED)
	{
		if (!performedChecks)
		{
			// The file was moved with no collision issues.
			(*resultPosition).action = Moved;
		}
		else
		{
			// There was a collision but now we have resolved it.
			LPWSTR filePath;
			psiItem->GetDisplayName(SIGDN_FILESYSPATH, &filePath);
			QFileInfo oldFile(QString::fromUtf16((const ushort *)filePath));
			QString oldFileName = oldFile.fileName();
			QString newFileName = QString::fromUtf16((const ushort *)pwszNewName);
			if (oldFileName.compare(newFileName) == 0)
			{
				// If the new file name is the same as the old, it means we replaced
				// the file.
				(*resultPosition).action = Replaced;
			}
			else
			{
				// If the new file name is different from the old file name, it means
				// a rename occurred.
				(*resultPosition).action = Renamed;
				(*resultPosition).newFileName = newFileName;
			}
			CoTaskMemFree(filePath);
		}
	}	
	
	resultPosition++;
	return S_OK;
}

IFACEMETHODIMP VistaFileOperation::PreCopyItem( DWORD dwFlags, IShellItem *psiItem, IShellItem *psiDestinationFolder, LPCWSTR pwszNewName )
{
	return S_OK;
}

IFACEMETHODIMP VistaFileOperation::PostCopyItem( DWORD dwFlags, IShellItem *psiItem, IShellItem *psiDestinationFolder, LPCWSTR pwszNewName, HRESULT hrCopy, IShellItem *psiNewlyCreated )
{
	return S_OK;
}

IFACEMETHODIMP VistaFileOperation::PreDeleteItem( DWORD dwFlags, IShellItem *psiItem )
{
	return S_OK;
}

IFACEMETHODIMP VistaFileOperation::PostDeleteItem( DWORD dwFlags, IShellItem *psiItem, HRESULT hrDelete, IShellItem *psiNewlyCreated )
{
	return S_OK;
}

IFACEMETHODIMP VistaFileOperation::PreNewItem( DWORD dwFlags, IShellItem *psiDestinationFolder, LPCWSTR pszNewName )
{
	return S_OK;
}

IFACEMETHODIMP VistaFileOperation::PostNewItem( DWORD dwFlags, IShellItem *psiDestinationFolder, LPCWSTR pszNewName, LPCWSTR pszTemplateName, DWORD dwFileAttributes, HRESULT hrNew, IShellItem *psiNewItem )
{
	return S_OK;
}

IFACEMETHODIMP VistaFileOperation::UpdateProgress( UINT iWorkTotal, UINT iWorkSoFar )
{
	return S_OK;
}

IFACEMETHODIMP VistaFileOperation::ResetTimer()
{
	return S_OK;
}

IFACEMETHODIMP VistaFileOperation::PauseTimer()
{
	return S_OK;
}

IFACEMETHODIMP VistaFileOperation::ResumeTimer()
{
	return S_OK;
}