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
#ifdef BT_UTEST
#include "BT_UpdaterTest.h"
#include "BT_Updater.h"
#include "../Source/EnumProc.h"

// register the test suite with the anonymous registery
//CPPUNIT_TEST_SUITE_REGISTRATION(UpdaterTest);

void UpdaterTest::testVersionStringParsing()
{
	const int NUM_VERSIONS_STRING = 6;
	string versionStrings[NUM_VERSIONS_STRING] = 
	{
		"1", "abc", "-2", "234sc", "2", ""
	};

	int versions[NUM_VERSIONS_STRING] = 
	{
		1, -1, -2, 234, 2, -1
	};

	for (int i = 0; i < NUM_VERSIONS_STRING; i++)
	{
		string versionString = versionStrings[i];
		int version = versions[i];

		vector<string> versionStrings;
		versionStrings.push_back(versionString);
		UpdateServer *us = (UpdateServer*) new MockUpdateServer(versionStrings);


		CPPUNIT_ASSERT_EQUAL(versionString, us->getVersionString());

		Updater *u = new Updater(us, 0, ".", 0, 0, 0);
		if (version == -1)
			CPPUNIT_ASSERT_THROW(u->getNewestVersionNumber(), UpdateCheckFailedException);
		else
			CPPUNIT_ASSERT_EQUAL(version, u->getNewestVersionNumber());

		delete us;
		delete u;

	}



}


void UpdaterTest::testRun()
{
	vector<string> versionStrings;
	versionStrings.push_back("800");
	versionStrings.push_back("900");
	versionStrings.push_back("1000");
	versionStrings.push_back("Error, no connection");
	versionStrings.push_back("1100");
	versionStrings.push_back("1200");

	MockUpdateServer *mus = new MockUpdateServer(versionStrings);
	Updater *u = new Updater((UpdateServer*)mus, 1100, ".", 0, 0, 0);

	u->run();

	vector<bool> wasDownloadAttempted = mus->_recordedDownloadAttemptsAfterUpdateChecks;
	CPPUNIT_ASSERT_EQUAL((size_t)6, wasDownloadAttempted.size());
	CPPUNIT_ASSERT_EQUAL(false, (bool)wasDownloadAttempted[0]);
	CPPUNIT_ASSERT_EQUAL(false, (bool)wasDownloadAttempted[1]);
	CPPUNIT_ASSERT_EQUAL(false, (bool)wasDownloadAttempted[2]);
	CPPUNIT_ASSERT_EQUAL(false, (bool)wasDownloadAttempted[3]);
	CPPUNIT_ASSERT_EQUAL(false, (bool)wasDownloadAttempted[4]);
	CPPUNIT_ASSERT_EQUAL(true, (bool)wasDownloadAttempted[5]);

	delete mus;
	delete u;

}

void UpdaterTest::testRunTiming()
{
	vector<string> versionStrings;
	versionStrings.push_back("800");
	versionStrings.push_back("900");
	versionStrings.push_back("1000");
	versionStrings.push_back("Error, no connection");
	versionStrings.push_back("1100");
	versionStrings.push_back("1200");

	MockUpdateServer *mus = new MockUpdateServer(versionStrings);

	int currentVersion = 1100;
	uint retryTimeAfterFailedUpdateCheck = 50;
	uint timeBetweenUpdateChecks = 150;
	Updater *u = new Updater((UpdateServer*)mus,
		currentVersion,
		".",
		retryTimeAfterFailedUpdateCheck,
		timeBetweenUpdateChecks,
		0);

	u->run();

	vector<bool> wasDownloadAttempted = mus->_recordedDownloadAttemptsAfterUpdateChecks;
	CPPUNIT_ASSERT_EQUAL((size_t)6, wasDownloadAttempted.size());
	CPPUNIT_ASSERT_EQUAL(false, (bool)wasDownloadAttempted[0]);
	CPPUNIT_ASSERT_EQUAL(false, (bool)wasDownloadAttempted[1]);
	CPPUNIT_ASSERT_EQUAL(false, (bool)wasDownloadAttempted[2]);
	CPPUNIT_ASSERT_EQUAL(false, (bool)wasDownloadAttempted[3]);
	CPPUNIT_ASSERT_EQUAL(false, (bool)wasDownloadAttempted[4]);
	CPPUNIT_ASSERT_EQUAL(true, (bool)wasDownloadAttempted[5]);

	vector<uint> timeBetweenDownloadAttempts = mus->_recordedTimeForEachUpdateCheck;
	CPPUNIT_ASSERT_EQUAL((size_t)6, timeBetweenDownloadAttempts.size());
	CPPUNIT_ASSERT(timeBetweenDownloadAttempts[0] < 10);
	CPPUNIT_ASSERT(145 < timeBetweenDownloadAttempts[1] && timeBetweenDownloadAttempts[1] < 155);
	CPPUNIT_ASSERT(145 < timeBetweenDownloadAttempts[2] && timeBetweenDownloadAttempts[2] < 155);
	CPPUNIT_ASSERT(145 < timeBetweenDownloadAttempts[3] && timeBetweenDownloadAttempts[3] < 155);
	CPPUNIT_ASSERT(45 < timeBetweenDownloadAttempts[4] && timeBetweenDownloadAttempts[4] < 55);
	CPPUNIT_ASSERT(145 < timeBetweenDownloadAttempts[5] && timeBetweenDownloadAttempts[5] < 155);

}

void UpdaterTest::testIsUpdateDownloaded()
{
	if (exists("a_temporary_test_data_directory"))
	{
		remove("a_temporary_test_data_directory\\BumpTopInstaller.msi");
		remove("a_temporary_test_data_directory\\version.txt");
		remove("a_temporary_test_data_directory\\desc.txt");
	}

	vector<string> versionStrings;
	versionStrings.push_back("800");
	MockUpdateServer *mus = new MockUpdateServer(versionStrings);
	Updater *u = new Updater(mus, 99, "a_temporary_test_data_directory", 0, 0, 0);

	create_directory("a_temporary_test_data_directory");

	ofstream installer("a_temporary_test_data_directory//BumpTopInstaller.msi");
	installer << "!!!!!";
	installer.close();

	CPPUNIT_ASSERT_EQUAL(false, u->isUpdateDownloaded());

	ofstream versionFile("a_temporary_test_data_directory//version.txt");
	versionFile << "2000";
	versionFile.close();

	CPPUNIT_ASSERT_EQUAL(false, u->isUpdateDownloaded());

	ofstream descFile("a_temporary_test_data_directory//desc.txt");
	descFile << "best version ever";
	descFile.close();

	CPPUNIT_ASSERT_EQUAL(true, u->isUpdateDownloaded());

	remove("a_temporary_test_data_directory\\BumpTopInstaller.msi");
	remove("a_temporary_test_data_directory\\version.txt");
	remove("a_temporary_test_data_directory\\desc.txt");
	remove("a_temporary_test_data_directory");


}

void UpdaterTest::tearDown()
{
	remove("a_temporary_test_data_directory\\BumpTopInstaller.msi");
	remove("a_temporary_test_data_directory\\version.txt");
	remove("a_temporary_test_data_directory\\desc.txt");
	remove("a_temporary_test_data_directory");

	remove("version.txt");
	remove("desc.txt");
	remove("BumpTopInstaller.msi");

}

void UpdaterTest::testRegisteringUpdateMessage()
{
	vector<string> versionStrings;
	versionStrings.push_back("800");
	MockUpdateServer *mus = new MockUpdateServer(versionStrings);
	Updater *u = new Updater(mus, 99, ".", 0, 0, 0);

	CPPUNIT_ASSERT_EQUAL(u->getUpdateDownloadedMessageId(), u->getUpdateDownloadedMessageId());

	delete u;
	delete mus;
	// updater->getUpdateDownloadedMessage()
	// should be consistent, even if you call it twice

}

void UpdaterTest::testSendingUpdateMessageToBumpTop()
{
	// start up an instance of bumptop
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	ZeroMemory( &pi, sizeof(pi) );

#ifdef DEBUG
	path bumptopDirPath = current_path().branch_path() / path("Source/Debug", native);
#else
	path bumptopDirPath = current_path().branch_path() / path("Source/Release", native);
#endif

	path bumptopPath = bumptopDirPath / path("BumpTop.exe");


	// Start the child process. 
	if( !CreateProcess( NULL,   // No module name (use command line)
		(LPSTR) bumptopPath.native_file_string().c_str(),        // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		0,              // No creation flags
		NULL,           // Use parent's environment block
		(LPSTR) bumptopDirPath.native_directory_string().c_str(),           // Use parent's starting directory 
		&si,            // Pointer to STARTUPINFO structure
		&pi )           // Pointer to PROCESS_INFORMATION structure
		) 
	{
		ostringstream error_message;
		error_message << "CreateProcess failed (" <<  GetLastError() << ")\n";
		CPPUNIT_ASSERT_MESSAGE(error_message.str(), false);
	}

	Sleep(500);

	
	// Search for BumpTop window
	bool foundBumptopWindow = false;
	HWND bumptopHwnd = 0;

	for (int i = 0; i < 50; i++)
	{
		
		CWindowIterator cwi(1000);
		for (HWND hwnd = cwi.First(); hwnd; hwnd=cwi.Next())
		{
			if ((GetWindowLong(hwnd,GWL_STYLE) & WS_VISIBLE)) {
				DWORD pidwin;
				GetWindowThreadProcessId(hwnd, &pidwin);
				TCHAR classnameBuffer[MAX_PATH+1];
				GetClassName(hwnd, classnameBuffer, MAX_PATH);
				TCHAR titleBuffer[MAX_PATH+1];
				GetWindowText(hwnd, titleBuffer, MAX_PATH);
				
				if (pidwin==pi.dwProcessId && string(classnameBuffer) == string("BumpTop") && string(titleBuffer) == string("BumpTop"))
				{
					foundBumptopWindow = true;
					bumptopHwnd = hwnd;
					break;
				}
			}
		}
		if (foundBumptopWindow)
			break;

		Sleep(100); // give the window a little time to start

	}
	
	
	CPPUNIT_ASSERT_MESSAGE("Testing if we found the BumpTop window", foundBumptopWindow);

	Sleep(500);
	vector<string> versionStrings;
	versionStrings.push_back("800");
	MockUpdateServer *mus = new MockUpdateServer(versionStrings);
	Updater *u = new Updater(mus, 99, ".", 0, 0, bumptopHwnd);
	int updateDownloadedMessageResponse = u->sendUpdateDownloadedMessageToBumpTop(true);

	SendMessageTimeout(bumptopHwnd, WM_CLOSE, 0, 0, SMTO_NORMAL, 1000, NULL);
	
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	CPPUNIT_ASSERT_EQUAL(3, updateDownloadedMessageResponse);

}
string MockUpdateServer::getVersionString()
{
	return _currentVersionString;
}


MockUpdateServer::MockUpdateServer(vector<string> versionStrings)
{

	_numTimesUpdaterChecked = 0;
	_wasDownloadAttemptedSinceLastUpdateCheck = false;
	_versionStrings = versionStrings;
	_currentVersionString = versionStrings[0];
	_downloadAttemptTimer.restart();
}

bool MockUpdateServer::onUpdaterCheckHandler()
{
	_recordedTimeForEachUpdateCheck.push_back(_downloadAttemptTimer.restart());
	_recordedDownloadAttemptsAfterUpdateChecks.push_back(_wasDownloadAttemptedSinceLastUpdateCheck);
	_wasDownloadAttemptedSinceLastUpdateCheck = false;
	_numTimesUpdaterChecked++;
	if (_numTimesUpdaterChecked < _versionStrings.size())
	{
		_currentVersionString = _versionStrings[_numTimesUpdaterChecked];
		return true;
	}
	else
	{
		return false;
	}
}

void MockUpdateServer::downloadNewVersion()
{
	_wasDownloadAttemptedSinceLastUpdateCheck = true;
}

#endif