#  Copyright 2012 Google Inc. All Rights Reserved.
#  
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#  
#      http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.

import subprocess
import os
import glob

new_argv = []
for arg in os.sys.argv[1:]:
    if arg.find("*") != -1:
        new_argv += glob.glob(arg)
    else:
        new_argv.append(arg)

print ""
print " ".join(new_argv)
print ""
try:
    subprocess.check_call(new_argv)
except subprocess.CalledProcessError, e:
    exit(e.returncode)
