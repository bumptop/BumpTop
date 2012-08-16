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

// @requires btfb_common.js
// @requires btfb_kineticscroll.js
// @requires blockui

// make overlay more transparent 
/*
$.blockUI.defaults.css.padding = 10;
$.blockUI.defaults.css.border = '0px solid transparent'; 
$.blockUI.defaults.css.color = '#FFFFFF'; 
$.blockUI.defaults.css.backgroundColor = '#000000';
$.blockUI.defaults.css.opacity = 0.8;
$.blockUI.defaults.css.fontWeight = 'bold';
$.blockUI.defaults.overlayCSS.opacity = 0.0;
$.blockUI.defaults.overlayCSS.backgroundColor = '#FFFFFF';
*/
		
// notify bumptop to disable the mouse events pass-through
window.disableBumpTopMouseEvents = true;

function FacebookWidgetPage() {
	this.uid = '';

	// Base constructor, must be constructed after document onReady
	this.initCommon = function() {
	    window.currentWidgetPage = this;
	    this.uid = window.BumpTopNative.getUid();

	    // bind the facebook link click
	    $(".logout").bind('click', this, this.logout);

	    // bind the status popup events
	    $('.status_button').bind('click', this, this.showStatusPopup);
	    $('#status_dialog .cancel').bind('click', this, this.hideStatusPopup);
	    $('.status_update > form').bind('submit', this, this.setStatus);

		// hide status and upload buttons, right-align logout button
		// TODO: if this is accepted, remove the buttons from all html files
	    $('.status_button').hide();
	    $('.navtitle').eq(3).hide();
	    $('.navtitle').eq(4).css("float", "right");
	    $('.navtitle').eq(4).css("marginRight", "10px");

	    // We do special translation changes for value attributes since they can't
	    // be wrapped with the usual _() method.
	    $('#status_dialog_submit').val(TR("Share"));

	    // update the last status message if there is one
	    var lastMessage = window.BumpTopNative.getPersistentStoreValue('status-message');
	    if (lastMessage.length > 0) {
	        this.setLastStatusText(lastMessage, true);
	    }
	};

	// takes the "px" off the end of css dimensions, so they can be used as numbers
	this.stripPx = function(pxStr) {
		return (pxStr.substring(0, pxStr.indexOf("px"))-0);
	};
	
	// Handles HTML5 dnd
	// Upload a list of files to the share.
	// files is an array of objects with properties "name", "mediaType", "size", 
	// and "url". The URL is a filedata: URL that can be passed to BumpTop.
	this.onDragEnter = function(event) {
		return false;
	};

	this.onDragLeave = function(event) {
		return false;
	};

	this.onDragOver = function(event) {
		return false;
	};

	this.onDrop = function(event) {
		// send the user to the drop page with the specified files
		var fileDataJson = JSON.stringify(event.originalEvent.dataTransfer);		
		window.BumpTopNative.setTemporaryStoreValue('upload_files_json', fileDataJson);
		window.location = 'facebook_upload.html';
	};
	
	// Launches the facebook website
	this.launchFacebookWebsite = function(event) {
		var instance = event.data;
		// TODO: launch the facebook profile page instead
		// http://www.facebook.com/profile.php?id=<uid>		
		instance.launchUrl('http://www.facebook.com?referrer=BumpTop');
	};

	// Launches an arbitrary website
	this.launchUrl = function(url) {
		window.BumpTopNative.launchUrl(url);
	};

	// Logs the user out of facebook and returns them to the login page
	this.logout = function(event) {
		window.BumpTopNative.logout();
	};
	
	// Helper function to determine if an event cursor happened in an element or not
	this.isInElementRect = function(jqelement, yBuffer, event) {		
		var position = jqelement.position();
		var width = jqelement.width();
		var height = jqelement.height();
		return ((event.pageX > position.left) &&
			(event.pageX < (position.left + width)) &&
			(event.pageY > position.top) &&
			(event.pageY < (position.top + height + yBuffer)));
	};

	// Helper functions to show/hide the status popup
	this.showStatusPopup = function(event) {
		$("#status_dialog .cancel").text(TR("Cancel"));
		$('#status_dialog').fadeIn('fast');
		$('.status_update_textarea').focus();
		$('.status_button').addClass('selected');
		
		event.preventDefault();
		event.stopPropagation();
		event.stopImmediatePropagation();		
		return false;
	};

	this.hideStatusPopup = function(event) {
		$('#status_dialog').fadeOut('fast');
		$('.status_button').removeClass('selected');
		
		event.preventDefault();
		event.stopPropagation();
		event.stopImmediatePropagation();		
		return false;
	};

	// Updates the last status text that was submitted
	this.setLastStatusText = function(message, show) {
		$('.last_status').html('&ldquo;' + message + '&rdquo;');
		if (show) { 
			$('.last_status').fadeIn();
		}
	};

	// Updates the user's facebook status
	this.setStatus = function(event) {		
		var instance = event.data;
		var textArea = $(this).find('textarea');
		var statusMessage = textArea.val();
		$(this).find('textarea').each(function() {
			// temporarily disable the fields
			$(this).attr("disabled", "true");	
			$(this).oneTime(2000, function(i) {
				$(this).removeAttr("disabled");
			});
		});		
		$(this).find('input').each(function() {
			// temporarily disable the fields
			var message = statusMessage;
			$(this).attr("disabled", "true");	
			$(this).val(TR("Sending..."));
			$(this).oneTime(2000, function(i) {
				$(this).removeAttr("disabled");
				$(this).val("Share");	
				$("#status_dialog .cancel").text(TR("Close"));
				
				// reset the fields and show the "previous" status
				window.BumpTopNative.setPersistentStoreValue('status-message', message);
				instance.setLastStatusText(message, true);
				textArea.val("");
			});			
		});
		window.BumpTopNative.setStatus(statusMessage);
		return false;
	};
}
