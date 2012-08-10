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

package tileui.ape
{
	import flash.events.Event;
	
	import mx.core.UIComponent;
	
	import org.cove.ape.APEngine;
	import org.cove.ape.APEVector;
	
	import tileui.ape.groups.Group;
	import tileui.ape.particles.RectangleParticle;
	import tileui.ape.particles.TileUIParticle;
	import tileui.physics.groups.IPhysicsGroup;
	
	public class APEComponent extends UIComponent
	{
		
		private var leftBound:RectangleParticle;
		private var rightBound:RectangleParticle;
		private var topBound:RectangleParticle;
		private var bottomBound:RectangleParticle;
		
		public var defaultGroup:IPhysicsGroup;

		public function init():void {
			
			if(APEngine.groups == null) {
				APEngine.init(1/4);
				
				APEngine.container = this;
		       	APEngine.damping = .95;
		 	}

	        defaultGroup = new Group();
	        defaultGroup.collideInternal = true;
	       
	       
	        leftBound = new RectangleParticle(10,10,10,10,0,true, 1, .1, .01);
	        leftBound.alwaysRepaint = true;
	        rightBound = new RectangleParticle(30,10,10,10,0,true, 1, .1, .01);
	        rightBound.alwaysRepaint = true;
	        topBound = new RectangleParticle(60,10,10,10,0,true, 1, .1, .01);
	        topBound.alwaysRepaint = true;
	        bottomBound = new RectangleParticle(80, 10, 10, 10,0,true, 1, .1, .01);
	        bottomBound.alwaysRepaint = true;
	        
	        defaultGroup.addPhysicsParticle(leftBound);
	        defaultGroup.addPhysicsParticle(rightBound);
	        defaultGroup.addPhysicsParticle(topBound);
	        defaultGroup.addPhysicsParticle(bottomBound);
	        
	        APEngine.addGroup(defaultGroup as org.cove.ape.Group);
	        
	        invalidateDisplayList();
	        
			this.addEventListener(Event.ENTER_FRAME, run);
		}
		
		public function removeParticle(particle:TileUIParticle):void {
			if(particle.group) {
				particle.group.removePhysicsParticle(particle);
			
				if(particle.group.physicsParticles.length == 0) {
					APEngine.removeGroup(particle.group as org.cove.ape.Group);
				}
			}
			else {
				defaultGroup.removePhysicsParticle(particle);
			}
		}
		
		
		public function addParticle(particle:TileUIParticle):void {
			particle.parentUIContainer = this;
			
			if(APEngine.groups.indexOf(defaultGroup) == -1) {
				APEngine.addGroup(defaultGroup as org.cove.ape.Group);
			}
			
			particle.group = defaultGroup;
			
			defaultGroup.addPhysicsParticle(particle);
			
		}
		
		private function run(event:Event=null):void {
			APEngine.step();
			
			if(this.visible) {
				APEngine.paint();
			}
		}
		
		override protected function updateDisplayList(unscaledWidth:Number, unscaledHeight:Number):void {
			super.updateDisplayList(unscaledWidth, unscaledHeight);
			
			if(!leftBound) return;
			
			leftBound.width = 500;
			leftBound.height = unscaledHeight + 500;
			leftBound.position = new APEVector(-leftBound.width/2,unscaledHeight/2 );
			
			rightBound.width = 500;
			rightBound.height = unscaledHeight + 500;
			rightBound.position = new APEVector(unscaledWidth+rightBound.width/2,unscaledHeight/2 );
			
			topBound.height = 500;
			topBound.width = unscaledWidth + 500;
			topBound.position = new APEVector(unscaledWidth/2,-topBound.height/2);
			
			bottomBound.height = 500;
			bottomBound.width = unscaledWidth + 500;
			bottomBound.position = new APEVector(unscaledWidth/2,unscaledHeight+bottomBound.height/2);
			
			leftBound.init();
			rightBound.init();
			topBound.init();
			bottomBound.init();	
		}
	}
}