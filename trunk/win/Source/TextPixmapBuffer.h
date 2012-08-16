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

#ifndef TEXT_PIXMAP_BUFFER
#define TEXT_PIXMAP_BUFFER

class TextPixmapBuffer;
#ifdef DXRENDER
	#include "BT_DXRender.h"
#endif

class TextPixmapCacheEntry;

class TextPixmapCache
{
private:
	QVector<TextPixmapCacheEntry> _vec;
	int _maxSize;
	int _size;
	int _cur;

public:
	TextPixmapCache(int size);
	~TextPixmapCache();
	void clear();
	void insert(TextPixmapCacheEntry e);
	const TextPixmapCacheEntry* get(int i) const;
	int size() const;

};

class TextPixmapCacheEntry
{
public:
	//KEY
	QColor _topColor;
	QColor _bottomColor;
	QFont _font;
	QString _text;		
	QSize _maxTextBounds;
	Qt::Alignment _textAlignment;		
	unsigned int _flags;

	//DATA
	QImage _buffer;	
	QSize _textBounds;

	bool equalKey (const TextPixmapBuffer &other) const;

};

class TextPixmapBuffer
{
	friend class BackgroundPixmapBuffer;
	friend class TextPixmapCacheEntry;

public:
	enum TextPixmapBufferFlag
	{
		Truncated			= (1 << 0),
		TruncateToSingleLine = (1 << 1),
		FilenameUnTruncated = (1 << 2),
		Simplified			= (1 << 3),		// all whitespace is replaced with a space
		HideFileExtension	= (1 << 4),
		FlexibleMaxBounds	= (1 << 5),

		RenderShadow		= (1 << 8),
		RenderFastShadow	= (1 << 9),
		RenderBackground	= (1 << 10),
		DisableClearType	= (1 << 11),
		ForceLinearFiltering = (1 << 12),

		SolidColor			= (1 << 16),
		GradientColor		= (1 << 17),

		Filename			= Truncated | RenderShadow,
		SelectedFilename	= FilenameUnTruncated | RenderShadow
	};

private:
	const static int lrBuffer = 4;
	const static int tBuffer = 1;
	const static int bBuffer = 2;

	QColor _topColor;
	QColor _bottomColor;
	QFont _font;
	QFontMetrics _fontMetrics;
	QImage _buffer;
	QString _text;
	QSize _textBounds;
	QSize _maxTextBounds;
	Qt::Alignment _textAlignment;
	QStringList _lines;
	QList<QSize> _lineSizes;
	unsigned int _flags;	

	bool _useCache;
	TextPixmapCache _cache;
	

#ifdef DXRENDER
	weak_ptr<DXRender> _dxRef;
	IDirect3DTexture9 * _glTextureId;
#else
	unsigned int _glTextureId;
#endif
	bool _isDirty;
	bool _isOrthographic;

private:
	void update(QImage& image);
	bool recurseOnLine(const QString& srcText, int startIndex, const QFontMetrics& metrics, int maxWidth, int maxWidthError, QStringList& linesOut, QList<QSize>& lineSizesOut, bool canReturnFalse) const;
	QSize getTextBounds(QStringList& linesOut, QList<QSize>& lineSizesOut) const;
	QString elidedText(const QString& line, int maxWidth, QSize& lineSizeOut, Qt::TextElideMode mode) const;

public:
	TextPixmapBuffer(int cacheSize = 0);
	TextPixmapBuffer(const QString& text, const QFont& font, unsigned int flags, const QSize& maxTextBounds, int cacheSize = 0);
	~TextPixmapBuffer();

	bool setMaxBounds(const QSize& maxTextBounds);
	bool setColor(const QColor& c);
	bool setGradientColors(const QColor& top, const QColor& bottom);
	bool setFont(const QFont& font);
	bool setText(const QString& text);
	bool setFlags(unsigned int flags);
	bool setTextAlignment(Qt::Alignment alignment);
	bool setOrthographic(bool isOrtho);
	bool pushFlag(TextPixmapBufferFlag flag);
	bool popFlag(TextPixmapBufferFlag flag);
	bool hasFlag(TextPixmapBufferFlag flag) const;

	void update();
#ifdef DXRENDER
	IDirect3DTexture9 * bindAsGLTexture(Vec3& maxUVsOut);
	void onRelease();
#else
	unsigned int bindAsGLTexture(Vec3& maxUVsOut);
#endif
	const QSize& getActualSize() const;
	const QString& getText() const;

	// temp
	const QImage& getBuffer() const;
	

//signals:
//	void renderBackground(QPainter& painter, const QList<QString>& lines);
};

#endif