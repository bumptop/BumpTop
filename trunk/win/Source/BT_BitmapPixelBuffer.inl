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

BitmapAndPixelBuffer::BitmapAndPixelBuffer()
{
	this->bitmap = NULL;
	this->pixelBuffer = NULL;
	this->originalWidth = 0;
	this->originalHeight = 0;
	this->textureDetail = Undefined;
}

BitmapAndPixelBuffer::BitmapAndPixelBuffer(QString textureKey, Gdiplus::Bitmap* bitmap, byte* pixelBuffer, uint originalWidth, uint originalHeight, TextureDetail textureDetail)
{
	this->textureKey = textureKey;
	this->bitmap = bitmap;
	this->pixelBuffer = pixelBuffer;
	this->originalWidth = originalWidth;
	this->originalHeight = originalHeight;
	this->textureDetail = textureDetail;
	assert(bitmap != NULL);
}