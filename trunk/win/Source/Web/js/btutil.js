// JSLint options: "The Good Parts" minus 'plusplus', 'onevar', 'nomen'
/*jslint white: true, laxbreak: true, undef: true, nomen: false, eqeqeq: true, bitwise: true, regexp: true, strict: true, newcap: true, immed: true, maxerr: 50, indent: 4 */

/*global $ window document console alert setTimeout BumpTopNative */

"use strict";

var btutil = {
	// Take a function 'callback' and install it with a unique id in the window
	// object. Return the name.
	nameCallback: function (callback) {
		var name = "_callback_" + (new Date()).getTime();
		while (window.hasOwnProperty(name)) {
			name += "_";
		}
		window[name] = callback;
		return name;
	},
	
	dict_copy: function (obj) {
		var result = {};
		for (var key in obj) {
			if (obj.hasOwnProperty(key)) {
				result[key] = obj[key];
			}
		}
		return result;
	},
	
	debug: (function () {
		var debugFuncs = {}; // Container for all the debugging functions
		var self = {}; // The actual object that will be exported
		
		// Enable Firebug (lite) for the page. Note that although the script
		// is local, it references CSS & images from the internet, so things
		// might look funky if you don't have an internet connection.
		debugFuncs.enableFirebug = function () {
			// Add Firebug lite to the window, minimized by default
			if (!(window.hasOwnProperty("firebug"))) {
				// This code is just a slightly modified version of the
				// Firebug Lite bookmarket.
				var firebug = document.createElement('script');
				firebug.setAttribute('src', 'http://getfirebug.com/releases/lite/1.2/firebug-lite-compressed.js');
				document.body.appendChild(firebug);
				(function () {
					if (window.hasOwnProperty("firebug") && window.firebug.version) {
						window.firebug.env.debug = false; // Keep it hidden by default
						window.firebug.init();
						$("#firebugIconDiv")
							.css("border", "1px solid LightGray")
							.css("height", 10);
					} else {
						setTimeout(arguments.callee);
					}
				}());
			}
		};
		
		debugFuncs.assert = function (cond, message) {
			if (!cond) {
				var log_message = "ASSERTION FAILED";
				if (typeof(message) !== "undefined") {
					log_message += ": " + message;
				}
				console.log(log_message);
				console.log("    from " + arguments.callee.caller);
				alert(log_message);
			}
		};
		
		// Turn debugging on or off.
		// When debugging is turned off, all of this objects debug functions 
		// become no-ops
		self.setDebugMode = function (isDebugging) {
			var noop = function () {};
			for (var name in debugFuncs) {
				if (debugFuncs.hasOwnProperty(name)) {
					self[name] = isDebugging ? debugFuncs[name] : noop;
				}
			}
		};

		$(document).ready(function () {
			// Note: This will actually be a no-op if debugging is not enabled
//			self.enableFirebug();
		});
		
		self.setDebugMode(BumpTopNative.isDebugEnabled());

		return self;
	}())
};

// Global translation function. All user-visible strings must be wrapped
// in this function.
function TR(str) {
	return BumpTopNative.translate(str);
}
function TRF(formatStr, args) {
	return BumpTopNative.translateFormat(formatStr, args);
}
