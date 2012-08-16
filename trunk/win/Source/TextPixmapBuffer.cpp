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
#include "BT_FileSystemManager.h"
#include "BT_GLTextureManager.h"
#include "BT_Util.h"
#include "BlitzBlur.h"
#include "TextPixmapBuffer.h"

#ifdef DXRENDER
#include "BT_DXRender.h"
#endif

#define QT_EXPERIMENTAL_LINEBREAKS 1

//NOTE: cache is only cleared when text is changed

TextPixmapCache::TextPixmapCache(int size)
: _size(0), _cur(0), _maxSize(size), _vec(size)
{
}

TextPixmapCache::~TextPixmapCache()
{
}
void TextPixmapCache::clear()
{
	_size = _cur = 0;
}
void TextPixmapCache::insert(TextPixmapCacheEntry e)
{
	_vec[_cur++] = e;
	if (_cur == _maxSize) {		
		_cur = 0;
	}

	if (_cur == 0 && _size == _maxSize)
	{
		cout << "CacheOverflow: ";	//Not a crit. error. Overflowed the cache, use a bigger cache in general.
		consoleWrite(e._text);
		consoleWrite("\n");
		//assert(false);

		//ASSERTION FAILURE??
		//Right now the cache size can only hold 4 toggles.
	}

	_size = min(_maxSize, _size+1);
}
const TextPixmapCacheEntry* TextPixmapCache::get(int i) const
{
	assert(i < _size);	
	return &_vec[i];
}
int TextPixmapCache::size() const
{
	return _size;
}


bool TextPixmapCacheEntry::equalKey(const TextPixmapBuffer &other) const
{
	return (other._flags == _flags && other._text == _text && other._bottomColor == _bottomColor && other._topColor == _topColor &&
		other._font == _font && other._maxTextBounds == _maxTextBounds && other._textAlignment == _textAlignment);
}



TextPixmapBuffer::TextPixmapBuffer(int cacheSize)
: _topColor(Qt::white)
, _bottomColor(Qt::white)
, _font(QFont("Arial", 14))
, _fontMetrics(_font)
, _maxTextBounds(0, 0)
, _flags(RenderShadow | SolidColor)
, _glTextureId(0)
, _textAlignment(Qt::AlignHCenter)
, _isDirty(true)
, _isOrthographic(true)
, _useCache(cacheSize != 0)
, _cache(cacheSize)
#ifdef DXRENDER
, _dxRef(dxr)
{
	_dxRef.lock();
#else
{
#endif
	_text = "Lorem Ipsum";
		
}

TextPixmapBuffer::TextPixmapBuffer(const QString& text, const QFont& font, unsigned int flags, const QSize& maxTextBounds, int cacheSize)
: _topColor(Qt::white)
, _bottomColor(Qt::white)
, _font(font)
, _fontMetrics(font)
, _maxTextBounds(maxTextBounds)
, _flags(flags)
, _glTextureId(0)
, _isDirty(true)
, _isOrthographic(true)
, _textAlignment(Qt::AlignHCenter)
, _useCache(cacheSize != 0)
, _cache(cacheSize)
#ifdef DXRENDER
, _dxRef(dxr)
{
	_dxRef.lock();
#else
{
#endif	
	_font.setKerning(false);
	if (hasFlag(Simplified))
		_text = text.simplified();
	else
		_text = text;

	// TODO: subtract the max bounds to account for the background?
	update(_buffer);

	// NOTE: it doesn't really make sense to bound by the height for now
	//	since that's dependent on the font size and the length of the 
	//	text, perhaps later we should add a ClipToBounds flag to force 
	//	all text to reside within a particular rect
	assert(_maxTextBounds.height() == 0);
}

TextPixmapBuffer::~TextPixmapBuffer()
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

void TextPixmapBuffer::update(QImage& buffer)
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
	
	//Check if its in the cache or not
	if (_useCache)
	{	
		bool cacheHit = false;		
		
		//Use the cache of it contains it
		for (int i = 0; i < _cache.size(); i++)
		{

			const TextPixmapCacheEntry *e = _cache.get(i);  //avoids the copy
			if (e->equalKey(*this))
			{
				_buffer = e->_buffer;
				_textBounds = e->_textBounds;
				cacheHit = true;
				break;
			}
						
		}

		if (cacheHit) return;
	}

	// calculate the dimensions of the object
	QStringList lines;
	QList<QSize> lineSizes;
	_textBounds = getTextBounds(lines, lineSizes);
	assert(lines.size() == lineSizes.size());

	// return if there is nothing to draw
	if (lines.isEmpty())
		return;

	// ensure valid size
	if (_textBounds.width() == 0 || _textBounds.height() == 0)
		return;
	
	_lines = lines;
	_lineSizes = lineSizes;

	// adjust the line heights slightly since they are too tight
	int newTextHeight = 0;
	int lineHeightAdjustment = 1;
	for (int i = 0; i < _lineSizes.size(); ++i)
	{
		_lineSizes[i].setHeight(_lineSizes[i].height() + lineHeightAdjustment);
		newTextHeight += _lineSizes[i].height();
	}

	// add some sort of buffer for the background vs the text
	_textBounds.setWidth(_textBounds.width() + 2 * lrBuffer);
	_textBounds.setHeight(newTextHeight + tBuffer + bBuffer);
		
	// NOTE: we have to use premultiplied alpha since cleartype is 
	//   now disabled for argb32 qimages
	// http://lists.trolltech.com/pipermail/qt-jambi-interest/2009-July/001356.html
	int newWidth = nextPowerOfTwo(_textBounds.width());
	int newHeight = nextPowerOfTwo(_textBounds.height());
	buffer = QImage(QSize(newWidth, newHeight), (hasFlag(DisableClearType) ? QImage::Format_ARGB32 : QImage::Format_ARGB32_Premultiplied));
	buffer.fill(Qt::transparent);

	QPainter painter;
	painter.begin(&buffer);
	painter.setPen(Qt::NoPen);
	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.setRenderHint(QPainter::TextAntialiasing, true);
	painter.setRenderHint(QPainter::HighQualityAntialiasing, true);	
	painter.setFont(_font);

	QFontMetrics metrics(_font);
	int descent = metrics.descent();
	int x = lrBuffer;
	int y = tBuffer;
	int totalHeight = _textBounds.height() - (descent * (lines.size() - 1));
	_textBounds.setHeight(totalHeight);
	int totalWidth = _textBounds.width();
	int prevWidth = 0;
	int maxWidth = _maxTextBounds.width();
	int tmpWidth = 0;
	int radius = _lineSizes.front().height() / 4;

	if (hasFlag(RenderShadow))
	{
		painter.setPen(QColor(20,20,20));

		// create the text blur
		x = lrBuffer;
#ifdef QT_EXPERIMENTAL_LINEBREAKS
		y = ((lines.size() > 1) ? (_textBounds.height() / lines.size()) - _lineSizes[0].height(): 0);
#else
		y = tBuffer;
#endif
		for (int i = 0; i < lines.size(); ++i)
		{		
			// center the text horizontally
			switch (_textAlignment)
			{
			case Qt::AlignLeft:
				x = lrBuffer;
				break;
			case Qt::AlignHCenter:
				x = (totalWidth - _lineSizes[i].width()) / 2;
				break;
			case Qt::AlignRight:
				x = totalWidth - _lineSizes[i].width();
				break;
			default: assert(false); break;
			}
			y += _lineSizes[i].height() - descent;
		
			painter.drawText(x, y, lines[i]);
		}

		// blit it to our final image
		// we are painting this multiple times to darken the shadow
		QImage blurred = Blitz::blur(buffer, 1);
		painter.drawImage(0, 0, blurred);
		blurred = Blitz::blur(blurred, 2);
		painter.drawImage(0, 1, blurred);
	}
	else if (hasFlag(RenderFastShadow))
	{
		painter.setPen(QColor(40,40,40));

		x = lrBuffer;
#ifdef QT_EXPERIMENTAL_LINEBREAKS
		y = ((lines.size() > 1) ? (_textBounds.height() / lines.size()) - _lineSizes[0].height(): 0);
#else 	
		y = tBuffer;
#endif
		for (int i = 0; i < lines.size(); ++i)
		{
			// center the text horizontally
			switch (_textAlignment)
			{
			case Qt::AlignLeft:
				x = lrBuffer;
				break;
			case Qt::AlignHCenter:
				x = (totalWidth - _lineSizes[i].width()) / 2;
				break;
			case Qt::AlignRight:
				x = totalWidth - _lineSizes[i].width();
				break;
			default: assert(false); break;
			}
			y += _lineSizes[i].height() - descent;

			// render the text
			painter.drawText(x, y + 1, lines[i]);
		}
	}

	// set the pen colors
	if (hasFlag(TextPixmapBuffer::GradientColor))
	{
		QLinearGradient gradient(0, 0, 0, totalHeight);
			gradient.setColorAt(0, _topColor);
			gradient.setColorAt(1, _bottomColor);
		
		painter.setPen(QPen(QBrush(gradient), 0));
	}
	else
	{
		painter.setPen(QPen(_topColor));
	}
		
	if (true) {// new block
		// create the text		
		x = lrBuffer;
#ifdef QT_EXPERIMENTAL_LINEBREAKS
		y = ((lines.size() > 1) ? (_textBounds.height() / lines.size()) - _lineSizes[0].height(): 0);
#else
		y = tBuffer;
#endif
		for (int i = 0; i < lines.size(); ++i)
		{		
			// center the text horizontally
			switch (_textAlignment)
			{
			case Qt::AlignLeft:
				x = lrBuffer;
				break;
			case Qt::AlignHCenter:
				x = (totalWidth - _lineSizes[i].width()) / 2;
				break;
			case Qt::AlignRight:
				x = totalWidth - _lineSizes[i].width();
				break;
			default: assert(false); break;
			}
			y += _lineSizes[i].height() - descent;

			// render the text
			painter.drawText(x, y, lines[i]);
		}
	}
	painter.end();

	//Store the QImage in the cache
	if (_useCache)
	{
		TextPixmapCacheEntry e;
		e._bottomColor = _bottomColor;
		e._topColor = _topColor;
		e._font = _font;

		e._text = _text;
		
		e._maxTextBounds = _maxTextBounds;
		e._textAlignment = _textAlignment;		
		e._flags = _flags;
		
		e._buffer = _buffer;
		e._textBounds = _textBounds;

		_cache.insert(e);
	}

	_isDirty = false;
}


enum TokenBoundary {
	NoBoundary,
	NormalBoundary,
	NewlineBoundary,
	EndOfStringBoundary
};

// Returns the previous token, or the string from the start index up to
// the closest boundary position after minWidth (but before nextIndex).
// NOTE: we don't touch the line, lineSize, or newIndexOut if there is no boundary found
TokenBoundary getPreviousToken(const QString& srcText, int startIndex, int nextIndex, const QFontMetrics& metrics, int minWidth, int& prevIndexOut, QString& lineOut, QSize& lineSizeOut)
{
	QTextBoundaryFinder boundaries(QTextBoundaryFinder::Word, srcText);
	boundaries.setPosition(nextIndex);
	int boundaryAfterMinWidth = nextIndex;
	int prevBoundary = boundaries.toPreviousBoundary();
	QSize lineSize;
	QString line;
	do 
	{		
		// bound by the start index
		if (prevBoundary <= startIndex)
			break;

		line = srcText.mid(startIndex, prevBoundary - startIndex);
		lineSize = metrics.boundingRect(QRect(), 0, line).size();
		
		// bound by the min size
		if (lineSize.width() < minWidth)
			break;

		// only increment the right bound if it is not a punctuation (so that lines don't end on punctuations)
		if (!srcText.at(prevBoundary).isPunct())
			boundaryAfterMinWidth = prevBoundary;
		prevBoundary = boundaries.toPreviousBoundary();
	} while(true);

	if (boundaryAfterMinWidth < nextIndex) {
		lineOut = srcText.mid(startIndex, boundaryAfterMinWidth - startIndex);
		lineSizeOut = metrics.boundingRect(QRect(), 0, lineOut).size();
		prevIndexOut = boundaryAfterMinWidth;
		if (srcText.at(boundaryAfterMinWidth) == QChar(' '))
			++prevIndexOut;
		return NormalBoundary;
	}

	return NoBoundary;
}

// Returns the next token, or the string from the start index up to 
// the closest boundary position before max width, the next new line
// or the next end of line.  
TokenBoundary getNextToken(const QString& srcText, int startIndex, const QFontMetrics& metrics, int maxWidth, int& nextIndexOut, QString& lineOut, QSize& lineSizeOut)
{
	QTextBoundaryFinder boundaries(QTextBoundaryFinder::Word, srcText);
	boundaries.setPosition(startIndex);
	
	// handle any newlines first
	int nextNewLineIndex = srcText.indexOf('\n', startIndex);
	if (nextNewLineIndex > -1)
	{
		lineOut = srcText.mid(startIndex, nextNewLineIndex - startIndex);
		lineSizeOut = metrics.boundingRect(QRect(), 0, lineOut).size();
		if (lineSizeOut.width() <= maxWidth)
		{
			nextIndexOut = nextNewLineIndex + 1;
			return NewlineBoundary;
		}
	}

	// lets just do a binary search of the string until we find a marker where
	// the line width from the start index is less than or equal to the max width
	int halfIndex = (startIndex + srcText.size()) / 2;
	int maxLen = srcText.size();
	do {
		boundaries.setPosition(halfIndex);
		int prevBoundary = boundaries.toPreviousBoundary();
		boundaries.setPosition(halfIndex);
		int nextBoundary = boundaries.toNextBoundary();

		lineOut = srcText.mid(startIndex, prevBoundary - startIndex);
		lineSizeOut = metrics.boundingRect(QRect(), 0, lineOut).size();
		bool prevBoundaryFits = !(lineSizeOut.width() > maxWidth);
		lineOut = srcText.mid(startIndex, nextBoundary - startIndex);
		lineSizeOut = metrics.boundingRect(QRect(), 0, lineOut).size();
		bool nextBoundaryFits = !(lineSizeOut.width() > maxWidth);

		if (prevBoundaryFits && nextBoundaryFits)
		{
			// move the cursor towards the next boundary
			int newHalfIndex = (halfIndex + maxLen) / 2;
			if (newHalfIndex == halfIndex)
			{
				// there is no other valid next boundary, and the whole lineOut fits
				lineOut = srcText.mid(startIndex);
				lineSizeOut = metrics.boundingRect(QRect(), 0, lineOut).size();
				nextIndexOut = -1;
				return EndOfStringBoundary;
			}
			halfIndex = newHalfIndex;
		}
		else if (!prevBoundaryFits && !nextBoundaryFits)
		{
			// move the cursor towards the previous boundary
			int newHalfIndex = ((startIndex + halfIndex) / 2);
			if (newHalfIndex == halfIndex)
			{
				// there is no other valid next boundary, and there is no
				// single block that can be broken down to fit on a lineOut
				// of the specified max width, so just break down the lineOut
				// at the max width
				int aveNumChars = maxWidth / metrics.averageCharWidth();
				lineOut = srcText.mid(startIndex, aveNumChars);
				lineSizeOut = metrics.boundingRect(QRect(), 0, lineOut).size();
				nextIndexOut = startIndex + lineOut.size();
				return (lineOut.size() == srcText.size()) ? EndOfStringBoundary : NormalBoundary;
			}
			halfIndex = newHalfIndex;
			maxLen = prevBoundary;
		}
		else
		{
			if (prevBoundary == startIndex)
			{
				// this is a single block of text that has a width > the the max width
				// so in this case, just return the max-width line if we can 
				lineOut = srcText.mid(startIndex);
				lineSizeOut = metrics.boundingRect(QRect(), 0, lineOut).size();
				nextIndexOut = -1;
				if (lineSizeOut.width() > maxWidth)
				{
					int aveNumChars = maxWidth / metrics.averageCharWidth();
					lineOut = srcText.mid(startIndex, aveNumChars);
					lineSizeOut = metrics.boundingRect(QRect(), 0, lineOut).size();
					nextIndexOut = startIndex + lineOut.size();
					return NormalBoundary;
				}
				return EndOfStringBoundary;
			}
			else 
			{
				// just use the lineOut up to the previous boundary OR a newline
				// if one exists			
				lineOut = srcText.mid(startIndex, prevBoundary - startIndex);
				lineSizeOut = metrics.boundingRect(QRect(), 0, lineOut).size();
				nextIndexOut = prevBoundary;

				// now just trim all the backspaces at the end
				while (lineOut.endsWith(' '))
					lineOut.chop(1);

				return NormalBoundary;
			}
		}
	} while (true);

	return NoBoundary;
}

// Returns the elided text, with the specified text bounds
QString TextPixmapBuffer::elidedText(const QString& line, int maxWidth, QSize& lineSizeOut, Qt::TextElideMode mode) const
{
	QString text = _fontMetrics.elidedText(line, mode, maxWidth);
			
	int index = 0;
	int rIndex = 0;
	if (text.size() != line.size())
		rIndex = 1;
	do 
	{
		index = text.size() - rIndex - 1;
		if (index >= 0) {
			if (text.at(index).isPunct() || text.at(index).isSpace())
				++rIndex;
			else {
				++index;
				break;
			}
		}
		else {
			++index;
			break;
		}
	} while(true);
	if (rIndex > 0)
		text = text.replace(index, (text.size() - 1) - index, QString());
	lineSizeOut = _fontMetrics.boundingRect(QRect(), 0, text).size();
	return text;
}

bool TextPixmapBuffer::recurseOnLine(const QString& srcText, int startIndex, const QFontMetrics& metrics, int maxWidth, int maxWidthError, QStringList& linesOut, QList<QSize>& lineSizesOut, bool canReturnFalse) const
{
#ifdef QT_EXPERIMENTAL_LINEBREAKS
	if ((startIndex < 0) || (startIndex >= srcText.size())) // invalid index
		return true;

	QString line;
	QSize lineSize;
	int cost = 0;
	int nextIndex = startIndex;
	TokenBoundary nextBoundary = getNextToken(srcText, startIndex, metrics, maxWidth, nextIndex, line, lineSize);
	switch (nextBoundary) {
		case NormalBoundary:
			cost = lineSize.width() - (maxWidth / 2);
			if (cost < 0) { // this line is short
				if (canReturnFalse) { // the parent may be able to spare some text
					return false;
				}
				else { // the parent is unable to spare any text, recurse below
					break;
				}
			}
			else { // this line is ok, recurse below
				break;
			}
		case NewlineBoundary: // recurse below
			break;
		case EndOfStringBoundary:
			cost = lineSize.width() - (maxWidth / 2);
			if (cost < 0) { // this line is short
				if (canReturnFalse) { // the parent may be able to spare some text
					return false;
				}
				else { // the parent is unable to spare any text, recurse below
					linesOut.insert(0, line);
					lineSizesOut.insert(0, lineSize);
					return true;
				}
			}
			else { // this line is ok, recurse below
				break;
			}
		default:
			assert(false);
			return true;
	}

	// recurse and handle it
	bool nextStringIsOK = recurseOnLine(srcText, nextIndex, metrics, maxWidth, maxWidthError, linesOut, lineSizesOut, true);
	if (nextStringIsOK) { // next string is ok, so insert ours
		linesOut.insert(0, line);
		lineSizesOut.insert(0, lineSize);
		return true;
	}
	else { // next string is too short, see if we can spare some words
		// get the previous token 
		int prevIndex = nextIndex;
		TokenBoundary prevBoundary = getPreviousToken(srcText, startIndex, nextIndex, metrics, (maxWidth / 2), prevIndex, line, lineSize);
		switch (prevBoundary) {
			case NoBoundary: // we can't truncate this line at all
				recurseOnLine(srcText, nextIndex, metrics, maxWidth, maxWidthError, linesOut, lineSizesOut, false);
				linesOut.insert(0, line);
				lineSizesOut.insert(0, lineSize);
				return true;
			case NormalBoundary:
				// TODO: make this a while loop
				recurseOnLine(srcText, prevIndex, metrics, maxWidth, maxWidthError, linesOut, lineSizesOut, false);
				linesOut.insert(0, line);
				lineSizesOut.insert(0, lineSize);
				return true;
			default:
				assert(false);
				return true;
		}
	}
#endif
	return false;
}

QSize TextPixmapBuffer::getTextBounds(QStringList& linesOut, QList<QSize>& lineSizesOut) const
{
	if (_text.isEmpty())
		return QSize();

	// clear the lines out
	linesOut.clear();

	QString srcText;
	if (hasFlag(HideFileExtension))
		srcText = _text.left(_text.size() - fsManager->getFileExtension(_text).size());
	else 
		srcText = _text;

	// get the text metrics and determine how to split the lines
	QRect textRect = _fontMetrics.boundingRect(QRect(), 0, srcText);
	textRect.translate(-textRect.left(), -textRect.top());
	int lineSpacing = NxMath::max(0, _fontMetrics.lineSpacing() - 1);
	int height = textRect.height();
	int width = textRect.width();
	int maxWidth = NxMath::max(_fontMetrics.averageCharWidth(), _maxTextBounds.width());

	// ensure that the area is a minimum size
	if (maxWidth < (2 * _fontMetrics.averageCharWidth()))
		return _maxTextBounds;

	if (hasFlag(TruncateToSingleLine) && hasFlag(Truncated))
	{
		// get the elided text for the single line
		QSize tmpSize;
		QString line = elidedText(srcText, maxWidth, tmpSize, Qt::ElideRight);
		linesOut.append(line);
		lineSizesOut.append(tmpSize);
		return QSize(tmpSize.width(), height);
	}
	else // !hasFlag(TruncateToSingleLine)
	{
		QSize tmpSize;
		QString line;
		int maxLineWidth = 0;

		// we know that the line does not fit on a single line of the specified 
		// preferred width
		if (hasFlag(Truncated))
		{
			// if the text is within the max bounds then just return it
			// Note: we ignore the line height 
			if (width <= maxWidth)
			{
				linesOut.append(srcText);
				lineSizesOut.append(QSize(textRect.size().width(), height));
				return textRect.size();
			}
			
			// break the text into two lines at a reasonable point and then 
			// truncate them appropriately
			int nextIndex = 0;
			QString line;
			QSize lineSize;
			TokenBoundary nextBoundary = getNextToken(srcText, 0, _fontMetrics, maxWidth, nextIndex, line, lineSize);
			if (nextBoundary == EndOfStringBoundary)
			{
				linesOut.append(line);
				lineSizesOut.append(lineSize);
				maxLineWidth = lineSize.width();
			}
			else if (nextBoundary == NormalBoundary) 
			{
				// if the line is going to be broken up into too small a string, then 
				// just make it a single string
				QString nextLine = srcText.mid(nextIndex);
				QSize nextLineSize = _fontMetrics.boundingRect(QRect(), 0, nextLine).size();
				if (nextLineSize.width() < (maxWidth / 2))
				{
					line = srcText;
					nextLine.clear();
				}

				// elide the text by the max width
				linesOut.append(elidedText(line, maxWidth, lineSize, Qt::ElideRight));
				lineSizesOut.append(lineSize);
				if (lineSize.width() > maxLineWidth)
					maxLineWidth = lineSize.width();

				// get the next line
				if (!nextLine.isEmpty())
				{
					linesOut.append(elidedText(nextLine, maxWidth, nextLineSize, Qt::ElideRight));
					lineSizesOut.append(nextLineSize);
					if (nextLineSize.width() > maxLineWidth)
						maxLineWidth = nextLineSize.width();
				}
			}

			return QSize(maxLineWidth, linesOut.size() * lineSpacing);
		}
		else if (hasFlag(FilenameUnTruncated))
		{
			// if the text is within the max bounds then just return it
			// Note: we ignore the line height 
			if (width <= maxWidth)
			{
				linesOut.append(srcText);
				lineSizesOut.append(QSize(textRect.size().width(), height));
				return textRect.size();
			}
			
			// break the text into two lines at a reasonable point and then 
			// truncate them appropriately
			int nextIndex = 0;
			QString line;
			QSize lineSize;
			TokenBoundary nextBoundary = getNextToken(srcText, 0, _fontMetrics, maxWidth, nextIndex, line, lineSize);
			if (nextBoundary == EndOfStringBoundary)
			{
				linesOut.append(line);
				lineSizesOut.append(lineSize);
				maxLineWidth = lineSize.width();
			}
			else if (nextBoundary == NormalBoundary) 
			{
				// if the line is going to be broken up into too small a string, then 
				// just make it a single string
				QString nextLine = srcText.mid(nextIndex);
				QSize nextLineSize = _fontMetrics.boundingRect(QRect(), 0, nextLine).size();
				if (nextLineSize.width() < (maxWidth / 2))
				{
					line = srcText;
					nextLine.clear();
				}

				// elide the text by the max width
				linesOut.append(elidedText(line, 2*maxWidth, lineSize, Qt::ElideMiddle));
				lineSizesOut.append(lineSize);
				if (lineSize.width() > maxLineWidth)
					maxLineWidth = lineSize.width();

				// get the next line
				if (!nextLine.isEmpty())
				{
					linesOut.append(elidedText(nextLine, 2*maxWidth, nextLineSize, Qt::ElideMiddle));
					lineSizesOut.append(nextLineSize);
					if (nextLineSize.width() > maxLineWidth)
						maxLineWidth = nextLineSize.width();
				}
			}

			return QSize(maxLineWidth, linesOut.size() * lineSpacing);
		}
		else
		{
			int maxHeight = 0;
			recurseOnLine(srcText, 0, _fontMetrics, maxWidth, 0, linesOut, lineSizesOut, false);
			for (int i = 0; i < lineSizesOut.size(); ++i) {
				if (lineSizesOut[i].width() > maxLineWidth)
					maxLineWidth = lineSizesOut[i].width();
				if (lineSizesOut[i].height() > maxHeight)
					maxHeight = lineSizesOut[i].height();
			}
			if (linesOut.size() == 2 && hasFlag(FlexibleMaxBounds))
			{   // if 2nd line is fairly short, join it with 1st
				if (lineSizesOut[1].width() + lineSizesOut[0].width() < maxLineWidth * 4 / 3)
				{
					linesOut[0] += linesOut[1];
					lineSizesOut[0].rwidth() += lineSizesOut[1].width();
					lineSizesOut.removeLast();
					linesOut.removeLast();
					maxLineWidth = lineSizesOut[0].width();
				}
			}
			return QSize(maxLineWidth, linesOut.size() * maxHeight);
		}
	}
	return QSize();
}

bool TextPixmapBuffer::setFlags(unsigned int flags)
{
	if (_flags != flags)
	{
		_flags = flags;
		_isDirty = true;
		return true;
	}
	return false;
}

bool TextPixmapBuffer::setTextAlignment(Qt::Alignment alignment)
{
	if (_textAlignment != alignment)
	{
		_textAlignment = alignment;
		_isDirty = true;
		return true;
	}
	return false;
}

bool TextPixmapBuffer::setOrthographic(bool isOrtho)
{
	if (_isOrthographic != isOrtho)
	{
		_isOrthographic = isOrtho;
		_isDirty = true;
		return true;
	}
	return false;
}

bool TextPixmapBuffer::pushFlag(TextPixmapBufferFlag flag)
{
	if (!(_flags & flag))
	{
		_flags |= flag;
		_isDirty = true;
		return true;
	}
	return false;
}

bool TextPixmapBuffer::popFlag(TextPixmapBufferFlag flag)
{
	if (_flags & flag)
	{
		_flags &= ~flag;
		_isDirty = true;
		return true;
	}
	return false;
}

bool TextPixmapBuffer::hasFlag(TextPixmapBufferFlag flag) const
{
	return (_flags & flag) == flag;
}

bool TextPixmapBuffer::setMaxBounds(const QSize& maxTextBounds)
{
	if (_maxTextBounds != maxTextBounds)
	{
		_maxTextBounds = maxTextBounds;
		_isDirty = true;
		return true;
	}
	return false;
}

bool TextPixmapBuffer::setColor(const QColor& c)
{
	if (_topColor != c || _bottomColor != c)
	{
		_topColor = _bottomColor = c;
		_isDirty = true;
		return true;
	}
	return false;
}

bool TextPixmapBuffer::setGradientColors(const QColor& top, const QColor& bottom)
{
	bool valueChanged = false;
	if (_topColor != top)
	{
		_topColor = top;
		_isDirty = true;
		valueChanged = true;
	}
	if (_bottomColor != bottom)
	{
		_bottomColor = bottom;
		_isDirty = true;
		valueChanged = true;
	}
	return valueChanged;
}

bool TextPixmapBuffer::setFont(const QFont& font)
{
	if (_font != font)
	{
		_font = font;
		_font.setKerning(false);
		_fontMetrics = QFontMetrics(_font);
		_isDirty = true;
		return true;
	}
	return false;
}

bool TextPixmapBuffer::setText(const QString& text)
{
	if (_text != text)
	{
		if (_useCache) _cache.clear();
		_text = text;
		_isDirty = true;
		return true;
	}
	return false;
}

const QSize& TextPixmapBuffer::getActualSize() const
{
	return _textBounds;
}

const QString& TextPixmapBuffer::getText() const
{
	return _text;
}

#ifdef DXRENDER
IDirect3DTexture9 * TextPixmapBuffer::bindAsGLTexture(Vec3& maxUVsOut) 
{
	if (!_glTextureId && 0 < _buffer.width() && 0 < _buffer.height())
		_glTextureId = dxr->createTextureFromData(_buffer.width(), _buffer.height(), _buffer.bits(), _buffer.bytesPerLine(), false, 1);
	dxr->device->SetTexture(0, _glTextureId);
#else
unsigned int TextPixmapBuffer::bindAsGLTexture(Vec3& maxUVsOut) //Vec3& pixelSizeOut, Vec3& uvSizeOut)
{
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
			if (hasFlag(ForceLinearFiltering))
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			}
			else
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			}
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
	maxUVsOut = Vec3((float) _textBounds.width() / _buffer.width(), 
		(float) _textBounds.height() / _buffer.height(), 0.0f);
	return _glTextureId;
}

#ifdef DXRENDER
void TextPixmapBuffer::onRelease()
{
	SAFE_RELEASE(_glTextureId);
}
#endif


void TextPixmapBuffer::update()
{
	update(_buffer);
}

const QImage& TextPixmapBuffer::getBuffer() const
{
	return _buffer;
}