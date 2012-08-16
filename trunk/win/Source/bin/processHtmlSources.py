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

from jinja2 import Environment, BaseLoader, TemplateNotFound
import logging
import os

import localization

SCRIPT_NAME = os.path.basename(__file__)
SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))

# Take a path relative to this script, return a path relative to the working dir
def script_relative(path):
	return os.path.normpath(os.path.join(SCRIPT_DIR, path))

HTML_SOURCES = [
	"../Sharing/sharedFolder.tmpl.html",
	"../Sharing/dialogs/shareWith.tmpl.html",
	"../Sharing/dialogs/ConnectExistingShareDialog.tmpl.html",
	"../Sharing/dialogs/DeleteTabDialog.tmpl.html",
	"../Sharing/dialogs/InviteFriendsDialog.tmpl.html",
	"../Sharing/dialogs/CreateShareDialog.tmpl.html",
	"../Sharing/dialogs/SettingsModifyDialog.tmpl.html",
	"../Sharing/dialogs/ShareCreatedDialog.tmpl.html",
	"../Facebook/facebook_login.tmpl.html",
	"../Facebook/facebook_newsfeed.tmpl.html",
	"../Facebook/facebook_no_connection.tmpl.html",
	"../Facebook/facebook_photoalbums.tmpl.html",
	"../Facebook/facebook_upload.tmpl.html"
]

TRANSLATIONS_FILENAME = script_relative("../Languages/BumpTopJavaHTML_en.json")

COMMON_JS_PATH = "../Web/js" # Path to common web files, relative to this script

# Templates are assumed to be relative to the script dir
TEMPLATE_PATH = os.path.dirname(os.path.realpath(__file__))

NLS_CLASS = "BumpTop_NLS"

logger = localization.Logger(SCRIPT_NAME)

# This class allows us to pass a relative path as the name of a template
# Most of this class just copied from the Jinja documentation on loaders,
# http://jinja.pocoo.org/2/documentation/api#jinja2.BaseLoader
class RelativeFileSystemLoader(BaseLoader):
    def __init__(self, base_path):
        self._base_path = base_path

    def get_source(self, environment, template):
        path = os.path.normpath(os.path.join(self._base_path, template))
        if not os.path.exists(path):
            raise TemplateNotFound(template)
        mtime = os.path.getmtime(path)
        with file(path) as f:
            source = f.read().decode('utf-8')
        return source, path, lambda: mtime == os.path.getmtime(path)

sources = set()
locations = {}

class DynamicTranslations:
	def __init__(self):
		pass

	# Called for every instance of {{ _(<some_string>) }} in the template
	def ugettext(self, message):
		sources.add(message)
		locations.setdefault(message, []).append(self._source)
		return '<span class="%s">%s</span>' % (NLS_CLASS, message)

	# This function is used for pluralization.
	def ngettext(self, singular, plural, n):
		text = singular if n == 1 else plural
		return self.ugettext(text)
		
	def set_source(self, filename):
		self._source = filename

def save_as_html(template_path, template_str):
	if ".tmpl" in template_path:
		out_filename = script_relative(template_path.replace(".tmpl", ""))
		logger.info("Writing " + out_filename)
		with open(out_filename, 'w') as f:
			f.write(template_str)
	else:
		logger.warning(template_path + " doesn't contain '.tmpl' -- ignoring.")

# Set up the Jinja environment

loader = RelativeFileSystemLoader(SCRIPT_DIR)
translations = DynamicTranslations()

env = Environment(loader=loader, extensions=["jinja2.ext.i18n"])
env.install_gettext_translations(translations)

for each in HTML_SOURCES:
	translations.set_source(each)
	template = env.get_template(each)

	# Find the path to the common JS dir from this template's dir
	# We want this path to always use slash as a separator, not backslash
	jsDir = os.path.relpath(COMMON_JS_PATH, os.path.dirname(each))
	jsDir = jsDir.replace(os.path.sep, "/")

	result = template.render(nls_class=NLS_CLASS, common_js_dir=jsDir)
	save_as_html(each, result)

# Write all the translations to a file in JSON format
localization.save_to_file(TRANSLATIONS_FILENAME, sources, locations)
	
