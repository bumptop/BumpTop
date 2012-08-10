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
- in rotateByRadian, radius should be cached if possible 
*/

package org.cove.ape {
	
	/**
	 * The Composite class can contain Particles, and Constraints. Composites can be added
	 * to a parent Group, along with Particles and Constraints.  Members of a Composite
	 * are not checked for collision with one another, internally.
	 */ 
	public class Composite extends AbstractCollection {
		
		private var delta:APEVector;
	
		
		public function Composite() {
			delta = new APEVector();
		}
	
	
		/**
		 * Rotates the Composite to an angle specified in radians, around a given center
		 */
		public function rotateByRadian(angleRadians:Number, center:APEVector):void {
			var p:AbstractParticle;
			var pa:Array = particles;
			var len:int = pa.length;
			for (var i:int = 0; i < len; i++) {
				p = pa[i];
				var radius:Number = p.center.distance(center);
				var angle:Number = getRelativeAngle(center, p.center) + angleRadians;
				p.px = (Math.cos(angle) * radius) + center.x;
				p.py = (Math.sin(angle) * radius) + center.y;
			}
		}  
		
		
		/**
		 * Rotates the Composite to an angle specified in degrees, around a given center
		 */
		public function rotateByAngle(angleDegrees:Number, center:APEVector):void {
			var angleRadians:Number = angleDegrees * MathUtil.PI_OVER_ONE_EIGHTY;
			rotateByRadian(angleRadians, center);
		}  
		

		/**
		 * The fixed state of the Composite. Setting this value to true or false will
		 * set all of this Composite's component particles to that value. Getting this 
		 * value will return false if any of the component particles are not fixed.
		 */	
		public function get fixed():Boolean {
			for (var i:int = 0; i < particles.length; i++) {
				if (! particles[i].fixed) return false;	
			}
			return true;
		}


		/**
		 * @private
		 */		
		public function set fixed(b:Boolean):void {
			for (var i:int = 0; i < particles.length; i++) {
				particles[i].fixed = b;	
			}
		}
		
		
		private function getRelativeAngle(center:APEVector, p:APEVector):Number {
			delta.setTo(p.x - center.x, p.y - center.y);
			return Math.atan2(delta.y, delta.x);
		}		
	}
}