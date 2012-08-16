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
#include "BT_FontManager.h"
#include "BT_WindowsOS.h"
#include "BT_Util.h"
#include "BT_SceneManager.h"
#include "BT_FileSystemManager.h"
#include "BT_Logger.h"

//
// FontDescription implementation
//
FontDescription::FontDescription()
: fontSize(0)
{}

FontDescription::FontDescription( QString name, int size )
: fontName(name)
, fontSize(size) 
{}

//
// FontManager implementation
//
bool CmpSizeGreaterThan(const QString &s1, const QString &s2)
{
	return s1.size() > s2.size();
}

FontManager::FontManager()
{}

FontManager::~FontManager()
{}

QFont FontManager::createFont( const QString& fontString, int fontSize )
{
	// certain fonts require scaling down...
	if (fontString.startsWith("calibri", Qt::CaseInsensitive) ||
		fontString.startsWith("candara", Qt::CaseInsensitive) || 
		fontString.startsWith("consolas", Qt::CaseInsensitive) ||
		fontString.startsWith("verdana", Qt::CaseInsensitive))
	{
		--fontSize;
	}

	// check if it's in the database first
	if (_fontFamilies.contains(fontString))
		return QFont(fontString, fontSize);

	// otherwise, check if we have a mapping from a font file to font family
	QString fontPath = native(make_file(winOS->GetSystemPath(FontsDirectory), fontString));
	int fontId = -1;
	if (_fontIds.contains(fontString))
		fontId = _fontIds[fontString];
	else
	{
		fontId = _fontDatabase.addApplicationFont(fontPath);
		_fontIds.insert(fontString, fontId);
		_fontFamilies = _fontDatabase.families();
		
		// sort the font families in descending size so that the search later is correct
		qSort(_fontFamilies.begin(), _fontFamilies.end(), CmpSizeGreaterThan);
	}
	if (fontId > -1)
	{
		QList<QString> families = _fontDatabase.applicationFontFamilies(fontId);
		return QFont(families.front(), fontSize);
	}

	// XXX: can cache this as well
	// XXX: what if we can't find the font?

	// otherwise, it's not a font file, not a direct font family representation
	//   (ie. Arial Bold) so we should go through the list and try and find out
	//   what it is.
	QListIterator<QString> iter(_fontFamilies);
	QString family;
	while (iter.hasNext())
	{
		QString tmpFamily = iter.next();
		if (fontString.startsWith(tmpFamily) && 
			(tmpFamily.size() > family.size()))
			family = tmpFamily;			
	}

	if (!family.isEmpty())
		return QFont(family, fontSize, _fontDatabase.weight(family, fontString), 
			_fontDatabase.italic(family, fontString));
	else
		return QFont();
}

QFont FontManager::getFont(const FontDescription& description)
{
	return createFont(description.fontName, description.fontSize);
}

const FontDescription& FontManager::getSystemFont(int fontSize)
{
	_systemFont = FontDescription(_fontDatabase.font("Arial", "", fontSize).family(), fontSize);
	return _systemFont;
}

bool FontManager::containsFont(const QString& rawFontString)
{
	if (createFont(rawFontString, 10) == QFont())
		return false;
	return true;
}

float FontManager::getDPIScale()
{
	static int dpiX = QApplication::desktop()->logicalDpiX();
	// int pdpiX = QApplication::desktop()->physicalDpiX();
	static int expectedDpi = 96;
	return float(dpiX) / expectedDpi;
}
