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

package tileui.modules
{
	import flash.events.Event;
	
	import tileui.tiles.layout.IGroupLayout;

	public class TileUIModuleEvent extends Event
	{
		public static const TILES_ADDED:String = "tilesAdded";
		public static const TILES_REMOVED:String = "tilesRemoved";
		
		private var _tileArray:Array;
		
		/**
		 * The Array of <code>Tile</code> objects that are either to be added or removed from the TileUI workspace.
		 * 
		 * @see tileui.tiles.Tile
		 */
		public function get tileArray():Array {
			return _tileArray;
		}
		/**
		 * The layout of the tiles when they are intially added to the TileUI workspace. This allows you to
		 * specify that the tiles should be added as a stack, etc. If <code>null</code> then all
		 * tiles are added individually and are not grouped.
		 * 
		 * @see tileui.tiles.layout.IGroupLayout
		 * @see tileui.tiles.layout.SpacedLayout
		 * @see tileui.tiles.layout.SpiralLayout
		 * @see tileui.tiles.layout.StackLayout
		 */
		public var groupLayout:IGroupLayout;
		
		/**
		 * Craetes a new <code>TileUIModuleEvent</code>.
		 * 
		 * <code>type</code> should be either <code>TileUIModuleEvent.TILES_ADDED</code> or <code>TileUIModuleEvent.TILES_REMOVED</code>. 
		 * <codee>tileArray</code> must be an Array of all the <code>Tile</code> objects that are either being added or removed. If tiles are being added then 
		 * you can also set a default group by passing in an instance of an <code>IGroupLayout</code> (such as <code>StackLayout</code>, 
		 * <code>SpiralLayout</code>, or <code>SpacedLayout</code>).
		 * 
		 * @see tileui.tiles.Tile
		 * @see tileui.tiles.layout.IGroupLayout
		 * @see tileui.tiles.layout.SpacedLayout
		 * @see tileui.tiles.layout.SpiralLayout
		 * @see tileui.tiles.layout.StackLayout
		 */
		public function TileUIModuleEvent(type:String, tileArray:Array = null, groupLayout:IGroupLayout=null, 
								bubbles:Boolean = false,
                                cancelable:Boolean = false
                                )
    	{
    		this.groupLayout = groupLayout;
    		
    		_tileArray = tileArray;
    		
    		super(type, bubbles, cancelable); 
    	}
    	
    	/**
    	 * @private
    	 */
    	override public function clone():Event {
    		return new TileUIModuleEvent(type, _tileArray, groupLayout, bubbles, cancelable);
    	}
	}
}