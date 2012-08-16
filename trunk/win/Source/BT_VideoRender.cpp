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

#include "BT_DXRender.h"
#include "BT_VideoRender.h"
#include "BT_RenderManager.h"

//http://msdn.microsoft.com/en-us/library/dd389098(VS.85).aspx
//http://msdn.microsoft.com/en-us/library/dd390957(VS.85).aspx
//http://msdn.microsoft.com/en-us/library/dd407299(VS.85).aspx

#ifdef DXRENDER

VideoRender::VideoRender(LPCWSTR file)
: _graph(NULL)
, _control(NULL)
, _event(NULL)
, _vmr9(NULL)
, _allocatorPresenter(NULL)
{
	// Initialize the COM library.
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr))
		return;
    
    // Create the vmr9Filter graph manager and query for interfaces.
    hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, 
                        IID_IGraphBuilder, (void **)&_graph);
    if (FAILED(hr))
        return;

    hr = _graph->QueryInterface(&_control);
    hr = _graph->QueryInterface(&_event);

    // Build the graph. IMPORTANT: Change this string to a file on your system.
    //hr = _graph->RenderFile(file, NULL);
	
	hr = InitVMR();
	_ASSERT(SUCCEEDED(hr));

	IBaseFilter * sourceFilter = NULL;
	hr = _graph->AddSourceFilter(file, QT_NT(L"src"), &sourceFilter);
	_ASSERT(SUCCEEDED(hr));
		
	ICaptureGraphBuilder2 * pBuilder = NULL;
	hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, (void **)&pBuilder);
	hr = pBuilder->SetFiltergraph(_graph);
	hr = pBuilder->RenderStream(NULL, &MEDIATYPE_Audio, sourceFilter, NULL, NULL); // connect to audio renderer
	hr = pBuilder->RenderStream(NULL, NULL, sourceFilter, NULL, _vmr9); // connect to VMR9
	SAFE_RELEASE(pBuilder);
	
	_ASSERTE(SUCCEEDED(hr) || !QT_NT("unable to construct graph"));
	if (FAILED(hr))
		return;	

	// Run the graph.
	hr = _control->Run();
	OAFilterState oaFilterState = {0};
	_control->GetState(0, &oaFilterState);
	//if (SUCCEEDED(hr))
	//{
	//	// Wait for completion.
	//	long evCode;
	//	_event->WaitForCompletion(INFINITE, &evCode);

	//	// Note: Do not use INFINITE in a real application, because it
	//	// can block indefinitely.
	//}
}

VideoRender::~VideoRender()
{
	if (_control)
		_control->Stop();
	SAFE_RELEASE(_control);
	SAFE_RELEASE(_event);
	SAFE_RELEASE(_vmr9);
	if (_graph)
	{
		long refs = _graph->Release();
		_ASSERT(0 == refs);
		_graph = NULL;
	}
	SAFE_DELETE(_allocatorPresenter);
    CoUninitialize();
}

HRESULT VideoRender::InitVMR()
{ 
    // Create the VMR. 
    HRESULT hr = CoCreateInstance(CLSID_VideoMixingRenderer9, NULL, CLSCTX_INPROC, IID_IBaseFilter, (void**)&_vmr9); 
    if (FAILED(hr))
    {
        return hr;
    }

    // Add the VMR9 to the graph.
    hr = _graph->AddFilter(_vmr9, L"VMR9"); 
	_ASSERT(SUCCEEDED(hr));
    if (FAILED(hr)) 
    {
        _vmr9->Release();
        return hr;
    }

    // Set the rendering mode.  
    IVMRFilterConfig9* pConfig; 
    hr = _vmr9->QueryInterface(&pConfig); 
    if (FAILED(hr)) 
		return hr;
    
    hr = pConfig->SetRenderingMode(VMRMode_Renderless); 
	SAFE_RELEASE(pConfig);

	_allocatorPresenter = new AllocatorPresenter();

	IVMRSurfaceAllocatorNotify9 * surfaceAllocatorNotify = NULL;
	hr = _vmr9->QueryInterface(&surfaceAllocatorNotify);
	
	surfaceAllocatorNotify->AdviseSurfaceAllocator(0, _allocatorPresenter);
	_allocatorPresenter->AdviseNotify(surfaceAllocatorNotify);

	SAFE_RELEASE(surfaceAllocatorNotify);
	return hr; 
} 

void VideoRender::GetTextureSize(unsigned int & width, unsigned int & height)
{
	if (_allocatorPresenter)
	{
		width = _allocatorPresenter->surfaceWidth;
		height = _allocatorPresenter->surfaceHeight;
	}
}

bool VideoRender::GetTexture(IDirect3DTexture9 ** texture)
{
	*texture = _allocatorPresenter->texture;
	if (_allocatorPresenter->present)
	{	
		_allocatorPresenter->present = false;
		return true;
	}
	else
		return false;
}

AllocatorPresenter::AllocatorPresenter() 
: texture(NULL)
, textureSurfaces(NULL)
, textureSurfaceCount(0)
, surfaces(NULL)
, surfaceCount(0)
, surfaceWidth(0)
, surfaceHeight(0)
, present(false)
, refs(0)
{
}

ULONG STDMETHODCALLTYPE AllocatorPresenter::AddRef()
{
	return ++refs;
}

ULONG STDMETHODCALLTYPE AllocatorPresenter::Release()
{
	return --refs;
}

HRESULT STDMETHODCALLTYPE AllocatorPresenter::QueryInterface(const IID & riid, void ** ppvObject)
{
	if (!ppvObject)
		return E_POINTER;

	if (IID_IVMRSurfaceAllocator9 == riid)
	{
		IVMRSurfaceAllocator9 * obj = dynamic_cast<IVMRSurfaceAllocator9 *>(this);
		obj->AddRef();
		*ppvObject = obj;
		return S_OK;
	}
	else if (IID_IVMRImagePresenter9 == riid)
	{
		IVMRImagePresenter9 * obj = dynamic_cast<IVMRImagePresenter9 *>(this);
		obj->AddRef();
		*ppvObject = obj;
		return S_OK;
	}
	else
	{
		*ppvObject = NULL;
		return E_NOINTERFACE;
	}
}

HRESULT STDMETHODCALLTYPE AllocatorPresenter::StartPresenting(DWORD_PTR dwUserID)
{
	return S_OK;
}

HRESULT STDMETHODCALLTYPE AllocatorPresenter::PresentImage(DWORD_PTR dwUserID, VMR9PresentationInfo * presentInfo)
{
	present = true;
	HRESULT hr = dxr->device->StretchRect(presentInfo->lpSurf, NULL, textureSurfaces[0], NULL, D3DTEXF_NONE);
	_ASSERT(SUCCEEDED(hr));
	//texture->AddDirtyRect(NULL);
	//texture->GenerateMipSubLevels();
	//hr = D3DXSaveSurfaceToFileA("surface.png", D3DXIFF_PNG, textureSurface, NULL, NULL);

	// Manual mipmap
	for (unsigned int i = 1; i < textureSurfaceCount; i++)
	{
		hr = dxr->device->StretchRect(textureSurfaces[i - 1], NULL, textureSurfaces[i], NULL, D3DTEXF_LINEAR);
		_ASSERT(SUCCEEDED(hr));
	}
	rndrManager->invalidateRenderer();
	return S_OK;
}

HRESULT STDMETHODCALLTYPE AllocatorPresenter::StopPresenting(DWORD_PTR dwUserID)
{
	return S_OK;
}

HRESULT STDMETHODCALLTYPE AllocatorPresenter::AdviseNotify(IVMRSurfaceAllocatorNotify9 *lpIVMRSurfAllocNotify)
{
	_ASSERT(dxr->device);
	if (!dxr->device)
		return E_FAIL;

	HRESULT hr = lpIVMRSurfAllocNotify->SetD3DDevice(dxr->device, dxr->d3d->GetAdapterMonitor(D3DADAPTER_DEFAULT));
	return S_OK;
}

HRESULT STDMETHODCALLTYPE AllocatorPresenter::GetSurface(DWORD_PTR dwUserID, DWORD SurfaceIndex, DWORD SurfaceFlags, IDirect3DSurface9 ** surface)
{
	if (!surface)
	{
		_ASSERT(0);
		return E_POINTER;
	}
	if (0 <= SurfaceIndex && SurfaceIndex < surfaceCount)
	{
		*surface = surfaces[SurfaceIndex];
		return S_OK;
	}
	else
	{
		//_ASSERT(0);
		return E_INVALIDARG;
	}
}

HRESULT STDMETHODCALLTYPE AllocatorPresenter::InitializeDevice(DWORD_PTR dwUserID, VMR9AllocationInfo * allocInfo, DWORD * numBuffers)
{
	HRESULT hr = S_OK;
	allocInfo->dwFlags |= VMR9AllocFlag_TextureSurface;
	surfaceCount = *numBuffers = allocInfo->MinBuffers;
	surfaces = new IDirect3DSurface9 * [surfaceCount];
	D3DFORMAT format = D3DFMT_UNKNOWN == allocInfo->Format ? D3DFMT_X8R8G8B8 : allocInfo->Format;
	char * fourcc = (char *)&format;
	allocInfo->Format = format;
	surfaceWidth = allocInfo->dwWidth;
	surfaceHeight = allocInfo->dwHeight;
	if (D3DPOOL_SYSTEMMEM < (unsigned int)allocInfo->Pool)
	{
		_ASSERTE(!QT_NT("invalid pool, set to default"));
		//allocInfo->Pool = D3DPOOL_DEFAULT;
		return E_INVALIDARG;
	}
	for (unsigned int i = 0; i < surfaceCount; i++)
	{
		surfaces[i] = NULL;
		hr = dxr->device->CreateOffscreenPlainSurface(allocInfo->dwWidth, allocInfo->dwHeight, format, allocInfo->Pool, &surfaces[i], NULL);
		_ASSERT(SUCCEEDED(hr));
	}
	
	textureSurfaceCount = 2;
	hr = dxr->device->CreateTexture(allocInfo->dwWidth, allocInfo->dwHeight, textureSurfaceCount, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &texture, NULL);
	_ASSERT(SUCCEEDED(hr));
	textureSurfaces = new IDirect3DSurface9 * [textureSurfaceCount];
	for (unsigned int i = 0; i < textureSurfaceCount; i++)
	{
		textureSurfaces[i] = NULL;
		hr = texture->GetSurfaceLevel(i, &textureSurfaces[i]);
		_ASSERT(SUCCEEDED(hr));	
	}

	return hr;
}

HRESULT STDMETHODCALLTYPE AllocatorPresenter::TerminateDevice(DWORD_PTR dwID)
{
	for (unsigned int i = 0; i < surfaceCount; i++)
		SAFE_RELEASE(surfaces[i]);
	SAFE_DELETE_ARRAY(surfaces);

	for (unsigned int i = 0; i < textureSurfaceCount; i++)
		SAFE_RELEASE(textureSurfaces[i]);
	SAFE_DELETE_ARRAY(textureSurfaces);

	SAFE_RELEASE(texture);
	return S_OK;
}

AllocatorPresenter::~AllocatorPresenter()
{
	_ASSERT(0 == refs);
}

#endif