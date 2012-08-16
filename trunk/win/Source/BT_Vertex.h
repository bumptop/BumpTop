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

#include <d3dx9core.h>

static int GetVertexElementOffset(D3DVERTEXELEMENT9 * declaration, unsigned int usage, unsigned int index)//returns -1 if usage not in declaration
{
	for(int i=0; ; i++)
	{
		//{0xFF,0,D3DDECLTYPE_UNUSED, 0,0,0}
		if (declaration[i].Stream == 0xff)
			if (declaration[i].Offset == 0)
				if (declaration[i].Type == D3DDECLTYPE_UNUSED)
					if (declaration[i].Method == 0)
						if (declaration[i].Usage == 0)
							if (declaration[i].UsageIndex == 0)
								return -1;

		if(declaration[i].Usage == usage)
			if(declaration[i].UsageIndex == index)
				return declaration[i].Offset;
	}
	return -1;
}

struct PositionColoured
{
	D3DXVECTOR3 position;
	D3DCOLOR colour;

	PositionColoured() : position(0,0,0), colour(0){}
	PositionColoured(float x, float y, float z, D3DCOLOR Color) : position(x,y,z), colour(Color) {}
	PositionColoured(const D3DXVECTOR3 & Position, const D3DCOLOR & Colour) : position(Position), colour(Colour) {}
	inline static unsigned int GetFlexibleVertexFormat()
	{
		return D3DFVF_XYZ | D3DFVF_DIFFUSE;
	}

	inline static void CreateVertexDeclaration(IDirect3DDevice9 * device, IDirect3DVertexDeclaration9 ** vertexDeclaration)
	{
		D3DVERTEXELEMENT9 elements [] = 
		{
			{ 0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
			{ 0, 12, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
			D3DDECL_END()
		};
		device->CreateVertexDeclaration(elements, vertexDeclaration );
	}
};

class PositionTextured
{
public:
	D3DXVECTOR3 position;
	D3DXVECTOR2 texture;

	PositionTextured() : position(0,0,0), texture(0,0){}
	PositionTextured(float x, float y, float z, float u, float v) : position(x,y,z), texture(u,v) {}
	PositionTextured(const D3DXVECTOR3 & Position, const D3DXVECTOR2 & Texture) : position(Position), texture(Texture) {}
	
	inline static unsigned int GetFlexibleVertexFormat()
	{
		return D3DFVF_XYZ | D3DFVF_TEX1 ; //TEX1 means contains 1 texture coordinate, which is stage 0
	}

	inline static void CreateVertexDeclaration(IDirect3DDevice9 * device, IDirect3DVertexDeclaration9 ** vertexDeclaration)
	{
		D3DVERTEXELEMENT9 elements [] = 
		{
			{ 0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
			{ 0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
			D3DDECL_END()
		};
		device->CreateVertexDeclaration(elements, vertexDeclaration );
	}
};

struct PositionTextured4D
{
	D3DXVECTOR3 position;
	D3DXVECTOR4 texture;

	PositionTextured4D() : position(0,0,0), texture(0,0,0,0) {}
	PositionTextured4D(float x, float y, float z, float tx, float ty, float tz, float tw) : position(x,y,z), texture(tx,ty,tz,tw) {}
	PositionTextured4D(const D3DXVECTOR3 & Position, const D3DXVECTOR4 & Texture) : position(Position), texture(Texture) {}
	inline static unsigned int GetFlexibleVertexFormat()
	{
		return D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE4(0); //TEX1 means contains 1 texture coordinate, which is stage 0
		//D3DFVF_TEXCOORDSIZE4(0) means TEXCOORD0 is 4 components
	}

	inline static void CreateVertexDeclaration(IDirect3DDevice9 * device, IDirect3DVertexDeclaration9 ** vertexDeclaration)
	{
		D3DVERTEXELEMENT9 elements [] = 
		{
			{ 0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
			{ 0, 12, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
			D3DDECL_END()
		};
		device->CreateVertexDeclaration(elements, vertexDeclaration );
	}
};

struct PositionNormalTextured
{
	D3DXVECTOR3 position;
	D3DXVECTOR3 normal;
	D3DXVECTOR2 texture;

	PositionNormalTextured(){};
	PositionNormalTextured(float x, float y, float z, float nx, float ny, float nz, float u, float v) : position(x, y, z), normal(nx, ny, nz), texture(u, v) {}
	PositionNormalTextured(D3DXVECTOR3 Position, D3DXVECTOR3 Normal, D3DXVECTOR3 Texture) : position(Position), normal(Normal), texture(Texture) {}

	inline static unsigned int GetFlexibleVertexFormat()
	{
		return D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1 ; //TEX1 means contains 1 texture coordinate, which is stage 0
	}

	inline static void CreateVertexDeclaration(IDirect3DDevice9 * device, IDirect3DVertexDeclaration9 ** vertexDeclaration)
	{
		D3DVERTEXELEMENT9 elements [] = 
		{
			{ 0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
			{ 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0 },
			{ 0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
			D3DDECL_END()
		};
		device->CreateVertexDeclaration(elements, vertexDeclaration );
	}
};

struct PositionNormalMultiTextured
{
	D3DXVECTOR3 position;
	D3DXVECTOR3 normal;
	D3DXVECTOR2 texture1;
	D3DXVECTOR2 texture2;

	PositionNormalMultiTextured(){};
	PositionNormalMultiTextured(float x, float y, float z, float nx, float ny, float nz, float u1, float v1, float u2, float v2) : position(x, y, z), normal(nx, ny, nz), texture1(u1, v1), texture2(u2, v2) {}
	PositionNormalMultiTextured(D3DXVECTOR3 Position, D3DXVECTOR3 Normal, D3DXVECTOR3 Texture1, D3DXVECTOR3 Texture2) : position(Position), normal(Normal), texture1(Texture1), texture2(Texture2) {}

	inline static unsigned int GetFlexibleVertexFormat()
	{
		return D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX2 ; //TEX1 means contains 2 texture coordinates, which is stage 0 and 1
	}

	inline static void CreateVertexDeclaration(IDirect3DDevice9 * device, IDirect3DVertexDeclaration9 ** vertexDeclaration)
	{
		D3DVERTEXELEMENT9 elements [] = 
		{
			{ 0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
			{ 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0 },
			{ 0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
			{ 0, 32, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 }, //same texcoords used for stage 0 and 1
			D3DDECL_END()
		};
		device->CreateVertexDeclaration(elements, vertexDeclaration );
	}
};

struct PositionNormalTangentBinormalTextured
{
	D3DXVECTOR3 position;
	D3DXVECTOR3 normal;
	D3DXVECTOR3 tangent; 
	D3DXVECTOR3 binormal;
	D3DXVECTOR2 texture;

	inline static void CreateVertexDeclaration(IDirect3DDevice9 * device, IDirect3DVertexDeclaration9 ** vertexDeclaration)
	{
		D3DVERTEXELEMENT9 elements [] = 
		{
			{ 0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
			{ 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
			{ 0, 24, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT, 0},
			{ 0, 36, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BINORMAL, 0},
			{ 0, 48, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
			D3DDECL_END()
		};
		device->CreateVertexDeclaration(elements, vertexDeclaration);
	}
};

struct PositionNormalTangentBinormalTexturedWeighted
{
	D3DXVECTOR3 position;
	D3DXVECTOR3 normal;
	D3DXVECTOR3 tangent; 
	D3DXVECTOR3 binormal;
	D3DXVECTOR2 texture;
	float weight;

	inline static void CreateVertexDeclaration(IDirect3DDevice9 * device, IDirect3DVertexDeclaration9 ** vertexDeclaration)
	{
		D3DVERTEXELEMENT9 elements [] = 
		{
			{ 0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
			{ 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
			{ 0, 24, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT, 0},
			{ 0, 36, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BINORMAL, 0},
			{ 0, 48, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
			{ 0, 56, D3DDECLTYPE_FLOAT1, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1},
			D3DDECL_END()
		};
		device->CreateVertexDeclaration(elements, vertexDeclaration);
	}
};

struct TransformedTextured
{
	D3DXVECTOR4 position;
	D3DXVECTOR2 texture;

	TransformedTextured() : position(), texture() {}
	TransformedTextured(const D3DXVECTOR4 & Position, const D3DXVECTOR2 & Texture) : position(Position), texture(Texture) {}
	TransformedTextured(float x, float y, float z, float w, float tx, float ty) : position(x,y,z, w), texture(tx, ty) {}
	inline static unsigned int GetFlexibleVertexFormat()
	{
		return D3DFVF_XYZRHW | D3DFVF_TEX1 ; //TEX1 means contains 1 texture coordinate, which is stage 0
	}

	inline static void CreateVertexDeclaration(IDirect3DDevice9 * device, IDirect3DVertexDeclaration9 ** vertexDeclaration)
	{
		D3DVERTEXELEMENT9 elements [] = 
		{
			{ 0, 0,  D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITIONT, 0 },
			{ 0, 16, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
			D3DDECL_END()
		};
		device->CreateVertexDeclaration(elements, vertexDeclaration );
	}
};

#endif