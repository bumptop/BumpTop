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

package tileui.tiles.pv3d.materials
{
	import org.papervision3d.core.proto.MaterialObject3D;
	import org.papervision3d.materials.ColorMaterial;
	import org.papervision3d.materials.BitmapFileMaterial;
	
	/**
	 * A utility class to help with the assignment of materials to the faces of a tile. 
	 * 
	 * This class is little more than a wrapper for the built in <code>MaterialsList</code> class in 
	 * PaperVision, but it allows us to explicitly define exactly which materials we are going to use
	 * for a 3D tile (as opposed to referring to them by String names). This class is meant to
	 * eliminate confusion when setting the list of materials when you create a new Tile.
	 */
	public class TileMaterialList
	{
		/**
		 * The material displayed on the top of the Tile.
		 */
		public var top:MaterialObject3D;
		
		/**
		 * The material displayed on the bottom of the Tile.
		 */
		public var bottom:MaterialObject3D;
		
		public var side1:MaterialObject3D;
		public var side2:MaterialObject3D;
		public var side3:MaterialObject3D;
		public var side4:MaterialObject3D;
		
		/**
		 * If any of the materials are not included then default <code>ColorMaterial</code> materials will be used
		 * by default.
		 */
		public function TileMaterialList(top:MaterialObject3D=null, bottom:MaterialObject3D=null, side1:MaterialObject3D=null, 
			side2:MaterialObject3D=null, side3:MaterialObject3D=null, side4:MaterialObject3D=null) {
			
			this.top = top ? top : new ColorMaterial(BitmapFileMaterial.LOADING_COLOR);
			this.bottom = bottom ? bottom : new ColorMaterial(BitmapFileMaterial.LOADING_COLOR);
			
			this.side1 = side1 ? side1 : new ColorMaterial(0x63777F);
			this.side2 = side2 ? side2 : new ColorMaterial(0x63777F);
			this.side3 = side3 ? side3 : new ColorMaterial(0x3D4A4F);
			this.side4 = side4 ? side4 : new ColorMaterial(0xC6EEFF);
		}
	}
}