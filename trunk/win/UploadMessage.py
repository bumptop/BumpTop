#!/usr/bin/python

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

from subprocess import call
import ftplib
import os
import sys
import urllib

PYTHON = 'C:\Python25\python.exe '
S3FUNNEL = PYTHON + '"' + os.getcwd() + '\\bin\\s3funnelcmd" download.bumptop.com '

def usage():
    print """
usage:

    UploadMessage.py
    
    upload a file as message.txt:
    > python UploadBumptop.py path/of/message.txt
	
	upload a file as message.txt:
    > python UploadBumptop.py path/of/vip_message.txt
    
    delete message.txt
    > python UploadBumpTop.py delete_message
	
    delete vip_message.txt
    > python UploadBumpTop.py delete_vip_message

    """
	
def main(argv=None):
    if argv is None:
        argv = sys.argv
    if len(argv) != 2:
        usage()
    elif os.path.basename(sys.argv[1]) == 'delete_message':
        call(S3FUNNEL + ' DELETE message.txt')
    elif os.path.basename(sys.argv[1]) == 'delete_vip_message':
        call(S3FUNNEL + ' DELETE vip_message.txt')
    elif os.path.basename(sys.argv[1]) != 'message.txt' and os.path.basename(sys.argv[1]) != 'vip_message.txt':
        print "Error: File must be called message.txt or vip_message.txt"
    else:
        call(S3FUNNEL + ' PUT %s' % sys.argv[1])
		
if __name__ == '__main__':
    main()
   