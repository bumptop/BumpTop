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
#include "BT_Camera.h"
#include "BT_DXRender.h"
#include "BT_SceneManager.h"
#include "BT_Vertex.h"
#include "BT_Util.h"
#include "BT_WindowsOS.h"
#include "BT_MenuActionManager.h"
#include "BT_GLTextureManager.h"
#include "BT_RenderManager.h"

#ifdef DXRENDER

DXRender::DXRender()
: d3d(NULL)
, device(NULL)
, d3dEx(false)
, _devBackBuffer(NULL)
, _devDepthBuffer(NULL)
, _swapChain(NULL)
, _swapBackBuffer(NULL)
, _swapDepthBuffer(NULL)
, font(NULL)
, _d3dLine(NULL)
, _sideLessBoxVertexBuffer(NULL)
, _billboardVertexBuffer(NULL)
, _desktopVertexBuffer(NULL)
, _desktopVertexDeclaration(NULL)
, nullTexture(NULL)
, _deviceLost(false)
{
}

HRESULT DXRender::initializeD3D(PWND pwnd)
{
	// Device should only be created once
	VASSERT(NULL == d3d, QString_NT("D3D Object already exists"));
	VASSERT(NULL == device, QString_NT("Device already exists"));
	SAFE_RELEASE(device);
	SAFE_RELEASE(d3d);
	
	HRESULT hr = S_OK;
	d3dEx = false;
	
	if (NULL == (d3d = Direct3DCreate9(D3D_SDK_VERSION)))
		return E_FAIL;
	
	D3DDISPLAYMODE displayMode;
	d3d->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &displayMode);
	d3d->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps);
	
	GLOBAL(settings).maxTextureSize = min(caps.MaxTextureHeight, caps.MaxTextureWidth);
	LOG(QString_NT("Max Texture Size: %1").arg(GLOBAL(settings).maxTextureSize));

	if (caps.TextureCaps & D3DPTEXTURECAPS_POW2)
	{
		if (caps.TextureCaps & D3DPTEXTURECAPS_NONPOW2CONDITIONAL)
			LOG(QString_NT("Conditional use of non-power of two textures"));
		else
			LOG(QString_NT("Can not use non power of two textures"));
	}
	else
		LOG(QString_NT("Can use non power of two textures"));
	
	// Log what is supported and what isn't
	hr = d3d->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, displayMode.Format, D3DUSAGE_DYNAMIC, D3DRTYPE_TEXTURE, D3DFMT_A8R8G8B8);
	if (FAILED(hr))
		LOG(QString_NT("D3DFMT_A8R8G8B8 not supported"));
	
	bool windowed = true;

	// Set up the structure used to create the D3DDevice. Most parameters are
	// zeroed out. We set Windowed to TRUE, since we want to do D3D in a
	// window, and then set the SwapEffect to "discard", which is the most
	// efficient method of presenting the back buffer to the display.  And 
	// we request a back buffer format that matches the current desktop display 
	// format.
	ZeroMemory(&_dummyDevicePresentationParameters, sizeof(_dummyDevicePresentationParameters));
	
	_dummyDevicePresentationParameters.BackBufferCount = 1;
	_dummyDevicePresentationParameters.BackBufferFormat = D3DFMT_UNKNOWN;
	_dummyDevicePresentationParameters.Windowed = windowed;
	_dummyDevicePresentationParameters.Flags = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
	_dummyDevicePresentationParameters.AutoDepthStencilFormat = D3DFMT_D16;
	_dummyDevicePresentationParameters.EnableAutoDepthStencil = TRUE;
	_dummyDevicePresentationParameters.MultiSampleType = D3DMULTISAMPLE_NONE;
	_dummyDevicePresentationParameters.hDeviceWindow = pwnd;
	_dummyDevicePresentationParameters.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	_dummyDevicePresentationParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
	_dummyDevicePresentationParameters.BackBufferWidth = 1;										// These are set to 1 since the device will be a dummy placeholder
	_dummyDevicePresentationParameters.BackBufferHeight = 1;										// We want to take up as little room as possible in the video memory

	// Copy most parameters from the dummy device and modify the important ones to make
	// our actual swap chain
	_presentationParameters = _dummyDevicePresentationParameters;
	_presentationParameters.BackBufferFormat = displayMode.Format;								// Use D3DFMT_X8R8G8B8 instead of D3DFMT_UNKNOWN to test to anti-aliasing
	_presentationParameters.EnableAutoDepthStencil = FALSE;										// We need to disable the auto creation of the depth buffer so we can make our own
	_presentationParameters.Flags = (DWORD)0;													// Remove the D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL since there is no depth buffer created here
	_presentationParameters.BackBufferWidth = winOS->GetWindowWidth();
	_presentationParameters.BackBufferHeight = winOS->GetWindowHeight();
	_presentationParameters.MultiSampleQuality = 0;
	
	if (GLOBAL(settings).useAntiAliasing)
	{
		DWORD multiSampleQualityLevel = 0;
		if(SUCCEEDED(d3d->CheckDeviceMultiSampleType(caps.AdapterOrdinal, caps.DeviceType, _presentationParameters.BackBufferFormat, windowed, D3DMULTISAMPLE_2_SAMPLES, &multiSampleQualityLevel)))
		{
			_presentationParameters.MultiSampleType = D3DMULTISAMPLE_2_SAMPLES;
			LOG(QString_NT("Video card supports multisampling of 2 samples and %1 levels").arg(multiSampleQualityLevel));
		}
		else
		{
			LOG(QString_NT("Video card does not support multisampling"));
		}
	}
	
	DWORD behaviourFlags;
	if (D3DDEVCAPS_HWTRANSFORMANDLIGHT & caps.DevCaps && caps.VertexShaderVersion >= D3DVS_VERSION(1, 1))
	{
		behaviourFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING;
		LOG(QString_NT("Using hardware vertex processing"));
	}
	else
	{
		behaviourFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
		LOG(QString_NT("Using software vertex processing"));
	}
	behaviourFlags |= D3DCREATE_MULTITHREADED;							// For video playback
	behaviourFlags |= D3DCREATE_FPU_PRESERVE;							// Qt WebKit relies on doubles for layout calculations, it crashes randomly if FPU is single precision

	hr = d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, pwnd, behaviourFlags, &_dummyDevicePresentationParameters, &device);
	if (FAILED(hr))
	{
		VASSERT(0, QString_NT("Failed to create IDirect3DDevice9"));
		return hr;	
	}
		
	recreateResources();
	
	// Set up the swap chain
	onResize(winOS->GetWindowWidth(), winOS->GetWindowHeight());

	return hr;
}

DXRender::~DXRender()
{
	LOG(QString_NT("Destroying DirectX"));
	releaseResources();

	ULONG refs = 0;
	if (device)
	{
		refs = device->Release();
		LOG(QString_NT("Released d3d device references = %1").arg(refs));
		if (refs != 0)
		{
			printf(QT_NT("Released d3d device references = %u\n"), refs);
			consoleWrite(QString(QT_NT("WARNING: Direct3D device has %1 remaining references\n")).arg(refs));
		}
		device = NULL;
	}
	SAFE_RELEASE(d3d);
}

void DXRender::initializeRenderState()
{
	if (_presentationParameters.MultiSampleType != D3DMULTISAMPLE_NONE &&
		scnManager->settings.useAntiAliasing)
	{
		device->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);
		LOG(QString_NT("Using AntiAliasing"));
	}
	device->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
	device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	device->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);

	device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW); //OpenGL default front face is CCW

	device->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	device->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	device->SetFVF(PositionNormalTextured::GetFlexibleVertexFormat());

	device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
	device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
	device->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	device->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

	// texture stage / sampler 1 is only used for desktop darken overlay
	device->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	device->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	device->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
	device->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	device->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	
	ZeroMemory(&textureMaterial, sizeof(textureMaterial));
	textureMaterial.Diffuse = D3DXCOLOR(1,1,1,1);
	textureMaterial.Ambient = D3DXCOLOR(1,1,1,1);
	textureMaterial.Specular = D3DXCOLOR(0.2f,0.2f,0.2f,1);
	textureMaterial.Power = 10;
	device->SetMaterial(&textureMaterial);

	ZeroMemory(&shadowMaterial, sizeof(shadowMaterial));
	shadowMaterial.Diffuse = D3DXCOLOR(0.1f, 0.1f, 0.1f, 1);

	ZeroMemory(&billboardMaterial, sizeof(billboardMaterial));
	billboardMaterial.Diffuse = D3DXCOLOR(1,1,1,1);

	D3DXMatrixIdentity(&identity);

	HRESULT hr = D3DXCreateFont(device, 14, 0, 400, 1, false, DEFAULT_CHARSET, 
		OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Arial", &font );
	VASSERT(D3D_OK == hr, QString_NT("Font creation failed: hr = %1").arg(hr));

}

void DXRender::initializeLighting()
{
	device->SetRenderState(D3DRS_LIGHTING, true);
	device->SetRenderState(D3DRS_SPECULARENABLE, false);
	
	D3DLIGHT9 light0 = {};
	D3DXVECTOR3 direction(-0.7f, -1, -0.7f);
	light0.Direction = *D3DXVec3Normalize(&direction, &direction);
	light0.Ambient = D3DXCOLOR(0xffffffff);
	light0.Diffuse = D3DXCOLOR(1, 1, 1, 1);
	light0.Specular = D3DXCOLOR(0.1f, 0.1f, 0.1f, 1);
	light0.Type = D3DLIGHT_DIRECTIONAL;
	device->SetLight(0, &light0);
	device->LightEnable(0, true);
}

HRESULT DXRender::Present()
{
	HRESULT hr = S_OK;
	
	if (!_swapChain)
	{
		return D3DERR_DEVICELOST;	
	}
	
	hr = _swapChain->Present(NULL, NULL, NULL, NULL, NULL);
	VASSERT(D3DERR_DRIVERINTERNALERROR != hr, QString_NT("Internal Driver Error. This is bad."));
	
	if (FAILED(hr))
	{
		if (!_deviceLost)
		{
			// Our device has been lost, we need to reset it.
			LOG(QString_NT("%1: Device has become lost... hr = %2").arg(timeGetTime()).arg(hr));
			_deviceLost = true;
		}
	}

	return hr;
}

bool DXRender::shouldResetDevice()
{
	_ASSERT(device);
	HRESULT hr = device->TestCooperativeLevel();
	if (hr != D3DERR_DEVICENOTRESET)
	{
		VASSERT(D3DERR_DRIVERINTERNALERROR != hr, QString_NT("Error. Internal driver error: hr = %1").arg(hr));
		return false;
	}
	return true;
}

bool DXRender::isDeviceReady()
{
	_ASSERT(device);
	HRESULT hr = device->TestCooperativeLevel();
	if (FAILED(hr))
	{
		if (!_deviceLost)
		{
			_deviceLost = true;
			LOG(QString_NT("%1: Device is not ready/lost. hr = %2").arg(timeGetTime()).arg(hr));
		}
		return false;
	}
	else
		return true;
}

bool DXRender::tryDevice()
{
	return resetDevice() == D3D_OK;
}

HRESULT DXRender::resetDevice()
{
	HRESULT hr = S_OK;

	if (isDeviceReady())
		return D3D_OK;
	
	if (!shouldResetDevice())
		return D3DERR_DEVICELOST;
	
	texMgr->onPowerSuspend();
	menuManager->onRelease();
	scnManager->onRelease();
	hr = releaseResources();
	if (FAILED(hr))
	{
		LOG(QString_NT("Could not release resources after device was lost: hr = %1").arg(hr));
		return hr;
	}
	
	LOG(QString_NT("Started resetting the device"));
	hr = device->Reset(&_dummyDevicePresentationParameters);
	if (FAILED(hr))
	{
		LOG(QString_NT("Could not reset the device: hr = %1").arg(hr));
		if (hr == D3DERR_INVALIDCALL)
		{
			device->AddRef();
			ULONG ref = device->Release();
			LOG(QString_NT("Ref Count: %1").arg(ref));
		}
		return hr;
	}
	LOG(QString_NT("Finished resetting the device"));
	
	LOG(QString_NT("Start recreating resources"));
	hr = recreateResources();
	if (FAILED(hr))
	{
		LOG(QString_NT("Could not recreate resources after device was lost: hr = %1").arg(hr));
		return hr;
	}
	LOG(QString_NT("Finish recreating resources"));

	texMgr->onPowerResume();
	
	ResizeWallsToWorkArea(winOS->GetWindowWidth(), winOS->GetWindowHeight());

	// Recreate the swap chain
	hr = onResize(winOS->GetWindowWidth(), winOS->GetWindowHeight());

	if (SUCCEEDED(hr))
	{
		LOG(QString_NT("%1: Device reset successfully").arg(timeGetTime()));
		_deviceLost = false;
	}
	return hr;
}

HRESULT DXRender::onResize(unsigned int width, unsigned int height)
{
	if (!isDeviceReady())
		return D3DERR_DEVICELOST;

	LOG(QString_NT("Start resize"));
	HRESULT hr = S_OK;
	
	_presentationParameters.BackBufferWidth = width;
	_presentationParameters.BackBufferHeight = height;
	
	D3DXMatrixPerspectiveFovRH(&proj, CAMERA_FOVY * (PI/180), float(_presentationParameters.BackBufferWidth) / float(_presentationParameters.BackBufferHeight), GLOBAL(nearClippingPlane), GLOBAL(farClippingPlane));
	device->SetTransform(D3DTS_PROJECTION, &proj);
	
	// update the camera once (important especially on first load where other
	// code like window2world depend on having a proper view matrix set up);
	updateCamera(cam->getEye(), cam->getDir(), cam->getUp());

	// Set the device to use the device's default buffers. This releases
	// DirectX's reference to our swap chain
	device->SetRenderTarget(0, _devBackBuffer);
	device->SetDepthStencilSurface(_devDepthBuffer);
	
	// Release the swap chain
	SAFE_RELEASE(_swapBackBuffer);
	SAFE_RELEASE(_swapDepthBuffer);
	SAFE_RELEASE(_swapChain);
	
	// Create the newly sized swap chain
	hr = device->CreateAdditionalSwapChain(&_presentationParameters, &_swapChain);
	if (FAILED(hr) || !_swapChain)
	{
		LOG(QString_NT("Failed to create additional swap chain: hr = %1").arg(hr));
		return hr;
	}

	_swapChain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &_swapBackBuffer);
		
	// Create the additional depth buffer
	hr = device->CreateDepthStencilSurface(_presentationParameters.BackBufferWidth, 
										   _presentationParameters.BackBufferHeight,
										   D3DFMT_D16,
										   _presentationParameters.MultiSampleType,
										   _presentationParameters.MultiSampleQuality,
										   true, &_swapDepthBuffer, NULL);
	if (FAILED(hr) || !_swapDepthBuffer)
	{
		LOG(QString_NT("Failed to create additional depth buffer: hr = %1").arg(hr));
		return hr;
	}

	device->SetRenderTarget(0, _swapBackBuffer);
	device->SetDepthStencilSurface(_swapDepthBuffer);

	// Update the cached viewport
	device->GetViewport(&viewport);

	// Move the origin to the bottom left
	D3DXMatrixTranslation(&orthoView, -float(viewport.Width) / 2 - 0.5f, -float(viewport.Height) / 2 + 0.5f, 0);
	
	// Set up the orthographic projection matrix
	D3DXMatrixOrthoLH(&orthoProj, viewport.Width, viewport.Height, 0, 1);

	rndrManager->invalidateRenderer();

	LOG(QString_NT("Finish resize"));
	return hr;
}

HRESULT DXRender::releaseResources()
{
	SAFE_RELEASE(font);
	SAFE_RELEASE(_d3dLine);
	SAFE_RELEASE(nullTexture);
	SAFE_RELEASE(_billboardVertexBuffer);
	SAFE_RELEASE(_desktopVertexBuffer);
	SAFE_RELEASE(_sideLessBoxVertexBuffer);
	SAFE_RELEASE(_desktopVertexDeclaration);
	SAFE_RELEASE(_devBackBuffer);
	SAFE_RELEASE(_devDepthBuffer);
	SAFE_RELEASE(_swapBackBuffer);
	SAFE_RELEASE(_swapDepthBuffer);
	SAFE_RELEASE(_swapChain);
	
	return S_OK;
}

HRESULT DXRender::recreateResources()
{
	device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &_devBackBuffer);
	device->GetDepthStencilSurface(&_devDepthBuffer);
	
	initializeRenderState();
	initializeLighting();
	initializeRender();

	return S_OK;
}

void DXRender::updateCamera(const Vec3 & camPos, const Vec3 & camTgt, const Vec3 & camUp)
{
	this->camPos = (const D3DXVECTOR3 &)camPos;
	this->camTgt = (const D3DXVECTOR3 &)(camTgt + camPos);
	this->camUp = (const D3DXVECTOR3 &)camUp;

	D3DXMatrixLookAtRH(&view, (const D3DXVECTOR3 *)&camPos, (const D3DXVECTOR3 *)&(camTgt + camPos), (const D3DXVECTOR3 *)&camUp);
	device->SetTransform(D3DTS_VIEW, &view);
	
	D3DXMatrixMultiply(&viewProj, &view, &proj);
	D3DXMatrixInverse(&viewProjInv, NULL, &viewProj);
}

void DXRender::switchToPerspective()
{
	device->SetTransform(D3DTS_VIEW, &view);
	device->SetTransform(D3DTS_PROJECTION, &proj);
	device->SetFVF(PositionNormalTextured::GetFlexibleVertexFormat());

	dxr->device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
	dxr->device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	dxr->device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
	dxr->device->SetSamplerState(0, D3DSAMP_MAXANISOTROPY, NxMath::min(4, caps.MaxAnisotropy));
	dxr->device->SetRenderState(D3DRS_ZENABLE, true);
	dxr->device->SetRenderState(D3DRS_LIGHTING, true);
	dxr->device->SetRenderState(D3DRS_AMBIENT, 0);
	dxr->device->LightEnable(0, true);
	dxr->device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	dxr->device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

	dxr->textureMaterial.Diffuse = D3DXCOLOR(0xffffffff);
	dxr->device->SetMaterial(&dxr->textureMaterial);
}

void DXRender::switchToOrtho()
{
	// H = Viewport Height (In Pixels)
	// W = Viewport Width (In Pixels)
	//
	// (0, H)               (W, H)
	//   +--------------------+
	//   |                    |
	//   |                    |
	//   |       SCREEN       |
	//   |                    |
	//   |                    |
	//   +--------------------+
	// (0, 0)               (W, 0)
	device->SetTransform(D3DTS_VIEW, &orthoView);
	device->SetTransform(D3DTS_PROJECTION, &orthoProj);
	device->SetFVF(PositionTextured::GetFlexibleVertexFormat());

	dxr->device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	dxr->device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	dxr->device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
	dxr->device->SetRenderState(D3DRS_ZENABLE, false);
	dxr->device->SetRenderState(D3DRS_AMBIENT, D3DXCOLOR(0xffffffff));
	dxr->device->LightEnable(0, false);
	dxr->device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	dxr->device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
}

void DXRender::window2world(float x, float y, Vec3 & nearPoint, Vec3 & farPoint)
{
	/*D3DXVECTOR3 s(x, y, 0);
	D3DXVec3Unproject((D3DXVECTOR3 *)&nearPoint, &s, &viewport, &proj, &view, &identity);

	s.z = 1;
	D3DXVec3Unproject((D3DXVECTOR3 *)&farPoint, &s, &viewport, &proj, &view, &identity);*/

	//-------
	
	D3DXVECTOR3 * n = (D3DXVECTOR3 *)&nearPoint, * f = (D3DXVECTOR3 *)&farPoint;
	n->x = f->x = (x / viewport.Width) * 2 - 1;
	n->y = f->y =  -((y / viewport.Height) * 2 - 1);
	n->z = 0;
	f->z = 1;

	D3DXVec3TransformCoord(n, n, &viewProjInv);
	D3DXVec3TransformCoord(f, f, &viewProjInv);
}

void DXRender::world2window(const Vec3& point, int & x, int & y)
{
	D3DXVECTOR3 window;
	D3DXVec3TransformCoord(&window, (const D3DXVECTOR3 *)&point, &viewProj);
	x = (window.x + 1) / 2 * viewport.Width + viewport.X;
	y = (-window.y + 1) / 2 * viewport.Height + viewport.Y;
}

IDirect3DTexture9 * DXRender::createTextureFromData(unsigned int width, unsigned int height, const unsigned char * data, unsigned int sourcePitch, bool flip, unsigned int mipmapLevels)
{
	IDirect3DTexture9 * texture = NULL;
	
	if (!isDeviceReady())
		return texture;

	if (caps.TextureCaps & D3DPTEXTURECAPS_POW2)
		VASSERT(isPowerOfTwo(width) && isPowerOfTwo(height), QString_NT("Texture not POW2: %1, %2").arg(width).arg(height));

	HRESULT hr = dxr->device->CreateTexture(width, height, mipmapLevels, D3DUSAGE_AUTOGENMIPMAP | D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &texture, NULL);
	if (hr != D3D_OK)
	{
		VASSERT(D3D_OK == hr, QString_NT("Failed to create texture: hr = %1, width = %2, height = %3, mipmapLevels = %4").arg(hr).arg(width).arg(height).arg(mipmapLevels));
		return NULL;
	}
	
	D3DLOCKED_RECT lockedRect = {0};
	hr = texture->LockRect(0, &lockedRect, NULL, 0);
	VASSERT(D3D_OK == hr, QString_NT("Could not lock texture: hr = %1").arg(hr));

	if (width * 4 == lockedRect.Pitch && lockedRect.Pitch == sourcePitch && !flip)
		memcpy(lockedRect.pBits, data, width * height * 4);
	else if (flip)
	{
		for (unsigned int i = 0; i < height; i++)
			memcpy((char *)lockedRect.pBits + (height - 1 - i) * lockedRect.Pitch, data + sourcePitch * i, sourcePitch);
	}
	else
	{
		for (unsigned int i = 0; i < height; i++)
			memcpy((char *)lockedRect.pBits + i * lockedRect.Pitch, data + sourcePitch * i, sourcePitch);
	}

	hr = texture->UnlockRect(0);
	VASSERT(D3D_OK == hr, QString_NT("Could not unlock texture: hr = %1").arg(hr));

	return texture;
}

unsigned int DXRender::getBackBufferWidth()
{
	return _presentationParameters.BackBufferWidth;
}

unsigned int DXRender::getBackBufferHeight()
{
	return _presentationParameters.BackBufferHeight;
}

#endif