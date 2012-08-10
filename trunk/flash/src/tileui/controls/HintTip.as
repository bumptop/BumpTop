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

package tileui.controls
{
	import mx.controls.ToolTip;
	import mx.controls.Button;
	import flash.events.Event;
	import mx.events.CloseEvent;
	import flash.events.MouseEvent;

	[Event(name="close", type="mx.events.CloseEvent")]
	
	[ExcludeClass]
	
	public class HintTip extends ToolTip
	{
		private var closeButton:Button;
	
		[Embed(source="../../assets/assets.swf#close_icon_up")]
		private var upSkin:Class;	
		
		[Embed(source="../../assets/assets.swf#close_icon_over")]
		private var overSkin:Class;
		
		override protected function measure():void {
			super.measure();
			
			this.measuredWidth += 20;
		}
		override protected function createChildren():void {
			super.createChildren();
			
			closeButton = new Button();
			closeButton.toolTip = "Hide this hint";
			
			closeButton.setStyle("upSkin", 	upSkin);
			closeButton.setStyle("overSkin", overSkin);
			closeButton.setStyle("downSkin", upSkin);
			
			closeButton.addEventListener(MouseEvent.CLICK, dispatchClose);
			
			closeButton.width = closeButton.height = 13;
			addChild(closeButton);
			
		}
		
		private function dispatchClose(event:Event):void {
			dispatchEvent(new CloseEvent(CloseEvent.CLOSE));
		}
		
		override protected function updateDisplayList(unscaledWidth:Number, unscaledHeight:Number):void {
			super.updateDisplayList(unscaledWidth, unscaledHeight);
			
			closeButton.setActualSize(13,13);
			
			closeButton.move(3, 4);
			
			textField.move(closeButton.x + closeButton.width + 4, textField.y);
		}
	}
}