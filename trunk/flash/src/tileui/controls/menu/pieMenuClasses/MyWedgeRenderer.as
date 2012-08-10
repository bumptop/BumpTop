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
	import mx.charts.renderers.WedgeItemRenderer;
	import mx.charts.series.items.PieSeriesItem;
	import mx.graphics.IFill;
	import mx.graphics.SolidColor;

	
	[ExcludeClass]
	
	public class MyWedgeRenderer extends WedgeItemRenderer
	{
		private var _wedge:PieSeriesItem;
	
		
		override public function set data(value:Object):void
		{
			_wedge = PieSeriesItem(value);
	
			super.data = value;
		}
		
		override protected function updateDisplayList(unscaledWidth:Number, unscaledHeight:Number):void {
			var fill:IFill = _wedge.fill;
			
			if(fill is SolidColor && (fill as SolidColor).alpha == 0) {
				
			}
			else {
				try {
					super.updateDisplayList(unscaledWidth, unscaledHeight);
				}
				catch(e:Error) {
					trace(e.message);
				}
			}
		}
	}
}