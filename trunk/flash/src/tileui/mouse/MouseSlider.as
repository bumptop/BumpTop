//
//  Copyright 2012 Google Inc. All Rights Reserved.
//  
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  
//      http://www.apache.org/licenses/LICENSE-2.0
//  
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//

package tileui.mouse
{
	import flash.events.Event;
	import flash.events.EventDispatcher;
	import flash.events.MouseEvent;
	import flash.geom.Point;
	
	import mx.collections.ArrayCollection;
	import mx.managers.ISystemManager;
	
	import org.papervision3d.core.utils.virtualmouse.VirtualMouseMouseEvent;
	
	import tileui.tiles.Tile;
	import tileui.tiles.TileGroup;

	[Event(name="change", type="flash.events.Event")]
	[Event(name="mouseUp", type="flash.events.MouseEvent")]
	
	[ExcludeClass]
	
	public class MouseSlider extends EventDispatcher
	{
		static public const VERTICAL:int = 0;
		static public const HORIZONTAL:int = 1;
		
		private var _direction:int;
		
		private var _regPoint:Point;
		
		private var _maxScreenDistance:Number;
		
		public var targetGroup:TileGroup;
		public var targetTile:Tile;
		public var targetTiles:ArrayCollection;
		
		private var _systemManager:ISystemManager;
		
		public function get maxScreenDistance():Number {
			return _maxScreenDistance;
		}
		
		[Bindable]
		public function set maxScreenDistance(value:Number):void {
			_maxScreenDistance = value;
		}
		
		public function MouseSlider(regPoint:Point, systemManager:ISystemManager, maxScreenDistance:Number=200, direction:int=VERTICAL) {
			_regPoint = new Point(regPoint.x, regPoint.y);
			
			_direction = direction;
			
			_maxScreenDistance = maxScreenDistance;
			
			_systemManager = systemManager;
			
			_systemManager.addEventListener(MouseEvent.MOUSE_MOVE, handleMouseMove);
			_systemManager.addEventListener(MouseEvent.MOUSE_UP, removeListener);
		}
		
		private function removeListener(event:MouseEvent):void {
			dispatchEvent(event.clone());
			
			_systemManager.removeEventListener(MouseEvent.MOUSE_MOVE, handleMouseMove);
			_systemManager.removeEventListener(MouseEvent.MOUSE_UP, removeListener);
		}
		
		private function handleMouseMove(event:MouseEvent):void {
			
			if(event is VirtualMouseMouseEvent) {
				return;
			}
			
			var point:Point = new Point(event.stageX, event.stageY);
			
			if(_direction == VERTICAL) {
				point.x = _regPoint.x;
			}
			else if(_direction == HORIZONTAL) {
				point.y = _regPoint.y;
			}
			
			var distance:Number = Point.distance(point, _regPoint);
			
			var percent:Number = Math.min(distance/_maxScreenDistance, 1);
			
			if(_direction == VERTICAL && point.y < _regPoint.y) {
				percent *= -1;
			}
			else if(_direction == HORIZONTAL && point.x < _regPoint.x) {
				percent *= -1;
			}
			
			if(percent != _value) {
				_value = percent;
				dispatchEvent(new Event(Event.CHANGE));
			}
		}
		
		private var _value:Number;
		
		[Bindable(event="change")]
		public function get value():Number {
			return _value;
		}
	}
}