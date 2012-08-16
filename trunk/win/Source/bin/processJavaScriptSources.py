#! /usr/bin/env python

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

# Run JSLint on JavaScript sources, and extract strings requiring translation.
# This script returns a failure code if JSLint detects any errors. Secondly,
# our modified version of JSLint extracts all string literals from the js source
# files, and all strings that are wrapped in a call to our translation function.
# This script compiles all the strings to be translated, and writes them to
# a JSON file in the Languages directory, merging with the previous contents of
# the file if it exists.

import collections
import json
import os
import subprocess
import sys

import localization # BumpTop helper module for localization

SCRIPT_NAME = os.path.basename(__file__)

# Take a path relative to this script, return a path relative to the working dir
def script_relative(path):
	thisdir = os.path.dirname(os.path.realpath(__file__))
	return os.path.normpath(os.path.join(thisdir, path))

LINT = script_relative("jslintAndExtractStrings.js")

# The source files that should be linted and checked for translations
# ALL BumpTop-written JavaScript should be listed here, even if it doesn't
# contain any user-visible strings, because it should be JSLINTed.
JAVASCRIPT_SOURCES = [
	"../Web/js/btutil.js",
	"../Sharing/BumpTop.js",
	"../Sharing/sharedFolder.js",
	"../Facebook/btfb_common.js",
	"../Facebook/btfb_kineticscroll.js",
	"../Facebook/btfb_login.js",
	"../Facebook/btfb_newsfeed.js",
	"../Facebook/btfb_photoalbums.js",
	"../Facebook/btfb_upload.js"	
]

TRANSLATIONS_FILENAME = script_relative("../Languages/BumpTopJavaScript_en.json")
UNTRANSLATED_FILENAME = script_relative("../Languages/BumpTopJavaScript_untranslated.txt")

all_translations = set()
all_untranslated = set()
locations = {} # Mapping from string => filename

logger = localization.Logger(SCRIPT_NAME)

returncode = 0

for filename in JAVASCRIPT_SOURCES:
	fd = None
	try:
		fd = os.open(script_relative(filename), os.O_RDONLY)
		logger.info("Processing %s..." % filename)
		p = subprocess.Popen(
			'cscript.exe "%s"' % LINT,
			stdin=fd,
			stdout=subprocess.PIPE,
			stderr=None) # Print stderr to the parent's stderr
		out, err = p.communicate()
		if p.returncode == 0:
			# Parse the output of JSLint
			translations = literals = None
			for line in out.splitlines():
				if line.startswith(("BumpTop stringLiterals:", "BumpTop translationStrings:")):
					# Everything after ':' is JSON; parse & add to appropriate list
					result_obj = json.loads(line.split(':', 1)[1])
					if "stringLiterals" in line:
						literals = result_obj
					else:
						translations = result_obj
	
			if translations is None or literals is None:
				logger.error("Failed to parse strings from " + LINT)
				raise Exception
	
			translations = frozenset(translations)
			untranslated = set(literals) - translations
			all_translations.update(translations)
			all_untranslated.update(untranslated)
	
			# For each translation, keep track of all the files it was found in
			for src in translations:
				locations.setdefault(src, []).append(filename)
		else:
			logger.error("%s returned %d" % (LINT, p.returncode))
			returncode = p.returncode

	finally:
		if fd:
			os.close(fd)	
			
localization.save_to_file(TRANSLATIONS_FILENAME, all_translations, locations)

# Finally, write all the untranslated strings to a text file, for sanity checking
with open(UNTRANSLATED_FILENAME, 'w+') as filep:
	for each in all_untranslated:
		filep.write(each + "\n")
		
sys.exit(returncode)
