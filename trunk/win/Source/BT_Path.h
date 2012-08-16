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

#ifndef _BT_PATH_
#define _BT_PATH_

#include "BT_Common.h"
#include "BT_TouchPoint.h"

class Path
{
private:
	// The minimum velocity (in pixels/sample) that qualifies as movement
	// TODO: This should be made independent of the sample rate
	static const float MOVEMENT_VELOCITY_THRESHOLD;

	// Max ratio of total length to displacement length for a "straight" line
	static const float STRAIGHTNESS_THRESHOLD;

	// The maximum ratio of horizontal deviation in a vertical line, or
	// vertical deviation in a horizontal line, over the length of the line
	static const float HORIZONTAL_OR_VERTICAL_SLOPE_THRESHOLD;
public:
	Path();
	Path(QColor pathColor);
		
	// Contains the TouchPoints with the same id created by down, move and up events
	QList<TouchPoint> points;

	// Arc length of the path
	float pathLength;

	// Path color
	QColor color;
	
	Vec3 getTotalDisplacementVector();
	int getTotalDisplacementSlope();
	Vec3 getLastAverageVelocity(int num);
	Vec3 getCurrentVelocity();
	bool isStationary();
	
	bool isApproximatelyStraight();
	bool isApproximatelyHorizontal();
	bool isApproximatelyVertical();

	TouchPoint& getFirstTouchPoint();
	TouchPoint& getLastTouchPoint();
	TouchPoint& getTouchPointAt(DWORD timeStamp);
};

#endif /* END OF _BT_PATH_ */