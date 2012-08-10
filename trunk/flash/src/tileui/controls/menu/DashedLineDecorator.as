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

////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2008 The Degrafa Team : http://www.Degrafa.com/team
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
//
// Algorithms based on code from Trevor McCauley, www.senocular.com
////////////////////////////////////////////////////////////////////////////////
package tileui.controls.menu{
	
	import com.degrafa.decorators.RenderDecoratorBase;
	
	import flash.display.Graphics;
	
	public class DashedLineDecorator extends RenderDecoratorBase{
		/**
		* A value representing the accuracy used in determining the length
		* of curveTo curves.
		*/
		public var _curveaccuracy:Number =6;
			
		private var isLine:Boolean = true;
		private var overflow:Number = 0;
		private var offLength:Number = 20;
		private var onLength:Number = 20;
		private var dashLength:Number = 40;
		private var pen:Object={x:0, y:0};

		public function DashedLineDecorator(){
			super();
		}
				
		/**
		 * Sets new lengths for dash sizes
		 * @param onLength Length of visible dash lines.
		 * @param offLength Length of space between dash lines.
		 * @return nothing
		 */
		public function setDash(onLength:Number, offLength:Number):void {
			onLength = onLength;
			offLength = offLength;
			dashLength = onLength + offLength;
		}
		/**
		 * Gets the current lengths for dash sizes
		 * @return Array containing the onLength and offLength values
		 * respectively in that order
		 */
		public function getDash():Array {
			return [onLength, offLength];
		}
		
		/**
		* Moves the current drawing position in graphics to (x, y).
		*/
		override public function moveTo(x:Number, y:Number,graphics:Graphics):void {
			doMoveTo(x,y,graphics);
		}
		
		/**
		* Draws a dashed line in graphics from the current drawing position
		* to (x, y).
		*/
		override public function lineTo(x:Number,y:Number,graphics:Graphics):void {
			var dx:Number = x-pen.x
			var dy:Number = y-pen.y;
			var a:Number = Math.atan2(dy, dx);
			var ca:Number = Math.cos(a)
			var sa:Number = Math.sin(a);
			var segLength:Number = lineLength(dx, dy);
			if (overflow){
				if (overflow > segLength){
					if (isLine) doLineTo(x,y,graphics);
					else doMoveTo(x,y,graphics);
					overflow -= segLength;
					return;
				}
				if (isLine) doLineTo(pen.x + ca*overflow, pen.y + sa*overflow,graphics);
				else doMoveTo(pen.x + ca*overflow, pen.y + sa*overflow,graphics);
				segLength -= overflow;
				overflow = 0;
				isLine = !isLine;
				if (!segLength) return;
			}
			var fullDashCount:int = Math.floor(segLength/dashLength);
			if (fullDashCount){
				var onx:Number = ca*onLength
				var ony:Number = sa*onLength;
				var offx:Number = ca*offLength
				var offy:Number = sa*offLength;
				for (var i:int=0; i<fullDashCount; i++){
					if (isLine){
						doLineTo(pen.x+onx, pen.y+ony,graphics);
						doMoveTo(pen.x+offx, pen.y+offy,graphics);
					}else{
						doMoveTo(pen.x+offx, pen.y+offy,graphics);
						doLineTo(pen.x+onx, pen.y+ony,graphics);
					}
				}
				segLength -= dashLength*fullDashCount;
			}
			if (isLine){
				if (segLength > onLength){
					doLineTo(pen.x+ca*onLength, pen.y+sa*onLength,graphics);
					doMoveTo(x, y,graphics);
					overflow = offLength-(segLength-onLength);
					isLine = false;
				}else{
					doLineTo(x, y,graphics);
					if (segLength == onLength){
						overflow = 0;
						isLine = !isLine;
					}else{
						overflow = onLength-segLength;
						doMoveTo(x, y,graphics);
					}
				}
			}else{
				if (segLength > offLength){
					doMoveTo(pen.x+ca*offLength, pen.y+sa*offLength,graphics);
					doLineTo(x, y,graphics);
					overflow = onLength-(segLength-offLength);
					isLine = true;
				}else{
					doMoveTo(x, y,graphics);
					if (segLength == offLength){
						overflow = 0;
						isLine = !isLine;
					}else overflow = offLength-segLength;
				}
			}
		}
		
		/**
		* Draws a dashed curve in graphics using the current from the current drawing position to
		* (x, y) using the control point specified by (cx, cy).
		*/
		override public function curveTo(cx:Number, cy:Number, x:Number, y:Number,graphics:Graphics):void {
			var sx:Number = pen.x;
			var sy:Number = pen.y;
			var segLength:Number = curveLength(sx, sy, cx, cy, x, y,_curveaccuracy);
			var t:Number = 0;
			var t2:Number = 0;
			var c:Array;
			if (overflow){
				if (overflow > segLength){
					if (isLine) doCurveTo(cx, cy, x, y,graphics);
					else doMoveTo(x, y,graphics);
					overflow -= segLength;
					return;
				}
				t = overflow/segLength;
				c = curveSliceUpTo(sx, sy, cx, cy, x, y, t);
				if (isLine) doCurveTo(c[2], c[3], c[4], c[5],graphics);
				else doMoveTo(c[4], c[5],graphics);
				overflow = 0;
				isLine = !isLine;
				if (!segLength) return;
			}
			
			var remainLength:Number = segLength - segLength*t;
			var fullDashCount:int = Math.floor(remainLength/dashLength);
			var ont:Number = onLength/segLength;
			var offt:Number = offLength/segLength;
			if (fullDashCount){
				for (var i:int=0; i<fullDashCount; i++){
					if (isLine){
						t2 = t + ont;
						c = curveSlice(sx, sy, cx, cy, x, y, t, t2);
						doCurveTo(c[2], c[3], c[4], c[5],graphics);
						t = t2;
						t2 = t + offt;
						c = curveSlice(sx, sy, cx, cy, x, y, t, t2);
						doMoveTo(c[4], c[5],graphics);
					}else{
						t2 = t + offt;
						c = curveSlice(sx, sy, cx, cy, x, y, t, t2);
						doMoveTo(c[4], c[5],graphics);
						t = t2;
						t2 = t + ont;
						c = curveSlice(sx, sy, cx, cy, x, y, t, t2);
						doCurveTo(c[2], c[3], c[4], c[5],graphics);
					}
					t = t2;
				}
			}
			remainLength = segLength - segLength*t;
			if (isLine){
				if (remainLength > onLength){
					t2 = t + ont;
					c = curveSlice(sx, sy, cx, cy, x, y, t, t2);
					doCurveTo(c[2], c[3], c[4], c[5],graphics);
					doMoveTo(x, y,graphics);
					overflow = offLength-(remainLength-onLength);
					isLine = false;
				}else{
					c = curveSliceFrom(sx, sy, cx, cy, x, y, t);
					doCurveTo(c[2], c[3], c[4], c[5],graphics);
					if (segLength == onLength){
						overflow = 0;
						isLine = !isLine;
					}else{
						overflow = onLength-remainLength;
						doMoveTo(x, y,graphics);
					}
				}
			}else{
				if (remainLength > offLength){
					t2 = t + offt;
					c = curveSlice(sx, sy, cx, cy, x, y, t, t2);
					doMoveTo(c[4], c[5],graphics);
					c = curveSliceFrom(sx, sy, cx, cy, x, y, t2);
					doCurveTo(c[2], c[3], c[4], c[5],graphics);
					
					overflow = onLength-(remainLength-offLength);
					isLine = true;
				}else{
					doMoveTo(x, y,graphics);
					if (remainLength == offLength){
						overflow = 0;
						isLine = !isLine;
					}else overflow = offLength-remainLength;
				}
			}
		}
		
		private function doMoveTo(x:Number, y:Number,graphics:Graphics):void {
			pen = {x:x, y:y};
			graphics.moveTo(x, y);
		}
		private function doLineTo(x:Number, y:Number,graphics:Graphics):void {
			if (x == pen.x && y == pen.y) return;
			pen = {x:x, y:y};
			graphics.lineTo(x, y);
		}
		private function doCurveTo(cx:Number, cy:Number, x:Number, y:Number,graphics:Graphics):void {
			if (cx == x && cy == y && x == pen.x && y == pen.y) return;
			pen = {x:x, y:y};
			graphics.curveTo(cx, cy, x, y);
		}
		
		//the below to be moved into a shared class.
								
		// private methods
		private function lineLength(sx:Number, sy:Number, ex:Number=0, ey:Number=0):Number {
			if (arguments.length == 2) return Math.sqrt(sx*sx + sy*sy);
			var dx:Number = ex - sx;
			var dy:Number = ey - sy;
			return Math.sqrt(dx*dx + dy*dy);
		}
		private function curveLength(sx:Number, sy:Number, cx:Number, cy:Number, ex:Number, ey:Number, accuracy:Number):Number {
			var total:Number = 0;
			var tx:Number = sx;
			var ty:Number = sy;
			var px:Number, py:Number, t:Number, it:Number, a:Number, b:Number, c:Number;
			var n:Number = (accuracy) ? accuracy : _curveaccuracy;
			for (var i:Number = 1; i<=n; i++){
				t = i/n;
				it = 1-t;
				a = it*it; b = 2*t*it; c = t*t;
				px = a*sx + b*cx + c*ex;
				py = a*sy + b*cy + c*ey;
				total += lineLength(tx, ty, px, py);
				tx = px;
				ty = py;
			}
			return total;
		}
		private function curveSlice(sx:Number, sy:Number, cx:Number, cy:Number, ex:Number, ey:Number, t1:Number, t2:Number):Array {
			if (t1 == 0) return curveSliceUpTo(sx, sy, cx, cy, ex, ey, t2);
			else if (t2 == 1) return curveSliceFrom(sx, sy, cx, cy, ex, ey, t1);
			var c:Array = curveSliceUpTo(sx, sy, cx, cy, ex, ey, t2);
			c.push(t1/t2);
			return curveSliceFrom.apply(this, c);
		}
		private function curveSliceUpTo(sx:Number, sy:Number, cx:Number, cy:Number, ex:Number, ey:Number, t:Number):Array {
			if (isNaN(t)) t = 1;
			if (t != 1) {
				var midx:Number = cx + (ex-cx)*t;
				var midy:Number = cy + (ey-cy)*t;
				cx = sx + (cx-sx)*t;
				cy = sy + (cy-sy)*t;
				ex = cx + (midx-cx)*t;
				ey = cy + (midy-cy)*t;
			}
			return [sx, sy, cx, cy, ex, ey];
		}
		private function curveSliceFrom(sx:Number, sy:Number, cx:Number, cy:Number, ex:Number, ey:Number, t:Number):Array {
			if (isNaN(t)) t = 1;
			if (t != 1) {
				var midx:Number = sx + (cx-sx)*t;
				var midy:Number = sy + (cy-sy)*t;
				cx = cx + (ex-cx)*t;
				cy = cy + (ey-cy)*t;
				sx = midx + (cx-midx)*t;
				sy = midy + (cy-midy)*t;
			}
			return [sx, sy, cx, cy, ex, ey];
		}
		
	}
}