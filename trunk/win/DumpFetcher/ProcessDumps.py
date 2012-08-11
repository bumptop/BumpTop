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

import subprocess
import re
import os
import sys

data = {}

BUILD_LOG_FILENAME = "build.log"
BUILD_LOG_PATH = os.path.join(os.getcwd(), BUILD_LOG_FILENAME)
BUILD_LOG_FILE = open(BUILD_LOG_PATH, 'w+')
VERBOSE = False

def LOG(str, logToScreen=True, logToFile=True):
	global BUILD_LOG_FILE, VERBOSE
	if logToScreen or VERBOSE:
		print str
	if logToFile:
		BUILD_LOG_FILE.write(str + "\n")

def END_LOG():
	global BUILD_LOG_FILE
	BUILD_LOG_FILE.close()

if __name__ == "__main__":
	# parse the command line args
	displayPrompt = True
	target = ""
	try:
		buildNumber = sys.argv[1]
	except:
		LOG("Must provide build number\n  python ProcessDumps.py 6197")
		sys.exit(2)
		
	symbolPaths = ['-s "\\\\10.67.0.51\\Symbols"', ('-s "internalLooseNSource%s\\Loose"' % buildNumber )]
	DUMPREPORTER = "DumpReporter.exe"
	imagePath = os.path.join(os.getcwd(), ("internalLooseNSource%s\\Loose" % buildNumber))
	dumpPath = os.path.join(os.getcwd(), ("%s Crash Report rev%s -\\" % (buildNumber, buildNumber)))
	
	targetFiles = os.listdir(dumpPath)
	targetFilesRegex = re.compile(".*\.(dmp)$")
		
	# Typical line
	#   0x5D     BumpTop!NxActorWrapper::isRequiringRende  +bt_nxactorwrapper.cpp (194)
	pattern = "0x[0-9A-F]{1,10}\s*([a-zA-Z]*!\S*)\s*\+(\S*)\s*\((\d+)\)"

	for file in targetFiles:
		if targetFilesRegex.match(file, re.IGNORECASE):
			print ("match: %s" % file)
			
			symbolPath = ""
			for spath in symbolPaths:
				symbolPath += ' ' + spath
					
			#print symbolPath
			dumpReporterPath = ("%s %s -i \"%s\" \"%s\"" % 
				(DUMPREPORTER, symbolPath, imagePath, os.path.join(dumpPath, file)))
			#print dumpReporterPath
			dumpReporter = subprocess.Popen(dumpReporterPath, stdout=subprocess.PIPE, stdin=subprocess.PIPE, stderr=subprocess.PIPE, bufsize=1)
			dumpReporterOut = dumpReporter.stdout.readlines()
			#print dumpReporterOut
			
			stackTrace = None
			# get stackTrace to only include the stack trace information / trim out header text
			for i, line in enumerate(dumpReporterOut):
				if line.startswith("Stack Trace:"):
					stackTrace = dumpReporterOut[i+1:]
					break
			
			if stackTrace:
				for line in stackTrace:
					#print line
					result = re.search(pattern, line, re.IGNORECASE)
					if result:
						#print line
						#groups = [1, 2, 3]
						#for num in groups:
						#	if result.group(num):
						#		print ("%s: %s" % (num, result.group(num)))
						# 1 - function name
						# 2 - filename
						# 3 - line number
						functionName = result.group(1)
						fileName = result.group(2)
						lineNumber = result.group(3)
						key = (functionName, fileName, lineNumber)
						data[key] = data.get(key, 0) + 1
						break
					#else:
					#	print "NO MATCH: %s" % line
				print "[file processed ok]"
			else:
				print "[file skipped]"
	for [functionName, fileName, lineNumber] in data:
		print "%s,%s,%s,%s" % (data[functionName, fileName, lineNumber], functionName, fileName, lineNumber)

	# close the log
	END_LOG()
	#DumpReporter.exe -i "K:\SVN\trunk\DumpFetcher\5790 - Release - 2.0.5790\internalLooseNSource5790\Loose" -s "K:\Symbols" "K:\SVN\trunk\DumpFetcher\5790 Crash Report rev5790 -\*.dmp"
