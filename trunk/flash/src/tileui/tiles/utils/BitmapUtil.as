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
	import flash.geom.Matrix;
	
	public class BitmapUtil
	{
		public static function getAverageColor(bitmap:BitmapData):Number {
			if(bitmap == null) {
				return 0;
			}
			
			var small:BitmapData = new BitmapData(1,1);
			
			var matrix:Matrix = new  Matrix();
			matrix.scale(1/bitmap.width, 1/bitmap.height);
			
			small.draw(bitmap, matrix, null, null, null, true);
			
			var color:Number = small.getPixel(0,0);
			
			small.dispose();
			
			return color;
		}
	}
}