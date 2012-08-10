package org.cove.ape {
	
	import flash.events.Event;
	
	/**
	 * CollisionEvent objects are dispatched during a collision between two 
	 * AbstractItem (Particle or Constraint) objects. You can retrieve the 
	 * AbstractItem that has collided with the target object by using the 
	 * <code>collidingItem</code> property.
	 */
	public class CollisionEvent extends Event {
		
		/**
		 * Defines the value of the type property of a collide event object
		 */
		public static const COLLIDE:String = "collide";
		public static const FIRST_COLLIDE:String = "firstCollide";
		
		private var _collidingItem:AbstractItem;
		
		public function CollisionEvent(
				type:String, 
				bubbles:Boolean = false, 
				cancelable:Boolean = false,
				collidingItem:AbstractItem = null) {
			
			super(type, bubbles, cancelable);	
			_collidingItem = collidingItem;
		}
		
		
		/**
		 * Returns the AbstractItem (either a Constraint or Particle) object that has
		 * collided with the target object
		 */
		public function get collidingItem():AbstractItem {
		
			if (_collidingItem is SpringConstraintParticle) {
				var scp:SpringConstraintParticle = _collidingItem 
						as SpringConstraintParticle;
				return scp.parent;
			}
			return _collidingItem;
		}
	}
}

