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

package tileui.tiles
{
	import mx.collections.ArrayCollection;
	import mx.events.CollectionEvent;
	import mx.events.CollectionEventKind;
	import mx.events.PropertyChangeEvent;
	
	import org.cove.ape.APEngine;
	
	import tileui.ape.particles.TileUIParticle;
	import tileui.physics.constraints.IPhysicsConstraint;
	import tileui.physics.constraints.ISpringContraint;
	import tileui.physics.groups.IPhysicsGroup;
	import tileui.physics.particles.IPhysicsParticle;
	import tileui.tiles.layout.IGroupLayout;
	import tileui.tween.TweenManager;
	
	public class TileGroup
	{
	
		/**
		 * ArrayCollection of all the Tiles contained in this group.
		 */
		public var tiles:ArrayCollection;
		
		/**
		 * @private
		 */
		private var _layout:IGroupLayout;
		
		public function set layout(value:IGroupLayout):void {
			if(_layout) {
				_previousLayout = _layout;
			}
			
			_layout = value;
			_layout.tiles = tiles;
			
			layoutTiles(true);
		}
		
		public function get layout():IGroupLayout {
			return _layout;	
		}
		
		/**
		 * @private
		 */
		private var _previousLayout:IGroupLayout;
		
		/**
		 * @private
		 */
		public function get previousLayout():IGroupLayout {
			return _previousLayout;
		}
		
		/**
		 * @private
		 */
		private var _defaultGroup:IPhysicsGroup;
		
		public function TileGroup(layout:IGroupLayout, defaultGroup:IPhysicsGroup, tiles:ArrayCollection=null):void {
			_defaultGroup = defaultGroup;
			_layout = layout;
			
			this.tiles = new ArrayCollection();
			
			if(tiles) {
				addTiles(tiles);
			}
		
			this.tiles.addEventListener(CollectionEvent.COLLECTION_CHANGE, collectionChangeListener);
		}
		
		private function collectionChangeListener(event:CollectionEvent):void {
			if(event.kind == CollectionEventKind.REFRESH) {
				layoutTiles(true);
			}
			
			if(event.kind == CollectionEventKind.UPDATE) {
				if(event.items[0] is PropertyChangeEvent) {
					var propEvent:PropertyChangeEvent = event.items[0];
					if(propEvent.property == "scale") {
						layoutTiles(false);
					}
				}
			}
		}
		
		
		/**
		 * Adds a tile to the group. If <code>doLayout</code> is true this will trigger a layout of all the Tiles.
		 */
		public function addTile(tile:Tile, doLayout:Boolean=true):void {
			if(!this.tiles.contains(tile)) {
				
				var oldGroup:TileGroup = tile.group;
				if(oldGroup && oldGroup != this) {
					oldGroup.removeTile(tile);
				}
				
				tile.group = this;
				
				tiles.addItem(tile);
			}
			
			if(doLayout) {
	 			layoutTiles();
			}
		}
		
		public function addTiles(tiles:ArrayCollection, doLayout:Boolean=true):void {
			var regroups:Array = new Array();
			
			ungroupTiles(tiles);
			
			for each(var tile:Tile in tiles) {
				if(!this.tiles.contains(tile)) {
					
					var oldGroup:TileGroup = tile.group;
					if(oldGroup && oldGroup != this) {
						oldGroup.removeTile(tile, false);
						
						if(regroups.indexOf(oldGroup) == -1) {
							regroups.push(oldGroup);
						}
					}
					
					tile.group = this;
					
					this.tiles.addItem(tile);
				}
			}
			
			for each(var group:TileGroup in regroups) {
				if(group.tiles.length > 0) {
					var particle:TileUIParticle = group.tiles[0].particle;
					var newPosX:Number = particle.px + particle.width*2;
					particle.px = newPosX;
				}
					
				
				group.layoutTiles(true);
			}
			
			if(doLayout) {
	 			layoutTiles();
			}
		}
		
		public function breakGroup():void {
			
			ungroupTiles(tiles);
			
			for each(var tile:Tile in tiles) {
				tile.group = null;
				
				tile.particle.mass = tile.originalMass;
				
				tile.particle.px += Math.random();
				tile.particle.py += Math.random();
				
				tile.tile3D.ignoreRotation = false;
				tile.particle.collidable = true;
				
				TweenManager.addTween(tile.tile3D, {y:0, time:.6});
				
				_defaultGroup.addPhysicsParticle(tile.particle);
				tile.particle.group = _defaultGroup;
			}
			
			tiles = new ArrayCollection();
		}
		
		public function removeAllTiles():void {
			for each(var tile:Tile in tiles) {
				removeTile(tile, false);
			}
		}
		
		public function removeTile(tile:Tile, doLayout:Boolean=true):void {
			if(!this.tiles.contains(tile)) {
				return;
			}
			tile.group = null;
			tiles.removeItemAt(tiles.getItemIndex(tile));
			
			if(doLayout) {
	 			layoutTiles();
			}
		}
		
		public function layoutTiles(useTween:Boolean = true):void {
			if(!tiles || tiles.length == 0) return;

			ungroupTiles(tiles);
			
			_layout.tiles = tiles;
			_layout.layout3DTiles(useTween);
			_layout.layoutParticles(useTween);
		}
		
		private function ungroupTiles(tiles:ArrayCollection):void {
			for each(var tile:Tile in tiles) {
				ungroupParticle(tile.particle);
			}
		}
		
		private function ungroupParticle(particle:TileUIParticle):void {
			for each(var g:IPhysicsGroup in APEngine.groups) {
				removeAnyConstraints(g, particle);
			}
			
			if(particle.group) {
				var group:IPhysicsGroup = particle.group;
				
				group.removePhysicsParticle(particle);
				
				if(group.physicsParticles.length == 0) {
					APEngine.removeGroup(group as org.cove.ape.Group);
				}
			}
			
			
		}
		
		private function removeAnyConstraints(group:IPhysicsGroup, particle:IPhysicsParticle):void {
			var constraintsToRemove:Array = new Array();
			
			for each(var constraint:IPhysicsConstraint in group.constraints) {
				if(constraint is ISpringContraint) {
					var particle1:IPhysicsParticle = (constraint as ISpringContraint).particle1;
					var particle2:IPhysicsParticle = (constraint as ISpringContraint).particle2;
					
					if(particle == particle1 || particle == particle2) {
						constraintsToRemove.push(constraint);
					}
				}
			}
			
			for each(var c:IPhysicsConstraint in constraintsToRemove) {
				group.removePhysicsConstraint(c);
			}
		}
		
	}
}