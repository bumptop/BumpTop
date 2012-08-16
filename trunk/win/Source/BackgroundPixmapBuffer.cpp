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
#include "BackgroundPixmapBuffer.h"
#include "TextPixmapBuffer.h"
#include "BT_GLTextureManager.h"

BackgroundPixmapBuffer::BackgroundPixmapBuffer()
: _topColor(Qt::red)
, _bottomColor(Qt::red)
, _flags(SolidColor)
, _glTextureId(0)
, _isDirty(true)
, _isOrthographic(true)
, _mask(NULL)
#ifdef DXRENDER
, _dxRef(dxr)
{
	_dxRef.lock();
}
#else
{}
#endif

BackgroundPixmapBuffer::~BackgroundPixmapBuffer()
{
	// clear the open gl texture
#ifdef DXRENDER
	onRelease();
	_dxRef.reset();
#else
	if (_glTextureId)
	{
		glDeleteTextures(1, &_glTextureId);
		_glTextureId = 0;
	}	
#endif
}

void BackgroundPixmapBuffer::setSize(const QSize& size)
{
	if (_size != size)
	{
		_size = size;
		_isDirty = true;
	}
}

void BackgroundPixmapBuffer::setColor(const QColor& c)
{
	if (_topColor != c || _bottomColor != c)
	{
		_topColor = _bottomColor = c;
		_isDirty = true;
	}
}

void BackgroundPixmapBuffer::setGradientColors(const QColor& top, const QColor& bottom)
{
	if (_topColor != top)
	{
		_topColor = top;
		_isDirty = true;
	}
	if (_bottomColor != bottom)
	{
		_bottomColor = bottom;
		_isDirty = true;
	}
}

void BackgroundPixmapBuffer::setCornerRadius(float radius)
{
	if (_cornerRadius != radius)
	{
		_cornerRadius = radius;
		_isDirty = true;
	}
}

bool BackgroundPixmapBuffer::setFlags(unsigned int flags)
{
	if (_flags != flags)
	{
		_flags = flags;
		_isDirty = true;
		return true;
	}
	return false;
}

bool BackgroundPixmapBuffer::setOrthographic(bool isOrtho)
{
	if (_isOrthographic != isOrtho)
	{
		_isOrthographic = isOrtho;
		_isDirty = true;
		return true;
	}
	return false;
}

void BackgroundPixmapBuffer::setTextPixmapBufferAsMask(TextPixmapBuffer * mask)
{
	if (_mask != mask)
	{
		_mask = mask;
		_isDirty = true;
	}
}

bool BackgroundPixmapBuffer::hasFlag(BackgroundPixmapBufferFlag flag) const
{
	return (_flags & flag) == flag;
}

void BackgroundPixmapBuffer::update(QImage& image)
{
	if (!_isDirty)
		return;

	// clear the open gl texture so that it will be updated at next bind
#ifdef DXRENDER
	SAFE_RELEASE(_glTextureId);
#else
	if (_glTextureId)
	{
		glDeleteTextures(1, &_glTextureId);
		_glTextureId = 0;
	}
#endif

	QSize size = _size;
	if (_mask)
		size.setWidth(size.width() + (2.0f * _mask->lrBuffer));

	// just create a blank image if there is nothing to render
	if (size.width() <= 0 || size.height() <= 0)
	{
		_buffer = QImage(QSize(1, 1), QImage::Format_ARGB32);
		_buffer.fill(Qt::transparent);
		return;
	}

	// create a new buffer
	int newWidth = nextPowerOfTwo(size.width());
	int newHeight = nextPowerOfTwo(size.height());
	int cornerRadius = _cornerRadius;
	_buffer = QImage(QSize(newWidth, newHeight), QImage::Format_ARGB32);

	QPainter painter;
	if (_mask)
	{
		QImage tmpBuffer = QImage(_buffer.size(), _buffer.format());
		tmpBuffer.fill(Qt::transparent);
		painter.begin(&tmpBuffer);
		painter.setRenderHint(QPainter::Antialiasing, true);
		painter.setRenderHint(QPainter::HighQualityAntialiasing, true);	
		painter.setPen(Qt::NoPen);

		// ensure that the text pixmap is up to date
		_mask->update();

		QStringList& lines = _mask->_lines;
		QList<QSize>& lineSizes = _mask->_lineSizes;
		QSize textBounds = _mask->_textBounds;

		// determine the new line sizes
		float minWidthRadiusMultiplier = 6.0f;
		for (int i = 1; i < lineSizes.size(); ++i) 
		{
			int curWidth = lineSizes[i].width();
			int prevWidth = lineSizes[i-1].width();

			if (abs(curWidth - prevWidth) < (minWidthRadiusMultiplier * cornerRadius))
			{
				if (curWidth < prevWidth)
				{
					// adjust the current width to match the previous width
					lineSizes[i].setWidth(prevWidth);
				}
				else if (curWidth > prevWidth)
				{
					// adjust the previous widths to match the current width					
					for (int j = i - 1; j >= 0; --j)
					{
						if (abs(curWidth - prevWidth) < (minWidthRadiusMultiplier * cornerRadius) && curWidth > prevWidth)
							lineSizes[j].setWidth(curWidth);
						if (j > 0)
							prevWidth = lineSizes[j - 1].width();
					}
				}
			}
		}

		// render the background
		int lrBuffer = 2.0f * _mask->lrBuffer;
		int x = lrBuffer;
		int y = _mask->tBuffer;
		int height = textBounds.height() / lineSizes.size();
		int prevLineLength = 0;
		QColor c = _topColor;
		c.setAlpha(255);

		for (int i = 0; i < lineSizes.size(); ++i) 
		{
			switch (_mask->_textAlignment)
			{
			case Qt::AlignLeft:
				x = lrBuffer;
				break;
			case Qt::AlignHCenter:
				x = (size.width() - lineSizes[i].width()) / 2 - lrBuffer;
				break;
			case Qt::AlignRight:
				x = size.width() - lineSizes[i].width();
				break;
			default: assert(false); break;
			}

			// render the rounded rect first
			painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
			painter.setBrush(QBrush(c));
			painter.drawRoundedRect(x, y, lineSizes[i].width() + (lrBuffer), height, cornerRadius, cornerRadius);

			// render any overlapping rects as necessary (+ buffer for inverse rounded corners)
			bool isPreviousLineEqual = (i > 0) && (lineSizes[i-1].width() == lineSizes[i].width());
			bool isPreviousLineLonger = (i > 0) && (lineSizes[i-1].width() > lineSizes[i].width());
			if (isPreviousLineEqual)
			{
				painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
				painter.setBrush(QBrush(c));
				painter.drawRect(x, y, lineSizes[i].width() + (lrBuffer), cornerRadius);			
			}
			else if (isPreviousLineLonger)
			{				
				painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
				painter.setBrush(QBrush(c));
				painter.drawRect(x - cornerRadius, y, lineSizes[i].width() + (lrBuffer) + (2.0f * cornerRadius), cornerRadius);			
				if (isPreviousLineLonger)
				{
					// draw the inverse rounded corners
					painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
					painter.setBrush(QBrush(Qt::transparent));
					painter.drawRoundedRect(x - (2.0f * cornerRadius), y, (2.0f * cornerRadius), (2.0f * cornerRadius), cornerRadius, cornerRadius);
					painter.drawRoundedRect(x + lineSizes[i].width() + (lrBuffer), y, (2.0f * cornerRadius), (2.0f * cornerRadius), cornerRadius, cornerRadius);
				}
			}

			bool isNextLineEqual = (i < (lineSizes.size() - 1)) && (lineSizes[i+1].width() == lineSizes[i].width());
			bool isNextLineLonger = (i < (lineSizes.size() - 1)) && (lineSizes[i+1].width() > lineSizes[i].width());
			if (isNextLineEqual)
			{
				painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
				painter.setBrush(QBrush(c));
				painter.drawRect(x, y + height - cornerRadius, lineSizes[i].width() + (lrBuffer), cornerRadius);
			}
			else if (isNextLineLonger)
			{
				painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
				painter.setBrush(QBrush(c));
				painter.drawRect(x - cornerRadius, y + height - cornerRadius, lineSizes[i].width() + (lrBuffer) + (2.0f * cornerRadius), cornerRadius);
				if (isNextLineLonger)
				{
					// draw the inverse rounded corners
					painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
					painter.setBrush(QBrush(Qt::transparent));
					painter.drawRoundedRect(x - (2.0f * cornerRadius), y + height - (2.0f * cornerRadius), (2.0f * cornerRadius), (2.0f * cornerRadius), cornerRadius, cornerRadius);
					painter.drawRoundedRect(x + lineSizes[i].width() + (lrBuffer), y + height - (2.0f * cornerRadius), (2.0f * cornerRadius), (2.0f * cornerRadius), cornerRadius, cornerRadius);
				}
			}

			y += height;
			prevLineLength = lineSizes[i].width();
		}
		painter.end();

		// draw onto the actual buffer
		_buffer.fill(QColor(255, 255, 255, 192).rgba());
		painter.begin(&_buffer);
		painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
		painter.drawImage(0, 0, tmpBuffer);
		painter.end();
	}
	else
	{
		_buffer.fill(Qt::transparent);
		painter.begin(&_buffer);
		painter.setRenderHint(QPainter::Antialiasing, true);
		painter.setRenderHint(QPainter::HighQualityAntialiasing, true);	
		painter.setPen(Qt::NoPen);

		if (hasFlag(GradientColor))
		{
			QLinearGradient gradient(0, 0, 0, size.height());
			gradient.setColorAt(0, _topColor);
			gradient.setColorAt(1, _bottomColor);

			painter.setBrush(QBrush(gradient));
			painter.drawRoundedRect(0, 0, size.width(), size.height(), cornerRadius, cornerRadius);
		}
		else if (hasFlag(SolidColor))
		{
			painter.setBrush(QBrush(_topColor));
			painter.drawRoundedRect(0, 0, size.width(), size.height(), cornerRadius, cornerRadius);
		}
		painter.end();
	}

	_isDirty = false;
}

void BackgroundPixmapBuffer::update()
{
	update(_buffer);
}
#ifdef DXRENDER
IDirect3DTexture9 * BackgroundPixmapBuffer::bindAsGLTexture(Vec3& maxUVsOut)
#else
unsigned int BackgroundPixmapBuffer::bindAsGLTexture(Vec3& maxUVsOut)
#endif
{
#ifdef DXRENDER
	if (!_glTextureId && 0 < _buffer.width() && 0 < _buffer.height())
		_glTextureId = dxr->createTextureFromData(_buffer.width(), _buffer.height(), _buffer.bits(), _buffer.bytesPerLine(), false, 1);
	dxr->device->SetTexture(0, _glTextureId);
#else
	if (!_glTextureId)
	{
		glGenTextures(1, &_glTextureId);
		glBindTexture(GL_TEXTURE_2D, _glTextureId);

		if (!_isOrthographic)
		{
			float maximumAnisotropy = 0.0f;
			glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maximumAnisotropy);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maximumAnisotropy);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
		}

		// clamping the uvs to ensure no texture bleeding
		if (GLEW_ARB_texture_border_clamp)
		{
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER_ARB);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER_ARB);
		}

		// copy the full image (glTexSubImage2D does not work well with GL_GENERATE_MIPMAP)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _buffer.width(), _buffer.height(),
			0, GL_BGRA, GL_UNSIGNED_BYTE, _buffer.bits());
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D, _glTextureId);
	}
#endif
	maxUVsOut = Vec3((float) _size.width() / _buffer.width(), 
		(float) _size.height() / _buffer.height(), 0.0f);
	return _glTextureId;
}

#ifdef DXRENDER
void BackgroundPixmapBuffer::onRelease()
{
	SAFE_RELEASE(_glTextureId);
}
#endif

const QSize& BackgroundPixmapBuffer::getActualSize() const
{
	return _size;
}

const QImage& BackgroundPixmapBuffer::getBuffer() const
{
	return _buffer;
}