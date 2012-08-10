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
	import flash.events.Event;
	import flash.utils.Dictionary;
	
	import org.papervision3d.objects.DisplayObject3D;
	import org.papervision3d.view.BasicView;
	import org.papervision3d.view.layer.ViewportLayer;
	import org.papervision3d.view.layer.util.ViewportLayerSortMode;
	
	public class LayeredBasicView extends BasicView
	{
		private var layersByName:Dictionary;
		private var layers:Array;
		private var layersToRender:Array;
		private var layerContainers:Array;
		
		public function LayeredBasicView(viewportWidth:Number=640, viewportHeight:Number=480, scaleToStage:Boolean=true, interactive:Boolean=false, layerNames:Array=null, layerNamesToRender:Array=null, cameraType:String="Target")
		{
			
			super(viewportWidth, viewportHeight, scaleToStage, interactive, cameraType);
			
			layersByName = new Dictionary();
			layers = [];
			layersToRender = [];
			layerContainers = [];
			
			for(var i:int=0; i<layerNames.length; i++) {
				var layerContainer:DisplayObject3D = new DisplayObject3D();
				layerContainers[i] = layerContainer;
				
				var layer:ViewportLayer = viewport.getChildLayer(layerContainer, true);
				layer.doubleClickEnabled = true;
				layers.push(layer);
				
				layersByName[layerNames[i]] = layer;
				
				if(layerNamesToRender != null && layerNamesToRender.indexOf(layerNames[i]) != -1) {
					layersToRender.push(layer);
				}
			}
			
			viewport.containerSprite.sortMode = ViewportLayerSortMode.INDEX_SORT;  
		}
		
		override protected function onRenderTick(event:Event=null) : void {
			renderer.renderLayers(scene, camera, viewport, layersToRender);
		}
		
		public function addChildToLayer(child:DisplayObject3D, layerName:String):void {
			var layer:ViewportLayer = layersByName[layerName];
			if(layer != null) {
				layer.addDisplayObject3D(child);
			}
		}
		
		public function removeChildFromLayer(child:DisplayObject3D, layerName:String):void {
			var layer:ViewportLayer = layersByName[layerName];
			if(layer != null) {
				layer.removeDisplayObject3D(child);
			}
		}
		
		public function getNamedLayer(name:String):ViewportLayer {
			return layersByName[name];
		}
	}
}