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
#include "BT_TestKeyboardManager.h"
#include "BT_KeyboardManager.h"
#include "BT_WindowsOS.h"

using namespace CppUnit;

// Globals
bool _fileDeleted;
//

// register the test suite with the anonymous registery
CPPUNIT_TEST_SUITE_REGISTRATION(KeyboardManagerTest);

KeyboardManagerTest::KeyboardManagerTest()
{}

KeyboardManagerTest::~KeyboardManagerTest()
{}

void KeyboardManagerTest::setUp()
{
}

void KeyboardManagerTest::tearDown()
{
}

void KeyboardManagerTest::testKey(unsigned int key)
{
	keyManager->addHandler(&_mockKeyHandler);

	_mockKeyHandler.reset();
	KeyCombo delKey(key);
	::SendMessage(winOS->GetWindowsHandle(), WM_KEYDOWN, delKey.subKeys.key, 0);
	CPPUNIT_ASSERT_EQUAL((unsigned int) delKey.subKeys.key, _mockKeyHandler.getLastKey());

	keyManager->removeHandler(&_mockKeyHandler);
}

void KeyboardManagerTest::testVistaSpecialKeys()
{
	testKey(VK_DELETE);
	testKey(VK_F1);
}

void Key_DummyDeleteSelection()
{
	_fileDeleted = true;
}

void KeyboardManagerTest::testCallbackMap()
{
	_fileDeleted = false;
	KeyCombo delKey(VK_DELETE);
	keyManager->mapKeyToFunction(delKey, Key_DummyDeleteSelection);
	::SendMessage(winOS->GetWindowsHandle(), WM_KEYDOWN, delKey.subKeys.key, 0);
	CPPUNIT_ASSERT(_fileDeleted);
}

#endif // BT_UTEST