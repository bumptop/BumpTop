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
#include "BT_Vertex.h"

#ifdef DXRENDER


void DXRender::initializeRender()
{
	PositionNormalTextured sideLessBoxVertices [] = 
	{
		PositionNormalTextured( 1, 1, 1, 0, 0, 1, 1, 0),
		PositionNormalTextured(-1, 1, 1, 0, 0, 1, 0, 0),
		PositionNormalTextured(-1,-1, 1, 0, 0, 1, 0, 1),

		PositionNormalTextured(-1,-1, 1, 0, 0, 1, 0, 1),
		PositionNormalTextured( 1,-1, 1, 0, 0, 1, 1, 1),
		PositionNormalTextured( 1, 1, 1, 0, 0, 1, 1, 0),


		PositionNormalTextured( 1,-1,-1, 0, 0,-1, 0, 1),
		PositionNormalTextured(-1,-1,-1, 0, 0,-1, 1, 1),
		PositionNormalTextured(-1, 1,-1, 0, 0,-1, 1, 0),

		PositionNormalTextured(-1, 1,-1, 0, 0,-1, 1, 0),
		PositionNormalTextured( 1, 1,-1, 0, 0,-1, 0, 0),
		PositionNormalTextured( 1,-1,-1, 0, 0,-1, 0, 1),
	};

	HRESULT hr = S_OK;
	hr = dxr->device->CreateVertexBuffer(sizeof(sideLessBoxVertices), D3DUSAGE_WRITEONLY, PositionNormalTextured::GetFlexibleVertexFormat(), D3DPOOL_DEFAULT, &_sideLessBoxVertexBuffer, NULL);
	VASSERT(D3D_OK == hr, QString_NT("Could not create sidelessBox vertex buffer: hr = %1").arg(hr));
	{
		void * _sideLessBoxVertexBufferData = NULL;
		hr = _sideLessBoxVertexBuffer->Lock(0, 0, &_sideLessBoxVertexBufferData, 0);
		VASSERT(D3D_OK == hr, QString_NT("Could not lock sidelessBox vertex buffer: hr = %1").arg(hr));
		memcpy(_sideLessBoxVertexBufferData, sideLessBoxVertices, sizeof(sideLessBoxVertices));
		hr = _sideLessBoxVertexBuffer->Unlock();
		VASSERT(D3D_OK == hr, QString_NT("Could not unlock sidelessBox vertex buffer: hr = %1").arg(hr));
		_sideLessBoxVertexBufferData = NULL;
	}

	hr = dxr->device->SetStreamSource(0, _sideLessBoxVertexBuffer, 0, sizeof(PositionNormalTextured));
	hr = dxr->device->SetFVF(PositionNormalTextured::GetFlexibleVertexFormat());


	hr = dxr->device->CreateVertexBuffer(sizeof(PositionTextured) * 4, D3DUSAGE_WRITEONLY, PositionTextured::GetFlexibleVertexFormat(), D3DPOOL_DEFAULT, &_billboardVertexBuffer, NULL);
	VASSERT(D3D_OK == hr, QString_NT("Could not create billboard vertex buffer: hr = %1").arg(hr));
	{
		PositionTextured * billboardVertexBufferData = NULL;
		hr = _billboardVertexBuffer->Lock(0, 0, (void **)&billboardVertexBufferData, 0);
		VASSERT(D3D_OK == hr, QString_NT("Could not lock billboard vertex buffer: hr = %1").arg(hr));
		billboardVertexBufferData[0] = PositionTextured(1,1,0,1,0);
		billboardVertexBufferData[1] = PositionTextured(0,1,0,0,0);
		billboardVertexBufferData[2] = PositionTextured(0,0,0,0,1);
		billboardVertexBufferData[3] = PositionTextured(1,0,0,1,1);
		hr = _billboardVertexBuffer->Unlock();
		VASSERT(D3D_OK == hr, QString_NT("Could not unlock billboard vertex buffer: hr = %1").arg(hr));
		billboardVertexBufferData = NULL;
	}

	unsigned char nullTextureData[16] = {255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255};
	nullTexture = createTextureFromData(2, 2, nullTextureData, 8, false, 1);
	VASSERT(nullTexture, QString_NT("Could not create nullTexture!"));

	// Create an instance of the DrawLine class
	hr = D3DXCreateLine(dxr->device, &_d3dLine);
	VASSERT(D3D_OK == hr, QString_NT("Could not create D3DLine object: hr = %1").arg(hr));
	_d3dLine->SetGLLines(true);
	_d3dLine->SetAntialias(false);
}

void DXRender::initializeDesktop(const Vec3 & dimension) // create the floor and walls vertices given full size
{
	if (!isDeviceReady())
		return;

	SAFE_RELEASE(_desktopVertexBuffer);
	SAFE_RELEASE(_desktopVertexDeclaration);

	HRESULT hr = S_OK;
	hr = dxr->device->CreateVertexBuffer(sizeof(PositionNormalMultiTextured) * 20, D3DUSAGE_WRITEONLY, PositionNormalMultiTextured::GetFlexibleVertexFormat(), D3DPOOL_DEFAULT, &_desktopVertexBuffer, NULL);
	VASSERT(D3D_OK == hr, QString_NT("Could not create desktop vertex buffer: hr = %1").arg(hr));

	{
		PositionNormalMultiTextured * desktopVertexBufferData = NULL;
		hr = _desktopVertexBuffer->Lock(0, 0, (void **)&desktopVertexBufferData, 0);
		VASSERT(D3D_OK == hr, QString_NT("Could not lock desktop vertex buffer: hr = %1").arg(hr));

		//floor vertices
		desktopVertexBufferData[ 0] = PositionNormalMultiTextured( dimension.x / 2, 0,-dimension.z / 2, 0, 1, 0, 0, 1, 0, 1);
		desktopVertexBufferData[ 1] = PositionNormalMultiTextured(-dimension.x / 2, 0,-dimension.z / 2, 0, 1, 0, 1, 1, 1, 1);
		desktopVertexBufferData[ 2] = PositionNormalMultiTextured(-dimension.x / 2, 0, dimension.z / 2, 0, 1, 0, 1, 0, 1, 0);
		desktopVertexBufferData[ 3] = PositionNormalMultiTextured( dimension.x / 2, 0, dimension.z / 2, 0, 1, 0, 0, 0, 0, 0);

		//front wall vertices
		desktopVertexBufferData[ 4] = PositionNormalMultiTextured( dimension.x / 2, dimension.y,	dimension.z / 2, 0, 0,-1, 0, 0, 0, 0);
		desktopVertexBufferData[ 5] = PositionNormalMultiTextured( dimension.x / 2, 0,			dimension.z / 2, 0, 0,-1, 0, 1, 0, 1);
		desktopVertexBufferData[ 6] = PositionNormalMultiTextured(-dimension.x / 2, 0,			dimension.z / 2, 0, 0,-1, 1, 1, 1, 1);
		desktopVertexBufferData[ 7] = PositionNormalMultiTextured(-dimension.x / 2, dimension.y,	dimension.z / 2, 0, 0,-1, 1, 0, 1, 0);

		//back wall vertices
		desktopVertexBufferData[ 8] = PositionNormalMultiTextured(-dimension.x / 2, dimension.y,	-dimension.z / 2, 0, 0, 1, 0, 0, 0, 0);
		desktopVertexBufferData[ 9] = PositionNormalMultiTextured(-dimension.x / 2, 0,			-dimension.z / 2, 0, 0, 1, 0, 1, 0, 1);
		desktopVertexBufferData[10] = PositionNormalMultiTextured( dimension.x / 2, 0,			-dimension.z / 2, 0, 0, 1, 1, 1, 1, 1);
		desktopVertexBufferData[11] = PositionNormalMultiTextured( dimension.x / 2, dimension.y,	-dimension.z / 2, 0, 0, 1, 1, 0, 1, 0);

		//left wall vertices
		desktopVertexBufferData[12] = PositionNormalMultiTextured( dimension.x / 2, dimension.y,	-dimension.z / 2,-1, 0, 0, 0, 0, 0, 0);
		desktopVertexBufferData[13] = PositionNormalMultiTextured( dimension.x / 2, 0,			-dimension.z / 2,-1, 0, 0, 0, 1, 0, 1);
		desktopVertexBufferData[14] = PositionNormalMultiTextured( dimension.x / 2, 0,			 dimension.z / 2,-1, 0, 0, 1, 1, 1, 1);
		desktopVertexBufferData[15] = PositionNormalMultiTextured( dimension.x / 2, dimension.y,	 dimension.z / 2,-1, 0, 0, 1, 0, 1, 0);

		//right wall vertices
		desktopVertexBufferData[16] = PositionNormalMultiTextured(-dimension.x / 2, dimension.y, dimension.z / 2, 1, 0, 0, 0, 0, 0, 0);
		desktopVertexBufferData[17] = PositionNormalMultiTextured(-dimension.x / 2, 0,			 dimension.z / 2, 1, 0, 0, 0, 1, 0, 1);
		desktopVertexBufferData[18] = PositionNormalMultiTextured(-dimension.x / 2, 0,			-dimension.z / 2, 1, 0, 0, 1, 1, 1, 1);
		desktopVertexBufferData[19] = PositionNormalMultiTextured(-dimension.x / 2, dimension.y,-dimension.z / 2, 1, 0, 0, 1, 0, 1, 0);

		hr = _desktopVertexBuffer->Unlock();
		VASSERT(D3D_OK == hr, QString_NT("Could not unlock desktop vertex buffer: hr = %1").arg(hr));
		desktopVertexBufferData = NULL;
	}

	PositionNormalMultiTextured::CreateVertexDeclaration(device, &_desktopVertexDeclaration);
}

void DXRender::renderDesktop(IDirect3DTexture9 * baseTextures [5], IDirect3DTexture9 * overlayTextures [5])
{
	dxr->device->SetTransform(D3DTS_WORLD, &dxr->identity);
	dxr->device->SetStreamSource(0, _desktopVertexBuffer, 0, sizeof(PositionNormalMultiTextured));
	dxr->device->SetVertexDeclaration(_desktopVertexDeclaration);

	dxr->device->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_BLENDTEXTUREALPHA);
	dxr->device->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
	dxr->device->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEMP); //blend to black
	
	dxr->device->SetTexture(0, baseTextures[0]);
	dxr->device->SetTexture(1, overlayTextures[0]);
	dxr->device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2); //floor

	dxr->device->SetTexture(0, baseTextures[1]);
	dxr->device->SetTexture(1, overlayTextures[1]);
	dxr->device->DrawPrimitive(D3DPT_TRIANGLEFAN, 4, 2); //front

	dxr->device->SetTexture(0, baseTextures[2]);
	dxr->device->SetTexture(1, overlayTextures[2]);
	dxr->device->DrawPrimitive(D3DPT_TRIANGLEFAN, 8, 2); //back

	dxr->device->SetTexture(0, baseTextures[4]);
	dxr->device->SetTexture(1, overlayTextures[4]);
	dxr->device->DrawPrimitive(D3DPT_TRIANGLEFAN, 12, 2); //left

	dxr->device->SetTexture(0, baseTextures[3]);
	dxr->device->SetTexture(1, overlayTextures[3]);
	dxr->device->DrawPrimitive(D3DPT_TRIANGLEFAN, 16, 2); //right

	dxr->device->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	dxr->device->SetFVF(PositionNormalTextured::GetFlexibleVertexFormat());
}

void DXRender::renderSideLessBox(const Vec3 & position, const Mat33 & orientation, const Vec3 & halfDimensions, IDirect3DTexture9 * texture)
{
	renderVertexBuffer(position, orientation, halfDimensions, texture, _sideLessBoxVertexBuffer);
}

void DXRender::renderVertexBuffer(const Vec3 & position, const Mat33 & orientation, const Vec3 & halfDimensions, IDirect3DTexture9 * texture, IDirect3DVertexBuffer9 * vertexBuffer)
{
	D3DXMATRIX world;
	orientation.getColumnMajorStride4(&world._11);

	world._11 *= halfDimensions.x;
	world._12 *= halfDimensions.x;
	world._13 *= halfDimensions.x;
	world._21 *= halfDimensions.y;
	world._22 *= halfDimensions.y;
	world._23 *= halfDimensions.y;
	world._31 *= halfDimensions.z;
	world._32 *= halfDimensions.z;
	world._33 *= halfDimensions.z;

	world._41 = position.x;
	world._42 = position.y;
	world._43 = position.z;

	world._14 = 0;
	world._24 = 0;
	world._34 = 0;
	world._44 = 1;

	HRESULT hr = S_OK;
	hr = dxr->device->SetTransform(D3DTS_WORLD, &world);
	hr = dxr->device->SetTexture(0, texture);
	hr = dxr->device->SetStreamSource(0, vertexBuffer, 0, sizeof(PositionNormalTextured));
	hr = dxr->device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 4);
}

void DXRender::renderLine(const D3DXVECTOR2* vertices, const int numVertices, const D3DCOLOR foreground, float lineWidth, float patternScale, const D3DCOLOR background)
{
	if (numVertices < 2)
		return;

	dxr->device->SetTransform(D3DTS_WORLD, &dxr->identity);
	
	_d3dLine->SetWidth(lineWidth);
	_d3dLine->SetPatternScale(patternScale);
		
	// Draw the background color line. Only necessary if the patternScale is not 0
	if (patternScale > 0.0f)
	{
		_d3dLine->SetPattern(0x55555555);
		_d3dLine->Draw(vertices, numVertices, background);
		_d3dLine->SetPattern(0xAAAAAAAA);
	}
	else
	{
		// Since there is no pattern necessary, set the pattern to all 1's
		_d3dLine->SetPattern(0xFFFFFFFF);
	}
		
	// Draw the foreground color line
	_d3dLine->Draw(vertices, numVertices, foreground);
}

void DXRender::renderLine(const QList<D3DXVECTOR2>& points, const QColor& foreground, float lineWidth, float patternScale, const QColor& background)
{
	if (points.size() < 2)
		return;
	
	// Fill the regular array with the data from the QList
	D3DXVECTOR2* vertices = new D3DXVECTOR2[points.size()];
	for (int i = 0; i < points.size(); i++)
	{	
		vertices[i] = points[i];
	}

	// Convert the colors into DirectX colors ---must be cleaner way
	D3DCOLOR foreCol = D3DCOLOR_RGBA(foreground.red(), foreground.green(), foreground.blue(), foreground.alpha());
	D3DCOLOR backCol = D3DCOLOR_RGBA(background.red(), background.green(), background.blue(), background.alpha());

	// Render the line
	renderLine(vertices, points.size(), foreCol, lineWidth, patternScale, backCol);
	delete [] vertices;
}

void DXRender::renderLine(const QList<Vec2>& points, const QColor& foreground, float lineWidth, float patternScale, const QColor& background)
{
	if (points.size() < 2)
		return;
	
	// Fill the DirectX coordinate struct with the values from the Vec2 Qlist
	D3DXVECTOR2* vertices = new D3DXVECTOR2[points.size()];
	for (int i = 0; i < points.size(); i++)
	{	
		vertices[i] = D3DXVECTOR2(points[i].x, points[i].y);
	}
	
	// Convert the colors into DirectX colors
	D3DCOLOR foreCol = D3DCOLOR_RGBA(foreground.red(), foreground.green(), foreground.blue(), foreground.alpha());
	D3DCOLOR backCol = D3DCOLOR_RGBA(background.red(), background.green(), background.blue(), background.alpha());

	// Render the line
	renderLine(vertices, points.size(), foreCol, lineWidth, patternScale, backCol);
	delete [] vertices;
}

void DXRender::beginRenderBillboard()
{
	dxr->device->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
	dxr->device->SetStreamSource(0, _billboardVertexBuffer, 0, sizeof(PositionTextured));
	dxr->device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	dxr->device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
	dxr->device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	dxr->device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
	dxr->device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	dxr->device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	dxr->device->SetRenderState(D3DRS_TEXTUREFACTOR, DWORD(D3DXCOLOR(dxr->billboardMaterial.Diffuse)));
}

void DXRender::renderBillboard(const Vec3 & offset, const Vec3 & size, const D3DCOLORVALUE & colour, const Vec3 & textureOffset, const Vec3 & textureSize)
{
	dxr->device->SetTransform(D3DTS_WORLD, &D3DXMATRIX(
		size.x,   0,        0,        0,
		0,        size.y,   0,        0,
		0,        0,        size.z,   0,
		offset.x, offset.y, offset.z, 1));

	dxr->device->SetTransform(D3DTS_TEXTURE0, &D3DXMATRIX(
		textureSize.x,   0,               0, 0,
		0,               textureSize.y,   0, 0,
		textureOffset.x, textureOffset.y, 1, 0,
		0,               0,               0, 0));
	
	D3DXCOLOR col(colour);
	
	if (col != D3DXCOLOR(dxr->billboardMaterial.Diffuse))
	{
		dxr->billboardMaterial.Diffuse = colour;
		dxr->device->SetRenderState(D3DRS_TEXTUREFACTOR, DWORD(col));
	}
	dxr->device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
}

void DXRender::endRenderBillboard()
{
	dxr->device->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
	dxr->device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	dxr->device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	dxr->device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
	dxr->device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
}

IDirect3DSurface9* DXRender::copyBackBuffer()
{
	if (!isDeviceReady())
		return NULL;

	_ASSERT(_swapBackBuffer);
	
	IDirect3DSurface9* screenSurface = NULL;
	HRESULT hr = device->CreateOffscreenPlainSurface(_presentationParameters.BackBufferWidth, _presentationParameters.BackBufferHeight, _presentationParameters.BackBufferFormat, D3DPOOL_SYSTEMMEM, &screenSurface, NULL);
	VASSERT(hr == D3D_OK, QString_NT("Could not create plain surface: hr = %1").arg(hr));
	
	hr = D3DXLoadSurfaceFromSurface(screenSurface, NULL, NULL, _swapBackBuffer, NULL, NULL, D3DX_FILTER_NONE, 0);
	VASSERT(hr == D3D_OK, QString_NT("Could not get render target data: hr = %1").arg(hr));
	
	return screenSurface;
}

#endif