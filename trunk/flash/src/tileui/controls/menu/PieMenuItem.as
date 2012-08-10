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
	public class PieMenuItem
	{
		public static const CLICK_TYPE:String = "click";
		public static const DRAG_TYPE:String = "drag";
		public static const REMOVE_TYPE:String = "remove";
		
		/**
		 * The label that appears on the menu item. Make it short and sweet.
		 */
		public var label:String;
		
		/**
		 * The string that gets displayed in the tooltip for a menu item. For example, if you
		 * have a menu item that is a click type item and you set the <code>action</code> to "open in browser", the 
		 * tooltip will be "click to open in browser".
		 */
		public var action:String;
		
		/**
		 * A number indicating what percentage of the total pie the menu item takes up. A value of 1 means
		 * the item is counted as a single item. A value of 2 would mean that the item is displayed twice as big
		 * as an item with a value of 1. Makes sense?
		 */
		public var value:Number;
		
		/**
		 * Can either be <code>PieMenuItem.CLICK_TYPE</code>, <code>PieMenuItem.DRAG_TYPE</code>,
		 * or <code>PieMenuItem.REMOVE_TYPE</code>. THis type is used to determine the coloring and the 
		 * tolltip that appears for the menu item.
		 */
		public var type:String;
		
		/**
		 * A callback function that will get called when the menu item is selected.
		 * <p>The callback function must have the following signature:
		 * <code>
		 * function callback(tile:ArrayCollection, menu:PieMenu=null):void {
		 * 		...
		 * }
		 * </code>
		 * </p>
		 * <p>The first parameter is an array of tileui.tiles.Tile objects that contains all the tiles
		 * that are in the group that the menu is acting upon. If the menu does not apply to a group but instead
		 * to a single tile then the <code>tiles</code> parameter is an ArrayCollection with only one item.</p>
		 * <p>The <code>menu</code> parameter is a reference to the <code>PieMenu</code> object that the item
		 * was in. Due to a little legacy code it's possible that this can be null. Woops, just allow it to be null
		 * and you'll be cool.</p>
		 */
		public var selectFunction:Function;
		
		/**
		 * @private
		 */
		public var selected:Boolean;
		
		/**
		 * Boolean indicating if selecting the menu item will close the menu.
		 * <p>Default value is true, which indicates that once the item is selected the menu will be
		 * closed. For many operations this is the desired behavior. However, there may be cases when the
		 * menu should remain open even after the menu item is selected.</p>
		 */
		public var closeOnSelect:Boolean;
		
		public var uniqueID:String;
		
		public function PieMenuItem(label:String, uniqueID:String, action:String=null, value:Number=1, type:String="click", selectFunction:Function=null, closeOnSelect:Boolean=true):void {
			this.label = label;
			this.action = action ? action : label.toLowerCase();
			this.value = value;
			this.type = type;
			this.selectFunction = selectFunction;
			this.closeOnSelect = closeOnSelect;
			this.uniqueID = uniqueID;
			
			this.selected = false;
		}
	}
}