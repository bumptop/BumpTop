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
- use vector methods in update()
*/

package org.cove.ape {
	
	internal class RimParticle {
		
		internal var curr:APEVector;
		internal var prev:APEVector;
	
		private var wr:Number;
		private var av:Number;
		private var sp:Number;
		private var maxTorque:Number;
		
		
		/**
		 * The RimParticle is really just a second component of the wheel model.
		 * The rim particle is simulated in a coordsystem relative to the wheel's 
		 * center, not in worldspace.
		 * 
		 * Origins of this code are from Raigan Burns, Metanet Software
		 */
		public function RimParticle(r:Number, mt:Number) {
	
			curr = new APEVector(r, 0);
			prev = new APEVector(0, 0);
			
			sp = 0; 
			av = 0;
			
			maxTorque = mt; 	
			wr = r;		
		}
		
		internal function get speed():Number {
			return sp;
		}
		
		internal function set speed(s:Number):void {
			sp = s;
		}
		
		internal function get angularVelocity():Number {
			return av;
		}
		
		internal function set angularVelocity(s:Number):void {
			av = s;
		}
		
		/**
		 * Origins of this code are from Raigan Burns, Metanet Software
		 */
		internal function update(dt:Number):void {
			
			//clamp torques to valid range
			sp = Math.max(-maxTorque, Math.min(maxTorque, sp + av));
	
			//apply torque
			//this is the tangent vector at the rim particle
			var dx:Number = -curr.y;
			var dy:Number =  curr.x;
	
			//normalize so we can scale by the rotational speed
			var len:Number = Math.sqrt(dx * dx + dy * dy);
			dx /= len;
			dy /= len;
	
			curr.x += sp * dx;
			curr.y += sp * dy;		
	
			var ox:Number = prev.x;
			var oy:Number = prev.y;
			var px:Number = prev.x = curr.x;		
			var py:Number = prev.y = curr.y;		
			
			curr.x += APEngine.damping * (px - ox);
			curr.y += APEngine.damping * (py - oy);	
	
			// hold the rim particle in place
			var clen:Number = Math.sqrt(curr.x * curr.x + curr.y * curr.y);
			var diff:Number = (clen - wr) / clen;
	
			curr.x -= curr.x * diff;
			curr.y -= curr.y * diff;
		}
	}
}



