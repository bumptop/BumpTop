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

package tileui.tiles.utils
{
	import flash.display.BitmapData;
	import flash.text.TextFormat;
	import flash.text.TextField;
	import mx.core.Application;
	import flash.display.BlendMode;
	import flash.geom.ColorTransform;
	import flash.geom.Matrix;
	import mx.controls.Label;
	import mx.controls.Text;
	import mx.controls.TextArea;
	
	public class TextOverlayUtil
	{
		public static function overlayText(bitmapData:BitmapData, text:String, textFormat:TextFormat=null, textAlpha:Number=1, blendMode:String="normal", verticalAlign:String="top"):BitmapData {
			if(!textFormat) {
				textFormat = new TextFormat();
			}
			
			var newBitmapData:BitmapData = bitmapData.clone();
			
			var textArea:TextArea = getLabel(bitmapData.width, bitmapData.height, text);
			
			var textField:TextField = getTextField(bitmapData.width,bitmapData.height,text, textFormat);
			
			textArea.htmlText = textField.htmlText;
			textArea.setStyle("fontSize", textField.getTextFormat().size);
			textArea.alpha = textAlpha;
			textArea.validateNow();
			textArea.width = bitmapData.width;
			
			var textHeight:Number = textArea.textHeight;
			
			var matrix:Matrix = new Matrix();
			
			switch(verticalAlign) {
				case "bottom":
					matrix.ty = bitmapData.height - textHeight;
					break;
				case "middle":
					matrix.ty = bitmapData.height/2 - textHeight/2;
					break
				default:
					break;
			}
			
			newBitmapData.draw(textArea, matrix, new ColorTransform(1,1,1,textAlpha), blendMode);
			
			(Application.application as Application).removeChild(textArea);
			return newBitmapData;
		}
		
		private static function getTextField(width:Number, height:Number, text:String, textFormat:TextFormat):TextField {
			
			var textField:TextField = new TextField();
			textField.multiline = true;
			textField.htmlText = text;
			
			textField.embedFonts = Application.application.systemManager.isFontFaceEmbedded(textFormat);
			
			textFormat.size = 200;
			textField.setTextFormat(textFormat);
			
			
			while(textField.textWidth > width || textField.textHeight > height) {
				textFormat.size = Number(textFormat.size) - 1;
				textField.setTextFormat(textFormat);
			}
			
			return textField;
		}
		
		private static function getLabel(width:Number, height:Number, text:String):TextArea {
			var textArea:TextArea =  new TextArea();
			textArea.text = text;
			textArea.width = width;
			textArea.height = height;
			
			(Application.application as Application).addChild(textArea);
			
			var size:Number = 20;
			textArea.setStyle("fontSize", size);
			textArea.setStyle("backgroundAlpha", 0.0);
			textArea.setStyle("borderStyle", "none");
			textArea.validateNow();
			
			return textArea;
		}
	}
}