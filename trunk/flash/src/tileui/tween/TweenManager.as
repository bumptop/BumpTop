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

package tileui.tween
{
	import caurina.transitions.Tweener;
	import mx.effects.easing.Linear;
	
	[ExcludeClass]
	
	/**
	 * <code>TweenManager</code> is a utility class to handle tweening. This allows us to change the Tweening method
	 * (ie from using Tweener to using TweenListe, or something else). All we need to do is change the implementation of
	 * <code>TweenManager.addTween()</code> in this one class.
	 */
	public class TweenManager
	{
		/**
		 * Adds a Tween using the Tweener format (ie passing the object to be tweened and then a vars Object with properties). 
		 * This was made to work easily with Tweener, but may need some massaging to get to work with the format of other Tweening
		 * engines.
		 */
		static public function addTween(object:Object, vars:Object):void {
			Tweener.addTween(object, vars);
		}
	}
}