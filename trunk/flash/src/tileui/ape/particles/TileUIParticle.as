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
	import flash.display.DisplayObject;
	import flash.display.DisplayObjectContainer;
	import flash.display.Stage;
	import flash.events.Event;
	import flash.events.MouseEvent;
	import flash.utils.getTimer;
	
	import mx.managers.SystemManager;
	
	import org.cove.ape.APEngine;
	import org.cove.ape.APEVector;
	import org.cove.ape.AbstractParticle;
	import org.cove.ape.WheelParticle;
	import org.papervision3d.core.utils.virtualmouse.VirtualMouseEvent;
	import org.papervision3d.core.utils.virtualmouse.VirtualMouseMouseEvent;
	import org.papervision3d.events.InteractiveScene3DEvent;
	
	import tileui.ape.groups.StiffGroup;
	import tileui.physics.events.ParticleEvent;
	import tileui.physics.groups.IPhysicsGroup;
	import tileui.physics.particles.IPhysicsParticle;

	[Event(name="particleUpdate", type="tileui.ape.particles.ParticleEvent")]
	[Event(name="particleCollision", type="tileui.ape.particles.ParticleEvent")]
	
	public class TileUIParticle extends WheelParticle implements IPhysicsParticle
	{
		public var groupParticles:Array;
		public var group:IPhysicsGroup;
		
		private var _originalWidth:Number;
		
		public function get originalWidth():Number {
			return _originalWidth;
		}
		
		public function get width():Number {
			return radius;
		}
		
		public function set width(value:Number):void {
			radius = value;
		}
		
		public function TileUIParticle(
				x:Number, 
				y:Number, 
				width:Number, 
				fixed:Boolean = false, 
				mass:Number = 1, 
				elasticity:Number = 0.3,
				friction:Number = 0,
				traction:Number = 1) {
		
			_originalWidth = width;
			
			super(x,y,width,fixed,mass,elasticity, friction, traction);
				
		}
		
		override public function set mass(m:Number):void {
			if(isNaN(m) == false) {
				super.mass = m;
			}
		}
		
		
		public function get dragging():Boolean {
			return _dragging;
		}
		
		private var _dragging:Boolean;
		private var _oldMass:Number;
		
		private var regX:Number;
		private var regY:Number;
		
		private var lastAPEVector:APEVector;
		
		public function startDrag(dragGroup:Boolean=true):void {
			
			if(group && group is StiffGroup &&  group.physicsParticles.indexOf(this) != 0 && group.physicsParticles.length > 0 && dragGroup) {
				group.physicsParticles[0].startDrag();
				return;
			}
			
			lastAPEVectors = new Array();
			
			_dragging = true;
			
			if(!_oldMass || _oldMass == this.mass) {
				_oldMass = this.mass;
	
			}
			
			var stage:Stage = SystemManager.getSWFRoot(this).stage;
			
			this.mass = 1000000;
			
			regX = stage.mouseX - px;
			regY = stage.mouseY - py;
			
			
			stage.addEventListener(MouseEvent.MOUSE_MOVE, stage_mouseMove, true);
			stage.addEventListener(MouseEvent.MOUSE_UP, stage_mouseUp, true);
			stage.addEventListener(Event.MOUSE_LEAVE, stage_mouseLeaveHandler);
		}
		
		private var lastAPEVectors:Array;
		private var lastMovement:int;
		
		
		private function stage_mouseMove(event:MouseEvent):void {
			if(event is VirtualMouseMouseEvent) {
				return;
			}
			
			event.stopImmediatePropagation();
			
			lastMovement = getTimer();
			
			var container:DisplayObjectContainer =   APEngine.container;

			var destX:Number = event.stageX - regX;
			var destY:Number = event.stageY - regY;
			
			if(destX < 0 || destX > container.width || destY < 0 || destY > container.height) {
				dispatchEvent(new ParticleEvent(ParticleEvent.DRAG_OUTSIDE));
			}
			
			destX = Math.max(0, destX);
			destX = Math.min(container.width, destX);
			destY = Math.max(0,destY);
			destY = Math.min(container.height,destY);
			
			
			var newAPEVector:APEVector = new APEVector(destX, destY);
			lastAPEVector = this.position.minus(newAPEVector);
			
			var maxVelocity:Number = _dragging ? 70 : 70;
			
			var ratio:Number = lastAPEVector.x != 0 ? lastAPEVector.y / lastAPEVector.x : 0;
			
			lastAPEVectors.push(lastAPEVector);
			
			if(lastAPEVectors.length == 3) {
				lastAPEVectors.shift();
			}
			
			
			if(Math.abs(lastAPEVector.x) > maxVelocity) {
				lastAPEVector.x = lastAPEVector.x > 0 ? maxVelocity : -maxVelocity;
				lastAPEVector.y = lastAPEVector.x * ratio;
			}
			if(Math.abs(lastAPEVector.y) > maxVelocity) {
				lastAPEVector.y = lastAPEVector.y > 0 ? maxVelocity : -maxVelocity;
				lastAPEVector.y = lastAPEVector.x * ratio;
			}
			
			
			this.realPosition = newAPEVector;
			APEngine.step();
			
			this.velocity = lastAPEVector.mult(-.7);
		}
		
		private function stage_mouseUp(event:MouseEvent):void {
			
			var timeSinceLastMove:int = getTimer() - lastMovement;
			
			if(timeSinceLastMove < 200) {
				var newAPEVector:APEVector = new APEVector(0,0);
			
				for each(var v:APEVector in lastAPEVectors) {
					newAPEVector = newAPEVector.plus(v);
				}
				
				newAPEVector = newAPEVector.divEquals(lastAPEVectors.length);
				
				this.velocity = newAPEVector.mult(-1);
			}
			else {
				this.velocity = new APEVector(0,0);
			}
			
			stopDrag();
		}
		
		public function stopDrag():void {
			_dragging=false;
			this.mass = _oldMass;
		
			var stage:Stage = SystemManager.getSWFRoot(this).stage;
			
			stage.removeEventListener(MouseEvent.MOUSE_MOVE, stage_mouseMove, true);
			stage.removeEventListener(MouseEvent.MOUSE_UP, stage_mouseUp, true);
			stage.removeEventListener(Event.MOUSE_LEAVE, stage_mouseLeaveHandler);
		}
		
		private function stage_mouseLeaveHandler(event:Event):void
	    {
	    	if(event is InteractiveScene3DEvent || event is VirtualMouseEvent) {
	    		return;
	    	}
	    	
	        stopDrag();
	    }
		
		public override function resolveCollision(
				mtd:APEVector, vel:APEVector, n:APEVector, d:Number, o:int, p:AbstractParticle):void {

			super.resolveCollision(mtd, vel, n, d, o, p);
			
			if(onCollisionFunction != null) {
				onCollisionFunction(this, p as IPhysicsParticle);
			}
		}
		
		public var onCollisionFunction:Function;
		
		public function set realPosition(p:APEVector):void {
			super.position = p;
		}
	
		override public function set position(p:APEVector):void {
			if(!_dragging)
				super.position = p;
		}
	
		
		override public function set px(x:Number):void {
			if(!_dragging)
				super.px = x;
		}
		
		override public function set py(y:Number):void {
			if(!_dragging)
				super.py = y;
		}
		
		public var onUpdateFunction:Function;
		
		override public function update(dt2:Number):void {
			if(!_dragging)
				super.update(dt2);
		
			if(onUpdateFunction != null)
				onUpdateFunction();
		}
		
		public var parentUIContainer:DisplayObject;
		
		
	}
}