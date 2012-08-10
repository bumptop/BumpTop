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
	import flash.display.BitmapData;
	
	import org.papervision3d.materials.BitmapMaterial;
	
	import tileui.tiles.pv3d.materials.TileMaterialList;
	import tileui.tiles.pv3d.Base3DTile;
	import tileui.tiles.pv3d.BitmapData3DTile;

	public class BitmapDataTile extends Tile
	{
		/**
		 * @private
		 */
		private var bitmapData:BitmapData;
		
		public function BitmapDataTile(bitmapData:BitmapData, width:Number=60, height:Number=10):void {
			this.bitmapData = bitmapData;
		
			super(width, height);
		}
		
		/**
		 * Creates a new instance of a <code>BitmapData3DTile</code> using the <code>bitmapData</code> property that was
		 * passed in in this tile's constructor.
		 */
		override protected function create3DTile(width:Number, height:Number):Base3DTile {
			return new BitmapData3DTile(bitmapData, width, height);
		}
	}
}