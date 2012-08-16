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
/*global window document $ BumpTop btutil TR console alert setInterval setTimeout clearTimeout escape */

"use strict";

console.log("Loading sharing widget...");

var debug = btutil.debug;
var ANIMATE = true;
var ITEM_WIDTH_PX = 168; // 150 + (2 * 8px) padding + (2 * 1px) border

// keeps track of whether the widget is busy (currently uploading file(s))
var uploading = false;

// number of files being uploaded
var uploadFileCount = 0;

// whether this is the first upload (startup)
var isFirstUpload = true;

// array to remember ongoing progress of all files being uploaded
var overallProgress = {};

// failure message displayed to user in tooltip when connecting to drop.io, uploading files, etc
var basicErrMsg = TR("Could not share file(s)");

// Global array to hold all tab objects being used
var tabArray = [];

var pendingUploads = 0;

// For all properties of an object (not including those of its prototype),
// create an array with the value of those properties.
// Useful for BumpTop API functions which *should* return an array, but instead
// return an object (due to limitations on return values from the C++ API)
function api_obj_to_array(obj) {
	var result = [];
	for (var key in obj) {
		if (obj.hasOwnProperty(key)) {
			result.push(obj[key]);
		}
	}
	return result;
}

function in_array(val, array) {
	for (var i in array) {
		if (val === array[i]) {
			return true;
		}
	}
	return false;
}

// Go through the tabs & store their settings in a file.
// Completely overwrites the previous settings file.
function updateSettingsFile() {
	var fileId = BumpTop.openFile("sharingTemp.json", "w");
	var settings = $.map(tabArray, function (tab) { 
		return tab.dropAccessParams; 
	});
	var fileContents = JSON.stringify(settings);
	BumpTop.writeFile(fileId, fileContents);
	BumpTop.renameAndCloseFile(fileId, "sharing.json");
}

// Return the Tab instance corresponding to the current selected tab,
// or null if the "new share" tab is currently selected.
function getCurrentTab() {
	if (tabArray.length > 0) {
		return tabArray[tabArray.length - 1];
	} else {
		return null;
	}
}

// Helper to show the specified status image on the drop target.
// 'name' must be a class of one of the <img> children of the
// base_container div.
function showStatusImage(name) {
	// Hide all others & show the specified image
	$("#droptarget .base_container img").hide(); 
	$("#droptarget .base_container img." + name).show();
}

// reset/init upload progress array and visuals, and start accepting drops
function resetProgress() {
	// reset progress indicator to 0
	$("#upload_progress").width(0);

	// reset progress array
	overallProgress = {};
	overallProgress.globalBytesSent = 0;
	overallProgress.globalBytesTotal = 0;
	
}

function uploadCancelled() {
	showStatusImage("ready");
	uploading = false;
	resetProgress();
}

function uploadComplete() {
	uploading = false;
	
	// show progress bar as full
	$("#upload_progress").css("width", $("#droptarget img.uploading").width());
}

// Constructor for tab objects.
var Tab = function (dropAccessParams) {
	var thisTab = this;

	this.dropAccessParams = dropAccessParams;
	this.selector = "#file_list"; // The CSS selector, for convenience
	
	// Keep track of space used and space available for drop
	this.bytesUsed = 0;
	this.maxBytes = 0;
	
	// Clone the connected_drop as the template for the new tab
	// Encapsulate it in a div starting from connected_drop0 and incrementing with each new tab
	var $tab = $(this.selector);
		
	// The width (in pixels) of the "fade out" image on the container edge.
	// We assume that it starts with a negative margin (see below).
	var leftFadeMargin = $tab.find(".left_edge").css("margin-left");
	leftFadeMargin = Math.abs(parseInt(leftFadeMargin.replace("px", ""), 10));

	// Keep track of the scroll state of the div, so we can restore it
	$tab.find(".scrolling_container").bind("scroll", function (evt) {
		var scrollPos = evt.target.scrollLeft;
		thisTab._savedScrollLeft = scrollPos;

		// When the container is scrolled all the way to the left, there should
		// be no fading visible on the left side. But we *do* want to show
		// the fading as soon as the container get scrolled. We achieve this
		// by starting the image with a negative margin, and incrementally
		// pulling it into the frame as the scroll bar position changes.
		var margin = Math.min(0, evt.target.scrollLeft - leftFadeMargin);
		$tab.find(".left_edge").css("margin-left", margin);
	});
};

Tab.prototype.getAuthParams = function () {
	var dropInfo = this.dropAccessParams;
	// Use the admin password if we have it, otherwise use the guest password
	var auth_params = {
		drop_name: dropInfo.dropName,
		token: (dropInfo.password === "") ? dropInfo.publicPassword : dropInfo.password
	};
	return auth_params;
};

// Create an email subscription to a BumpShare
Tab.prototype.subscribeToShare = function () {
	var self = this;
	var dropInfo = self.dropAccessParams;
	if (dropInfo.email.length === 0) {
		return;
	}
	var params = self.getAuthParams();
	params.email_addr = dropInfo.email;
	BumpTop.createShareSubscription(params, function (data, success) {
		if (!success) {
			console.log("Unable to subscribe to drop");
			console.log(data);
			self.warn(TR("Unable to subscribe to drop"));
		}
	});
};

// Delete the current tab from the UI, and prompt the user to delete the
// share as well.
Tab.prototype.deleteCurrentTab = function () {
	var self = this;
	var dropInfo = self.dropAccessParams;
	var index = $("#tabContainer").tabs('option', 'selected');
	var dialogParams = { 
		dropName: dropInfo.dropName,
		isAdmin: (dropInfo.password === "") ? false : true		
	};
	var result = BumpTop.openDialogWindow("DeleteTabDialog.html", dialogParams);
	if (result) {
		if (result.choice === "Yes") {
			// Delete the share too, if they chose to do so
			BumpTop.deleteShare(self.getAuthParams(), function (data, success) {
				if (!success) {
					console.log("Unable to delete drop");
					console.log(data);
				}
			});
		}
		$("#tabContainer").tabs('remove', index);
		tabArray.splice(index, 1);
		updateSettingsFile();
	}
};

Tab.prototype.updateTabSettings = function () {
	var self = this;
	var dropInfo = self.dropAccessParams;
	var newDropInfo = BumpTop.openDialogWindow("SettingsModifyDialog.html", dropInfo);
	if (newDropInfo) {
		// If user is not an admin, they can only change their email subscribed to the share
		if (dropInfo.password === "") {
			if (newDropInfo.email !== dropInfo.email) {
				dropInfo.email = newDropInfo.email;
				self.subscribeToShare();
				updateSettingsFile();
			}
			return;
		}
		// Replace any nulls with empty strings
		newDropInfo.dropName = dropInfo.dropName;	// Name does not change
		newDropInfo.password = newDropInfo.password || "";
		newDropInfo.publicPassword = newDropInfo.publicPassword || "";
		newDropInfo.email = newDropInfo.email || "";
		
		var params = {
			drop_name: newDropInfo.dropName, 
			token: dropInfo.password,	// Old password
			admin_email: newDropInfo.email
		};
		
		// Only send these parameters to be updated if they have changed since last time
		// Otherwise, the value of the token will be set as the new password
		if (newDropInfo.password !== dropInfo.password) {
			params.admin_password = newDropInfo.password;	// New password
		}
		if (newDropInfo.publicPassword !== dropInfo.publicPassword) {
			params.password = newDropInfo.publicPassword;
		}
		
		BumpTop.updateShare(params, function (data, success) {
			if (success) {
				// If drop was successfully updated, then overwrite current dropInfo associated with the tab
				dropInfo.password = data.admin_token;
				dropInfo.publicPassword = (typeof data.guest_token !== "undefined") ? data.guest_token : "";
				if (newDropInfo.email !== dropInfo.email) {
					dropInfo.email = newDropInfo.email;
					self.subscribeToShare();
				}
				updateSettingsFile();
			} else {
				console.log("Unable to update drop");
				console.log(data);
				self.warn(TR("Unable to update drop"));
			}
		});
	}
};

// Load the settings from the settings file.
function loadSettings() {
	var fileId = BumpTop.openFile("sharing.json", "r");
	if (fileId !== -1) {
		var contents = BumpTop.readFile(fileId);
		BumpTop.closeFile(fileId);
		var settings = JSON.parse(contents);
		debug.assert(typeof settings !== "undefined");
		if (!$.isArray(settings)) {
			settings = [settings];
		}
		for (var i = 0; i < settings.length; i++) {
			tabArray.push(new Tab(settings[i]));
		}
	}
}

// We save the scroll bar state every time it changes.
// Calling this method will restore it to the last saved state.
Tab.prototype.restoreScrollBarState = function () {
	var scrollingContainer = $(this.selector).find(".scrolling_container").get(0);
	scrollingContainer.scrollLeft = this._savedScrollLeft;
}; 

Tab.prototype.showMessage = function (className, message, timeout) {	
	var $messageArea = $(this.selector).find(".message_area");

	// Callback function to fade message after timeout
	function clearMessage() {
		// For some reason, unless we call hide(), messages that were faded out
		// on a previous tab will be visible when we switch to that tab.
		$messageArea.fadeOut("slow", function () { 
			$(this).hide(); 
		});
	}
	
	timeout = (typeof timeout === 'undefined') ? 3000 : timeout;
	$messageArea
		.text(message)
		.removeClass("ui-state-highlight ui-state-error")
		.addClass(className)
		.show();

	if (timeout > 0) {
		setTimeout(clearMessage, timeout);
	}
};

// Show a notification message in the message area
// timeout -- optional, specifies the timeout in millisecs (0 for no timeout).
Tab.prototype.notify = function (message, timeout) {
	this.showMessage("ui-state-highlight", message, timeout);
};

// Show a warning message in the message area
// timeout -- optional, specifies the timeout in millisecs (0 for no timeout).
Tab.prototype.warn = function (message, timeout) {
	this.showMessage("ui-state-error", message, timeout);
};

function mixInSharedItem(item) {
	var self = item;

	// Set the icon for this item.
	self.setIcon = function (domNode) {
		var DEFAULT_ICON_URL = "images/icon_default.png";
		var KNOWN_EXTENSIONS = ["doc", "docx", "xls", "xlsx", "ppt", "pptx", "pdf"];

		var ext_index = self.title.lastIndexOf(".");
		var ext = (ext_index >= 0) ? self.title.substr(ext_index + 1).toLowerCase() : null;
			
		var isImage;
		if (self.hasOwnProperty("type")) {
			isImage = (self.type === "image");
		} else {
			isImage = in_array(ext, ["png", "jpg", "jpeg", "gif"]);
		}
		
		var imageSrc = DEFAULT_ICON_URL;
		if (self.thumbnail) {
			imageSrc = self.thumbnail;
		} else if (isImage && self.url) {
			// Use the image itself, shrunk down
			imageSrc = self.url;
		} else if (in_array(ext, KNOWN_EXTENSIONS)) {
			if (in_array(ext, ["pptx", "xlsx", "docx"])) {
				ext = ext.slice(0, 3); // Strip off the "x"
			}
			imageSrc = "images/icon_" + ext + ".png";
		}
		$(domNode).find(".icon").attr("src", imageSrc);
	};

	self.update = function (updatedItem) {
		debug.assert(self.creation_time === updatedItem.creation_time,
			"mismatching items: " + self.title + " and " + updatedItem.title);

		self.title = updatedItem.title;
		self.thumbnail = updatedItem.thumbnail;
		self.url = updatedItem.url;
		$(self.domNode).find(".description").text(self.title).end();
	};
	
	self.isDeleted = false;
	self.isUploading = false;
	
	return self;
}

// Constructs a date string given the date a file was created
Tab.prototype.constructDateString = function (dateCreated) {
	var dateToday = new Date();
	var dateString;
	
	var amPM = (dateCreated.getHours() >= 12) ? TR("PM") : TR("AM");
	var hours = (dateCreated.getHours() >= 13) ? dateCreated.getHours() - 12 : dateCreated.getHours();
	var minutes = (dateCreated.getMinutes() < 10) ? ("0" + dateCreated.getMinutes()) : dateCreated.getMinutes();
	
	//	Number of milliseconds in one day
	var msecondsInDay = 1000 * 60 * 60 * 24;
	
	// Number of milliseconds ellapsed in the day the file was created
	var msecondsDayCreated = (dateCreated.getHours() * 60 * 60 * 1000) +
		(dateCreated.getMinutes() * 60 * 1000) + 
		(dateCreated.getSeconds() * 1000) + 
		dateCreated.getMilliseconds();
	
	if (dateCreated.getDate() === dateToday.getDate() &&
		dateCreated.getTime() >= (dateToday.getTime() - msecondsInDay)) {
		dateString = TR("Today, ") + hours + TR(":") + minutes + " " + amPM;
	} else if (dateCreated.getTime() >= (dateToday.getTime() - ((2 * msecondsInDay) - msecondsDayCreated))) {
		dateString = TR("Yesterday, ") + hours + TR(":") + minutes + " " + amPM;
	} else {
		var localeDate = dateCreated.toLocaleDateString();
		dateString = localeDate.substring(localeDate.indexOf(" "));
	}
	
	return dateString;
};

// Create the HTML element for the given item, add it to the DOM, and return it.
// position may be "start", "end", "before", or "after"
// If "before" or "after" is specified, relativeTo indicates the element we 
// should insert before or after.
Tab.prototype.addToPage = function (item, position, relativeTo) {
	var self = this;

	var template =
		'<div class="shared_item">' +
		'	<div class="styled_container">' +
		'		<a class="delete" href="#"/>' +
		'		<div class="overlay"><img class="throbber" src="images/throbber.gif"/></div>' +
		'		<div class="icon_container"><img class="icon"/></div>' +
		'		<div class="description">Filename.ext</div>' +
		'		<div class="date_created"></div>' +
		'	</div>' +
		'</div>';
		
	var dateString = self.constructDateString(new Date(item.created_at));
		
	var $item = $(template).clone()
		.hover(
			// On hover
			function () {
				$(this).find(".delete").show();	
			},
			// On hover out
			function () {
				$(this).find(".delete").hide();
			})
		.find(".date_created").text(dateString).end() // Set the date created
		.find(".description").text(item.title).end() // Set the text label
		.find(".overlay").hide().end() // Hide by default
		.width(0)
		.show();

	var domNode = $item.get(0);
	item.setIcon(domNode);
	
	// Add a handler for the "delete" button for each item.	
	$item.find(".delete").click(function () {
		var params = self.getAuthParams();
		params.asset_name = item.name;
		$item.find(".overlay").show();
		$item.find(".date_created").html("<i>" + TR("Deleting...") + "</i>");
		
		BumpTop.deleteFile(params, function (data, success) {
			if (success) {
				item.isDeleted = true;
				self.showOrHideElement(domNode);
			} else {
				self.warn(TR("Failed to delete file"));
			}
		});
	});

	if (item.isUploading) {
		$item.find(".overlay").show();
	} else {
		$item.dblclick(function () {
			$(this).find(".overlay").show();
			var thisEl = this;
			BumpTop.launch(item, function (success) {
				if (!success) {
					self.warn(TR("Unable to download file."));
				}
				$(thisEl).find(".overlay").hide();
			});
		});
	}

	if (position === "before") {
		$(relativeTo).before($item); 
	} else if (position === "after") {
		$(relativeTo).after($item);			
	} else if (position === "start") {
		$item.prependTo($(this.selector).find(".bump_share"));
	} else if (position === "end") {
		$item.appendTo($(this.selector).find(".bump_share"));
	}

	domNode.data = item; // Keep a ref to the original item
	item.domNode = domNode;
	return domNode;
};

Tab.prototype.completeAnimations = function () {
	this._last_update = (new Date()).getTime();
	// Make sure the animations are completed, rather than aborted.
	this._abort_animations = false;
};

// A helper for showing and hiding items in the share.
// 'el' is the DOM element. It must already be in the correct place in the DOM.
// If the el.data.isDeleted is true, the elementwill be "shown" by increasing 
// its width. Otherwise, it will be "hidden" by setting its width to 0, and then 
// removing it from the DOM.
// 'boolOrCallback' is an optional arg. If false, no animations will occur.
// Otherwise, animations will occur, and if boolOrCallback is a function, it
// will be called when the animations are complete. 
Tab.prototype.showOrHideElement = function (el, boolOrCallback) {
	var self = this;
	var ANIM_DURATION = 400; // Duration of animation in milliseconds
	var shouldAnimate = (boolOrCallback !== false);

	function adjustWidth() {
		var $container = $(self.selector).find(".bump_share");
		var oldContainerWidth = parseInt($container.css("width").replace("px", ""), 10);
		$container.width(oldContainerWidth + (el.data.isDeleted ? -ITEM_WIDTH_PX : ITEM_WIDTH_PX));
	}

	// Actually show or hide the element (possibly animated)
	
	var callback = $.isFunction(boolOrCallback) ? boolOrCallback : function () {};
	
	if (shouldAnimate) {
		if (el.data.isDeleted) {
			$(el)
				.fadeOut(ANIM_DURATION / 2, function () {
					$(el).css("visibility", "hidden");
				})
				.animate({ width: 0 }, ANIM_DURATION / 2, "linear", function () {
					$(el).remove();
					adjustWidth();
					callback();
				});
		} else {
			adjustWidth();
			$(el).animate({ width: ITEM_WIDTH_PX }, ANIM_DURATION, "linear", callback);
		}
	} else {
		if (el.data.isDeleted) {
			$(el).remove();
			adjustWidth();
		} else {
			adjustWidth();
			$(el).width(ITEM_WIDTH_PX);
		}
	}
};

// Update the list of items in the shared directory.
Tab.prototype._update_list = function (items) {
	var self = this;
	var $thisTab = $(this.selector);
	var currentElements = $thisTab.find(".shared_item").get();
	var animations = [];
	var updateTimestamp = (new Date()).getTime();
	var el, item;

	debug.assert($thisTab.length === 1, "Found more than 1 tab!");

	self.bytesUsed = 0;

	// Augment the objects
	$.each(items, function (i, item) {
		mixInSharedItem(item);
		self.bytesUsed = self.bytesUsed + item.filesize;
	});

	var uploadingElements = {};

	// Helper function for actually showing an element after it's been added.
	// Checks if the new element corresponds to a recently-uploaded one, and
	// if so, shows it immediately, and hides the other (temporary) one.
	function showAddedElement(el) {
		if (uploadingElements.hasOwnProperty(el.data.id)) {
			var uploadingEl = uploadingElements[el.data.id];
			delete uploadingElements[el.data.id];
			$(uploadingEl).remove();
			self.showOrHideElement(el, false);
		} else {
			animations.push(el);
		}
	}

	// Walk through the two lists and look for the differences
	while (currentElements.length > 0 && items.length > 0) {
		var oldItemCreation = currentElements[0].data.creation_time;
		var newItemCreation = items[0].creation_time;

		if (currentElements[0].data.isUploading) {
			el = currentElements.shift();
			// If it has the "id" field, then it has finished uploading, and
			// we should find a matching item has been added.
			if (el.data.hasOwnProperty("id")) {
				uploadingElements[el.data.id] = el;
			}
		} else if (oldItemCreation < newItemCreation) {
			// Add a new item
			el = this.addToPage(items.shift(), "before", currentElements[0]);
			showAddedElement(el);
		} else if (oldItemCreation > newItemCreation) {
			// Delete an existing item
			el = currentElements.shift();
			el.data.isDeleted = true;
			animations.push(el);
		} else {
			// Update the existing item
			el = currentElements.shift();
			item = el.data;
			item.update(items.shift());

			// If it didn't complete animating in the last update, do it now
			// TODO: Does this work if the item is not in the visible tab??
			if ($(el).width() === 0) {
				animations.push(el);
			}
		}
	}

	// Any leftover elements are deletions
	$.each(currentElements, function (i, el) {
		if (!el.data.isUploading) {
			el = currentElements.shift();
			el.data.isDeleted = true;
			animations.push(el);
		}
	});
	// Any leftover items are additions
	$.each(items, function (i, item) {
		showAddedElement(self.addToPage(item, "end"));
	});

	// Animate the items sequentially
	// In jQuery, animations for diff't items happen asynchronously, so we need
	// to do this to prevent all the animations from happening at once
	// NOTE: Deleted elements will be hidden here, and deleted on next update

	// We use these two instance variables as a crude way to control animations.
	// If _last_update changes, any animations started by this method call
	// will be aborted.	If _abort_animations is true, the animations will
	// simply be aborted, but this should ONLY be done if we are doing another
	// update. That is, NOT OTHER CODE SHOULD SET THIS TO TRUE!
	this._last_update = updateTimestamp;
	this._abort_animations = true;

	var i = animations.length;
	(function nextAnim() {
		i -= 1;
		// Checking against the updateTimestamp ensures that we stop
		// the animations from this update as soon as a new update happens.
		if (ANIMATE && self._last_update === updateTimestamp) {
			if (i >= 0) {
				self.showOrHideElement(animations[i], nextAnim);
			}
		} else if (!ANIMATE || !self._abort_animations) {
			for (; i >= 0; i--) {
				self.showOrHideElement(animations[i], false);
			}
		}
	}());
};

// Update the tab contents from the network.
Tab.prototype.update = function () {
	var self = this;
	var auth_params = self.getAuthParams();
	BumpTop.getFileInfoList(auth_params, function (data, success) {
		if (success) {
			debug.assert($.isArray(data));
			self._update_list(data);
		} else {
			console.log("getFileInfoList failed");
			console.dir(data);
			// Try and make this less fragile, look for return code
			if (data.response.message === "The drop does not exist.") {
				$(self.selector).find(".centered_container").hide();
				$(self.selector).find(".sharing_control").hide();
				$(self.selector).find(".share_deleted").show();
			} else {
				self.warn(TR("Unable to connect to server"), 0);
			}
		}
	});	
};

Tab.prototype.displayDropName = function () {
	var self = this;
	var dropInfo = self.dropAccessParams;
	// Put the name of the drop in the tab label, and the link in the tab links
	// This must be done here, and not in the Tab constructor, because the
	// drop name might change after the Tab has been created.
	$(self.labelNode).text(dropInfo.dropName);
	$(self.selector).find("a.drop_link").text(BumpTop.getDropUrl(dropInfo.dropName));
};

// Connect to existing drop and retrieve the token representation of the admin and guest password
Tab.prototype.connectToExistingDrop = function (isFirstConnect) {
	var self = this;
	var dropInfo = self.dropAccessParams;

	self.displayDropName();

	BumpTop.getShareInfo(self.getAuthParams(), function (data, success) {
		if (success) {
			self.notify(TR("Loading..."));
			if (isFirstConnect) {
				dropInfo.password = (typeof data.admin_token !== "undefined") ? data.admin_token : "";
				dropInfo.publicPassword = (typeof data.guest_token !== "undefined") ? data.guest_token : "";
				self.subscribeToShare();
				updateSettingsFile();
			}
			self.maxBytes = data.max_bytes;
			self.bytesUsed = data.current_bytes;
		} else {
			// User could not connect, so remove password so they only have user priveledges
			self.dropAccessParams.password = "";
			console.log("Unable to connect");

			// TODO: move into flyout
			alert(basicErrMsg);
			uploadCancelled();
		}
	});
};

// Creates a new share.
// 'callback' is an optional arg; if defined, it will be called (with no args)
// on successful completion.
Tab.prototype.createNewShare = function (callback) {
	var self = this;
	var dropInfo = this.dropAccessParams;
	var params = {
		name: dropInfo.dropName,
		admin_password: dropInfo.password,
		admin_email: dropInfo.email,
		password: dropInfo.publicPassword
	};
	BumpTop.createNewShare(params, function (data, success) {
		if (success) {
			// Drop name is changed automatically if it is a duplicate name
			// So overwrite the original with the new name that drop.io assigned

			self.maxBytes = data.max_bytes;
			self.bytesUsed = data.current_bytes;
			dropInfo.dropName = data.name;
			self.displayDropName();
			// Display drop information to the user
			console.log("Drop name: " + dropInfo.dropName);
//			BumpTop.openDialogWindow("ShareCreatedDialog.html", {dropName: dropInfo.dropName});
			if (typeof callback !== "undefined") {
				callback();
			}
			self.connectToExistingDrop(true);
		} else {
			self.warn(TR("Unable to create new share"), 0);

			// TODO: move into flyout
			alert(basicErrMsg);
			uploadCancelled();
		}
	});
};

// Prompt the user with a dialog to connect to an existing or new share
// Return true if the connection info was successfully entered, otherwise false
function getInfoFromDialog(fileName, callback) {
//	var dropInfo = BumpTop.openDialogWindow(fileName);

	var dropInfo = {};

	if (dropInfo) {
		// Replace any nulls with empty strings
		dropInfo.dropName = dropInfo.dropName || "";
		dropInfo.password = dropInfo.password || "";
		dropInfo.publicPassword = dropInfo.publicPassword || "";
		dropInfo.email = dropInfo.email || "";
		
		var newTab = new Tab(dropInfo);
		tabArray.push(newTab);

		if (fileName === "CreateShareDialog.html") {
			newTab.createNewShare(callback);
		} else {
			newTab.connectToExistingDrop(true);
		}
	}
}

// Upload a list of files to the share.
// files is an array of objects with properties "name", "mediaType", "size", 
// and "url". The URL is a filedata: URL that can be passed to BumpTop.
Tab.prototype.uploadFiles = function (files) {
	uploading = true;
	var self = this;
	var dropInfo = this.getAuthParams();
	var params = {
		version: "2.0",
		api_key: BumpTop.getApiKey(),
		format: "json"
	};
	$.extend(params, dropInfo);

	// Keep track of the entire upload, so that we know when it's done 
	var uploadData = {};
	uploadData.count = files.length;
	uploadData.pending = files.length;
	uploadData.files = {};
	uploadData.url = "";
	uploadData.files.status = "uploading";
	uploadData.files.toStr = "";

	// Keep track of total # of uploads in a global
	pendingUploads += 1;

	showStatusImage("uploading");
	resetProgress();
	
	function makeUploadCallback(fileSize) {
		return btutil.nameCallback(function (data, success) {
			var result = data;
			var succeeded = success;

			// XXX: Big hack here. Because we are directly using the native
			// sendHttpPostFileUpload, we have to extra work to put the
			// result back in the expected format. Ideally, we should use
			// a wrapper API like all the other ones, and we wouldn't have
			// to do this.

			if (success) {
				result = BumpTop.parseDropioResponse(data);
				succeeded = result.success;
			}

			if (succeeded) {
				self.bytesUsed += fileSize;
			} else {
				self.warn("Upload failed");
				console.log("Upload failed");
				console.dir(result.data);
				
				// TODO: move into flyout
				alert(basicErrMsg);
				return;
			}

			uploadData.pending -= 1;
			
			// if all dropped files have finished uploading
			if (uploadData.pending === 0) {
				pendingUploads -= 1;

				var newData = result.data;
				BumpTop.remapItemFields(newData, dropInfo.drop_name, dropInfo.token);

				if (uploadFileCount > 1) {
					uploadData.files.url = "http://drop.io/" + dropInfo.drop_name;
				} else {
					uploadData.files.url = newData.hidden_url;
				}
				uploadData.files.status = "link";

				// shorten drop.io url using tinyurl and display flyout
				// note	escape() turns spaces into %20, but QUrl turns %20 back into spaces (which breaks the url)
				//		so replace spaces with +, which escape does not convert (+ is equivalent to space in urls)
				BumpTop.sendXmlHttpRequest("GET"
					, "http://tinyurl.com/api-create.php?url=" + escape(uploadData.files.url.replace(' ', '+'))
					, btutil.nameCallback(function tinyUrlCallback(tinyUrl, success) {
						// use tinyurl if successful, otherwise fallback to drop.io url
						if (success) {
							uploadData.files.url = tinyUrl;
						}
						BumpTop.showFlyout(uploadData.files);
						showStatusImage("upload_complete");
						uploadComplete();
					})
				);
			}
		});
	}

	// find total size of all dropped files and build string of uploading filenames
	var allFilesSize = 0;
	for (var i = 0; i < files.length; i++) {
		allFilesSize += files[i].size;
		uploadData.files.toStr += files[i].name + ", ";
	}
	uploadData.files.toStr = uploadData.files.toStr.substring(0, uploadData.files.toStr.length - 2);
	
	// files too big to fit in a drop (100mb+)
	if (allFilesSize > self.maxBytes) {
		self.warn(TR("File won't fit in a drop!"));
		alert(TR("Files must be under 100MB"));
		pendingUploads -= 1;
		uploadCancelled();
		return true;
	}
	// files won't fit in current drop
	else if ((allFilesSize + self.bytesUsed) > self.maxBytes) {
		pendingUploads -= 1;
		return false;
	}
	
	// start all uploads
	for (var j = 0; j < files.length; j++) {
		BumpTop.sendHttpPostFileUpload(
			"http://assets.drop.io/upload",
			params,
			{ file: files[j].url },
			makeUploadCallback(files[j].size)
		);
	}

	BumpTop.showFlyout(uploadData.files);
	
	return true;
};

// show the progress of sharing upload
// current behaviour: crops empty file image higher and higher, showing full file image underneath
// empty file image height is reset to 100% just before opening sharing flyout, in makeUploadCallback
function updateProgress(whichFile, bytesSent, bytesTotal) {
	// if this is the first progress received for this file, init the sent to 0 and add its total to the global total
	if (!overallProgress.hasOwnProperty(whichFile)) {
		overallProgress[whichFile] = 0;
		overallProgress.globalBytesTotal += bytesTotal;
	}

	// update the global bytes sent with the difference since the last received progress
	overallProgress.globalBytesSent += bytesSent - overallProgress[whichFile];
	overallProgress[whichFile] = bytesSent;

	// recalculate overall progress
	var progress = 0;
	var almostDone = 0.98;
	if (overallProgress.globalBytesTotal !== 0) {
		progress = Math.min(almostDone, overallProgress.globalBytesSent / overallProgress.globalBytesTotal);
	}
	
	// Expand the progress bar image proportional to the progress
	var newWidth = progress * $("#droptarget img.uploading").width();
	$("#upload_progress").css("width", newWidth);
}

function uploadFiles(files) {	
	// whether a new share needs to be created
	// three cases:	1) multiple files are being dropped now (files.length > 1)
	//				2) multiple files were dropped last time (uploadedFileCount > 1)
	//				3) startup, don't know whether we can add to last used share
	var newShare = isFirstUpload || uploadFileCount > 1 || files.length > 1;
	uploadFileCount = files.length;
	isFirstUpload = false;

	// give upload started feedback asap
	updateProgress("", 0, 0);	
	
	function performUpload() {
		// try uploading to current share
		if (getCurrentTab().uploadFiles(files) === false) {
			// if false is returned, there isnt enough space, so upload to new share
			getInfoFromDialog("CreateShareDialog.html", performUpload);
		}
	}

	if (newShare === true || getCurrentTab() === null) {
		getInfoFromDialog("CreateShareDialog.html", performUpload);
	} else {
		performUpload();
	}
}

function onDragEnter(evt) {
	// evt is actually a jQuery wrapper; we need the original DOM event
	var dataTransfer = evt.originalEvent.dataTransfer;
	
	// immediately don't accept if already in the upload process
	if (uploading) {
		return true;
	}

	var acceptDrop = false;
	// Ensure that only files are being dropped
	for (var i = 0; i < dataTransfer.types.length; i++) {
		if (dataTransfer.types[i] === "Files") {
			acceptDrop = true;
		} else {
			acceptDrop = false;
			break;
		}
	}
	// If we are accepting the drop, we have to return false, to
	// prevent the default action
	return !acceptDrop;
}

function onDragOver(evt) {
	return false; // Means we accept the drag
}

function onDrop(evt) {	
	// evt is actually a jQuery wrapper; we need the original DOM event
	var files = evt.originalEvent.dataTransfer.files;
	uploadFiles(files);

	return false; // Means we accepted the drop	
}

function onDragLeave(evt) {
	return false;
}

$(document).ready(function () {
	// Register the required HTML5 drag-and-drop events
	// See http://www.whatwg.org/specs/web-apps/current-work/multipage/dnd.html#dnd
	$("body")
		.bind('dragenter', onDragEnter)
		.bind('dragleave', onDragLeave)
		.bind('dragover', onDragOver)
		.bind('drop', onDrop);
		
	$("#connectExistingShare").click(function () {
		getInfoFromDialog("ConnectExistingShareDialog.html");
	});
});

$(window).load(function () {
	var UPDATE_FREQUENCY_MILLIS = 10000; // Update every 10 secs

	// Try to load any saved settings.
	loadSettings();
	
	// if there is an existing share, connect to it to update the max size and remaining space
	/*if (tabArray.length > 0) {
		tabArray[tabArray.length - 1].connectToExistingDrop(false);
	}*/
});
