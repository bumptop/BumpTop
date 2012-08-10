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
- Need removeForces method(s)
- Center and Position are the same, needs review. Also the comments need review.
- Should have alwaysRepaint functionality for Constraints, and bump up to AbstractItem
- See if there's anywhere where Vectors can be downgraded to simple Point classes
*/

package org.cove.ape {
	
	import flash.display.DisplayObject;
	import flash.utils.getQualifiedClassName;

	 	
	/**
	 * The abstract base class for all particles.
	 * 
	 * <p>
	 * You should not instantiate this class directly -- instead use one of the subclasses.
	 * </p>
	 */
	public class AbstractParticle extends AbstractItem {
		
		/** @private */
		public var curr:APEVector;
		/** @private */
		internal var prev:APEVector;
		/** @private */
		internal var samp:APEVector;
		/** @private */
		internal var interval:Interval;
		
		private var temp:APEVector;
		private var forces:APEVector;
		private var forceList:Array;
		private var collision:Collision;
		private var firstCollision:Boolean;
				
		private var _kfr:Number;
		private var _mass:Number;
		private var _invMass:Number;
		private var _friction:Number;
		
		public var _fixed:Boolean;
		public var _collidable:Boolean;
		
		private var _center:APEVector;
		public var multisample:int;
			
		private var damping:Number;
		
		/** 
		 * @private
		 */
		public function AbstractParticle (
				x:Number, 
				y:Number, 
				isFixed:Boolean, 
				mass:Number, 
				elasticity:Number,
				friction:Number) {
			
			if (getQualifiedClassName(this) == "org.cove.ape::AbstractParticle") {
				throw new ArgumentError("AbstractParticle can't be instantiated directly");
			}
			
			interval = new Interval(0,0);
			
			curr = new APEVector(x, y);
			prev = new APEVector(x, y);
			samp = new APEVector();
			temp = new APEVector();
			fixed = isFixed;
			
			forces = new APEVector();
			forceList = new Array();
			
			collision = new Collision(new APEVector(), new APEVector());
			collidable = true;
			firstCollision = false;
			
			this.mass = mass;
			this.elasticity = elasticity;
			this.friction = friction;
			
			setStyle();
			
			_center = new APEVector();
			multisample = 0;
			
			damping = APEngine.damping;
		}
	
		
		/**
		 * The mass of the particle. Valid values are greater than zero. By default, all particles
		 * have a mass of 1. The mass property has no relation to the size of the particle.
		 * 
		 * @throws ArgumentError ArgumentError if the mass is set less than zero. 
		 */
		public function get mass():Number {
			return _mass; 
		}
		
		
		/**
		 * @private
		 */
		public function set mass(m:Number):void {
			if (m <= 0) throw new ArgumentError("mass may not be set <= 0"); 
			_mass = m;
			_invMass = 1 / _mass;
		}	
	
		
		/**
		 * The elasticity of the particle. Standard values are between 0 and 1. 
		 * The higher the value, the greater the elasticity.
		 * 
		 * <p>
		 * During collisions the elasticity values are combined. If one particle's
		 * elasticity is set to 0.4 and the other is set to 0.4 then the collision will
		 * be have a total elasticity of 0.8. The result will be the same if one particle
		 * has an elasticity of 0 and the other 0.8.
		 * </p>
		 * 
		 * <p>
		 * Setting the elasticity to greater than 1 (of a single particle, or in a combined
		 * collision) will cause particles to bounce with energy greater than naturally 
		 * possible.
		 * </p>
		 */ 
		public function get elasticity():Number {
			return _kfr; 
		}
		
		
		/**
		 * @private
		 */
		public function set elasticity(k:Number):void {
			_kfr = k;
		}
		

		/**
		 * Determines the number of intermediate position steps during each collision test.
		 * Setting this number higher on fast moving particles can prevent 'tunneling' 
		 * -- when a particle moves so fast it misses collision with another particle.
		 * 
		 * <p>
		 * If two particles both have multisample levels greater than 0 then their 
		 * multisample levels must be equal to be tested correctly. For example, if one 
		 * particle has a multisample level of 4 and another has a multisample level of 5
		 * then the two particles will not be tested for multisampled collision. This is
		 * due to the unequal amount of steps in the collision test. 
		 * </p>
		 * 
		 * <p>
		 * Multisampling is currently buggy and should only be applied to particles that
		 * have high velocity. You can selectively apply multisampling by doing something
		 * like: 
		 * </p>
		 * 
		 * <p>
		 * <code>
		 * if (myparticle.velocity.magnitude() > 20) {
		 *     myparticle.multisample = 100;
		 * } else {
		 *     myparticle.multsample = 0;
		 * }
		 * </code>
		 * </p>
		 */ 
		 /*
		public function get multisample():int {
			return _multisample; 
		}
		*/
		
		/**
		 * @private
		 */
		 /*
		public function set multisample(m:int):void {
			_multisample = m;
		}*/
		
		
		/**
		 * Returns A Vector of the current location of the particle
		 */	
		public function get center():APEVector {
			_center.setTo(px, py)
			return _center;
		}
		
				
		/**
		 * The surface friction of the particle. Values must be in the range of 0 to 1.
		 * 
		 * <p>
		 * 0 is no friction (slippery), 1 is full friction (sticky).
		 * </p>
		 * 
		 * <p>
		 * During collisions, the friction values are summed, but are clamped between 1 
		 * and 0. For example, If two particles have 0.7 as their surface friction, then
		 * the resulting friction between the two particles will be 1 (full friction).
		 * </p>
		 *
		 * <p>
		 * There is a bug in the current release where colliding non-fixed particles with
		 * friction greater than 0 will behave erratically. A workaround is to only set 
		 * the friction of fixed particles.
		 * </p>
		 * 
		 * @throws ArgumentError ArgumentError if the friction is set less than zero or 
		 * greater than 1
		 */	
		public function get friction():Number {
			return _friction; 
		}
	
		
		/**
		 * @private
		 */
		public function set friction(f:Number):void {
			if (f < 0 || f > 1)  {
				throw new ArgumentError("Legal friction must be >= 0 and <=1");
			}
			_friction = f;
		}
		
		
		/**
		 * The fixed state of the particle. If the particle is fixed, it does not move in
		 * response to forces or collisions. Fixed particles are good for surfaces.
		 */
		public function get fixed():Boolean {
			return _fixed;
		}

 
		/**
		 * @private
		 */
		public function set fixed(f:Boolean):void {
			_fixed = f;
		}
		
		
		/**
		 * The position of the particle. Getting the position of the particle is useful
		 * for drawing it or testing it for some custom purpose. 
		 * 
		 * <p>
		 * When you get the <code>position</code> of a particle you are given a copy of 
		 * the current location. Because of this you cannot change the position of a 
		 * particle by altering the <code>x</code> and <code>y</code> components of the 
		 * Vector you have retrieved from the position property. You have to do something
		 * instead like: <code> position = new Vector(100,100)</code>, or you can use the
		 * <code>px</code> and <code>py</code> properties instead.
		 * </p>
		 * 
		 * <p>
		 * You can alter the position of a particle three ways: change its position, set
		 * its velocity, or apply a force to it. Setting the position of a non-fixed 
		 * particle is not the same as setting its fixed property to true. A particle held
		 * in place by its position will behave as if it's attached there by a 0 length
		 * spring constraint. 
		 * </p>
		 */
		public function get position():APEVector {
			return new APEVector(curr.x,curr.y);
		}
		
		
		/**
		 * @private
		 */
 		public function set position(p:APEVector):void {
			curr.copy(p);
			prev.copy(p);
		}


		/**
		 * The x position of this particle
		 */
		public function get px():Number {
			return curr.x;
		}


		/**
		 * @private
		 */
		public function set px(x:Number):void {
			curr.x = x;
			prev.x = x;	
		}


		/**
		 * The y position of this particle
		 */
		public function get py():Number {
			return curr.y;
		}


		/**
		 * @private
		 */
		public function set py(y:Number):void {
			curr.y = y;
			prev.y = y;	
		}
		
		
		/**
		 * The velocity of the particle. If you need to change the motion of a particle, 
		 * you should either use this property, or one of the addForce methods. Generally,
		 * the addForce methods are best for slowly altering the motion. The velocity 
		 * property is good for instantaneously setting the velocity, e.g., for 
		 * projectiles.
		 * 
		 */
		public function get velocity():APEVector {
			return curr.minus(prev);
		}
		
		
		/**
		 * @private
		 */	
		public function set velocity(v:APEVector):void {
			prev = curr.minus(v);	
		}
		
		
		/**
		 * Determines if the particle can collide with other particles or constraints.
		 * The default state is true.
		 */
		public function get collidable():Boolean {
			return _collidable;
		}
	
				
		/**
		 * @private
		 */		
		public function set collidable(b:Boolean):void {
			_collidable = b;
		}
		
		
		/**
		 * Assigns a DisplayObject to be used when painting this particle.
		 */ 
		public function setDisplay(d:DisplayObject, offsetX:Number=0, offsetY:Number=0,
				 rotation:Number=0):void {
			
			displayObject = d;
			displayObjectRotation = rotation;
			displayObjectOffset = new APEVector(offsetX, offsetY);
		}
		
		
		/**
		 * Adds a force to the particle. Using this method to a force directly to the
		 * particle will only apply that force for a single APEngine.step() cycle. 
		 * 
		 * @param f An IForce object.
		 */ 
		 public function addForce(f:IForce):void {
		 	forceList.push(f);
		 }
		
	
		/**
		 * The <code>update()</code> method is called automatically during the
		 * APEngine.step() cycle. This method integrates the particle.
		 */
		public function update(dt2:Number):void {
			
			if (_fixed) return;
			
			accumulateForces();
	
			temp.copy(curr);			
			var nv:APEVector = velocity;
			nv.plusEquals(forces.multEquals(dt2));
			curr.plusEquals(nv.multEquals(damping));
			prev.copy(temp);

			clearForces();
		}
		
		
		/**
		 * Resets the collision state of the particle. This value is used in conjuction
		 * with the CollisionEvent.FIRST_COLLISION event.
		 */	
		public function resetFirstCollision():void {
			firstCollision = false;
		}
		
		
		/**
		 * @private
		 */
		internal function initDisplay():void {
			displayObject.x = displayObjectOffset.x;
			displayObject.y = displayObjectOffset.y;
			displayObject.rotation = displayObjectRotation;
			sprite.addChild(displayObject);
		}	
		
			
		/**
		 * @private
		 */		
		internal function getComponents(collisionNormal:APEVector):Collision {
			var vel:APEVector = velocity;
			var vdotn:Number = collisionNormal.dot(vel);
			collision.vn = collisionNormal.mult(vdotn);
			collision.vt = vel.minus(collision.vn);	
			return collision;
		}
	
	
		/**
		 * @private
		 * 
		 * Make sure to align  the overriden versions of this method in
		 * WheelParticle
		 */	
		public function resolveCollision(mtd:APEVector, vel:APEVector, n:APEVector, d:Number,
				o:int, p:AbstractParticle):void {
					
			testParticleEvents(p);
			if (fixed || (! solid) || (! p.solid)) return;
			
			curr.copy(samp);
			curr.plusEquals(mtd);
			velocity = vel;
		}
		
		
		/**
		 * @private
		 */		
		internal function testParticleEvents(p:AbstractParticle):void {		
			
			if (hasEventListener(CollisionEvent.COLLIDE)) {
				dispatchEvent(new CollisionEvent(
						CollisionEvent.COLLIDE, false, false, p));
			}
			
			if (hasEventListener(CollisionEvent.FIRST_COLLIDE) && ! firstCollision) {
				firstCollision = true;
				dispatchEvent(new CollisionEvent(
						CollisionEvent.FIRST_COLLIDE, false, false, p));
			}
		}
		
			
		/**
		 * @private
		 */		
		public function get invMass():Number {
			return (fixed) ? 0 : _invMass; 
		}
		
		
		/**
		 * Accumulates both the particle forces and the global forces
		 */
		private function accumulateForces():void {
			
			var f:IForce;
			
			var len:int = forceList.length;
			for (var i:int = 0; i < len; i++) {
				f = forceList[i];
				forces.plusEquals(f.getValue(_invMass));
			}
			
			var globalForces:Array = APEngine.forces;
			len = globalForces.length;
			for (i = 0; i < len; i++) {
				f = globalForces[i];
				forces.plusEquals(f.getValue(_invMass));
			}
		}
		
		
		/**
		 * Clears out all forces on the particle
		 */
		private function clearForces():void {
			forceList.length = 0;
			forces.setTo(0,0);
		}
	}	
}