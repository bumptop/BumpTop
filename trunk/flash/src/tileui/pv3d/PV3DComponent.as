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
	import flash.filters.DropShadowFilter;
	
	import mx.core.UIComponent;
	
	import org.papervision3d.cameras.Camera3D;
	import org.papervision3d.objects.DisplayObject3D;
	import org.papervision3d.render.BasicRenderEngine;
	import org.papervision3d.view.layer.ViewportLayer;
	
	import tileui.controls.menu.PieMenu;
	import tileui.tiles.pv3d.Base3DTile;

	import flash.filters.GlowFilter;

	[ExcludeClass]
	
	public class PV3DComponent extends UIComponent
	{
		[Bindable]
		public var camera:Camera3D;
		
		public var basicView:LayeredBasicView;
		private var menuHolder:UIComponent;
		
		private var mainLayer:ViewportLayer;
		private var selectionLayer:ViewportLayer;
		
		public var selectionDrawingSprite:ViewportLayer;
		
		override protected function createChildren():void
		{
			super.createChildren();
			
			basicView = new LayeredBasicView(640, 320, true, true, ["main", "selectionDrawing", "selection"], ["main", "selectionDrawing", "selection"]);
			selectionDrawingSprite = basicView.getNamedLayer("selectionDrawing");
			
			basicView.renderer = new BasicRenderEngine();
			basicView.startRendering();
			
			camera = basicView.cameraAsCamera3D;
			
			addChildAt(basicView, 0);
			basicView.filters = [new DropShadowFilter(2, 45, 0, .5)];
			
			menuHolder = new UIComponent();
			addChild(menuHolder);
			
			resetCameraPosition();
		}
		
		public function addTile(tile:Base3DTile):void {
			tile.parentUIContainer = this;
			addDO3DToLayer(tile, "main");
		}
		
		public function removeTile(tile:Base3DTile):void {
			basicView.removeChildFromLayer(tile, "main");
			basicView.removeChildFromLayer(tile, "selection");
		}
		
		public function addTileToSelectionLayer(tile:Base3DTile):void {
			tile.parentUIContainer = this;
			addDO3DToLayer(tile, "selection");
		}
		
		public function addDO3DToLayer(do3D:DisplayObject3D, layer:String):void {
			basicView.addChildToLayer(do3D, layer);
		}
		
		public function removeDO3DFromLayer(do3D:DisplayObject3D, layer:String):void {
			basicView.removeChildFromLayer(do3D, layer);
		}
		
		public function removeTileFromSelectionLayer(tile:Base3DTile):void {
			basicView.removeChildFromLayer(tile, "selection");
		}
		
		public function addMenu(menu:PieMenu):void {
			if(menuHolder.numChildren > 0) {
				removeAllMenus();	
			}
			
			menuHolder.addChild(menu);
		}
		
		public function removeAllMenus():void {
			while(menuHolder.numChildren > 0) {
				menuHolder.removeChildAt(0);
			}
		}
		
		public function resetCameraPosition():void {
			camera.x = 0;
			camera.z = -1800;
			camera.y = 3500;
			camera.zoom = 61;
			camera.focus = 61;
		}

		override protected function updateDisplayList(unscaledWidth:Number, unscaledHeight:Number):void {
			super.updateDisplayList(unscaledWidth, unscaledHeight);
			
			menuHolder.setActualSize( unscaledWidth, unscaledHeight);
		}
		
	}
}