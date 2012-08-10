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
	
	public class SpacedLayout implements IGroupLayout
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
			
			var n:int=_tiles.length;
			
			for(var i:int=0; i<n; i++) {
				var tile:Tile = _tiles[i];
				var particle:TileUIParticle = tile.particle;
				
				particle.collidable = true;
				
				particle.group = group;
				
				group.addPhysicsParticle(particle);
				
				if(i > 0) {
					for(var j:int=0; j<n; j++) {
						if(j!=i) {
							var particle2:TileUIParticle = _tiles[j].particle;
							
							var constraint:ISpringContraint = new SpringConstraint(particle, particle2, 1, false, 0, 0);
							constraint.restLength = Math.max(constraint.restLength, particle.width*2);
							
							group.addPhysicsConstraint(constraint);
						}
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
				tile.tile3D.y = 0;
				
				tile.tile3D.disconnectFromPhysics = false;
			}
		}
		
	}
}