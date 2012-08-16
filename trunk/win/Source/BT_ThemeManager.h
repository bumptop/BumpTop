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

#ifndef BT_THEMEMANAGER
#define BT_THEMEMANAGER

#include "BT_Singleton.h"
#include "BT_ColorVal.h"

// -----------------------------------------------------------------------------

class ThemeEventHandler
{
public:
	virtual void onThemeChanged() = 0;
};

// -----------------------------------------------------------------------------

class ThemeManager
{
	Q_DECLARE_TR_FUNCTIONS(ThemeManager)

	Json::Value	_root;
	Json::Reader _reader;
	bool _loaded;

	typedef QHash<QString, pair<unsigned int, Json::Value> > CacheContainer;
	CacheContainer _cache;
	unsigned int _maxCacheLimit;

	// events
	set<ThemeEventHandler *> _handlers;

	// Singleton
	friend class Singleton<ThemeManager>;
	ThemeManager();

private:
	bool validateThemeAgainstSchema(const Json::Value& root);
	bool validateValuesRecursive(const Json::Value& root, const Json::Value& schemaRoot, QString pathToRoot);
	bool mergeValuesRecursive(Json::Value& to, const Json::Value& from, QDir toRootPath, QDir fromRootPath);
	bool mergeValues(QFileInfo themeDescPath, QDir defaultThemePath, QDir backupThemePath);
	bool overrideValuesRecursive(const Json::Value& from, Json::Value& to);
	void cacheValue(QString keyPath, const Json::Value& value);
	void notifyOnChange();

public:
	~ThemeManager();

	// operations
	bool reloadDefaultTheme(bool notifyAllHandlers=true);
	void registerThemeEventHandler(ThemeEventHandler * handler);
	void unregisterThemeEventHandler(ThemeEventHandler * handler);

	// accessors
	Json::Value getValue(QString keyPath);

	// used to resolve arrays/compound properties as bt objects
	ColorVal getValueAsColor(QString keyPath, ColorVal defaultVal);
	QString getValueAsQString(QString keyPath, QString defaultVal);
	QString getValueAsFontFamilyName(QString keyPath, QString defaultVal);
	bool getValueAsBool(QString keyPath, bool defaultVal);
	int getValueAsInt(QString keyPath, int defaultVal);
	double getValueAsDouble(QString keyPath, double defaultVal);
	const char * getValueAsCString(QString keyPath, const char * defaultVal);
	uint getValueAsUInt(QString keyPath, uint defaultVal);
};

// -----------------------------------------------------------------------------

#define themeManager Singleton<ThemeManager>::getSharedInstance()

// -----------------------------------------------------------------------------

#endif // BT_THEMEMANAGER