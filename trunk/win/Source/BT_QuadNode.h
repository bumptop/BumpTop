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

#include "BT_Box2.h"

#ifndef BT_QuadNode_h__
#define BT_QuadNode_h__

class ClusterItem;

class QuadNode
{
private:
	QuadNode *_nodes[4];
	QVarLengthArray<ClusterItem *, 4> _items;
	Box2 _box;
	int _depth;
	bool _dirty;
	Vec2 _itemPos;

	void split();
	bool intersectCircRect(const Vec2 &center, const NxReal &radius, const Box2 &rect);	

public:
	QuadNode(const Box2 &bounds, int depth);
	~QuadNode();

	void addObject(ClusterItem *obj, Vec2 pos);
	void draw();
	bool isSplitted();
	void getBumpObjectsAtPoint(Vec2 pos, NxReal radius);
	void dirtifyNodesAt(Vec2 center, NxReal radius);

	void getAllObjectsNear(QVarLengthArray<ClusterItem *, 16> &list, const Vec2 &center, const NxReal &radius);
	
};

#endif // BT_QuadNode_h__