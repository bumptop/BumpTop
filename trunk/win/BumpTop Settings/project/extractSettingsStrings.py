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

'''
def recMergeXRCTranslatedStrings(root, uniquePrefix, db, whiteList):	
	for child in root.childNodes:
		if child.nodeType == Node.ELEMENT_NODE:
			if child.nodeName == "object":
				key = uniquePrefix
				if child.hasAttribute("name"):
					key = "%s.%s" % (uniquePrefix, child.getAttribute("name"))
				# recurse on this object
				recExtractXRCStrings(child, key, db, whiteList)
			elif child.nodeName in whiteList:
				key = "%s.%s" % (uniquePrefix, child.nodeName)
				if child.hasChildNodes():
					assert child.firstChild.nodeType == Node.TEXT_NODE
					if "translation" in db[key]:
						child.firstChild.data = db[key]["translation"]
'''

def recExtractXRCStrings(root, uniquePrefix, db, whiteList):
	for child in root.childNodes:
		if child.nodeType == Node.ELEMENT_NODE:
			if child.nodeName == "object":
				key = uniquePrefix
				if child.hasAttribute("name"):
					key = "%s.%s" % (uniquePrefix, child.getAttribute("name"))
				# recurse on this object
				recExtractXRCStrings(child, key, db, whiteList)
			elif child.nodeName in whiteList:
				key = "%s.%s" % (uniquePrefix, child.nodeName)
				if child.hasChildNodes():
					assert child.firstChild.nodeType == Node.TEXT_NODE
					text = child.firstChild.data		
					translation = ""
					if key in db:		
						if ("source" in db[key]) and (text == db[key]["source"]):
							if "translation" in db[key]:
								translation = db[key]["translation"]
					db[key] = { "source": text, "translation" : translation }
					
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
						translation = mChild.firstChild.data
			if key:
				db[key] = { "source" : source, "translation" : translation }
	
def extractCustomStrings(db):
	key = "bumptopsettings.OnInitExistingInstance.caption"
	if key not in db:
		db[key] = { "source" : "There is another BumpTop Settings dialog already open!", "translation" : "" }
	key = "bumptopsettings.OnInitExistingInstance.text"
	if key not in db:
		db[key] = { "source" : "Am I seeing double?", "translation" : "" }
	key = "bumptopsettings.OnInitNoSettings.caption"
	if key not in db:
		db[key] = { "source" : "Could not find BumpTop's Settings file.", "translation" : "" }
	key = "bumptopsettings.OnInitNoSettings.text"
	if key not in db:
		db[key] = { "source" : "Hmm... Missing BumpTop Settings", "translation" : "" }
	key = "bumptopsettings.OnInitNoWindow.caption"
	if key not in db:
		db[key] = { "source" : "Hmm... No BumpTop window specified for this Settings dialog to edit. Certain changes will not be available and have been disabled.", "translation" : "" }
	key = "bumptopsettings.OnInitNoWindow.text"
	if key not in db:
		db[key] = { "source" : "No BumpTop Window Specified", "translation" : "" }
	key = "bumptopsettings.OnWindowCheckTimerClosed.caption"
	if key not in db:
		db[key] = { "source" : "Uh oh! The BumpTop window that this Settings dialog is editing has closed. Certain changes will not be applied and have been disabled.", "translation" : "" }
	key = "bumptopsettings.OnWindowCheckTimerClosed.text"
	if key not in db:
		db[key] = { "source" : "BumpTop Closed", "translation" : "" }
	key = "bumptopsettings.OnCancelOutstandingChanges.caption"
	if key not in db:
		db[key] = { "source" : "You have outstanding changes to the settings, do you want to apply them?", "translation" : "" }
	key = "bumptopsettings.OnCancelOutstandingChanges.text"
	if key not in db:
		db[key] = { "source" : "To apply changes, or not to apply changes", "translation" : "" }
	key = "bumptopsettings.OnToggleInfiniteDesktopPrompt.caption"
	if key not in db:
		db[key] = { "source" : "This will change the view and behaviour of your BumpTop.  Do you wish to continue?", "translation" : "" }
	key = "bumptopsettings.OnToggleInfiniteDesktopPrompt.text"
	if key not in db:
		db[key] = { "source" : "Do you want to go infinite?", "translation" : "" }
	key = "bumptopsettings.SettingsAppEventHandler.Top_Down_Default"
	if key not in db:
		db[key] = { "source" : "Top Down (Default)", "translation" : "" }
	key = "bumptopsettings.SettingsAppEventHandler.Overhead"
	if key not in db:
		db[key] = { "source" : "Overhead", "translation" : "" }
	key = "bumptopsettings.SettingsAppEventHandler.Bottom_Right_Corner"
	if key not in db:
		db[key] = { "source" : "Bottom Right Corner", "translation" : "" }
	key = "bumptopsettings.SettingsAppEventHandler.User_Defined"
	if key not in db:
		db[key] = { "source" : "User Defined", "translation" : "" }
	key = "bumptopsettings.SettingsAppEventHandler.Low_resolution_textures"
	if key not in db:
		db[key] = { "source" : "Low resolution textures", "translation" : "" }
	key = "bumptopsettings.SettingsAppEventHandler.Standard_resolution_textures"
	if key not in db:
		db[key] = { "source" : "Standard resolution textures", "translation" : "" }
	key = "bumptopsettings.SettingsAppEventHandler.High_resolution_textures"
	if key not in db:
		db[key] = { "source" : "High resolution textures", "translation" : "" }	
	key = "bumptopsettings.SettingsAppEventHandler.Low_resolution_textures_tooltip"
	if key not in db:
		db[key] = { "source" : "Uses less video memory for photo frames and pictures", "translation" : "" }
	key = "bumptopsettings.SettingsAppEventHandler.Med_resolution_textures_tooltip"
	if key not in db:
		db[key] = { "source" : "A good balance between performance and appearance", "translation" : "" }
	key = "bumptopsettings.SettingsAppEventHandler.High_resolution_textures_tooltip"
	if key not in db:
		db[key] = { "source" : "Use high quality images for photo frames and pictures", "translation" : "" }
	
			
def saveTSFromXRC(doc, language, existingDb):
	root = doc.firstChild
	whiteList = ["label", "title", "tooltip"]
	# extract all the strings as a set of key:value pairs
	recExtractXRCStrings(root, "btsettings", existingDb, whiteList)
	extractCustomStrings(existingDb)
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
		translation = ""
		translationAttr = " type=\"unfinished\""
		if len(value["translation"]) > 0:
			translation = value["translation"]
			translationAttr = ""		
		message = """
	<message>
		<location filename="%s" line="0"/>
		<source>%s</source>
		<translation%s>%s</translation>
	</message>""" % (key.encode("UTF-8"), value["source"].encode("UTF-8"), translationAttr, translation.encode("UTF-8"))
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
		"tr"
	]
	for language in languages:	
		
		# if possible, try and load the existing ts file
		existingDb = {}
		tsDoc = minidom.parse(TS_FILEOUT % language)
		root = tsDoc.getElementsByTagName("context")[0]
		recExtractTSStrings(root, existingDb)
			
		# extract any new strings from the xrc
		doc = minidom.parse('Settings.xrc')
		saveTSFromXRC(doc, language, existingDb)

	# It's nice to have some kind of output from the script.
	print "Extracted strings for %d languges." % len(languages)
