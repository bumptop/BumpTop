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
#include "BT_TestCommon.h"

#ifdef BT_UTEST
#include "BT_SceneManager.h"
#include "BT_OverlayComponent.h"
#include "BT_WindowsOS.h"

using CppUnit::Test;
using CppUnit::TestFactoryRegistry;
using CppUnit::TextUi::TestRunner;
using CppUnit::CompilerOutputter;

int runBumptopTests(bool useConsole)
{
	// get the top level suite from the anonymous registry
	Test * suite = TestFactoryRegistry::getRegistry().makeTest();

	// add the suite to the test runner
	TestRunner runner;
		runner.addTest(suite);

	// format the runner output for the compiler if we are using the console
	if (useConsole)
		runner.setOutputter(new CompilerOutputter(&runner.result(), std::cerr));
	else
	{
		// show a message otherwise
		MessageClearPolicy clearPolicy;
			clearPolicy.setTimeout(4);
		Message * message = new Message("runBumptopTests", "Running BumpTop Unit Tests", Message::Ok, clearPolicy);
		scnManager->messages()->addMessage(message);
	}

	// run the tests and return status
	return (runner.run() ? 1 : 0);
}

#endif // BT_UTEST