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

#ifndef _BT_DRAG_HANDLER_
#define _BT_DRAG_HANDLER_

// -----------------------------------------------------------------------------

class DragHandler : public IDropSource
{
	LONG			_refCount;
	bool			_uninitializeOLE;

protected:

	HRESULT CreateDropSource(IDropSource ** ppDropSourceOut);
	DWORD DropModeToDropEffect(uint dropMode);

public:

	DragHandler();
	~DragHandler();

	// IUnknown
	STDMETHODIMP QueryInterface(REFIID iid, void ** ppvObject);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	// IDropSource
	STDMETHODIMP QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState);
	STDMETHODIMP GiveFeedback(DWORD dwEffect);

	DWORD InitiateDrag(const vector<QString>& fileList, uint allowedDropModes);
};

// -----------------------------------------------------------------------------

#else
	class DragHandler;
#endif