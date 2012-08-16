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

#ifndef _BT_UPDATEINSTALLER_TEST_
#define _BT_UPDATEINSTALLER_TEST_
#ifdef BT_UTEST
#include "BT_UpdateInstaller.h"
#include "BT_TestCommon.h"

using CppUnit::TestFixture;



class UpdateInstallerTest : public TestFixture
{
	CPPUNIT_TEST_SUITE( UpdateInstallerTest );
	CPPUNIT_TEST(testBasic);
	CPPUNIT_TEST_SUITE_END();

private:

public:
	// fixture setup 
	//void setUp();
	//void tearDown();

	// tests
	void testBasic();
};

class MockUpdateInstaller : public UpdateInstaller
{
public:
	MockUpdateInstaller(QString installPath);

	vector<QString> _removedFiles;
	QString _shellExecuteVerb;
	QString _shellExecuteExe;

	virtual BOOL ShellExecute( DWORD& dwRet , int nShow , QString tcVerb, QString tcExe , QString tcWorkingDir /*= NULL */, QString tcArgs /*= NULL */ );




};


#endif // BT_UTEST
#endif // _BT_UPDATEINSTALLER_TEST_
