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
- collidible SpringConstraints should have their own collection controlled from within the
  add/remove constraint methods here -- so collision checks dont involve non-collidable
  constraints.
- need a removeForces method
- container should be automatic, but settable
- globally, change all internal getters to directly access properties, for performance.
  its better to break encapsulation within the library (not the public interface),
  rather than suffer the performance hit -- although it would be worth it to review
  just how slow the getters are in as3
*/

package org.cove.ape {
	
	import flash.display.DisplayObjectContainer;
	
	/**
	 * The main engine class. 
	 * 
	 */
	public final class APEngine {
		
		/**@private */
		internal static var forces:Array;
			
		public static var groups:Array;
		private static var numGroups:int;
		private static var timeStep:Number;
		
		private static var _damping:Number;
		private static var _container:DisplayObjectContainer;
		
		private static var _constraintCycles:int;
		private static var _constraintCollisionCycles:int;
		
	
		/**
		 * Initializes the engine. You must call this method prior to adding any 
		 * particles or constraints.
		 * 
		 * @param dt The delta time value for the engine. This parameter can be used --
		 * in conjunction with speed at which <code>APEngine.step()</code> is called -- 
		 * to change the speed of the simulation. Typical values are 1/3 or 1/4. Lower 
		 * values result in slower, but more accurate simulations, and higher ones 
		 * result in faster, less accurate ones. Note that this only applies to the 
		 * forces added to particles. If you do not add any forces, the <code>dt</code> 
		 * value won't matter.
		 */
		public static function init(dt:Number = 0.25):void {
			timeStep = dt * dt;
			
			numGroups = 0;
			groups = new Array();
			forces = new Array();
			
			_damping = 1;
			_constraintCycles = 0;
			_constraintCollisionCycles = 1;
		}


		/**
		 * The global damping. Values should be between 0 and 1. Higher numbers result
		 * in less damping. A value of 1 is no damping. A value of 0 won't allow any
		 * particles to move. The default is 1.
		 * 
		 * <p>
		 * Damping will slow down your simulation and make it more stable. If you find
		 * that your sim is "blowing up', try applying more damping. 
		 * </p>
		 * 
		 * @param d The damping value. Values should be >=0 and <=1.
		 */
		public static function get damping():Number {
			return _damping;
		}
		
		
		/**
		 * @private
		 */
		public static function set damping(d:Number):void {
			_damping = d;
		}


		/**
		 * Determines the number of times in a single <code>APEngine.step()</code> cycle that 
		 * the constraints have their positions corrected. Increasing this number can result in
		 * stiffer, more stable configurations of constraints, especially when they are in large
		 * complex arrangements. The trade off is that the higher you set this number the more 
		 * performance will suffer.
		 *
		 * <p>
		 * This setting differs from the <code>constraintCollisionCycles</code> property in that it
		 * only resolves constraints during a <code>APEngine.step()</code>. The default value
		 * is 0. Because this property doesn't correct for collisions, you should only use it when
		 * the collisions of an arrangement of particles and constraints are not an issue. If you 
		 * do set this value higher than the default of 0, then  <code>constraintCollisionCycles</code>
		 * should at least be 1, in order to check collisions one time during the 
		 * <code>APEngine.step()</code> cycle.
		 * </p>
		 * 
		 */
		public static function get constraintCycles():int {
			return _constraintCycles;
		}
		
		
		/**
		 * @private
		 */
		public static function set constraintCycles(numCycles:int):void {
			_constraintCycles = numCycles;
		}	
		
		
		/**
		 * 
		 * Determines the number of times in a single <code>APEngine.step()</code> cycle that
		 * the constraints have their positions corrected and particles in collision have their
		 * positions corrected. This can greatly increase stability and prevent breakthroughs,
		 * especially with large complex arrangements of constraints and particles. The larger
		 * this number, the more stable the simulation, at an expense of performance.
		 *
		 * <p> 
		 * This setting differs from the <code>constraintCycles</code> property in that it
		 * resolves both constraints and collisions during a <code>APEngine.step()</code>,
		 * as opposed to just the constraints. The default value is 1.
		 * </p>
		 */
		public static function get constraintCollisionCycles():int {
			return _constraintCollisionCycles;
		}
		
		
		/**
		 * @private
		 */
		public static function set constraintCollisionCycles(numCycles:int):void {
			_constraintCollisionCycles = numCycles;
		}			
		
		
		/**
		 * The default container used by the default painting methods of the particles and
		 * constraints. If you wish to use to the built in painting methods you must set 
		 * this first.
		 *
		 * @param s An instance of the Sprite class that will be used as the default 
		 * container.
		 */
		public static function get container():DisplayObjectContainer {
			return _container;
		}
			
		
		/**
		 * @private
		 */
		public static function set container(d:DisplayObjectContainer):void {
			_container = d;
		}
		
	
		/**
		 * Adds a force to all particles in the system. The forces added to the APEngine
		 * class are persistent - once a force is added it is continually applied each
		 * APEngine.step() cycle.
		 * 
		 * @param f A IForce object
		 */ 
		public static function addForce(f:IForce):void {
			forces.push(f);
		}
		

		/**
		 * Removes a force from the engine.
		 */
		public static function removeForce(f:IForce):void {
			var fpos:int = forces.indexOf(f);
			if (fpos == -1) return;
			forces.splice(fpos, 1);
		}
		
		
		/**
		 * Removes all forces from the engine.
		 */
		public static function removeAllForce():void {
			forces = new Array();
		}			
		
				
		/**
		 * Adds a Group to the engine.
		 */
		public static function addGroup(g:Group):void {
			groups.push(g);
			g.isParented = true;
			numGroups++;
			g.init();
		}
		
		
		/**
		 * Removes a Group from the engine.
		 */
		public static function removeGroup(g:Group):void {
			
			var gpos:int = groups.indexOf(g);
			if (gpos == -1) return;
			
			groups.splice(gpos, 1);
			g.isParented = false;
			numGroups--;
			g.cleanup();
		}
		
		
		/**
		 * The main step function of the engine. This method should be called
		 * continously to advance the simulation. The faster this method is 
		 * called, the faster the simulation will run. Usually you would call
		 * this in your main program loop. 
		 */			
		public static function step():void {
			integrate();
			for (var j:int = 0; j < _constraintCycles; j++) {
				satisfyConstraints();
			}
			for (var i:int = 0; i < _constraintCollisionCycles; i++) {
				satisfyConstraints();
				checkCollisions();
			}
		}


		/**
		 * Calling this method will in turn call each Group's paint() method.
		 * Generally you would call this method after stepping the engine in
		 * the main program cycle.
		 */			
		public static function paint():void {
			for (var j:int = 0; j < numGroups; j++) {
				var g:Group = groups[j];
				g.paint();
			}
		}
				

		private static function integrate():void {	
			for (var j:int = 0; j < numGroups; j++) {
				var g:Group = groups[j];
				g.integrate(timeStep);
			}
		}
	
		
		private static function satisfyConstraints():void {
			for (var j:int = 0; j < numGroups; j++) {
				var g:Group = groups[j];
				g.satisfyConstraints();
			}
		}


		private static function checkCollisions():void {
			for (var j:int = 0; j < numGroups; j++) {
				var g:Group = groups[j];
				g.checkCollisions();
			}
		}	
	}	
}
