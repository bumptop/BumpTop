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
#include "BT_Logger.h"
#include "BT_SceneManager.h"
#include "BT_WindowsOS.h"
#include "BT_QtUtil.h"

Logger::Logger()
{
	_filepath = native(winOS->GetDataDirectory() / "log.txt");
	_isLogging = false;
	_verbose = false;
	openLog();
}

Logger::Logger(QString filePath)
{
	_filepath = filePath;
	_isLogging = false;
	_verbose = false;
	openLog();
}

Logger::~Logger()
{
	// Some log calls still access the _ignoreSourceFiles dictionary and cause a crash,
	// so set logging to false to short circuit out the access.
	setIsLogging(false);
	closeLog();
}

void Logger::openLog()
{
	_logFile.setFileName(getFilepath());
	QIODevice::OpenMode openFlags = QFile::WriteOnly | QFile::Truncate;
	//clear log if bigger than 1MB
	if (_logFile.open(QFile::ReadOnly))
	{
		if (_logFile.bytesAvailable() > 1024 * 1024)
			openFlags = QFile::WriteOnly | QFile::Truncate;
		_logFile.close();
	}

	if (_logFile.open(openFlags))
	{
		// set the codec (it will use the local code page otherwise) for decoding the text stream
		_logStream = new QTextStream(&_logFile);
		_logStream->setCodec("UTF-8");
		*_logStream << "\r\nLOG_START \t" << QDateTime::currentDateTime().toString() << "\r\n\r\n";
		_logStream->flush();
	}
	else
		_logStream = NULL;
}

void Logger::setIsLogging(bool value)
{
	_isLogging = value;
}

void Logger::setVerboseLogging(bool value)
{
	_verbose = value;
}

bool Logger::isLogging(const QString & sourceFile, StartOrEnd isStart)
{
	return _isLogging && !(isStart != UNDEFINED && _ignoreStartOrEnd) && !_ignoreSourceFiles[sourceFile] ;
}

bool Logger::isVerbose()
{
	return _verbose;
}

void Logger::ignoreSourceFile(const QString & sourceFile)
{
	_ignoreSourceFiles[sourceFile] = true;
}

void Logger::ignoreStartOrEnd(bool ignoreStartOrEnd)
{
	_ignoreStartOrEnd = ignoreStartOrEnd;
}

void Logger::log(QString message)
{
	log(message, UNDEFINED);
}

void Logger::log(QString message, StartOrEnd isStart)
{
	if (_isLogging && _logStream)
	{
		switch (isStart)
		{
		case UNDEFINED: 
			break;
		case START: 
			*_logStream << "START \t";
			break;
		case FINISH: 
			*_logStream << "FINISH \t";
			break;
		}

		*_logStream << message << endl;
		_logStream->flush();
	}
}

void Logger::logException(const char * file, int line, unsigned long code)
{
	if (_logStream)
	{
		*_logStream << "EXCEPTION \t" << file << "(" << line << "):" << code << endl;
		_logStream->flush();
	}
}

void Logger::logAssert(const char * file, int line)
{
	printf("!!!\n ASSERT_FAIL \t %s(%d) \n!!!\n", file, line);
	if (_logStream)
	{
		*_logStream << "ASSERT_FAIL \t" << file << "(" << line << ")" << endl;
		_logStream->flush();
	}
}

void Logger::closeLog()
{
	if (_logStream)
	{
		*_logStream << "LOG_END \t" << QDateTime::currentDateTime().toString() << "\r\n\r\n";
		_logStream->flush();
		_logFile.close();
		SAFE_DELETE(_logStream);
	}
}

QString Logger::getFilepath()
{
	return _filepath;
}

GUIDLogger::GUIDLogger() : Logger(native(winOS->GetDataDirectory() / "guid_log.txt"))
{
}

TestLogger::TestLogger() : Logger(native((scnManager->runAutomatedJSONTestsOnStartup) ? scnManager->JSONTestLogPath / "test_log.txt" : winOS->GetTestsDirectory() / "test_log.txt"))
{
}