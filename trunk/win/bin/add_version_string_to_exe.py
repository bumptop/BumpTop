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


def usage():
    print """
usage:

    > python add_version_string_to_exe.py VERSION_STRING [EXE_NAME]
    
    -- requires ResHacker.exe (http://angusj.com/resourcehacker/)
    -- by default, modifies "Source\Release\BumpTop.exe"

    """

def check_call(*args):
    print ' '.join(args)
    subprocess.check_call(' '.join(args))
    
def add_version_string_to_exe(prefix, exe_name, version_string):
    version_string_file = file('TEMP_VERSION_STRING.txt', 'w')
    version_string_file.write(version_string)
    version_string_file.close()
    
    if prefix != '': 
        prefix += '\\'

    #check_call(prefix + 'ResHacker.exe -addoverwrite ' + exe_name + ', ' + exe_name[0:-4] + '_' + version_string + '.exe, TEMP_VERSION_STRING.txt, BUMPTOP_VERSION_STRING, 1, 0')

    check_call(prefix + 'ResHacker.exe -addoverwrite ' + exe_name + ', ' + exe_name + ', TEMP_VERSION_STRING.txt, BUMPTOP_VERSION_STRING, 1, 0')
    os.remove('TEMP_VERSION_STRING.txt')
    

def main(argv=None):
    if argv is None:
        argv = sys.argv
    
    if len(argv) != 3 and len(argv) != 2:
        usage()
    else:
        version_string = argv[1]
        acceptable_version_strings = ['STANDARD', 'VIP', 'OEM_LENOVO', 'OEM_ASUS', 'OEM_ACER', 'OEM_HP', 'OEM_JW', 'OEM_INTEL_ATOM', 'VGA_ASUS', 'VGA_TUL', 'VGA_COLORFUL', 'VGA_SAPPHIRE', 'VGA_HIS', 'VGA_INPAI', 'TRADESHOW', 'TRADESHOW_SAPPHIRE']
        if version_string not in acceptable_version_strings:
            print "Version string must be one of: " + ", ".join(acceptable_version_strings)
            exit(1)
            
        if len(argv) == 3:
            exe_name = argv[2]
        else:
            exe_name = r'Source\Release\BumpTop.exe'
        add_version_string_to_exe(os.path.dirname(argv[0]), exe_name, version_string)

    
if __name__ == '__main__':
    main()
    
