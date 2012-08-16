# Copyright 2011 Google Inc. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os
import subprocess
import sys

# Take a path relative to this script, return a path relative to the working dir
def script_relative(path):
	thisdir = os.path.dirname(os.path.realpath(__file__))
	return os.path.normpath(os.path.join(thisdir, path))

BUMPTOP_EXE_PATH = script_relative("../Debug/BumpTop.exe")
RESULT_FILE = script_relative("../Tests/test_log.txt")

# Invoke the tests
subprocess.Popen([BUMPTOP_EXE_PATH, "-automatedJSONTesting"]).communicate()

# Examine the results
retcode = 1
completed = False
with open(RESULT_FILE) as f:
	for line in f.readlines():
		if line.startswith("TESTING COMPLETE"):
			completed = True
			print line.strip()
			retcode = 0 if ("0 FAILED" in line) else 1

if not completed:
	print "ERROR: Tests aborted -- see %s" % RESULT_FILE
			
# Return 0 on success, non-zero on failure
sys.exit(retcode)
