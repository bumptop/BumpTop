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
					
def printUnfinisedTS(root):
	for child in root.childNodes:
		if child.nodeType == Node.ELEMENT_NODE:
			key = ""
			source = ""
			translation = ""
			unfinished = False
			for mChild in child.childNodes:
				mChildName = mChild.nodeName
				if mChildName == "location":
					key = mChild.getAttribute("filename")
				elif mChildName == "source":
					if mChild.hasChildNodes():
						assert mChild.firstChild.nodeType == Node.TEXT_NODE
						source = mChild.firstChild.data
				elif mChildName == "translation":
					if mChild.hasAttribute("type"):
						if mChild.getAttribute("type") == "unfinished":
							unfinished = True
			if unfinished:
				print source.encode("UTF-8")

if __name__ == "__main__":
	languages = [
		("en", "English", "Canada"), 
		("zh_TW", "Chinese", "Taiwan"), 
		("zh_CN", "Chinese", "China"), 
		("ja", "Japanese", "Japan"), 
		("ko", "Korean", "Korea"), 
		("fr", "French", "France"), 
		("it", "Italian", "Italy"), 
		("de", "German", "Germany")
		("ru", "Russian", "Russia"), 
		("es", "Spanish", "Spain"), 
		("pt", "Portuguese", "Portugal"),
		("tr", "Turkish", "Turkey"),
		("nl", "Dutch", "Holland"),
		("br", "Brazilian Portuguese", "Brazil")
	]
	for language in languages:
		# try and load the existing ts file
		existingDb = {}
		workingDir = os.getcwd()
		tsFilePath = os.path.join(workingDir, TS_FILEIN % language[0])
		tsDoc = minidom.parse(tsFilePath)
		root = tsDoc.getElementsByTagName("context")[0]
		printUnfinisedTS(root)
		
