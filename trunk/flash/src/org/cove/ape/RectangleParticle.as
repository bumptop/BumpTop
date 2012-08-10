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
- repaint when width or height changes, check radius in circle as well.
- review getProjection() for precomputing. radius can definitely be precomputed/stored
*/

package org.cove.ape {
	
	import flash.display.Graphics;
	
	/**
	 * A rectangular shaped particle. 
	 */ 
	public class RectangleParticle extends AbstractParticle {
	
		private var _extents:Array;
		public var _axes:Array;
		private var _radian:Number;
		
		
		/**
		 * @param x The initial x position.
		 * @param y The initial y position.
		 * @param width The width of this particle.
		 * @param height The height of this particle.
		 * @param rotation The rotation of this particle in radians.
		 * @param fixed Determines if the particle is fixed or not. Fixed particles
		 * are not affected by forces or collisions and are good to use as surfaces.
		 * Non-fixed particles move freely in response to collision and forces.
		 * @param mass The mass of the particle
		 * @param elasticity The elasticity of the particle. Higher values mean more elasticity.
		 * @param friction The surface friction of the particle. 
		 * <p>
		 * Note that RectangleParticles can be fixed but still have their rotation property 
		 * changed.
		 * </p>
		 */
		public function RectangleParticle (
				x:Number, 
				y:Number, 
				width:Number, 
				height:Number, 
				rotation:Number = 0, 
				fixed:Boolean = false,
				mass:Number = 1, 
				elasticity:Number = 0.3,
				friction:Number = 0) {
			
			super(x, y, fixed, mass, elasticity, friction);
			
			_extents = new Array(width/2, height/2);
			_axes = new Array(new APEVector(0,0), new APEVector(0,0));
			radian = rotation;
		}
		
		
		/**
		 * The rotation of the RectangleParticle in radians. For drawing methods you may 
		 * want to use the <code>angle</code> property which gives the rotation in
		 * degrees from 0 to 360.
		 * 
		 * <p>
		 * Note that while the RectangleParticle can be rotated, it does not have angular
		 * velocity. In otherwords, during collisions, the rotation is not altered, 
		 * and the energy of the rotation is not applied to other colliding particles.
		 * </p>
		 */
		public function get radian():Number {
			return _radian;
		}
		
		
		/**
		 * @private
		 */		
		public function set radian(t:Number):void {
			_radian = t;
			setAxes(t);
		}
			
		
		/**
		 * The rotation of the RectangleParticle in degrees. 
		 */
		public function get angle():Number {
			return radian * MathUtil.ONE_EIGHTY_OVER_PI;
		}


		/**
		 * @private
		 */		
		public function set angle(a:Number):void {
			radian = a * MathUtil.PI_OVER_ONE_EIGHTY;
		}
			
		
		/**
		 * Sets up the visual representation of this RectangleParticle. This method is called 
		 * automatically when an instance of this RectangleParticle's parent Group is added to 
		 * the APEngine, when  this RectangleParticle's Composite is added to a Group, or the 
		 * RectangleParticle is added to a Composite or Group.
		 */				
		public override function init():void {
			cleanup();
			if (displayObject != null) {
				initDisplay();
			} else {
			
				var w:Number = _extents[0] * 2;
				var h:Number = _extents[1] * 2;
				
				sprite.graphics.clear();
				sprite.graphics.lineStyle(lineThickness, lineColor, lineAlpha);
				sprite.graphics.beginFill(fillColor, fillAlpha);
				sprite.graphics.drawRect(-w/2, -h/2, w, h);
				sprite.graphics.endFill();
			}
			paint();
		}
		
		
		/**
		 * The default painting method for this particle. This method is called automatically
		 * by the <code>APEngine.paint()</code> method. If you want to define your own custom painting
		 * method, then create a subclass of this class and override <code>paint()</code>.
		 */	
		public override function paint():void {
			sprite.x = curr.x;
			sprite.y = curr.y;
			sprite.rotation = angle;
		}
		
		
		public function set width(w:Number):void {
			_extents[0] = w/2;
		}

		
		public function get width():Number {
			return _extents[0] * 2
		}


		public function set height(h:Number):void {
			_extents[1] = h / 2;
		}


		public function get height():Number {
			return _extents[1] * 2
		}
				
		
		/**
		 * @private
		 */	
		internal function get axes():Array {
			return _axes;
		}
		

		/**
		 * @private
		 */	
		internal function get extents():Array {
			return _extents;
		}
		
		
		/**
		 * @private
		 */	
		internal function getProjection(axis:APEVector):Interval {
			
			var abs0:Number = axis.dot(_axes[0]);
			if(abs0 < 0) abs0 = -abs0;
			
			var abs1:Number = axis.dot(_axes[1]);
			if(abs1 < 0) abs1 = -abs1;
			
			var radius:Number =
			    _extents[0] * abs0+
			    _extents[1] * abs1;
			
			var c:Number = samp.dot(axis);
			
			interval.min = c - radius;
			interval.max = c + radius;
			return interval;
		}


		/**
		 * 
		 */					
		private function setAxes(t:Number):void {
			var s:Number = Math.sin(t);
			var c:Number = Math.cos(t);
			
			_axes[0].x = c;
			_axes[0].y = s;
			_axes[1].x = -s;
			_axes[1].y = c;
		}
	}
}