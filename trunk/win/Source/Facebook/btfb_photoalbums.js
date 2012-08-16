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

// Represents a slideshow of photos
function FacebookWidgetPhotoSlideshow() {
	this.photos = [];
	this.currentIndex = 0;
	this.isInSlideshow = false;
	this.slideshowUpdateTimer = null;
	this.scrollWrapper = null;
	this.isFocused = true;
	
	// Constructor
	this.init = function() {
		// bind the slideshow buttons
		$('#btfb_slideshow_view').find('.back').bind('click', this, this.hideView);
		$('#btfb_slideshow_view').find('.play').bind('click', this, this.toggleSlideshow);
		
		// bind the kinetic scroll wrapper
		$("#btfb_slideshow_view .scroll_wrapper").kineticScroll();
	};

	// photos navigation
	this.nextPhoto = function() {
		var nextIndex = (this.currentIndex + 1) % this.photos.length;
		if (nextIndex != this.currentIndex) {
			this.blurImageAtIndex(this.currentIndex);
			this.focusImageAtIndex(nextIndex);
			this.currentIndex = nextIndex;
		}
	};

	this.focusImageAtIndex = function(index) {
		// the photos correspond 1:1 to elements in the slideshow
		
		// scroll to the item forward
		var image = $('.slideshow_image:eq(' + index + ')');
		var pos = image.position();
		var width = image.width();
		var containerWidth = $('#btfb_slideshow_view').width();	
		var imageOffset = pos.left - ((containerWidth - width) / 2);
		var target = $('.slideshow_images:first');
		if (index === 0) {
			imageOffset = 0;
		}
		target.css({
			'position': 'relative'
		});
		if (this.isFocused) {
			image.stop(true, true).animate({
				"opacity": 1
			}, {
				duration: 1500,
				easing: 'easeInOutCubic'
			});
			target.stop(true, true).animate({				
				"left": -imageOffset 
			}, {
				duration: 500,
				easing: 'easeInOutCubic'
			});
		}
		else {
			image.stop(true, true).css({
				"opacity": 1
			});
			target.stop(true, true).css({				
				"left": -imageOffset 
			});
		}
	};

	this.blurImageAtIndex = function(index) {
		// the photos correspond 1:1 to elements in the slideshow
		
		// scroll to the item forward
		var image = $('.slideshow_image:eq(' + index + ')');
		if (this.isFocused) {
			image.stop(true, true).animate({
				"opacity": 0.15
			}, {
				duration: 1500,
				easing: 'easeInOutCubic'
			});
		}
		else {
			image.stop(true, true).css({
				"opacity": 0.15
			});
		}
	};

	// Show/hide the slideshow view
	this.showView = function() {		
		this.isInSlideshow = true;
		$("#btfb_header .logo").fadeOut('fast');
		$('#btfb_slideshow_view').stop(true, true).show('slow');
	};

	this.hideView = function(event) {
		var instance = event.data;
		// TODO: put into animation callback
		instance.isInSlideshow = false;
		$("#btfb_header .logo").fadeIn('fast');
		$('#btfb_slideshow_view').stop(true, true).hide('show');
		
		// stop the timer if necessary
		if ($('#btfb_slideshow_view').find('.play > a').text() === TR('stop')) {
			instance.toggleSlideshow(event);
		}
	};

	this.isVisible = function() {
		return this.isInSlideshow;
	};		

	// Start/stop/update slideshow
	this.toggleSlideshow = function(event) {
		var instance = event.data;	
		if (instance.slideshowUpdateTimer === null) {
			// start the timer
			instance.slideshowUpdateTimer = setInterval(function() {
				return function() {
					instance.onUpdate();
				};
			}(), 20 * 1000 /*s/ms*/);
			// change opacity of the other images
			var i = 0;
			var images = $('.slideshow_image').each(function() {	
				if (instance.currentIndex != i) {
					instance.blurImageAtIndex(i);		
				} else { 
					instance.focusImageAtIndex(i);
				}
				i++;
			});
			// change the button text
			$('#btfb_slideshow_view').find('.play > a').text(TR('stop'));
		} else {
			// stop the timer
			clearInterval(instance.slideshowUpdateTimer);
			instance.slideshowUpdateTimer = null;
			// reset the opacity of the images
			$('.slideshow_image').each(function() {
				if (this.isFocused) {
					$(this).stop(true, true).animate({
						"opacity": 1
					}, {
						duration: 1500,
						easing: 'easeInOutCubic'
					});
				}
				else {
					$(this).stop(true, true).css({
						"opacity": 1
					});
				}
			});
			// change the button text
			$('#btfb_slideshow_view').find('.play > a').text(TR('play'));
		}
	};

	this.onUpdate = function() {
		this.nextPhoto();
	};

	// Sets the set of photos to display
	// TODO: handle if we are already in a view?
	this.setPhotos = function(rawPhotosJsonObject) {
		this.photos = rawPhotosJsonObject;

		// add any new photos
		// TODO: adapt so that we only get the limit up to the last entry for the next update
		$('.slideshow_image:not(.slideshow_image_template)').remove();
		var defaultHeight = $("#btfb_slideshow_view .slideshow_images").css("height");
		defaultHeight = defaultHeight.substring(0, defaultHeight.indexOf("px"));
		var spacing = $('.slideshow_image_template').css('marginLeft');
		spacing = spacing.substring(0, spacing.indexOf("px"));
		var feedWidth = spacing;
		for (var i = this.photos.length-1; i >= 0; --i)
		{
			var template = $('.slideshow_image_template:first').clone().insertBefore('.slideshow_image:first');

			if (this.photos[i].title.length > 0) {
				template.find('.title').text(this.photos[i].title + ' (' + this.photos[i].numPhotos + ')');
			}
			if (this.photos[i].numComments > 0) {
				template.find('.comments').text(this.photos[i].numPhotos);
			}
			var img = template.find('img');
			img.attr("src", this.photos[i].url);
			var permalink = this.photos[i].permalink;
			template.find('.permalink').each(function() {
				var link = permalink;
				return function() {
					$(this).bind('touchclick', function() {
						window.currentWidgetPage.launchUrl(link);
						return false;
					});
				};
			}());
			template.removeClass('slideshow_image_template');
			
			// if the image is smaller than the default size, resize and center
			if (this.photos[i].height < defaultHeight) {
				if (this.photos[i].height > 0) {
					img.css("height", this.photos[i].height);
					var verticalCenter = (defaultHeight - this.photos[i].height) / 2;
					template.css("marginTop", verticalCenter);
				} else {
					// set template/img size to 0 in case the image never loads
					img.css("height", 0);
					template.css("height", 0);
					template.css("width", 0);
					
					// if facebook returned 0 as the height, wait for image to load and set height
					img.load(function(template) {
						return function() {
							var imgHeight = template.find('img').css("height");
							imgHeight = imgHeight.substring(0, imgHeight.indexOf("px"));
							var verticalCenter = (defaultHeight - imgHeight) / 2;
							template.find('img').css("marginTop", verticalCenter);
						};
					} (template));
				}
			}
			template.show('slow');
		}

		// highlight the current image
		this.currentIndex = 0;
	};
	
	// Scrolls to the beginning of the slideshow
	this.scrollToFront = function(event) {
		var images = $('.slideshow_images');
		var imagesPosition = images.position();
		if (imagesPosition.left < 0) {
			images.css({
				position: 'relative'
			});
			images.stop(true, true).animate({
				"left": 0
			}, {
				duration: 1200,
				easing: 'easeInOutCubic'
			});
		}
	};
}

// Represents the facebook photo albums page
function FacebookWidgetPhotoAlbumsPage() {
	this.slideshow = null;
	this.monthNames = [TR("January"), TR("February"), TR("March"), 
		TR("April"), TR("May"), TR("June"), TR("July"), TR("August"), 
		TR("September"), TR("October"), TR("November"), TR("December")];

	// Constructor
	this.init = function() {
	    // call super constructor
	    this.initCommon();
	    // save the current page
	    window.BumpTopNative.setPersistentStoreValue('last_page', 'photoalbums');

	    // bind the kinetic scroll wrapper
	    $("#btfb_content .feed").kineticScroll();

	    // bind the slideshow
	    this.slideshow = new FacebookWidgetPhotoSlideshow();
	    this.slideshow.init();

	    // setup all the buttons
	    // $('.image').live('touchclick', this, this.startSlideshowOnAlbum);
	    /*
	    $('.description > .title').live('touchclick', this, this.launchAlbumFromTitle);
	    $('.description > .comments').live('touchclick', this, this.launchAlbumFromTitle);
	    $('.label').live('touchclick', this, this.launchAlbumFromDate);
	    */
	    $('#btfb_header').bind('click', this, this.scrollToFront);

	    // populate the photo albums
	    // try and load the cached updates, and then update as necessary
	    if (!this.loadCachedFeed()) {
	        this.requestPhotoAlbumsUpdate();
	    }

	    // setup a timer to periodically update the feed
	    $(document).everyTime(5 * 60 /*s/min*/ * 1000 /*ms/s*/, this.requestPhotoAlbumsUpdate);

	    // the proxy drag and drop is being implemented in the parent
	    $("body")
			.bind('dragenter', this, this.onDragEnter)
			.bind('dragleave', this, this.onDragLeave)
			.bind('dragover', this, this.onDragOver)
			.bind('drop', this, this.onDrop);
	};

	// Tries and load the cached feed
	this.loadCachedFeed = function() {
		var jsonData = window.BumpTopNative.getPersistentStoreValue('photoframe-data');
		if (jsonData.length > 0) {
			this.onRequestPhotoAlbumsUpdateSuccessful(jsonData, true, true);
			return true;
		}
		return false;
	};

	// Sends a request for the photo albums to be updated
	this.requestPhotoAlbumsUpdate = function() {	
		window.BumpTopNative.requestAlbums("window.currentWidgetPage.onRequestPhotoAlbumsUpdateSuccessful", true);
	};

	// Asynchronous result handler for a successful photo album request
	this.onRequestPhotoAlbumsUpdateSuccessful = function(jsonData, successful, isCachedJsonData) {
		var feed = JSON.parse(jsonData);

		// default arguments
		isCachedJsonData = typeof(isCachedJsonData) != 'undefined' ? isCachedJsonData : false;

		// get the current list of album ids
		var albumIds = [];
		$('.aid').each(function() {
			albumIds.push($(this).html());
		});

		// clear the loading item
		if (feed.length > 0) {
			$('.loading_label').remove();
		} else {
			$('.loading_label div').text(TR('Could not load your photo albums'));
		}

		// add any new feed entries
		// TODO: adapt so that we only get the limit up to the last entry for the next update
		var heightOffset = 75;
		var defaultHeight = $("#btfb_slideshow_view .slideshow_images").css("height");
		defaultHeight = this.stripPx(defaultHeight) - heightOffset;
		var uniqueAlbums = [];
		for (var i = 0; i < feed.length; ++i) {
			var entryExists = (jQuery.inArray(feed[i].aid, albumIds) > -1);
			if (!entryExists) {
				if (feed[i].numPhotos > 0) {
					// save the values that we need for caching
					uniqueAlbums.push(feed[i]);

					var albumModifiedDate = new Date();
					albumModifiedDate.setTime(feed[i].modifiedDate * 1000);
					if (feed[i].title.length === 0) {
						feed[i].title = TR("Album");
					}

					var template = $('.item_template:first').clone().insertAfter('.item:last');
					template.find('.label').text(this.getMonthAsString(albumModifiedDate) + ' ' + albumModifiedDate.getFullYear());

					template.find('.title').bind('touchclick', function() {
						var link = feed[i].permalink;
						return function() {
							window.currentWidgetPage.launchUrl(link);
							return false;
						};
					}());

					if (feed[i].numComments > 0) {
						template.find('.comments').text(feed[i].numPhotos);
					}

					// update the images
					img = template.find('.image_src');
					img.attr("src", feed[i].albumCoverUrl);

					// limit the title length
					var title = TRF('%1 (%2 Photos)', [feed[i].title, feed[i].numPhotos]);
					template.find('.title').text(title);
					template.find('.aid').text(feed[i].aid);
					template.removeClass('item_template');

					// if image size is smaller than the default, resize and center
					if (feed[i].albumCoverHeight < defaultHeight) {
						if (feed[i].albumCoverHeight > 0) {
							var verticalCenter = (defaultHeight - feed[i].albumCoverHeight) / 2;
							template.find('.image').css("marginTop", verticalCenter);
							template.find('.description').css("marginTop", verticalCenter);
						} else {
							// if facebook returned 0 as the height, wait for image to load and set height/center
							img.load(function(template) {
								return function() {
									var imgHeight = template.find('.image_src').css("height");
									imgHeight = imgHeight.substring(0, imgHeight.indexOf("px"));
									var verticalCenter = (defaultHeight - imgHeight) / 2;
									template.find('.image').css("marginTop", verticalCenter);
									template.find('.description').css("marginTop", verticalCenter);
								};
							} (template));
						}
					}
					template.show('slow');

					var instance = this;
					template.find('.image').bind('touchclick', function() {
						var thisRef = $(template);
						return function() {
							$.blockUI({
								message: $('#loading_dialog'),
								css: {
									top: '45%',
									width: '250px',
									padding: '10px',
									left: '35%',
									border: '0px solid transparent',
									color: '#FFFFFF',
									backgroundColor: '#000000',
									opacity: 0.8,
									fontWeight: 'bold'
								},
								overlayCSS: {
									opacity: 0.0,
									backgroundColor: '#FFFFFF'
								}
							});
							setTimeout(function() {
								instance.requestPhotoAlbumPhotos(thisRef);
							}, 1000);
						};
					}());
				}
			}
		}

		// trim the ui a bit to make it less cluttered
		this.hideDuplicateMonths();

		// set the width of the feed
		var spacing = this.stripPx($('.item_template').css('marginLeft'));
		var feedWidth = spacing;
		$('.item').each(function() {
			if (!$(this).hasClass('item_template')) {
				var item = this;
				// When the image has loaded determine the size and set the feed width accordingly
				$(this).find('img').load(function() {
					// limit the title length
					$(item).find('.title').css({maxWidth: $(this).width()});
					feedWidth += ($(item).width() + spacing);
					$('.feed').width(feedWidth);
				});
			}
		});

		// save the persistent data
		if (!isCachedJsonData) {
			if (uniqueAlbums.length > 0) {
				var existingJsonData = window.BumpTopNative.getPersistentStoreValue('newsfeed-data');
				if (existingJsonData.length > 0) {
					var existingArray = JSON.parse(existingJsonData);
					jsonData = JSON.stringify(existingArray.concat(uniqueAlbums));
				}
				window.BumpTopNative.setPersistentStoreValue('photoframe-data', jsonData);
			}
		}
	};

	// Helper to hide all the dates that are duplicates, ie. 
	// November, November, September -> hide the second November
	this.hideDuplicateMonths = function() {
		var items = $('.item');
		var prevItemMonth = '';
		for (var i = 0; i < items.length; ++i) {
			var itemMonth = $(items[i]).find('.label > a');
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
	
	// Sends a request for the photo albums photos
	this.requestPhotoAlbumPhotos = function(jqElement) {
		var aid = jqElement.find('.aid:first').html();
		window.BumpTopNative.requestPhotosForAlbum(aid, "window.currentWidgetPage.onRequestPhotoAlbumPhotosSuccessful");
		return false;
	};

	// Asynchronous result handler for a successful photo album photos request
	this.onRequestPhotoAlbumPhotosSuccessful = function(jsonData) {
		this.slideshow.setPhotos(JSON.parse(jsonData));
		$.unblockUI();
		this.slideshow.showView();
		var spacing = this.stripPx($('.slideshow_image_template').css('marginLeft'));
		var feedWidth = spacing;
		$('.slideshow_image').each(function () {
			if (!$(this).hasClass('slideshow_image_template')) {			
				$(this).find('img').load(function() {
					feedWidth += ($(this).width() + spacing);
					$('.slideshow_images:first').width(feedWidth);
				});
			}
		});
	};
	
	// Scrolls to make the status input field visible
	this.scrollToFront = function(event) {
		var instance = event.data;
		if (instance.slideshow.isVisible()) {
			instance.slideshow.scrollToFront();
		}
		else {
			var feed = $('.feed');
			var feedPosition = feed.position();
			if (feedPosition.left < 0) {
				feed.css({
					position: 'relative'
				});
				feed.stop(true, false).animate({
					"left": 0
				}, {
					duration: 1200,
					easing: 'easeInOutCubic'
				});
			}
		}
	};
}

FacebookWidgetPhotoAlbumsPage.prototype = new FacebookWidgetPage();
