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

#ifndef BACKGROUND_PIXMAP_BUFFER
#define BACKGROUND_PIXMAP_BUFFER

class TextPixmapBuffer;
#ifdef DXRENDER
#include "BT_DXRender.h"
#endif

class BackgroundPixmapBuffer
{
public:
	enum BackgroundPixmapBufferFlag
	{
		SolidColor		= (1 << 0),
		GradientColor	= (1 << 1)
	};

private:
	TextPixmapBuffer * _mask;
	QColor _topColor;
	QColor _bottomColor;
	QSize _size;
	QImage _buffer;
	float _cornerRadius;
	unsigned int _flags;
#ifdef DXRENDER
	weak_ptr<DXRender> _dxRef;
	IDirect3DTexture9 * _glTextureId;
#else
	unsigned int _glTextureId;
#endif
	bool _isDirty;
	bool _isOrthographic;

	void update(QImage& image);

public:
	BackgroundPixmapBuffer();
	~BackgroundPixmapBuffer();

	void setSize(const QSize& size);
	void setColor(const QColor& c);
	void setGradientColors(const QColor& top, const QColor& bottom);
	void setCornerRadius(float radius);
	bool setFlags(unsigned int flags);
	bool setOrthographic(bool isOrtho);
	void setTextPixmapBufferAsMask(TextPixmapBuffer * mask);
	bool hasFlag(BackgroundPixmapBufferFlag flag) const;

	void update();
#ifdef DXRENDER
	IDirect3DTexture9 * bindAsGLTexture(Vec3& maxUVsOut);
	void onRelease();
#else
	unsigned int bindAsGLTexture(Vec3& maxUVsOut);
#endif
	const QSize& getActualSize() const;

	// temp
	const QImage& getBuffer() const;
};

#endif