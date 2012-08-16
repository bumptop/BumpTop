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

#ifndef _COLOR_VAL_
#define _COLOR_VAL_

// -----------------------------------------------------------------------------

// TODO: REMOVE this class for QColor, blah

class ColorVal
{
public:

	union
	{
		uint val32;

		struct 
		{
			unsigned char r;
			unsigned char g;
			unsigned char b;
			unsigned char a;
		}bigEndian;
	};

public:

	inline ColorVal(unsigned char alpha = 0xFF, unsigned char red = 0x00, unsigned char green = 0x00, unsigned char blue = 0x00);

	// Overloaded Operators
	inline ColorVal operator*(const float in);
	inline ColorVal& operator=(const ColorVal& in);
#ifdef DXRENDER
	inline operator D3DXCOLOR () const { return D3DXCOLOR(D3DCOLOR(*this)); }
	inline operator D3DCOLOR () const { return D3DCOLOR((bigEndian.a << 24) | (bigEndian.r << 16) | (bigEndian.g << 8) | (bigEndian.b)); }
#else
	// Setters
	inline void setAsOpenGLColor() const;
#endif
	inline QColor asQColor() const;
};

// -----------------------------------------------------------------------------

#include "BT_ColorVal.inl"

// -----------------------------------------------------------------------------

#else
	class ColorVal;
#endif