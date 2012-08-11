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

# for each directory
# read the email and find out what version the email came from
# create that directory if it does not yet exist
# copy the dump file into a basenamed-file in that directory if it does not already exist and not marked as completed (REVIEWED) (see markAsReviewed)

import os
import os.path
import re
import shutil
import sys

# defines
FEEDBACK_FILE = "feedback.txt"
DUMP_FILE = "Bump.dmp"
SUBJECT_LINE = "Subject:"
COMPLETED_PREFIX = "COMPLETED"

def handleEmail(cwd, dir):
	global FEEDBACK_FILE, DUMP_FILE, SUBJECT_LINE, COMPLETED_PREFIX

	revisionNumber = -1
	feedbackFilePath = os.path.join(dir, FEEDBACK_FILE)
	dumpFilePath = os.path.join(dir, DUMP_FILE)
	feedbackFileExists = os.path.exists(feedbackFilePath)
	dumpFileExists = os.path.exists(dumpFilePath)

	# ensure that the feedback email exists as well as the dump
	if feedbackFileExists and dumpFileExists:
		# open the feedback email and extract the revision number
		# (and the pro code?)
		feedbackFile = open(feedbackFilePath, 'rU');
		line = feedbackFile.readline()
		while line:
			if (line.startswith(SUBJECT_LINE)):
				match = re.match(r".*rev(\d+).*", line)
				assert match
				revisionNumber = int(match.group(1))
				break;
			line = feedbackFile.readline()
		feedbackFile.close()
	
		# create the directory if it does not already exist
		assert revisionNumber > -1
		dumpDir = os.path.join(cwd, str(revisionNumber) + "_dumps")
		if not os.path.exists(dumpDir):
			os.mkdir(dumpDir)

		# copy the dump into the dump dir
		newDumpFileName = os.path.basename(dir) + ".dmp"
		if newDumpFileName.startswith(COMPLETED_PREFIX):
			newDumpFileName = newDumpFileName[len(COMPLETED_PREFIX):]
		newDumpFilePath = os.path.join(dumpDir + "/", newDumpFileName)
		if not os.path.exists(newDumpFilePath):
			try:
				shutil.copy(dumpFilePath, newDumpFilePath)
				print "Copied: " + newDumpFilePath
			except:
				print "Error copying: " + dumpFilePath

if __name__ == "__main__":
	workingDir = os.getcwd()
	dirList = os.listdir(workingDir)

	# for each directory in this one
	for dir in dirList:
		dirPath = os.path.join(workingDir, dir)
		if os.path.isdir(dirPath):
			handleEmail(workingDir, dirPath)


