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
	import flash.text.TextFormat;
	import flash.text.TextField;
	import org.papervision3d.materials.MovieMaterial;
	import mx.controls.Label;
	import tileui.tiles.pv3d.materials.TileMaterialList;
	import flash.display.MovieClip;
	
	public class Text3DTile extends Base3DTile
	{
		public function Text3DTile(text:String, width:Number=40, height:Number=5, textFormat:TextFormat=null, backgroundColor:Number=0xffffff) {
			var field:TextField = new TextField();
			field.text = text;
			field.cacheAsBitmap = true;
			field.embedFonts = true;
			field.antiAliasType = "advanced";
			
			textFormat = new TextFormat("arialEmbedded", 60, true);
			
			
			field.setTextFormat(textFormat);
			
			var sprite:MovieClip = new MovieClip();
			
			
			sprite.addChild(field);
			
			
			field.x = (width - field.textWidth)/2;
			
			field.y = (width - field.textHeight)/2
			
			
			sprite.graphics.beginFill(backgroundColor);
			sprite.graphics.drawRect(0,0,field.x + field.width, field.y + field.height);
			sprite.graphics.endFill();
			
			
			var material:MovieMaterial = new MovieMaterial(sprite, false);
			
			var materials:TileMaterialList = new TileMaterialList(material);
			
			super(width, height, materials);

		}
	}
}