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

#ifndef _TEST_PHOTOFRAMEACTOR_
#define _TEST_PHOTOFRAMEACTOR_
#ifdef BT_UTEST

#include "BT_TestCommon.h"
#include "BT_PhotoFrameActor.h"

using CppUnit::TestFixture;

/*
* FileTransfer tests
*/
class PhotoFrameActorTest : public TestFixture
{
	CPPUNIT_TEST_SUITE( PhotoFrameActorTest );
		CPPUNIT_TEST( testConstructor );
	CPPUNIT_TEST_SUITE_END();

public:
	// fixture setup 
	void setUp();
	void tearDown();

	// tests
	void testConstructor();
};

#endif // BT_UTEST
#endif // _TEST_FILETRANSFER_