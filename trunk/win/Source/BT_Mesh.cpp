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
#include "BT_Mesh.h"
#include "BT_QtUtil.h"

#ifdef DXRENDER

Mesh::Mesh(LPCWSTR file)
: _mesh(NULL)
, _meshMaterials(NULL)
, _meshTextures(NULL)
, _meshMaterialCount(0)
{
	ID3DXBuffer * meshMaterialBuffer = NULL;
	HRESULT hr = D3DXLoadMeshFromX(file, 0, dxr->device, NULL, &meshMaterialBuffer, NULL, &_meshMaterialCount, &_mesh);
	_ASSERT(SUCCEEDED(hr));
	if (meshMaterialBuffer && _meshMaterialCount > 0)
	{
		D3DXMATERIAL* d3dxMaterials = (D3DXMATERIAL*)meshMaterialBuffer->GetBufferPointer();
		_meshMaterials = new D3DMATERIAL9[_meshMaterialCount];
		_meshTextures = new IDirect3DTexture9 * [_meshMaterialCount];
		for (unsigned long i = 0; i < _meshMaterialCount; i++)
		{
			_meshMaterials[i] = d3dxMaterials[i].MatD3D;
			if (D3DXCOLOR(0xff000000) == _meshMaterials[i].Ambient)
				_meshMaterials[i].Ambient = D3DXCOLOR(0.75,0.75,0.75,0.75);
			_meshTextures[i] = NULL;
			if (d3dxMaterials[i].pTextureFilename)
			{
				QString texturePath = native(QFileInfo(QString::fromUtf16((const ushort *)file)).dir() / QFileInfo(d3dxMaterials[i].pTextureFilename).fileName());
				hr = D3DXCreateTextureFromFileW(dxr->device, (LPCWSTR)texturePath.utf16(), &_meshTextures[i]);
				_ASSERT(SUCCEEDED(hr));
			}
		}
	}
	meshMaterialBuffer->Release();	
}

Mesh::~Mesh()
{
	SAFE_RELEASE(_mesh);
	SAFE_DELETE_ARRAY(_meshMaterials);
	if (_meshTextures)
		for (unsigned int i = 0; i < _meshMaterialCount; i++)
			SAFE_RELEASE(_meshTextures[i]);
	SAFE_DELETE_ARRAY(_meshTextures);
}

unsigned int Mesh::GetMeshMaterialCount() const
{
	return _meshMaterialCount;
}

void Mesh::SetTexture(unsigned int index, IDirect3DTexture9 * texture)
{
	_ASSERT(index < _meshMaterialCount);
	SAFE_RELEASE(_meshTextures[index]);
	_meshTextures[index] = texture;
	texture->AddRef();
}

void Mesh::Render(const Vec3 &position, const Mat33 &orientation, const Vec3 &halfDimensions)
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

	//dxr->device->SetRenderState(D3DRS_LIGHTING, true);
	//dxr->device->LightEnable(0, true);
	dxr->device->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_MIRROR);
	dxr->device->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_MIRROR);
	
	for (unsigned int i = 0; i < _meshMaterialCount; i++)
	{
		dxr->device->SetTexture(0, _meshTextures[i]);
		dxr->device->SetMaterial(&_meshMaterials[i]);
		_mesh->DrawSubset(i);
	}

	dxr->device->SetMaterial(&dxr->textureMaterial);

	dxr->device->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	dxr->device->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
}

#endif