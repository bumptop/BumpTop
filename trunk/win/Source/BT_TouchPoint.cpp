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
#include "BT_TouchPoint.h"
#include "BT_Util.h"

const char *TouchPoint::STATE_NAMES[] = { "DOWN", "MOVE", "UP" };

TouchPoint::TouchPoint(int x_val, int y_val, int width_val/*=1*/, int height_val/*=1*/) :
	x(x_val),
	y(y_val),
	width(width_val),
	height(height_val),
	state(Up),
	color(1.0f)
{}

TouchPoint::TouchPoint() :
	x(-1),
	y(-1),
	width(1),
	height(1),
	color(1.0f)
{}

Vec3 TouchPoint::getPositionVector()
{
	return Vec3(x, y, 0.0f);
}

Vec3 TouchPoint::calculateDisplacementFrom(TouchPoint &touchPoint)
{
	return Vec3(x - touchPoint.x, y - touchPoint.y, 0.0f);
}

Vec3 TouchPoint::calculateVelocityFrom(TouchPoint &touchPoint)
{
	ulong deltaTime = relativeTimeStamp - touchPoint.relativeTimeStamp;
	if(deltaTime <= 0)
	{
		//Could occur when touch up is received right after touch move.
		//Usually returns last points velocity.
		return touchPoint.velocity;
	}
	return calculateDisplacementFrom(touchPoint) / deltaTime;
}

Vec3 TouchPoint::calculateMidPoint(TouchPoint &touchPoint)
{
	float midX = ((x - touchPoint.x) / 2) + touchPoint.x;
	float midY = ((y - touchPoint.y) / 2) + touchPoint.y;
	return Vec3(midX, midY, 0.0f);
}

tuple<NxActorWrapper*, BumpObject*, Vec3> TouchPoint::pickObject()
{
	return pick(x, y);
}

BumpObject* TouchPoint::getPickedObject()
{
	return pickObject().get<1>();
}

Vec3 TouchPoint::unProject(NxPlane &plane)
{
	Vec3 point;
	NxF32 distance;
	Vec3 closePlane, farPlane, dir;

	window2world(x, y, closePlane, farPlane);
	dir = farPlane - closePlane;
	dir.normalize();
	Ray touchRay(closePlane, dir);

	NxRayPlaneIntersect(touchRay, plane, distance, point);
	return point;
}

tuple<int, NxReal, Vec3, NxPlane> TouchPoint::unProjectToDesktop(float padding)
{
	return ::unProjectToDesktop(x, y, padding);
}