/*
 *  Copyright 2012 Google Inc. All Rights Reserved.
 *  
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  
 *      http://www.apache.org/licenses/LICENSE-2.0
 *  
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <gtest/gtest.h>
#include <string>

#include "BumpTop/QStringHelpers.h"

namespace {
  class QStringHelpersTest : public ::testing::Test {
  };

  TEST_F(QStringHelpersTest, Converting_back_and_forth_from_Qstring_to_std_string) {
    QString qstring = QString::fromWCharArray(L"ĄĘÓĆŁŃŹŻąęóćłńśżź");

    std::string std_string = utf8(qstring);
    std::string bad_std_string = qstring.toStdString();

    EXPECT_NE(qstring.size(), std_string.size());

    EXPECT_TRUE(qstring == QStringFromUtf8(std_string));
    EXPECT_FALSE(qstring == QStringFromUtf8(bad_std_string));
  }

  TEST_F(QStringHelpersTest, Test_compare_function) {
    QString a = "a";
    QString b = "b";

    EXPECT_EQ(0, a.compare(a));
    EXPECT_EQ(-1, a.compare(b));
    EXPECT_EQ(1, b.compare(a));
  }

  TEST_F(QStringHelpersTest, test_version_comparison) {
    EXPECT_EQ(versionStringLessThanVersionString("1.0", "1.1"), true);
    EXPECT_EQ(versionStringLessThanVersionString("1.1", "1.0"), false);
    EXPECT_EQ(versionStringLessThanVersionString("1.0.1", "1.0"), false);
    EXPECT_EQ(versionStringLessThanVersionString("1.0", "1.0.1"), true);
    EXPECT_EQ(versionStringLessThanVersionString("1.0", "1.0"), false);
    EXPECT_EQ(versionStringLessThanVersionString("1.5", "1.11"), true);
    EXPECT_EQ(versionStringLessThanVersionString("1.11", "1.5"), false);
    EXPECT_EQ(versionStringLessThanVersionString("", "1.0"), true);
    EXPECT_EQ(versionStringLessThanVersionString("1.0", ""), false);
    EXPECT_EQ(versionStringLessThanVersionString("2.0.1", "1.0.1"), false);
    EXPECT_EQ(versionStringLessThanVersionString("1.0.1", "2.0.1"), true);
  }
}
