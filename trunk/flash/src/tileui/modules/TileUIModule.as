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
	import mx.modules.Module;
	
	[Event(name="tilesAdded", type="tileui.modules.TileUIModuleEvent")]
	[Event(name="tilesRemoved", type="tileui.modules.TileUIModuleEvent")]
	
	/**
	 * To use the TileUI component you will create a module that extends <code>TileUIModule</code>.
	 * The <code>TileUIModule</code> is a slightly modified <code>Module</code>, which adds the 
	 * <code>title</code> property and the <code>TileUIModuleEvent.TILES_ADDED</code> and
	 * <code>TileUIModuleEvent.TILES_REMOVED</code> events.
	 * 
	 * <p>Within your module you will dispatch the appropriate add or remove events and pass in
	 * and array of <code>tileui.tiles.Tile</code> objects. </p>
	 *
	 * @example To have your module load tiles into TileUI:  
	 * <listing version="3.0">
	 * 		
	 * </listing>
	 */
	public class TileUIModule extends Module
	{
		public function TileUIModule() {
			super();
		}	
	}
}