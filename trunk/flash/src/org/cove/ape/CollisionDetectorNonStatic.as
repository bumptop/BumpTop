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
- multisampling is buggy. needs to be replaced with swept tests
- get rid of all the object testing and use the double dispatch pattern
- depths in obbvscircle should not be instantiated each time
*/

package org.cove.ape {
	

	internal final class CollisionDetectorNonStatic {	
		
		private var cpa:AbstractParticle;
		private var cpb:AbstractParticle;
		private var collNormal:APEVector;
		private var collDepth:Number;
		
		private var resolver:CollisionResolverNonStatic;
		
		public function CollisionDetectorNonStatic():void {
			resolver = new CollisionResolverNonStatic();
		}

		/**
		 * Tests the collision between two objects. This initial test determines
		 * the multisampling state of the two particles.
		 */	
		internal function test(objA:AbstractParticle, objB:AbstractParticle):void {
			
			if (objA._fixed && objB._fixed) return;
			
			if (objA.multisample == 0 && objB.multisample == 0) {
				normVsNorm(objA, objB);
							
			} else if (objA.multisample > 0 && objB.multisample == 0) {
				sampVsNorm(objA, objB);
				
			} else if (objB.multisample > 0 && objA.multisample == 0) {
				sampVsNorm(objB, objA);

			} else if (objA.multisample == objB.multisample) {
				sampVsSamp(objA, objB);

			} else {
				normVsNorm(objA, objB);
			}
		}
		
		
		/**
		 * default test for two non-multisampled particles
		 */
		 
		 private var lastObjA:AbstractParticle;
		 private var lastObjB:AbstractParticle;
		 
		private function normVsNorm(
				objA:AbstractParticle, objB:AbstractParticle):Boolean {
			/*
			if(lastObjA != objA) {
				lastObjA = objA;
				objA.samp.copy(objA.curr);
			}
			
			if(lastObjB != objB) {
				lastObjB = objB;
				objB.samp.copy(objB.curr);
			}*/
			
			objA.samp.copy(objA.curr);
			objB.samp.copy(objB.curr);
			
			if (testTypes(objA, objB)) {
				//CollisionResolver.resolve(cpa, cpb, collNormal, collDepth);
				resolver.resolve(cpa, cpb, collNormal, collDepth);
				return true;
			}
			return false
		}
		
		
		/**
		 * Tests two particles where one is multisampled and the other is not. Let objectA
		 * be the multisampled particle.
		 */
		private function sampVsNorm(
				objA:AbstractParticle, objB:AbstractParticle):void {
			
			if (normVsNorm(objA,objB)) return;
			
			var s:Number = 1 / (objA.multisample + 1); 
			var t:Number = s;
			
			for (var i:int = 0; i <= objA.multisample; i++) {
				objA.samp.setTo(objA.prev.x + t * (objA.curr.x - objA.prev.x), 
								objA.prev.y + t * (objA.curr.y - objA.prev.y));
				
				if (testTypes(objA, objB)) {
					//CollisionResolver.resolve(cpa, cpb, collNormal, collDepth);
					resolver.resolve(cpa, cpb, collNormal, collDepth);
					return;
				}
				t += s;
			}
		}


		/**
		 * Tests two particles where both are of equal multisample rate
		 */		
		private function sampVsSamp(
				objA:AbstractParticle, objB:AbstractParticle):void {
			
			if (normVsNorm(objA,objB)) return;
			
			var s:Number = 1 / (objA.multisample + 1); 
			var t:Number = s;
			
			for (var i:int = 0; i <= objA.multisample; i++) {
				
				objA.samp.setTo(objA.prev.x + t * (objA.curr.x - objA.prev.x), 
								objA.prev.y + t * (objA.curr.y - objA.prev.y));
				
				objB.samp.setTo(objB.prev.x + t * (objB.curr.x - objB.prev.x), 
								objB.prev.y + t * (objB.curr.y - objB.prev.y));
				
				if (testTypes(objA, objB)) {
					//CollisionResolver.resolve(cpa, cpb, collNormal, collDepth);
					resolver.resolve(cpa, cpb, collNormal, collDepth);
					return;	
				} 
				t += s;
			}
		}
		
		
		/**
		 * Tests collision based on primitive type.
		 */	
		private function testTypes(
				objA:AbstractParticle, objB:AbstractParticle):Boolean {	
			
			if (objA is CircleParticle && objB is CircleParticle) {
				return testCirclevsCircle(objA as CircleParticle, objB as CircleParticle);
				
			} else if (objA is RectangleParticle && objB is CircleParticle) {
				return testOBBvsCircle(objA as RectangleParticle, objB as CircleParticle);
				
			} else if (objA is CircleParticle && objB is RectangleParticle)  {
				return testOBBvsCircle(objB as RectangleParticle, objA as CircleParticle);
				
			} else if (objA is RectangleParticle && objB is RectangleParticle) {
				return testOBBvsOBB(objA as RectangleParticle, objB as RectangleParticle);
			
			}
			
			return false;
		}
	
	
		/**
		 * Tests the collision between two RectangleParticles (aka OBBs). If there is a 
		 * collision it determines its axis and depth, and then passes it off to the 
		 * CollisionResolver for handling.
		 */
		private function testOBBvsOBB(
				ra:RectangleParticle, rb:RectangleParticle):Boolean {
			
			collDepth = Number.POSITIVE_INFINITY;
			
			for (var i:int = 0; i < 2; i++) {
		
			    var axisA:APEVector = ra.axes[i];
			    var depthA:Number = testIntervals(
			    		ra.getProjection(axisA), rb.getProjection(axisA));
			    if (depthA == 0) return false;
				
			    var axisB:APEVector = rb.axes[i];
			    var depthB:Number = testIntervals(
			    		ra.getProjection(axisB), rb.getProjection(axisB));
			    if (depthB == 0) return false;
			    
			    //var absA:Number = Math.abs(depthA);
			    //var absB:Number = Math.abs(depthB);
			    
			    //faster than Math.abs
			    var absA:Number = depthA;
			    if(absA < 0) absA = -absA;
			    
			     var absB:Number = depthB;
			    if(absB < 0) absB = -absB;
			    
			    var absCollDepth:Number = collDepth;
			    if(absCollDepth < 0) absCollDepth = -absCollDepth;
			    
			    
			    if (absA < absCollDepth || absB < absCollDepth) {
			    	var altb:Boolean = absA < absB;
			    	collNormal = altb ? axisA : axisB;
			    	collDepth = altb ? depthA : depthB;
			    }
			}
			
			cpa = ra;
			cpb = rb
			return true;
		}		
	
	
		/**
		 * Tests the collision between a RectangleParticle (aka an OBB) and a 
		 * CircleParticle. If there is a collision it determines its axis and depth, and 
		 * then passes it off to the CollisionResolver.
		 */
		private function testOBBvsCircle(
				ra:RectangleParticle, ca:CircleParticle):Boolean {
			 
			collDepth = Number.POSITIVE_INFINITY;
			var depths:Array = new Array(2);
			
			// first go through the axes of the rectangle
			for (var i:int = 0; i < 2; i++) {
	
				var boxAxis:APEVector = ra._axes[i];
				var depth:Number = testIntervals(
						ra.getProjection(boxAxis), ca.getProjection(boxAxis));
				if (depth == 0) return false;
	
				var absDepth:Number = depth;
			    if(absDepth < 0) absDepth = -absDepth;
			    
				var absCollDepth:Number = collDepth;
			    if(absCollDepth < 0) absCollDepth = -absCollDepth;
			    
	
				if (absDepth < absCollDepth) {
					collNormal = boxAxis;
					collDepth = depth;
				}
				depths[i] = depth;
			}	
			
			// determine if the circle's center is in a vertex region
			var r:Number = ca._radius;
			
			var absA:Number = depths[0];
			if(absA < 0) absA = -absA;
			    
			var absB:Number = depths[1];
			if(absB < 0) absB = -absB;
			    
			if (absA < r && absB < r) {
	
				var vertex:APEVector = closestVertexOnOBB(ca.samp, ra);
	
				// get the distance from the closest vertex on rect to circle center
				collNormal = vertex.minus(ca.samp);
				var mag:Number = collNormal.magnitude();
				collDepth = r - mag;
	
				if (collDepth > 0) {
					// there is a collision in one of the vertex regions
					collNormal.divEquals(mag);
				} else {
					// ra is in vertex region, but is not colliding
					return false;
				}
			}
			
			cpa = ra;
			cpb = ca
			return true;
		}
	
	
		/**
		 * Tests the collision between two CircleParticles. If there is a collision it 
		 * determines its axis and depth, and then passes it off to the CollisionResolver
		 * for handling.
		 */	
		private function testCirclevsCircle(
				ca:CircleParticle, cb:CircleParticle):Boolean {
			
			var depthX:Number = testIntervals(ca.getIntervalX(), cb.getIntervalX());
			if (depthX == 0) return false;
			
			var depthY:Number = testIntervals(ca.getIntervalY(), cb.getIntervalY());
			if (depthY == 0) return false;
			
			collNormal = ca.samp.minus(cb.samp);
			var mag:Number = collNormal.magnitude();
			collDepth = (ca._radius + cb._radius) - mag;
			
			if (collDepth > 0) {
				collNormal.divEquals(mag);
				cpa = ca;
				cpb = cb
				return true;
			}
			return false;
		}
	
	
		/**
		 * Returns 0 if intervals do not overlap. Returns smallest depth if they do.
		 */
		private function testIntervals(
				intervalA:Interval, intervalB:Interval):Number {
			
			if (intervalA.max < intervalB.min) return 0;
			if (intervalB.max < intervalA.min) return 0;
			
			var lenA:Number = intervalB.max - intervalA.min;
			var lenB:Number = intervalB.min - intervalA.max;
			
			var absA:Number = lenA;
			if (absA < 0)  absA = -absA;
			
			var absB:Number = lenB;
			if (absB < 0)  absB = -absB;
			
			return (absA < absB) ? lenA : lenB;
			
			//return (Math.abs(lenA) < Math.abs(lenB)) ? lenA : lenB;
		}
		
		
		/**
		 * Returns the location of the closest vertex on r to point p
		 */
	 	private function closestVertexOnOBB(p:APEVector, r:RectangleParticle):APEVector {
	
			var d:APEVector = p.minus(r.samp);
			var q:APEVector = new APEVector(r.samp.x, r.samp.y);
	
			for (var i:int = 0; i < 2; i++) {
				var dist:Number = d.dot(r.axes[i]);
	
				if (dist >= 0) dist = r.extents[i];
				else if (dist < 0) dist = -r.extents[i];
	
				q.plusEquals(r.axes[i].mult(dist));
			}
			return q;
		}
	}
}
