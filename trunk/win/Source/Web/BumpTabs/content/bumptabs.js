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

var BumpTop = function () {
	var Cc = Components.classes;
	var Ci = Components.interfaces;

	// According to http://mxr.mozilla.org/mozilla-central/source/nsprpub/pr/include/prio.h
	var PR_RDONLY = 0x01;
	var PR_WRONLY = 0x02;
	var PR_RDWR = 0x04;
	var PR_CREATE_FILE = 0x08;
	var PR_APPEND = 0x10;
	var PR_TRUNCATE = 0x20;

	// Return the data dir (an inst of nsILocalFile). Create the dir if it doesn't exist.
	function getTabDir() {
		var dir = Cc["@mozilla.org/file/directory_service;1"]
			.getService(Ci.nsIProperties)
			.get("ProfD", Ci.nsILocalFile);
		dir.append("BumpTabs");

		if (dir.exists()) {
			if (!dir.isDirectory()) {
				alert("Could not create data dir -- one with the same name already exists.");
			}
		} else {
			dir.create(Ci.nsIFile.DIRECTORY_TYPE, 0777);
		}
		
		return dir;
	}

	// Given a URL, create a web shortcut file (i.e., a .url) in the given dir.
	function writeUrlToDir(url, dir) {
		var file;

		// Continue looping until we can make a unique filename
		for (var i = 0; i < 999999; i++) {
			file = dir.clone();
			file.append("url" + i + ".url");
			file.QueryInterface(Ci.nsILocalFile);
			if (!file.exists()) {
				break;
			}
		}

		file.create(Ci.nsIFile.NORMAL_FILE_TYPE, 0666);

		try {
			var outputStream = Cc["@mozilla.org/network/file-output-stream;1"]
				.createInstance(Ci.nsIFileOutputStream);
			outputStream.init(file, PR_WRONLY | PR_TRUNCATE, 0666, 0);
			var converter = Components.classes["@mozilla.org/intl/converter-output-stream;1"].
									  createInstance(Components.interfaces.nsIConverterOutputStream);
			converter.init(outputStream, "UTF-8", 0, 0);
			converter.writeString("[InternetShortcut]\nURL=" + url);
			converter.close(); // Also closes outputStream
		} catch (err) {
			alert("Error: " + err);
		}
	}

	var prefManager = Cc["@mozilla.org/preferences-service;1"].getService(Ci.nsIPrefBranch);

	// Return an object with a single method. This allows us to call the 
	// function from the global context.
	return {
		bumpCurrentTabs: function () {
			var dir = getTabDir();
			var browsers = gBrowser.browsers;
			for (var i = 0; i < browsers.length; i++) {
				var url = browsers[i].currentURI.spec;
				writeUrlToDir(url, dir);
			}
			var file = Components.classes["@mozilla.org/file/local;1"]
				.createInstance(Components.interfaces.nsILocalFile);
			var exePath = prefManager.getCharPref("extensions.BumpTabs.BumpTopExePath");
			file.initWithPath(exePath);
			if (!file.exists()) {
				alert("The path " + exePath + " does not exist. Please go to about:config and change the pref 'extensions.BumpTabs.BumpTopExePath.");
			}

			// create an nsIProcess
			var process = Components.classes["@mozilla.org/process/util;1"]
									.createInstance(Components.interfaces.nsIProcess);
			process.init(file);
			var args = ["-d", dir.path];
			process.run(false, args, args.length);
		}
	};
}();
