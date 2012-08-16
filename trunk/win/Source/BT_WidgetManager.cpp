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
#include "BT_WidgetManager.h"
#include "BT_FileSystemActor.h"
#include "BT_FileSystemManager.h"
#include "BT_SceneManager.h"
#include "BT_WindowsOS.h"
#include "BT_Logger.h"
#include "BT_TextManager.h"
#include "BT_RenderManager.h"
#include "BT_Util.h"

// ----------------------------------------------------------------------------

void Widget::onFileAdded( QString strFileName )
{
	QFileInfo info(strFileName);
	if (info.dir() == QDir(workingDirectory))
		scnManager->onFileAdded(strFileName);
}

void Widget::onFileRemoved( QString strFileName )
{
	QFileInfo info(strFileName);
	if (info.dir() == QDir(workingDirectory))
		scnManager->onFileRemoved(strFileName);
}

void Widget::onFileNameChanged( QString strOldFileName, QString strNewFileName )
{
	QFileInfo info(strOldFileName);
	if (info.dir() == QDir(workingDirectory))
		scnManager->onFileNameChanged(strOldFileName, strNewFileName);
}

void Widget::onFileModified( QString strFileName )
{
	// check if it was the widget properties that was modified
	if (widgetPropertiesPath == strFileName)
	{
		// reload the widget
		widgetManager->reloadWidgetActorOverrides(this, true);
		textManager->invalidate();
		rndrManager->invalidateRenderer();
	}
	else
	{
		QFileInfo info(strFileName);
		if (info.dir() == QDir(workingDirectory))
			scnManager->onFileModified(strFileName);
	}
}

vector<QString> Widget::getWatchDir()
{
	return watchDirs;
}

bool Widget::isWidgetOverrideActor( FileSystemActor * actor )
{
	QString actorPath = actor->getFullPath();
	for (int i = 0; i < actorOverrides.size(); ++i)
	{
		if (fsManager->isIdenticalPath(actorPath, actorOverrides[i].filePath))
			return true;
	}
	return false;
}

void Widget::launchWidgetOverride(FileSystemActor * actor)
{
	QString actorPath = actor->getFullPath();
	for (int i = 0; i < actorOverrides.size(); ++i)
	{
		if (fsManager->isIdenticalPath(actorPath, actorOverrides[i].filePath))
		{
			if (!actorOverrides[i].launchOverride.isEmpty())
				fsManager->launchFile(actorOverrides[i].launchOverride, actorOverrides[i].launchOverrideParams);
		}
	}	
}

QString Widget::getWidgetOverrideLabel(FileSystemActor * actor)
{
	QString actorPath = actor->getFullPath();
	for (int i = 0; i < actorOverrides.size(); ++i)
	{
		if (fsManager->isIdenticalPath(actorPath, actorOverrides[i].filePath))
		{
			return actorOverrides[i].label;
		}
	}	
	return dummyLabel;
}

float Widget::getWidgetOverrideScale( FileSystemActor * actor )
{
	QString actorPath = actor->getFullPath();
	for (int i = 0; i < actorOverrides.size(); ++i)
	{
		if (fsManager->isIdenticalPath(actorPath, actorOverrides[i].filePath))
		{
			return actorOverrides[i].scale;
		}
	}	
	return 1.0f;
}

Widget::Widget()
: hProcess(NULL)
{}


// ----------------------------------------------------------------------------

WidgetManager::WidgetManager()
: _loaded(false)
{
	// XXX: create the widget directory if it does not already exist
}


Json::Value WidgetManager::getValue(QString keyPath)
{
	if (_loaded)
	{
		// otherwise, reload the value from the json node hierarchy

		QStringList tokens = keyPath.split(".");
		Json::Value node = _root;
		for (int i = 0; i < tokens.size(); ++i)
		{
			// ensure valid key path
			if (!node.isMember(stdString(tokens[i])))
			{
				QString err = QT_TR_NOOP("No such key path exists in the current widget:\n") + keyPath;
				::MessageBox(winOS->GetWindowsHandle(), (LPCTSTR) err.utf16(), (LPCWSTR)QT_TR_NOOP("BumpTop Widget Error").utf16(), MB_OK | MB_ICONERROR);
				throw invalid_argument("No such key path");
			}

			node = node[stdString(tokens[i])];
		}
	
		return node;
	}
	return Json::Value();
}

bool WidgetManager::isWidgetDirectory( QString filepath ) const
{
	QString expectedDirSuffix(QT_NT(".widget"));
	if (filepath.size() > expectedDirSuffix.size())
	{
		QString lowerFilePath = filepath.toLower();

		return (filepath.indexOf(expectedDirSuffix) == (filepath.size() - expectedDirSuffix.size()));
	}
	return false;
}

Widget * WidgetManager::loadWidgetProperties(QDir widgetPath, QFileInfo widgetPropertiesPath)
{
	// load the json file and fill in a new Widget structure
	QString widgetDescStr = read_file_utf8(native(widgetPropertiesPath));
	
	// load the description file
	LOG("Parsing widget json file...");
	QByteArray tmp = widgetDescStr.toUtf8();
	_loaded = _reader.parse(tmp.constData(), _root);
	if (_loaded)
	{
		Widget * w = new Widget;
		try 
		{
			// XXX: validate the schema

			// fill the widget
			QString app = qstringFromValue(getValue("widget.startupApplication.path").asString());
			if (!app.isEmpty())
				w->application = native(make_file(widgetPath, app));
			w->applicationParams = qstringFromValue(getValue("widget.startupApplication.args").asString());
			w->widgetPropertiesPath = native(widgetPropertiesPath);
			w->widgetDirectory = native(widgetPath);
			QDir workingDir = native(widgetPath / qstringFromValue(getValue("widget.relativeWorkingDirectory").asString()));
			w->workingDirectory = native(workingDir);	
			if (!reloadWidgetActorOverrides(w, false))
				throw runtime_error("Invalid actor overrides");

			w->watchDirs.push_back(w->workingDirectory);
			w->watchDirs.push_back(w->widgetDirectory);

			return w;
		} 
		catch (...)
		{
			delete w;
		}
	}
	else
	{
		// notify the user
		static bool userNotified = false;
		if (!userNotified)
		{
			::MessageBox(NULL, (LPCWSTR)QT_TR_NOOP("Could not load one or more widgets").utf16(), (LPCWSTR)QT_TR_NOOP("Widget goes boom!").utf16(), MB_OK);
			userNotified = true;
		}
	}

	return NULL;
}

void WidgetManager::TerminateExistingWidgetProcesses()
{
	QDir widgetPath = winOS->GetWidgetsDirectory();
	QString widgetPathStr = native(widgetPath);

	TCHAR szFilename[MAX_PATH];
	DWORD processIDs[1024];
	DWORD cbNeeded = 0;
	int numProcesses = 0;

	// try and enum all processes
	if (!EnumProcesses(processIDs, sizeof(processIDs), &cbNeeded))
		return;

	// determine how many process id's were returned
	numProcesses = cbNeeded / sizeof(DWORD);

	// save process handle <-> filename
	for (unsigned int i = 0; i < numProcesses; ++i)
	{
		if (processIDs[i])
		{
			// Get a handle to the process.
			HANDLE hProcess = OpenProcess( PROCESS_TERMINATE | PROCESS_QUERY_INFORMATION |
				PROCESS_VM_READ,
				FALSE, processIDs[i] );

			// Get the process name.
			if (NULL != hProcess )
			{
				HMODULE hMod = 0;

				if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded))
				{
					GetModuleFileNameEx(hProcess, hMod, szFilename, sizeof(szFilename)/sizeof(TCHAR));
					QString filename = QString::fromUtf16((const ushort *) szFilename);
					if (filename.startsWith(widgetPathStr, Qt::CaseInsensitive))
					{
						// terminate the process
						TerminateProcess(hProcess, 1);
						continue;
					}
				}

				CloseHandle(hProcess);
			}
		}
	}
}

void WidgetManager::initializeWidget(Widget * w)
{
	if (!w->application.isEmpty())
	{
		// start the process associated with this widget
		// see http://msdn.microsoft.com/en-us/library/ms682512(VS.85).aspx
		STARTUPINFO si = {0};
			si.cb = sizeof(STARTUPINFO);
		PROCESS_INFORMATION pi = {0};

		// Start the child process. 
		if( !CreateProcess((LPCTSTR) w->application.utf16(),		// No module name (use command line)
			(LPWSTR) w->applicationParams.utf16(), // Command line
			NULL,							// Process handle not inheritable
			NULL,							// Thread handle not inheritable
			FALSE,							// Set handle in heritance to FALSE
			CREATE_DEFAULT_ERROR_MODE,      // No creation flags
			NULL,							// Use parent's environment block
			(LPWSTR) w->workingDirectory.utf16(), // Use parent's starting directory 
			&si,							// Pointer to STARTUPINFO structure
			&pi)							// Pointer to PROCESS_INFORMATION structure
			) 
		{
			QString error = QT_TR_NOOP("Could not load widget: %1").arg(w->application);
			::MessageBox(winOS->GetWindowsHandle(), (LPCTSTR) error.utf16(), (LPCWSTR)QT_TR_NOOP("Could not load widget!").utf16(), MB_OK | MB_ICONWARNING);
			return;
		}

		CloseHandle(pi.hThread);

		// save the process' process handle
		w->hProcess = pi.hProcess;
	}

	// add to list of active widgets
	_widgets.push_back(w);

	// start watching the widget's working directory
	fsManager->addObject(w);
}

void WidgetManager::uninitializeWidget(Widget * w)
{
	// stop watching the widget's working directory
	fsManager->removeObject(w);

	// TerminateProcess this widget's process
	if (!w->application.isEmpty() && w->hProcess)
		TerminateProcess(w->hProcess, 0);
	
	// delete this widget
	_widgets.erase(find(_widgets.begin(), _widgets.end(), w));
	SAFE_DELETE(w);
}

void WidgetManager::initializeActiveWidgets()
{
	TerminateExistingWidgetProcesses();

	if (_widgets.empty())
	{
		// loop through the widgets directory and find any widgets in that folder
		QDir widgetsDir = winOS->GetWidgetsDirectory();
		vector<QString> widgets = fsManager->getDirectoryContents(native(widgetsDir), "*");
		for (int i = 0; i < widgets.size(); ++i)
		{
			// ensure that the widget is valid
			QDir widgetPath(widgets[i]);
			QFileInfo widgetPropertiesPath(widgetPath, QT_NT("widget.json"));
			if (QFileInfo(widgetPath.absolutePath()).isDir() && isWidgetDirectory(widgets[i]) && exists(widgetPropertiesPath))
			{
				// create the widget
				Widget * w = loadWidgetProperties(widgetPath, widgetPropertiesPath);
				if (w)
				{
					initializeWidget(w);
				}
			}
		}
	}
}

void WidgetManager::uninitializeActiveWidgets()
{
	while (!_widgets.empty())
	{
		uninitializeWidget(_widgets.front());
	}
}

const vector<Widget *>& WidgetManager::getActiveWidgets() const
{
	return _widgets;
}

Widget * WidgetManager::getActiveWidgetForFile( QString filepath ) const
{
	for (int i = 0; i < _widgets.size(); ++i)
	{
		QString widgetWorkingDir = _widgets[i]->workingDirectory;
		if (filepath.startsWith(widgetWorkingDir, Qt::CaseInsensitive))
		{
			// the file is located in a particular widget's working directory, so
			// we assume that it is managed by that widget
			return _widgets[i];
		}
	}
	return NULL;
}

bool WidgetManager::reloadWidgetActorOverrides( Widget * w, bool reloadProperties )
{
	if (reloadProperties)
	{
		// re-read the properties file
		QString widgetDescStr = read_file_utf8(w->widgetPropertiesPath);

		// load the description file
		LOG("Parsing widget json file...");
		QByteArray tmp = widgetDescStr.toUtf8();
		_loaded = _reader.parse(tmp.constData(), _root);
	}

	if (_loaded)
	{
		try 
		{
			// read all the actor overrides
			w->actorOverrides.clear();
			Json::Value iconOverrides = getValue("widget.icons");
			for (int i = 0; i < iconOverrides.size(); ++i)
			{
				WidgetActorOverride ovr;					
					ovr.filePath = native(make_file(w->workingDirectory, qstringFromValue(iconOverrides[i]["filename"])));
					QString launchApp = qstringFromValue(iconOverrides[i]["launchApplication"]["path"]);
					if (!launchApp.isEmpty())
						ovr.launchOverride = native(make_file(w->widgetDirectory, launchApp));
					ovr.launchOverrideParams = qstringFromValue(iconOverrides[i]["launchApplication"]["args"]);
					ovr.label = qstringFromValue(iconOverrides[i]["label"]);
					ovr.scale = -1.0f;
					if (iconOverrides[i].isMember("scale"))
					{
						ovr.scale = NxMath::min(6.0f, (float) iconOverrides[i]["scale"].asDouble());
					}

				// update scene objects depending on override
				//  - move the object if it already exists
				if (reloadProperties)
				{
					vector<FileSystemActor *> actors = scnManager->getFileSystemActors(ovr.filePath, false, false);
					for (int j = 0; j < actors.size(); ++j)
					{
						actors[j]->setText(ovr.label);
						Vec3 actorDims = actors[j]->getDims();
						float aspect = (actorDims.x / actorDims.y);
						Vec3 dims(GLOBAL(settings).xDist, GLOBAL(settings).zDist / aspect, GLOBAL(settings).yDist);
						if (ovr.scale > 0.0f)
						{
							dims *= ovr.scale;
							actors[j]->setSizeAnim(actors[j]->getDims(), dims, 25);
							
						}
					}
				}

				w->actorOverrides.push_back(ovr);
			}

			return true;
		} 
		catch (...)
		{}
	}
	return false;
}