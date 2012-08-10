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

package tileui.ape.particles
{
	import org.cove.ape.RectangleParticle;
	
	import tileui.physics.particles.IPhysicsParticle;

	public class RectangleParticle extends org.cove.ape.RectangleParticle implements IPhysicsParticle
	{
		public function RectangleParticle(x:Number, y:Number, width:Number, height:Number, rotation:Number=0, fixed:Boolean=false, mass:Number=1, elasticity:Number=0, friction:Number=0)
		{
			super(x, y, width, height, rotation, fixed, mass, elasticity, friction);
		}
		
	}
}