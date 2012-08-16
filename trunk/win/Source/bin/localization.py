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

__all__ = ["Logger", "save_to_file"]

import json
import os
import sys

SCRIPT_NAME = os.path.basename(__file__)

# A helper class for logging message in a way that Visual Studio can
# recognize the errors
class Logger:
	def __init__(self, prefix):
		self._prefix = prefix
		
	def info(self, message):
		sys.stderr.write("%s (1): info: %s\n" % (self._prefix, message))
		sys.stderr.flush()
		
	def error(self, message):
		sys.stderr.write("%s (1): error: %s\n" % (self._prefix, message))
		sys.stderr.flush()
	
	def warning(self, message):
		sys.stderr.write("%s (1): warning: %s\n" % (self._prefix, message))
		sys.stderr.flush()
		
_logger = Logger(SCRIPT_NAME)

# Write all the translations to a file in JSON format
# filename - the filename to read the existing strings from, and write to
# sources - a set of localizable strings
# locations - a dict for mapping from a localizable string to a string
#   describing the source location.
def save_to_file(filename, sources, locations):
	existing_translations = {}
	
	# Try to read the old file in so we can update it
	if os.path.isfile(filename):
		try:
			with open(filename, 'r') as filep:
				for message in json.load(filep):
					existing_translations[message["source"]] = message["translation"]
		except Exception as err:
			_logger.warning("Couldn't read %s: %s" % (TRANSLATIONS_FILENAME, err))
	
	# Now write the current set of translation strings, ordered by the source string
	
	final_list = []
	for src in sorted(sources):
		location = ",".join([str(x) for x in locations[src]])
		translation = existing_translations.get(src, "")
		# The JSON file is just a list of these objects
		final_list.append({ "location": location, "source": src, "translation": translation})
	
	# Write it out to the file
	with open(filename, 'w+') as filep:
			json.dump(final_list, filep, sort_keys=True, indent=4)
