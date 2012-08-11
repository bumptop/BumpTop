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

import copy
import codecs
import os
import os.path
import re
import shutil
import subprocess
import sys
from xml.dom import minidom, Node

import bti18n

# see http://stackoverflow.com/questions/275174/how-do-i-perform-html-decoding-encoding-using-python-django
htmlCodes = [
    ['&', '&amp;'],
    ['<', '&lt;'],
    ['>', '&gt;'],
    ['"', '&quot;']]
htmlCodesReversed = htmlCodes[:]
htmlCodesReversed.reverse()

class Stats: pass
stats = Stats()
stats.missing_count = 0 # Strings that exist in .ts file, but not present in .txt
stats.missing_untranslated_count = 0 # Strings UNTRANSLATED in .ts file, and not present in .txt
stats.deleted_count = 0 # Source exists in both files; translated in .ts but blank in .txt
stats.untranslated_count = 0 # Source exists & is untranslated in both files
stats.old_total = 0

def htmlDecode(s, codes=htmlCodesReversed):
    """ Returns the ASCII decoded version of the given HTML string. This does
        NOT remove normal HTML tags like <p>. It is the inverse of htmlEncode()."""
    for code in codes:
        s = s.replace(code[1], code[0])
    return s

def htmlEncode(s, codes=htmlCodes):
    """ Returns the HTML encoded version of the given string. This is useful to
        display a plain ASCII text string on a web page."""
    for code in codes:
        s = s.replace(code[0], code[1])
    return s
	
# Files given to translators contain explicit escape codes, to make it easier
# to preserve them. Replace the escape codes with the actual whitespace char
def unescape_whitespace(string):
	return string.replace('\\n', '\n').replace('\\r', '\r').replace('\\t', '\t')
							
#returns a dictionary of source text key and translated value
def readTxtEntries(filename):
	txtFile = codecs.open(filename, 'r', 'utf-16')
	original = ""
	translated = ""
	entries = {}
	waitingForOriginal = False
	waitingForTranslated = False
	for line in txtFile:
		if line.startswith("#"):
			# add the previous entry
			original = unescape_whitespace(original).rstrip()
			translated = unescape_whitespace(translated).rstrip()
			entries[original] = translated
			original = ""
			translated = ""
			waitingForOriginal = False
			waitingForTranslated = False
		else:
			if line.startswith("Original English (do not edit):"):
				waitingForOriginal = True
				waitingForTranslated = False
			elif line.startswith("Translated:"):
				waitingForTranslated = True
				waitingForOriginal = False
			elif waitingForOriginal:
				original += line + "\n"
			elif waitingForTranslated: 
				translated += line + "\n"
	return entries
		
def mergeTranslation(root, txtTranslations):
	mergedTranslations = {}
	global stats
	for child in root.childNodes:
		# Extract the translation from the .ts XML
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
			
			normalizedSource = unescape_whitespace(source)
			translation = translation.rstrip() # Strip trailing newline
			
			stats.old_total += 1

			# Replace the old translation (from the .ts) with the new one from the .txt
			# as long as the new one is non-empty
			if normalizedSource in txtTranslations:
				newTranslation = txtTranslations[normalizedSource].rstrip()
				if len(newTranslation) == 0:
					if len(translation) > 0:
						stats.deleted_count += 1
					else:
						stats.untranslated_count += 1
				else:
					translation = newTranslation
			else:
				stats.missing_count += 1
				if len(translation) == 0:
					stats.missing_untranslated_count += 1

			mergedTranslations[source] = { 
				"location" : location, 
				"source" : source, 
				"translation" : translation, 
				"type" : "unfinished" if len(translation) == 0 else ""
			}

	return mergedTranslations
					
def saveTS(tsFile, contextName, existingDb):
	tsFile.write("<context>\n")
	tsFile.write("\t<name>%s</name>" % contextName)
	for key, value in existingDb.items():
		location = value["location"]
		source = key
		translation = value["translation"]
		type = value["type"]
		typeString = ""
		if type == "unfinished":
			typeString = ' type="unfinished" '
		message = """
	<message>
		<location filename="%s" line="0"/>
		<source>%s</source>
		<translation%s>%s</translation>
	</message>""" % (htmlEncode(location).encode("UTF-8"), htmlEncode(source).encode("UTF-8"), typeString, htmlEncode(translation).encode("UTF-8"))
		tsFile.write(message)
	tsFile.write("\n</context>\n")

if __name__ == "__main__":
	USAGE = ("Usage: %s TXT_FILE TS_FILE [MERGED_FILENAME]\n" +
		"Reads the list of translations in text format from TXT_FILE,\n" +
		"and in QtLiguist (XML) format from TS_FILE, and merges them\n" +
		"into a new file named MERGED_FILENAME.\n" +
		"MERGED_FILENAME is optional, and if left out will be determined\n" +
		"automatically and created in the current directory.\n" +
		"The language is determined from the filename, assuming it ends in _XYZ.txt, \n" +
		"where XYZ is the language code (e.g. 'en', 'zh_cn').\n" +
		"\n" +
		"Prints statistics and warnings to stdout.\n")

	script_name = os.path.basename(sys.argv[0])
	if not 3 <= len(sys.argv) <= 4:
		sys.stderr.write(USAGE % script_name)
		sys.exit(1)
		
	txt_filename = sys.argv[1]
	ts_filename = sys.argv[2]

	assert txt_filename.endswith(".txt"), "First input file extension must be .txt"
	assert ts_filename.endswith(".ts"), "Second input file extension must be .ts"

	language = bti18n.language_from_filename(txt_filename)
	assert language == bti18n.language_from_filename(ts_filename), "Language inferred from filenames doesn't match"
	
	if len(sys.argv) > 3:
		out_filename = sys.argv[3]
		assert out_filename.endswith(".ts"), "Output file extension must be .ts"
		assert language == bti18n.language_from_filename(out_filename), "Language inferred from filenames doesn't match"
	else:
		# Output filename is based on the input .ts file.
		# BumpTop_ja.ts => BumpTop-merged_ja.ts
		pattern = "_%s.ts$" % language[0]
		replacement = "_merged_%s.ts" % language[0]
		out_filename = re.sub(pattern, replacement, os.path.basename(ts_filename))

	print ("Merging '%s' and '%s'..." %
		(txt_filename, ts_filename))
		
	txtTranslations = readTxtEntries(txt_filename)
	tsDoc = minidom.parse(ts_filename)
	
	# Write a qt linguist ts file, just writing the XML by hand)	
	# See http://www.i18nguy.com/unicode/language-identifiers.html
	with open(out_filename, 'w+') as mergedFile:
		header = '''<?xml version="1.0" encoding="utf-8"?>
	<!DOCTYPE TS>
	<TS version="2.0" language="%s">\n''' % language[0]
		mergedFile.write(header)

		contexts = tsDoc.getElementsByTagName("context")
		for context in contexts:
			contextName = context.getElementsByTagName("name")[0].firstChild.data
			mergedTranslations = mergeTranslation(context, txtTranslations)
			saveTS(mergedFile, contextName, mergedTranslations)
		
		mergedFile.write("</TS>")

	print "%d total source messages in .ts file.\n" % stats.old_total
	if stats.missing_count > 0:
		print ("WARNING: %4d source strings exist in the .ts but NOT in the .txt"
			% stats.missing_count)
		print  "         %4d still require translation" % stats.missing_untranslated_count
	if stats.deleted_count > 0:
		print ("WARNING: %4d are translated in the .ts file but blank in the .txt" 
			% stats.deleted_count)
	if stats.untranslated_count > 0:
		print  "WARNING: %4d remain untranslated in the .txt" % stats.untranslated_count
	print "\nTranslations merged into '%s'.\n" % out_filename

	bti18n.check_ts_merge(ts_filename, out_filename)
	
	answer = raw_input("\nReplace original .ts file with merged version? Y/N: ")
	if answer.lower() == "y":
		shutil.copyfile(out_filename, ts_filename)
		old_dir = os.getcwd()
		os.chdir("../Source")

		# Run lupdate to update the line numbers in the .ts file
		print "Running lupdate..."
		subprocess.Popen(["bin/lupdate.exe", "BumpTop.pro"]).communicate()
		
		# Run lrelease to convert the .ts file to a .qm
		print "Running lrelease..."
		subprocess.Popen(["bin/lrelease.exe", "BumpTop.pro"]).communicate()
	else:
		print "Ok, leaving original .ts file untouched."
