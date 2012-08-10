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
	
	import tileui.ape.groups.StiffGroup;
	import tileui.ape.particles.TileUIParticle;
	import tileui.physics.groups.IPhysicsGroup;
	import tileui.tiles.Tile;
	import tileui.tiles.pv3d.Base3DTile;
	import tileui.tween.TweenManager;
	
	
	public class StackLayout implements IGroupLayout
	{
		public function get selectionMode():String {
			return "group";
		}
		
		protected var _rotation:Number = 0;
		
		
		protected var _tiles:ArrayCollection;
		
		public function set tiles(value:ArrayCollection):void {
			_tiles = value;
		}
		
		public function layoutParticles(useTween:Boolean=true):void
		{
			if(!_tiles) return;
			
			var group:IPhysicsGroup = new StiffGroup();
			
			group.collideInternal = false;
			
			var n:int = _tiles.length;
			
			var particle0:TileUIParticle = (_tiles[0] as Tile).particle;
				
			var totalMass:Number = 0;
			
			for(var i:int=0; i<n; i++) {
				var tile:Tile = _tiles[i];
				var particle:TileUIParticle = tile.particle;
				
				totalMass += tile.originalMass*tile.scale*i*100;
				
				particle.collidable = i==0;
				
				particle.group = group;
				group.addPhysicsParticle(particle);
				
				if(i > 0) {
					if(useTween) {
						TweenManager.addTween(particle, {px:particle0.px, py:particle0.py, time:.6});
					}
					else {
						particle.px = particle0.px;
						particle.py = particle0.py;
					}
				}
			}
			
			particle0.mass = totalMass > 0 ? totalMass :_tiles[0].originalMass;
			
			var tempArray:Array = [];
			for each(var g:Group in APEngine.groups) {
				tempArray.push(g);
			}
			group.addCollidableList(tempArray);
			
			APEngine.addGroup(group as org.cove.ape.Group);
		}
		
		public function layout3DTiles(useTween:Boolean=true):void
		{
			var lastTile:Tile;
			var lastY:Number = 0;
			var lastRotation:Number = 0;

			if(!_tiles) return;
			
			var n:int = _tiles.length;
			
			for(var i:int=0; i<n; i++)
			{
				var tile:Tile = _tiles.getItemAt(i) as Tile;
				var particle:TileUIParticle = tile.particle;
				
				var tile3D:Base3DTile = tile.tile3D;
				tile3D.ignoreRotation = true;
				tile3D.disconnectFromPhysics = false;
				
				var rotationY:Number = lastRotation;
				
				if(useTween) {
					TweenManager.addTween(tile, {offsetX:0, offsetZ:0,  time:.5});
					TweenManager.addTween(tile3D, {rotationY:rotationY, rotationZ:0, y:lastY, time:1});
				}
				else {
					tile.offsetX = 0;
					tile.offsetZ = 0;
					
					tile3D.y = lastY;
					tile3D.rotationY = rotationY;
					tile3D.rotationZ = 0;
				}
				
				
				lastTile = tile;
				lastY += lastTile.height * lastTile.scale;
				lastRotation += _rotation;
			}
			
		}
		
	}
}