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

// Represents the facebook upload page
function FacebookWidgetUploadPage() {
	this.supportedImageFormats = [".gif", ".jpg", ".jpeg", ".png", ".psd", ".tiff", ".tif", ".jp2", ".iff", ".bmp", ".wbmp", ".xbm"];
	this.expectedNumFilesToTransfer = 0;
	
	// Constructor
	this.init = function() {
		// call super constructor
		this.initCommon();

		// We do special translation changes for value attributes since they can't
		// be wrapped with the usual _() method.
		$('#create_album_submit').val(TR('Create Album'));

		// try and populate the view with the files in the temporary store
		var jsonData = window.BumpTopNative.getTemporaryStoreValue('upload_files_json');
		if (jsonData.length > 0) {
			var dataTransfer = JSON.parse(jsonData);
			var files = dataTransfer.files;
			var numInvalidFiles = 0;
			for (var i = 0; i < files.length; ++i) {
				if (this.isFileOfSupportedImageFormat(files[i].name)) {
					var file = $('.filepath_template:first').clone().insertBefore('.filepath:first');
					file.removeClass('filepath_template');
					file.find('img').attr('src', window.BumpTopNative.getFilePathForUrl(files[i].url));
					file.find('.name').text(files[i].name);
					file.find('.remove').bind('click', this, this.removeFile);
					file.data('path', files[i].url);
				} else {
					numInvalidFiles += 1;
				}
			}
			if (numInvalidFiles > 0) {
				var errorStr = TRF('%1 non-image file(s) skipped', [numInvalidFiles]);
				alert(errorStr);
			}
			window.BumpTopNative.removeTemporaryStoreValue('upload_files_json');
		} else {
			// only save the current page if we weren't sent here from a drop
			window.BumpTopNative.setPersistentStoreValue('last_page', 'upload');
		}

		// upload button
		$('#upload_button').bind('click', this, this.upload);
		$('#cancel_button').bind('click', this, this.cancelUpload);
		$('.clear_all').bind('click', this, this.removeAllFiles);
		$('.create_album').bind('click', this, this.createNewAlbum);
		$('.dialog_cancel').bind('click', $.unblockUI);

		// the proxy drag and drop is being implemented in the parent
		$('body')
			.bind('dragenter', this, this.onDragEnterOverride)
			.bind('dragleave', this, this.onDragLeaveOverride)
			.bind('dragover', this, this.onDragOverOverride)
			.bind('drop', this, this.onDropOverride);

		// request the albums
		window.BumpTopNative.requestAlbums('window.currentWidgetPage.onRequestAlbumsSuccessful', false);
	};

	// Handle the albums request
	this.onRequestAlbumsSuccessful = function(jsonData, successful) {
		var albums = JSON.parse(jsonData);
		$('.albums_list').empty();
		if (albums.length > 0) {
			for (var i = 0; i < albums.length; ++i) {			
				$('.albums_list').append('<option value="' + albums[i].aid + '">' + albums[i].title + ' (' + albums[i].numPhotos + ')' + '</option>');
				$('.albums_list option:last').data('link', albums[i].permalink);
			}
		} else {
			$('.albums_list').append('<option value="">' + TR('Default BumpTop Photos') + '</option>');
		}
	};

	// Launches UI to create a new album
	this.createNewAlbum = function (event) {
		$('#create_album_submit').bind('click', function() {
			var name = $('#create_album_name').val();
			var location = $('#create_album_location').val();
			var description = $('#create_album_description').val();
			if (name.length > 0) {
				// create the album
				window.BumpTopNative.requestCreateAlbum(name, location, description);					
				// reset the fields
				$('#create_album_name').val('');
				$('#create_album_location').val('');
				$('#create_album_description').val('');
				// reload the albums
				window.BumpTopNative.requestAlbums('window.currentWidgetPage.onRequestAlbumsSuccessful', false);											
				// close the dialog
				$.unblockUI();
			} else {
				alert(TR('Please enter an album name'));
			}
			
			event.preventDefault();
			event.stopPropagation();
			event.stopImmediatePropagation();
			return false;
		});
		$.blockUI({ 
			message: $('#create_album_dialog'),
			css: {
				padding: '20px',
				width: '44%',
				left: '24%',
				top: '25%',
				border: '2px solid rgb(109, 140, 203)' 
			}
		});
	};

	// Helper to determine if this is a supported image format
	this.isFileOfSupportedImageFormat = function(name) {
		for (var i = 0; i < this.supportedImageFormats.length; ++i) {
			var format = this.supportedImageFormats[i];
			var ending = name.toLowerCase().substr(-format.length);
			if (ending == format) {
				return true;
			}
		}
		return false;
	};
	
	this.areFilesOfSupportedImageFormat = function(instance, files) {
		var acceptDrop = true;
		for (var i = 0; i < files.length && acceptDrop; ++i) {
			var file = files[i];
			if (!instance.isFileOfSupportedImageFormat(file.name)) {
				acceptDrop = false;
			}
		}
		return acceptDrop;
	};

	// Removes this file from the list
	this.removeFile = function(event) {
		$(this).parent().parent().fadeOut('slow', function() {
			$(this).remove();
		});
	};

	this.removeAllFiles = function(event) {
		$('.filepath').each(function() {
			var file = $(this);
			if (!file.hasClass('filepath_template')) {
				$(this).fadeOut('slow', function() {
					$(this).remove();
				});
			}
		});
	};

	// Event handler for the "cancel" button
	this.cancelUpload = function (event) {
		var lastPage = window.BumpTopNative.getPersistentStoreValue('last_page');
		if (!lastPage || lastPage === "upload") {
			lastPage = "newsfeed";
		}
		window.location = "facebook_" + lastPage + ".html";
	};

	// Handles uploading
	this.upload = function(event) {
		var instance = event.data;
		
		// get the file urls to upload
		var fileUrls = [];
		$('.filepath').each(function() {
			var file = $(this);
			if (!file.hasClass('filepath_template')) {
				fileUrls.push(file.data('path'));
			}
		});
		
		// upload the files
		instance.expectedNumFilesToTransfer = fileUrls.length;
		var aid = $('.albums_list').val();
		window.BumpTopNative.requestUploadPhotos(aid, fileUrls, 'window.currentWidgetPage.onUploadSuccessful');
		
		// block until we can return (only if we've got images queued to upload)
		if(instance.expectedNumFilesToTransfer > 0) {
			$.blockUI({
				message: $('#updating_dialog'),
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
		}		
	};

	this.onUploadSuccessful = function(result, successful) {
		this.expectedNumFilesToTransfer -= 1;
		if (this.expectedNumFilesToTransfer <= 0) {
			// show the photos uploaded dialog
			$.blockUI({
				message: $('#photos_uploaded_dialog'),
				css: {
					padding: '20px',
					width: '34%',
					left: '29%',
					top: '27%',
					border: '2px solid rgb(109, 140, 203)'
				}
			});
			this.removeAllFiles();

			var id = $('.albums_list').val();
			var instance = this;
			$('.albums_list option').each(function() {
				if (id == $(this).val()) {
					var permalink = $(this).data('link');
					var thisInstance = instance;
					$('#uploaded_view_in_browser').unbind().bind('click', function() {
						thisInstance.launchUrl(permalink);
					});
					$('#uploaded_return_to_feed').unbind().bind('click', function() {
						$.unblockUI();
						window.location = "facebook_newsfeed.html";
					});
				}
			});

			// reload the albums
			window.BumpTopNative.requestAlbums('window.currentWidgetPage.onRequestAlbumsSuccessful', false);
		}
	};

	// Handles HTML5 dnd
	// Upload a list of files to the share.
	// files is an array of objects with properties "name", "mediaType", "size", 
	// and "url". The URL is a filedata: URL that can be passed to BumpTop.
	this.onDragEnterOverride = function(event) {
		var instance = event.data;
		$.unblockUI();

		// event is actually a jQuery wrapper; we need the original DOM event
		var dataTransfer = event.originalEvent.dataTransfer;

		var acceptDrop = false;
		// Ensure that only files are being dropped
		for (var i = 0; i < dataTransfer.types.length; i++) {		
			if (dataTransfer.types[i] === "Files") {
				acceptDrop = instance.areFilesOfSupportedImageFormat(instance, dataTransfer.files);
				if (!acceptDrop) {
					alert(TR("Facebook only accepts images!"));
				}
			} else {
				acceptDrop = false;
				break;
			}
		}
		// If we are accepting the drop, we have to return false, to
		// prevent the default action
		return !acceptDrop;
	};

	this.onDragLeaveOverride = function(event) {
		return false;
	};

	this.onDragOverOverride = function(event) {
		return false; // Means we accept the drag
	};

	this.onDropOverride = function(event) {
		var instance = event.data;
		
		// get the existing paths
		var fileUrls = [];
		$('.filepath').each(function() {
			var file = $(this);
			if (!file.hasClass('filepath_template')) {
				fileUrls.push(file.data('path'));
			}
		});
		
		// get all the files to be dropped
		var foundExistingFile = false;
		var files = event.originalEvent.dataTransfer.files;
		for (var i = 0; i < files.length; ++i) {			
			if (jQuery.inArray(files[i].url, fileUrls) == -1) {
				var file = $('.filepath_template:first').clone().insertBefore('.filepath:first');			
				file.removeClass('filepath_template');
				file.find('img').attr('src', window.BumpTopNative.getFilePathForUrl(files[i].url));
				file.find('.name').text(files[i].name);
				file.find('.remove').bind('click', instance, instance.removeFile);
				file.data('path', files[i].url);
			} else {
				foundExistingFile = true;
			}
		}
		if (foundExistingFile) {
			alert(TR('Some files are already in queue to upload!'));
		}
	};
}
FacebookWidgetUploadPage.prototype = new FacebookWidgetPage();
