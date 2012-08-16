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
#include "BT_UpdateInstallerTest.h"
#include "BT_UpdateInstaller.h"

CPPUNIT_TEST_SUITE_REGISTRATION(UpdateInstallerTest);



void UpdateInstallerTest::testBasic()
{

	MockUpdateInstaller *u = new MockUpdateInstaller(".");
	u->runInstaller();

	// using a "runas" verb causes crash; no worries, Vista automatically prompts for UAC privileges
	CPPUNIT_ASSERT_EQUAL(QString(), u->_shellExecuteVerb); 
	CPPUNIT_ASSERT_EQUAL(QString("msiexec.exe"), u->_shellExecuteExe); 

}

MockUpdateInstaller::MockUpdateInstaller( QString installPath )
: UpdateInstaller(installPath)
{}

BOOL MockUpdateInstaller::ShellExecute( DWORD& dwRet , int nShow , QString tcVerb, QString tcExe , QString tcWorkingDir /*= NULL */, QString tcArgs /*= NULL */ )
{
	_shellExecuteExe = tcExe;
	_shellExecuteVerb = tcVerb;
	return TRUE;
}

#endif // BT_UTEST