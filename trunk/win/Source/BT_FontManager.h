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

#ifndef _FONT_MANAGER_
#define _FONT_MANAGER_

// -----------------------------------------------------------------------------

#include "BT_Singleton.h"

struct FontDescription
{
public:
	QString fontName;
	int fontSize;

public:
	FontDescription();
	FontDescription(QString name, int size);
};

// -----------------------------------------------------------------------------

class FontTypeMap
{
	QHash<QString, QString> _typeMapping;
	QHash<QString, QString> _fileMapping;

public:
	FontTypeMap();

	// operations
	QString getFilePathForFontFace(QString fontface);
	QString getFilePathForFontFile(QString fontfile);
	bool containsFontFace(QString fontface) const;
	QString firstFont() const;
	QList<QString> getFonts() const;
};
// -----------------------------------------------------------------------------

class FontManager
{	
	QFontDatabase _fontDatabase;
	QList<QString> _fontFamilies;
	QHash<QString, int> _fontIds;
	FontDescription _systemFont;

	// singleton; private ctor
	friend class Singleton<FontManager>;
	FontManager();

protected:
	QFont createFont(const QString& fontString, int fontSize);

public:
	~FontManager();

	// accessors
	QFont getFont(const FontDescription& description);
	const FontDescription& getSystemFont(int fontSize);		// the fallback font
	bool containsFont(const QString& rawFontString);		// whether this system supports a particular font

	// static 
	static float getDPIScale();
};

// -----------------------------------------------------------------------------

#define fontManager Singleton<FontManager>::getInstance()

// -----------------------------------------------------------------------------

#else
class FontManager;
#endif