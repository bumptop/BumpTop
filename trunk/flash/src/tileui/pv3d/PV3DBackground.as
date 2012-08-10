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
	import flash.display.BitmapData;
	import flash.display.BlendMode;
	import flash.display.GradientType;
	import flash.display.InterpolationMethod;
	import flash.display.SpreadMethod;
	import flash.display.Sprite;
	import flash.events.Event;
	import flash.geom.Matrix;
	import flash.geom.Point;
	import flash.geom.Rectangle;
	import flash.text.TextFormat;
	
	import mx.core.BitmapAsset;
	import mx.graphics.GradientEntry;
	import mx.graphics.LinearGradient;
	import mx.graphics.RadialGradient;
	
	import org.papervision3d.materials.BitmapFileMaterial;
	import org.papervision3d.materials.BitmapMaterial;
	import org.papervision3d.objects.DisplayObject3D;
	import org.papervision3d.objects.primitives.Plane;
	import org.papervision3d.view.BasicView;
	
	import tileui.tiles.utils.TextOverlayUtil;

	public class PV3DBackground extends BasicView
	{
		private var platform:DisplayObject3D;
		private var platform2:DisplayObject3D;
		private var platform3:DisplayObject3D;
		private var platform4:DisplayObject3D;
		private var platform5:DisplayObject3D;
		
		[Embed(source='/assets/floor_desktop.jpg')]
		private var floorClass:Class;
	
		[Embed(source='/assets/win-7-wall.jpg')]
		private var wallClass:Class;
	
		[Embed(source='/assets/win-7-front.jpg')]
		private var frontClass:Class;
	
		private var material:BitmapFileMaterial;

		public var sceneWidth:Number = 600;
		public var sceneHeight:Number = 600;
		
		private var _floorBitmapData:BitmapData;
		public function get floorBitmapData():BitmapData { return _floorBitmapData; }
		public function set floorBitmapData(value:BitmapData):void {
			_floorBitmapData = value;
			render();
		}
		
		private var _sideWallBitmapData:BitmapData;
		public function get sideWallBitmapData():BitmapData { return _sideWallBitmapData; }
		public function set sideWallBitmapData(value:BitmapData):void {
			_sideWallBitmapData = value;
			render();
		}
		
		private var _backWallBitmapData:BitmapData;
		public function get backWallBitmapData():BitmapData { return _backWallBitmapData; }
		public function set backWallBitmapData(value:BitmapData):void {
			_backWallBitmapData = value;
			render();
		}
		
		private var _showBackground:Boolean;
		public function set showBackground(value:Boolean):void {
			_showBackground = value;
			render();
		}
		
		public function get showBackground():Boolean {
			return _showBackground;
		}
		
		private var _showSides:Boolean;
		public function set showSides(value:Boolean):void {
			_showSides = value;
			render();
		}
		
		public function get showSides():Boolean {
			return _showSides;
		}
		
		public function PV3DBackground()
		{
			super(640, 320, true, false);
			setupScene();
		}
	

		private function setupScene():void
		{
			_camera.x = 0;
			_camera.z = -1800;
			_camera.y = 3500;
			_camera.zoom = 61;
			_camera.focus = 61; 
		}
		
		public function render():void {
			generateScene(sceneWidth, sceneHeight);
			
			if(framesToRender == 0) {			
				startRendering();
				addEventListener(Event.ENTER_FRAME, enterFrameRenderer);
			}
			
			framesToRender = 2;
			
			singleRender();
		}
		
		private function generateScene(w:int, h:int):void {
			if(w == 0 || h == 0) {
				return;
			}
			
			if(platform) {
				scene.removeChild(platform);
				platform = null;
			}
			if(platform2) {
				scene.removeChild(platform2);
				platform2 = null;
			}
			if(platform3) {
				scene.removeChild(platform3);
				platform3 = null;
			}
			
			if(platform4) {
				scene.removeChild(platform4);
				platform4 = null;
			}
					
				
				
			if(showSides) {
				//top
				if(backWallBitmapData != null) {
					platform2 = createBitmapPlane(w, 500, backWallBitmapData);
				}
				else {
					platform2 = createLinearGradientPlane(w, 500, 0x445866, 0x29333B);
				}
				
				platform2.y = 250;
				platform2.z = h/2;
				scene.addChild(platform2);
				
				
				if(sideWallBitmapData != null) {
					platform3 = createBitmapPlane(h + 200, 600, sideWallBitmapData);
				}
				else {
					platform3 = createLinearGradientPlane(h + 200, 600, 0x445866, 0x29333B);
				}
				
				platform3.rotationY = -90;
				platform3.x = -w/2;
				platform3.y = 300;
				platform3.z = -100
				scene.addChild(platform3);
				
				if(sideWallBitmapData != null) {
					platform4 = createBitmapPlane(h + 200, 600, sideWallBitmapData);
				}
				else {
					platform4 = createLinearGradientPlane(h + 200, 600, 0x445866, 0x29333B);
				}
				
				platform4.rotationY = 90;
				
				platform4.x = w/2;
				platform4.y = 300;
				platform4.z = -100;
				
				scene.addChild(platform4);
			}
			
			if(showBackground) {
				if(floorBitmapData != null) {
					platform = createBitmapPlane(w, h, floorBitmapData);
				}
				else {
					platform = createRadialGradientPlane(w,h, 0xA2B0C1, 0x617893, 0x5A687A);
				}
				
				
				platform.rotationX = 90;
				scene.addChild(platform);
			}
		}
		
		private function enterFrameRenderer(event:Event):void {
			viewportWidth = sceneWidth;
			viewportHeight = sceneHeight;
			
			this.width = sceneWidth;
			this.height = sceneHeight;
			
			framesToRender--;
			
			generateScene(sceneWidth, sceneHeight);
			singleRender();
			
			if(framesToRender == 0) {
				stopRendering(true, false);
				removeEventListener(Event.ENTER_FRAME, enterFrameRenderer);
			}
		}
		
		private var framesToRender:int = 0;
		
		private function createRadialGradientPlane(w:Number, h:Number, color1:uint, color2:uint, color3:uint):Plane {
			var bitmapData:BitmapData = createRadialGradientBitmapData(w, h, color1, color2, color3);
			
			var material:BitmapMaterial = new BitmapMaterial(bitmapData, true);
			material.smooth = true;
			material.doubleSided=true;
			return new Plane(material, w, h);
		}
		
		private function createRadialGradientBitmapData(w:Number, h:Number, color1:uint, color2:uint, color3:uint):BitmapData {
			var bitmapData:BitmapData = new BitmapData(w,h);
			
			var sprite:Sprite = new Sprite();
			sprite.width = w;
			sprite.height = h;
			
			var m:Matrix = new Matrix();
			m.createGradientBox(w, h, -45 * Math.PI / 180);
			
			sprite.graphics.beginGradientFill(GradientType.RADIAL, [color1, color2, color3], [1,1,1], [0, .7*255, 255], m, SpreadMethod.PAD, InterpolationMethod.RGB, .1);
			sprite.graphics.drawRect(0,0, w, h*1.5);
			sprite.graphics.endFill();
			
			bitmapData.draw(sprite);
			
			return bitmapData;
		}
		
		private function createLinearGradientPlane(w:Number, h:Number, color1:uint, color2:uint):Plane {
			var bitmapData:BitmapData = new BitmapData(w, h, false, 0x000000);
			
			var sprite:Sprite = new Sprite();
			sprite.width = w;
			sprite.height = h;
			
			var m:Matrix = new Matrix();
			m.createGradientBox(w, h, 90 * Math.PI / 180);
			
			sprite.graphics.lineStyle(3, 0);
			sprite.graphics.beginGradientFill(GradientType.LINEAR, [color1, color2], [1,1], [0,255], m);
			sprite.graphics.drawRect(0,0, w, h);
			sprite.graphics.endFill();
			
			bitmapData.draw(sprite);
			
			var material:BitmapMaterial = new BitmapMaterial(bitmapData);

			return new Plane(material, w, h);
		}
		
		private function createBitmapFilePlane(w:Number, h:Number, url:String):Plane {
			var material:BitmapMaterial = new BitmapFileMaterial(url);
			
			return new Plane(material, w, h);
		}
		
		private function createBitmapPlane(w:Number, h:Number, bitmapData:BitmapData):Plane {
			var material:BitmapMaterial = new BitmapMaterial(bitmapData);
			return new Plane(material, w, h, 8, 8);
		}
		
		
		public function resetCameraPosition():void {
			camera.x = 0;
			camera.z = -1800;
			camera.y = 3500;
			camera.zoom = 61;
			camera.focus = 61;
		}


	}
}