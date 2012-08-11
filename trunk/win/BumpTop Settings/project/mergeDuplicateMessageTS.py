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
import os
import os.path

#defines
TS_FILEOUT = "Settings_%s.ts"

def mergeDuplicates(root, db):
	for child in root.childNodes:
		if (child.nodeType == Node.ELEMENT_NODE) and (child.nodeName == "message"):
			location = ""
			source = ""
			translation = ""
			type = ""
			for mChild in child.childNodes:
				mChildName = mChild.nodeName
				if mChildName == "location":
					location = mChild.getAttribute("filename")
				elif mChildName == "source":
					if mChild.hasChildNodes():
						assert mChild.firstChild.nodeType == Node.TEXT_NODE
						source = mChild.firstChild.data
				elif mChildName == "translation":
					type = mChild.getAttribute("type")
					if mChild.hasChildNodes():
						assert mChild.firstChild.nodeType == Node.TEXT_NODE
						translation = mChild.firstChild.data
			
			newLocation = location
			newType = type
			newTranslation = translation
			# if message entry already exists, try to merge in the tranlation to th unfinished one
			# location should be taken from the unfinished one
			oldLocation = ""
			if source in db :
				oldType = db[source]["type"]
				oldTranslation = db[source]["translation"]
				oldLocation = db[source]["location"]
				if oldType == "unfinished" :
					newLocation = oldLocation
					newType = oldType
				else :
					newTranslation = oldTranslation
					newType = ""
				#print "merged newLocation %s \n location1 = %s " % (newLocation, source)
			db[source] = { "location" : newLocation, "source" : source, "translation" : newTranslation, "type" : newType }
			
			#print " location = %s \nsource =  %s \n type = %s " % (location, source, type)
					
def saveTS(doc, language, existingDb):
	root = doc.firstChild
	whiteList = ["label", "title", "tooltip"]
	# extract all the strings as a set of key:value pairs
	#removeDuplicates(root, "btsettings", existingDb, whiteList)
	# write a qt linguist ts file (don't bother creating an
	# xml doc for now, we'll just do it by hand)	
	# see http://www.i18nguy.com/unicode/language-identifiers.html
	header = '''<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.0" language="%s">
<context>
<name>BumpTopSettings</name>''' % language
	footer = '''
</context>
</TS>'''
	global TS_FILEOUT
	workingDir = os.getcwd()
	tsFilePath = os.path.join(workingDir, TS_FILEOUT % language)
	tsFile = open(tsFilePath, 'w+')
	tsFile.write(header)
	for key, value in existingDb.items():
		location = value["location"]
		source = key
		translation = value["translation"]
		type = value["type"]
		typeString = ""
		if "unfinished" == type :
			typeString = """ type="unfinished" """
		message = """
	<message>
		<location filename="%s" line="0"/>
		<source>%s</source>
		<translation%s>%s</translation>
	</message>""" % (location.encode("UTF-8"), source.encode("UTF-8"), typeString, translation.encode("UTF-8"))
		tsFile.write(message)
	tsFile.write(footer)				
	tsFile.close()

if __name__ == "__main__":
	languages = [
		"en", 
		"zh_TW", 
		"zh_CN", 
		"ja", 
		"ko", 
		"fr", 
		"it", 
		"de", 
		"ru", 
		"es", 
		"pt",
		"tr",
		"br",
		"nl",
	]
	for language in languages:	
		
		# if possible, try and load the existing ts file
		existingDb = {}
		tsDoc = minidom.parse(TS_FILEOUT % language)
		root = tsDoc.getElementsByTagName("context")[0]
		mergeDuplicates(root, existingDb)
		saveTS(tsDoc, language, existingDb)
