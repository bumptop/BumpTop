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
#include "BT_ProxySettingsTest.h"
#include "BT_ProxySettings.h"

CPPUNIT_TEST_SUITE_REGISTRATION(ProxySettingsTest);

void ProxySettingsTest::testGettingProxySettingsFromIE()
{
// these aren't real unit tests, since they're dependent on the state of IE
// I just changed the state of IE and then wrote a test that matched the current
// state of IE to verify we were pulling proxy settings from IE correctly
// 
// 	ProxySettings* ps = new ProxySettings();
// 	ps->copyProxySettingsFromIE();
// 
// 	wprintf(ps->_autoConfigurationUrl.c_str());
// 	CPPUNIT_ASSERT_EQUAL(MANUAL, ps->getProxySetting());
// 
// 	CPPUNIT_ASSERT(QString("proxy.toronto.edu:80") == ps->_httpProxyUrl);
// 	CPPUNIT_ASSERT(QString("proxy.toronto.edu:80") == ps->_ftpProxyUrl);

	// another example
// 	wprintf(ps->_autoConfigurationUrl.c_str());
// 	CPPUNIT_ASSERT_EQUAL(AUTO_CONFIGURATION_URL, ps->getProxySetting());
// 	CPPUNIT_ASSERT(QString("http://proxy.toronto.edu/") == ps->_autoConfigurationUrl);
}

#endif // BT_UTEST
