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
import re
import sys
import time
import getpass
import errno
import poplib
import string
import email
import cStringIO
import mimetypes

LOG_FILENAME = "DumpFetcher.log"
LOG_PATH = os.path.join(os.getcwd(), LOG_FILENAME)
LOG_FILE = open(LOG_PATH, 'a+')
VERBOSE = False

# sources:
#http://docs.python.org/library/email-examples.html
#http://www.devshed.com/c/a/Python/String-Manipulation/3/
#http://book.opensourceproject.org.cn/lamp/python/pythontext/opensource/0321112547_ch05lev1sec1.html
#http://book.opensourceproject.org.cn/lamp/python/propython3rd/opensource/0596009259/python3-chp-14-sect-6.html
#http://python.about.com/od/cgiformswithpython/ss/retrievemail_4.htm

def LOG(str, logToScreen=True, logToFile=True):
	global LOG_FILE, VERBOSE
	if logToScreen or VERBOSE:
		print str
	if logToFile:
		LOG_FILE.write(str + "\n")
		
def END_LOG():
	global LOG_FILE
	LOG_FILE.close()
	
def sanitizeString (dirtyString):
	stripChars = ["\\", ":", ",", "+", "{", "}", "?", "/"]
	for stripChar in stripChars:
		dirtyString = string.replace(dirtyString, stripChar, "")
	return dirtyString

def unpackMessage (msg):
	# given an email message, looks for an appropriate revision number/GUID from a sanitized subject field
	# and writes all attachments to email to folder
	# All crashes from a certain build will be grouped since folder name will be based off of revision + OEM version
	
	global revisions, types
	messageSubject = msg.get("Subject")

	start = time.time()
	LOG ("Message: %s" % messageSubject)
	messageSubject = sanitizeString(messageSubject)

	# grab GUID if possible, or use date/time stamp from email
	GUIDmatch = re.match( "(.*)\{([A-Z0-9-]{36,36})\}(.*)", messageSubject)
	if GUIDmatch != None:
		GUID = GUIDmatch.group(2)
	else:
		#remove : and \ , and + from date
		GUID = sanitizeString(msg.get("Date"))
	
	# find revision number, matching 4 or 5 numbers after "rev"
	revisionMatch = re.match( "(.*)rev([0-9]{4,5})(.*)", messageSubject)
	revision = 0
	if revisionMatch != None:
		revision = revisionMatch.group(2)
	
	# find message type, based on text before "rev" in subject
	typeMatch = re.match( "(.*)rev([0-9]{4,5})(.*)", messageSubject)
	# just use whole subject
	type = messageSubject
	# or revision # if available
	if typeMatch != None:
		type = typeMatch.group(1)

	LOG ("\tType recognized: %s" % type, False, True)
	# increment tally of type of email downloaded
	if (type in types.keys()):
		types[type] += 1
	else:
		types[type] = 1
	#print ("\t\tType %s: %s" % (type, types[type]))
	
	targetFolder = sanitizeString(str(revision) + " " + messageSubject)
	
	LOG("\tProcessed in: [%ds]" % (time.time() - start), False, True)
	# try to create a folder based on subject field; if it already exists, just write into existing folder
	try:
		os.mkdir(targetFolder)
	except OSError, e:
		# Ignore directory exists error
		if e.errno != errno.EEXIST:
			raise
	counter = 1
	numAttachments = 0
	start = time.time()
	for part in msg.walk():
		# multipart/* are just containers
		if part.get_content_maintype() == 'multipart':
			continue
		# Applications should really sanitize the given filename so that an
		# email message can't be used to overwrite important files
		filename = part.get_filename()
		LOG (("\tFound attachment: %s" % filename), False, True)
		if not filename:
			orig = ''
			ext = mimetypes.guess_extension(part.get_content_type())
			if ((ext == '.exe') or (ext == '.pdf')):
				orig = ext
				ext = '.txt'
			if not ext:
				# Use a generic bag-of-bits extension
				ext = '.bin'
			filename = 'part-%03d%s%s' % (counter, orig, ext)
		counter += 1
		# write the file to targetFolder
		outputFilename = ("%s-%s" % (GUID, filename))
		LOG (("\tWriting as: %s" % outputFilename), False, True)
		fp = open(os.path.join(targetFolder, outputFilename), 'wb')
		fp.write(part.get_payload(decode=True))
		numAttachments += 1
		fp.close()
	LOG("\tAttachments written: %s [%ds]" % (numAttachments, time.time() - start))

def DumpMessages(host, user, password):
	# Establish a connection to the POP server.
	a = poplib.POP3_SSL(host)
	# Print server response
	print a.user(user)
	print a.pass_(password)
	# The mailbox is now locked - ensure we unlock it!
	try:
		(numMsgs, totalSize) = a.stat()
		LOG ("Total size: %s" % totalSize)
		LOG ("Number of messages: %s\n" % numMsgs)
		for thisNum in range(1, numMsgs+1):
			# get the email
			(server_msg, body, octets) = a.retr(thisNum)
			# join it into a list of strings
			message = string.join(body, "\n")
			# decode the message
			msg = email.message_from_string(message)
			# rip out attachments
			unpackMessage(msg)
			print ("\t%s" % a.dele(thisNum))
	finally:
		print ("Committing operations and disconnecting from server")
		print a.quit()

if __name__=='__main__':
	if (len(sys.argv) < 3 or len(sys.argv) > 4):
		# incorrect number of parameters - may wish to do more argument cleaning later to ensure *@*.com, etc.
		print "This script connects to a POP3S server, grabs all attachments from\nall emails and saves them into directories, grouped by message subject.\nThe build revision number in the form rev????? is pulled in front to ensure\nreports are sorted by revision number.\n"
		print "Usage:", sys.argv[0], "host username [password]"
		print "\tExamples:\nDumpPOP3attachments pop.gmail.com bumptopfeedback@gmail.com"
		print "DumpPOP3attachments pop.gmail.com bumptopfeedback@gmail.com SecretPassword"
	elif len(sys.argv) == 3:
		# prompt for password
		password = getpass.getpass()
	elif len(sys.argv) == 4:
		# password is provided
		password = sys.argv[3]
	
	if len(sys.argv) >= 3:
		# store stats of this pull
		revisions = {}
		types = {}
		# we should have what we need - log in and dump messages to disk
		wholeProcessStart = time.time()
		LOG("------------------Starting processing----------------")
		DumpMessages(sys.argv[1], sys.argv[2], password)
		diffTime =  (time.time() - wholeProcessStart)
		LOG("------------------Completed processing: [%dm %ds]----------------" % (diffTime / 60, diffTime % 60))
		END_LOG()
