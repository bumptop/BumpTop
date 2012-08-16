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

// A jQuery UI plugin for providing a scrolling div with momentum. 
(function($) {

// If the mouse moves more than this distance between mouse down and mouse up,
// it is considered to be a drag rather than a click.
var MAX_CLICK_DISTANCE = 8;

// Maximum velocity for scrolling
var MAX_VELOCITY = 3;

$.widget("ui.kineticScroll", $.extend({}, $.ui.mouse, {

	_init: function () {
		this._mouseInit();
		this.jqParent = $(this.element).parent();
		if (this.jqParent.children().length !== 1) {
			throw "ERROR: kineticScroll expects a container with a single child";
		}
		// Records the last known position of the mouse
		this.previousPosX = 0;

		// Position & time used as reference for velocity calculation (updated on a timer)
		this.velocityRefX = 0;
		this.velocityRefTime = 0;
	},

	destroy: function () {
		if (this.element.data('kineticScroll')) {
			this.element.removeData("kineticScroll");
			this._mouseDestroy();
		}
	},

	// Called to allow us to capture the mouse.	
	_mouseCapture: function (event) {
		return true;
	},
	
	// Called at the start of a drag (or click) event
	_mouseStart: function (event) {
		this.previousPos = {
			x: event.pageX,
			y: event.pageY
		};

		this.mouseDownPos = {
			x: event.pageX,
			y: event.pageY
		};
		
		// Set a timer to record the mouse position every 50ms.
		// On mouse up, the last recorded point will serve as the reference
		// velocity calculations. 
		var self = this;
		this.mousePosInterval = setInterval(function () {
			self.velocityRefX = self.previousPos.x;
			self.velocityRefTime = (new Date()).getTime();		
		}, 50);
		
		// Record the total distance travelled by the mouse while it's down,
		// in order to determine whether it's a drag or single click
		this.totalDistance = 0;
	},

	// Called when the mouse is moved during a drag event
	_mouseDrag: function (event) {
		var delta = {
			x: this.previousPos.x - event.pageX, 
			y: this.previousPos.y - event.pageY
		};
		this.previousPos.x = event.pageX;
		this.previousPos.y = event.pageY;
		
		var dragDist = delta.x;
		
		if (this.totalDistance <= MAX_CLICK_DISTANCE) {
			this.totalDistance += Math.sqrt(delta.x * delta.x + delta.y * delta.y);
			
			// If we just moved far enough, we need to drag for the entire
			// distance, not just the amount moved in this increment.
			if (this.totalDistance > MAX_CLICK_DISTANCE) {
				dragDist = this.mouseDownPos.x - event.pageX;
			}
		}
		
		if (this.totalDistance > MAX_CLICK_DISTANCE) {
			// Stop any existing animation
			this.jqParent.stop(true, true);
	
			var newScrollLeft = Math.max(0, this.jqParent.scrollLeft() + dragDist);
			this.jqParent.scrollLeft(newScrollLeft);	
		}
		// We need to cancel the action to avoid selecting text, dragging images, etc.
		return false;
	},

	// Called at the end of a drag event, on mouse up
	_mouseStop: function (event) {
		// Check if this is a normal single click
		if (this.totalDistance <= MAX_CLICK_DISTANCE) {
			var el = document.elementFromPoint(event.pageX, event.pageY);
			$(el).trigger('touchclick');
			return;
		}
		
		$(this.jqParent).stop(true, true);

		var deltaT = (new Date()).getTime() - this.velocityRefTime;
		
		var velocity = (event.pageX - this.velocityRefX) / deltaT;
		// limit velocity to reasonable speed (so movement is animated, not instant)
		velocity = Math.min(velocity, MAX_VELOCITY);
		velocity = Math.max(velocity, -1 * MAX_VELOCITY);
		console.log(velocity);
		var accel = -0.004; // pixels/ms

		// Make sure acceleration (friction) is opposite to velocity
		if (velocity < 0) {
			accel *= -1;
		}

		// v**2 = v0**2 + 2as, where
		//    v - final velocity
		//    v0 - initial velocity
		//    a - acceleration
		//    s - distance travelled
		// With v = 0, solving for s:
		var dist = (velocity * velocity) / (2 * accel);

		// s = ut + 0.5at**2
		// Solving for t:
		var time = -velocity / accel;

		this.jqParent.animate({ scrollLeft: "+=" + dist }, time, "easeOutCubic");
	}
}));

// We must set distance to 0 to ensure that drag is invoked even if the mouse
// hasn't moved at all (required so that we can send the "touchclick" event)
$.ui.kineticScroll.defaults = $.extend({}, $.ui.mouse.defaults, { distance: 0 });

})(jQuery);