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

package tileui
{
	import flash.display.BitmapData;
	import flash.display.DisplayObject;
	import flash.display.Sprite;
	import flash.events.Event;
	import flash.events.MouseEvent;
	import flash.events.TimerEvent;
	import flash.geom.ColorTransform;
	import flash.geom.Matrix;
	import flash.geom.Point;
	import flash.ui.Mouse;
	import flash.utils.Dictionary;
	import flash.utils.Timer;
	
	import mx.binding.utils.BindingUtils;
	import mx.collections.ArrayCollection;
	import mx.controls.VSlider;
	import mx.core.UIComponent;
	import mx.events.CloseEvent;
	import mx.events.DragEvent;
	import mx.events.FlexEvent;
	import mx.events.SliderEvent;
	import mx.managers.DragManager;
	import mx.styles.CSSStyleDeclaration;
	import mx.styles.StyleManager;
	
	import org.papervision3d.Papervision3D;
	import org.papervision3d.core.geom.renderables.Vertex3D;
	import org.papervision3d.core.log.PaperLogger;
	import org.papervision3d.core.utils.virtualmouse.VirtualMouseMouseEvent;
	import org.papervision3d.events.InteractiveScene3DEvent;
	import org.papervision3d.materials.BitmapFileMaterial;
	import org.papervision3d.materials.BitmapMaterial;
	import org.papervision3d.objects.primitives.Plane;
	
	import tileui.ape.APEComponent;
	import tileui.ape.particles.TileUIParticle;
	import tileui.controls.menu.CloseMenu;
	import tileui.controls.menu.DrawingCanvas;
	import tileui.controls.menu.PieMenu;
	import tileui.controls.menu.PieMenuItem;
	import tileui.modules.TileUIModule;
	import tileui.modules.TileUIModuleEvent;
	import tileui.mouse.MouseSlider;
	import tileui.physics.groups.IPhysicsGroup;
	import tileui.pv3d.PV3DBackground;
	import tileui.pv3d.PV3DComponent;
	import tileui.pv3d.PV3DUtils;
	import tileui.pv3d.SelectionPlane;
	import tileui.tiles.Tile;
	import tileui.tiles.TileGroup;
	import tileui.tiles.layout.FanLayout;
	import tileui.tiles.layout.IGroupLayout;
	import tileui.tiles.layout.LeafLayout;
	import tileui.tiles.layout.MessyPileLayout;
	import tileui.tiles.layout.SpacedLayout;
	import tileui.tiles.layout.SpiralLayout;
	import tileui.tiles.layout.StackLayout;
	import tileui.tiles.layout.TiledLayout;
	import tileui.tiles.pv3d.Base3DTile;
	import tileui.tween.TweenManager;
	import flash.display.BitmapData;
	public class TileUIComponent extends UIComponent
	{
	
		private var clippingMask:Sprite;
		
		private var drawingCanvas:DrawingCanvas;
		private var middleLayer:Sprite;
		private var pv3dBG:PV3DBackground;
		private var ape:APEComponent;
		
		/**
		 * A static reference to the instance of the TileUI component. This is only useful if you have a single TileUI 
		 * component in your application. If you have more than one then the last instantiated instance will be what
		 * gets set as <code>TileUIComponent.instance</code>. This static variable allows you to access the instance of the TileUI
		 * component without having a direct reference to it (ie from within a module, or elsewhere in your application).
		 */
		static public var instance:TileUIComponent;
		
		public function TileUIComponent():void {
			super();
			
			TileUIComponent.instance = this;
		}
		
		private var _floorBitmapData:BitmapData;
		public function get floorBitmapData():BitmapData { return _floorBitmapData; }
		public function set floorBitmapData(value:BitmapData):void {
			_floorBitmapData = value;
			
			if(pv3dBG) {
				pv3dBG.floorBitmapData = value;
			}
		}
		
		private var _sideWallBitmapData:BitmapData;
		public function get sideWallBitmapData():BitmapData { return _sideWallBitmapData; }
		public function set sideWallBitmapData(value:BitmapData):void {
			_sideWallBitmapData = value;
			
			if(pv3dBG) {
				pv3dBG.sideWallBitmapData = value;
			}
		}
		
		private var _backWallBitmapData:BitmapData;
		public function get backWallBitmapData():BitmapData { return _backWallBitmapData; }
		public function set backWallBitmapData(value:BitmapData):void {
			_backWallBitmapData = value;
			
			
			if(pv3dBG) {
				pv3dBG.backWallBitmapData = value;
			}
		}
		
		/**
		 * @private
		 */
		override protected function createChildren():void {
			
			super.createChildren();
			
			clippingMask = new Sprite();
			addChild(clippingMask);
			
			drawingCanvas = new DrawingCanvas();
			drawingCanvas.clipContent = false;
			drawingCanvas.addEventListener(Event.CHANGE, checkSelection);
			drawingCanvas.addEventListener("release", selectionReleased);
			
			pv3dBG = new PV3DBackground();
			pv3dBG.floorBitmapData = _floorBitmapData;
			pv3dBG.sideWallBitmapData = _sideWallBitmapData;
			pv3dBG.backWallBitmapData = _backWallBitmapData;
			
			BindingUtils.bindProperty(pv3dBG, "showBackground", this, "showFloor");
			BindingUtils.bindProperty(pv3dBG, "showSides", this, "showSides");
			addChild(pv3dBG);
			
			middleLayer = new Sprite();
			addChild(middleLayer);
			
			addChild(drawingCanvas);
			
			pv3d = new PV3DComponent();
			
			drawingCanvas.addChild(pv3d);
			pv3d.percentHeight = pv3d.percentWidth = 100;
			
			ape = new APEComponent();
			ape.visible = false;
			drawingCanvas.addChild(ape);
			ape.percentHeight = ape.percentWidth = 100;
			
			initPV3D();
			
			checkDrawing();
			
			this.addEventListener(DragEvent.DRAG_ENTER, checkForTileDrag);	
		}
		
		private function checkForTileDrag(event:DragEvent):void {
			var data:Object = event.dragSource.dataForFormat("tileui");
			
			if(data != null) {
				DragManager.acceptDragDrop(this);
			}	
		}
		
		private function checkDrawing(...args):void {
			drawingCanvas.drawMenu = !dragging && menusShowing <= 0;
		}
		
		
		/**
		 * @private
		 */
		override protected function updateDisplayList(unscaledWidth:Number, unscaledHeight:Number):void {
			super.updateDisplayList(unscaledWidth, unscaledHeight);
			
			clippingMask.graphics.clear();
			clippingMask.graphics.beginFill(0x000000);
			clippingMask.graphics.drawRect(0,0, unscaledWidth, unscaledHeight);
			
			drawingCanvas.move(0,0);
			drawingCanvas.setActualSize(unscaledWidth, unscaledHeight);
			
			pv3dBG.mask = clippingMask;
			
			pv3dBG.sceneWidth = unscaledWidth;
			pv3dBG.sceneHeight = unscaledHeight;
			
			pv3dBG.render();
		}
		
		// Define a static variable for initializing the style property.
		private static var stylesInitialized:Boolean = initializeStyles();
		
		// Define a static method to initialize the style.
		private static function initializeStyles():Boolean {
			var styleDeclaration:CSSStyleDeclaration = StyleManager.getStyleDeclaration("Accordion");
			
			// If there's no style declaration already, create one.
			if (!styleDeclaration)
				styleDeclaration = new CSSStyleDeclaration();
			
			styleDeclaration.defaultFactory = function ():void {
				this.borderThickness = 1;
				this.headerHeight = 26;
				this.backgroundAlpha = .42;
				this.highlightAlphas = [.13, .04];
				this.fillAlphas = [1,1,1,1];
				this.fillColors = [0x1d2b3c, 0x293a4e, 0x4a7dba, 0x1d4b81];
				this.selectedFillColors = [0x4a7dba, 0x1d4b81];
				this.borderColor = 0x999999;
				this.textRollOverColor = 0xffffff;
				this.textSelectedColor = 0xffffff;
				this.dropShadowEnabled = true;
				this.shadowDirection = "left";
				this.headerStyleName = "accordionHeader";
				this.verticalGap = 0;
				this.backgroundColor = 0xffffff;
				
			};
			
			StyleManager.setStyleDeclaration("Accordion", styleDeclaration,false);
			
			
			var headerDecl:CSSStyleDeclaration = StyleManager.getStyleDeclaration(".accordionHeader");
			if(!headerDecl)
				headerDecl = new CSSStyleDeclaration(".accordionHeader");
			
			headerDecl.defaultFactory = function():void {
				this.letterSpacing = 1;
				this.color = 0xffffff;
				this.fontFamily = "Arial";
				this.fontSize = 14;
				this.fontWeight = "bold";	
			};
			
			StyleManager.setStyleDeclaration(".accordionHeader", headerDecl,false);
			
			
			return true;
		}
		
		
		
		
		
		
		
		
		
		
		
		[Bindable]
		private var pv3d:PV3DComponent;
		
		
		private var tiles:Array;
		
		private var _showFloor:Boolean = true;
		
		public function get showFloor():Boolean {
			return _showFloor;
		}
		
		[Bindable]
		/**
		 * Boolean indicating if the "floor" plane should be drawn. Disabled for the demo (ie you can try setting it
		 * to false but the floor will still show).
		 */
		public function set showFloor(value:Boolean):void {
			_showFloor = true;
		}
		
		[Bindable]
		/**
		 * Boolean indicating if the "side" planes should be drawn. These planes are the top, 
		 * left, and right sides. No bottom plane is used in the component (the color shown
		 * along the bottom edge is determined by the background color or the component, not
		 * an actual 3D plane.
		 */
		public var showSides:Boolean = true;
		
		private function initPV3D():void
		{
			PaperLogger.getInstance().unregisterLogger(PaperLogger.getInstance().traceLogger);
			
			BitmapFileMaterial.LOADING_COLOR = 0x406e95;
			Papervision3D.useDEGREES = true;
			BitmapMaterial.AUTO_MIP_MAPPING = true;
			
			tiles = new Array();
			
			ape.init();
		}
		
		
		private var _dragging:Boolean = false;
		
		private function set dragging(value:Boolean):void {
			_dragging = value;
			checkDrawing();
		}
		
		private function get dragging():Boolean {
			return _dragging;
		}
		
		
		private var _menusShowing:int=0;
		
		private function get menusShowing():int {
			return _menusShowing;
		}
		
		private function set menusShowing(value:int):void {
			_menusShowing = value;
			checkDrawing();
		}
		
		
		private var menuTimer:Timer;
		private var menuTile:Tile;
		
		/**
		 * When a drag operation on a Tile occurs the <code>draggingTile</code> will be set to the currently
		 * dragging tile. This might be useful if you are performing drag and drop operations from the TileUI
		 * component to some other Flex component.
		 */
		public var draggingTile:Tile;
		
		private var lastTilePressed:Tile;
		
		private function tileMouseDown(event:Event):void {
			dragging = true;
			
			var tile:Tile = event.target as Tile;
			var particle:TileUIParticle = tile.particle;
			
			lastTilePressed = tile;
			
			var group:TileGroup = tile.group;
			if(group) {
				var layout:IGroupLayout = group.layout;
				
				if(layout.selectionMode == "group") {
					
					particle.startDrag(true);
				}
				else {
					particle.startDrag(false);
				}
			}
			else {
				particle.startDrag(true);
			}
			
			
			draggingTile = tile;
			
			menuTile = tile;
			
			cancelMenuTimer();
			
			tile.addEventListener(InteractiveScene3DEvent.OBJECT_RELEASE, showMenuDelayed);
			systemManager.addEventListener(MouseEvent.MOUSE_UP, dragEnd);
			systemManager.addEventListener(MouseEvent.MOUSE_MOVE, cancelMenuTimer);
			
		}
		
		private var timer:Timer;
		
		private function showMenuDelayed(event:Event):void {
			var tile:Tile = lastTilePressed;
			var tile3D:Base3DTile = tile.tile3D;
			
			menuTile = tile;
			
			timer = new Timer(100, 1);
			timer.addEventListener(TimerEvent.TIMER_COMPLETE, showMenu);
			timer.start();
		}
		
		private function cancelMenuTimer(event:Event=null):void {
			if(timer) timer.stop();
		}
		
		private function tileDoubleClick(event:Event):void {
			cancelMenuTimer();
		}
		
		private function showMenu(event:TimerEvent=null):void {
			
			if(timer) timer.stop();
			
			if(!menuTile) return;
			
			var tile:Tile = menuTile;
			
			
			if(Math.abs(tile.particle.velocity.magnitude()) > 1) {
				return;
			}
			
			var menu:PieMenu = new PieMenu();
			pv3d.addMenu(menu);
			
			menusShowing++;
			
			var group:TileGroup = menuTile.group;
			var layout:IGroupLayout = group ? group.layout : null;
			
			if(group && group.tiles.length > 1 && layout && layout.selectionMode == "group") {
				menu.tiles = menuTile.group.tiles;
			}
			else {
				menu.tiles = new ArrayCollection();
				menu.tiles.addItem(tile);
			}
			
			var defaultItems:ArrayCollection;
			
			if(tile.group && tile.group.tiles.length > 1 && layout && layout.selectionMode == "group") {
				if(tile.group.layout is StackLayout) {
					defaultItems = new ArrayCollection([
						new PieMenuItem("Remove", "defaultRemove", "remove this stack", 1, PieMenuItem.REMOVE_TYPE, removeTilesByMenu),
						new PieMenuItem("Break", "defaultBreak", "break apart this stack", 1, PieMenuItem.CLICK_TYPE, breakTiles),
						new PieMenuItem("Resize", "defaultResize", "resize the tiles", 1, PieMenuItem.DRAG_TYPE, resizePile),
						new PieMenuItem("Twist", "defaultTwist", "twist this stack", 1, PieMenuItem.DRAG_TYPE, rotateTiles),
						new PieMenuItem("Fan Out", "defaultFan", "fan out this stack", 1, PieMenuItem.DRAG_TYPE, fanTiles),
						new PieMenuItem("Grid", "defaultGrid", "layout as a grid", 1, PieMenuItem.DRAG_TYPE, tileTiles),
						new PieMenuItem("Flip Page", "defaultLeaf", "leaf through this stack", 1, PieMenuItem.DRAG_TYPE, leafTiles)
					]);
				}
				else if(tile.group.layout is MessyPileLayout) {
					defaultItems = new ArrayCollection([
						new PieMenuItem("Remove", "defaultRemove", "remove this stack", 1, PieMenuItem.REMOVE_TYPE, removeTilesByMenu),
						new PieMenuItem("Break Pile", "defaultBreak", "break apart this stack", 1, PieMenuItem.CLICK_TYPE, breakTiles),
						new PieMenuItem("Create Pile", "defaultStack", "stack up this pile", 1, PieMenuItem.CLICK_TYPE, makeStack),
						new PieMenuItem("Resize", "defaultResize", "resize the tiles", 1, PieMenuItem.DRAG_TYPE, resizePile),
						new PieMenuItem("Fan Out", "defaultFan", "fan out this stack", 1, PieMenuItem.DRAG_TYPE, fanTiles),
						new PieMenuItem("Grid", "defaultGrid", "layout as a grid", 1, PieMenuItem.DRAG_TYPE, tileTiles),
					]);
				}
				else if(tile.group.layout is SpacedLayout) {
					defaultItems = new ArrayCollection([
						new PieMenuItem("Remove", "defaultRemove", "remove this stack", 1, PieMenuItem.REMOVE_TYPE, removeTilesByMenu),
						new PieMenuItem("Break Pile", "defaultBreak", "break apart this stack", 1, PieMenuItem.CLICK_TYPE, breakTiles),
						new PieMenuItem("Create Pile", "defaultStack", "stack up this pile", 1, PieMenuItem.CLICK_TYPE, makeStack),
						new PieMenuItem("Resize", "defaultResize", "resize the tiles", 1, PieMenuItem.DRAG_TYPE, resizePile),
						new PieMenuItem("Fan Out", "defaultFan", "fan out this stack", 1, PieMenuItem.DRAG_TYPE, fanTiles),
						new PieMenuItem("Grid", "defaultGrid", "layout as a grid", 1, PieMenuItem.DRAG_TYPE, tileTiles),
					]);
				}
				else {
					defaultItems = new ArrayCollection([
						new PieMenuItem("Remove", "defaultRemove", "remove this stack", 1, PieMenuItem.REMOVE_TYPE, removeTilesByMenu),
						new PieMenuItem("Break", "defaultBreak", "break apart this stack", 1, PieMenuItem.CLICK_TYPE, breakTiles),
						new PieMenuItem("Resize", "defaultResize", "resize the tiles", 1, PieMenuItem.DRAG_TYPE, resizePile),
						new PieMenuItem("Fan Out", "defaultFan", "fan out this stack", 1, PieMenuItem.DRAG_TYPE, fanTiles),
						new PieMenuItem("Grid", "defaultGrid", "layout as a grid", 1, PieMenuItem.DRAG_TYPE, tileTiles),
					]);
				}
			}
			else {
				defaultItems = new ArrayCollection([
					new PieMenuItem("Resize", "defaultRemove", "resize this tile", 1, PieMenuItem.DRAG_TYPE, resizePile),
					new PieMenuItem("Remove", "defaultResize", "remove this tile", 1, PieMenuItem.REMOVE_TYPE, removeTilesByMenu),
				]);
			}
			
			var newItems:Array = getMenuItems(tile);
			
			for each(var item:PieMenuItem in newItems) {
				defaultItems.addItem(item);
			}
			menu.width = menu.height = 300;
			
			var screenCoords:Array = PV3DUtils.getScreenCoordinates(tile.tile3D, pv3d.camera);
			
			
			var posX:Number = screenCoords[0].x - menu.width/2 + this.width/2;
			var posY:Number = screenCoords[0].y - menu.height/2 + this.height/2;
			
			menu.dataProvider = defaultItems;
			
			menu.startAngle = calculateMenuAngle(new Point(posX, posY), menu.width, menu.height);
			
			menu.move(posX, posY);
			
			menu.addEventListener(MouseEvent.ROLL_OUT, hideMenu);
			menu.addEventListener(FlexEvent.HIDE, menuHidden);
			
		}
		
		private function getMenuItems(tile:Tile):Array {
			var uniqueIDs:Dictionary = new Dictionary(true);
			
			var newItems:Array = new Array();
			
			if(tile.group) {
				for each(var t:Tile in tile.group.tiles) {
					var items:Array = t.getMenuItems(tile.group ? tile.group.tiles : new ArrayCollection([tile]), tile.group);
					
					for(var i:int=0; i<items.length; i++) {
						var item:PieMenuItem = items[i];
						
						
						if(uniqueIDs[item.uniqueID] != true) {
							newItems.push(item);
							uniqueIDs[item.uniqueID] = true;
						}
					}
				}
			}
			else {
				newItems = tile.getMenuItems(new ArrayCollection([tile]), null); 
			}
			
			return newItems;
		}
		
		private function calculateMenuAngle(position:Point, width:Number, height:Number):Number {
			var angle:Number = 0;
			
			if(position.y < height/2) {
				angle = 180;
			}
			
			if(position.x < width/2) {
				if(position.y < height/2) {
					angle = -135;
				}
				else if(position.y > this.height - height) {
					angle = -45;
				}
				else {
					angle = -90;
				}
			}
			else if(position.x > this.width - width) {
				if(position.y < height/2) {
					angle = 135;
				}
				else if(position.y > this.height - height) {
					angle = 45;
				}
				else {
					angle = 90;
				}
			}
			
			return angle;
		}
		
		private function hideMenu(event:MouseEvent):void {
			var menu:PieMenu = event.target as PieMenu;
			menu.dataProvider = new ArrayCollection();
			
			if(menuTile) {
				menuTile = null;
			}
		}
		
		private function menuHidden(event:FlexEvent):void {
			menusShowing = 0;
		}
		
		private function dragEnd(event:MouseEvent):void {
			dragging = false;
			draggingTile = null;
			systemManager.removeEventListener(MouseEvent.MOUSE_UP, dragEnd);
		}
		
		/*#############################
		 * 
		 * Tile management (adding, deleting)
		 * 
		 *###########################*/
		 
		public function addTiles(array:Array):void {
			for each(var tile:Tile in array) {
				addTile(tile);
			}
		}
		
		public function addTile(tile:Tile):void {
			var x:Number;
			var y:Number;
			
			if(!tile.originPoint) {
				x = this.width/2 - 10 + Math.random()*20;
				y = this.height/2 - 10 + Math.random()*20;
			}
			else {
				x = tile.originPoint.x + Math.random();
				y = tile.originPoint.y + Math.random();
			}	
			addTileAt(tile, x, y);
		}
		
		private function addTileAt(tile:Tile, x:Number, y:Number):void {
			tiles.push(tile);
			
			var tile3D:Base3DTile = tile.tile3D;
			var particle:TileUIParticle = tile.particle;
			particle.speed = -5 + Math.random()*10;
			
			
			ape.addParticle(particle);
			pv3d.addTile(tile3D);
			
			tile.tileAddedToComponents();
			
			tile.addEventListener(InteractiveScene3DEvent.OBJECT_DOUBLE_CLICK, tileDoubleClick);
			tile.addEventListener(InteractiveScene3DEvent.OBJECT_PRESS, tileMouseDown);
			
			TweenManager.addTween(particle, {speed:0, time:.5});
			
			particle.px = x;
			particle.py = y;
		}
		
		private function removeTilesByMenu(array:ArrayCollection, menu:PieMenu=null):void {
			removeTiles(array);
		}
		
		public function removeAllTiles():void {
			removeTiles(new ArrayCollection(tiles));
		}
		
		public function removeTiles(array:ArrayCollection):void {
			for(var i:int=array.length-1; i>=0; i--) {
				removeTile(array[i] as Tile);
			}
		}
		
		public function removeTile(tile:Tile):void {
			
			tiles.splice(tiles.indexOf(tile), 1);
			
			if(tile.group) {
				tile.group.removeTile(tile, true);
			}
			
			tile.tile3D.disconnectFromPhysics = true;
			
			killTile(tile);
		}
		
		private function killTile(tile:Tile):void {
			ape.removeParticle(tile.particle);
			pv3d.removeTile(tile.tile3D);
		}
		
		
		/*#############################
		 * 
		 * Tile selection handling
		 * 
		 *###########################*/
		 
		private var selection:Array;
		
		private function checkSelection(event:Event=null):void {
			selection = new Array();
			
			for each(var tile:Tile in tiles)
			{
				var points:Array = PV3DUtils.getScreenCoordinates(tile.tile3D, pv3d.camera);
				
				for each(var point:Point in points) {
					
					point.x += this.width/2;
					point.y += drawingCanvas.y;
					point.y += this.height/2;
					
					point = this.localToGlobal(point);
					
					if(drawingCanvas.hitTestComponent.hitTestPoint(point.x, point.y, true)) {
						selection.push(tile);
						break;
					}
				}
			}
		}
		
		private function selectionReleased(event:Event):void {
			if(selection == null || selection.length == 0) return;
			
			var tileGroup:TileGroup;
			
			switch(drawingCanvas.mode) {
				case DrawingCanvas.MESSY:
					for each(var tile:Tile in tiles) {
						tile.particle.px += Math.random();
						tile.particle.py += Math.random();
					}
					tileGroup = makePile(new MessyPileLayout(), ape.defaultGroup, new ArrayCollection(selection));
					break;
				case DrawingCanvas.SPACED:
					tileGroup = makePile(new SpacedLayout(), ape.defaultGroup, new ArrayCollection(selection));
					break;
				case DrawingCanvas.SPIRAL:
					var layout:SpiralLayout = new SpiralLayout();
					layout.rotation = 5;
					tileGroup = makePile(layout, ape.defaultGroup, new ArrayCollection(selection));
					break;
				case DrawingCanvas.STACKED:
					tileGroup =makePile(new SpiralLayout(), ape.defaultGroup, new ArrayCollection(selection));
					break;
				case DrawingCanvas.REMOVE:
					removeTiles(new ArrayCollection(selection));
				case DrawingCanvas.RESIZE:
					resizePile(new ArrayCollection(selection));
					break;		
			}
			
			selection = [];
		}
		
		
		/*#############################
		 * 
		 * Pile manipulation
		 * 
		 *###########################*/
		 
		private function rotateTiles(tiles:ArrayCollection, menu:PieMenu):void {
			var regPoint:Point = new Point(stage.mouseX,stage.mouseY);
			
			var tile:Tile = tiles[0];
			
			var mouseSlider:MouseSlider = new MouseSlider(regPoint, systemManager, 200, MouseSlider.HORIZONTAL);
			mouseSlider.targetGroup= tile.group;
			mouseSlider.addEventListener(Event.CHANGE, rotateMouseSliderChange);
		}
		
		private function rotateMouseSliderChange(event:Event):void {
			var slider:MouseSlider = event.target as MouseSlider;
			
			var group:TileGroup = slider.targetGroup;
			
			var value:Number = slider.value;
			
			if(group.layout is SpiralLayout) {
				if(Math.abs(value) < .1) {
					value = 0;
				}
				else {
					if(value < 0) {
						value += .1;
					}
					else {
						value -= .1;
					}
				}
				
				(group.layout as SpiralLayout).rotation = value * 20;
			}
		}
		
		private function resizePile(tiles:ArrayCollection, menu:PieMenu=null):void {
			var regPoint:Point = new Point(stage.mouseX,stage.mouseY);
			
			var mouseSlider:MouseSlider = new MouseSlider(regPoint, systemManager, 200, MouseSlider.HORIZONTAL);
			
			mouseSlider.targetTiles = tiles;
			mouseSlider.addEventListener(Event.CHANGE, resizeSliderChange);
		}
		
		private function resizeSliderChange(event:Event):void {
			var slider:MouseSlider = event.target as MouseSlider;
			
			var value:Number = slider.value;
			
			var tiles:ArrayCollection = slider.targetTiles;
			
			var newScale:Number = Math.max((value + 1), .3);
			
			for each(var tile:Tile in tiles) {
				TweenManager.addTween(tile, {scale:newScale, time:.6});
			}
		}
		
		
		/*#############################
		 * 
		 * Pile creation and interaction
		 * 
		 *###########################*/
		 
		private var currentTileGroup:TileGroup;
		
		/**
		 * Fanning Interaction
		 */
		private function fanTiles(tiles:ArrayCollection, menu:PieMenu):void {
			currentTileGroup = tiles[0].group;
			
			currentTileGroup.layout = new FanLayout();
			
			systemManager.addEventListener(MouseEvent.MOUSE_MOVE, fanTilesToMouse);
			systemManager.addEventListener(MouseEvent.MOUSE_UP, fanTilesRelease, true);
			
			createSelectionPlaneForCurrentGroup();
		}
		
		private function fanTilesToMouse(event:MouseEvent):void {
			if(event is VirtualMouseMouseEvent) return;
			
			var point:Point = new Point(event.stageX, event.stageY);
			point = this.globalToLocal(point);
			(currentTileGroup.layout as FanLayout).fanTo = point;
		}
		
		private function fanTilesRelease(event:MouseEvent):void {
			if(event is VirtualMouseMouseEvent) return;
			
			event.stopImmediatePropagation();
			event.stopPropagation();
			
			systemManager.removeEventListener(MouseEvent.MOUSE_MOVE, fanTilesToMouse);
			systemManager.removeEventListener(MouseEvent.MOUSE_UP, fanTilesRelease, true);
		}
		
		
		/**
		 * Flip page interaction
		 */
		private function leafTiles(tiles:ArrayCollection, menu:PieMenu):void {
			var regPoint:Point = new Point(stage.mouseX,stage.mouseY);
			
			var tile:Tile = tiles[0];
			
			currentTileGroup = tiles[0].group;
			
			var leafLayout:LeafLayout = new LeafLayout();
			leafLayout.rotation = (currentTileGroup.layout as SpiralLayout).rotation;
			
			currentTileGroup.layout = leafLayout;
			
			var mouseSlider:MouseSlider = new MouseSlider(regPoint, systemManager, 200, MouseSlider.HORIZONTAL);
			mouseSlider.targetGroup = tile.group;
			mouseSlider.addEventListener(Event.CHANGE, leafMouseSliderChange);
			mouseSlider.addEventListener(MouseEvent.MOUSE_UP, leafTilesRelease);
			
			createSelectionPlaneForCurrentGroup();
		}
		
		private function leafMouseSliderChange(event:Event):void {
			var slider:MouseSlider = event.target as MouseSlider;
			
			var group:TileGroup = slider.targetGroup;
			var layout:LeafLayout = group.layout as LeafLayout;
			
			layout.direction = (slider.value > 0) ? "right" : "left";
			layout.percent = Math.abs(slider.value);
		}
		
		
		private function leafTilesRelease(event:MouseEvent):void {
			event.stopImmediatePropagation();
			event.stopPropagation();
			
			var mouseSlider:MouseSlider = event.target as MouseSlider;
			
			mouseSlider.removeEventListener(Event.CHANGE, leafMouseSliderChange);
			mouseSlider.removeEventListener(MouseEvent.MOUSE_UP, leafTilesRelease);
		}
		
		
		/**
		 * Grid Layout interaction
		 */
		private function tileTiles(tiles:ArrayCollection, menu:PieMenu):void {
			currentTileGroup = tiles[0].group;
			
			currentTileGroup.layout = new TiledLayout();
			
			systemManager.addEventListener(MouseEvent.MOUSE_MOVE, tiledTilesToMouse);
			systemManager.addEventListener(MouseEvent.MOUSE_UP, tiledTilesRelease, true);
			
			createSelectionPlaneForCurrentGroup();
		}
		
		private function tiledTilesToMouse(event:MouseEvent):void {
			if(event is VirtualMouseMouseEvent) return;
			
			var point:Point = new Point(event.stageX, event.stageY);
			point = this.globalToLocal(point);
			
			(currentTileGroup.layout as TiledLayout).tileTo = point;
		}
		
		private function tiledTilesRelease(event:MouseEvent):void {
			event.stopImmediatePropagation();
			event.stopPropagation();
			
			systemManager.removeEventListener(MouseEvent.MOUSE_MOVE, tiledTilesToMouse);
			systemManager.removeEventListener(MouseEvent.MOUSE_UP, tiledTilesRelease, true);
		}
		
		
		/**
		 * Selection background overlays and close buttons
		 */
		private var numSelectionPlanes:int=0;
		private var selectionPlanesForGroups:Dictionary = new Dictionary(true);
			
		private function createSelectionPlaneForCurrentGroup():void {
			var selectionPlane:SelectionPlane = createTileGridBackground(currentTileGroup);
			
			if(numSelectionPlanes>0) {
				removeEventListener(Event.ENTER_FRAME, enterFrame_selectionPlaneDrawingHandler);
				addEventListener(Event.ENTER_FRAME, enterFrame_selectionPlaneDrawingHandler);
			}
		}
		
		private function enterFrame_selectionPlaneDrawingHandler(event:Event):void {
			if(numSelectionPlanes == 0) {
				removeEventListener(Event.ENTER_FRAME, enterFrame_selectionPlaneDrawingHandler);
			}
			else {
				drawAllSelectionPlanes();
			}
		}
		
		private function drawAllSelectionPlanes():void {
			var allGroups:Array = [];
			
			for each(var plane:SelectionPlane in selectionPlanesForGroups) {
				allGroups.push(plane.tileGroup);
			}	
			
			for each(var group:TileGroup in allGroups) {
				createTileGridBackground(group);
			}
		}
		
		private function createTileGridBackground(tileGroup:TileGroup):SelectionPlane {
			
			if(tileGroup == null || tileGroup.tiles == null || tileGroup.tiles.length == 0) return null;
			
			var max3DCoords:Array = calculateMax3DXYCoordinatesForTiles(tileGroup.tiles.source, 60);
			
			var topLeft:Point = new Point(max3DCoords[0], max3DCoords[1]);
			var botRight:Point = new Point(max3DCoords[2], max3DCoords[3]);
			
			var w:Number = botRight.x - topLeft.x;
			var h:Number = botRight.y - topLeft.y;
			
			if(w < 1 || h < 1) {
				return null;
			}
			
			var x:Number = topLeft.x + (botRight.x - topLeft.x)/2 - 20;
			var y:Number = topLeft.y + (botRight.y - topLeft.y)/2 - 20;
			
			var selectionPlane:SelectionPlane = selectionPlanesForGroups[tileGroup];
			
			if(selectionPlane) {
				if(selectionPlane.width == w && selectionPlane.height == h && selectionPlane.x == x && selectionPlane.z == y) {
					return selectionPlane;
				}
				
				pv3d.removeDO3DFromLayer(selectionPlane, "selectionDrawing");
			}
			else {
				numSelectionPlanes++;
			}
			
			selectionPlane = new SelectionPlane(botRight.x - topLeft.x, botRight.y - topLeft.y);
			selectionPlane.rotationX = 90;
			selectionPlane.x = x;
			selectionPlane.z = y;
			selectionPlane.tileGroup = tileGroup;
			
			selectionPlane.addEventListener(CloseEvent.CLOSE, selectionPlane_closeHandler, false, 0, true);
			
			selectionPlanesForGroups[tileGroup] = selectionPlane;
			
			pv3d.addDO3DToLayer(selectionPlane, "selectionDrawing");
			
			return selectionPlane;
		}
		
		private function selectionPlane_closeHandler(event:CloseEvent):void {
			var selectionPlane:SelectionPlane = event.target as SelectionPlane;
			var tiles:ArrayCollection = selectionPlane.tileGroup.tiles;
			releaseLayout(tiles);
			
			removeSelectionPlane(selectionPlanesForGroups[selectionPlane.tileGroup]);
		}
		
		private function removeSelectionPlane(plane:SelectionPlane):void {
			plane.removeEventListener(CloseEvent.CLOSE, selectionPlane_closeHandler);
			
			pv3d.removeDO3DFromLayer(plane, "selectionDrawing");
			delete selectionPlanesForGroups[plane.tileGroup];
			
			numSelectionPlanes--;
			
			drawAllSelectionPlanes();
		}
		
		private function calculateMax3DXYCoordinatesForTiles(array:Array, buffer:Number=0):Array {
			var minX:Number;
			var minY:Number;
			var maxX:Number;
			var maxY:Number;
			
			for each(var tile:Tile in array) {
				
				if(isNaN(minX) || tile.tile3D.x < minX) {
					minX = tile.tile3D.x;
				}
				
				if(isNaN(maxX) || tile.tile3D.x > maxX) {
					maxX = tile.tile3D.x;
				}
				
				if(isNaN(minY) || tile.tile3D.z < minY) {
					minY = tile.tile3D.z;
				}
				
				if(isNaN(maxY) || tile.tile3D.z > maxY) {
					maxY = tile.tile3D.z;
				}
			}
			
			return [minX-buffer, minY-buffer, maxX+buffer*2, maxY+buffer*2];
		}
		
		
		
		
		/**
		 * Messy Pile Layout creation
		 */
		private function makeMessyPile(tiles:ArrayCollection, menu:PieMenu):void {
			var tile0:Tile = tiles[0] as Tile;
			if(tile0.group) {
				tile0.group.breakGroup();
			}
			
			makePile(new MessyPileLayout(), ape.defaultGroup, tiles);
		}
		
		
		/**
		 * Stack creation
		 */
		private function makeStack(tiles:ArrayCollection, menu:PieMenu):void {
			var tile0:Tile = tiles[0] as Tile;
			if(tile0.group) {
				tile0.group.breakGroup();
			}
			
			makePile(new SpiralLayout(), ape.defaultGroup, tiles);
		}
		
		private function makePile(layout:IGroupLayout, physicsGroup:IPhysicsGroup, tiles:ArrayCollection):TileGroup {
			for each(var tile:Tile in tiles) {
				pv3d.removeTile(tile.tile3D);
				pv3d.addTileToSelectionLayer(tile.tile3D);
			}
			
			return new TileGroup(layout, physicsGroup, tiles);
		}
		
		private function breakTiles(tiles:ArrayCollection, menu:PieMenu):void {
			var firstTile:Tile = tiles[0];
			firstTile.group.breakGroup();
			
			for each(var tile:Tile in tiles) {
				pv3d.removeTileFromSelectionLayer(tile.tile3D);
				pv3d.addTile(tile.tile3D);
			}
		}
		
		private function releaseMenuHide(event:FlexEvent):void {
			var tiles:ArrayCollection = (event.currentTarget as PieMenu).tiles;
			releaseLayout(tiles);
		}
		
		private function releaseLayout(tiles:ArrayCollection, menu:PieMenu=null):void {
			pv3d.selectionDrawingSprite.graphics.clear();
			
			if(tiles.length == 0) return;
			
			
			if(menu) {
				menu.removeEventListener(FlexEvent.HIDE, releaseMenuHide);
			}
			
			var tile:Tile = tiles[0];
			tile.group.previousLayout.tiles = tile.group.tiles;
			
			tile.group.layout = tile.group.previousLayout;
		}
	
		
		/*#############################
		 * 
		 * 3D camera settings
		 * 
		 *###########################*/
		public function resetPerspective():void {
			pv3dBG.resetCameraPosition();
			pv3d.resetCameraPosition();
			pv3dBG.singleRender();
		}
		
		private function adjustPerspective(event:SliderEvent):void {
			pv3d.camera.z = event.value;
			pv3dBG.camera.z = event.value;
			
			var slider:VSlider = event.target as VSlider;
			
			var percentage:Number = 1 - (event.value - slider.minimum)/(slider.maximum - slider.minimum);
			
			var zoom:Number = 60 + percentage*14;
			pv3d.camera.zoom = pv3d.camera.focus = zoom;
			pv3dBG.camera.zoom = pv3dBG.camera.focus = zoom;
		}
		
		public function moveCameraX(value:Number):void {
			pv3d.camera.x = pv3dBG.camera.x = value;
			pv3dBG.singleRender();
		}
		
		public function moveCameraY(value:Number):void {
			pv3d.camera.y = pv3dBG.camera.y = value;
			pv3dBG.singleRender();
		}
		
		public function moveCameraZ(value:Number):void {
			pv3d.camera.z = pv3dBG.camera.z = value;
			pv3dBG.singleRender();
		}
		
		public function zoom(value:Number):void {
			pv3d.camera.zoom = pv3d.camera.focus = pv3dBG.camera.zoom = pv3dBG.camera.focus = value;
			pv3dBG.singleRender();
		}
		
		/*#############################
		 * 
		 * Modules (registering and event handlers)
		 * 
		 *###########################*/
		/**
		 * Registers the module with the TileUI component so the component will listen for TILES_ADDED and
		 * TILE_REMOVED events and add and remove the tiles appropriately. You only need to use the <code>registerModule</code>
		 * method if you have loaded your module yourself (ie not via the <code>loadModule</code> method or the
		 * <code>addModuleDirectly</code> method. For example, if you define your module in MXML in you main application
		 * and then want to register it with the TileUI component, you would call <code>registerModule</code>.
		 */
		public function registerModule(module:TileUIModule):void {
			module.addEventListener(TileUIModuleEvent.TILES_ADDED, moduleTilesAdded);
			module.addEventListener(TileUIModuleEvent.TILES_REMOVED, moduleTilesRemoved);
		}
		
		/**
		 * @private
		 *
		 * When we receive a TILES_ADDED event we call the <code>addTiles</code> method and check to see if we're
		 * supposed to create a new TileGroup for the newly added tiles.
		 */
		private function moduleTilesAdded(event:TileUIModuleEvent):void {
			addTiles(event.tileArray);
			
			//since a TILES_ADDED event might include a default group to load the tiles as (ie stack or messy, etc) then
			//we need to create that group if needed. The act of creating a new TileGroup implicitly does the layout stuff
			if(event.groupLayout) {
				var tileGroup:TileGroup = makePile(event.groupLayout, ape.defaultGroup, new ArrayCollection(event.tileArray));
			}
		}
		
		private function moduleTilesRemoved(event:TileUIModuleEvent):void {
			removeTiles(new ArrayCollection(event.tileArray));
		}
	}
}