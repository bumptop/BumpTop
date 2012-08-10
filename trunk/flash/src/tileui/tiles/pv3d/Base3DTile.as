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
	import mx.core.UIComponent;
	
	import org.papervision3d.core.proto.MaterialObject3D;
	import org.papervision3d.materials.utils.MaterialsList;
	import org.papervision3d.objects.primitives.Cube;
	
	import tileui.tiles.pv3d.materials.TileMaterialList;
	
	public class Base3DTile extends Cube
	{
		/**
		 * @private
		 */
		public var disconnectFromPhysics:Boolean = false;
		
		/**
		 * @private
		 */
		public var ignoreRotation:Boolean = false;
		
		private var _materials:TileMaterialList;
		
		public function Base3DTile( width:Number=60, height:Number=10, materials:TileMaterialList=null, topOnly:Boolean=false)
		{
			if(materials == null) {
				materials = new TileMaterialList();
			}
			
			_materials = materials;
			
			var ml:MaterialsList = new MaterialsList();
			ml.addMaterial(materials.side1, 'front');
			ml.addMaterial(materials.side2, 'back');
			ml.addMaterial(materials.side3, 'right');
			ml.addMaterial(materials.side4, 'left');
			ml.addMaterial(materials.bottom, 'bottom');
			ml.addMaterial(materials.top, 'top');
			
			var excludedFaces:Number = Cube.NONE;
			if(topOnly) {
				excludedFaces = Cube.BOTTOM + Cube.LEFT + Cube.RIGHT + Cube.FRONT + Cube.BACK
			}
			
			super(ml, width, width, height, 1, 1, 1, Cube.NONE, excludedFaces);	
		}
		
		public function get tileMaterials():TileMaterialList {
			return _materials;
		}
		
		public function get faceMaterial():MaterialObject3D {
			return _materials.top;
		}
		
		public var parentUIContainer:UIComponent;
	}
}