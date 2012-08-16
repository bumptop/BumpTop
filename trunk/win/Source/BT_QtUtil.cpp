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
#include "BT_QtUtil.h"
#include "BT_SceneManager.h"
#include "BT_WindowsOS.h"

// convenience functions for generating random numbers
void randomSeed()
{
	qsrand(time(NULL));
}
// for generating a random integer between the specified ranges [..)
// by definition, qrand() only returns values from 0..RAND_MAX
int randomInt(int lowerBound, int upperBound)
{
	assert(lowerBound < upperBound);	
	return (qrand() % (upperBound - lowerBound)) + lowerBound;
}

// convenience functions for rounding values to greater steps
float roundToNearest(float value, float step)
{
	float i = NxMath::floor(value / step) * step;
	float j = i + step;
	if ((value - i) < (j - value))
		return i;
	else
		return j;
}
float roundToNext(float value, float step)
{
	return NxMath::ceil(value / step) * step;
}

// for concatenating qdirectories like in boost::filesystem
QDir operator/(const QDir& lhs, const QDir& rhs) 
{ 
	return QDir(lhs.absolutePath() + QDir::separator() + rhs.absolutePath()); 
}
QDir operator/(const QDir& lhs, const QString& rhs) 
{ 
	return QDir(lhs.absolutePath() + QDir::separator() + rhs); 
}
QDir& operator/=(QDir& lhs, const QDir& rhs) 
{ 
	lhs.setPath((lhs/rhs).absolutePath()); 
	return lhs;
}

// native qstring representation of a qdir/qfileinfo
QString native(const QDir& dir)
{
	return QDir::toNativeSeparators(dir.absolutePath());
}
QString native(const QFile& f)
{
	return QDir::toNativeSeparators(QFileInfo(f).absoluteFilePath());
}
QString native(const QFileInfo& p)
{
	return QDir::toNativeSeparators(p.absoluteFilePath());
}
QString native(const QString& p)
{
	return QDir::toNativeSeparators(p);
}

// convenience functions to check file existance
bool exists(QString path)
{
#ifdef WIN32
	return (TRUE == PathFileExists((LPCTSTR) path.utf16()));
#else
	return QFileInfo(path).exists();
#endif
}
bool exists( QFile path )
{
	return exists(native(path));
}
bool exists( QFileInfo path )
{
	return exists(native(path));
}
bool exists( QDir path )
{
	return exists(native(path));
}

// convenience functions to get the parent
QDir parent(const QDir& dir)
{
	return QFileInfo(dir.absolutePath()).dir();
}
QDir parent(const QFileInfo& path)
{
	return path.dir();
}
QDir parent(const QString& path)
{
	return QFileInfo(path).dir();
}

// convenience functions to get the filename
QString filename(const QString& path)
{
	return QFileInfo(path).fileName();
}

// Add quotes to a path string if it is not already quoted
void ensureQuoted(QString& pathStr)
{
	if (pathStr.isEmpty())
		return;

	pathStr = pathStr.trimmed();
	pathStr.replace("\"", "\\\"");
	if (!pathStr.endsWith("\"") && 
		!pathStr.startsWith("\""))
	{
		pathStr.prepend("\"");
		pathStr.append("\"");
	}
}
void ensureUnquoted(QString& pathStr)
{
	pathStr.replace("\"", "");
}

void ensurePathSlash( QString& pathStr )
{
	if (pathStr.isEmpty())
		return;

	pathStr = pathStr.trimmed();
	if (!pathStr.endsWith("\\") &&
		!pathStr.endsWith("/"))
#ifdef WIN32
		pathStr.append("\\");
#else
		pathStr.append("/");
#endif
}

// quick shorthand for converting std string to qstring
QString qstring(const std::string& str)
{
	return QString::fromUtf8(str.c_str());
}
std::string stdString(const QString& str)
{
	return std::string(str.toUtf8().constData()); 
}
// convenience function to convert a json string value to qstring
QString qstringFromValue(const Json::Value& val)
{
	assert(val.isString());
	return qstring(val.asString());
}

// creates a QFile object from a parent directory and a filename
QFileInfo make_file(const QDir& dir, const QString& name)
{
	return QFileInfo(dir, name);
}
QFileInfo make_file(const QString& dir, const QString& name)
{
	return QFileInfo(QDir(dir), name);
}

// creates a new directory with the given QDir
void create_directory(const QDir& dir)
{
	QDir().mkpath(dir.absolutePath());
}

// returns whether a QDir is empty
bool empty(const QDir& dir)
{
	return (dir == QDir());
}

// loads a file into a unicode QString
QString read_file(QString filename, const char * codec)
{
	// open the file for reading
	QFile file(filename);
	if (file.open(QFile::ReadOnly))
	{
		// set the codec (it will use the local code page otherwise) for decoding the text stream
		QTextStream stream(&file);
		if (codec)
			stream.setCodec(codec);
		assert(stream.status() == QTextStream::Ok);

		// read the contents
		QString contents = stream.readAll();

		// close the file
		file.close();

		return contents;
	}
	return QString();
}
QString read_file_utf8(QString filename)
{
	return read_file(filename, "UTF-8");
}

// checks a file to see if it starts with a specified header
bool validate_file_startswith(QString filename, const char * codec, QString header)
{
	// open the file for reading
	QFile file(filename);
	if (file.open(QFile::ReadOnly))
	{
		// set the codec (it will use the local code page otherwise) for decoding the text stream
		QTextStream stream(&file);
		if (codec)
			stream.setCodec(codec);
		assert(stream.status() == QTextStream::Ok);

		// read the contents
		QString headerStr = stream.read(header.size());

		// close the file
		file.close();

		return (headerStr == header);
	}
	return false;
}
bool validate_file_startswith_utf8(QString filename, QString header)
{
	return validate_file_startswith(filename, "UTF-8", header);
}

// writes a file from a unicode QString
bool write_file_utf8(QString contents, QString filename)
{
	// open the file for writing
	QFile file(filename);
	if (file.open(QFile::WriteOnly | QFile::Truncate))
	{
		// set the codec (it will use the local code page otherwise) for decoding the text stream
		QTextStream stream(&file);
		stream.setCodec("UTF-8");
		assert(stream.status() == QTextStream::Ok);

		// read the contents
		stream << contents;

		// close the file
		file.close();

		return true;
	}
	return false;
}
bool append_file_utf8(QString appendContents, QString filename)
{
	// open the file for writing
	QFile file(filename);
	if (file.open(QFile::WriteOnly | QIODevice::Append))
	{
		// set the codec (it will use the local code page otherwise) for decoding the text stream
		QTextStream stream(&file);
		stream.setCodec("UTF-8");
		assert(stream.status() == QTextStream::Ok);

		// read the contents
		QByteArray tmp = appendContents.toUtf8();
		stream << tmp.constData();

		// close the file
		file.close();

		return true;
	}
	return false;
}

// converts a string into proper case
QString make_proper_case(QString& str)
{
	QString tmp = str;
	bool makeUpper = true;
	for (int i = 0; i < tmp.size(); ++i)
	{
		if (tmp[i].isSpace())
			makeUpper = true;
		else if (makeUpper)
		{
			tmp[i] = tmp[i].toUpper();
			makeUpper = false;
		}
	}
	return tmp;
}

// Convert a Bounds object (assumed to be in screen co-ordinates) to a QRect
QRect boundsToQRect( Bounds bounds )
{
	Vec3 extents, center;
	bounds.getExtents(extents);
	bounds.getCenter(center);
	return QRect(
		QPoint(center.x - extents.x, center.y - extents.y),
		QPoint(center.x + extents.x, center.y + extents.y));
}

Translations::Translations()
: _translator(NULL)
, _multiTouchTranslator(NULL)
, _webTranslator(NULL)
{
	_translator = new QTranslator();
	QString locale; // = QLocale::system().name();
	winOS->LoadSettingsFile();
	locale = winOS->GetLocaleLanguage();

	// load the base translation file if there is one
	QString localeFileName = QString(QT_NT("BumpTop.%1.qm")).arg(locale);
	QString localeFile = native(make_file(winOS->GetLanguagesDirectory(), localeFileName));
	bool loaded = _translator->load(localeFile, native(winOS->GetExecutableDirectory()));
	assert(loaded);

	// load the multitouch override file if there is one
	int isTouchScreen = GetSystemMetrics(SM_TABLETPC);
	if (isTouchScreen)
	{
		_multiTouchTranslator = new QTranslator();
		localeFileName = QString(QT_NT("BumpTop_Multitouch.%1.qm")).arg(locale);
		localeFile = native(make_file(winOS->GetLanguagesDirectory(), localeFileName));
		if (exists(localeFile))
		{
			loaded = _multiTouchTranslator->load(localeFile, native(winOS->GetExecutableDirectory()));
			assert(loaded);
		}
	}

	// load the web-related translations
	_webTranslator = new QTranslator();
	localeFileName = QString(QT_NT("BumpTop_Web.%1.qm")).arg(locale);
	localeFile = native(make_file(winOS->GetLanguagesDirectory(), localeFileName));
	if (exists(localeFile))
	{
		loaded = _webTranslator->load(localeFile, native(winOS->GetExecutableDirectory()));
		assert(loaded);
	}
}

QString Translations::translate( const char * context, const char * str )
{
	// try loading from the multitouch string first
	if (_multiTouchTranslator)
	{
		QString qstr = _multiTouchTranslator->translate(context, str);
		if (!qstr.isEmpty())
			return qstr;
	}

	// load the normal string first
	QString qstr = _translator->translate(context, str);
	if (qstr.isEmpty())
		qstr = str;
	return qstr;
}

QString Translations::translateWeb( const char * context, const char * str )
{
	// try loading from the web widget string table first
	if (_webTranslator)
	{
		QString qstr = _webTranslator->translate(context, str);
		if (!qstr.isEmpty())
			return qstr;
	}
	return translate(context, str);
}
