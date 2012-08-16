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

#ifndef _BT_UPDATER_TEST_
#define _BT_UPDATER_TEST_
#ifdef BT_UTEST

#include "BT_Updater.h"
#include "BT_TestCommon.h"

using CppUnit::TestFixture;




/*
* FileTransfer tests
*/
class UpdaterTest : public TestFixture
{
	CPPUNIT_TEST_SUITE( UpdaterTest );
	CPPUNIT_TEST(testVersionStringParsing);
	CPPUNIT_TEST(testRun);
	CPPUNIT_TEST(testRunTiming);
	CPPUNIT_TEST(testIsUpdateDownloaded);
	CPPUNIT_TEST(testRegisteringUpdateMessage);
	CPPUNIT_TEST(testSendingUpdateMessageToBumpTop);
	CPPUNIT_TEST_SUITE_END();

private:

public:
	// fixture setup 
	//void setUp();
	void tearDown();

	// tests
	void testVersionStringParsing();
	void testRun();
	void testRunTiming();
	void testIsUpdateDownloaded();
	void testRegisteringUpdateMessage();
	void testSendingUpdateMessageToBumpTop();
};

class MockUpdateServer : UpdateServer
{
	friend class UpdaterTest;
protected:
	int _numTimesUpdaterChecked;


	string _currentVersionString;
	vector<string> _versionStrings;

	bool _wasDownloadAttemptedSinceLastUpdateCheck;
	vector<bool> _recordedDownloadAttemptsAfterUpdateChecks;

	Stopwatch _downloadAttemptTimer;
	vector<uint> _recordedTimeForEachUpdateCheck;
public:
	MockUpdateServer(vector<string> versionStrings);
	string getVersionString();

	bool onUpdaterCheckHandler();
	void downloadNewVersion();
};


#endif // BT_UTEST
#endif // _BT_UPDATER_TEST_