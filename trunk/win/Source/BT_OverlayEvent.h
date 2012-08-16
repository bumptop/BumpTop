// Copyright 2012 Google Inc. All Rights Reserved.
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

#pragma once

#ifndef _BT_OVERLAYEVENT_
#define _BT_OVERLAYEVENT_

// -----------------------------------------------------------------------------

// forward declarations
class OverlayComponent;

// -----------------------------------------------------------------------------

	/*
	 * Overlay Event
	 * 
	 * - The base event class for OverlayEventHandlers to get information about the 
	 *	 specified event from.
	 */
	class OverlayEvent
	{
		OverlayComponent * _target;
		unsigned int _time;

	public:
		OverlayEvent(OverlayComponent * target);
		virtual ~OverlayEvent();

		// accessors
		OverlayComponent * getTarget() const;
		unsigned int getTime() const;
	};


	/* 
	 * Mouse Overlay Event
	 *
	 * - Mouse specific event information
	 */
	class MouseOverlayEvent : public OverlayEvent
	{
		Vec3 _screenPos;
		Vec3 _absScreenPos;
		int _button;
		bool _intersectsTarget;
		// XXX: Vec3 _worldPos;
		// XXX: Vec3 _prevScreenPos;

	public:
		MouseOverlayEvent( OverlayComponent * target, const Vec3& screenPos, const Vec3& absScreenPos, int button=0, bool intersectsTarget=false);
		virtual ~MouseOverlayEvent();

		// accessors
		const Vec3& getPosition() const;
		const Vec3& getAbsolutePosition() const;
		int getButton() const;
		bool intersectsTarget() const;
	};


	/*
 	 * Timer Overlay Event
	 *
	 * - Timer specific event information
	 */
	class TimerOverlayEvent : public OverlayEvent
	{
		unsigned int _millisElapsed;

	public:
		TimerOverlayEvent(OverlayComponent * target, unsigned int millisElapsed);
		virtual ~TimerOverlayEvent();

		// accessors

		// Returns the elapsed time in milliseconds
		unsigned int getElapsed() const;
	};


	/*
	 * Mouse Overlay Event Handler
	 *
	 * - Handler interface for handling mouse events.
	 */
	class MouseOverlayEventHandler
	{
	protected:
		bool _wasHovered;
		MouseOverlayEventHandler();

	public:
		virtual bool onMouseDown(MouseOverlayEvent& mouseEvent) = 0;
		virtual bool onMouseUp(MouseOverlayEvent& mouseEvent) = 0;
		virtual bool onMouseMove(MouseOverlayEvent& mouseEvent) = 0;
		
		bool wasMouseLastHovered() const;
		void setMouseLastHovered(bool value);

		bool intersects(const Vec3& pos, const Bounds& bounds);
	};


	/*
	 * Timer Overlay Event Handler
	 *
	 * - Handler interface for handling timer events.
	 */
	class TimerOverlayEventHandler
	{
	public:
		virtual bool onTimer(TimerOverlayEvent& timerEvent) = 0;
	};

#else
	class OverlayEvent;
	class MouseOverlayEvent;
	class TimerOverlayEvent;
	class MouseOverlayEventHandler;
	class TimerOverlayEventHandler;
#endif