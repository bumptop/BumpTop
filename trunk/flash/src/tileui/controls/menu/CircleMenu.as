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

package tileui.controls.menu
{
	import mx.core.UIComponent;

	[ExcludeClass]
	
	public class CircleMenu extends UIComponent
	{
		public var radius:Number = 40;
		
		override protected function updateDisplayList(unscaledWidth:Number, unscaledHeight:Number):void {
			super.updateDisplayList(unscaledWidth, unscaledHeight);
			
			var themeColor:Number = getStyle("themeColor");
			graphics.clear();
			graphics.beginFill(themeColor, .8);
			graphics.drawCircle(0,0,radius);
			graphics.drawCircle(0,0, radius/3);
			graphics.endFill();
		}
	}
}