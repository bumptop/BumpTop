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

- scale the post collision velocity by both the position *and* mass of each particle. 
  currently only the position and average inverse mass is used. as with the velocity,
  it might be a problem since the contact point is not available when the mass is 
  needed.
- review all p1 p2 getters (eg get mass). can it be stored instead of computed everytime?
- consider if the API should let the user set the SCP's properties directly. elasticity, 
  friction, mass, etc are all inherited from the attached particles
- consider a more accurate velocity getter. should use a parameterized value
  to scale the velocity relative to the contact point. one problem is the velocity is
  needed before the contact point is established.
- setCorners should be revisited.
- getContactPointParam should probably belong to the rectangleparticle and circleparticle  
  classes. also the functions respective to each, for better OOD
- clean up resolveCollision with submethods
- this class is internal, why are the methods public?
*/

package org.cove.ape {
	
	import flash.display.Sprite;
	import flash.display.DisplayObject;
	
	internal class SpringConstraintParticle extends RectangleParticle {
		
		internal var parent:SpringConstraint;
		
		private var p1:AbstractParticle;
		private var p2:AbstractParticle;
		
		private var avgVelocity:APEVector;
		private var lambda:APEVector;
		private var scaleToLength:Boolean;
		
		private var rca:APEVector;
		private var rcb:APEVector;
		private var s:Number;
		
		private var _rectScale:Number;
		private var _rectHeight:Number;
		private var _fixedEndLimit:Number;
				
		public function SpringConstraintParticle(
				p1:AbstractParticle, 
				p2:AbstractParticle, 
				p:SpringConstraint, 
				rectHeight:Number, 
				rectScale:Number,
				scaleToLength:Boolean) {
			
			super(0,0,0,0,0,false);
			
			this.p1 = p1;
			this.p2 = p2;
			
			lambda = new APEVector(0,0);
			avgVelocity = new APEVector(0,0);
			
			parent = p;
			this.rectScale = rectScale;
			this.rectHeight = rectHeight;
			this.scaleToLength = scaleToLength;
			
			fixedEndLimit = 0;
			rca = new APEVector();
			rcb = new APEVector();
		}
		
		
			
		internal function set rectScale(s:Number):void {
			_rectScale = s;
		}
		
		
		/**
		 * @private
		 */		
		internal function get rectScale():Number {
			return _rectScale;
		}
		
		
		internal function set rectHeight(r:Number):void {
			_rectHeight = r;
		}
		
		
		/**
		 * @private
		 */	
		internal function get rectHeight():Number {
			return _rectHeight;
		}

		
		/**
		 * For cases when the SpringConstraint is both collidable and only one of the
		 * two end particles are fixed, this value will dispose of collisions near the
		 * fixed particle, to correct for situations where the collision could never be
		 * resolved.
		 */	
		internal function set fixedEndLimit(f:Number):void {
			_fixedEndLimit = f;
		}
		
		
		/**
		 * @private
		 */	
		internal function get fixedEndLimit():Number {
			return _fixedEndLimit;
		}
	
	
		/**
		 * returns the average mass of the two connected particles
		 */
		public override function get mass():Number {
			return (p1.mass + p2.mass) / 2; 
		}
		
		
		/**
		 * returns the average elasticity of the two connected particles
		 */
		public override function get elasticity():Number {
			return (p1.elasticity + p2.elasticity) / 2; 
		}
		
		
		/**
		 * returns the average friction of the two connected particles
		 */
		public override function get friction():Number {
			return (p1.friction + p2.friction) / 2; 
		}
		
		
		/**
		 * returns the average velocity of the two connected particles
		 */
		public override function get velocity():APEVector {
			var p1v:APEVector =  p1.velocity;
			var p2v:APEVector =  p2.velocity;
			
			avgVelocity.setTo(((p1v.x + p2v.x) / 2), ((p1v.y + p2v.y) / 2));
			return avgVelocity;
		}	
		
		
		public override function init():void {
			if (displayObject != null) {
				initDisplay();
			} else {
				var inner:Sprite = new Sprite();
				parent.sprite.addChild(inner);
				inner.name = "inner";
							
				var w:Number = parent.currLength * rectScale;
				var h:Number = rectHeight;
				
				inner.graphics.clear();
				inner.graphics.lineStyle(
						parent.lineThickness, parent.lineColor, parent.lineAlpha);
				inner.graphics.beginFill(parent.fillColor, parent.fillAlpha);
				inner.graphics.drawRect(-w/2, -h/2, w, h);
				inner.graphics.endFill();
			}
			paint();
		}

		
		public override function paint():void {
			
			var c:APEVector = parent.center;
			var s:Sprite = parent.sprite;
			
			if (scaleToLength) {
				s.getChildByName("inner").width = parent.currLength * rectScale;
			} else if (displayObject != null) {
				s.getChildByName("inner").width = parent.restLength * rectScale;
			}
			s.x = c.x; 
			s.y = c.y;
			s.rotation = parent.angle;
		}
		
		
		/**
		 * @private
		 */
		internal override function initDisplay():void {
			displayObject.x = displayObjectOffset.x;
			displayObject.y = displayObjectOffset.y;
			displayObject.rotation = displayObjectRotation;
			
			var inner:Sprite = new Sprite();
			inner.name = "inner";
			
			inner.addChild(displayObject);
			parent.sprite.addChild(inner);
		}	
		
				
	   /**
		 * @private
		 * returns the average inverse mass.
		 */		
		public override function get invMass():Number {
			if (p1.fixed && p2.fixed) return 0;
			return 1 / ((p1.mass + p2.mass) / 2);  
		}
		
		
		/**
		 * Returns the value of the parent SpringConstraint <code>fixed</code> property.
		 */
		public override function get fixed():Boolean {
			return parent.fixed;
		}
		
		
		/**
		 * called only on collision
		 */
		internal function updatePosition():void {
			var c:APEVector = parent.center;
			curr.setTo(c.x, c.y);
			
			width = (scaleToLength) ? 
					parent.currLength * rectScale : 
					parent.restLength * rectScale;
			height = rectHeight;
			radian = parent.radian;
		}
		
			
		public override function resolveCollision(mtd:APEVector, vel:APEVector, n:APEVector, 
				d:Number, o:int, p:AbstractParticle):void {
				
			testParticleEvents(p);
			if (fixed || ! p.solid) return;
			
			var t:Number = getContactPointParam(p);
			var c1:Number = (1 - t);
			var c2:Number = t;
			
			// if one is fixed then move the other particle the entire way out of 
			// collision. also, dispose of collisions at the sides of the scp. The higher
			// the fixedEndLimit value, the more of the scp not be effected by collision. 
			if (p1.fixed) {
				if (c2 <= fixedEndLimit) return;
				lambda.setTo(mtd.x / c2, mtd.y / c2);
				p2.curr.plusEquals(lambda);
				p2.velocity = vel;

			} else if (p2.fixed) {
				if (c1 <= fixedEndLimit) return;
				lambda.setTo(mtd.x / c1, mtd.y / c1);
				p1.curr.plusEquals(lambda);
				p1.velocity = vel;		

			// else both non fixed - move proportionally out of collision
			} else { 
				var denom:Number = (c1 * c1 + c2 * c2);
				if (denom == 0) return;
				lambda.setTo(mtd.x / denom, mtd.y / denom);
			
				p1.curr.plusEquals(lambda.mult(c1));
				p2.curr.plusEquals(lambda.mult(c2));
			
				// if collision is in the middle of SCP set the velocity of both end 
				// particles
				if (t == 0.5) {
					p1.velocity = vel;
					p2.velocity = vel;
				
				// otherwise change the velocity of the particle closest to contact
				} else {
					var corrParticle:AbstractParticle = (t < 0.5) ? p1 : p2;
					corrParticle.velocity = vel;
				}
			}
		}
		
		
		/**
		 * given point c, returns a parameterized location on this SCP. Note
		 * this is just treating the SCP as if it were a line segment (ab).
		 */
		private function closestParamPoint(c:APEVector):Number {
			var ab:APEVector = p2.curr.minus(p1.curr);
			var t:Number = (ab.dot(c.minus(p1.curr))) / (ab.dot(ab));
			return MathUtil.clamp(t, 0, 1);
		}
	
	
		/**
		 * returns a contact location on this SCP expressed as a parametric value in [0,1]
		 */
		private function getContactPointParam(p:AbstractParticle):Number {
			
			var t:Number;
			
			if (p is CircleParticle)  {
				t = closestParamPoint(p.curr);
			} else if (p is RectangleParticle) {
					
				// go through the sides of the colliding rectangle as line segments
				var shortestIndex:Number;
				var paramList:Array = new Array(4);
				var shortestDistance:Number = Number.POSITIVE_INFINITY;
				
				for (var i:int = 0; i < 4; i++) {
					setCorners(p as RectangleParticle, i);
					
					// check for closest points on SCP to side of rectangle
					var d:Number = closestPtSegmentSegment();
					if (d < shortestDistance) {
						shortestDistance = d;
						shortestIndex = i;
						paramList[i] = s;
					}
				}
				t = paramList[shortestIndex];
			}
			return t;
		}
		
		
		/**
		 * 
		 */
		private function setCorners(r:RectangleParticle, i:int):void {
		
			var rx:Number = r.curr.x;
			var ry:Number = r.curr.y;
			
			var axes:Array = r.axes;
			var extents:Array = r.extents;
			
			var ae0_x:Number = axes[0].x * extents[0];
			var ae0_y:Number = axes[0].y * extents[0];
			var ae1_x:Number = axes[1].x * extents[1];
			var ae1_y:Number = axes[1].y * extents[1];
			
			var emx:Number = ae0_x - ae1_x;
			var emy:Number = ae0_y - ae1_y;
			var epx:Number = ae0_x + ae1_x;
			var epy:Number = ae0_y + ae1_y;
			
			
			if (i == 0) {
				// 0 and 1
				rca.x = rx - epx;
				rca.y = ry - epy;
				rcb.x = rx + emx;
				rcb.y = ry + emy;
			
			} else if (i == 1) {
				// 1 and 2
				rca.x = rx + emx;
				rca.y = ry + emy;
				rcb.x = rx + epx;
				rcb.y = ry + epy;
				
			} else if (i == 2) {
				// 2 and 3
				rca.x = rx + epx;
				rca.y = ry + epy;
				rcb.x = rx - emx;
				rcb.y = ry - emy;
				
			} else if (i == 3) {
				// 3 and 0
				rca.x = rx - emx;
				rca.y = ry - emy;
				rcb.x = rx - epx;
				rcb.y = ry - epy;
			}
		}
		
		
		/**
		 * pp1-pq1 will be the SCP line segment on which we need parameterized s. 
		 */
		private function closestPtSegmentSegment():Number {
			
			var pp1:APEVector = p1.curr;
			var pq1:APEVector = p2.curr;
			var pp2:APEVector = rca;
			var pq2:APEVector = rcb;
			
			var d1:APEVector = pq1.minus(pp1);
			var d2:APEVector = pq2.minus(pp2);
			var r:APEVector = pp1.minus(pp2);
		
			var t:Number;
			var a:Number = d1.dot(d1);
			var e:Number = d2.dot(d2);
			var f:Number = d2.dot(r);
			
			var c:Number = d1.dot(r);
			var b:Number = d1.dot(d2);
			var denom:Number = a * e - b * b;
			
			if (denom != 0.0) {
				s = MathUtil.clamp((b * f - c * e) / denom, 0, 1);
			} else {
				s = 0.5 // give the midpoint for parallel lines
			}
			t = (b * s + f) / e;
			 
			if (t < 0) {
				t = 0;
			 	s = MathUtil.clamp(-c / a, 0, 1);
			} else if (t > 0) {
			 	t = 1;
			 	s = MathUtil.clamp((b - c) / a, 0, 1);
			}
			 
			var c1:APEVector = pp1.plus(d1.mult(s));
			var c2:APEVector = pp2.plus(d2.mult(t));
			var c1mc2:APEVector = c1.minus(c2);
			return c1mc2.dot(c1mc2);
		}
	}
}