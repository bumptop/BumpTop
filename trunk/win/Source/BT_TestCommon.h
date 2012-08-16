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

#ifndef _TESTCOMMON_
#define _TESTCOMMON_
#ifdef BT_UTEST

// includes
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

// global run function, runs the current set of BumpTop tests
// and returns whether they suceeded or not.
// XXX: only using simple test management for now, eventually should move to something a little more complex
int runBumptopTests(bool useConsole=false);

#endif // BT_UTEST
#endif // _TESTCOMMON_