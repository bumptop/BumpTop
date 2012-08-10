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
	import org.papervision3d.core.proto.MaterialObject3D;
	import org.papervision3d.objects.primitives.Plane;
	
	import tileui.controls.menu.CloseMenu;

	import flash.display.BitmapData;

	import org.papervision3d.materials.BitmapMaterial;

	import org.papervision3d.events.InteractiveScene3DEvent;

	import mx.events.CloseEvent;

	import flash.geom.ColorTransform;

	import flash.display.Sprite;

	import tileui.tiles.layout.IGroupLayout;

	import tileui.tiles.TileGroup;

	[Event(name="close", type="mx.events.CloseEvent")]
	
	public class SelectionPlane extends Plane
	{
		public var tileGroup:TileGroup;
		
		private var closePlane:Plane;
		
		public var width:Number;
		public var height:Number;
		
		public function SelectionPlane(width:Number=0, height:Number=0, segmentsW:Number=0, segmentsH:Number=0)
		{
			this.width = width;
			this.height = height;
			
			var roundedBox:Sprite = new Sprite();
			//roundedBox.graphics.lineStyle(1, 0xffffff, 1);
			roundedBox.graphics.beginFill(0,.7);
			roundedBox.graphics.drawRoundRectComplex(0, 0, width, height, 15, 15, 15, 15);
			roundedBox.graphics.endFill();
			
			var bitmap:BitmapData = new BitmapData(width, height, true, 0);
			bitmap.draw(roundedBox, null, new ColorTransform(1,1,1,.7), null, null, true);
			
			var material:BitmapMaterial = new BitmapMaterial(bitmap);
			material.smooth = true;
			
			useOwnContainer = true;
			
			super(material, width, height, segmentsW, segmentsH);
			
			addCloseButton(width/2, -height/2);
		}
		
		private function addCloseButton(x:int, y:int):void {
			var closeButton:CloseMenu = new CloseMenu();
			var bitmap:BitmapData = new BitmapData(closeButton.width, closeButton.height, true, 0);
			bitmap.draw(closeButton, null, new ColorTransform(1,1,1,.8), null, null, true);
			
			var closeMaterial:BitmapMaterial = new BitmapMaterial(bitmap);
			closeMaterial.interactive = true;
			closeMaterial.smooth = true;
			
			closePlane = new Plane(closeMaterial, bitmap.width, bitmap.height);
			closePlane.addEventListener(InteractiveScene3DEvent.OBJECT_PRESS, close3DPress);
			
			closePlane.x = x;
			closePlane.y = y;
			addChild(closePlane, "close");	
		}
		
		private function close3DPress(event:InteractiveScene3DEvent):void {
			dispatchEvent(new CloseEvent(CloseEvent.CLOSE));
		}
	}
}