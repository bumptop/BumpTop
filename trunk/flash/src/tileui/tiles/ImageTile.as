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
	import tileui.tiles.pv3d.Base3DTile;
	import tileui.tiles.pv3d.Image3DTile;
	import flash.net.URLRequest;

	/**
	 * Loads an image from a URL as a Tile. This uses the <code>Image3DTile</code> to represent the tile in 3D. See
	 * the <code>Image3DTile</code> class for more details.
	 * 
	 * @see tileui.tiles.pv3d.Image3DTile
	 */
	public class ImageTile extends Tile
	{
		private var url:String;
		
		public function ImageTile(url:String, width:Number=60, height:Number=10):void {
			this.url = url;
			
			super(width, height);
		}
		
		/**
		 * Creates a new <code>Image3DTile</code>.
		 * 
		 * @see tileui.tiles.pv3d.Image3DTile
		 */
		override protected function create3DTile(width:Number, height:Number):Base3DTile {
			return new Image3DTile(url, width, height);
		}
	}
}