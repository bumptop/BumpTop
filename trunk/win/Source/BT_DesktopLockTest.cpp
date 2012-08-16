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
#include "BT_DesktopLockTest.h"
#include "BT_DesktopLock.h"
#include "BT_WindowsOS.h"

CPPUNIT_TEST_SUITE_REGISTRATION(DesktopLockTest);

void DesktopLockTest::testBasic()
{

	DesktopLock *firstLock, *secondLock;
	firstLock = new DesktopLock("desktop.test.lock");
	CPPUNIT_ASSERT(firstLock->tryLock());
	CPPUNIT_ASSERT(firstLock->tryLock());
	//firstLock->tryLock();
	//CPPUNIT_ASSERT_EQUAL((DWORD)0, GetLastError());
	CPPUNIT_ASSERT(firstLock->isLocked());
	secondLock = new DesktopLock("desktop.test.lock");
	CPPUNIT_ASSERT(!secondLock->tryLock());
	CPPUNIT_ASSERT(firstLock->isLocked());
	firstLock->unlock();
	CPPUNIT_ASSERT(!firstLock->isLocked());
	CPPUNIT_ASSERT(secondLock->tryLock());
	delete secondLock;
	CPPUNIT_ASSERT(firstLock->tryLock());
	CPPUNIT_ASSERT(firstLock->isLocked());


}

#endif // BT_UTEST
