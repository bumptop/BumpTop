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
from xml.dom import minidom, Node
from re import escape
import os
import os.path
import sys
import copy

#defines
TS_FILEIN = "Settings_%s.ts"
POT_FILEOUT = "Settings_%s.pot"
					
def recExtractTSStrings(root, db):
	for child in root.childNodes:
		if (child.nodeType == Node.ELEMENT_NODE) and (child.nodeName == "message"):
			key = ""
			source = ""
			translation = ""
			for mChild in child.childNodes:
				mChildName = mChild.nodeName
				if mChildName == "location":
					key = mChild.getAttribute("filename")
				elif mChildName == "source":
					if mChild.hasChildNodes():
						assert mChild.firstChild.nodeType == Node.TEXT_NODE
						source = mChild.firstChild.data
				elif mChildName == "translation":
					if mChild.hasChildNodes():
						assert mChild.firstChild.nodeType == Node.TEXT_NODE
						translation = mChild.firstChild.data.replace("\n", "\\n")
			if key:
				db[key] = { "source" : source, "translation" : translation }
				
def replaceDuplicateID_STATICStrings(db):
	for key in db:
		source = db[key]["source"]
		if key.find("wxID_STATIC") > -1:
			for key2 in db:
				if db[key2]["source"] == source:
					db[key2] = db[key]
	tmpDb = copy.copy(db)
	for key in tmpDb:
		if key.find("wxID_STATIC") > -1:
			del db[key]
					
def escapeQuote(str):
	return str.replace('"', '\\"')
	
def savePOTFromTS(existingDb, languageInfo):
	header = '''# SOME DESCRIPTIVE TITLE.
# Copyright (C) YEAR THE PACKAGE'S COPYRIGHT HOLDER
# This file is distributed under the same license as the PACKAGE package.
# FIRST AUTHOR <EMAIL@ADDRESS>, YEAR.
#
msgid ""
msgstr ""
"Project-Id-Version: BumpTop 1.0\\n"
"Report-Msgid-Bugs-To: feedback@bumptop.com\\n"
"POT-Creation-Date: 2009-04-22 15:23-0400\\n"
"PO-Revision-Date: 2009-04-22 15:26-0500\\n"
"Last-Translator: BumpTop\\n"
"Language-Team: BumpTop\\n"
"MIME-Version: 1.0\\n"
"Content-Type: text/plain; charset=UTF-8\\n"
"Content-Transfer-Encoding: 8bit\\n"
"X-Poedit-Language: %s\\n"
"X-Poedit-Country: %s\\n"

''' % (language[1], language[2])
	global POT_FILEOUT
	workingDir = os.getcwd()
	potFilePath = os.path.join(workingDir, POT_FILEOUT % language[0])
	potFile = open(potFilePath, 'w+')
	potFile.write(header)
	for key, value in existingDb.items():
		translation = value["translation"]
		message = '''#: Settings.cpp:0
msgid "%s"
msgstr "%s"

''' % (escapeQuote(value["source"]).encode("UTF-8"), escapeQuote(translation).encode("UTF-8"))
		potFile.write(message)		
	potFile.close()

if __name__ == "__main__":
	languages = [
		("en", "English", "Canada"), 
		("zh_TW", "Chinese", "Taiwan"), 
		("zh_CN", "Chinese", "China"), 
		("ja", "Japanese", "Japan"), 
		("ko", "Korean", "Korea"), 
		("fr", "French", "France"), 
		("it", "Italian", "Italy"), 
		("de", "German", "Germany"), 
		("ru", "Russian", "Russia"), 
		("es", "Spanish", "Spain"), 
		("pt", "Portuguese", "Portugal"),
		("tr", "Turkish", "Turkey")
	]
	for language in languages:
		# try and load the existing ts file
		existingDb = {}
		workingDir = os.getcwd()
		tsFilePath = os.path.join(workingDir, TS_FILEIN % language[0])
		tsDoc = minidom.parse(tsFilePath)
		root = tsDoc.getElementsByTagName("context")[0]
		recExtractTSStrings(root, existingDb)
		
		# replace any duplicates
		replaceDuplicateID_STATICStrings(existingDb)
		
		# create the pot file
		savePOTFromTS(existingDb, language)
		
		
