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

# Merges the results of processJavaScriptSource.py and processHTMLSources.py
# into a single TS file that can be translated and loaded by BumpTop

import json
import os
import sys
import localization # BumpTop helper module for localization

# Take a path relative to this script, return a path relative to the working dir
def script_relative(path):
	thisdir = os.path.dirname(os.path.realpath(__file__))
	return os.path.normpath(os.path.join(thisdir, path))

SOURCES = [
	script_relative("../Languages/BumpTopJavaScript_en.json"),
	script_relative("../Languages/BumpTopJavaHTML_en.json")
]

QT_TRANSLATIONS_FILENAME = script_relative("../Languages/BumpTop_Web_en.ts")
translation_entries = []
existing_translation_entry_sources = set()

SCRIPT_NAME = os.path.basename(__file__)
logger = localization.Logger(SCRIPT_NAME)

# Read all the translation entries
for filename in SOURCES:
	fd = open(filename, 'r')
	logger.info("Processing %s..." % filename)
	data = json.load(fd)
	for entry in data:
		# ensure that there are no duplicate sources
		if entry["source"] not in existing_translation_entry_sources:
			translation_entries.append(entry)
			existing_translation_entry_sources.add(entry["source"])
	fd.close()

# Helper func to html-encode a string
htmlCodes = [
    ['&', '&amp;'],
    ['<', '&lt;'],
    ['>', '&gt;'],
    ['"', '&quot;'],
]
def htmlEncode(s):
    for code in htmlCodes:
        s = s.replace(code[0], code[1])
    return s
		
# Write the TS file output
fd = open(QT_TRANSLATIONS_FILENAME, 'w+')
fd.write(
'''<?xml version="1.0" encoding="utf-8"?>
<!--
// Copyright 2011 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
-->
<!DOCTYPE TS>
<TS version="2.0" language="en_US">
<context>
    <name>JavaScriptAPI</name>
''')
for entry in translation_entries:
	fd.write(
	'''	
	<message>
        <location filename="%s" line="0"/>
        <source>%s</source>
        <translation>%s</translation>
    </message>''' % (htmlEncode(entry['location']), htmlEncode(entry['source']), htmlEncode(entry['translation'])))

fd.write(
'''</context>
</TS>
''')
fd.close()
