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

package tileui.controls.menu.pieMenuClasses
{
	import mx.charts.series.PieSeries;
	import mx.core.mx_internal;
	
	use namespace mx_internal;
	
	[ExcludeClass]
	
	public class MyPieSeries extends PieSeries
	{
		override protected function updateDisplayList(unscaledWidth:Number,
												  unscaledHeight:Number):void
		{
			super.updateDisplayList(unscaledWidth,unscaledHeight);
	
			
			
			graphics.clear();
			
			var radius:Number = getInnerRadiusInPixels();
			graphics.lineStyle(1, 0x000000, 0);
			graphics.beginFill(getStyle("themeColor"), 0);
			graphics.drawCircle(unscaledWidth/2, unscaledHeight/2, radius);
		}
	}
}