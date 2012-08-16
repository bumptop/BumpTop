/*
  Throbber Plugin for jQuery
  Version: v0.1.1

  Copyright (C) 2008 by Systemantics, Bureau for Informatics

  Lutz Issler
  Mauerstr. 10-12
  52064 Aachen
  GERMANY

  Web:    www.systemantics.net
  Email:  mail@systemantics.net

  This plugin is distributed under the terms of the
  GNU Lesser General Public license. The license can be obtained
  from http://www.gnu.org/licenses/lgpl.html.
*/

(function($){

	var defaultOptions = {
			ajax: true,
			delay: 0,
			image: "throbber.gif",
			parent: "",
			wrap: ""
		};

	// Hide all AJAX throbbers on AJAX stop
	$().ajaxStop(function() {
		_throbberHide($(".throbber_ajax"));
	});

	// Internal method
	// Important: options.parent must contain a proper parent (or null)!
	_throbberShow = function(options, jelement) {
		var jparent;
		if (options.parent) {
			jelement = null;
			jparent = $(options.parent);
		} else {
			jparent = (jelement ? jelement.parent() : $("body"));
		}
		if (jparent.find(".throbber").length==0) {
			// No throbber there yet
			window.clearTimeout(jparent.data("throbber_timeout"));
			jparent.data("throbber_timeout", window.setTimeout(function() {
				// Create throbber
				var throbber = $('<img src="'+options.image+'" class="throbber" />');
				if (options.ajax) {
					throbber.addClass("throbber_ajax");
				}
				if (jelement) {
					// Show local throbber
					throbber.data("throbber_element", jelement);
					jelement
						.hide()
						.after(throbber);
				} else {
					// Show attached throbber
					jparent
						.children().hide().end()
						.append(throbber);
				}
				if (options.wrap!="") {
					throbber.wrap(options.wrap);
				}
			}, options.delay));
		}
	};

	// Internal method
	// Hide all specified throbbers
	_throbberHide = function(throbbers) {
		throbbers.each(function() {
			var throbber = $(this);
			var jelement = throbber.data("throbber_element");
			if (jelement) {
				// Hide local throbber
				jelement.show();
			} else {
				throbber.siblings().show();
			}
			window.clearTimeout(throbber.parent().data("throbber_timeout"));
			throbber.remove();
		});
	}

	// Add throbbers to a set of elements
	$.fn.throbber = function(event, options) {
		if (typeof event=="undefined") {
			// Called as throbber()
			event = "click";
			options = {};
		} else if (typeof event=="object") {
			// Called as throbber(options)
			options = event;
			event = "click";
		} else if (typeof options=="undefined") {
			// Called as throbber(event)
			options = {};
		}
		options = $.extend({}, defaultOptions, options);
		$(this).each(function() {
			var jtarget = $(this);
			jtarget.bind(event, function() {
				_throbberShow(options, jtarget);
			});
		});
		return $(this);
	};

	// Immediately show a throbber
	$.throbberShow = function(options) {
		options = $.extend({}, defaultOptions, options);
		_throbberShow(options, null);
		return $;
	};

	// Hide all throbbers
	$.throbberHide = function() {
		_throbberHide($(".throbber"));
		return $;
	};

})(jQuery);