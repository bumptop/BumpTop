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

from xml.dom import minidom, Node
from re import escape

import collections
import os
import sys
import copy

import bti18n

# Ignore obsolete translations (messages removed from the source code)?
IGNORE_OBSOLETE = True

# Keep messages in the order they were in the TS file?
MAINTAIN_ORDER = True

# Output escape characters (e.g. '\\n') instead of a raw LF, CR, or tab char
ESCAPE_WHITESPACE = True

# Replace whitespace characters with their escaped equivalent
# so that there's a better chance the translators will preserve them
def escape_whitespace(string):
	return string.replace('\n', '\\n').replace('\r', '\\r').replace('\t', '\\t')

# Walk the XML tree at root, and print out all the translation strings
# Return a dictionary of summary statistics, with keys:
# src_count, src_word_count, untranslated_count, untranslated_word_count
def writeTranslations(root, file):
	translations = {}

	# Initialize variables for summary stats
	stats = {}
	stats["src_count"] = 0
	stats["src_word_count"] = 0
	stats["untranslated_count"] = 0
	stats["untranslated_word_count"] = 0

	# Walk the tree
	for child in root.childNodes:
		if child.nodeType == Node.ELEMENT_NODE and child.nodeName == "message":
			source = None
			translation = None
			location = ""
			obsolete = False
			for mChild in child.childNodes:
				mChildName = mChild.nodeName
				if mChildName == "source":
					if mChild.hasChildNodes():
						assert mChild.firstChild.nodeType == Node.TEXT_NODE
						source = mChild.firstChild.data
				elif mChildName == "translation":
					# Obsolete translations are ones where the message was
					# removed from the source code. We keep the translation
					# in the TS in case we need it again, but skip it here.
					if mChild.getAttribute("type") == "obsolete":
						obsolete = True

					if mChild.hasChildNodes():
						assert mChild.firstChild.nodeType == Node.TEXT_NODE
						translation = mChild.firstChild.data
					else:
						translation = ""
				elif mChildName == "location":
					location = "%s:%s" % (mChild.getAttribute("filename"), mChild.getAttribute("line"))
				else:
					assert mChildName == "#text", "Unrecognized node name '%s'" % mChildName

			# Skip obsolete translations
			if obsolete and IGNORE_OBSOLETE:
				continue

			# Flag strings containing whitespace; they should be manually inspected
			if source != escape_whitespace(source) or len(source) != len(source.strip()):
				print "WARNING: Whitespace in source message at " + location
				
			if ESCAPE_WHITESPACE:
				source = escape_whitespace(source)
				translation = escape_whitespace(translation)

			assert len(source) > 0, "Empty message source at " + location

			# Use a dictionary to ensure no duplicate source strings
			if source not in translations:
				word_count = len(source.split())
				stats["src_word_count"] += word_count

				# The value includes the insertion index, so that we can output
				# them in the same order they occurred in the TS file
				translations[source] = (len(translations), translation)
			
				if len(translation) == 0:
					stats["untranslated_count"] += 1
					stats["untranslated_word_count"] += word_count

	stats["src_count"] = len(translations)

	# Sort the translations based on the insertion order
	sorted_translations = sorted(translations.items(), key=lambda x: x[1][0])
	for source, (insertion_index, translation) in sorted_translations:
		file.write("#####################\n")
		file.write("Original English (do not edit):\n")
		file.write(source.encode("UTF-8") + "\n")
		file.write("Translated:\n")
		if (len(translation) > 0):
			file.write(translation.encode("UTF-8") + "\n")
		else:
			file.write("@TRANSLATEME@\n")

	return stats

if __name__ == "__main__":
	USAGE = ("Usage: %s TS_FILE [DEST]\n" +
		"Reads the list of translations from TS_FILE, in QtLinguist (XML) format,\n" +
		"and writes them out in text format to DEST, which can be a dir or file.\n" +
		"If DEST is not specified, it defaults to the current dir.\n" +
		"The language is determined from the filename, assuming it ends in _XYZ.ts, \n" +
		"where XYZ is the language code (e.g. 'en', 'zh_cn').\n" +
		"\n" +
		"Prints statistics and warnings to stdout.\n")

	script_name = os.path.basename(sys.argv[0])
	if not 2 <= len(sys.argv) <= 3:
		sys.stderr.write(USAGE % script_name)
		sys.exit(1)
		
	in_filename = sys.argv[1]
	if len(sys.argv) > 2:
		dest = sys.argv[2]
	else:
		dest = "." # Default to the current dir

	assert in_filename.endswith(".ts"), "File extension must be .ts"

	# If dest is a dir, pick a filename
	if os.path.isdir(dest):
		out_filename = os.path.join(dest, os.path.basename(in_filename).replace(".ts", ".txt"))
	else:
		out_filename = dest

	# Ensure that the language codes in the filenames match
	language = bti18n.language_from_filename(in_filename)
	assert language == bti18n.language_from_filename(out_filename), "Language inferred from filenames doesn't match"

	# Try and load the existing .ts file
	tsDoc = minidom.parse(in_filename)

	# If a key doesn't exist in this dict, it defaults to 0		
	stats = collections.defaultdict(int)

	with open(out_filename, 'w') as f:
		for el in tsDoc.getElementsByTagName("context"):
			context_stats = writeTranslations(el, f)
	
			# All stats are integers; add these ones to the totals
			for key, val in context_stats.iteritems():
				stats[key] += context_stats[key]

	# Summarize the statistics across the entire doc
	print "--------------------------------------------------------------------"
	print "Language:     %s (%s)" % (language[0], language[1][0])
	print "Unique:       %d (%d words)" % (stats["src_count"], stats["src_word_count"])
	print "Untranslated: %d (%d words)" % (stats["untranslated_count"], stats["untranslated_word_count"])
	print "Output written to %s" % out_filename
