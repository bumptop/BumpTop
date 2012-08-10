/*
Copyright (c) 2006, 2007 Alec Cove

Permission is hereby granted, free of charge, to any person obtaining a copy of this 
software and associated documentation files (the "Software"), to deal in the Software 
without restriction, including without limitation the rights to use, copy, modify, 
merge, publish, distribute, sublicense, and/or sell copies of the Software, and to 
permit persons to whom the Software is furnished to do so, subject to the following 
conditions:

The above copyright notice and this permission notice shall be included in all copies 
or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF 
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE 
OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/*
TODO:
*/

package org.cove.ape {
	
	import flash.display.Sprite;
	import flash.display.DisplayObject;
	import flash.events.EventDispatcher;
	
	/** 
	 * The base class for all constraints and particles
	 */
	public class AbstractItem extends EventDispatcher {
		
		private var _sprite:Sprite;
		
		private var _solid:Boolean;
		private var _visible:Boolean;
		private var _alwaysRepaint:Boolean;
		
	
		/** @private */
		internal var lineThickness:Number;
		/** @private */
		internal var lineColor:uint;
		/** @private */
		internal var lineAlpha:Number;
		/** @private */
		internal var fillColor:uint;
		/** @private */
		internal var fillAlpha:Number;
		/** @private */
		internal var displayObject:DisplayObject;
		/** @private */
		internal var displayObjectOffset:APEVector;
		/** @private */
		internal var displayObjectRotation:Number;
		
		
		public function AbstractItem() {
			_solid = true;
			_visible = true;	
			_alwaysRepaint = false;
		}
		
		
		/**
		 * This method is automatically called when an item's parent group is added to the engine,
		 * an item's Composite is added to a Group, or the item is added to a Composite or Group.
		 */
		public function init():void {}
		
				
		/**
		 * The default painting method for this item. This method is called automatically
		 * by the <code>APEngine.paint()</code> method. 
		 */			
		public function paint():void {}	
		
		
		/**
		 * This method is called automatically when an item's parent group is removed
		 * from the APEngine.
		 */
		public function cleanup():void {
			sprite.graphics.clear();
			for (var i:int = 0; i < sprite.numChildren; i++) {
				sprite.removeChildAt(i);
			}
		}
		
		
		/**
		 * For performance, fixed Particles and SpringConstraints don't have their <code>paint()</code>
		 * method called in order to avoid unnecessary redrawing. A SpringConstraint is considered
		 * fixed if its two connecting Particles are fixed. Setting this property to <code>true</code>
		 * forces <code>paint()</code> to be called if this Particle or SpringConstraint <code>fixed</code>
		 * property is true. If you are rotating a fixed Particle or SpringConstraint then you would set 
		 * it's repaintFixed property to true. This property has no effect if a Particle or 
		 * SpringConstraint is not fixed.
		 */
		public final function get alwaysRepaint():Boolean {
			return _alwaysRepaint;
		}
		
		
		/**
		 * @private
		 */
		public final function set alwaysRepaint(b:Boolean):void {
			_alwaysRepaint = b;
		}	
		
				
		/**
		 * The visibility of the item. 
		 */	
		public function get visible():Boolean {
			return _visible;
		}
		
		
		/**
		 * @private
		 */			
		public function set visible(v:Boolean):void {
			_visible = v;
			sprite.visible = v;
		}


		/**
		 * Sets the solidity of the item. If an item is not solid, then other items colliding
		 * with it will not respond to the collision. This property differs from 
		 * <code>collidable</code> in that you can still test for collision events if
		 * an item's <code>collidable</code> property is true and its <code>solid</code>
		 * property is false. 
		 * 
		 * <p>
		 * The <code>collidable</code> property takes precidence over the <code>solid</code>
		 * property if <code>collidable</code> is set to false. That is, if <code>collidable</code>
		 * is false, it won't matter if <code>solid</code> is set to true or false.
		 * </p>
		 * 
		 * <p>
		 * If you don't need to check for collision events, using <code>collidable</code>
		 * is much more efficient. Always use <code>collidable</code> unless you need to
		 * handle collision events.
		 * </p>
		 */	
		public function get solid():Boolean {
			return _solid;
		}
		
		
		/**
		 * @private
		 */			
		public function set solid(s:Boolean):void {
			_solid = s;
		}


		/**
		 * Sets the line and fill of this Item.
		 */ 		
		public function setStyle(
				lineThickness:Number=0, lineColor:uint=0x000000, lineAlpha:Number=1,
				fillColor:uint=0xffffff, fillAlpha:Number=1):void {
			
			setLine(lineThickness, lineColor, lineAlpha);		
			setFill(fillColor, fillAlpha);		
		}		
		
		
		/**
		 * Sets the style of the line for this Item. 
		 */ 
		public function setLine(thickness:Number=0, color:uint=0x000000, alpha:Number=1):void {
			lineThickness = thickness;
			lineColor = color;
			lineAlpha = alpha;
		}
			
			
		/**
		 * Sets the style of the fill for this Item. 
		 */ 
		public function setFill(color:uint=0xffffff, alpha:Number=1):void {
			fillColor = color;
			fillAlpha = alpha;
		}
		
		
		/**
		 * Provides a Sprite to use as a container for drawing or adding children. When the
		 * sprite is requested for the first time it is automatically added to the global
		 * container in the APEngine class.
		 */	
		public function get sprite():Sprite {
			
			if (_sprite != null) return _sprite;
			
			if (APEngine.container == null) {
				throw new Error("The container property of the APEngine class has not been set");
			}
			
			_sprite = new Sprite();
			APEngine.container.addChild(_sprite);
			return _sprite;
		}	
	}
}
