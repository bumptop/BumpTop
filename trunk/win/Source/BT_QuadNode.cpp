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
#include "BT_QuadNode.h"
#include "BT_DXRender.h"
#include "BT_Box2.h"
#include "BT_BumpObject.h"

const int NW = 0, NE = 1, SW = 2, SE = 3;
const int MAX_QUADTREE_DPETH = 5;

QuadNode::QuadNode(const Box2 &bounds, int depth)
{
	_box = bounds;
	_depth = depth;
	_dirty = false;
	for (int i = 0; i < 4; i++)
	{
		_nodes[i] = NULL;
	}
}

QuadNode::~QuadNode()
{
	for (int i = 0; i <4 ; i++)
	{
		delete _nodes[i];
	}
}

bool QuadNode::isSplitted()
{
	return _nodes[0] != NULL;
}

void QuadNode::dirtifyNodesAt(Vec2 center, NxReal radius)
{
	if (intersectCircRect(center, radius, _box))
	{
		
		if (isSplitted())
		{
			for (int i = 0; i < 4; i++)
			{
				_nodes[i]->dirtifyNodesAt(center, radius);
			}
		}
		else
			_dirty = true;
	}

}

void QuadNode::getAllObjectsNear(QVarLengthArray<ClusterItem *, 16> &list, const Vec2 &center, const NxReal &radius)
{
	if (intersectCircRect(center, radius, _box))
	{

		if (isSplitted())
		{
			for (int i = 0; i < 4; i++)
			{
				_nodes[i]->getAllObjectsNear(list, center, radius);
			}

			//JUST FOR LOGIC TESTING< REMOVE LATER
			assert(_items.size() == 0);
		}
		else
		{
			for (int i = 0; i < _items.size(); i++)
			{
				list.append(_items[i]);
			}

		}
			
			
	}
}

//Returns true if a circle intersects a axis aligned rectangle
bool QuadNode::intersectCircRect(const Vec2 &center, const NxReal &radius, const Box2 &rect)
{

	//calculate the half lengths of rectangles
	const NxReal halfRectWidth = rect.width()/2;
	const NxReal halfRectHeight = rect.height()/2;


	//Moves the center of the circle to the first quadrant (so you don't have to repeat the check for all four quads)
	Vec2 c;
	c.x = abs(center.x - (rect.min.x + halfRectWidth));
	if (c.x > (halfRectWidth + radius)) { return false; }

	c.y = abs(center.y - (rect.min.y + halfRectHeight));
	if (c.y > (halfRectHeight + radius)) { return false; }	
	

	if (c.x <= (halfRectWidth)) { return true; } 
	if (c.y <= (halfRectHeight)) { return true; }

	const NxReal cornerDistance_sq = (c.x - halfRectWidth) *  (c.x - halfRectWidth)+
		(c.y - halfRectHeight) * (c.y - halfRectHeight);

	return cornerDistance_sq <= (radius * radius);	
}

//Only leaf nodes can have items
void QuadNode::addObject(ClusterItem *obj, Vec2 pos)
{

	assert(_items.isEmpty() || !isSplitted());

	//If this box doesn't contain items, set it as the item
	if (!isSplitted() && _items.isEmpty())
	{
		_items.append(obj);
		_itemPos = pos;
		return;
	}


	
	if (_depth == MAX_QUADTREE_DPETH)
	{
		_items.append(obj);
		//This likely means there are identical points.  jitter points by random a mount
		return;
	}

	//otherwise split the node and insert both objects deeper
	if (!isSplitted()) split();

	Vec2 c = _box.getCenter();

	if (pos.x < c.x)
	{
		if (pos.y < c.y)
			_nodes[NW]->addObject(obj, pos);
		else
			_nodes[SW]->addObject(obj, pos);

	}
	else
		if (pos.y < c.y)
			_nodes[NE]->addObject(obj, pos);
		else
			_nodes[SE]->addObject(obj, pos);

	if (!_items.isEmpty())
	{
		
	
		pos = _itemPos;
		if (pos.x < c.x)
		{
			if (pos.y < c.y)
				_nodes[NW]->addObject(_items[0], pos);
			else
				_nodes[SW]->addObject(_items[0], pos);

		}
		else
			if (pos.y < c.y)
				_nodes[NE]->addObject(_items[0], pos);
			else
				_nodes[SE]->addObject(_items[0], pos);

		assert(_items.size() == 1);
		_items.clear();
	}

}

void QuadNode::split()
{
	Vec2 c = _box.getCenter();
	//create NW
	Box2 bound;
	bound.min = _box.min;
	bound.max = c;
	_nodes[0] = new QuadNode(bound, _depth+1);

	//create NE
	bound.min.x = c.x;
	bound.min.y = _box.min.y;
	bound.max.x = _box.max.x;
	bound.max.y = c.y;

	_nodes[1] = new QuadNode(bound, _depth+1);

	//create SW
	bound.min.x = _box.min.x;
	bound.min.y = c.y;
	bound.max.x = c.x;
	bound.max.y = _box.max.y;

	_nodes[2] = new QuadNode(bound, _depth+1);

	//create SE
	bound.min = c;
	bound.max = _box.max;
	_nodes[3] = new QuadNode(bound, _depth+1);
}

void QuadNode::draw()
{
	//draw all children
	if (isSplitted())
	{
		for (int i = 0; i < 4; i++)
			_nodes[i]->draw();
	}

	//draw a box

	D3DMATERIAL9 material = {0};
	material.Diffuse = D3DXCOLOR(1.0f, 1.0f,0.6f, 0.3f);
	material.Ambient = material.Diffuse;
	dxr->device->SetMaterial(&material);

	D3DXVECTOR3 *vertices = new D3DXVECTOR3[5];

	int curVertex = 0;
	D3DXVECTOR3 a(_box.min.x, 0.05f, _box.min.y);  //point 0
	D3DXVECTOR3 b(_box.max.x, 0.05f, _box.min.y); 
	D3DXVECTOR3 c(_box.max.x, 0.05f, _box.max.y);
	D3DXVECTOR3 d(_box.min.x, 0.05f, _box.max.y);


	vertices[curVertex++] = a;  //point 0
	vertices[curVertex++] = b; 
	vertices[curVertex++] = c;
	vertices[curVertex++] = d;
	vertices[curVertex++] = a;  //point 0
	

	dxr->device->DrawPrimitiveUP(D3DPT_LINESTRIP, 4, vertices, sizeof(D3DXVECTOR3));

	delete [] vertices;

	//draw a rectangle too
	if (_dirty)
	{

		D3DMATERIAL9 material = {0};
		material.Diffuse = D3DXCOLOR(1.0f, 1.0f,0.6f, 0.1f);
		material.Ambient = material.Diffuse;
		dxr->device->SetMaterial(&material);


		D3DXVECTOR3 *vertices = new D3DXVECTOR3[6];
		int curVertex = 0;

		vertices[curVertex++] = a;
		vertices[curVertex++] = b; 
		vertices[curVertex++] = c;
		vertices[curVertex++] = a;
		vertices[curVertex++] = c;
		vertices[curVertex++] = d;

		dxr->device->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 2, vertices, sizeof(D3DXVECTOR3));
		
	}






}