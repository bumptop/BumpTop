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
	import org.papervision3d.materials.ColorMaterial;
	import tileui.tiles.pv3d.Base3DTile;
	import tileui.tiles.pv3d.materials.TileMaterialList;
	import mx.utils.ColorUtil;
	
	/**
	 * A 3D tile representing a single color. The top of the Tile is the color passed
	 * in the constructor. The sides alternate between darker tinted versions of the color. The tinted is done
	 * using <code>ColorUtil.adjustBrightness()</code> and one side is tinted by setting the brightness 
	 * of the color to -100, and the other side to -50;
	 */
	public class ColorTile extends Tile
	{
		/**
		 * @private
		 */
		protected var color:Number;
		
		public function ColorTile(color:Number, width:Number=60, height:Number=10):void {
			this.color = color;
				
			super(width, height);
		}
		
		/**
		 * Creates 3 separate <code>ColorMaterial</code> materials, one for the top and bottom, which is the
		 * color of the tile, one for sides 1 and 3, which is a tinted version (-100) and one for sides
		 * 2 and 4 (-50).
		 */
		override protected function create3DTile(width:Number, height:Number):Base3DTile {
			var material:ColorMaterial = new ColorMaterial(color);
			
			var material2:ColorMaterial = new ColorMaterial(ColorUtil.adjustBrightness(color, -100));
			var material3:ColorMaterial = new ColorMaterial(ColorUtil.adjustBrightness(color, -50));

			return new Base3DTile(width, height, new TileMaterialList(material, material, material2, material3, material2, material3));
		}
	}
}