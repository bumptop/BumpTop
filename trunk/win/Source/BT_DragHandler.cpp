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
#include "BT_DragHandler.h"
#include "BT_DragDrop.h"
#include "BT_WindowsOS.h"
#include "BT_SceneManager.h"
#include "BT_Util.h"
#include "BT_StatsManager.h"

DragHandler::DragHandler()
: _refCount(1)
{
	_uninitializeOLE = SUCCEEDED(OleInitialize(NULL));
}

DragHandler::~DragHandler()
{
	if (_uninitializeOLE)
		OleUninitialize();
}

HRESULT DragHandler::CreateDropSource(IDropSource ** ppDropSourceOut)
{
	// ensure valid out ptr
	if(!ppDropSourceOut)
		return E_POINTER;

	*ppDropSourceOut = new DragHandler;
	if (!*ppDropSourceOut)
		return E_OUTOFMEMORY;
	return S_OK;
}

DWORD DragHandler::DropModeToDropEffect(unsigned int dropMode)
{
	DWORD dwEffect = 0;

	// set drop effect based on drop mode
	if (dropMode & DropLink) dwEffect |= DROPEFFECT_LINK;
	if (dropMode & DropCopy) dwEffect |= DROPEFFECT_COPY;
	if (dropMode & DropMove) dwEffect |= DROPEFFECT_MOVE;

	return dwEffect;
}

HRESULT DragHandler::QueryInterface (REFIID riid, void ** ppvOut)
{
	// ensure valid out ptr
	if (!ppvOut) {
		return E_POINTER;
	}

	// return only IDataObject interface
	*ppvOut = NULL;
	if (riid == IID_IDropSource || riid == IID_IUnknown) {
		this->AddRef();
		*ppvOut = static_cast<IDropSource *>(this);
		return S_OK;
	}
	return E_NOINTERFACE;
}

ULONG DragHandler::AddRef (void)
{
	return InterlockedIncrement(&_refCount);
}

ULONG DragHandler::Release (void)
{
	LONG decr = InterlockedDecrement(&_refCount);
	if (decr == 0) {
		delete this;
	}
	return decr;
}

HRESULT DragHandler::QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState)
{
	if (!DragAndDrop::IsDragInProgress())
	{
		return DRAGDROP_S_CANCEL;
	}

	// cancel drop if Escape pressed
	if ((fEscapePressed == TRUE))
	{
		return DRAGDROP_S_CANCEL;
	}

	// drop if LMB released
	if(!(grfKeyState & (MK_LBUTTON | MK_RBUTTON)))
	{
		return DRAGDROP_S_DROP;
	}

	// continue otherwise
	return S_OK;
}

HRESULT DragHandler::GiveFeedback(DWORD dwEffect)
{
	return DRAGDROP_S_USEDEFAULTCURSORS;
}

DWORD DragHandler::InitiateDrag(const vector<QString>& fileList, unsigned int allowedDropModes)
{
	// ensure valid file list
	if (fileList.empty())
		return 0;

	// try and coerce the drag object data from explorer
	IDataObject * dataObject = NULL;
	IDropSource * dropSource = NULL;

	// grab the first parent path instead of the working directory in case
	// we are moving items from hard piles
	QDir firstFilePath(fileList.front());

	IShellFolder2 * shellFolder = winOS->GetShellFolderFromAbsDirPath(
		native(QFileInfo(firstFilePath.absolutePath()).dir()));
	if (shellFolder)
	{
		LPCITEMIDLIST * pidls = new LPCITEMIDLIST[fileList.size()];
		for (int i = 0; i < fileList.size(); ++i)
		{
			pidls[i] = winOS->GetRelativePidlFromAbsFilePath(fileList[i]);
		}
		shellFolder->GetUIObjectOf(NULL, fileList.size(), pidls, IID_IDataObject, NULL, (void **) &dataObject);

		if (!dataObject)
		{
			consoleWrite("Could not get DataObject from working directory ShellFolder of the Selection.");
		}

		// use our own drag source implementation
		DragHandler::CreateDropSource(&dropSource);

		shellFolder->Release();
	}

	// initiate drag
	DragAndDrop::SetDragInProgress(true);
	DWORD effect = 0;
	DWORD dragResult = DoDragDrop(dataObject, dropSource, 
		DropModeToDropEffect(allowedDropModes), &effect);

	// handle results
	DragAndDrop::SetDragInProgress(false);
	if (dragResult == DRAGDROP_S_DROP) {
		// do nothing at the moment
		switch (effect) {
			case DROPEFFECT_COPY:
				break;
			case DROPEFFECT_MOVE:
				break;
			case DROPEFFECT_LINK:
				break;
			default:
				break;
		};

		// record this drag
		statsManager->getStats().bt.interaction.dragAndDrop.toExplorer++;
	}

	// cleanup
	dropSource->Release();
	dataObject->Release();

	return effect;
}


