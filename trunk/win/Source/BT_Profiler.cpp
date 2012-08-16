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
#include "BT_Profiler.h"
#include "BT_WindowsOS.h"
#include "BT_Util.h"
#include "BT_QtUtil.h"

Profiler::Profiler()
{
	_isProfiling = false;
	
	_code_block_names.push_back(QString(QT_NT("EventLoop")));
	_code_block_names.push_back(QString(QT_NT("EventLoop>Updates")));
	_code_block_names.push_back(QString(QT_NT("EventLoop>Windows message loop")));
	_code_block_names.push_back(QString(QT_NT("EventLoop>isRenderRequired")));
	_code_block_names.push_back(QString(QT_NT("EventLoop>Idle")));
	_code_block_names.push_back(QString(QT_NT("EventLoop>Misc")));
	_code_block_names.push_back(QString(QT_NT("EventLoop>Rendering")));

	_code_block_names.push_back(QString(QT_NT("RenderingSum")));
	_code_block_names.push_back(QString(QT_NT("RenderingSum>Actors")));
	_code_block_names.push_back(QString(QT_NT("RenderingSum>SwapBuffers")));
	_code_block_names.push_back(QString(QT_NT("RenderingSum>Slideshow")));
	_code_block_names.push_back(QString(QT_NT("RenderingSum>Background")));
	_code_block_names.push_back(QString(QT_NT("RenderingSum>Cage and interior walls")));
	_code_block_names.push_back(QString(QT_NT("RenderingSum>Lasso")));
	_code_block_names.push_back(QString(QT_NT("RenderingSum>InPileShifting separator")));
	_code_block_names.push_back(QString(QT_NT("RenderingSum>MultitouchOverlay separator")));
	_code_block_names.push_back(QString(QT_NT("RenderingSum>Pins")));
	_code_block_names.push_back(QString(QT_NT("RenderingSum>Overlays & Text")));
	_code_block_names.push_back(QString(QT_NT("RenderingSum>Menus")));
	_code_block_names.push_back(QString(QT_NT("RenderingSum>FPS")));
	_code_block_names.push_back(QString(QT_NT("RenderingSum>Misc")));
	
	_code_block_names.push_back(QString(QT_NT("Init")));
	_code_block_names.push_back(QString(QT_NT("Init>Load Settings")));
	_code_block_names.push_back(QString(QT_NT("Init>Init Scene Manager")));
	_code_block_names.push_back(QString(QT_NT("Init>Load Settings file 2")));
	_code_block_names.push_back(QString(QT_NT("Init>Load Stats")));
	_code_block_names.push_back(QString(QT_NT("Init>Reload default theme")));
	_code_block_names.push_back(QString(QT_NT("Init>texMgr initIL")));
	_code_block_names.push_back(QString(QT_NT("Init>InitNovodex")));
	_code_block_names.push_back(QString(QT_NT("Init>sysTray init")));
	_code_block_names.push_back(QString(QT_NT("Init>texMgr init")));
	_code_block_names.push_back(QString(QT_NT("Init>keyMgr")));
	_code_block_names.push_back(QString(QT_NT("Init>lassoMenu")));
	_code_block_names.push_back(QString(QT_NT("Init>registerMouseHndl")));
	_code_block_names.push_back(QString(QT_NT("Init>registerKeyHndl")));
	_code_block_names.push_back(QString(QT_NT("Init>menuMgrInit")));
	_code_block_names.push_back(QString(QT_NT("Init>constructMousePointer")));
	_code_block_names.push_back(QString(QT_NT("Init>setWindowAsForeground")));
	_code_block_names.push_back(QString(QT_NT("Init>ResetExplorerHost")));
	_code_block_names.push_back(QString(QT_NT("Init>Init widgets")));
	_code_block_names.push_back(QString(QT_NT("Init>Show Splash Screen")));
	_code_block_names.push_back(QString(QT_NT("Init>Init updater")));
	_code_block_names.push_back(QString(QT_NT("Init>rndrManger initGL")));
	_code_block_names.push_back(QString(QT_NT("Init>Create Window")));
	_code_block_names.push_back(QString(QT_NT("Init>Reload default theme")));
	_code_block_names.push_back(QString(QT_NT("Init>Register Main Window")));
	_code_block_names.push_back(QString(QT_NT("Init>Ensure Monitor Exists")));
	_code_block_names.push_back(QString(QT_NT("Init>EnumDisplayMonitors")));
	_code_block_names.push_back(QString(QT_NT("Init>FindWindowEx")));
	_code_block_names.push_back(QString(QT_NT("Init>InitNx")));
	_code_block_names.push_back(QString(QT_NT("Init>Reset Motion callback")));
	_code_block_names.push_back(QString(QT_NT("Init>Explorer Alive Checker")));
	_code_block_names.push_back(QString(QT_NT("Init>startRecycleBinUpdater")));
	_code_block_names.push_back(QString(QT_NT("Init>clearMessages")));
	_code_block_names.push_back(QString(QT_NT("Init>validateAuth1")));
	_code_block_names.push_back(QString(QT_NT("Init>validateAuth2")));
	_code_block_names.push_back(QString(QT_NT("Init>checkTrial")));
	_code_block_names.push_back(QString(QT_NT("Init>notAuthorized")));
	_code_block_names.push_back(QString(QT_NT("Init>saveSettingsFile")));
	_code_block_names.push_back(QString(QT_NT("Init>Find User App Data Dir")));


	_code_block_names.push_back(QString(QT_NT("InitNx")));
	_code_block_names.push_back(QString(QT_NT("InitNx>getStats")));
	_code_block_names.push_back(QString(QT_NT("InitNx>CreateWalls")));
	_code_block_names.push_back(QString(QT_NT("InitNx>Resize Walls")));
	_code_block_names.push_back(QString(QT_NT("InitNx>LoadSceneFromFile")));
	_code_block_names.push_back(QString(QT_NT("InitNx>ZoomToBounds")));

	_code_block_names.push_back(QString(QT_NT("LoadScene")));
	_code_block_names.push_back(QString(QT_NT("LoadScene>Header")));
	_code_block_names.push_back(QString(QT_NT("LoadScene>ExHeader")));
	_code_block_names.push_back(QString(QT_NT("LoadScene>Pile")));
	_code_block_names.push_back(QString(QT_NT("LoadScene>Actor")));
	_code_block_names.push_back(QString(QT_NT("LoadScene>Camera")));
	_code_block_names.push_back(QString(QT_NT("LoadScene>crossReference")));

	

	_repeated_code_names.push_back(QString(QT_NT("setFilePath")));
	//_code_block_names.push_back(QString(QT_NT("setFilePath>loadTexture")));
	//_code_block_names.push_back(QString(QT_NT("setFilePath>loadTexture2")));

	_repeated_code_names.push_back(QString(QT_NT("loadTexture")));
	_repeated_code_names.push_back(QString(QT_NT("loadThumbnail")));
	_repeated_code_names.push_back(QString(QT_NT("resolveShortcut")));
	_repeated_code_names.push_back(QString(QT_NT("unserializePile")));
	_repeated_code_names.push_back(QString(QT_NT("unserializeActor")));
	_repeated_code_names.push_back(QString(QT_NT("getStickyNote")));
	_repeated_code_names.push_back(QString(QT_NT("ReadMatrix")));
	_repeated_code_names.push_back(QString(QT_NT("ReadVec")));
	_repeated_code_names.push_back(QString(QT_NT("photoFrame")));
	_repeated_code_names.push_back(QString(QT_NT("customActor")));
	_repeated_code_names.push_back(QString(QT_NT("initNewActor")));
	_repeated_code_names.push_back(QString(QT_NT("syncStickyNoteWithFileContents")));
	_repeated_code_names.push_back(QString(QT_NT("postSetFilePath")));
	_repeated_code_names.push_back(QString(QT_NT("unserializeDone")));
	
	for_each(QString code_block_name, _code_block_names)
	{
		_code_blocks[code_block_name] = 0;
	}

	for_each(QString code_block_name, _repeated_code_names)
	{
		_repeated_code_blocks[code_block_name] = 0;
	}

}

Profiler::~Profiler()
{
}

void Profiler::incrementTime(QString code_block_name, uint ms)
{
	if (_isProfiling && _code_blocks.find(code_block_name) != _code_blocks.end())
	{
		_code_blocks[code_block_name] += ms;

		if (code_block_name.contains('>'))
		{
			QStringList split_code_block_name = code_block_name.split(">");
			_code_blocks[split_code_block_name[0]] += ms;
		}
		
	}
	else if (_isProfiling)
	{
		assert(false);
	}
}

void Profiler::incrementRepeated(QString code_block_name, uint ms)
{
	_timer.pause();
	_repeated_code_names.push_back(code_block_name);
	_repeated_code_blocks[code_block_name] = ms;

	if (_isProfiling)
	{
		if (code_block_name.contains('>'))
		{
			QStringList split_code_block_name = code_block_name.split(">");
			//consoleWrite(split_code_block_name[0] + " : " + QString::number(_repeated_code_blocks[split_code_block_name[0]]) + " : " + QString::number(ms));
			_repeated_code_blocks[split_code_block_name[0]] += ms;
		}

	}
	else if (_isProfiling)
	{
		assert(false);
	}
	_timer.unpause();
}

void Profiler::start()
{
	_timer.restart();
	_isProfiling = true;
	_numFrames = 0;
}

void Profiler::onFrame()
{
	if (_isProfiling)
	{
		_numFrames++;

	}
}

bool Profiler::isProfiling()
{
	return _isProfiling;
}

#define cout_and_file(stream, data) cout << data;  stream << data
void Profiler::stop(bool ignoreNumFrames)
{
	QString logFilePath = native(make_file(winOS->GetDataDirectory(), "performance.log"));
	QFile file(logFilePath);
	int width = 70;
	if (file.open(QFile::WriteOnly | QFile::Truncate))
	{
		// set the codec (it will use the local code page otherwise) for decoding the text stream
		QTextStream stream(&file);
		assert(stream.status() == QTextStream::Ok);

		uint profiling_interval = _timer.restart();
		cout_and_file(stream, "Profiling interval (ms):" << profiling_interval << endl);
		cout_and_file(stream, "Number of frames:" << _numFrames << endl);

		double numFramesDouble = (double) _numFrames;
		if (ignoreNumFrames)
			numFramesDouble = 1;
		for_each(QString code_block_name, _code_block_names)
		{
			uint ms = _code_blocks[code_block_name];
			if (code_block_name.contains('>'))
				continue;

			cout << setw(width) << left << code_block_name.toAscii().constData() << setw(20) << left << ms / numFramesDouble << "\n";
			stream << qSetFieldWidth(width) << left << code_block_name << qSetFieldWidth(20) << left << ms / numFramesDouble  << "\n";

			for_each(QString code_block_name2, _code_block_names)
			{
				if (code_block_name2.contains(code_block_name + '>'))
				{
					uint ms = _code_blocks[code_block_name2];
					QStringList split_code_block_name = code_block_name2.split(">");

					cout << setw(width) << left << "     " + std::string(split_code_block_name[1].toAscii().constData()) << setw(20) << left << ms / numFramesDouble  << "\n";
					stream << qSetFieldWidth(width) << left << "     " + split_code_block_name[1] << qSetFieldWidth(20) << left << ms / numFramesDouble << "\n";

					_code_blocks[code_block_name2] = 0;
				}
			}

			_code_blocks[code_block_name] = 0;
		}

		for_each(QString code_block_name, _repeated_code_names)
		{
			uint ms = _repeated_code_blocks[code_block_name];
			if (code_block_name.contains('>'))
				continue;

			cout << setw(width) << left << code_block_name.toAscii().constData() << setw(20) << left << ms / numFramesDouble << "\n";
			stream << qSetFieldWidth(width) << left << code_block_name << qSetFieldWidth(20) << left << ms / numFramesDouble << "\n";

			for_each(QString code_block_name2, _repeated_code_names)
			{
				if (code_block_name2.contains(code_block_name + '>'))
				{
					uint ms = _repeated_code_blocks[code_block_name2];
					QStringList split_code_block_name = code_block_name2.split(">");

					cout << setw(width) << left << "     " + std::string(split_code_block_name[1].toAscii().constData()) << setw(20) << left << ms / numFramesDouble << "\n";
					stream << qSetFieldWidth(width) << left << "     " + split_code_block_name[1] << qSetFieldWidth(20) << left << ms / numFramesDouble << "\n";

					_repeated_code_blocks[code_block_name2] = 0;
				}
			}

			_repeated_code_blocks[code_block_name] = 0;
		}

		// close the file
		file.close();
	}

	_numFrames = 0;
	_isProfiling = false;
}