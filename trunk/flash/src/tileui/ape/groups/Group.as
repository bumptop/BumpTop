//
//  Copyright 2012 Google Inc. All Rights Reserved.
//  
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  
//      http://www.apache.org/licenses/LICENSE-2.0
//  
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//

package tileui.ape.groups
{
	import org.cove.ape.AbstractConstraint;
	import org.cove.ape.AbstractParticle;
	import org.cove.ape.Group;
	
	import tileui.physics.constraints.IPhysicsConstraint;
	import tileui.physics.groups.IPhysicsGroup;
	import tileui.physics.particles.IPhysicsParticle;

	public class Group extends org.cove.ape.Group implements IPhysicsGroup
	{
		public function Group(collideInternal:Boolean=false)
		{
			super(collideInternal);
		}
		
		override public function get collideInternal():Boolean
		{
			return super.collideInternal;
		}
		
		override public function set collideInternal(value:Boolean):void
		{
			super.collideInternal = value;
		}
		
		public function addPhysicsParticle(particle:IPhysicsParticle):void
		{
			super.addParticle(particle as AbstractParticle);
		}
		
		public function removePhysicsParticle(particle:IPhysicsParticle):void {
			super.removeParticle(particle as AbstractParticle);
		}
		
		public function get physicsParticles():Array {
			var tempArray:Array = [];
			for each(var particle:AbstractParticle in super.particles) {
				tempArray.push(particle);
			}
			return tempArray;
		}
		
		public function addPhysicsConstraint(constraint:IPhysicsConstraint):void {
			super.addConstraint(constraint as AbstractConstraint);
		}
		
		public function removePhysicsConstraint(constraint:IPhysicsConstraint):void {
			super.removeConstraint(constraint as AbstractConstraint);
		}
		
		override public function addCollidableList(list:Array):void {
			super.addCollidableList(list);
		}
		
	}
}