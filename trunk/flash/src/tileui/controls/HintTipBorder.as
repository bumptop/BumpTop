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

package tileui.controls
{
	import mx.skins.halo.ToolTipBorder;
	import flash.display.Graphics;
	import flash.filters.DropShadowFilter;

	[ExcludeClass]
	
	public class HintTipBorder extends ToolTipBorder
	{
		override protected function updateDisplayList(w:Number, h:Number):void
		{	
			super.updateDisplayList(w, h);
	
			var borderStyle:String = getStyle("borderStyle");
			var backgroundColor:uint = getStyle("backgroundColor");
			var backgroundAlpha:Number= getStyle("backgroundAlpha");
			var borderColor:uint = getStyle("borderColor");
			var cornerRadius:Number = getStyle("cornerRadius");
			var shadowColor:uint = getStyle("shadowColor");
			var shadowAlpha:Number = 0.1;
	
			var g:Graphics = graphics;
			g.clear();
			
			filters = [];
			
			// border 
			drawRoundRect(
				0, 0, w, h - 2, 3,
				borderColor, backgroundAlpha); 
	
			// left pointer 
			g.beginFill(borderColor, backgroundAlpha);
			g.moveTo(w, h/2-6);
			g.lineTo(w+11, h/2);
			g.lineTo(w, h/2+6);
			g.moveTo(w, h/2-6);
			g.endFill();
			
			filters = [ new DropShadowFilter(2, 90, 0, 0.4) ];
		}
	}
}