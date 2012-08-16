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

#ifdef DXRENDER

#pragma once

#pragma comment(lib, "Strmiids.lib")
#define interface struct /* this should have been defined in <ObjBase.h>, but wasn't for some reason */
#include <dshow.h>
#include <Vmr9.h>

class AllocatorPresenter : public IVMRSurfaceAllocator9, public IVMRImagePresenter9
{
	friend class VideoRender;

	long refs;
	
	IVMRSurfaceAllocatorNotify9 * surfaceAllocatorNotify;
	IDirect3DTexture9 * texture; // texture of current frame
	IDirect3DSurface9 ** textureSurfaces; // surface of texture
	unsigned int textureSurfaceCount; 
	unsigned int surfaceCount, surfaceWidth, surfaceHeight;
	IDirect3DSurface9 ** surfaces; // surface buffers
	bool present;
	
	AllocatorPresenter();
	~AllocatorPresenter();

	ULONG STDMETHODCALLTYPE AddRef();
	ULONG STDMETHODCALLTYPE Release();
	HRESULT STDMETHODCALLTYPE QueryInterface(const IID & riid, void ** ppvObject);

	HRESULT STDMETHODCALLTYPE StartPresenting(DWORD_PTR dwUserID);
	HRESULT STDMETHODCALLTYPE PresentImage(DWORD_PTR dwUserID, VMR9PresentationInfo *lpPresInfo);
	HRESULT STDMETHODCALLTYPE StopPresenting(DWORD_PTR dwUserID);

	HRESULT STDMETHODCALLTYPE AdviseNotify(IVMRSurfaceAllocatorNotify9 *lpIVMRSurfAllocNotify);
	HRESULT STDMETHODCALLTYPE GetSurface(DWORD_PTR dwUserID, DWORD SurfaceIndex, DWORD SurfaceFlags, IDirect3DSurface9 **lplpSurface);
	HRESULT STDMETHODCALLTYPE InitializeDevice(DWORD_PTR dwUserID, VMR9AllocationInfo *lpAllocInfo, DWORD *lpNumBuffers);
	HRESULT STDMETHODCALLTYPE TerminateDevice(DWORD_PTR dwID);

};

class VideoRender
{
	IGraphBuilder * _graph;
    IMediaControl * _control;
    IMediaEvent   * _event;
	IBaseFilter * _vmr9;
	AllocatorPresenter * _allocatorPresenter;
	RECT _rect;

	HRESULT InitVMR();

public:
	VideoRender(LPCWSTR file);
	~VideoRender();

	void GetTextureSize(unsigned int & width, unsigned int & height);
	bool GetTexture(IDirect3DTexture9 ** texture); //returns if this is a new frame compared to last call
};




#endif