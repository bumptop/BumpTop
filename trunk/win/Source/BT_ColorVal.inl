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

ColorVal::ColorVal(unsigned char alpha, unsigned char red, unsigned char green, unsigned char blue)
{
	bigEndian.a = alpha;
	bigEndian.r = red;
	bigEndian.g = green;
	bigEndian.b = blue;
}

ColorVal ColorVal::operator*(const float in)
{
	ColorVal val = *this;

	val.bigEndian.r = int(float(bigEndian.r) * in);
	val.bigEndian.g = int(float(bigEndian.g) * in);
	val.bigEndian.b = int(float(bigEndian.b) * in);
	val.bigEndian.a = int(float(bigEndian.a) * in);

	return val;
}

ColorVal& ColorVal::operator=(const ColorVal& in)
{
	val32 = in.val32;
	return *this;
}

#ifdef DXRENDER
#else
void ColorVal::setAsOpenGLColor() const
{
	// Set this color using OpenGL
	glColor4f(float(bigEndian.r) / 255.0f, float(bigEndian.g) / 255.0f, float(bigEndian.b) / 255.0f, float(bigEndian.a) / 255.0f);
}
#endif

QColor ColorVal::asQColor() const
{
	return QColor(bigEndian.r, bigEndian.g, bigEndian.b, bigEndian.a);
}
