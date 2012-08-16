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

#ifndef BT_Cluster_h__
#define BT_Cluster_h__

#include "BT_BumpObject.h"


class ClusterItem;

class Cluster : public BumpObject
{
	friend class Bubbles;
	friend class Selection;
private:
	NxReal calcForce(const QVarLengthArray<ClusterItem*, 16> &objs,  Vec3 pos);
	QVector<Vec2> calcBoundary(const NxReal resolution);
	

	QList<ClusterItem*> _items;


	QVector<Vec2> _boundary;
	Vec2 _boundMin, _boundMax;  //Bounding box of boundary

	//TODO remove by saving the info into saved pose (in drag)
	Vec2 _dragStartPos;
	QVector<Vec2> _relDragPos;

	//This cluster is not drawn
	bool _invisible;
	bool _allowRecalc;	
	bool _isSpreadOut;


	int _boundaryAnim;

public:
	Cluster();
	void add(ClusterItem*);
	void remove(BumpObject *);

	void draw(const QColor color, const NxReal resolution);
	void adjustFieldStrength();

	void drawDebug();

	void reset();
	//These are world coordinates on the floor plane
	bool containsPoint(NxReal x, NxReal y);
	void select();

	void spreadOut();
	void spreadBack();
	void cancelSpread();
	
	virtual void onLaunch();	

	//Renderable
	virtual void onRender(uint flags);

	//BumpObject
	virtual void onSelect();
	virtual void onDeselect();

	//Selectable
	virtual void onDragBegin(FinishedDragCallBack func /* = NULL */);
	virtual void onDragEnd();
	virtual void onDrag(Vec2 mousePos);

	QVector<BumpObject*> getItems();
	
	
	
};

#endif // BT_Cluster_h__