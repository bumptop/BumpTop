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

__all__ = [
	"get_languages", 
	"language_from_filename", 
	"parse_ts_file",
	"check_ts_merge"]

import os
import re
import sys
from xml.dom import minidom, Node

BT_LANGUAGES = {
	"en": ("English", "Canada"),
	"zh_TW": ("Chinese", "Taiwan"), 
	"zh_CN": ("Chinese", "China"), 
	"ja": ("Japanese", "Japan"), 
	"ko": ("Korean", "Korea"),
	"fr": ("French", "France"), 
	"it": ("Italian", "Italy"), 
	"de": ("German", "Germany"),
	"ru": ("Russian", "Russia"), 
	"es": ("Spanish", "Spain"), 
	"pt": ("Portuguese", "Portugal"),
	"tr": ("Turkish", "Turkey"),
	"nl": ("Dutch", "Holland"),
	"br": ("Brazilian Portuguese", "Brazil")
}

def get_languages():
	"""Returns a dictionary of all languages supported by BumpTop"""

	return BT_LANGUAGES 

def language_from_filename(filename):
	"""Figure out the language from a filename that ends with _XYZ.<filext>,
	where XYZ is a language code (e.g. "en", "fr", "de", etc.)"""

	basename, ext = os.path.splitext(filename)
	match = re.search("_([a-zA-Z]+)$", basename.lower())
	if match is None:
		raise Exception("Last part of filename before extension must end with '_XYZ', where XYZ is a valid language code.")
	lang = match.group(1)
	if lang not in BT_LANGUAGES:
		raise Exception("'%s' is not a language supported by BumpTop." % lang)
	return (lang, BT_LANGUAGES[lang])

# Files given to translators contain explicit escape codes, to make it easier
# to preserve them. Replace the escape codes with the actual whitespace char
def _unescape_whitespace(string):
	return string.replace('\\n', '\n').replace('\\r', '\r').replace('\\t', '\t')
	
def _parse_translations(contextNode):
	translations = {}
	for child in contextNode.childNodes:
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
			# We use rstrip() to ensure there is no trailing newline
			normalizedSource = _unescape_whitespace(source).rstrip()
			translation = translation.rstrip()

			translations[source] = { 
				"location" : location, 
				"source" : source, 
				"translation" : translation, 
				"type" : "unfinished" if len(translation) == 0 else ""
			}
	return translations
		
def parse_ts_file(filename):
	tsDoc = minidom.parse(filename)	
	contexts = tsDoc.getElementsByTagName("context")
	result = {}
	for context in contexts:
		contextName = context.getElementsByTagName("name")[0].firstChild.data
		result[contextName] = _parse_translations(context)
	return result
	
def _verify_translations(translationDict):
	"""Verify a set of translations that were extracted from a .ts file
	using parse_ts_file(). Just does some basic sanity checks."""
	
	for contextName, strings in translationDict.iteritems():
		for source, trans in strings.iteritems():
			assert trans["type"] in ["unfinished", ""], "Unknown translation type"
			assert len(trans["source"]) > 0, "Empty source string"
			if trans["type"] == "unfinished":
				assert len(trans["translation"]) == 0, "Non-empty translation with type 'unfinished'"
			else:
				assert len(trans["translation"]) > 0, "Empty translation not marked 'unfinished'"

def _count_sources(translationDict):
	count = 0
	for contextName, strings in translationDict.iteritems():
		for source in strings:
			count += 1
	return count

def check_ts_merge(old_ts_filename, new_ts_filename):
	"""Do some basic checks on two TS files, assuming that
	new_ts_filename is intended to replace old_ts_filename.
	Prints summary statistics (for sanity checking) to stdout."""
	
	old_translations = parse_ts_file(old_ts_filename)
#	sys.stdout.write("Verifying .ts file '%s'..." % old_ts_filename)
	_verify_translations(old_translations)
#	print "ok."
	new_translations = parse_ts_file(new_ts_filename)
#	sys.stdout.write("Verifying .ts file '%s'..." % new_ts_filename)
	_verify_translations(new_translations)
#	print "ok."
	
	untouched_count = 0 # Translations are identical in both files
	added_count = 0 # String untranslated in old file, translated in new file
	updated_count = 0 # Translation was changed between old file and new file
	deleted_count = 0 # Translation existed in old file, was empty in new file
	missing_count = 0 # Source string is missing from new file
	untranslated_count = 0 # String is untranslated in both files

	print "Comparing translations..."

	for contextName, oldDict in old_translations.iteritems():
		if contextName not in new_translations:
			print "WARNING: No context named '%s' in new translations" % contextName
		else:
			newDict = new_translations[contextName]
			for source, trans in oldDict.iteritems():
				if source not in newDict:
					missing_count += 1
				else:
					newTrans = newDict[source]
					if trans["type"] == "unfinished":
						if newTrans["type"] == "unfinished":
							untranslated_count += 1
						else:
							added_count += 1
					else:
						if newTrans["type"] == "unfinished":
							deleted_count += 1
						elif newTrans["translation"] == trans["translation"]:
							untouched_count += 1
						else:
							updated_count += 1

	print "  %4d identical translations" % untouched_count
	print "  %4d new translations added from .txt file" % added_count
	print "  %4d updated translations" % updated_count
	if untranslated_count > 0:
		print "WARNING: %4d strings remain untranslated" % untranslated_count
	if deleted_count > 0:
		print "WARNING: %4d translations made blank in the new .ts file" % deleted_count
	if missing_count > 0:
		print "WARNING: %4d strings from original .ts file don't appear in new .ts file" % missing_count
	old_total = _count_sources(old_translations)
	new_total = _count_sources(new_translations)
	if old_total != new_total:
		print ("ERROR: %d source strings in original .ts file, %s in new file" %
			(old_total, new_total))
