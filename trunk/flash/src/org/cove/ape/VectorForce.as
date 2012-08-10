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
		
	/**
	 * A force represented by a 2D vector. 
	 */
	public class VectorForce implements IForce {
	
		private var fvx:Number;
		private var fvy:Number;	
		
		private var value:APEVector;	
		private var scaleMass:Boolean;
		
		
		public function VectorForce(useMass:Boolean, vx:Number, vy:Number) {
			fvx = vx;
			fvy = vy;
			scaleMass = useMass;
			value = new APEVector(vx, vy);
		}
				
		
		public function set vx(x:Number):void {
			fvx = x;
			value.x = x;
		}
		
		
		public function set vy(y:Number):void {
			fvy = y;
			value.y = y;
		}
		
		
		public function set useMass(b:Boolean):void {
			scaleMass = b
		}
		
		
		public function getValue(invmass:Number):APEVector {
			if (scaleMass) { 
				value.setTo(fvx * invmass, fvy * invmass);
			}
			return value;
		}
	}
}