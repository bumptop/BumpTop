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

# for each file, run the script and capture the look
import json
import os
import os.path
import re
import string
import subprocess

#defines
DUMP_DB_FILE = "dumps.json"
SUMMARY_FILE = "crash_summary.txt"
CDB_PATH = 'C:\\Program Files\\Debugging Tools for Windows\\cdb %s'
CDB_ARGS = '-y srv*c:\\symbols*http://msdl.microsoft.com/download/symbols -i "c:\\windows\\i386" -z "%s" -c "!analyze -v; Q" -i "%s"' 
EXEC_PATH = '\\\\10.67.0.50\\BumpShare\\Releases\\2738 - 1.0.2738 - v2_download.com\\Loose'
# minidump header defines
BUCKET_ID = "DEFAULT_BUCKET_ID"
PROCESS_NAME = "PROCESS_NAME"
ERROR_CODE = "ERROR_CODE"
BUG_CHECK = "BUGCHECK_STR"
STACK_TRACE = "STACK_TEXT"
IMAGE_NAME = "IMAGE_NAME"
# dump db
RELFN_BACKTRACE = "relfn_backtrace_d1"
RELFN_BACKTRACE_I = "relfn_backtrace_di"
DUMPS_PARSED = "dumps_parsed"

def getFileExtension(filePath):
	(filename, ext) = os.path.splitext(filePath)
	return ext
def getFileName(filePath):
	(filename, ext) = os.path.splitext(filePath)
	return filename
	
def extractOutput(header, output, fromIndex):
	index = output.find(header, fromIndex)
	if index < 0:
		return ("", fromIndex)
	else:
		endIndex = output.find("\n\n", index)
		return (output[index:endIndex].strip(), endIndex)
		
def extractCallStack(stackTrace):
	rawFunctions = re.findall(r"(\s?\w{8} ){5}([^0][^x][\w!\+]+)", stackTrace)
	return [f[1] for f in rawFunctions]
	
def storeCallStack(callStack, dumpDb):
	global RELFN_BACKTRACE
	
	# store the function backtrace depth of 1
	if RELFN_BACKTRACE not in dumpDb:
		dumpDb[RELFN_BACKTRACE] = {}
	for i in range(len(callStack)):
		fn = callStack[i]		
		if i == len(callStack)-1:
			# add the first item in the stack if not already there
			if fn not in dumpDb[RELFN_BACKTRACE]:
				dumpDb[RELFN_BACKTRACE][fn] = {}
		else:
			# otherwise, increment backtrace reference from previous function
			prevFn = callStack[i+1]		
			if fn not in dumpDb[RELFN_BACKTRACE]:
				dumpDb[RELFN_BACKTRACE][fn] = {
					prevFn : 0
				}
			else:
				if prevFn not in dumpDb[RELFN_BACKTRACE][fn]:
					dumpDb[RELFN_BACKTRACE][fn][prevFn] = 0
			dumpDb[RELFN_BACKTRACE][fn][prevFn] += 1
	
	# store the function backtrace depth i
	if RELFN_BACKTRACE_I not in dumpDb:
		dumpDb[RELFN_BACKTRACE_I] = {}
	backtraceStr = ""
	for fn in callStack:
		if len(backtraceStr) > 0:
			backtraceStr += " < "
		backtraceStr += fn
		if backtraceStr not in dumpDb[RELFN_BACKTRACE_I]:
			dumpDb[RELFN_BACKTRACE_I][backtraceStr] = 0
		dumpDb[RELFN_BACKTRACE_I][backtraceStr] += 1
	
		
def handleDump(filePath, dumpDb, summaryFile):
	global CDB_PATH, CDB_ARGS, EXEC_PATH
	global PRE_STACKTRACE_HEADER
	if getFileExtension(filePath) == ".dmp":
		print "Parsing: %s" % (filePath)
		cdbArgs = CDB_ARGS % (filePath, EXEC_PATH)
		cdb = CDB_PATH % (cdbArgs)
		
		# launch cdb on the dump and capture the output
		cdbProc = subprocess.Popen(cdb, stdout=subprocess.PIPE, stdin=subprocess.PIPE, stderr=subprocess.PIPE, bufsize=1)
		cdbOut = cdbProc.stdout.read()
				
		# try and extract the crash information and the stack trace
		index = 0
		(bucketId, index) = extractOutput(BUCKET_ID, cdbOut, index)
		(processName, index) = extractOutput(PROCESS_NAME, cdbOut, index)
		(errorCode, index) = extractOutput(ERROR_CODE, cdbOut, index)
		(bugCheck, index) = extractOutput(BUG_CHECK, cdbOut, index)
		(stackTrace, index) = extractOutput(STACK_TRACE, cdbOut, index)
		(imageName, index) = extractOutput(IMAGE_NAME, cdbOut, index)
		
		# parse the stack trace into the reverse list of function calls
		callStack = extractCallStack(stackTrace)
		storeCallStack(callStack, dumpDb)
		stackTrace = "STACK TRACE:\n\t" + "\n\t".join(callStack)
		
		# create a summary and write it to the global file
		summary = "--- %s ---------------\n%s\n%s\n%s\n%s\n%s\n%s\n\n" % (filePath, processName, imageName, bucketId, bugCheck, errorCode, stackTrace)		
		summaryFile.write(summary)
		summaryFile.flush()	
		print "Parsed: %s" % (filePath)
	
if __name__ == "__main__":
	workingDir = os.getcwd()
	dirList = os.listdir(workingDir)
	
	# load the dump db
	dumpDb = {}
	dumpDbPath = os.path.join(workingDir, DUMP_DB_FILE)	
	if os.path.exists(dumpDbPath):
		dumpDbFile = open(dumpDbPath, 'rU')
		dumpDb = json.load(dumpDbFile)
		dumpDbFile.close()
	if DUMPS_PARSED not in dumpDb:
		dumpDb[DUMPS_PARSED] = {}
			
	# open the summary file
	summaryFilePath = os.path.join(workingDir, SUMMARY_FILE)
	
	# open the dump db
		
	# for each file	
	fileCount = 0
	for file in dirList:
		if file not in dumpDb[DUMPS_PARSED]:
			filePath = os.path.join(workingDir, file)		
			if os.path.isfile(filePath):
				handleDump(filePath, dumpDb, summaryFile)
			# write the dump db every two files
			if fileCount % 10 == 0:
				dumpDbFile = open(dumpDbPath, 'w+')
				json.dump(dumpDb, dumpDbFile, indent=4)
				dumpDbFile.close()
			fileCount += 1
			# mark the file as parsed already
			dumpDb[DUMPS_PARSED][file] = 1
		
	# close the files
	summaryFile.close()
