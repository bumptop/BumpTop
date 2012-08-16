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
#include "BT_StatsManagerTest.h"
#include "BT_StatsManager.h"
#include "BT_Stopwatch.h"

CPPUNIT_TEST_SUITE_REGISTRATION(StatsManagerTest);

void StatsManagerTest::testBasic()
{
	// check that the new timer class behaves equivalently to the old one we were using
	Stopwatch t2;
	StopwatchInSeconds t3;
	t2.restart();
	t3.restart();
	Sleep(100);

	double t2_time = t2.elapsed() / 1000.0;
	double t3_time = t3.elapsed();

	CPPUNIT_ASSERT_EQUAL(t2_time, t3_time);
}

#endif // BT_UTEST
