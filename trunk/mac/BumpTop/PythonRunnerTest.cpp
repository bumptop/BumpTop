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

#include "BumpTop/PythonRunner.h"

namespace {

  // The fixture for testing class Foo.
  class PythonRunnerTest : public ::testing::Test {
  };

  TEST_F(PythonRunnerTest, TestQDir) {
    QDir directory(QString("."));
    ASSERT_TRUE(directory.exists());
  }

  TEST_F(PythonRunnerTest, BasicPythonTestRuns) {
    bool python_script_successful = runPythonScriptInDirectory("print 'hello world'", ".");
    ASSERT_TRUE(python_script_successful);
  }

  TEST_F(PythonRunnerTest, ScriptReturnsBadExitCode) {
    bool python_script_successful = runPythonScriptInDirectory("exit(1)", ".");
    ASSERT_FALSE(python_script_successful);
  }

}  // namespace
