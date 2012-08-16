/*
 Copyright 2011 Google Inc. All Rights Reserved.

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
*/

// Represents the facebook newsfeed page
function FacebookWidgetNewsFeedPage() {
	this.monthNames = [TR("January"), TR("February"), TR("March"), 
		TR("April"), TR("May"), TR("June"), TR("July"), TR("August"), 
		TR("September"), TR("October"), TR("November"), TR("December")];

	// time of oldest (rightmost) photo in the feed
	var earliestPhotoTime = -1;
	
	// whether the user has clicked More but the feed hasn't been updated yet
	var loadingMore = false;

	// position of the feed scrollbar
	var scrollPos = 0;

	// map of html items to their corresponding photo pid
	var pidToItemsMap = {};

	// map of html items to their corresponding post id
	var postIdToItemsMap = {};

	// which photo the slideshow is focused on (0 = start on the first photo)
	var slideShowPos = 0;

	// the last position where the slideshow scrolled to
	var slideShowLast = 0;
	
	// constants for post types
	var STATUS_POST = "status";
	var PHOTO_POST = "photo";
	
	// constants for feed filters
	var STATUS_FILTER = "Status Updates";
	var PHOTO_FILTER = "Photos";
	var LINKS_FILTER = "Links";
	
	// width of feed without items (but including space + more button)
	var emptyFeedWidth = $('.load_more').width() + 2 * ($('.load_more').css("marginLeft").substring(0, $('.load_more').css("marginLeft").indexOf("px")));

	// Constructor
	this.init = function() {
		console.log("Loading photo feed...");

		// call super constructor
		this.initCommon();
		// save the current page
		window.BumpTopNative.setPersistentStoreValue('last_page', 'newsfeed');

		// bind the kinetic scroll wrapper
		$("#btfb_content .feed").kineticScroll();
		$("#btfb_content .feed").width(emptyFeedWidth);

		$('.image').live('mouseover', this.onItemMouseOver);

	    // status bar events
	    $('.status_update_textarea').bind("touchclick", this, this.scrollToStatus);
	    $('#btfb_header').bind('click', this, this.scrollToStatus);
	    
		// set up the dropdown options w/ checkboxes
	    this.initOptions();

		// populate the feed and status now
		// try and load the cached updates, and then update as necessary
		if (!this.loadCachedFeed()) {
			this.requestFeedUpdate();
		}

		// Do an update ASAP, then set a timer to periodically update
		$(document).oneTime("1ms", this.requestFeedUpdate);
		$(document).everyTime("300s", this.requestFeedUpdate);

		// the proxy drag and drop is being implemented in the parent
		$("body")
			.bind('dragenter', this, this.onDragEnter)
			.bind('dragleave', this, this.onDragLeave)
			.bind('dragover', this, this.onDragOver)
			.bind('drop', this, this.onDrop);

		// set auto advancing of slide show
		var duration = window.BumpTopNative.getSlideShowDuration() + "s";
		$(document).everyTime(duration, this.advanceSlideShow);
	};
	
	// load existing options from cache, bind checkbox actions
	this.initOptions = function () {
		// button to open settings
		var optionsList = $('.options_overlay .options_list');
		$('.options_overlay .open_button').bind("touchclick", function() {
			var optionsOverlay = $('.options_overlay');

			if (optionsList.css("display") == "none") {
				optionsList.css("display", "block");
			} else {
				optionsList.css("display", "none");
			}
		});

		// init checkboxes to cached preferences
		var statusCheckbox = $('.options_overlay [name=post_type_status]');
		var statusLabel = $('.options_overlay #status_option');
		var photosCheckbox = $('.options_overlay [name=post_type_photos]');
		var photosLabel = $('.options_overlay #photos_option');
		var slideshowCheckbox = $('.options_overlay [name=slideshow]');
		var slideshowLabel = $('.options_overlay #slideshow_option');

		// if values for checkboxes are not saved, save them as all enabled (default)
		// if values are saved, set the checkboxes to those values
		var postTypes = window.BumpTopNative.getPersistentStoreValue("newsfeed-posttypes");
		if (postTypes == "") {
			window.BumpTopNative.setPersistentStoreValue("newsfeed-posttypes", STATUS_FILTER + "," + LINKS_FILTER + "," + PHOTO_FILTER);
		} else {
			if (postTypes.indexOf(STATUS_FILTER) == -1) {
				statusCheckbox.attr("checked", false);
			}
			if (postTypes.indexOf(PHOTO_FILTER) == -1) {
				photosCheckbox.attr("checked", false);
			}
		}
		var slideshowOn = window.BumpTopNative.getPersistentStoreValue("newsfeed-slideshow");
		if (slideshowOn === "false") {
			slideshowCheckbox.attr("checked", false);
		}

		// change feed filter when clicking status checkbox (also check/uncheck box because it doesn't work by default)
		statusLabel.bind("touchclick", function(requestFeedUpdate) {
			if (statusCheckbox.attr("disabled") === false) {
				return function() {
					var postTypes = "";
					if (statusCheckbox.attr("checked") === false) {
						postTypes += "," + STATUS_FILTER + "," + LINKS_FILTER;
						statusCheckbox.attr("checked", "checked");

						// enable photos checkbox in case it was disabled
						photosCheckbox.attr("disabled", false);
					} else {
						statusCheckbox.attr("checked", false);

						// disable photos checkbox so status and photos cant both be disabled
						photosCheckbox.attr("disabled", true);
					}
					if (photosCheckbox.attr("checked") === true) {
						postTypes += "," + PHOTO_FILTER;
					}
					// save updated post types
					window.BumpTopNative.setPersistentStoreValue("newsfeed-posttypes", postTypes.substr(1));

					// clear items in feed and reload (using new post tpes)
					window.BumpTopNative.setPersistentStoreValue("newsfeed-data", "");
					window.BumpTopNative.setPersistentStoreValue("newsfeed-last-update", "");
					window.BumpTopNative.setPersistentStoreValue("newsfeed-latest-postid", "");
					$('.item:not(.item_template)').remove();
					$('.feed').width(emptyFeedWidth);
					earliestPhotoTime = -1;
					requestFeedUpdate();

					// show more button's spinner while loading
					$('#btfb_content .feed .load_more .more_throbber').css('display', 'inline');
					$('#btfb_content .feed .load_more span').css('display', 'none');
					$('#btfb_content .feed .load_more').css("position", "absolute");
				};
			}
		} (this.requestFeedUpdate));

		// change feed filter when clicking photos checkbox (also check/uncheck box because it doesn't work by default)
		photosLabel.bind("touchclick", function(requestFeedUpdate) {
			if (photosCheckbox.attr("disabled") === false) {
				return function() {
					var postTypes = "";
					if (statusCheckbox.attr("checked") === true) {
						postTypes += "," + STATUS_FILTER + "," + LINKS_FILTER;
					}
					if (photosCheckbox.attr("checked") === false) {
						postTypes += "," + PHOTO_FILTER;
						photosCheckbox.attr("checked", "checked");

						// enable status checkbox in case it was disabled
						statusCheckbox.attr("disabled", false);
					} else {
						photosCheckbox.attr("checked", false);

						// disable status checkbox so status and photos cant both be disabled
						statusCheckbox.attr("disabled", true);
					}
					// save updated post types
					window.BumpTopNative.setPersistentStoreValue("newsfeed-posttypes", postTypes.substr(1));

					// clear items in feed and reload (using new post types)
					window.BumpTopNative.setPersistentStoreValue("newsfeed-data", "");
					window.BumpTopNative.setPersistentStoreValue("newsfeed-last-update", "");
					window.BumpTopNative.setPersistentStoreValue("newsfeed-latest-postid", "");
					$('.item:not(.item_template)').remove();
					$('.feed').width(emptyFeedWidth);
					earliestPhotoTime = -1;
					requestFeedUpdate();
					
					// show more button's spinner while loading
					$('#btfb_content .feed .load_more .more_throbber').css('display', 'inline');
					$('#btfb_content .feed .load_more span').css('display', 'none');
					$('#btfb_content .feed .load_more').css("position", "absolute");
				};
			}
		} (this.requestFeedUpdate));

		// turn slideshow on/off when clicking checkbox (also check/uncheck box because it doesn't work by default)
		slideshowLabel.bind("touchclick", function() {
			var newSlideshowOn = "";
			if (slideshowCheckbox.attr("checked") === false) {
				newSlideshowOn = "true";
				slideshowCheckbox.attr("checked", "checked");
			} else {
				newSlideshowOn = "false";
				slideshowCheckbox.attr("checked", false);
			}

			// save updated slideshow preference
			window.BumpTopNative.setPersistentStoreValue("newsfeed-slideshow", newSlideshowOn);
		});
	};

	// Tries and load the cached feed
	this.loadCachedFeed = function() {
		var jsonData = window.BumpTopNative.getPersistentStoreValue('newsfeed-data');
		if (jsonData.length > 0) {
			this.onRequestFeedUpdateSuccessful(jsonData, true, true);
			return true;
		}
		return false;
	};

	// Sends a request for the high resolution images for the specified pids
	this.requestHighResolutionPhotos = function(pidsList) {
		window.BumpTopNative.requestHighResolutionPhotos(pidsList, "window.currentWidgetPage.onRequestHighResolutionPhotosSuccessful");
	};

	// Asynchronous result handler for a successful loading of images that have high-resolution counterparts
	this.onRequestHighResolutionPhotosSuccessful = function(jsonData, successful, isCachedJsonData) {
		var feed = JSON.parse(jsonData);

		// pairs of album ids and their corresponding user ids, used to get album info
		var aidUids = [];

		var defaultHeight = this.stripPx($('.item_template').find('.image_src.auto').css('maxHeight'));
		var defaultWidth = this.stripPx($('.item_template').find('.image_src.auto').css('maxWidth'));
		var borderWidth = 5;
		var borderStr = "px solid #ccc";

		// get a map of the pid to items
		var existingItems = $(".item.photo").each(function() {
			var itemData = $(this).data("fb_data");
			var pid = itemData.photoId;
			if (pid && pid.length > 0) {
				// save current item
				pidToItemsMap[pid] = $(this);
			}
		});

		// find all the items with the specified pids and update their urls to point to the high-resolution images
		for (var i = 0; i < feed.length; ++i) {
			if (pidToItemsMap.hasOwnProperty(feed[i].pid) && !pidToItemsMap[feed[i].pid].hasClass("hires")) {
				// store album id/user id pair to get this containing album's name and size
				aidUids.push(feed[i].aid + "," + feed[i].owner);
				pidToItemsMap[feed[i].pid].addClass(feed[i].aid);
				
				var autoImg = pidToItemsMap[feed[i].pid].find('.image_src.auto');
				var manualImg = pidToItemsMap[feed[i].pid].find('.image_src.manual');
				
				var autoHeight = feed[i].height;
				var autoWidth = feed[i].width;

				// set visible img tag to hi-res source
				autoImg.attr("src", feed[i].url);

				// if image height is smaller than the max, center it vertically and add left/right borders
				if (autoHeight < defaultHeight) {
					autoImg.css("marginTop", (defaultHeight - autoHeight) / 2 - borderWidth);
					autoImg.css("borderTop", borderWidth + borderStr);
					autoImg.css("borderBottom", borderWidth + borderStr);
				}
				// if image width is smaller than the max, center is horizontally and add top/bottom borders
				if (autoWidth < defaultWidth) {
					autoImg.css("marginLeft", (defaultWidth - autoWidth) / 2 - borderWidth);
					autoImg.css("borderLeft", borderWidth + borderStr);
					autoImg.css("borderRight", borderWidth + borderStr);
				}
				
				// if image height is greater than the max, shrink it amd center it horizontally
				if (autoHeight > defaultHeight) {
					var scaledWidth = (defaultHeight / autoHeight) * autoWidth;
					autoImg.css("marginLeft", (defaultWidth - scaledWidth) / 2);
				}
				// if image width is greater than the max, shrink it amd center it vertically
				if (autoWidth > defaultWidth) {
					var scaledHeight = (defaultWidth / autoWidth) * autoHeight;
					autoImg.css("marginTop", (defaultHeight - scaledHeight) / 2);
				}

				// remove invisible img tag from dom, won't be needed since hi-res image was returned
				manualImg.remove();

				// set flag to say hi-res image was returned
				pidToItemsMap[feed[i].pid].addClass("hires");
			}
		}

		// where hi-res photos were not returned, use fb's naming convention to get hi-res version manually
		existingItems.each(function() {
			var itemData = $(this).data("fb_data");
			if (!pidToItemsMap[itemData.photoId].hasClass("hires")) {
				// get lo-res source from visible img tag
				var src = pidToItemsMap[itemData.photoId].find('.image_src.auto').attr("src");

				// replace '_s' with '_n' at end of photo filename (fb convention for hi-res photos)
				var new_src = src.replace("_s.jpg", "_n.jpg");

				// set new source to invisible img tag
				pidToItemsMap[itemData.photoId].find('.image_src.manual').attr("src", new_src);

				// if invisible image loads, the hi-res photo exists, so replace lo-res version with hi-res version
				pidToItemsMap[itemData.photoId].find('.image_src.manual').load(function(currentPid) {
					return function() {
						if (!pidToItemsMap[currentPid].hasClass("hires")) {
							var manualImg = pidToItemsMap[currentPid].find('.image_src.manual');
							var manualHeight = manualImg.height();
							var manualWidth = manualImg.width();

							manualImg.css("display", "block");

							// if image height is smaller than the max, center it vertically and add left/right borders
							if (manualHeight < defaultHeight) {
								manualImg.css("marginTop", (defaultHeight - manualHeight) / 2 - borderWidth);
								manualImg.css("borderTop", borderWidth + borderStr);
								manualImg.css("borderBottom", borderWidth + borderStr);
							}
							// if image width is smaller than the max, center is horizontally and add top/bottom borders
							if (manualWidth < defaultWidth) {
								manualImg.css("marginLeft", (defaultWidth - manualWidth) / 2 - borderWidth);
								manualImg.css("borderLeft", borderWidth + borderStr);
								manualImg.css("borderRight", borderWidth + borderStr);
							}

							// if image height is greater than the max, shrink it amd center it horizontally
							if (manualHeight > defaultHeight) {
								var scaledWidth = (defaultHeight / manualHeight) * manualWidth;
								manualImg.css("marginLeft", (defaultWidth - scaledWidth) / 2);
							}
							// if image width is greater than the max, shrink it amd center it vertically
							if (manualWidth > defaultWidth) {
								var scaledHeight = (defaultWidth / manualWidth) * manualHeight;
								manualImg.css("marginTop", (defaultHeight - scaledHeight) / 2);
							}

							pidToItemsMap[currentPid].find('.image_src.auto').remove();
							pidToItemsMap[currentPid].addClass("hires");
						}
					};
				} (itemData.photoId));
			}
		});
		
		// make request for album names and sizes
		if (aidUids.length > 0) {
			this.requestAlbumsInfo(aidUids);
		}
	};
	
	// Sends a request for the facebook photo album info for specified album id/user id pairs
	this.requestAlbumsInfo = function(aidUids) {
		window.BumpTopNative.requestAlbumsInfo(aidUids, "window.currentWidgetPage.onRequestAlbumsInfoSuccessful");
	};

	// Asynchronous result handler for a successful loading of photo album info
	this.onRequestAlbumsInfoSuccessful = function(jsonData, successful, isCachedJsonData) {
		var feed = JSON.parse(jsonData);

		for (var i = 0; i < feed.length; ++i) {
			var photoEl = $('.item.photo.' + feed[i].aid);
			if (photoEl.size > 0) {
				if (feed[index].aid && feed[index].title && feed[index].numPhotos) {
					photoEl.find('.item_album_name').text(feed[index].title);
					photoEl.find('.item_album_size').text("(" + feed[index].numPhotos + " Photos)");
					photoEl.find('.item_description').css("marginTop", 0);
					photoEl.find('.item_description').css("fontSize", "18px");
					photoEl.find('.photo_overlay').css("display", "block");
				}
			}
		}
	};

	// Sends a request for the facebook page info for specified pageIds
	this.requestPageInfo = function(pageIdsList) {
		window.BumpTopNative.requestPageInfo(pageIdsList, "window.currentWidgetPage.onRequestPageInfoSuccessful");
	};

	// Asynchronous result handler for a successful loading of page info
	this.onRequestPageInfoSuccessful = function(jsonData, successful, isCachedJsonData) {
		var feed = JSON.parse(jsonData);
		
		// get a map of the pageId to items
		var pageIdToItemsMap = {};
		for (var i = 0; i < feed.length; ++i) {
			pageIdToItemsMap[feed[i].pageId] = feed[i];
		}
		
		// find all the items with the specified pageIds and update their name and profile link
		var existingItems = $(".item:not(.item_template)").each(function() {			
			var itemData = $(this).data("fb_data");
			var pageId = itemData.sourceId;
			if (pageIdToItemsMap[pageId]) {
				$(this).find('.author').each(function() {		
					var byText = TRF('by %1', [pageIdToItemsMap[pageId].pageName]);
					$(this).text(byText)
					.bind("touchclick", function() {
						var url = pageIdToItemsMap[pageId].pageUrl;
						return function () {
							window.currentWidgetPage.launchUrl(url);
						};
					}());
				});
			}			
		});
	};

	// Sends a request for the feed to be updated
	this.requestFeedUpdate = function() {
	    if (!loadingMore && !window.BumpTopNative.isUpdatingDisabled()) {
	        window.BumpTopNative.requestNewsFeed("window.currentWidgetPage.onRequestFeedUpdateSuccessful");
	    }
	};
	
	// Sends a request for older photos to append to the feed
	this.requestFeedMore = function() {
	    if (!loadingMore) {
		    loadingMore = true;
		    window.BumpTopNative.requestNewsFeed("window.currentWidgetPage.onRequestFeedUpdateSuccessful", earliestPhotoTime);
		    // replace "more" text with spinner
			$('#btfb_content .feed .load_more .more_throbber').css('display', 'inline');
		    $('#btfb_content .feed .load_more span').css('display', 'none');
		    scrollPos = $('#btfb_content').scrollLeft();
		}
	};

	// returns whether this date is today
	this.isToday = function(date) {
		var today = new Date();
		return (today.getYear() == date.getYear() && today.getMonth() == date.getMonth() && today.getDate() == date.getDate());
	};

	// Validate the given feed.
	// Ensure the given array of feed items is in chronological, 
	// newest-first order by creation time. Also check that there are no
	// duplicate ids.
	// On failure, print error messages and throw an exception.
	function validateFeed(feedItems) {
		var imageUrls = {};
		var previousCtime = Number.MAX_VALUE;
		var errorMsg;
		for (var i = 0; i < feedItems.length; i++) {
			if (feedItems[i].creationDate > previousCtime) {
				errorMsg = "ERROR in validateFeed: Item " + i + " out of order";
				console.log(errorMsg);
//				throw errorMsg;
				return;
			}
			previousCtime = feedItems[i].creationDate;

			var url = feedItems[i].photoId;
			if (url && imageUrls.hasOwnProperty(url)) {
				errorMsg = "ERROR in validateFeed: duplicate image " + url;
//				throw errorMsg;
				console.log(errorMsg);
				return;
			}
			imageUrls[url] = feedItems[i];
		}
	}
	
	// Asynchronous result handler for a successful feed request
	this.onRequestFeedUpdateSuccessful = function(jsonData, successful, isCachedJsonData) {
		// The limit to how many new items to add
		var MAX_FEED_ITEMS = 10;

		// If undefined, assume false.
		var isCached = isCachedJsonData || false;

		var feed = JSON.parse(jsonData);
		if (!$.isArray(feed)) {
			console.log(jsonData);
			throw "ERROR: JSON data is not an array";
		}

		// commonly used elements
		var feedEl = $('.feed');
		var itemTemplate = $('.item_template');
		var more_button = $('.load_more');

		// clear the loading item
		if (feed.length > 1) {
			$('.loading_label').remove();
			more_button.css('display', 'inline');
			more_button.bind("touchclick", this.requestFeedMore);
		} else {
			if (!loadingMore) {
				$('.loading_label div').text(TR('There are no photos in your feed to display. Upload photos to see them here!'));
				return;
			} else {
				// if there are no new items after clicking more, hide the more button
				feedEl.width(feedEl.width() - more_button.width() - this.stripPx(more_button.css('marginLeft')));
				more_button.remove();
				loadingMore = false;
				return;
			}
		}

		// Sort the feed (newest first)
		function sortJsonData(a, b) {
			return a.creationDate > b.creationDate;
		}
		feed.sort(sortJsonData);
		validateFeed(feed);

		//whether the existing items are too old and should be removed
		var trimExisting = true;

		var existingItems = $(".item:not(.item_template)");

		// update the time strings for existing items
		existingItems.each(function (getTimeSinceString) {
			return function () {
				var timeLabel = getTimeSinceString($(this).data("fb_data").creationDate * 1000);
				if (timeLabel != "") {
					$(this).find('.item_time').text(timeLabel);
				}
			};
		} (this.getTimeSinceString));

		var latestItem = $(".item:not(.item_template):not(.update)").eq(0).data("fb_data");
		if (latestItem && existingItems.length > 0 && !loadingMore) {
			// Find all the new items in the list
			var lastItemIndex = 0;
			var limit = Math.min(MAX_FEED_ITEMS, feed.length);
			for (; lastItemIndex < limit; ++lastItemIndex) {
				if (feed[lastItemIndex].creationDate <= latestItem.creationDate) {
					trimExisting = false;
					break;
				}
			}

			feed = feed.slice(0, lastItemIndex); // Limit size of feed
		} else {
			trimExisting = false;
			feed = feed.slice(0, MAX_FEED_ITEMS);
		}

		//remove existing entries (and the space allocated for them in the feed) if they are too old
		if (trimExisting) {
			existingItems.remove();
			feedEl.width(emptyFeedWidth);
			console.log("trimmed");
		}

		// Add any new feed entries.
		// TODO: adapt so that we only get the limit up to the last entry for the next update

		var latestPostId = 0;
		var pids = [];
		var postIds = [];
		var lastItem = $('.item:not(.item_template):not(.update):last');
		var postType = "";

		// expands status balloon to fit text and centers text
		function setBalloonDims(balloonEl, textEl) {
			// expand status bubble to fit text, and vertically center text
			var balloonHeight = balloonEl.height();
			var statusHeight = textEl.outerHeight();

			var heightOffset = (balloonHeight - statusHeight) / 2;
			if (statusHeight < balloonHeight) {
				// vertically center text if bubble is big enough
				textEl.css("marginTop", heightOffset + "px");
			} else {
				// enlarge bubble to fit text
				balloonEl.height(statusHeight);
			}
		}
		
		for (var i = feed.length - 1; i >= 0; --i) {
			// commonly used item elements
			var template;
			var itemMiddleStatus;
			var itemMiddlePhoto;
			var statusTextField;
			var statusText;
			var statusLink;
			var itemProfilepic;
			
			// dummy post for status updating item, just use it to apply user's name and profile pic
			if (feed[i].postId == "dummy_status_update") {
				// never make more than one dummy post
				if ($('.item.update').length > 0) {
					continue;
				}

				// init the item (remove photo div, show textarea, show share button, etc)
				template = itemTemplate.eq(0).clone().insertBefore('.item:first');
				template.addClass("update");
				template.removeClass("item_template");
				
				// commonly used elements
				var statusUpdateButton = template.find('.status_update_button');
				statusTextField = template.find('.status_text_field');

				// remove unused elements, show textarea and fill with default text
				template.find('.item_middle_status').css("cursor", "default");
				template.find('.item_middle_photo').remove();
				template.find('.item_time_cl_container').remove();
				statusUpdateButton.css("display", "block");
				statusTextField.css("display", "block");
				statusTextField.text(TR("What's on your mind?"));

				// erase default text and change colour to black when clicking on textarea
				statusTextField.bind("touchclick", function(statusTextField) {
					return function() {
						if (statusTextField.val() == statusTextField.text()) {
							statusTextField.val("");
							statusTextField.css("color", "#000");
						}
					};
				} (statusTextField));
				// put back default text and change colour to gray when textarea loses focus and is empty
				statusTextField.bind("blur", function(statusTextField) {
					return function() {
						if (statusTextField.val() == "") {
							statusTextField.val(statusTextField.text());
							statusTextField.css("color", "#ddd");
						}
					};
				} (statusTextField));
				// if text is not empty or default, set status on facebook when clicking share button
				statusUpdateButton.bind("touchclick", function(statusTextField) {
					return function() {
						// if the text (val) is not the default (text ie. "What's on your mind?") or blank
						if (statusTextField.val() !== statusTextField.text() && statusTextField.val() !== "") {
							window.BumpTopNative.setStatus(statusTextField.val());
							alert("Status updated");
							statusTextField.css("color", "#ddd");
							statusTextField.val(statusTextField.text());
						}
					};
				} (statusTextField));
			} else {
				//store earliest photo's creation time (to know where to start when loading more)
				if (earliestPhotoTime == -1 || feed[i].creationDate < earliestPhotoTime) {
					earliestPhotoTime = feed[i].creationDate;
				}

				// save the values that we need for caching
				latestPostId = feed[i].postId;
				
				// save the pid and mark the post as a photo or status post
				if (feed[i].photoId && feed[i].photoId.length > 0) {
					pids.push(feed[i].photoId);
					postType = PHOTO_POST;
				} else {
					postType = STATUS_POST;
				}
				postIds.push(feed[i].postId);

				if (!loadingMore) {
					template = itemTemplate.eq(0).clone().insertBefore('.item:not(.update):first');
				} else {
					template = itemTemplate.eq(0).clone().insertAfter(lastItem);
				}
				
				itemMiddleStatus = template.find('.item_middle_status');
				itemMiddlePhoto = template.find('.item_middle_photo');
				statusText = template.find('.status_text');
				statusLink = template.find('.status_link');

				// post-type-specific customization
				if (postType == PHOTO_POST) {
					// get lo-res image loading asap
					template.find('.image_src.auto').attr("src", feed[i].imageSrc);
					
					// mark item as photo post and remove status-related elements
					template.addClass(PHOTO_POST);
					itemMiddleStatus.remove();
					template.find('.status_pointer').remove();
					
					// link photo to photo on facebook
					itemMiddlePhoto.bind('touchclick', function() {
						var url = feed[i].permalink;
						return function() {
							window.currentWidgetPage.launchUrl(url);
							return false;
						};
					} ());
					
					// apply photo caption
					if (feed[i].imageAlt != "") {
						// if it exists, apply and make visible
						template.find('.item_description').text(feed[i].imageAlt);
						template.find('.photo_overlay').css("display", "block");
					} else {
						// if it doesn't exist, give div a height so album text is large and centered when its applied
						template.find('.item_description').height(10);
						template.find('.item_album_name').css("fontSize", "22px");
						template.find('.item_album_size').css("fontSize", "22px");
					}
				} else if (postType == STATUS_POST) {
					// mark item as status post and remove photo-related elements
					template.addClass(STATUS_POST);
					itemMiddlePhoto.remove();
					
					// append link url to message
					feed[i].message = feed[i].message + " " + feed[i].extlink;

					// find links and create clickable elements for them
					if (this.createLinksInStringEl(statusText, statusLink, feed[i].message, true)) {
						// move link below spacer element and display them both
						var linkTitle = template.find('.status_text_container > .status_link:last').remove();
						linkTitle.insertAfter(template.find('.status_spacer'));
						linkTitle.css("display", "inline");
						template.find('.status_spacer').css("display", "block");

						// replace link url with title if possible
						if (feed[i].imageAlt && feed[i].imageAlt !== "www.youtube.com") {
							template.find('.status_text_container > .status_link:last').text(feed[i].imageAlt);
						}

						// add thumbnail if included in post (eg. for links)
						if (feed[i].imageSrc) {
							var statusThumb = template.find('.status_thumb').clone().insertAfter(template.find('.status_spacer'));
							statusThumb.attr("src", feed[i].imageSrc);
							statusThumb.css("display", "inline");

							statusThumb.bind('touchclick', function() {
								var url = feed[i].extlink;
								return function() {
									window.currentWidgetPage.launchUrl(url);
									return false;
								};
							} ());
						}

						// add link domain and description
						if (feed[i].domain) {
							var statusLinkDomain = template.find('.status_link_domain').clone().appendTo(template.find('.status_text_container'));
							statusLinkDomain.text(feed[i].domain);
							statusLinkDomain.css("display", "block");
						}
						if (feed[i].description) {
							var statusLinkDesc = template.find('.status_link_desc').clone().appendTo(template.find('.status_text_container'));
							statusLinkDesc.text(feed[i].description);
							statusLinkDesc.css("display", "inline");
						}
						
						// wrap the link content in a div so anywhere around the elements can be clicked to launch the link
						var statusLinkLabel = "<div class='status_link_container'/>";
						template.find('.status_text_container > :not(div)').wrapAll(statusLinkLabel);

						// make sure link container is the last element in the status container
						template.find('.status_link_container').remove().insertAfter(template.find('.status_spacer'));

						template.find('.status_link_container').bind('touchclick', function() {
							var url = feed[i].extlink;
							return function() {
								window.currentWidgetPage.launchUrl(url);
								return false;
							};
						} ());
					}					
					
					// automatically decrease font size for longer messages (decrease by increment for every charGroupSize number of characters)
					var charGroupSize = 20;
					var maxSize = 54;
					var increment = 2;
					var numCharGroups = feed[i].message.length / charGroupSize;
					var fontSize = maxSize - (increment * numCharGroups);
					statusText.css("fontSize", fontSize + "px");
					template.find('.status_text_container > .status_link').css("fontSize", fontSize + "px");
					
					// link titles always fixed font size
					fontSize = 22;
					template.find('.status_link_container > .status_link').css("fontSize", fontSize + "px");
					
					// link status bubble to post on facebook
					itemMiddleStatus.bind('touchclick', function() {
						var url = feed[i].permalink;
						return function() {
							window.currentWidgetPage.launchUrl(url);
							return false;
						};
					} ());
				}
				template.removeClass('item_template');

				// get time strings (x minutes ago)
				var timeSinceLabel = this.getTimeSinceString(feed[i].creationDate * 1000);
				template.find('.item_time').text(timeSinceLabel);

				// apply numbers of comments and likes (and remove elements if 0)
				if (feed[i].numComments > 0 || feed[i].numComments > 0) {
					if (feed[i].numComments > 0) {
						template.find('.comments_num').text(feed[i].numComments);
					} else {
						template.find('.comments').remove();
					}
					if (feed[i].numLikes > 0) {
						template.find('.likes_num').text(feed[i].numLikes);
					} else {
						template.find('.likes').remove();
						template.find('.comments_num').css("marginRight", 0);
					}
				} else {
					template.find('.item_cl').remove();
				}

				// if there is a thumbnail in the status, can't center until it loads (when height is known); otherwise center immediately
				template.find('.status_link_container > .status_thumb').load(function(template) {
					return function() {
						setBalloonDims(template.find('.item_middle_status'), template.find('.status_text_container'));
					};
				} (template));
				setBalloonDims(template.find('.item_middle_status'), template.find('.status_text_container'));
			}
			// following code applies to all post types (including dummy post for updating status)
			
			// apply owner's name and link it to their profile
			template.find('.item_profile').text(feed[i].ownerName)
				.bind("touchclick", function() {
					var url = feed[i].ownerProfileLink;
					return function() {
						window.currentWidgetPage.launchUrl(url);
					};
				} ());

			itemProfilepic = template.find('.item_profilepic');
			// apply owner's profile picture and link it to their profile
			itemProfilepic.attr("src", feed[i].ownerPicSquare)
				.bind("touchclick", function() {
					var url = feed[i].ownerProfileLink;
					return function() {
						window.currentWidgetPage.launchUrl(url);
					};
				}());
			// when the profile pic loads (and its dimensions are known)
			itemProfilepic.load(function(template) {
				return function() {
					var itemProfilepicContainer = template.find('.item_profilepic_container');
					var itemLeft = template.find('.item_left');
					itemProfilepic = template.find('.item_profilepic');
					itemMiddleStatus = template.find('.item_middle_status');

					itemProfilepicContainer.height(itemProfilepic.height());
					
					// calculate margin needed to align pic to bottom of feed
					var picMargin = itemTemplate.height() - itemProfilepic.height() - 3/*shadow width*/;
					itemLeft.css("marginTop", picMargin);

					// calculate the gap between the top of the profile picture and the bottom of the status bubble
					var picStatusGap = picMargin - itemMiddleStatus.outerHeight();
					if (picStatusGap > 0) {
						// if there is a gap between the profile pic and the status bubble, move the bubble down
						// include a buffer so that the bubble does not go:
						// a) above the top of the feed
						// b) below the 2/3 mark of the pointer overlapping with the profile pic
						var buffer = Math.min(picStatusGap, template.find('.status_pointer').height() * (2 / 3));
						itemMiddleStatus.animate({
							marginTop: picStatusGap - buffer
						}, 500);
						template.find('.status_text_field').height(itemMiddleStatus.height() - 40);
					} else if (picStatusGap < 0) {
						// if the profile pic is overlapping with the status bubble, push the profile pic down
						// within its container (cropping from the bottom)
						itemLeft.css("marginTop", picMargin + (-1 * picStatusGap));
						itemProfilepicContainer.height(itemProfilepicContainer.height() + picStatusGap);
					}
					
					// fade in item when all elements have been sized and laid out
					template.animate({
						opacity: 1
					}, 1000);
				};
			} (template));
			
			// calculate margin needed to align bottom (name, time, comments/likes) with bottom of feed
			var nameMarginTop = itemTemplate.height() - template.find('.item_right').height();
			template.find('.item_right').css("marginTop", nameMarginTop);

			// Store the original item in a custom jQuery data field
			template.data("fb_data", feed[i]);

			// set the width of the feed
			feedEl.width(feedEl.width() + template.width() + this.stripPx(template.css('marginLeft')));
			
			// in case the profile picture doesn't load, show the item
			template.oneTime("5000ms", function(template) {
				return function () {
					template.animate({
						opacity: 1
					}, 500);
				};
			} (template));
		}

		// request higher resolution photos where possible
		this.requestHighResolutionPhotos(pids);

		// hide more button spinner always
		$('#btfb_content .feed .load_more span').css('display', 'inline');
		$('#btfb_content .feed .load_more .more_throbber').css('display', 'none');
		$('#btfb_content .feed .load_more').css("position", "static");

		// scroll the feed and return
		if (loadingMore) {
			$('#btfb_content').scrollLeft(scrollPos);
			loadingMore = false;
			return;
		}

		// feed will have scrolled back to left end, so tell the slideshow that the current picture is the first one
		slideShowPos = 0;

		// save the persistent data
		if (!isCached) {
			if (feed.length > 0) {
				var cacheItems = [];
				var existingJsonData = window.BumpTopNative.getPersistentStoreValue('newsfeed-data');
				if (existingJsonData.length > 0) {
					cacheItems = JSON.parse(existingJsonData);
				}
				cacheItems = feed.concat(cacheItems);
				validateFeed(cacheItems);
				cacheItems = cacheItems.slice(0, MAX_FEED_ITEMS); // Limit cache size
				jsonData = JSON.stringify(cacheItems);
				window.BumpTopNative.setPersistentStoreValue('newsfeed-data', jsonData);
				window.BumpTopNative.setPersistentStoreValue('newsfeed-latest-postid', '' + latestPostId);
			}
			window.BumpTopNative.setPersistentStoreValue('newsfeed-last-update', '' + new Date().getTime());
		}
	};

	// find links (between "http://" and " ") and make them clickable
	// adds spans following initEl with link and post-link text
	// link spans are cloned using tempLinkEl
	this.createLinksInStringEl = function(initEl, tempLinkEl, initMsg, removeHttpPrefix) {
		// initial status element
		var preLinkEl = initEl;

		// initial message text
		var preLinkMsg = initMsg;
		
		if (!preLinkEl || !preLinkMsg) {
			return false;
		}

		// initial link position
		var linkPos = preLinkMsg.indexOf("http://");
		
		// if there are no links, return put the msg into the el and return
		if (linkPos == -1) {
			preLinkEl.text(preLinkMsg);
			return false;
		}

		while (linkPos != -1) {
			// text from beginning of link to end of message
			var linkToEndText = preLinkMsg.substr(linkPos);

			// position of last char of link (end on whitespace - space, newline, or tab, whichever is first)
			var spaceIndex = linkToEndText.indexOf(" ");
			var newlineIndex = linkToEndText.indexOf("\n");
			var tabIndex = linkToEndText.indexOf("\t");
			
			var linkEndPos;
			if (spaceIndex == -1 && newlineIndex == -1 && tabIndex == -1) {
				linkEndPos = -1;
			} else {
				if (spaceIndex == -1) {
					spaceIndex = linkToEndText.length;
				}
				if (newlineIndex == -1) {
					newlineIndex = linkToEndText.length;
				}
				if (tabIndex == -1) {
					tabIndex = linkToEndText.length;
				}
				
				linkEndPos = Math.min(spaceIndex, Math.min(newlineIndex, tabIndex));
			}

			// text of link only
			var linkText = "";

			// text from end of link to end of message
			var afterLinkText = "";

			// separate and assign the string parts
			if (linkEndPos != -1) {
				// if there was a space, cut it there
				linkText = linkToEndText.substr(0, linkEndPos);
				afterLinkText = linkToEndText.substr(linkEndPos);
			} else {
				// if there was no space, go all the way to end of message
				linkText = linkToEndText;
			}
			
			preLinkMsg = preLinkMsg.substr(0, linkPos);
			preLinkEl.text(preLinkMsg);

			// create element for link, insert after original text element, link it to actual target
			var linkEl = tempLinkEl.clone().insertAfter(preLinkEl);
			//linkEl.css("display", "inline");
			linkEl.bind('touchclick', function(linkText) {
				var url = linkText;
				return function() {
					window.currentWidgetPage.launchUrl(url);
					return false;
				};
			} (linkText));

			// remove the "http://"
			if (removeHttpPrefix) {
				linkText = linkText.substr(7);
			}

			// two methods of displaying long links:
			// 1) ellipsis truncate at a fixed number of characters
			// 2) insert spaces at fixed intervals so the text wraps
			var linkMaxLength = 20;
			// ellipsis truncate long links since they dont wrap properly
			if (linkText.length > linkMaxLength) {
				linkText = linkText.substr(0, linkMaxLength);
				linkText = linkText.concat("...");
			}
			// add spaces every fixed number of characters so the text can wrap
			/*for (var i = linkMaxLength; i < linkText.length; i += linkMaxLength) {
				linkText = linkText.substr(0, i) + " " + linkText.substr(i);
			}*/
			linkEl.text(linkText);

			// create element for text after link, insert after link
			var afterLinkEl = preLinkEl.clone().insertAfter(linkEl);
			afterLinkEl.text(afterLinkText);

			// set up next iteration
			preLinkEl = afterLinkEl;
			preLinkMsg = afterLinkText;
			linkPos = afterLinkText.indexOf("http://");
		}
		
		return true;
	};
	
	// Helper to hide all the dates that are duplicates, ie. 
	// November, November, September -> hide the second November
	this.hideDuplicateMonths = function() {
		var items = $('.item');
		var prevItemMonth = '';
		for (var i = 0; i < items.length; ++i) {
			var itemMonth = $(items[i]).find('.label');
			var itemMonthText = itemMonth.text();
			if (itemMonthText.length > 0) {
				if (prevItemMonth == itemMonthText) {
					itemMonth.html('&nbsp;');
				}
				prevItemMonth = itemMonthText;
			}
		}
	};

	// Helper to get the month string from a date
	// WHY-TF does javascript date not have a date format!?!
	this.getMonthAsString = function(date) {
		return this.monthNames[date.getMonth()];
	};
	
	// helper to get the time since (x minutes ago) string based on creation time in ms
	this.getTimeSinceString = function(when) {
		// time difference in ms
		var timeSince = new Date().getTime() - when;
		var timeSinceLabel = "";

		// divide by 60 until number is less than 1 (or two iterations have been done (minutes, hours))
		var steps = 0;
		timeSince = timeSince / (1000 * 60);
		while (timeSince > 1 && steps < 3) {
			timeSince /= 60;
			steps++;
		}
		
		// add numeral to string and flag if its 1 (so "1 minutes" doesn't happen)
		var plural = true;
		timeSinceLabel = Math.round(timeSince * 60);
		if (timeSinceLabel === 1) {
			plural = false;
		}
		
		// append unit name depending on steps and numeral, if too old (yesterday+) then use date
		if (steps === 0) {
			timeSinceLabel += " seconds ago";
		} else if (steps === 1) {
			timeSinceLabel += " minutes ago";
		} else if (steps === 2 && timeSinceLabel < 24) {
			timeSinceLabel += " hours ago";
		} else if (steps === 2 && timeSinceLabel < 48) {
			timeSinceLabel = "Yesterday";
		} else {
			var entryDate = new Date();
			entryDate.setTime(when);
			if (this.getMonthAsString) {
				timeSinceLabel = this.getMonthAsString(entryDate) + ' ' + entryDate.getDate();
			} else {
				timeSinceLabel = "";
			}
		}
		
		// de-pluralize if necessary
		if (plural === false) {
			timeSinceLabel = timeSinceLabel.replace("s ago", " ago");
		}
		
		return timeSinceLabel;
	};

	// Scrolls to make the status input field visible
	this.scrollToStatus = function(event) {
		$("#btfb_content").animate(
			{ "scrollLeft": 0 },
			{ duration: 1200, easing: "easeInOutCubic" });
	};

	// Handles a mouse-over on a feed item	
	this.onItemMouseOver = function(event) {
		// un-hover other images
		var thisRef = $(this);

		// Marks the item as non-selected
		function onItemMouseOut(jqItem) {
			// mark the item as not selected and hide the author (if there is one)			
			var label = jqItem.find('.label');
			var media = jqItem.find('.media');
			if (media.hasClass('selected')) {
				label.removeClass('highlight');
				media.removeClass('selected');
			}
		}

		$('.item').each(function() {
			if ($(this) != thisRef) {			
				onItemMouseOut($(this));
			}
		});
				
		// mark the item as selected and show the author
		$(this).parent().prev('.label').toggleClass('highlight');
		$(this).parent().toggleClass('selected');
    };

    // auto-advance through pictures (only when webactor is zoomed out)
    this.advanceSlideShow = function() {
        if (!loadingMore && !window.BumpTopNative.isFocused() && !window.BumpTopNative.isAnimationDisabled()) {
            var newPos = 0;
            var pics = $(".item:not(.item_template)");
            var currentPos = $('#btfb_content').scrollLeft();
            var scrollBarAtEnd = (currentPos == $('.feed').width() - $('#btfb_content').width());
            var interference = (currentPos != slideShowLast);
            var spacing = $('.item_template').css('marginLeft').substring(0, $('.item_template').css('marginLeft').indexOf("px"));

            // if we want to slide to first pic (slideShowPos==-1), or the current pic the last pic, or the scrollbar is at the end
            if (slideShowPos == -1 || slideShowPos >= pics.length - 1 || scrollBarAtEnd) {
                slideShowPos = 0;

            } else { // general case, advance slideshow to next visible picture
                var index = 0;
                var skipFirst = true;
                pics.each(function() {
                    // if this pic is fully visible (not cut off on the left side) then this is the next pic
                    if ($(this).offset().left > 0) {
                        // if the user hasn't interfered with the slideshow, skip the first visible pic since its the current pic
                        if (interference === false && skipFirst === true) {
                            skipFirst = false;
                            return true;
                        }
                        // calculate new position using this pic's width and margin
                        newPos = currentPos + $(this).offset().left - spacing;
                        slideShowPos = index;
                        return false;
                    }
                    index++;
                });
            }
            // slide transition until scrollLeft=newPos
            $("#btfb_content").animate(
	            { "scrollLeft": newPos },
	            { duration: 1000, easing: "easeInOutCubic" }
            );
            // set current position as last position where slideshow stopped
            slideShowLast = newPos;
        }
    };
}
FacebookWidgetNewsFeedPage.prototype = new FacebookWidgetPage();
