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

// JSLint options: "The Good Parts" minus 'plusplus', 'onevar', 'nomen'
/*jslint white: true, laxbreak: true, undef: true, nomen: false, eqeqeq: true, bitwise: true, regexp: true, strict: true, newcap: true, immed: true, maxerr: 50, indent: 4 */
/*global window console $ BumpTopNative btutil alert */

"use strict";

// Check if we already have an API object installed. If so, assume that it's
// the native API. If not, create an install the web API.
if (!window.hasOwnProperty("BumpTopNative")) {
	// Install the Web API
	window.BumpTop = (function () {
		var BumpTop = {};
	
		BumpTop.getFileInfoList = function (callback) {
//			var items = api_obj_to_array(BumpTop.getFileInfoList(LastUpdate));
//			callback(items, true);
		};
		
		return BumpTop;
	}());
	console.log("BumpTop web API installed\n");
}

function createDropioClient(default_options) {
	var self = {};
	var BASE_URL = "http://api.drop.io";
	var defaults = $.extend({ version: "2.0", format: "json" }, default_options);

	function toQueryString(opts) {
		var queryString = "";
		for (var key in opts) {
			if (opts.hasOwnProperty(key)) {
				var sep = (queryString.length > 0) ? "&" : "?";
				queryString += sep + encodeURIComponent(key) + "=" + encodeURIComponent(opts[key]);
			}
		}
		return queryString;
	}

	// Takes a template for a Drop.io API method and a list of parameters.
	// The template may contain variables that are prefix by ':', which will
	// be replaced by the value of the same-named property in 'params'.
	// Any remaining properties will be put into the query string.
	self.getApiUrl = function (template, params) {
		var opts = $.extend({}, defaults);
		$.extend(opts, params);
		
		// Find all the variable parts of the pathTemplate
		var pattern = /\/:([A-Za-z_]+)($|\/)/g;
		var match;
		var templateVars = [];
		while ((match = pattern.exec(template))) {
			templateVars.push(match[1]);
		}

		// Substitute options into the path template
		var name;
		var path = template;
		for (var i = 0; i < templateVars.length; i++) {
			name = templateVars[i];
			if (opts.hasOwnProperty(name)) {
				path = path.replace(":" + name, opts[name]);
				delete opts[name];
			} else {
				// ERROR: Missing a required param
				return null;
			}
		}
		path += toQueryString(opts);			
		return BASE_URL + path;
	};
	
	// A helper function for parsing a response from Drop.io.
	// This should NOT be public, but because sendHttpPostFileUpload is being
	// called directly from sharedFolder.js, it needs this to parse the response
	// properly. Instead, that should be changed to call a wrapper function,
	// to keep all the callers of this function inside this object.
	self.parseDropioResponse = function (data, requestInfo) {
		var resultObj, succeeded;
		try {
			resultObj = JSON.parse(data);

			// Look  for specific failures from the Drop.io API
			succeeded = !((resultObj.hasOwnProperty("response") && 
				resultObj.response.hasOwnProperty("result") &&
				resultObj.response.result === "Failure"));
		} catch (err) {
			succeeded = false;
			resultObj = {"response": data, "error": ("" + err) };
		}
		// If the request failed, include some debug info
		if (!succeeded) {
			resultObj = { request: requestInfo, response: resultObj };
		}

		return { data: resultObj, success: succeeded };
	};
	
	// Helper function for making a Drop.io API call.
	// Substitutes 'options' into the URL, and makes the request.
	function callDropioApi(pathTemplate, httpMethod, options, callback) {
		var url = self.getApiUrl(pathTemplate, options);

		// This object is for debugging use
		var requestInfo = {};
		requestInfo.request = httpMethod + " " + url;

		// The callback from BumpTop needs to be a function that is
		// accessible in the global scope -- nameCallback gives us that.
		// We use an extra helper here because we want to hand back JSON
		// from the JavaScript API, rather than a raw string.
		var callbackName = btutil.nameCallback(function (data, success) {
			if (success) {
				var parsedResult = self.parseDropioResponse(data, requestInfo);
				callback(parsedResult.data, parsedResult.success);
			} else {
				callback(data, false);
			}
		});
		BumpTopNative.sendXmlHttpRequest(httpMethod, url, callbackName);		
	}
	
	self.createSubscription = function (options, callback) {
		callDropioApi("/drops/:drop_name/subscriptions", "POST", options, callback);
	};

	self.createDrop = function (options, callback) {
		callDropioApi("/drops/", "POST", options, callback);
	};
	
	self.updateDrop = function (options, callback) {
		callDropioApi("/drops/:drop_name", "PUT", options, callback);
	};

	self.getDrop = function (options, callback) {
		callDropioApi("/drops/:drop_name", "GET", options, callback);
	};
	
	self.deleteDrop = function (options, callback) {
		callDropioApi("/drops/:drop_name", "DELETE", options, callback);
	};

	self.getAssetList = function (options, callback) {
		callDropioApi("/drops/:drop_name/assets", "GET", options, callback);
	};
	
	self.deleteAsset = function (options, callback) {
		callDropioApi("/drops/:drop_name/assets/:asset_name", "DELETE", options, callback);
	};
	
	return self;
}

function createDropioApi(wrappedApi) {
	var self = {};
	var API_KEY = "e21929fd5f2bcb279c56078997d93835c7ec37f7";
	var PREMIUM_KEY = "a2j10jfshvba1d9sj91m9ks7s2d1tn9sd32fs93op2du9";
	var dropio = createDropioClient({api_key: API_KEY});

	// A helper for ensuring that the parameters passed to a function
	// include all the required ones. Takes 2 or more arguments: the first is
	// the parameters object, everything after that is a string naming a
	// required parameter.
	// e.g. ensureRequiredParams(paramObj, "req1", "req2", "req3");
	function ensureRequiredParams(params /*, ... */) {
		btutil.debug.assert(arguments.length > 1);

		for (var i = 1; i < arguments.length; i++) {
			btutil.debug.assert(params.hasOwnProperty(arguments[i]));
		}
	}
	
	// TODO: This should not be public! It's really just an internal helper.
	// But do to the fact that sendHttpPostFileUpload is not properly
	// encapsulated, there is a need to use this function during uploading.
	// Fix this!
	self.remapItemFields = function (obj, drop_name, token) {
		var template = "/drops/:drop_name/assets/:asset_name/download/original";
		obj.url = dropio.getApiUrl(template, { drop_name: drop_name, token: token, asset_name: obj.name });

		obj.creation_time = Date.parse(obj.created_at);
		// Some APIs return a date format that isn't parseable by JavaScript.
		// e.g. "2009-12-13 02:37:39 UTC" instead of "2009/12/13 02:37:39 +0000"
		// Catch that case, and correct it.
		if (isNaN(obj.creation_time) && obj.created_at.indexOf("UTC") >= 0) {
			var dateStr = obj.created_at.replace(/-/g, "/").replace("UTC", "+0000");
			obj.creation_time = Date.parse(dateStr);
		}

		// On Drop.io, the name is unique to the drop, among existing files.
		// So name + creation_time should be fully unique.
		obj.id = obj.name + "-" + obj.creation_time;

		return obj;
	};

	// For all objects in the list, convert from the Drop.io API format
	// to our preferred format. Essentially just renames a few fields.
	function remapFields(list, drop_name, token) {
		var obj;
		for (var i = 0; i < list.length; i++) {
			self.remapItemFields(list[i], drop_name, token);
		}
		return list;
	}
	
	self.createShareSubscription = function (params, callback) {
		ensureRequiredParams(params, "drop_name", "token", "email_addr");
		dropio.createSubscription({
				drop_name: params.drop_name,
				token: params.token,
				type: "email",
				emails: params.email_addr,
				asset_added: true,
				asset_updated: true,
				asset_deleted: true
			}, callback);
	};
	
	self.createNewShare = function (params, callback) {
		ensureRequiredParams(params, "name", "password", "admin_password", "admin_email");
		// uncomment following line to enable premium features
		//var fullParams = $.extend({ premium_code: PREMIUM_KEY }, params);
		dropio.createDrop(params, callback);
	};
	
	self.updateShare = function (params, callback) {
		ensureRequiredParams(params, "drop_name", "token");
		dropio.updateDrop(params, callback);
	};
	
	self.getShareInfo = function (params, callback) {
		ensureRequiredParams(params, "drop_name", "token");
		dropio.getDrop(params, callback);
	};
	
	self.deleteShare = function (params, callback) {
		ensureRequiredParams(params, "drop_name", "token");
		dropio.deleteDrop(params, callback);
	};
	
	self.getFileInfoList = function (params, callback) {
		ensureRequiredParams(params, "drop_name", "token");
		dropio.getAssetList($.extend(params, {order: "latest"}), function (data, success) {
			var actualData = data;
			if (success) {
				// TODO: The results here are actually paged. This will only fetch
				// the most recent items in the list.
				actualData = remapFields(data.assets, params.drop_name, params.token);
			}
			callback(actualData, success);
		});
	};
	
	self.deleteFile = function (params, callback) {
		ensureRequiredParams(params, "drop_name", "token", "asset_name");
		dropio.deleteAsset(params, callback);
	};
	
	// Launch an item natively, as if it was double-clicked on.
	// Callback is called after the file has been downloaded and launched;
	// it takes one parameter: true if the download/launch succeeded.
	self.launch = function (item, callback) {
		var callbackName = btutil.nameCallback(callback);
		BumpTopNative.launch(item.url, callbackName, item.original_filename);
	};
	
	self.openDialogWindow = function (fileName, data) {	
		var returnData;
		if (typeof data === "undefined") {
			returnData = BumpTopNative.openDialogWindow(fileName);
		} else {
			returnData = BumpTopNative.openDialogWindow(fileName, data);
		}
		
		// Check to see if the object is empty
		for (var prop in returnData) {
			if (returnData.hasOwnProperty(prop)) {
				return returnData; // Return object if it has any field set
			}
		}
		return null;
	};

	self.getDropUrl = function (dropName) {
		return "http://drop.io/" + dropName;
	};
	
	// TODO: We shouldn't need this!
	self.getApiKey = function () {
		return API_KEY;
	};
	
	// TODO: This function should be necessary! It indicates that the Drop.io
	// API is not propertly encapsulated.
	self.parseDropioResponse = function (strResponse, requestInfo) {
		return dropio.parseDropioResponse(strResponse, requestInfo);
	};
	
	function apiWrapper(obj, name) {
		var func = function () { 
			return obj[name].apply(obj, arguments);
		};
		return func;
	}
	
	// The list of API for which we should just call directly through to the 
	// native implementation.
	var inheritedApi = [
		"sendHttpPostFileUpload",
		"sendXmlHttpRequest",
		"openFile",
		"readFile",
		"writeFile",
		"closeFile",
		"renameAndCloseFile",
		"openInBrowser",
		"openFileBrowser",
		"copyTextToClipboard",
		"showFlyout"
	];

	// Add them all as methods on this object
	for (var i = 0; i < inheritedApi.length; i++) {
		var name = inheritedApi[i];
		self[name] = apiWrapper(BumpTopNative, name);
	}

	return self;
}
// Install the Drop.io API
window.BumpTop = createDropioApi(BumpTopNative);
