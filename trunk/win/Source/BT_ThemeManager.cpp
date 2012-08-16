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
#include "BT_DialogManager.h"
#include "BT_FontManager.h"
#include "BT_FileSystemManager.h"
#include "BT_Logger.h"
#include "BT_QtUtil.h"
#include "BT_SceneManager.h"
#include "BT_ThemeManager.h"
#include "BT_Util.h"
#include "BT_WindowsOS.h"

ThemeManager::ThemeManager()
: _loaded(false)
, _maxCacheLimit(25)
{}

ThemeManager::~ThemeManager()
{
	_loaded = false;
	_handlers.clear();
}

bool ThemeManager::reloadDefaultTheme(bool notifyAllHandlers)
{
	LOG("ThemeManager::reloadDefaultTheme");

	QDir backupDefaultThemePath = winOS->GetThemesDirectory(true);
	QDir userThemesPath = winOS->GetUserThemesDirectory(false);
	QDir userDefaultThemePath = winOS->GetUserThemesDirectory(true);
	QFileInfo userDefaultThemePropertiesPath = make_file(userDefaultThemePath, "theme.json");
	LOG(native(backupDefaultThemePath));
	LOG(native(userThemesPath));
	LOG(native(userDefaultThemePath));
	LOG(native(userDefaultThemePropertiesPath));

	// if no themes exist, then copy over the default theme
	bool copiedDefaultTheme = false;
	if (!exists(userDefaultThemePath) || 
		fsManager->getDirectoryContents(native(userDefaultThemePath)).empty())
	{
		fsManager->deleteFileByName(native(userDefaultThemePath), true);
		fsManager->copyFileByName(native(backupDefaultThemePath), 
			native(userThemesPath), QString(), false, true, false);
		copiedDefaultTheme = true;
	}

	// if the user's theme exists, then try and load it first
	if (exists(userDefaultThemePropertiesPath))
	{
		QString userDefaultThemePropertiesStr = read_file_utf8(native(userDefaultThemePropertiesPath));
		_reader.parse(userDefaultThemePropertiesStr.toUtf8().constData(), _root);
	}

	// now merge the default theme back into it (in case there were issues)
	// XXX: should we sae a backup of the theme properties first?
	bool mergedValues = mergeValues(userDefaultThemePropertiesPath, userDefaultThemePath, backupDefaultThemePath);

	// if we were copying over the default theme, then what we really want to do is
	// merge the language specific theme.json with the normal theme.json and then 
	// remove the language specific theme.json.  (we do the same when we extract
	// the themes, we merge with the default language theme.json files?)
	// 
	// on langage change, if there is a theme language override available in the
	// default themes dir, then merge it with the current theme?
	if (copiedDefaultTheme)
	{
		QFileInfo backupDefaultLangThemePropertiesPath = make_file(backupDefaultThemePath, QString("theme.%1.json").arg(winOS->GetLocaleLanguage()));
		if (exists(backupDefaultLangThemePropertiesPath))
		{			
			// override the root values
			Json::Value tmpRoot;
			QString backupDefaultLangThemePropertiesStr = read_file_utf8(native(backupDefaultLangThemePropertiesPath));
			_reader.parse(backupDefaultLangThemePropertiesStr.toUtf8().constData(), tmpRoot);
			overrideValuesRecursive(tmpRoot, _root);

			// save the new root
			Json::StyledWriter jsonWriter;
			QString outStr = QString::fromUtf8(jsonWriter.write(_root).c_str());
			write_file_utf8(outStr, native(userDefaultThemePropertiesPath));
		}
	}

	// clear the cache
	_cache.clear();

	// notify all theme event handlers of the change
	if (notifyAllHandlers)
		notifyOnChange();

	// NOTE: it should be always loaded if we are merging the default theme
	_loaded = true;
	return _loaded;
}

bool ThemeManager::overrideValuesRecursive(const Json::Value& from, Json::Value& to)
{
	vector<std::string> fromMembers = from.getMemberNames();
 	for (int i = 0; i < fromMembers.size(); ++i)
	{
		const std::string& memberName = fromMembers[i];

		if (!to.isMember(memberName))
			to[memberName] = from[memberName];
		else
		{
			if (from[memberName].isObject())
			{
				overrideValuesRecursive(from[memberName], to[memberName]);
			}
			else
			{
				to[memberName] = from[memberName];
			}
		}
	}
	return true;
}

bool ThemeManager::validateValuesRecursive(const Json::Value& root, const Json::Value& schemaRoot, QString pathToRoot)
{
	vector<std::string> schemaMembers = schemaRoot.getMemberNames();
	for (int i = 0; i < schemaMembers.size(); ++i)
	{
		const std::string& memberName = schemaMembers[i];
		std::string pathToMember = stdString(pathToRoot);
			pathToMember += "." + memberName;

		// ensure the child exists
		if (!root.isMember(memberName))
			throw invalid_argument("Missing value: " + pathToMember);

		// and is of the correct type
		if (root[memberName].isBool() != schemaRoot[memberName].isBool() ||
			root[memberName].isInt() != schemaRoot[memberName].isInt() ||
			root[memberName].isUInt() != schemaRoot[memberName].isUInt() ||
			root[memberName].isIntegral() != schemaRoot[memberName].isIntegral() ||
			root[memberName].isDouble() != schemaRoot[memberName].isDouble() ||
			root[memberName].isNumeric() != schemaRoot[memberName].isNumeric() ||
			root[memberName].isString() != schemaRoot[memberName].isString() ||
			root[memberName].isArray() != schemaRoot[memberName].isArray() ||
			root[memberName].isObject() != schemaRoot[memberName].isObject())
		{
			throw invalid_argument("Incorrect value type: " + pathToMember);
		}

		// and is of the correct size
		// override for font family checks
		if (memberName != "family")
		{
			if (root[memberName].size() != schemaRoot[memberName].size())
				throw invalid_argument("Incorrect value size: " + pathToMember);
		}

		// recurse down to that member
		if (root[memberName].isObject())
			validateValuesRecursive(root[memberName], schemaRoot[memberName], QString::fromStdString(pathToMember));
	}
	return true;
}

bool ThemeManager::mergeValues( QFileInfo themeDescPath, QDir defaultThemePath, QDir backupThemePath )
{
	// load the default theme properties
	QString line;
	QFileInfo backupThemeDescPath = make_file(backupThemePath, "theme.json");
	assert(exists(backupThemeDescPath));

	QString backupThemeDescStr = read_file_utf8(native(backupThemeDescPath));

	// load the description file
	Json::Value backupRoot;
	QByteArray tmp = backupThemeDescStr.toUtf8();
	_reader.parse(tmp.constData(), backupRoot);	

	// merge the backup theme properties to the current theme properties
	bool result = mergeValuesRecursive(_root, backupRoot, defaultThemePath, backupThemePath);

	// ensure that the schema is copied over, now that they are merged since this is the only 
	// property which will fail validation if every other actual property is correct
	_root["header"]["schema"] = backupRoot["header"]["schema"];

	// save the new root
	Json::StyledWriter jsonWriter;
	QString outStr = QString::fromUtf8(jsonWriter.write(_root).c_str());
	write_file_utf8(outStr, native(themeDescPath));

	return result;
}

bool ThemeManager::mergeValuesRecursive( Json::Value& to, const Json::Value& from, QDir toRootPath, QDir fromRootPath )
{
	// handle the textures blocks differently
	bool hasMerged = false;
	bool validateTextures = false;
	vector<std::string> members = from.getMemberNames();
	vector<std::string>::iterator iter = find(members.begin(), members.end(), "relativeRoot");
	if (iter != members.end())
	{
		const std::string& memberName = *iter;

		// copy over the relative root member if it does not exist
		if (!to.isMember(memberName))
			to[memberName] = from[memberName];

		// create new paths for subsequent recursions
		fromRootPath = fromRootPath / qstring(from[memberName].asString());
		toRootPath = toRootPath / qstring(to[memberName].asString());

		// remove the item
		members.erase(iter);

		// enable validation of textures (so that missing textures are filled)
		validateTextures = true;
	}

	// now, for the rest of the items
	for (int i = 0; i < members.size(); ++i)
	{
		const std::string memberName = members[i];
		bool replaceMember = false;
		bool hasMergedChild = false;

		// recurse down to that member
		if ((from[memberName].isObject())&&(to[memberName].isObject())) {
			if(to.isMember(memberName))
				hasMergedChild = mergeValuesRecursive(to[memberName], from[memberName], toRootPath, fromRootPath);
			else {
				hasMergedChild = true;
				to[memberName] = from[memberName];
			}
		}

		
		if (!to.isMember(memberName))
		{
			// replace if it does not exist
			replaceMember = true;
		}
		else
		{
			if (from[memberName].isBool() != to[memberName].isBool() ||
				from[memberName].isInt() != to[memberName].isInt() ||
				from[memberName].isUInt() != to[memberName].isUInt() ||
				from[memberName].isIntegral() != to[memberName].isIntegral() ||
				from[memberName].isDouble() != to[memberName].isDouble() ||
				from[memberName].isNumeric() != to[memberName].isNumeric() ||
				from[memberName].isString() != to[memberName].isString() ||
				from[memberName].isArray() != to[memberName].isArray() ||
				from[memberName].isObject() != to[memberName].isObject())
			{
				// replace if they are not of the same type
				replaceMember = true;
			}

			if (!hasMergedChild)
			{
				if (memberName != "family" && memberName != "ext")
				{
					if (from[memberName].size() != to[memberName].size())
					{
						// replace if they are not of the same size
						// (and it's not a variable member)
						replaceMember = true;
					}
				}
			}
		}
		if (replaceMember)
			to[memberName] = from[memberName];

		// if we need to validate the textures, then do so now
		if (validateTextures)
		{
			// copy the texture over if it does not exist
			if (from[memberName].isString())
			{
				QFileInfo toTexture = make_file(toRootPath, qstring(to[memberName].asString()));
				QFileInfo fromTexture = make_file(fromRootPath, qstring(from[memberName].asString()));
				if (!exists(toTexture))
				{
					QFile::copy(native(fromTexture), native(toTexture));
				}
			}
		}

		hasMerged = hasMerged || replaceMember;
	}
	return hasMerged;
}

bool ThemeManager::validateThemeAgainstSchema( const Json::Value& root )
{
	// check if the schema in the user's theme directory is the expected 
	// version associated with this version of bumptop

	// load the schema we are checking against and merge that to the user's theme properties
	Json::Value schemaRoot;
	QFileInfo schemaPath = make_file(winOS->GetThemesDirectory(false), "theme.schemas");

	QString schemaStr = read_file_utf8(native(schemaPath));
	QByteArray tmp = schemaStr.toUtf8();
	if (!_reader.parse(tmp.constData(), schemaRoot))
		throw invalid_argument("Could not load the Theme Schema Definitions");

	// ensure that the schema class exists
	std::string schemaClass = schemaRoot["expectedSchema"].asString();
	if (!schemaRoot.isMember(schemaClass))
		throw invalid_argument("Could not find the specified Schema Class: " + schemaClass);
	schemaRoot = schemaRoot[schemaClass];

	// ensure that the schema is the same
	if (root["header"]["schema"].asString() != schemaClass)
		throw runtime_error("Out-of-date theme detected");

	// recursively validate the root against the schema root
	return validateValuesRecursive(root, schemaRoot, QString());
}

Json::Value ThemeManager::getValue(QString keyPath)
{
	if (_loaded)
	{
		// check if the item is in the cache, and return that if possible
		CacheContainer::iterator iter = _cache.find(keyPath);
		if (iter != _cache.end())
		{
			iter.value().first++;
			return iter.value().second;
		}
		else
		{
			// otherwise, reload the value from the json node hierarchy

			QStringList tokens = keyPath.split(".");
			Json::Value node = _root;
			for (int i = 0; i < tokens.size(); ++i)
			{
				// ensure valid key path
				if (!node.isMember(stdString(tokens[i])))
				{
					QString err = QT_TR_NOOP("No such key path exists in the current theme:\n") + keyPath;
					::MessageBox(winOS->GetWindowsHandle(), (LPCTSTR) err.utf16(), (LPCWSTR)QT_TR_NOOP("BumpTop Theme Error").utf16(), MB_OK | MB_ICONERROR);
					#ifdef BTDEBUG
						throw invalid_argument("No such key path");
					#endif
					return Json::Value();
				}

				node = node[stdString(tokens[i])];
			}

			// update the cache
			cacheValue(keyPath, node);
	
			return node;
		}
	}
	return Json::Value();
}

bool ThemeManager::getValueAsBool(QString keyPath, bool defaultVal)
{
	Json::Value val = getValue(keyPath);
	if(val == Json::Value())
		return defaultVal;
	else
		return val.asBool();
}

int ThemeManager::getValueAsInt(QString keyPath, int defaultVal)
{
	Json::Value val = getValue(keyPath);
	if(val == Json::Value())
		return defaultVal;
	else
		return val.asInt();
}

double ThemeManager::getValueAsDouble(QString keyPath, double defaultVal)
{
	Json::Value val = getValue(keyPath);
	if(val == Json::Value())
		return defaultVal;
	else
		return val.asDouble();
}

const char * ThemeManager::getValueAsCString(QString keyPath, const char * defaultVal)
{
	Json::Value val = getValue(keyPath);
	if(val == Json::Value())
		return defaultVal;
	else
		return val.asCString();
}

uint ThemeManager::getValueAsUInt(QString keyPath, uint defaultVal)
{
	Json::Value val = getValue(keyPath);
	if(val == Json::Value())
		return defaultVal;
	else
		return val.asUInt();
}

ColorVal ThemeManager::getValueAsColor(QString keyPath, ColorVal defaultVal)
{
	Json::Value val = getValue(keyPath);
	assert(val.isArray() && (val.size() == 4));
	unsigned int index = 0;

	if(val == Json::Value())
		return defaultVal;
	else
		return ColorVal(val[index + 3].asUInt(), val[index].asUInt(), 
			val[index + 1].asUInt(), val[index + 2].asUInt());
}

QString ThemeManager::getValueAsQString(QString keyPath, QString defaultVal)
{
	Json::Value val = getValue(keyPath);
	if(val == Json::Value())
		return defaultVal;
	else
		return qstringFromValue(val);
}

QString ThemeManager::getValueAsFontFamilyName(QString keyPath, QString defaultVal)
{
	Json::Value val = getValue(keyPath);
	assert(val.isArray() && (val.size() > 0));

	if(val == Json::Value())
		return defaultVal;
	else {
		for (unsigned int i = 0; i < val.size(); ++i)
		{
			QString fontName = qstringFromValue(val[i]);
			if (fontManager->containsFont(fontName))
				return fontName;
		}
		return fontManager->getSystemFont(10).fontName;
	}
}

void ThemeManager::notifyOnChange()
{
	set<ThemeEventHandler *>::iterator iter = _handlers.begin();
	while (iter != _handlers.end())
	{
		(*iter)->onThemeChanged();
		iter++;
	}
}

void ThemeManager::registerThemeEventHandler( ThemeEventHandler * handler )
{
	assert(handler);
	_handlers.insert(handler);
	// notify it on add, so that it can reload if necessary
	handler->onThemeChanged();
}

void ThemeManager::unregisterThemeEventHandler( ThemeEventHandler * handler )
{
	// NOTE: one of the issues with using singletons is that we have no control
	//		 (the way it's currently implemented) over it's destruction order, 
	//		 which means that other singletons that depend on classes such as the
	//		 theme manager must specify an explicit dependency.  In this case, 
	//		 dependent classes which call unregisterThemeEventHandler need to 
	//		 do so from a weak_ptr reference to ensure that if the theme manager
	//		 is already deleted, that we do not experience heap errors.
	assert(handler);
	if (_handlers.find(handler) != _handlers.end())
		_handlers.erase(handler);
}

void ThemeManager::cacheValue( QString keyPath, const Json::Value& value )
{
	// check if the item already exists in the cache, if so, then just increment
	// the hit counter for that entry
	CacheContainer::iterator hitIter = _cache.find(keyPath);
	if (hitIter != _cache.end())
	{
		hitIter.value().first++;
	}
	else
	{
		// check if we will exceed the max cache limit, if so, then remove the 
		// least cached item
		int initialCount = 1;
		if (_cache.size() == _maxCacheLimit)
		{
			unsigned int leastUsedCount = (1 << 31);
			QString leastUsedKeyPath;
			CacheContainer::iterator iter = _cache.begin();
			while (iter != _cache.end())
			{
				if (iter.value().first < leastUsedCount)
				{
					leastUsedKeyPath = iter.key();
					leastUsedCount = iter.value().first;
				}
				iter++;
			}
			_cache.remove(leastUsedKeyPath);
			initialCount = leastUsedCount + 1;
		}

		// add the item to the cache and increment it
		_cache.insert(keyPath, make_pair(initialCount, value));
	}
}