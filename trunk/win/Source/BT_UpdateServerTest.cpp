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
#include "BT_UpdateServerTest.h"
#include "BT_UpdateServer.h"
#include "BT_Updater.h"

CPPUNIT_TEST_SUITE_REGISTRATION(UpdateServerTest);

void UpdateServerTest::testBasicDownload()
{
	UpdateServer* us = (UpdateServer*) new UpdateServerImpl(".");
	
	CPPUNIT_ASSERT(us->getVersionString() != "");

	// the updater will indirectly test for us... it throws an exception if the version
	// string isn't a proper number
	Updater* u = new Updater(us, 0, ".", 0, 0, 0);
	CPPUNIT_ASSERT_NO_THROW(u->getNewestVersionNumber());
}

void UpdateServerTest::testBumpTopDownload()
{
	if (exists("BumptopInstaller.msi"))
		remove("BumptopInstaller.msi");
	CPPUNIT_ASSERT(!exists("BumptopInstaller.msi"));

	// Test that this will overwrite a previously existing .exe
	ofstream myfile;
	myfile.open("BumptopInstaller.msi");
	myfile << "garbage";
	myfile.close();


	UpdateServer* us = (UpdateServer*) new UpdateServerImpl(".");
	cout << "Downloading BumpTop installer (this will take a while-- you might want to disable this test while developing)" << endl;
	CPPUNIT_ASSERT_NO_THROW(us->downloadNewVersion()); // this will take a while


	boost::uintmax_t fileSize = file_size("BumptopInstaller.msi");
	CPPUNIT_ASSERT(4000000 < fileSize && fileSize < 10000000); // between roughly 4MB and 10MB

}

void UpdateServerTest::testStagingBumpTopDownload()
{
	if (exists("BumptopInstaller.msi"))
		remove("BumptopInstaller.msi");
	CPPUNIT_ASSERT(!exists("BumptopInstaller.msi"));

	// Test that this will overwrite a previously existing .exe
	ofstream myfile;
	myfile.open("BumptopInstaller.msi");
	myfile << "garbage";
	myfile.close();


	UpdateServer* us = (UpdateServer*) new StagingUpdateServer(".");
	cout << "Downloading staging BumpTop installer (this will take a while-- you might want to disable this test while developing)" << endl;
	CPPUNIT_ASSERT_NO_THROW(us->downloadNewVersion()); // this will take a while


	boost::uintmax_t fileSize = file_size("BumptopInstaller.msi");
	CPPUNIT_ASSERT(4000000 < fileSize && fileSize < 10000000); // between roughly 4MB and 10MB
	
}

#endif
