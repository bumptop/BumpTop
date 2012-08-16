// Copyright 2011 Google Inc. All Rights Reserved.
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
#include "BT_MD5HasherTest.h"
#include "BT_MD5Hasher.h"

CPPUNIT_TEST_SUITE_REGISTRATION(MD5HasherTest);


void MD5HasherTest::testBasic()
{
	MD5Hasher hasher;;
	hasher.updateWithFile("BT_Common.cpp");
	CPPUNIT_ASSERT_EQUAL(string("5ebcb5f6e9189b822f72ca160ecbb7fe"), hasher.hexDigest());

	MD5Hasher hasher2;
	hasher2.update("this is a test", strlen("this is a test"));
	CPPUNIT_ASSERT_EQUAL(string("54b0c58c7ce9f2a8b551351102ee0938"), hasher2.hexDigest());

}

#endif // BT_UTEST