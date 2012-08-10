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

package tileui.ape.constraints
{
	import org.cove.ape.AbstractParticle;
	import org.cove.ape.SpringConstraint;
	
	import tileui.physics.constraints.ISpringContraint;
	import tileui.physics.particles.IPhysicsParticle;

	public class SpringConstraint extends org.cove.ape.SpringConstraint implements ISpringContraint
	{
		public function SpringConstraint(p1:IPhysicsParticle, p2:IPhysicsParticle, stiffness:Number=0, collidable:Boolean=false, rectHeight:Number=1, rectScale:Number=1, scaleToLength:Boolean=false)
		{
			super(p1 as AbstractParticle, p2 as AbstractParticle, stiffness, collidable, rectHeight, rectScale, scaleToLength);
		}
		
		public function get particle1():IPhysicsParticle {
			return super.p1 as IPhysicsParticle;
		}
		
		public function get particle2():IPhysicsParticle {
			return super.p2 as IPhysicsParticle;
		}
	}
}