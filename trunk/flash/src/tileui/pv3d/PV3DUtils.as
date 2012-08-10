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

package tileui.pv3d
{
	import flash.geom.Point;
	
	import org.papervision3d.cameras.Camera3D;
	import org.papervision3d.core.math.Number3D;
	import org.papervision3d.objects.DisplayObject3D;
	import org.papervision3d.core.geom.renderables.Vertex3D;
	import org.papervision3d.core.math.Matrix3D;
	
	public class PV3DUtils
	{
		static public function projectVertex(vertex:Vertex3D, view:Matrix3D, camera:Camera3D):Number3D
		{
			var m11 :Number = view.n11;
			var m12 :Number = view.n12;
			var m13 :Number = view.n13;
			var m21 :Number = view.n21;
			var m22 :Number = view.n22;
			var m23 :Number = view.n23;
			var m31 :Number = view.n31;
			var m32 :Number = view.n32;
			var m33 :Number = view.n33;
			
			var focus    :Number = camera.focus;
			var fz       :Number = focus * camera.zoom;
			
			var vx:Number = vertex.x;
			var vy:Number = vertex.y;
			var vz:Number = vertex.z;
				
			var s_z:Number = vx * m31 + vy * m32 + vz * m33 + view.n34;
			var s_x:Number = vx * m11 + vy * m12 + vz * m13 + view.n14;
			var s_y:Number = vx * m21 + vy * m22 + vz * m23 + view.n24;
					
			var persp:Number = fz / (focus + s_z);
			
			var projected:Number3D = new Number3D();
			
			projected.x = s_x * persp;
			projected.y = s_y * persp;
			projected.z = s_z;
			
			return projected;
		}
		
		static public function getScreenCoordinates(obj:DisplayObject3D, camera:Camera3D):Array {
			var coords:Array = new Array();
			
			var vertices:Array = obj.geometry.vertices;
			
			for each(var vertex:Vertex3D in vertices) {
				var screenCoords:Number3D = projectVertex(vertex, obj.view, camera);
				coords.push(new Point(screenCoords.x, screenCoords.y));
			}
			
			return coords;
		}
		
		static public function getScreenCoordinatesForVertex(vertex:Vertex3D, obj:DisplayObject3D, camera:Camera3D):Point {
			var screenCoords:Number3D = projectVertex(vertex, obj.view, camera);
			return new Point(screenCoords.x, screenCoords.y);
		}
		
	}
}