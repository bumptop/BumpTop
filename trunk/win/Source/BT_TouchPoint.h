// Copyright 2012 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef _BT_TOUCH_POINT_
#define _BT_TOUCH_POINT_

class NxActorWrapper;
class BumpObject;

class TouchPoint
{
public:
	// Enumeration of the different possible states for a touch point
	// If you change the enum, be sure to change STATE_NAMES in the cpp!
	enum State { Down = 0, Move, Up };
	static const char *STATE_NAMES[];

	// Screen co-ordinates (in pixels) of the touch point
	int x;
	int y;

	// The size of the contact point in pixels
	int width;
	int height;

	// The time this point was detected relative to some point in time
	ulong relativeTimeStamp;

	// Velocity of this point
	Vec3 velocity;

	// Colour of the point
	QColor color;

	// Whether this point was detected on a Down, Move, or Up event
	State state;

	TouchPoint();
	TouchPoint(int x_val, int y_val, int width_val=1, int height_val=1);
	
	// Regular getters
	Vec3 getPositionVector();
	
	// Calculation based getters
	Vec3 calculateDisplacementFrom(TouchPoint &touchPoint);
	Vec3 calculateVelocityFrom(TouchPoint &touchPoint);
	Vec3 calculateMidPoint(TouchPoint &touchPoint);
	
	// Getters involving the BumpTop World
	tuple<NxActorWrapper*, BumpObject*, Vec3> pickObject();
	BumpObject* getPickedObject();
	Vec3 unProject(NxPlane& plane = NxPlane(Vec3(0.0f), Vec3(0.0f, 1.0f, 0.0f)));
	tuple<int, NxReal, Vec3, NxPlane> unProjectToDesktop(float padding = 0.0f);
};

#endif /* _BT_TOUCH_POINT_ */
