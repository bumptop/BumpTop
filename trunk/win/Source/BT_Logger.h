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

#ifndef _BT_LOGGER_
#define _BT_LOGGER_

#include "BT_Singleton.h"

enum StartOrEnd
{
	UNDEFINED = NULL,
	START,
	FINISH,
};

class Logger
{
protected:
	QTextStream		* _logStream;
	QFile			_logFile;
	bool			_isLogging; //global logging, except for assertion fails which are always logged
	bool			_verbose;
	QString			_filepath;

	//set to true to disable logging for a source file, file name must be Visual Studio __FILE__ styled such as ".\BT_WindowsOS.cpp"
	QHash<QString, bool> _ignoreSourceFiles; 
	bool			_ignoreStartOrEnd;

	void openLog();

public:
	Logger();
	Logger(QString filePath);

	~Logger();
	void	setIsLogging(bool value);
	void	setVerboseLogging(bool value);
	bool	isLogging(const QString & sourceFile, StartOrEnd isStart);
	bool	isVerbose();

	//set to true to disable logging for a source file, file name must be Visual Studio __FILE__ styled such as ".\BT_WindowsOS.cpp"
	void	ignoreSourceFile(const QString & sourceFil);
	void	ignoreStartOrEnd(bool ignoreStartOrEnd);

	void	closeLog();

	QString getFilepath();

	virtual void log(QString message);
	virtual void log(QString message, StartOrEnd isStart);
	
	void logException(const char * file, int line, unsigned long code);
	void logAssert(const char * file, int line);
};

#define logger Singleton<Logger>::getInstance()
#define LOG(message) if (logger->isLogging(__FILE__, UNDEFINED)) logger->log(message)
#define LOG_START(message) if (logger->isLogging(__FILE__, START))logger->log(message, START)
#define LOG_FINISH(message) if (logger->isLogging(__FILE__, FINISH)) logger->log(message, FINISH)
#define LOG_STATEMENT(statement) LOG_START(#statement);statement;LOG_FINISH(#statement)
#define LOG_FUNCTION_REACHED() if (logger->isLogging(__FILE__, UNDEFINED)) logger->log(QString(__FUNCTION__));
#define LOG_LINE_REACHED_IMP(file, line) if (logger->isLogging(file, UNDEFINED)) logger->log(file"("MAKE_STRING_A(line)"):reached")
#define LOG_LINE_REACHED()	LOG_LINE_REACHED_IMP(__FILE__, __LINE__)
#define LOG_ASSERT(expr) ((void)(!(expr) ? logger->logAssert(__FILE__, __LINE__) : 0))

#define VASSERT(expr, message) assert(expr)

#if !defined BTDEBUG 
#define ASSERT(expr)	LOG_ASSERT(expr)
#define ASSERTE(expr)	ASSERT(expr)
#define assert(expr)	ASSERT(expr)
#define _ASSERT(expr)	ASSERT(expr)
#define _ASSERTE(expr)	ASSERT(expr)
#define VASSERT(expr, message) LOG_ASSERT(expr); if (!(expr) && logger->isLogging(__FILE__, UNDEFINED))logger->log(QString_NT("\t") + message)
#endif
// -----------------------------------------------------------------------------

class GUIDLogger : public Logger
{
public:
	GUIDLogger();
};

#define guidLogger Singleton<GUIDLogger>::getInstance()
#define GUID_LOG(message) if (guidLogger->isLogging(__FILE__, UNDEFINED)) guidLogger->log(message)

// -----------------------------------------------------------------------------

class TestLogger : public Logger
{
public:
	TestLogger();
};

#define testLogger Singleton<TestLogger>::getInstance()
#define TEST_LOG(message) if (testLogger->isLogging(__FILE__, UNDEFINED)) testLogger->log(message)

// -----------------------------------------------------------------------------

#else
	class Logger;
#endif // _BT_LOGGER_
