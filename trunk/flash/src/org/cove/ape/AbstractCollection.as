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
- get sprite() is duplicated in AbstractItem. Should be in some parent class.
- checkCollisionsVsCollection and checkInternalCollisions methods use SpringConstraint. 
  it should be AbstractConstraint but the isConnectedTo method is in SpringConstraint.
- same deal with the paint() method here -- needs to test connected particles state 
  using SpringConstraint methods but should really be AbstractConstraint. need to clear up
  what an AbstractConstraint really means.
- would an explicit cast be more efficient in the paint() method here?
*/

package org.cove.ape {
	
	import flash.display.Sprite;
	import flash.utils.getQualifiedClassName;
	
	
	/**
	 * The abstract base class for all grouping classes. 
	 * 
	 * <p>
	 * You should not instantiate this class directly -- instead use one of the subclasses.
	 * </p>
	 */	
	public class AbstractCollection {
		
	
		private var _sprite:Sprite;
		private var _particles:Array;
		private var _constraints:Array;
		private var _isParented:Boolean;
		
		
		public function AbstractCollection() {
			if (getQualifiedClassName(this) == "org.cove.ape::AbstractCollection") {
				throw new ArgumentError("AbstractCollection can't be instantiated directly");
			}
			_isParented = false;
			_particles = new Array();
			_constraints = new Array();
			
		}
		
		
		/**
		 * The Array of all AbstractParticle instances added to the AbstractCollection
		 */
		public function get particles():Array {
			return _particles;
		}
		
		
		/**
		 * The Array of all AbstractConstraint instances added to the AbstractCollection
		 */	
		public function get constraints():Array {
			return _constraints;	
		}

		
		/**
		 * Adds an AbstractParticle to the AbstractCollection.
		 * 
		 * @param p The particle to be added.
		 */
		public function addParticle(p:AbstractParticle):void {
			particles.push(p);
			if (isParented) p.init();
		}
		
		
		/**
		 * Removes an AbstractParticle from the AbstractCollection.
		 * 
		 * @param p The particle to be removed.
		 */
		public function removeParticle(p:AbstractParticle):void {
			var ppos:int = particles.indexOf(p);
			if (ppos == -1) return;
			particles.splice(ppos, 1);
			p.cleanup();
		}
		
		
		/**
		 * Adds a constraint to the Collection.
		 * 
		 * @param c The constraint to be added.
		 */
		public function addConstraint(c:AbstractConstraint):void {
			constraints.push(c);
			if (isParented) c.init();
		}


		/**
		 * Removes a constraint from the Collection.
		 * 
		 * @param c The constraint to be removed.
		 */
		public function removeConstraint(c:AbstractConstraint):void {
			var cpos:int = constraints.indexOf(c);
			if (cpos == -1) return;
			constraints.splice(cpos, 1);
			c.cleanup();
		}
		
		private var detector:CollisionDetectorNonStatic;
		
		/**
		 * Initializes every member of this AbstractCollection by in turn calling 
		 * each members <code>init()</code> method.
		 */
		public function init():void {
			detector = new CollisionDetectorNonStatic();
				
			for (var i:int = 0; i < particles.length; i++) {
				particles[i].init();	
			}
			for (i = 0; i < constraints.length; i++) {
				constraints[i].init();
			}
		}
		
				
		/**
		 * paints every member of this AbstractCollection by calling each members
		 * <code>paint()</code> method.
		 */
		public function paint():void {
			
			var p:AbstractParticle;
			var len:int = _particles.length;
			for (var i:int = 0; i < len; i++) {
				p = _particles[i];
				if ((! p.fixed) || p.alwaysRepaint) p.paint();	
			}
			
			var c:SpringConstraint;
			len = _constraints.length;
			for (i = 0; i < len; i++) {
				c = _constraints[i];
				if ((! c.fixed) || c.alwaysRepaint) c.paint();
			}
		}
		
		
		/**
		 * Calls the <code>cleanup()</code> method of every member of this AbstractCollection.
		 * The cleanup() method is called automatically when an AbstractCollection is removed
		 * from its parent.
		 */
		public function cleanup():void {
			
			for (var i:int = 0; i < particles.length; i++) {
				particles[i].cleanup();	
			}
			for (i = 0; i < constraints.length; i++) {
				constraints[i].cleanup();
			}
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
		
	
		/**
		 * Returns an array of every particle and constraint added to the AbstractCollection.
		 */
		public function getAll():Array {
			return particles.concat(constraints);
		}	
		
		
		/**
		 * @private
		 */
		internal function get isParented():Boolean {
			return _isParented;
		}	


		/**
		 * @private
		 */		
		internal function set isParented(b:Boolean):void {
			_isParented = b;
		}	
		
								
		/**
		 * @private
		 */
		public function integrate(dt2:Number):void {
			var len:int = _particles.length;
			for (var i:int = 0; i < len; i++) {
				var p:AbstractParticle = _particles[i];
				p.update(dt2);	
			}
		}		
		
			
		/**
		 * @private
		 */
		internal function satisfyConstraints():void {
			var len:int = _constraints.length;
			for (var i:int = 0; i < len; i++) {
				var c:AbstractConstraint = _constraints[i];
				c.resolve();	
			}
		}			
		

		/**
		 * @private
		 */	
		 internal function checkInternalCollisions():void {
		 
			// every particle in this AbstractCollection
			var plen:int = _particles.length;
			for (var j:int = 0; j < plen; j++) {
				
				var pa:AbstractParticle = _particles[j];
				if (pa == null || ! pa._collidable) continue;
				
				// ...vs every other particle in this AbstractCollection
				for (var i:int = j + 1; i < plen; i++) {
					var pb:AbstractParticle = _particles[i];
					//if (pb != null && pb._collidable) CollisionDetector.test(pa, pb);
					if (pb != null && pb._collidable) detector.test(pa, pb);
				}
				
				// ...vs every other constraint in this AbstractCollection
				var clen:int = _constraints.length;
				for (var n:int = 0; n < clen; n++) {
					var c:SpringConstraint = _constraints[n];
					if (c != null && c._collidable && ! c.isConnectedTo(pa)) {
						c.scp.updatePosition();
						//CollisionDetector.test(pa, c.scp);
						detector.test(pa, c.scp);
					}
				}
			}
		}
	
	
		/**
		 * @private
		 */	
		internal function checkCollisionsVsCollection(ac:AbstractCollection):void {
			
			// every particle in this collection...
			var plen:int = _particles.length;
			for (var j:int = 0; j < plen; j++) {
				
				var pga:AbstractParticle = _particles[j];
				if (pga == null || ! pga.collidable) continue;
				
				// ...vs every particle in the other collection
				var acplen:int = ac.particles.length;
				for (var x:int = 0; x < acplen; x++) {
					var pgb:AbstractParticle = ac.particles[x];
					//if (pgb != null && pgb.collidable) CollisionDetector.test(pga, pgb);
					if (pgb != null && pgb.collidable) detector.test(pga, pgb);
				}
				// ...vs every constraint in the other collection
				var acclen:int = ac.constraints.length;
				for (x = 0; x < acclen; x++) {
					var cgb:SpringConstraint = ac.constraints[x];
					if (cgb != null && cgb.collidable && ! cgb.isConnectedTo(pga)) {
						cgb.scp.updatePosition();
						//CollisionDetector.test(pga, cgb.scp);
						detector.test(pga, cgb.scp);
					}
				}
			}
			
			// every constraint in this collection...
			var clen:int = _constraints.length;
			for (j = 0; j < clen; j++) {
				var cga:SpringConstraint = _constraints[j];
				if (cga == null || ! cga.collidable) continue;
				
				// ...vs every particle in the other collection
				acplen = ac.particles.length;
				for (var n:int = 0; n < acplen; n++) {
					pgb = ac.particles[n];
					if (pgb != null && pgb.collidable && ! cga.isConnectedTo(pgb)) {
						cga.scp.updatePosition();
						//CollisionDetector.test(pgb, cga.scp);
						detector.test(pgb, cga.scp);
					}
				}
			}
		}			
	}
}


