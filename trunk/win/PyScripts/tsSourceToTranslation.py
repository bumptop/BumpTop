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

#defines
TS_FILEIN = "BumpTop_%s.ts"

def createSourceMapping(root, prefix, prefixCount, db):
	for child in root.childNodes:
		if child.nodeType == Node.ELEMENT_NODE:
			if child.nodeName == "context":
				# recurse into this with the context name as the prefix
				prefixCount += 1
				createSourceMapping(child, child.getElementsByTagName("name")[0].firstChild.data, prefixCount, db)
			elif child.nodeName == "message":
				prefixCount += 1
				childLocation = child.getElementsByTagName("location")[0]
				childSource = child.getElementsByTagName("source")[0]
				key = prefix + "|" + childLocation.getAttribute("filename") + "|" + childLocation.getAttribute("line") + "|"  + str(prefixCount)
				db[key] = childSource.firstChild.data
				
def swapSourceTranslations(root, doc, prefix, prefixCount, db):
	for child in root.childNodes:
		if child.nodeType == Node.ELEMENT_NODE:
			if child.nodeName == "context":
				prefixCount += 1
				# recurse into this with the context name as the prefix
				swapSourceTranslations(child, doc, child.getElementsByTagName("name")[0].firstChild.data, prefixCount, db)
			elif child.nodeName == "message":
				prefixCount += 1
				childLocation = child.getElementsByTagName("location")[0]
				childSource = child.getElementsByTagName("source")[0]
				childTranslation = child.getElementsByTagName("translation")[0]
				key = prefix + "|" + childLocation.getAttribute("filename") + "|" + childLocation.getAttribute("line") + "|"  + str(prefixCount)
				childTranslation.appendChild(doc.createTextNode(childSource.firstChild.data))
				if childTranslation.hasAttribute("type"):
					childTranslation.removeAttribute("type")
				childSource.removeChild(childSource.firstChild)
				childSource.appendChild(doc.createTextNode(db[key]))

if __name__ == "__main__":
	languages = [
		("zh_TW", "Chinese", "Taiwan"), 
		("zh_CN", "Chinese", "China"), 
		("ja", "Japanese", "Japan"), 
		("ko", "Korean", "Korea"), 
		("fr", "French", "France"), 
		("it", "Italian", "Italy"), 
		("de", "German", "Germany"), 
		("ru", "Russian", "Russia"), 
		("es", "Spanish", "Spain"), 
		("pt", "Portuguese", "Portugal")
	]
	
	# load the english ts file, and create a mapping of each of the context/file/line
	# to the source, then go through each other language, and move the source to the
	# translation and then replace the source with the 
	count = 0
	db = {}
	workingDir = os.getcwd()
	enTsFilePath = os.path.join(workingDir, TS_FILEIN % "en")
	enTsFile = minidom.parse(enTsFilePath)
	createSourceMapping(enTsFile.childNodes[1], "", count, db)
		
	for language in languages:		
		# try and load the existing ts file
		#try:
		count = 0
		tsFilePath = os.path.join(workingDir, TS_FILEIN % language[0])
		print tsFilePath
		if os.path.exists(tsFilePath):
			tsFile = minidom.parse(tsFilePath)
			swapSourceTranslations(tsFile.childNodes[1], tsFile, "", count, db)
			
			xml = tsFile.toxml("UTF-8")
			tsFileOut = open(tsFilePath, 'w+')
			tsFileOut.write(tsFile.toxml("UTF-8"))
			tsFileOut.close()
