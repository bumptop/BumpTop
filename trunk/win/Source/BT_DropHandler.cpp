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

#include "BT_Common.h"
#include "BT_DropHandler.h"
#include "BT_DragDrop.h"
#include "BT_SceneManager.h"
#include "BT_Util.h"
#include "BT_WindowsOS.h"
#include "BT_StatsManager.h"

DropHandler::DropHandler()
: _refCount(1)
, _workingDirectoryDropTarget(NULL)
, _isRegistered(false)
{
	_prevDropPoint.x = _prevDropPoint.y = -1;
	_uninitializeOLE = SUCCEEDED(OleInitialize(NULL));
}

DropHandler::~DropHandler()
{
	if (_revokeOnDestroy)
		RevokeHandler();
	if (_uninitializeOLE)
		OleUninitialize();
}

HRESULT DropHandler::QueryInterface (REFIID riid, void ** ppvOut)
{
	// ensure valid out ptr
	if (!ppvOut) {
		return E_POINTER;
	}

	// return only IDataObject interface
	*ppvOut = NULL;
	if (riid == IID_IDropTarget || riid == IID_IUnknown) {
		this->AddRef();
		*ppvOut = static_cast<IDropTarget *>(this);
		return S_OK;
	}
	return E_NOINTERFACE;
}

ULONG DropHandler::AddRef (void)
{
	return InterlockedIncrement(&_refCount);
}

ULONG DropHandler::Release (void)
{
	LONG decr = InterlockedDecrement(&_refCount);
	if (decr == 0) {
		delete this;
	}
	return decr;
}

HRESULT DropHandler::DragEnter(IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect)
{
	if (DragAndDrop::IsDragInProgress())
	{
		DragAndDrop::SetDragInProgress(false);
		*pdwEffect = DROPEFFECT_NONE;
	}
	else
	{
		_workingDirectoryDropTarget->DragEnter(pDataObject, grfKeyState, pt, pdwEffect);
		DragAndDrop::SetDragInProgress(true);
		SetFocus(_dropWindow);
	}
	return S_OK;
}

HRESULT DropHandler::DragOver(DWORD grfKeyState, POINTL pt, DWORD * pdwEffect)
{
	if (DragAndDrop::IsDragInProgress())
		_workingDirectoryDropTarget->DragOver(grfKeyState, pt, pdwEffect);
	else
		*pdwEffect = DROPEFFECT_NONE;		
	return S_OK;
}

HRESULT DropHandler::DragLeave(void)
{
	if (DragAndDrop::IsDragInProgress())
	{
		_workingDirectoryDropTarget->DragLeave();
		DragAndDrop::SetDragInProgress(false);
	}
	return S_OK;
}

HRESULT DropHandler::Drop(IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect)
{
	if (DragAndDrop::IsDragInProgress())
	{
		_workingDirectoryDropTarget->Drop(pDataObject, grfKeyState, pt, pdwEffect);

		// update drop point source with the number of items being dropped
		{
			FORMATETC formatEtc = 
			{ 
				CF_HDROP,
				NULL, 
				DVASPECT_CONTENT, 
				-1, 
				TYMED_HGLOBAL
			};

			unsigned int numFiles = 1;			
			STGMEDIUM storageMedium = {0};
			if (SUCCEEDED(pDataObject->QueryGetData(&formatEtc)) &&
				SUCCEEDED(pDataObject->GetData(&formatEtc, &storageMedium)))
			{
				HDROP hDrop = (HDROP) storageMedium.hGlobal;
				DROPFILES * dropFiles = (DROPFILES *) GlobalLock(hDrop);

				// get the number of files
				numFiles = DragQueryFile(hDrop, -1, NULL, 0);
				
				// clean up
				DragFinish(hDrop);
				GlobalUnlock(hDrop);
				ReleaseStgMedium(&storageMedium);
			}

			_numPrevDrops = numFiles;
			winOS->SetLastDropPointFromDrag(numFiles);
			GetCursorPos(&_prevDropPoint);
			ScreenToClient(_dropWindow, &_prevDropPoint);
		}

		// record this drop
		statsManager->getStats().bt.interaction.dragAndDrop.fromExplorer++;
	}

	// disable d&d
	DragAndDrop::SetDragInProgress(false);

	return S_OK;
}

bool DropHandler::RegisterHandler(HWND dropWindow, bool revokeOnDtor)
{
	// ensure not already registered
	if (_isRegistered)
		return false;

	// save the IDropTarget of the shell folder of the working directory
	LOG(QString_NT("Drop registered to %1").arg(native(scnManager->getWorkingDirectory())));
	
	IShellFolder2 * shellFolder = winOS->GetShellFolderFromAbsDirPath(
		native(scnManager->getWorkingDirectory()));
	if (shellFolder)
	{
		// try and coerce the drop target interface from the shell folder
		IDropTarget * dropTarget = NULL;
		shellFolder->CreateViewObject(NULL, IID_IDropTarget, (void **) &dropTarget);
		if (dropTarget)
		{
			_workingDirectoryDropTarget = dropTarget;
		}
		shellFolder->Release();
	}

	// register and return status
	_revokeOnDestroy = revokeOnDtor;
	_dropWindow = dropWindow;
	_isRegistered = SUCCEEDED(RegisterDragDrop(_dropWindow, this));
	return _isRegistered;
}

void DropHandler::RevokeHandler()
{
	// revoke 
	if (_isRegistered)
	{
		RevokeDragDrop(_dropWindow);
		_isRegistered = false;
	}

	if (_workingDirectoryDropTarget)
	{
		LOG(QString_NT("Registering Drop Target"));
		_workingDirectoryDropTarget->Release();
	}
}

bool DropHandler::GetLastDropPoint(POINT * ptOut) 
{
	if (ptOut) {
		if (_numPrevDrops > 0)
		{
			*ptOut = _prevDropPoint;
			--_numPrevDrops;
		}
		else
		{
			// if we have already queried the positions for the last n files
			// then just use the desktop center as the new point
			ptOut->x = -1;
			ptOut->y = -1;
		}
	}
	return false;
}

void DropHandler::SetLastDropPoint(const POINT & point, unsigned int expectedCount)
{
	_prevDropPoint = point;
	_numPrevDrops += expectedCount;
}