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

#ifndef BT_WIDGETMANAGER
#define BT_WIDGETMANAGER

#include "BT_Singleton.h"
#include "BT_Watchable.h"

// ----------------------------------------------------------------------------

// SECURITY ISSUES: can people just be tricked by downloading a file and having
//	it launch now with the start of bumptop?  Does windows/av software do special 
//	detection of startup apps that would miss bumptop?
//
// If bumptop crashes, what happens to that process? (don't run it if already running? OR 
//	kill existing processes and reload them since we don't know their context (prompt user))
// 
// Can these widget apps launch other applications that won't be caught by bumptop when 
//	shutting down?
// 
// Do we need a shutdown application?

// ----------------------------------------------------------------------------

class FileSystemActor;

// ----------------------------------------------------------------------------

class WidgetActorOverride
{
public:
	QString filePath;
	QString launchOverride;
	QString launchOverrideParams;
	QString label;
	float scale;
};

class Widget : public Watchable
{
public:
	vector<QString> watchDirs;
	QString dummyLabel;
	QString widgetPropertiesPath;
	QString widgetDirectory;
	QString workingDirectory;
	QString application;
	QString applicationParams;
	vector<WidgetActorOverride> actorOverrides;

	HANDLE hProcess;

public:
	Widget();
	
	// accessors
	bool isWidgetOverrideActor(FileSystemActor * actor);
	void launchWidgetOverride(FileSystemActor * actor);
	QString getWidgetOverrideLabel(FileSystemActor * actor);
	float getWidgetOverrideScale(FileSystemActor * actor);

	// watchable overrides
	virtual void onFileAdded(QString strFileName);
	virtual void onFileRemoved(QString strFileName);
	virtual void onFileNameChanged(QString strOldFileName, QString strNewFileName);
	virtual void onFileModified(QString strFileName);
	virtual vector<QString> getWatchDir();
};

// ----------------------------------------------------------------------------

class WidgetManager
{
	Q_DECLARE_TR_FUNCTIONS(WidgetManager)

	Json::Value _root;
	Json::Reader _reader;
	bool _loaded;

	vector<Widget *> _widgets;

	// singleton
	friend class Singleton<WidgetManager>;
	WidgetManager();

private:
	Widget * loadWidgetProperties(QDir widgetPath, QFileInfo widgetPropertiesPath);
	void initializeWidget(Widget * w);
	void uninitializeWidget(Widget * w);
	bool isWidgetDirectory(QString filepath) const;

	Json::Value getValue(QString keyPath);

	void TerminateExistingWidgetProcesses();

public:
	// operations
	void initializeActiveWidgets();
	void uninitializeActiveWidgets();
	bool reloadWidgetActorOverrides(Widget * w, bool reloadProperties);

	// accessors
	const vector<Widget *>& getActiveWidgets() const;
	Widget * getActiveWidgetForFile(QString filepath) const; 
};

// ----------------------------------------------------------------------------

#define widgetManager Singleton<WidgetManager>::getSharedInstance()

// ----------------------------------------------------------------------------

#endif // BT_WIDGETMANAGER