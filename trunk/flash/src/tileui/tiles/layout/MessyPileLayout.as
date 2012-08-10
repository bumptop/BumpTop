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

package tileui.tiles.layout
{
	import mx.collections.ArrayCollection;
	
	import org.cove.ape.APEngine;
	import org.cove.ape.Group;
	
	import tileui.ape.constraints.SpringConstraint;
	import tileui.ape.groups.MessyGroup;
	import tileui.ape.particles.TileUIParticle;
	import tileui.physics.constraints.ISpringContraint;
	import tileui.physics.groups.IPhysicsGroup;
	import tileui.tiles.Tile;
	import tileui.tween.TweenManager;
	
	public class MessyPileLayout implements IGroupLayout
	{
		public function get selectionMode():String {
			return "group";
		}
		private var _tiles:ArrayCollection;
		
		public function set tiles(value:ArrayCollection):void {
			_tiles = value;
		}
		
		public function layoutParticles(useTween:Boolean=true):void
		{
			var group:IPhysicsGroup = new MessyGroup();
			
			group.collideInternal = false;
			
			var n:int = _tiles.length;
			
			for(var i:int=0; i<n; i++) {
				var tile:Tile = _tiles[i];
				var particle:TileUIParticle = tile.particle;
				
				if(particle.group) {
					particle.group.removePhysicsParticle(particle);
				}
				
				particle.collidable = true;
				particle.px += Math.random();
				particle.group = group;
				particle.mass = tile.originalMass;
				
				group.addPhysicsParticle(particle)
				
				if(i > 0) {
					var particleBefore:TileUIParticle = _tiles[i-1].particle;
						
					var constraint:ISpringContraint = makeConstraint(particle, particleBefore)
					constraint.restLength = 40;
					constraint.stiffness = .8;
					
					group.addPhysicsConstraint(constraint);
					
					if(i%4==0) {
						var particle4Before:TileUIParticle = _tiles[i-4].particle;
						
						constraint = makeConstraint(particle, particle4Before)
						constraint.restLength = 60;
						constraint.stiffness = .8;
					
						group.addPhysicsConstraint(constraint);
					}
				}
			}
			
			var tempArray:Array = [];
			for each(var g:Group in APEngine.groups) {
				tempArray.push(g);
			}
			group.addCollidableList(tempArray);
			
			APEngine.addGroup(group as org.cove.ape.Group);
		}
		
		public function layout3DTiles(useTween:Boolean=true):void
		{
			var n:int = _tiles.length;
			
			for(var i:int=0; i<n; i++)
			{
				var tile:Tile = _tiles[i];
				tile.tile3D.ignoreRotation = false;
				tile.tile3D.disconnectFromPhysics = false;
				
				TweenManager.addTween(tile.tile3D, {rotationX:0, rotationY:0, rotationZ:0, y:0, time:.6});
				TweenManager.addTween(tile, {offsetX:0, offsetZ:0, time:.6});
			}
		}
		
		private function makeConstraint(pt1:TileUIParticle, pt2:TileUIParticle):ISpringContraint {
			var constraint:ISpringContraint = new SpringConstraint(pt1, pt2, 1, false, 0, 0);
			
			return constraint;
		}
		
	}
}