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

#include "BT_Common.h"
#include "BT_GestureContext.h"
#include "BT_Path.h"
#include "BT_TouchPoint.h"
#include "BT_Util.h"
#include "BT_WindowsOS.h"

const float Path::MOVEMENT_VELOCITY_THRESHOLD = TO_PIXELS(52.08f);	// 5 pixels
const float Path::STRAIGHTNESS_THRESHOLD = 1.05f;
const float Path::HORIZONTAL_OR_VERTICAL_SLOPE_THRESHOLD = 0.08f;

Path::Path() :
	pathLength(0.0f),
	points()
{
	color.setRgb(255, 255, 255);
}

Path::Path(QColor pathColor) :
	pathLength(0.0f),
	points()
{
	color = pathColor;
}

Vec3 Path::getTotalDisplacementVector()
{
	if(points.isEmpty())
		return Vec3(0.0f);
	
	Vec3 first = Vec3(points.first().x, points.first().y, 0.0f);
	Vec3 last = Vec3(points.last().x, points.last().y, 0.0f);
	return (last - first);
}

// Calculate the average velocity of the path over the given number of points.
// TODO: Is a number of points the right thing to do here? Would time be better?
Vec3 Path::getLastAverageVelocity(int numPoints)
{
	if(this->points.isEmpty())
		return Vec3(0.0f);

	Vec3 averageVelocity = Vec3(0.0f);
	int index = 1;
	QListIterator<TouchPoint> it(points);
	it.toBack();
	while(it.hasPrevious() && index <= numPoints)
	{
		TouchPoint touchPoint = it.previous();
		if(touchPoint.state != TouchPoint::Up)
		{
			averageVelocity += touchPoint.velocity;	
			index++;
		}
	}
	averageVelocity /= index;
	return averageVelocity;
}

// Get the current velocity of the path.
// "Current" is defined as the average velocity over the last 4 points.
// TODO: This is currently in pixels/sample, but it should be independent of
// the sample rate (i.e., time-based)
Vec3 Path::getCurrentVelocity()
{
	// 4 is an arbitrary number here that seems to work. Again, this should
	// probably be time-based rather than a particular number of samples
	return getLastAverageVelocity(4);
}

// Return True if the touch point is (relatively) stationary, otherwise False.
bool Path::isStationary()
{
	return getCurrentVelocity().magnitude() < MOVEMENT_VELOCITY_THRESHOLD;
}

bool Path::isApproximatelyStraight()
{
	return (pathLength / getTotalDisplacementVector().magnitude()) < STRAIGHTNESS_THRESHOLD;
}

bool Path::isApproximatelyHorizontal()
{
	if (isApproximatelyStraight()) 
	{
		Vec3 displacement = getTotalDisplacementVector();
		if (displacement.x != 0.0f)
			return fabs(displacement.y / displacement.x) < HORIZONTAL_OR_VERTICAL_SLOPE_THRESHOLD;
	}
	return false;
}

bool Path::isApproximatelyVertical()
{
	if (isApproximatelyStraight())
	{
		Vec3 displacement = getTotalDisplacementVector();
		if (displacement.y != 0.0f)
			return fabs(displacement.x / displacement.y) < HORIZONTAL_OR_VERTICAL_SLOPE_THRESHOLD; 
	}
	return false;
}

TouchPoint& Path::getFirstTouchPoint()
{
	return points.first();
}

TouchPoint& Path::getLastTouchPoint()
{
	return points.last();
}

// Gets the touch point that has the closest relativeTimeStamp to timeStamp
TouchPoint& Path::getTouchPointAt(DWORD timeStamp)
{
	QMutableListIterator<TouchPoint> pointIterator(points);
	
	bool hasBeenSmaller = false;
	
	// Iterate through all the points in the path
	while(pointIterator.hasNext())
	{
		TouchPoint& point = pointIterator.next();
		if (point.relativeTimeStamp < timeStamp)
		{
			// Record the fact that there have been points before the given
			// time stamp.
			hasBeenSmaller = true;
		}
		else if (point.relativeTimeStamp == timeStamp)
		{
			return point;
		}
		else if (hasBeenSmaller)
		{
			// If a previous touch point has had a time stamp less than the 
			// given time stamp, and now the point's time stamp is greater or equal
			// to the given time stamp, then this point is concurrent.
			return point;
		}
		else
		{
			break;
		}
	}
	
	// If no point is available, handle gracefully by returning the first point
	return points.first();
}