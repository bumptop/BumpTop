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

package tileui.tiles
{
	import flash.events.Event;
	import flash.events.EventDispatcher;
	import flash.events.MouseEvent;
	import flash.events.TimerEvent;
	import flash.filters.DropShadowFilter;
	import flash.filters.GlowFilter;
	import flash.geom.Point;
	import flash.utils.Timer;
	
	import mx.collections.ArrayCollection;
	import mx.core.Application;
	import mx.core.IToolTip;
	import mx.managers.ToolTipManager;
	
	import org.papervision3d.events.InteractiveScene3DEvent;
	
	import tileui.ape.particles.TileUIParticle;
	import tileui.physics.events.ParticleEvent;
	import tileui.physics.particles.IPhysicsParticle;
	import tileui.tiles.pv3d.Base3DTile;
	import tileui.tween.TweenManager;
	
	[Event(name="doubleClick", type="flash.events.MouseEvent")]
	[Event(name="mouseDown", type="flash.events.MouseEvent")]
	[Event(name="mouseUp", type="flash.events.MouseEvent")]
	[Event(name="rollOver", type="flash.events.MouseEvent")]
	[Event(name="rollOut", type="flash.events.MouseEvent")]
	
	public class Tile extends EventDispatcher
	{
		/**
		 * When a Tile is created and first added to the workspace, if <code>originPoint</code> is set it will get added
		 * at that point. Otherwise, if <code>originPoint</code> is not set the Tile will get added at the center of the
		 * workspace.
		 */
		public var originPoint:Point;
		
		/**
		 * @private
		 */
		public var originalMass:Number;
		
		/**
		 * @private
		 */
		private static var tooltip:IToolTip;
		
		/**
		 * If <code>toolTipString</code> is set then hovering the mouse over this Tile will show the tooltip.
		 */
		public var toolTipString:String;
		
		
		/**
		 * @private
		 */
		protected var _tile3D:Base3DTile;
		
		/**
		 * The <code>Base3DTile</code> that is the PaperVision3D representation of this tile in 3D.
		 * 
		 * @see tileui.tiles.pv3d.Base3DTile
		 */
		public function get tile3D():Base3DTile {
			return _tile3D;
		}
	
		/**
		 * @private
		 */
		private var _particle:TileUIParticle;
		
		/**
		 * @private
		 */
		public function get particle():TileUIParticle {
			return _particle;
		}
	
		/**
		 * If this Tile is part of a group (ie a stack of tiles) then this property will indicate the <code>TileGroup</code>
		 * that it belongs to.
		 */
		public var group:TileGroup;
		
		/**
		 * @private
		 */
		private var _offsetX:Number = 0;
		
		/**
		 * @private
		 */
		public function get offsetX():Number {
			return _offsetX;
		}
		
		/**
		 * @private
		 */
		public function set offsetX(value:Number):void {
			_offsetX = value;
			
			particleUpdateHandler();
		}
		
		/**
		 * @private
		 */
		private var _offsetZ:Number = 0;
		
		/**
		 * @private
		 */
		public function get offsetZ():Number {
			return _offsetZ;
		}
		
		/**
		 * @private
		 */
		public function set offsetZ(value:Number):void {
			_offsetZ = value;
			
			particleUpdateHandler();
		}
		
		private var _width:Number;
		
		public function get width():Number {
			return _width;
		}
		
		private var _height:Number;
		
		public function get height():Number {
			return _height;
		}
		
		public function Tile(width:Number=60, height:Number=10):void {
			
			_width = width;
			_height = height;
			
			_tile3D = create3DTile(width, height);
			
			originalMass = width;
			
			_particle = new TileUIParticle(0,0, width/1.7, false, originalMass, 0, 0, 0);
			
			_particle.addEventListener(ParticleEvent.DRAG_OUTSIDE, particleDragOutside);
			_particle.onCollisionFunction = particleCollisionHandler;
			_particle.onUpdateFunction = particleUpdateHandler;
		}
		
		/**
		 * To create your own custom extension of <code>Tile</code> you shoudl override this method to return
		 * the appropriate kind of 3D tile you want to use.
		 * 
		 * @param width The size of the square top face of the Tile. All Tiles have a square face on the top and bottom, you cannot
		 * make the top face a rectangle, it must be square.
		 * 
		 * @param height The vertical height of the Tile, which is the height above the ground that the tile rises.
		 * 
		 * @see tileui.tiles.pv3d.Base3DTile
		 * @see tileui.tiles.pv3d.Image3DTile
		 * @see tileui.tiles.pv3d.BitmapData3DTile
		 * @see tileui.tiles.pv3d.Text3DTile
		 * @see tileui.tiles.pv3d.ImageText3DTile
		 */
		protected function create3DTile(width:Number, height:Number):Base3DTile {
			return new Base3DTile(width, height);
		}
		
		/**
		 * If you want to do something to the Tile after it gets added to the display list then you can extend this method.
		 * Make sure to call <code>super.tileAddedToComponents()</code>, because that is needed to add the requisite mouse handlers
		 * for mouse interaction with a tile.
		 */
		public function tileAddedToComponents():void {
			_tile3D.addEventListener(InteractiveScene3DEvent.OBJECT_PRESS, redispatchEvent);
			_tile3D.addEventListener(InteractiveScene3DEvent.OBJECT_RELEASE, redispatchEvent);
			_tile3D.addEventListener(InteractiveScene3DEvent.OBJECT_DOUBLE_CLICK, redispatchEvent);
			_tile3D.addEventListener(InteractiveScene3DEvent.OBJECT_OVER, redispatchEvent);
			_tile3D.addEventListener(InteractiveScene3DEvent.OBJECT_OUT, redispatchEvent);
		 }
		
		/**
		 * @private
		 */
		private var _scale:Number = 1;
		
		[Bindable]
		/**
		 * Sets the scale (as a percentage) of the Tile. This is used to etermine the size of the displayed tile.
		 * To set the size of a Tile you can set hte width and height in the constructor, and you can also set the
		 * <code>scale</code>. The <code>scale</code> gets set when the "Resiuze" menu item is invoked on a Tile or group
		 * of Tiles.
		 */
		public function set scale(value:Number):void {
			_scale = value;
			
			_tile3D.scaleX = _scale;
			_tile3D.scaleY = _scale;
			_tile3D.scaleZ = _scale;
			_particle.width = _particle.originalWidth*_scale;
			
			_particle.mass = originalMass * _scale;
		}
		
		public function set scaleX(value:Number):void {
			_tile3D.scaleX = value;
			_particle.width = _particle.originalWidth*_scale;
		}
		
		/**
		 * @private
		 */
		public function get scale():Number {
			return _scale;
		}
		
		/**
		 * @private
		 */
		private function particleDragOutside(event:ParticleEvent):void {
			dispatchEvent(new ParticleEvent(ParticleEvent.DRAG_OUTSIDE));
		}
		
		/**
		 * @private
		 */
		public function update():void {
			 particleUpdateHandler();
		}
		
		/**
		 * @private
		 */
		private function particleUpdateHandler():void {
					
			if(!_tile3D || _tile3D.disconnectFromPhysics) return;
			
			
			var tileContainerWidth:Number = _tile3D.parentUIContainer.width;
			var tileContainerHeight:Number = _tile3D.parentUIContainer.height;
			
			
			var xPercent:Number = (_particle.px + _offsetX)/_particle.parentUIContainer.width;
			var yPercent:Number = (_particle.py - _offsetZ)/_particle.parentUIContainer.height;
			
			_tile3D.x = tileContainerWidth*xPercent - tileContainerWidth/2;
			_tile3D.z = -tileContainerHeight*yPercent + tileContainerHeight/2;
			
			var yPos:Number = _tile3D.y;
			
			if(!_tile3D.ignoreRotation)
				_tile3D.rotationY = 180+_particle.angle;
		}
		
		/**
		 * @private
		 */
		private function particleCollisionHandler(p1:IPhysicsParticle, p2:IPhysicsParticle):void {
			
			if(_tile3D.disconnectFromPhysics || (_particle.group && _particle.group.collideInternal == false)) return;
				
			if(!_particle.dragging) {
				TweenManager.addTween(_tile3D, {rotationZ:_particle.velocity.y * 1.2, rotationX:_particle.velocity.x*1.2, time:.2});
				TweenManager.addTween(_tile3D, {rotationZ:0, rotationX:0, time:.4, delay:.2, transition:"easeInOutBack"});
			}
		}
		
		
		/**
		 * Your custom extensons of Tile should implement the <code>getMenuItems</code> method, which should
		 * return all custom menu items that should be displayed for your tile.
		 * 
		 * @param tiles An ArrayColleciton of <code>Tile</code> objects that are in the current Tile's group. If the Tile is not in a group,
		 * the <code>tiles</code> parameter will be an ArrayCollection with only one item. Often you will want to show a given menu item if
		 * the tile is grouped with all the same type of tiles. To do this you can check the types of the items in the <code>tiles</code> 
		 * ArrayCollection and return the appropriate menu items.
		 * 
		 * @param group the <code>TileGroup</code> that he menu was invoked on.
		 */
		public function getMenuItems(tiles:ArrayCollection, group:TileGroup):Array {
			return [];
		}
		
		/**
		 * @private
		 */
		public function get parentApplication():Application {
			return tile3D.parentUIContainer.parentApplication as Application;
		}
		
		/**
		 * @private
		 */
		private var tooltipTimer:Timer;

		/**
		 * @private
		 */
		protected function redispatchEvent(event:Event):void {
			if(toolTipString && toolTipString != "") {
				switch(event.type) {
					case InteractiveScene3DEvent.OBJECT_OVER:
						if(Tile.tooltip) {
							ToolTipManager.destroyToolTip(Tile.tooltip);
							Tile.tooltip = null;
						}
						
						tooltipTimer = new Timer(1000, 1);
						tooltipTimer.addEventListener(TimerEvent.TIMER_COMPLETE, showTooltip);
						
						Tile.tooltip = ToolTipManager.createToolTip(toolTipString, (event as InteractiveScene3DEvent).sprite.stage.mouseX, (event as InteractiveScene3DEvent).sprite.stage.mouseY);
						Tile.tooltip.visible = false;
						tooltipTimer.start();
					
						break;
					case InteractiveScene3DEvent.OBJECT_OUT:
					case InteractiveScene3DEvent.OBJECT_PRESS:
					case InteractiveScene3DEvent.OBJECT_RELEASE:
					case InteractiveScene3DEvent.OBJECT_RELEASE_OUTSIDE:
					case InteractiveScene3DEvent.OBJECT_CLICK:
						if(ToolTipManager.currentToolTip) {
							ToolTipManager.destroyToolTip(ToolTipManager.currentToolTip);
						}
						
						if(Tile.tooltip) {
							ToolTipManager.destroyToolTip(Tile.tooltip);
							Tile.tooltip = null;
						}
						
						if(tooltipTimer && tooltipTimer.running) {
							tooltipTimer.stop();
						}
						break;
				}
			}
			
			dispatchEvent(event.clone());
		}
		
		/**
		 * @private
		 */
		private function showTooltip(event:TimerEvent):void {
			if(tooltip) {
				tooltip.visible = true;
			}
		}
	}
}