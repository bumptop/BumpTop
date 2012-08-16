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
#include "BT_Cluster.h"
#include "BT_DXRender.h"
#include "BT_Vertex.h"
#include "BT_Util.h"
#include "BT_Selection.h"
#include "BT_SceneManager.h"
#include "BT_Bubbles.h"
#include "BT_AnimationManager.h"
#include "BT_MouseEventManager.h"
#include "BT_QuadNode.h"
#include "Delaunay.h"
#include "BT_ClusterItem.h"



typedef QVarLengthArray<ClusterItem*, 16> qvlaObjList;

Cluster::Cluster()
{
	setBumpObjectType(BumpCluster);
	setFrozen(true);
	setCollisions(false);
	reset();
}

void Cluster::reset()
{
	_items.clear();
	_invisible = false;
	_allowRecalc = true;
	_isSpreadOut = false;
	//_boundary.clear();
	
}

void Cluster::select()
{
	assert(!_items.isEmpty());
	sel->clear();
	foreach (ClusterItem *k, _items)
	{
		sel->add(k->obj);
	}
}

void Cluster::onRender(uint flags)
{
	foreach (ClusterItem *k, _items)
	{
		k->obj->onRender();
	}
	
}

static Vec2 computeDesiredVec(NxReal dx, NxReal dy, NxReal targetLength)
{
	NxReal ratio;
	if (abs(dx) > abs(dy))
	{
		ratio = targetLength / abs(dx);
	}
	else
	{
		ratio = targetLength / abs(dy);
	}

	if (ratio < 1.0f) ratio = 1.0f;

	dx *= ratio;
	dy *= ratio;

	return Vec2(dx, dy, 0);
}

QVector<Vec2> _spread(QVector<Vec2> points, NxReal targetLength)
{
	//jitter points.  this moves the COM
	for (int i = 0; i < points.size(); i++)
	{
		points[i].x = points[i].x - 0.05f + 0.1f * i / points.size();
		points[i].z = 0;
	}

	Vec2 com(0, 0, 0);
	foreach (Vec2 v, points)
	{
		com.x += v.x;
		com.y += v.y;
	}
	com.x /= points.size();
	com.y /= points.size();

	QVector<Edge> edges = getDelaunayEdges(points);

	int n = points.size();
	int m = edges.size();

	NEWMAT::Matrix A(m + 1 , n);
	A = 0.0f;
	NEWMAT::ColumnVector bx(m + 1);
	bx = 0.0f;
	NEWMAT::ColumnVector by(m + 1);
	by = 0.0f;

	for (int i = 0 ; i < edges.size(); i++)
	{
		Vec2 p0 = points[edges[i].start];
		Vec2 p1 = points[edges[i].end];

		double dx = p1.x - p0.x;
		double dy = p1.y - p0.y;

		Vec2 desiredD = computeDesiredVec(dx, dy, targetLength);

		Edge e = edges[i];
		A.element(i,edges[i].start) = -1.0f;
		A.element(i,edges[i].end) = 1.0f;

		bx.element(i) = desiredD.x;
		by.element(i) = desiredD.y;
	}

	for (int i = 0; i < n; i++)
		A.element(m, i) = 1.0f;

	bx.element(m) = com.x * n;
	by.element(m) = com.y * n;

	//Calculate ATAiAT
	NEWMAT::Matrix AT = A.t();
	NEWMAT::Matrix ATA = AT * A;

	NEWMAT::Matrix ATAi;
	try
	{
		ATAi = ATA.i();
	}
	catch (NEWMAT::Exception)
	{
		cout << "error in inversion " << NEWMAT::Exception::what() << endl;
		return points;
	}

	NEWMAT::Matrix ATAiAT = ATAi * AT;

	NEWMAT::ColumnVector x = ATAiAT * bx;
	NEWMAT::ColumnVector y = ATAiAT * by;

	for (int i = 0; i < n; i++)
	{
		points[i].x = x.element(i);
		points[i].y = y.element(i);
	}

	return points;

}

void Cluster::cancelSpread()
{
	assert(_isSpreadOut);
	bubbleManager->exitSpreadState();

	_isSpreadOut = false;
}

void Cluster::spreadOut()
{
	assert(!_isSpreadOut);
	Bounds b;
	assert(!_items.isEmpty());

	QVector<Vec2> points;

	//grab the points from the array

	foreach (ClusterItem *k, _items)
	{
		points.append(k->pos);
	}

	for (int i = 0; i < 10; i++)
	{
		points = _spread(points, 20.0f);
	}

	//Animate the movement
	
	//Store the original and desired positions
	for (int i = 0; i < _items.size(); i++)
	{
		BumpObject *obj = _items[i]->obj;		

		obj->captureStateBeforeDrag(_items[i]->obj);

		Mat34 finalPose(GLOBAL(straightIconOri), Vec3(points[i].x, obj->getGlobalPosition().y, points[i].y));

		obj->setPoseAnim(obj->getGlobalPose(), finalPose, BubbleClusters::MAX_ANIM_STEPS);
	}

	bubbleManager->setSelCluster(this);
	bubbleManager->enterSpreadState();
	

	//toggle state
	_isSpreadOut = true;
	
}

void Cluster::spreadBack()
{
	assert (_isSpreadOut);
	
	Bounds b;

	//Animate back to original position
	for (int i = 0; i < _items.size(); i++)
	{
		BumpObject *obj = _items[i]->obj;

		if (!obj->isSelected())
		{
			obj->setPoseAnim(obj->getGlobalPose(), obj->stateBeforeDrag().pose, BubbleClusters::MAX_ANIM_STEPS);
		}

	
	}

	bubbleManager->setSelCluster(NULL);
	cancelSpread();
	
}



bool Cluster::containsPoint(NxReal x, NxReal y)
{
	if (_items.isEmpty()) return false;

	//first do a bounding box check
	if (x < _boundMin.x || y < _boundMin.y || x > _boundMax.x || y > _boundMax.y) return false;


	int n_vertices = _boundary.size();
	bool in_polygon = false;
	double slope, intersect_y;
	Vec2 v0, v1;

	// A polygon is defined by 3 or more vertices
	if (n_vertices < 3) {
		return false;
	}

	int i = 0;

	// For each pair of adjacent vertices (j,i) in the polygon
	for (int j = 0; j < n_vertices; j++) {
		i = (j + 1) % n_vertices;

		v0 = _boundary[j];
		v1 = _boundary[i];

		// This block checks that the point lies below the line segment joingin v0 and v1
		// (in the sense of lower y value)
		if ((x >= v1.x && x < v0.x) ||
			(x >= v0.x && x < v1.x)) {
				// Point lies between the two vertices in the x dimension
				// Check now to see if the point lies below the line

				// Simplified algebra that finds the y-value of the line connecting v0 and v1
				// at the x-value of the point in interest
				slope = (v1.y - v0.y)/(v1.x - v0.x);
				intersect_y = slope*(x - v0.x) + v0.y;
				if (y < intersect_y) {
					in_polygon = !in_polygon;
				}
		}
	}
	return in_polygon;
	
}

void Cluster::add(ClusterItem *k)
{
	_items.append(k);
}

void Cluster::remove(BumpObject * obj)
{
	for (int i = 0; i < _items.size(); i++)
	{
		if (_items[i]->obj == obj)
			_items.removeAt(i);
	}
}

inline int round(NxReal r) {
	return (r > 0.0) ? floor(r + 0.5) : ceil(r - 0.5);
}


NxReal static inline fieldForce(NxReal d, NxReal factor)
{

	static const NxReal inner_radius = BubbleClusters::nearRange / 2.0f * 1.5f;
	static const NxReal outer_radius = BubbleClusters::farRange / 2.0f * 1.5f;

	if (d >= outer_radius) {
		return 0.0f;
	}
	double x = d / outer_radius;
	double a = inner_radius / outer_radius;

	return factor * (x * x - 2.0f * x + 1.0f) / (a * a - 2.0f * a + 1.0f);
}



NxReal * precomputeFieldValue()
{
	NxReal *data = new NxReal[BubbleClusters::precomputeFieldSize * BubbleClusters::precomputeFieldSize * 4];
	for (int x = 0; x < BubbleClusters::precomputeFieldSize * 2; ++x)
	{
		for (int y = 0; y < BubbleClusters::precomputeFieldSize * 2; ++y)
		{
			NxReal r = sqrt((NxReal)((x - BubbleClusters::precomputeFieldSize) * (x - BubbleClusters::precomputeFieldSize)
				+ (y - BubbleClusters::precomputeFieldSize) * (y - BubbleClusters::precomputeFieldSize)));
			data[x + y * BubbleClusters::precomputeFieldSize * 2] = fieldForce(r, 1.0f);
		}
	}
	return data;
}

NxReal eval(Vec2 objPos, int _x, int _y) {

	static NxReal *precomputedFieldValue = precomputeFieldValue();


	const int x = (_x - round(objPos.x) + BubbleClusters::precomputeFieldSize);
	const int y = (_y - round(objPos.y) +  BubbleClusters::precomputeFieldSize);
	if ((x < 0) || (x >= 2 *  BubbleClusters::precomputeFieldSize) || (y < 0)
		|| (y >= 2 *  BubbleClusters::precomputeFieldSize))
		return 0.0f;
	return precomputedFieldValue[x + y * BubbleClusters::precomputeFieldSize * 2];
}

inline void assertCloseToInt(NxReal x)
{
	const NxReal epsilon = 0.0001f;
	NxReal low = (int)x;
	
	assert (x - low < epsilon || (low + 1) - x < epsilon);
}

//calculate the force at certain position with respect to objs in list
//use the height ratio to do adjustments
NxReal Cluster::calcForce(const qvlaObjList &objs, Vec2 pos)
{
	NxReal force = 0;

	//make the value is close to integers
	assertCloseToInt(pos.x);
	assertCloseToInt(pos.y);

	

	const uint objsSize = objs.size();
	for (int i = 0; i < objsSize; i++)
	{
		BumpObject *obj = objs[i]->obj;
		

		if (obj->getParent() == this)							
			
			force += objs[i]->ratio * eval(objs[i]->pos, pos.x, pos.y);
		
		else	
			
			force += -0.4f * objs[i]->ratio * eval(objs[i]->pos, pos.x, pos.y);
		
	}

	return force;
}


static inline Vec2 bezier(Vec2 &a, Vec2 &b, Vec2 &c, Vec2 &d, float t)
{
	Vec2 ab = lerp (a,b,t);
	Vec2 bc = lerp(b,c,t);
	Vec2 cd = lerp(c,d,t);
	Vec2 abbc = lerp(ab, bc, t);
	Vec2 bccd = lerp(bc,cd,t);
	return lerp(abbc, bccd, t);
}

static inline Vec2 quadratic(Vec2 &a, Vec2 &b, Vec2 &c, float t)
{
	Vec2 ab = lerp (a,b,t);
	Vec2 bc = lerp(b,c,t);	

	return lerp(ab, bc, t);
}



//smoothes out the boundary
QVector<Vec2> processBoundary(QVector<Vec2> boundary)
{
	QVector<Vec2> out;

	//prefilter that removees every other point	
	{		
		QVector<Vec2> filter;

		//chop out every 2
		for (int i = 0; i < boundary.size() -1; i+=2)
		{
			filter.append(boundary[i]);
		}
		boundary = filter;
	}

	//return boundary;

	//pretrube points
	//for (int i = 0; i < boundary.size(); i++)
	//{
	//	//boundary[i].x += ((i % 23) - 0.5f)*(1/23.0f);
	//	//boundary[i].y += ((i % 17) - 0.5f) * (1/17.0f);
	//}

	//return boundary;


	//Do a sliding window of bezier
	
	for (int i = 0; i < boundary.size(); i++)
	{

		Vec2 a = boundary[(i)% boundary.size()];
		Vec2 b = boundary[(i+1)% boundary.size()];
		Vec2 c = boundary[(i+2)% boundary.size()];
		Vec2 d = boundary[(i+3)% boundary.size()];

		out.append(bezier(a,b,c,d,0.5f));
	}

	return out;

	//cubic interpolation of points
	const bool cubic = true;
	const int numControlPoints = cubic ? 4 : 3;
	//grab each group of four points
	int i;
	for (i = 0; i + numControlPoints - 1< boundary.size()  ; i+=numControlPoints)
	{

		//do an interpolation of 10pts

		const float stepSize = 0.1f;
		//cubic spline
		if (cubic)
		{
			Vec2 a = boundary[i];
			Vec2 b = boundary[i+1];
			Vec2 c = boundary[i+2];
			Vec2 d = boundary[i+3];

			for (float t = 0.0f; t <= 1.0f; t +=stepSize)
			{
				out.append(bezier(a,b,c,d,t));
			}

		}
		else
		{
			Vec2 a = boundary[i];
			Vec2 b = boundary[i+1];
			Vec2 c = boundary[i+2];			

			for (float t = 0.0f; t <= 1.0f; t +=stepSize)
			{
				out.append(quadratic(a,b,c,t));
			}

		}
		
	}

	while (i < boundary.size())
	{
		out.append(boundary[i]);
		i++;
	}

	return out;

}

void getAllObjects(qvlaObjList &list)
{
	list.clear();
	const QVector<ClusterItem*> &kernels = bubbleManager->getObjList();

	for (int i = 0; i < kernels.size(); i++)
	{		
		list.append(kernels[i]);
	}
}

//this reduces the strength of the field so that when objects are stacked on each other
//it doesn't cause unnecessary bulging
void Cluster::adjustFieldStrength()
{
	qvlaObjList allObjsList;
	getAllObjects(allObjsList);	

	static const NxReal maxF = fieldForce(0.0f, 1.0f);

	foreach (ClusterItem *k, _items)
	{		
		NxReal f = 0.0f;			
		
		for (int i = 0; i < allObjsList.size(); ++i) 
		{			
			f += fieldForce(k->dist(allObjsList[i]), 1.0f);
		}


		const NxReal ratio = maxF /f;
		k->ratio = ratio;
				
	}

}

//highly optimized code to calculate the contour line of the force field
//uses marching square algorithm
QVector<Vec2> Cluster::calcBoundary(const NxReal stepSize)
{

	static const byte N = 1, E = 2, S = 3, W = 4;
	//							  0			4
	static const byte lookup[] = {E,S,W,W,  E,S,W,W,  N,N,N,N,   E,S,E,0};

	QVector<Vec2> boundary;

	//start at an objects global position
	Vec2 cur = Vec2(round(_items[0]->pos.x), round(_items[0]->pos.y), 0);

	const Vec2 east(stepSize, 0, 0);
	const Vec2 north(0, stepSize, 0);

	const NxReal maxForceDist = 5000.0f;
	const NxReal distBeforeTreeRecalc = 10.0f;//25.0f;

	cur += north;

	//keep going north until boundary is less than one
	qvlaObjList list;
	getAllObjects(list);
	while (true)
	{		
		//bubbleManager->getQuadTree()->getAllObjectsNear(list, cur, maxForceDist);		

		if (!(calcForce(list, cur) > 1)) break;
		cur += north;
	}

	//go north east
	cur += Vec2(stepSize/2, -stepSize/2, 0);

	//Nw is guarenteed to be empty

	boundary.append(cur);

	Vec2 start = cur;
	//start the algorithm

	
	Vec2 qCur = cur;
	getAllObjects(list);
	//bubbleManager->getQuadTree()->getAllObjectsNear(list, cur, maxForceDist);
	

	const int maxIterations = 1000;
	byte lastDir = 0;
	int r = 0;	//ls 4 bits hold nw,ne,sw,se
	for (int i = 0; i < maxIterations; i++)
	{

		//recalc items from quad tree once a while
		if (cur.distanceSquared(qCur) > distBeforeTreeRecalc)
		{
			qCur = cur;
			list.clear();
			//bubbleManager->getQuadTree()->getAllObjectsNear(list, cur, maxForceDist);
			getAllObjects(list);
		}

		//Calc four corner

		if (lastDir == N || lastDir == W || lastDir == 0)
		{
			bool nw = calcForce(list, cur + Vec2(-stepSize/2, stepSize/2, 0)) > 1;
			if (nw) r = r | 8;
			else r = r & ~8;
			 
		}
		if (lastDir == N || lastDir == E || lastDir == 0)
		{
			bool ne = calcForce(list, cur + Vec2(stepSize/2, stepSize/2, 0)) > 1;
			if (ne) r = r | 4;
			else r = r & ~4;
		}
		if (lastDir == S || lastDir == W || lastDir == 0)
		{
			bool sw = calcForce(list, cur + Vec2(-stepSize/2, -stepSize/2, 0)) > 1;
			if (sw) r = r | 2;
			else r = r & ~2;
		}
		if (lastDir == S || lastDir == E || lastDir == 0)
		{
			bool se = calcForce(list, cur + Vec2(stepSize/2, -stepSize/2, 0)) > 1;
			if (se) r = r | 1;
			else r = r & ~1;
		}		

		//Move cur in the right direction
		assert (r >= 0 && r < 16);
		byte nextDir = lookup[r];

		switch (nextDir)
		{
		case N:
			cur += north;	
			r = r >> 2;
			break;
		case S:
			cur -= north;
			r = r << 2;
			break;
		case E:
			cur += east;
			r = r << 1;
			break;
		case W:
			cur -= east;
			r = r >> 1;
			break;
		default:
			assert(false);
		}

		//mask lower four bits of r after bit shift
		r = r & 0xf;
		lastDir = nextDir;		

		//Add cur
		if (i > 3 && cur.equals(start, 0.01f)) break;

		boundary.append(cur);		
		
		if (i == maxIterations - 1)
		{
			consoleWrite("Cluster::calcBoundary failed, ran into infinite loop\n");
		}

	}

	//debug the boundary

	_boundMin.x = _boundMax.x = boundary[0].x;
	_boundMin.y = _boundMax.y = boundary[0].y;

	foreach (Vec2 p, boundary)
	{
		_boundMin.x = min(_boundMin.x , p.x);
		_boundMin.y = min(_boundMin.y , p.y);

		_boundMax.x = max(_boundMax.x , p.x);
		_boundMax.y = max(_boundMax.y , p.y);
	}

	//THIS IS DEBUG CODE IF YOU WANT TO DRAW A RED OVERLAY AROUND THE BOUNDARY

	/*
	for (NxReal y = _boundMin.y - stepSize; y <= _boundMax.y; y+= stepSize)
	{
		for (NxReal x = _boundMin.x - stepSize; x <= _boundMax.x; x+= stepSize)
		{
			//draw rect around certain point
			if (calcForce(list, Vec2(x + stepSize/2, y + stepSize/2, 0)) < 1)
			{
				//draw rect around (x,y, x + stepSize, y + stepSize)


				D3DXVECTOR3 a(x,0,y);
				D3DXVECTOR3 b(x+stepSize,0,y);
				D3DXVECTOR3 c(x+stepSize,0,y+stepSize);
				D3DXVECTOR3 d(x,0,y+stepSize);


				D3DXVECTOR3 *vertices = new D3DXVECTOR3[6];

				vertices[0] = a;
				vertices[1] = b;
				vertices[2] = c;
				vertices[3] = a;
				vertices[4] = c;
				vertices[5] = d;


				D3DMATERIAL9 material = {0};
				material.Diffuse = D3DXCOLOR(1.0f, 0.0f, 0.0f, 1.0f);
				material.Ambient = material.Diffuse;
				dxr->device->SetMaterial(&material);


				dxr->device->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 2, vertices, sizeof(D3DXVECTOR3));

				delete [] vertices;

				

			}
			
		}
	}
	*/
	

	//process boundary
	if (bubbleManager->allowSmoothCurve())
		boundary = processBoundary(boundary);

	//Calculate the bounding box
	_boundMin.x = _boundMax.x = boundary[0].x;
	_boundMin.y = _boundMax.y = boundary[0].y;

	foreach (Vec2 p, boundary)
	{
		_boundMin.x = min(_boundMin.x , p.x);
		_boundMin.y = min(_boundMin.y , p.y);

		_boundMax.x = max(_boundMax.x , p.x);
		_boundMax.y = max(_boundMax.y , p.y);
	}

	return boundary;
}

static inline bool isConvex(const Vec2& a, const Vec2& b, const Vec2& c)
{
	// test concavity using winding order (CW/CCW) of points
	const Vec2& d = b - a;
	const Vec2& e = c - b;
	const float cross = d.y * e.x - d.x * e.y;
	return cross < 0;
}

static void triangulatePolygon(QVector<Vec2> polygon, QVector<D3DXVECTOR3>& triangles)
{
	if (polygon.size() < 3)
		return;
	triangles.reserve((polygon.size() - 2) * 3);
	// divide a polygon into triangles by cutting away one triangle at a time
	while (polygon.size() > 2)
	{
		const unsigned oldSize = polygon.size();
		for (unsigned i = 0; i < polygon.size(); i++)
		{
			const unsigned j = i + 1 >= polygon.size() ? 0 : i + 1;
			const unsigned k = j + 1 >= polygon.size() ? 0 : j + 1;
			const Vec2& a = polygon[i], b = polygon[j], c = polygon[k];
			if (!isConvex(a, b, c))
				continue;
			bool empty = true;
			for (unsigned l = (k + 1) % polygon.size(); l != i; l = l + 1 >= polygon.size() ? 0 : l + 1)
				// point is in triangle if forming a new triangle using two original triangle vertices
				//  and point has the same winding order as original triangle
				if (isConvex(a, b, polygon[l]) && isConvex(b, c, polygon[l]) && isConvex(c, a, polygon[l]))
				{ // triangle abc is not entirely inside the polygon if there is a polygon vertex in it
					empty = false;
					break; 
				}
			if (!empty)
				continue;
			triangles.push_back(D3DXVECTOR3(a.x, 0.2f, a.y));
			triangles.push_back(D3DXVECTOR3(b.x, 0.2f, b.y));
			triangles.push_back(D3DXVECTOR3(c.x, 0.2f, c.y));
			polygon.erase(polygon.begin() + j);
			i--;
		}
		if (oldSize == polygon.size())
			break;
	}
}

void Cluster::draw(QColor color, NxReal resolution)
{
	if (_items.empty() || _invisible) return;

	assert (!_items.empty());

	//check all the _items parents are clusters
	foreach (ClusterItem *k, _items)
	{
		k->update();
		assert(k->obj->getParent() == this);
	}

	//use the marching squares algorithm to calculate the bounduary	
	
	//Create the bounduary
	_boundary = calcBoundary(resolution);	

	if (selected)
	{
		QVector<D3DXVECTOR3> triangles;
		triangulatePolygon(_boundary, triangles);
		D3DMATERIAL9 material = {0};
		material.Diffuse = D3DXCOLOR(color.redF(), color.greenF(), color.blueF(), 0.1f);
		material.Ambient = material.Diffuse;
		dxr->device->SetMaterial(&material);
		dxr->device->DrawPrimitiveUP(D3DPT_TRIANGLELIST, triangles.size() / 3, &triangles[0], sizeof(triangles[0]));
	}

	if (BubbleClusters::useVolumeLine)
	{		

	/*	QVector<Vec2> tempBound;
		_boundaryAnim++;
		if (_boundaryAnim > _boundary.size() || _boundaryAnim < 3) _boundaryAnim = min(3, _boundary.size());

		for (int i = 0; i < _boundaryAnim; i++ )
		{
			tempBound.append(_boundary[i]);
		}*/


		bubbleManager->drawVolumeBoundary(_boundary, D3DXCOLOR(color.redF(), color.greenF(), color.blueF(), selected ? 1.0f : 0.5f));
	}
	else
	{
		//draw new boundary
		
		D3DMATERIAL9 material = {0};
		material.Diffuse = D3DXCOLOR(color.redF(), color.greenF(), color.blueF(), selected ? 1.0f : 0.5f);
		material.Ambient = material.Diffuse;
		dxr->device->SetMaterial(&material);

		D3DXVECTOR3 *vertices = new D3DXVECTOR3[_boundary.size() * 2];
		int curVertex = 0;
		for (int i = 0; i < _boundary.size(); i++)
		{
			vertices[curVertex++] = D3DXVECTOR3(_boundary[i].x , 0.05f, _boundary[i].y);

			vertices[curVertex++] = D3DXVECTOR3(_boundary[(i + 1) % _boundary.size()].x , 0.05f, _boundary[(i + 1) % _boundary.size()].y);

		}

		dxr->device->DrawPrimitiveUP(D3DPT_LINELIST, _boundary.size(), vertices, sizeof(D3DXVECTOR3));

		delete [] vertices;
	}

}

void Cluster::onLaunch()
{
	if (_isSpreadOut)
		spreadBack();
	else
		spreadOut();
}

void Cluster::onSelect()
{
	Selectable::onSelect();
}

void Cluster::onDeselect()
{
	Selectable::onDeselect();
}

void Cluster::onDragBegin(FinishedDragCallBack func )
{
	if (_isSpreadOut)
	{
		cancelSpread();
	}

	_allowRecalc = false;
	bubbleManager->setAllowClusterRecalc(false);
	Vec2 mousePos = pointOnFloor(mouseManager->primaryTouchX, mouseManager->primaryTouchY);

	_relDragPos.clear();
	foreach (ClusterItem *k, _items)
	{
		_relDragPos.append(k->obj->getGlobalPosition() - mousePos);		
	}

	_dragStartPos = mousePos;

	BumpObject::onDragBegin();
}

void Cluster::onDragEnd()
{
	_allowRecalc = true;
	bubbleManager->setAllowClusterRecalc(true);
	_relDragPos.clear();
	BumpObject::onDragEnd();
}

void Cluster::onDrag(Vec2 mousePos)
{
	const float kSpring = 150.0f;
	const float kDamping = 20.0f;

	adjustPointToInsideWalls(mousePos);
	Vec3 deltaDist = (mousePos - _dragStartPos);
	if (deltaDist.magnitude() > 0.005f)
	{
		for (int i = 0; i < _items.size(); i++)
		{
			BumpObject *obj = _items[i]->obj;

			Vec3 desiredPosition = (mousePos + _relDragPos[i]) ;
			adjustPointToInsideWalls(desiredPosition);
			Vec3 deltaDist = desiredPosition - obj->getGlobalPosition();			

			Vec3 force = obj->getMass() * 
				((2.0f * kSpring) * deltaDist - 
				(1.0f * kDamping) * obj->getLinearVelocity());		
			obj->addForceAtLocalPos(force, Vec3(0.0f));

		}
	}				
}

QVector<BumpObject*> Cluster::getItems()
{
	QVector<BumpObject*> items;
	foreach(ClusterItem *k, _items)
	{
		items.append(k->obj);
	}
	return items;
}


//DEBUG for all n choose 2 edges
QVector<Edge> getAllEdges(QVector<Vec2> points)
{

	NxReal minX, maxX, minY, maxY;
	minX = maxX = points[0].x;
	minY = maxY = points[0].y;

	for (int i = 1; i < points.size(); i++)
	{
		minX = min(minX, points[i].x);
		maxX = max(maxX, points[i].x);

		minY = min(minY, points[i].y);
		maxY = max(maxY, points[i].y);
	}

	for (int i = 0; i < points.size(); i++)
	{
		points[i].x = (points[i].x - minX) / (maxX - minX) * 640;
		points[i].y = points[i].y - minY / (maxY - minY) * 480;

	}

	QVector<Edge> edges;

	for (int i = 0; i < points.size(); i++)
	{
		for (int j = i + 1; j < points.size(); j++)
		{

			Edge e;
			e.start = i;
			e.end = j;
			edges.append(e);

		}
	}
	return edges;
}

void Cluster::drawDebug()
{
	assert(!_items.empty());

	QVector<Vec2> points;
	foreach (ClusterItem *k, _items)
	{
		BumpObject *obj = k->obj;
		//points.append(Vec2(-obj->getGlobalPosition().x + 260, obj->getGlobalPosition().z + 150, 0));
		points.append(Vec2(obj->getGlobalPosition().x, obj->getGlobalPosition().z, 0));
	}
	
	for (int i = 0; i < points.size(); i++)
	{
		points[i].x = points[i].x - 0.05f + 0.1f * i / points.size();
		//points[i].y = points[i].y + 1.0f * i / points.size();
	}

	////sort points by increasing x then increasing y

	
	QVector<Edge> edges = getDelaunayEdges(points);

	D3DXVECTOR3 *vertices = new D3DXVECTOR3[edges.size() * 2];
	int i = 0;
	foreach (Edge e, edges)
	{
		assert(e.start >= 0 && e.start < points.size());
		assert(e.end >= 0 && e.end < points.size());
		vertices[i] = D3DXVECTOR3(points[e.start].x, 1.0f, points[e.start].y);
		i++;
		vertices[i] = D3DXVECTOR3(points[e.end].x, 1.0f, points[e.end].y);
		i++;
	}

	D3DMATERIAL9 material = {0};
	material.Diffuse = D3DXCOLOR(1.0f, 0.0f, 0.0f, 1.0f);
	material.Ambient = material.Diffuse;
	dxr->device->SetMaterial(&material);


	dxr->device->DrawPrimitiveUP(D3DPT_LINELIST, edges.size(), vertices, sizeof(D3DXVECTOR3));



	delete [] vertices;

}
