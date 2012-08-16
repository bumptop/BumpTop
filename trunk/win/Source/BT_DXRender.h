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

#ifdef DXRENDER

/* TODO DXR:
multitouch gesture
compressed texture
handle non-default display adapter
d3dx9 dll redist
marking menu line drawing
multiple mice
photocrop gesture
slideshow::render
print mode clear color
multiple desktop demo
*/
class DXRender
{
	friend class Singleton<DXRender>;

public:
	IDirect3D9 * d3d;
	IDirect3DDevice9 * device;
	bool d3dEx;

	ID3DXFont * font;

	D3DCAPS9 caps;
	
	D3DVIEWPORT9 viewport;

	D3DXMATRIX view, proj, identity, viewProj, viewProjInv, orthoProj, orthoView;
	D3DXVECTOR3 camPos, camTgt, camUp; //cached from updateCamera function

	D3DMATERIAL9 textureMaterial, shadowMaterial, billboardMaterial;
	IDirect3DTexture9* nullTexture;
	
private:
	// Handles to the main frame buffer
	IDirect3DSurface9* _devBackBuffer;
	IDirect3DSurface9* _devDepthBuffer;

	// Handles to the additional swap buffer
	// We use this additional buffer as our main one because we
	// can destroy this one and create a different sized one without resetting the device
	IDirect3DSwapChain9* _swapChain;
	IDirect3DSurface9* _swapBackBuffer;
	IDirect3DSurface9* _swapDepthBuffer;
	
	// Presentation parameters
	D3DPRESENT_PARAMETERS _presentationParameters;
	D3DPRESENT_PARAMETERS _dummyDevicePresentationParameters;
	
	// Line drawing class interface
	ID3DXLine* _d3dLine;
	
	IDirect3DVertexBuffer9 * _sideLessBoxVertexBuffer; //4 triangle list
	IDirect3DVertexBuffer9 * _billboardVertexBuffer; //2 triangle fan

	IDirect3DVertexBuffer9 * _desktopVertexBuffer;
	IDirect3DVertexDeclaration9 * _desktopVertexDeclaration;

	bool _deviceLost;

	DXRender();
	
	HRESULT resetDevice();
	HRESULT releaseResources();
	HRESULT recreateResources();

	void initializeRenderState();
	void initializeLighting();
	void initializeRender();

public:
	~DXRender();
	HRESULT initializeD3D(PWND pwnd);

	void initializeDesktop(const Vec3 & dimension);

	HRESULT Present();
	HRESULT onResize(unsigned int width, unsigned int height);
	bool tryDevice();
	bool shouldResetDevice();
	bool isDeviceReady();

	void updateCamera(const Vec3 & camPos, const Vec3 & camTgt, const Vec3 & camUp);
	void switchToPerspective();
	void switchToOrtho();
	void window2world(float x, float y, Vec3 & v, Vec3 & w);
	void world2window(const Vec3& point, int & x, int & y);

	IDirect3DTexture9 * createTextureFromData(unsigned int width, unsigned int height, const unsigned char * data, unsigned int sourcePitch, bool flip = false, unsigned int mipmapLevels = 0);

	void renderDesktop(IDirect3DTexture9 * baseTextures [5], IDirect3DTexture9 * overlayTextures [5]);
	void renderSideLessBox(const Vec3 & position, const Mat33 & orientation, const Vec3 & halfDimensions, IDirect3DTexture9 * texture);
	void renderVertexBuffer(const Vec3 & position, const Mat33 & orientation, const Vec3 & halfDimensions, IDirect3DTexture9 * texture, IDirect3DVertexBuffer9 * vertexBuffer);
	
	void renderLine(const D3DXVECTOR2* vertices, const int numVertices, const D3DCOLOR foreground, float lineWidth = 1.8f, float patternScale = 0.0f, const D3DCOLOR background = 0x000000ff);
	void renderLine(const QList<D3DXVECTOR2>& points, const QColor& foreground, float lineWidth = 1.8f, float patternScale = 0.0f, const QColor& background = QColor());
	void renderLine(const QList<Vec2>& points, const QColor& foreground, float lineWidth = 1.8f, float patternScale = 0.0f, const QColor& background = QColor());
	
	void beginRenderBillboard();
	void renderBillboard(const Vec3 & offset, const Vec3 & size, const D3DCOLORVALUE & colour, const Vec3 & textureOffset = Vec3(0,0,0), const Vec3 & textureSize = Vec3(1,1,0));
	void endRenderBillboard();
	
	unsigned int getBackBufferWidth();
	unsigned int getBackBufferHeight();
	
	IDirect3DSurface9* copyBackBuffer();
};

#define dxr Singleton<DXRender>::getSharedInstance()
#endif