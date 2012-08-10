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

package tileui.controls.menu
{
	import mx.binding.utils.BindingUtils;
	import mx.charts.HitData;
	import mx.charts.PieChart;
	import mx.charts.effects.SeriesSlide;
	import mx.charts.effects.SeriesZoom;
	import mx.charts.events.ChartItemEvent;
	import mx.charts.series.PieSeries;
	import mx.charts.series.items.PieSeriesItem;
	import mx.collections.ArrayCollection;
	import mx.core.Application;
	import mx.core.ClassFactory;
	import mx.events.EffectEvent;
	import mx.events.FlexEvent;
	import mx.graphics.IFill;
	import mx.graphics.SolidColor;
	import mx.graphics.Stroke;
	
	import tileui.controls.menu.pieMenuClasses.MyPieSeries;

	public class PieMenu extends  PieChart
	{
		[Bindable]
		private var mySeries:PieSeries;
		
		private var slideIn:SeriesSlide;
		private var slideOut:SeriesSlide;
		
		public var selectEvent:String=ChartItemEvent.ITEM_MOUSE_DOWN;
		
		
		public function PieMenu():void {
			super();

			var themeColor:Number = Application.application.getStyle("themeColor");
			
			this.setStyle("innerRadius", .2);
			this.showDataTips = true;
			this.dataTipFunction = getDataTip;
			this.dataTipMode = "single";
			this.setStyle("showDataTipTargets", false);
			this.setStyle("dataTipRenderer", new ClassFactory(tileui.controls.menu.pieMenuClasses.DataTipRenderer));
		
			mySeries = new MyPieSeries();
			mySeries.setStyle("itemRenderer", new ClassFactory(tileui.controls.menu.pieMenuClasses.MyWedgeRenderer));
			mySeries.field = "value";
			mySeries.fillFunction = fillFunction;
			mySeries.setStyle("labelPosition", "inside");
			mySeries.labelFunction = getLabel;
			mySeries.setStyle("color", "0xffffff");
			mySeries.setStyle("fontSize", 18);
			mySeries.setStyle("fontWeight", "normal");
			mySeries.setStyle("stroke", new Stroke(0x3A6090, 0, 1));
			mySeries.setStyle("radialStroke", new Stroke(0x3A6090, 0, 1));
			
			mySeries.filters = [];
			seriesFilters = [];
			
			slideIn = new SeriesSlide();
			slideIn.direction = _tweenInDirection;
			mySeries.setStyle("showDataEffect", slideIn);
			
			slideOut = new SeriesSlide();
			slideOut.direction = _tweenOutDirection;
			slideOut.addEventListener(EffectEvent.EFFECT_END, hideIfEmpty);
			mySeries.setStyle("hideDataEffect", slideOut);
			
			BindingUtils.bindProperty(mySeries, "startAngle", this, "startAngle");
			
			this.series = [mySeries];
		}
		
		override protected function createChildren():void {
			super.createChildren();
			
			this.addEventListener(ChartItemEvent.ITEM_ROLL_OVER, hilight);
			this.addEventListener(selectEvent, selectItem);
			
			this.addEventListener(FlexEvent.CREATION_COMPLETE, initDP);
		}
		
		private var _dp:Object;
		
		override public function set dataProvider(value:Object):void {
			_dp = value;
			
			var fills:Array = new Array();
	     	
	     	for(var i:int=0; i<value.length; i++) {
	     		var fill:IFill = fillFunction({item:value[i]}, i);
	     		fills.push(fill);
	     	}
	     	
	     	if(fills.length > 0) {
	     		this.series[0].setStyle("fills", fills);
	     	}
	     	
	     	if(this.initialized) {
				super.dataProvider = value;
			}	
		}
		
		private function initDP(event:FlexEvent):void {
			super.dataProvider = _dp;
		}
		
		override protected function updateDisplayList(unscaledWidth:Number, unscaledHeight:Number):void {
	     	super.updateDisplayList(unscaledWidth, unscaledHeight);
	     	
	        graphics.clear();
	    }
	    
	    public var tiles:ArrayCollection;
	    
	  
	    
	    protected function hilight(event:ChartItemEvent):void {
	     	var item:PieSeriesItem = event.hitData.chartItem as PieSeriesItem;
	     	
	     	
	     	for each(var obj:Object in dataProvider) {
	     		obj.selected = false;
	     	}
	     	
	     	item.item.selected = true;
	     	
	     	var chart:PieChart = event.target as PieChart;
	     	chart.invalidateDisplayList();
	     }
	     
	     protected function selectItem(event:ChartItemEvent):void {
	     	if(event.hitData == null) return;
	     	
	     	var item:PieSeriesItem = event.hitData.chartItem as PieSeriesItem;
	     	var menuItem:Object = item.item;
	     	
	     	if(menuItem.selectFunction) {
	     		var returnVal:* = menuItem.selectFunction(tiles, this);
	     		
	     		if(returnVal is PieMenu) {
	     			var subMenu:PieMenu = returnVal as PieMenu;
	     			subMenu.tiles = tiles;
	     			
	     			this.parent.addChild(subMenu);
	     			subMenu.move(stage.mouseX - subMenu.width/2, stage.mouseY - subMenu.height/2);
	     		}
	     	}
	     	
	     	if(menuItem.closeOnSelect || menuItem.closeOnSelect == null) {
	     		dataProvider = new ArrayCollection();
				updateAllDataTips();
	     	}
	     }
	     
	
	     
	    protected function fillFunction ( element:*, index:Number ):IFill {
	    	var fill:SolidColor = new SolidColor();
	    	
	    	var color1:Number = Application.application.getStyle("themeColor");
	    	
	    	if(element.item.type == "drag") {
	    		color1 = 0xffa800;
	    	}
	    	else if(element.item.type == "remove") {
	    		color1 = 0xff0000;
	    	}
	    	
	    	
	    	var color2:Number = color1 & 0xeeeeee;
	    	
	    	fill.color = color1;
	    	
			if(element.item.label == "") {
	    		fill.alpha = 0;
	    	}
	    	else if(element.item.selected) {
	    		fill.alpha = 1;
	    		fill.color = color2;
	    	}
	    	else {
	    		fill.alpha = .85;
	    	}
	    	
	    	return fill;
		}
		
		
		
		
		
		
		private function getLabel(data:Object, field:String, index:Number, percentValue:Number):String {
			return data.label;
		}
		
		
		
		
		private function hideIfEmpty(event:EffectEvent):void {
			this.visible = (dataProvider.length > 0)
		}
		
		
		
		
		[Bindable]
		private var _tweenInDirection:String = "right";
		
		[Bindable]
		private var _tweenOutDirection:String = "left";
		
		public function set tweenDirection(value:String):void {
			_tweenInDirection = value;
			
			_tweenOutDirection = (_tweenInDirection == "left") ? "right" : "left";
			
			if(slideIn)
				slideIn.direction = _tweenInDirection;
			
			if(slideOut)
				slideOut.direction = _tweenOutDirection;
				
		}
		
		public function get tweenDirection():String {
			return _tweenInDirection;
		}
		
		private function getDataTip(hitData:HitData):String {
			if(hitData.item.type == "drag") {
				return "<b>Drag</b> to " + (hitData.item.action ? hitData.item.action : hitData.item.label);
			}
			else {
				return "<b>Click</b> to " + (hitData.item.action ? hitData.item.action : hitData.item.label);
			}
		}
		
		[Bindable]
		public var startAngle:Number=0;
	}
}