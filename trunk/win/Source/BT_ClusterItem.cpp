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
#include "BT_ClusterItem.h"
#include "BT_BumpObject.h"

ClusterItem::ClusterItem(BumpObject *obj)
{
	this->obj = obj;
	pos = Vec2(obj->getGlobalPosition().x, obj->getGlobalPosition().z, 0);
	ratio = 1.0f;
}

ClusterItem::ClusterItem()
{
	obj = NULL;
	pos = 0;
	ratio = 0.0f;
	assert(false);
}

void ClusterItem::update()
{
	pos = Vec2(obj->getGlobalPosition().x, obj->getGlobalPosition().z, 0);
}

NxReal ClusterItem::dist(ClusterItem *k)
{
	NxReal dx = pos.x - k->pos.x;
	NxReal dy = pos.y - k->pos.y;
	return sqrt(dx*dx + dy*dy);
}

NxReal ClusterItem::distPos(const Vec2 &p)
{
	NxReal dx = pos.x - p.x;
	NxReal dy = pos.y - p.y;
	return sqrt(dx*dx + dy*dy);

}

NxReal ClusterItem::distSq(ClusterItem *k)
{
	NxReal dx = pos.x - k->pos.x;
	NxReal dy = pos.y - k->pos.y;
	return (dx*dx + dy*dy);
}

NxReal ClusterItem::distPosSq(const Vec2 &p)
{
	NxReal dx = pos.x - p.x;
	NxReal dy = pos.y - p.y;
	return (dx*dx + dy*dy);

}