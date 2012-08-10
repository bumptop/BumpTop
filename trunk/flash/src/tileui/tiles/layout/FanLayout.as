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
	
	import tileui.tiles.Tile;
	import tileui.tiles.pv3d.Base3DTile;
	import tileui.tween.TweenManager;
	
	import tileui.ape.particles.TileUIParticle;
	
	[ExcludeClass]
	
	public class FanLayout extends StackLayout
	{
		override public function get selectionMode():String {
			return "tile";
		}
		
		protected var _fanTo:Point;
		
		public function set fanTo(value:Point):void {
			_fanTo = value;
			layout3DTiles(true);
		}
		
		public function get fanTo():Point {
			return _fanTo;
		}
		
		override public function layout3DTiles(useTween:Boolean=true):void
		{
			var lastY:Number = 0;
			
			var n:int = _tiles.length;
			
			if(!_fanTo) return;
			
			var baseTile:Tile = _tiles.getItemAt(0) as Tile;
			var baseX:Number = baseTile.particle.px;
			var baseY:Number = baseTile.particle.py;
				
					
			var base3DX:Number = baseTile.tile3D.x;
			var base3DZ:Number = baseTile.tile3D.z;
			var base3DY:Number = baseTile.tile3D.y;
				
			for(var i:int=0; i<n; i++)
			{
				var tile:Tile = _tiles.getItemAt(i) as Tile;
				var particle:TileUIParticle = tile.particle;
				
				var tile3D:Base3DTile = tile.tile3D;
				tile3D.ignoreRotation = true;
				
				var rotationY:Number;
				var rotationZ:Number = 0;
				
				if(_fanTo) {
					var xDist:Number = _fanTo.x - baseX;
					var yDist:Number = _fanTo.y - baseY;
				
					var percent:Number = 1 - (n-i)/n;
					
					var x:Number = baseX + (xDist*percent);
					var y:Number = baseY + (yDist*percent);
				
					var offsetX:Number = x - tile.particle.px;
					var offsetZ:Number = y - tile.particle.py;
				
					//lastY = percent * 50;
					lastY += tile.height;
					
					rotationY = offsetX > 0 ? -25*percent: 25*percent;
				}
				
				if(useTween) {
					TweenManager.addTween(tile, {offsetX:offsetX, offsetZ:-offsetZ,  time:.5});
					TweenManager.addTween(tile3D, {rotationY:rotationY, rotationZ:rotationZ, y:lastY, time:1});
				}
				else {
					tile.offsetX = offsetX;
					tile.offsetZ = -offsetZ;
					
					tile3D.y = lastY;
					tile3D.rotationY = rotationY;
					tile3D.rotationZ = rotationZ;
				}
	
				tile.update();
			}
			
		}
	}
}