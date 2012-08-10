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

package tileui.tiles.pv3d
{
	import flash.display.BitmapData;
	
	import mx.utils.ColorUtil;
	
	import org.papervision3d.materials.BitmapMaterial;
	import org.papervision3d.materials.ColorMaterial;
	
	import tileui.tiles.pv3d.materials.TileMaterialList;
	import tileui.tiles.utils.BitmapUtil;
	
	public class BitmapData3DTile extends Base3DTile
	{
		public function BitmapData3DTile(bitmapData:BitmapData, width:Number=60, height:Number=10):void {
			var average:Number = BitmapUtil.getAverageColor(bitmapData);
			
			var material:ColorMaterial = new ColorMaterial(average);
			material.interactive = true;
			
			var tint1:ColorMaterial = new ColorMaterial(ColorUtil.adjustBrightness(average, -100));
			var tint2:ColorMaterial = new ColorMaterial(ColorUtil.adjustBrightness(average, -50));
			tint1.interactive = false;
			tint2.interactive = false;
			
			super(width, height, new TileMaterialList(material, material, tint1, tint2, tint1, tint2));
		}
	}
}