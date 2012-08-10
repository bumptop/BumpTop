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

package tileui.controls.menu
{
	import flash.events.Event;
	import flash.events.MouseEvent;
	import flash.geom.Point;
	import flash.utils.getTimer;
	
	import mx.collections.ArrayCollection;
	import mx.containers.Canvas;
	import mx.core.UIComponent;
	import mx.events.FlexEvent;
	
	import org.papervision3d.core.utils.virtualmouse.VirtualMouseMouseEvent;

	[Event(name="change", type="flash.events.Event")]
    [Event(name="release", type="flash.events.Event")]
    
    [ExcludeClass]
    
	public class DrawingCanvas extends Canvas
	{
		private var time:Number;
	
		private var line:UIComponent;
		private var circle:CircleMenu;
		
		public static const SPACED:int=0;
		public static const MESSY:int=1;
		public static const STACKED:int=2;
		public static const SPIRAL:int=3;
		public static const REMOVE:int=4;
		public static const RESIZE:int=5;
		
		public function DrawingCanvas():void {
			super();
			
			this.addEventListener(MouseEvent.MOUSE_DOWN, handleMouseDown);
		}
		
		public function get hitTestComponent():UIComponent {
			return line;
		}
		
		[Bindable]
		public var drawMenu:Boolean;
		
		[Bindable]
		public var selecting:Boolean = false;
		
		private var firstPoint:Point;
		
		public var mode:int;
		
		private var points:Array;
		
		override protected function createChildren():void {
			super.createChildren();
			
			line = new UIComponent();
			
			this.rawChildren.addChild(line);
			
			circle = new CircleMenu();
			circle.radius = 40;
			circle.visible = false;
			this.rawChildren.addChild(circle);
		}
		
		override protected function updateDisplayList(unscaledWidth:Number, unscaledHeight:Number):void {
			super.updateDisplayList(unscaledWidth, unscaledHeight);
			
			//need to fill with something (even alpha of 0 is fine) otherwise we don't get
			//mouse events (only Flex 4, Flex 3 doesn't care)
			graphics.clear();
			graphics.beginFill(0xff0000, 0);
			graphics.drawRect(0,0,unscaledWidth, unscaledHeight);
			graphics.endFill();
		}
		
		private function handleMouseDown(event:MouseEvent):void {
			if(!drawMenu || event.target != this) return;
			
			rawChildren.setChildIndex(line, rawChildren.numChildren-1);
			rawChildren.setChildIndex(circle, rawChildren.numChildren-1);
			
			mode = DrawingCanvas.STACKED;
			
			var point:Point = new Point(event.stageX, event.stageY);
			point = this.globalToLocal(point);
			
		
			firstPoint = point;
			
			
			
			time = getTimer();
			
			points = [new Point(this.mouseX, this.mouseY)];
			
			line.graphics.clear();
			line.graphics.moveTo(this.mouseX, this.mouseY);
			line.graphics.lineStyle(0, 0, .7);
			
			var themeColor:Number = getStyle("themeColor");
			line.graphics.beginFill(themeColor, .3); 
			
			circle.move(this.mouseX, this.mouseY)
			circle.visible = true;
			
			systemManager.addEventListener(MouseEvent.MOUSE_UP, handleMouseUp);
			
			systemManager.addEventListener(MouseEvent.MOUSE_MOVE, handleMouseMove);
			
			circle.addEventListener(MouseEvent.MOUSE_OVER, circleRollOver);
		
			selecting = true;
		}
		
		private function drawLasso():void {
			var themeColor:Number = getStyle("themeColor");
			
			line.graphics.clear();
			
			line.graphics.moveTo(this.mouseX, this.mouseY);
			line.graphics.lineStyle(0, 0, .7);
			line.graphics.beginFill(themeColor, .3); 
			
			if(points.length > 1) {
				var firstPoint:Point = points[0];
				line.graphics.moveTo(firstPoint.x, firstPoint.y);
			}
			
			for(var i:int=1; i<points.length; i++) {
				line.graphics.lineTo(points[i].x, points[i].y);
			}
			
			line.graphics.endFill();
		}
		
		private function circleRollOver(event:MouseEvent):void {
			
			if(getTimer() - time < 200) return;
			
			systemManager.removeEventListener(MouseEvent.MOUSE_MOVE, handleMouseMove);
			circle.removeEventListener(MouseEvent.MOUSE_OVER, circleRollOver);
			
			var pieMenu:PieMenu = new PieMenu();
			
			pieMenu.width = pieMenu.height = 300;
			pieMenu.setStyle("innerRadius", circle.radius/pieMenu.width*2);
		
			pieMenu.addEventListener(MouseEvent.MOUSE_DOWN, cancelEvent);
			pieMenu.addEventListener(FlexEvent.HIDE, removeMenu);
			pieMenu.addEventListener(MouseEvent.ROLL_OUT, clearMenu);
			
			systemManager.removeEventListener(MouseEvent.MOUSE_UP, handleMouseUp);
			
			
			rawChildren.addChild(pieMenu);
	
			pieMenu.dataProvider = new ArrayCollection([
			        new PieMenuItem("Create Pile", "selectionStacked", "stack these tiles", 1, PieMenuItem.CLICK_TYPE, setStackedMode),
			        new PieMenuItem("Spiral", "selectionSpiral", "stack and rotate these tiles", 1, PieMenuItem.CLICK_TYPE, setSpiralMode),
			        new PieMenuItem("Resize", "selectionResize", "resize the tiles", 1, PieMenuItem.DRAG_TYPE, setResizeMode),
			        new PieMenuItem("Remove", "selectionRemove", "remove the tiles", 1, PieMenuItem.REMOVE_TYPE, setRemoveMode)
			        ]);
			       
			pieMenu.move(circle.x - pieMenu.width/2, circle.y - pieMenu.height/2);
			
			this.removeEventListener(MouseEvent.MOUSE_DOWN, handleMouseDown);
		}
		
		private function cancelEvent(event:Event):void {
			event.stopPropagation();
			event.stopImmediatePropagation();
		}
		
		private function setSpacedMode(tiles:Array, menu:PieMenu=null):void {
			mode = SPACED;
			handleMouseUp();
		}
		
		private function setResizeMode(tiles:Array, menu:PieMenu=null):void {
			mode = RESIZE;
			handleMouseUp();
		}
		
		private function setMessyMode(tiles:Array, menu:PieMenu=null):void {
			mode = MESSY;
			handleMouseUp();
		}
		
		private function setStackedMode(tiles:Array, menu:PieMenu=null):void {
			mode = STACKED;
			handleMouseUp();
		}
		
		private function setSpiralMode(tiles:Array, menu:PieMenu=null):void {
			mode = SPIRAL;
			handleMouseUp();
		}
		
		private function setRemoveMode(tiles:Array, menu:PieMenu=null):void {
			mode = REMOVE;
			handleMouseUp();
		}
		
		private function removeMenu(event:Event):void {
			var menu:PieMenu = event.currentTarget as PieMenu;
			rawChildren.removeChild(menu);
			
			systemManager.addEventListener(MouseEvent.MOUSE_UP, handleMouseUp);
			
		}
		
		private function clearMenu(event:Event):void {
			var menu:PieMenu = event.currentTarget as PieMenu;
			menu.dataProvider = new ArrayCollection();
			removeSelection();
			line.graphics.clear();
		}
		
		private function handleMouseUp(event:MouseEvent=null):void {
			var e:Event = new Event("release");
			dispatchEvent(e);
			
			removeSelection();
			
			points = [];
			drawLasso();
			line.graphics.clear();
		}
		
		private function removeSelection():void {
			circle.visible = false;
		
			line.graphics.endFill();
			line.graphics.clear();
			
			systemManager.removeEventListener(MouseEvent.MOUSE_UP, handleMouseUp);
			systemManager.removeEventListener(MouseEvent.MOUSE_MOVE, handleMouseMove);
			
			selecting = false;
			
			this.addEventListener(MouseEvent.MOUSE_DOWN, handleMouseDown);
		}
		
		private var lastX:Number = 0;
		private var lastY:Number = 0;
		
		private function handleMouseMove(event:MouseEvent):void {
			
			if(event is VirtualMouseMouseEvent) {
				return;
			}
			
			var point:Point = new Point(event.stageX, event.stageY);
			point = this.globalToLocal(point);
			
			if(Math.abs(point.x - lastX) > 5 || Math.abs(point.y-lastY) > 5) {
				points.push(new Point(point.x, point.y));
				lastX = point.x;
				lastY = point.y;
			}
			
			drawLasso();
			
			line.graphics.lineStyle(0, 0, .7);
			line.graphics.lineTo(point.x, point.y);
			line.graphics.lineStyle(0, 0, 0);
			
			dispatchEvent(new Event(Event.CHANGE));
		}
		
		
	}
}