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

$(document).ready(function() {
	var pageUrl = document.location.href;

	function setCheckbox() {
		var trimmedVal = $.trim($("#email").val());
		$("#emailCheckbox").attr("checked", trimmedVal.length > 0);			
	}

	if (pageUrl.match("ConnectExistingShareDialog.html")) {
		// TODO: We should load the saved email address here

		$("#email")
			.val(TR("Enter your email address"))
			.addClass("with_instructions");
			
		$("#email")
			.keyup(setCheckbox)
			.change(setCheckbox);
	}

	// For all the text boxes that have instruction messages inside them,
	// remember the original instructions, and add event handlers to hide and
	// show the instructions when appropriate.
	$("input.with_instructions")
		.each(function () {
			// Remember the original text as the instruction message
			$(this).data("instruction_message", $(this).val());
		})
		.focus(function () {
			// If it hasn't been modified, clear the instructions & styling
			if ($(this).hasClass("with_instructions")) {
				$(this).val("").removeClass("with_instructions");
			}
		})
		.blur(function () {
			// If the text is empty, put the instructions back in
			var trimmedVal = $.trim($(this).val());
			if (trimmedVal.length === 0) {
				$(this)
					.val($(this).data("instruction_message"))
					.addClass("with_instructions");
			}
		});

	// For textboxes (input type="text") that could possibly have the
	// "with_instructions" class, use this helper to fetch the contents,
	// to ensure you don't get the instruction text rather than the real value	
	function textbox_val(selector) {
		return $(selector).hasClass("with_instructions") ? "" : $(selector).val();
	}
	
	$("#okButton, #yesButton").click(function() {
		var dialogResults = {};
		var dropUrlPrefix = "http://drop.io/";
		if (pageUrl.match("ConnectExistingShareDialog.html")) {
			if ($("#dropName").val() === "") {
				$("#emptyNameAlert").show();
				return;
			}
			// This field might actually contain a full URL. Check for that,
			// and return just the name in either case.
			var dropName = $("#dropName").val().replace(dropUrlPrefix, "");

			dialogResults.dropName = dropName;
			dialogResults.password = $("#password").val();
			dialogResults.email = textbox_val("#email");
		} else if (pageUrl.match("DeleteTabDialog.html")) {
			dialogResults.choice = "Yes";
		} else if (pageUrl.match("InitializationDialog.html")) {
			dialogResults.email = $("#email").val();
		} else if (pageUrl.match("InviteFriendsDialog.html")) {
			dialogResults.emailFriends = $("#emailFriends").val();
		} else if (pageUrl.match("CreateShareDialog.html")) {
			dialogResults.dropName = $("#dropUrl").text().slice(dropUrlPrefix.length);
		}
		if (pageUrl.match("CreateShareDialog.html") || pageUrl.match("SettingsModifyDialog.html")) {
			dialogResults.password = $("#password").val();	
			dialogResults.publicPassword = $("#publicPassword").val();
			dialogResults.email = $("#email").val();
		}
		BumpTopNative.setDialogBoxResult(dialogResults);
		BumpTopNative.closeDialogWindow();
	});
	
	// Non-admin specific functionality
	$("#okUserButton").click(function() {
		var dialogResults = {};
		if (pageUrl.match("SettingsModifyDialog.html")) {
			dialogResults.email = $("#userEmail").val();
		}
		BumpTopNative.setDialogBoxResult(dialogResults);
		BumpTopNative.closeDialogWindow();
	});
	
	$("#cancelButton, #finishButton, #cancelUserButton").click(function() {
		BumpTopNative.closeDialogWindow();
	});
	
	$("#noButton").click(function() {
		var dialogResults = {};
		dialogResults.choice = "No";
		BumpTopNative.setDialogBoxResult(dialogResults);
		BumpTopNative.closeDialogWindow();
	});
});