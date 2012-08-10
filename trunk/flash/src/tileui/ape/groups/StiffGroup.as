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
	import org.cove.ape.AbstractParticle;
	
	public class StiffGroup extends Group
	{
		public function StiffGroup():void {
			super(false);
		}
		
		public override function integrate(dt2:Number):void {
			var len:int = particles.length;
			
			var particle0:AbstractParticle = particles[0];
			
			if(!particle0) return;
			
			particle0.update(dt2);
			
			for (var i:int = 1; i < len; i++) {
				var particle:AbstractParticle = particles[i];
				particle.position = particle0.position;
				particle.update(dt2);	
			}
		}
		
	}
}