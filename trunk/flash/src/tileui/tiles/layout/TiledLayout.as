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
	import flash.geom.Point;
	
	import mx.collections.ArrayCollection;
	
	import tileui.tiles.Tile;
	import tileui.tiles.pv3d.Base3DTile;
	import tileui.tween.TweenManager;
	
	import tileui.ape.particles.TileUIParticle;
	
	[ExcludeClass]
	
	public class TiledLayout implements IGroupLayout
	{
		public function get selectionMode():String {
			return "tile";
		}
		
		private var _tiles:ArrayCollection;
		
		private var _tileTo:Point;
		
		public function set tileTo(value:Point):void {
			_tileTo = value;
			layout3DTiles(true);
		}
		
		public function get tileTo():Point {
			return _tileTo;
		}
		
		public function set tiles(tiles:ArrayCollection):void
		{
			_tiles = tiles;
		}
		
		public function layoutParticles(useTween:Boolean=true):void
		{
		}
		
		public function layout3DTiles(useTween:Boolean=true):void
		{
			
			var tilesWide:Number = Math.ceil(Math.sqrt(_tiles.length));
			
			if(!_tileTo) return;
			
			var row:int=0;
			var tilesInCurrentRow:int=0;
			
			var tile0:Tile = _tiles.getItemAt(0) as Tile;
			var baseX:Number = tile0.particle.px;
			var baseY:Number = tile0.particle.py;
			
			var width:Number = (_tileTo.x - baseX);
			var height:Number = (_tileTo.y - baseY);
			var tileWidth:Number = width/(tilesWide - 1);
			var tileHeight:Number = height/(tilesWide - 1);
			
			for(var i:int=0; i<_tiles.length; i++) {
				var tile:Tile = _tiles.getItemAt(i) as Tile;
				var tile3D:Base3DTile = tile.tile3D;
				var particle:TileUIParticle = tile.particle;
				
				if(tilesInCurrentRow >= tilesWide) {
					row++;
					tilesInCurrentRow = 0;
				}
				
				
				var column:Number = i % tilesWide;
				
				var x:Number = baseX + (tileWidth*column);
				var y:Number = baseY + (tileHeight*row);
				
				var offsetX:Number = x - tile.particle.px;
				var offsetZ:Number = y - tile.particle.py;
				
				tile.tile3D.disconnectFromPhysics = false;
				tile.tile3D.ignoreRotation = true;
				
				var raised:Boolean = false;
				
				if(column % 2 == 0) {
					raised = row % 2 == 0;
				}
				else {
					raised = row % 2 != 0;
				}
				var y3d:Number = raised ? 20 - tile.height : 20 + tile.height;
				
				TweenManager.addTween(tile.tile3D, {rotationY:0, rotationX:0, rotationZ:0, time:1, y:y3d});
				TweenManager.addTween(tile, {offsetX:offsetX, offsetZ:-offsetZ, time:1});

				tile.update();
				
				tilesInCurrentRow++;		
			}
			
		}
		
	}
}