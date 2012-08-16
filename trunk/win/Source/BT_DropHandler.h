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

#pragma once

#ifndef _BT_DROP_HANDLER_
#define _BT_DROP_HANDLER_

// -----------------------------------------------------------------------------

class DropHandler : public IDropTarget
{
	LONG			_refCount;
	bool			_uninitializeOLE;

	// drop
	bool			_isRegistered;
	bool			_revokeOnDestroy;
	HWND			_dropWindow;
	POINT			_prevDropPoint;
	int				_numPrevDrops;

	// drop effect overrides
	bool			_forceLinkEffect;		// used when dragging in single set of linkable objects (drives, VRTUAL folders, etc.)
	bool			_forceCopyEffect;		// used when dragging in single set of objects from a different root

	IDropTarget *	_workingDirectoryDropTarget;

public:
	DropHandler();
	~DropHandler();

	// IUnknown
	STDMETHODIMP QueryInterface(REFIID iid, void ** ppvObject);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	// IDropTarget
	STDMETHODIMP DragEnter(IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect);
	STDMETHODIMP DragOver(DWORD grfKeyState, POINTL pt, DWORD * pdwEffect);
	STDMETHODIMP DragLeave(void);
	STDMETHODIMP Drop(IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect);

	//
	bool RegisterHandler(HWND dropWindow, bool revokeOnDtor=false);
	void RevokeHandler();
	bool GetLastDropPoint(POINT * ptOut);
	void SetLastDropPoint(const POINT & point, unsigned int expectedCount);
};

// -----------------------------------------------------------------------------

#else
	class DropHandler;
#endif