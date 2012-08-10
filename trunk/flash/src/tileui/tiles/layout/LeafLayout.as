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
	import tileui.tiles.Tile;
	import tileui.tiles.pv3d.Base3DTile;
	import tileui.tween.TweenManager;
	
	import tileui.ape.particles.TileUIParticle;
	
	[ExcludeClass]
	
	public class LeafLayout extends SpiralLayout
	{
		public static const LEFT:String = "left";
		public static const RIGHT:String = "right";
		
		private var _percent:Number = 0;
		
		public function get percent():Number {
			return _percent;
		}
		
		public function set percent(value:Number):void {
			_percent = value;
			
			layout3DTiles(true);
		}
		
		private var _direction:String;
		
		public function get direction():String {
			return _direction;
		}
		
		public function set direction(value:String):void {
			_direction = value;
			layout3DTiles(true);
		}
		
		
		override public function layout3DTiles(useTween:Boolean=true):void
		{
			if(!_tiles || _tiles.length == 0) {
				return;
			}
			
			var breakIndex:Number = Math.round(_tiles.length*(1-_percent));
			
			stackLayoutFunction(useTween, breakIndex);
			
			var tile0:Tile = _tiles.getItemAt(0) as Tile;
			
			var height:Number = getStackHeight(breakIndex);
			
			for(var i:int=breakIndex; i<_tiles.length; i++) {
				
				var angle:Number = -90;
				
				var prevTile:Tile = i > 0 ? _tiles.getItemAt(i-1) as Tile : _tiles.getItemAt(0) as Tile;
				var xPos:Number = tile0.particle.width*2 + (prevTile.height + 3) * (i - breakIndex);
				
				if(_direction == "left") {
					xPos *= -1;
					angle *= -1;
				}
				
				var tile:Tile = _tiles.getItemAt(i) as Tile;
				var tile3D:Base3DTile = tile.tile3D;
				tile3D.disconnectFromPhysics = false;
				tile3D.ignoreRotation = true;
				
				TweenManager.addTween(tile3D, {rotationY:0, rotationZ:angle, y:height, time:.6});
				TweenManager.addTween(tile, {offsetX:xPos, offsetZ:0, time:.6});
			}
		}
		
		private function getStackHeight(breakIndex:Number):Number {
			var height:Number = 0;
			
			for(var i:int=0; i<breakIndex; i++) {
				height += (_tiles.getItemAt(i) as Tile).height;	
			}
			
			return height;
		}
		
		private function stackLayoutFunction(useTween:Boolean, breakIndex:int):void {
			var lastTile:Tile;
			var lastY:Number = 0;
			var lastRotation:Number = 0;
			
			if(!_tiles) return;
			
			for(var i:int=0; i<breakIndex; i++)
			{
				var tile:Tile = _tiles.getItemAt(i) as Tile;
				var particle:TileUIParticle = tile.particle;
				
				var tile3D:Base3DTile = tile.tile3D;
				tile3D.ignoreRotation = true;
				tile3D.disconnectFromPhysics = false;
				
				var rotationY:Number = lastRotation;
				
				
				if(useTween) {
					TweenManager.addTween(tile, {offsetX:0, offsetZ:0,  time:.6});
					TweenManager.addTween(tile3D, {rotationY:rotationY, rotationZ:0, y:lastY, time:.6});
				}
				else {
					tile.offsetX = 0;
					tile.offsetZ = 0;
					
					tile3D.y = lastY;
					tile3D.rotationY = rotationY;
					tile3D.rotationZ = 0;
				}
				
				
				lastTile = tile;
				lastY += lastTile.height;
				lastRotation += _rotation;
			}
			
			
		}
		
		override public function get selectionMode():String {
			return "tile";
		}
		
		
	}
}