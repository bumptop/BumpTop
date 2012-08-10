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

## used to clean the qt directory after doing a fresh build

import os, fnmatch

def locate(pattern, root=os.curdir):
    '''Locate all files matching supplied filename pattern in and below
    supplied root directory.'''
    for path, dirs, files in os.walk(os.path.abspath(root)):
        for filename in fnmatch.filter(files, pattern):
            yield os.path.join(path, filename)

QT_DIR = "Qt-Win_x86-4.5.2"
## delete non-header files from the src directory
for path in locate("*", QT_DIR + "/src"):
    if not path.endswith(".h"):
        print "Deleting", path
        os.remove(path)

for extension in "ilk", "pri", "prl":
    for path in locate("*." + extension, QT_DIR):
        print "Deleting", path
        os.remove(path)
