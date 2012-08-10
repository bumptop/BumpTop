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
	import mx.utils.ColorUtil;
	
	import org.papervision3d.events.FileLoadEvent;
	import org.papervision3d.materials.BitmapFileMaterial;
	import org.papervision3d.materials.BitmapMaterial;
	import org.papervision3d.materials.ColorMaterial;
	
	import tileui.tiles.pv3d.materials.CheckPolicyBitmapFileMaterial;
	import tileui.tiles.pv3d.materials.TileMaterialList;
	import tileui.tiles.utils.BitmapUtil;

	public class Image3DTile extends Base3DTile
	{
		private var _url:String;
		
		public static var HIDE_SIDES:Boolean = false;
		public static var SMOOTH:Boolean = false;
		
		public function Image3DTile( url:String, width:Number=60, height:Number=10 )
		{
			_url = url;
			
			var bitmapMaterial:BitmapFileMaterial = new CheckPolicyBitmapFileMaterial(url);
			bitmapMaterial.precise = false;
			bitmapMaterial.interactive = true;
		
			bitmapMaterial.smooth = Image3DTile.SMOOTH;
			
			bitmapMaterial.addEventListener(FileLoadEvent.LOAD_COMPLETE, bitmapLoaded);
			
			super(width, height, new TileMaterialList( bitmapMaterial ), Image3DTile.HIDE_SIDES);
		}
		
		
		protected function bitmapLoaded(event:FileLoadEvent):void {
			var material:BitmapMaterial = event.currentTarget as BitmapMaterial;
			material.smooth = Image3DTile.SMOOTH;
			
			var average:Number = BitmapUtil.getAverageColor(material.bitmap);
			
			var color1:Number = ColorUtil.adjustBrightness(average, -100);
			var color2:Number = ColorUtil.adjustBrightness(average, -50);
			
			var side1:ColorMaterial = this.materials.materialsByName["left"] as ColorMaterial;
			side1.fillColor = color1;
			
			var side2:ColorMaterial = this.materials.materialsByName["right"] as ColorMaterial;
			side2.fillColor = color2;
			
			var side3:ColorMaterial = this.materials.materialsByName["front"] as ColorMaterial;
			side3.fillColor = color1;
			
			var side4:ColorMaterial = this.materials.materialsByName["back"] as ColorMaterial;
			side4.fillColor = color2;
			
			dispatchEvent(event.clone());
		}
		
	}
}