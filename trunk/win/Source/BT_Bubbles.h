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

#ifndef BT_Bubbles_h__
#define BT_Bubbles_h__

#include "BT_OverlayComponent.h"

class Cluster;
class BumpObject;
class QuadNode;
class ClusterItem;

class MouseOverlayEvent;

namespace BubbleClusters
{
	
	static const int precomputeFieldSize = 40;

	//far should be bigger than near
	static const NxReal nearRange = 25.0f;
	static const NxReal farRange = precomputeFieldSize;

	static const NxReal nearRangeSq = nearRange*nearRange;
	static const NxReal farRangeSq = farRange*farRange;

	//must be even numbers
	static const NxReal highResolutionStepSize = 2.0f;
	static const NxReal lowResolutionStepSize = 4.0f;

	const int MAX_ANIM_STEPS = 10;	// speed of animation for spreading a cluster
	const bool useVolumeLine = true;	//use advanced line drawing algorithm
}


class BubbleOverlayControl : public AbsoluteOverlayLayout
{
public:
	BubbleOverlayControl();
	~BubbleOverlayControl();
	bool onMouseDown(MouseOverlayEvent& mouseEvent);

};


class Bubbles
{
	Q_DECLARE_TR_FUNCTIONS(Bubbles)

	friend class Singleton<Bubbles>;
	friend class BubbleOverlayControl;

	
	enum BubbleState { BUB_NORMAL, BUB_SPREADOUT};


private:
	QVector<Cluster*> _clusters;
	QHash<BumpObject*, Cluster*> _clusterMap;
	QVector<ClusterItem *> _objList;
	
	int _mode;
	bool _enabled;
	bool _highQuality;
	bool _drawWire;
	bool _smoothCurve;
	

	bool _allowClusterRecalc;
	QuadNode * _root;
	BubbleOverlayControl _overlayControl;

	BubbleState _state;
	Cluster * _selCluster;


	void calcClusters();

public:

	

	Bubbles();
	
	void update();
	void drawClusters();
	void drawDebugClusters();

	Cluster* pickCluster(int x, int y);
	bool onMouseDown(int x, int y, MouseOverlayEvent& mouseEvent);
	void collapseSpreadedCluster();
	
	Cluster* getCluster(BumpObject *obj);
	const QVector<ClusterItem *> & getObjList();
	void cycleMode();

	void setAllowClusterRecalc(bool value);
	bool allowSmoothCurve();

	void reset();

	QuadNode * getQuadTree();

	//experimental drawing
	void drawVolumeLine(Vec2 start, Vec2 end);
	void drawVolumeBoundary(QVector<Vec2> boundary, D3DXCOLOR color);

	//states
	void enterSpreadState();
	void exitSpreadState();

	//sel
	void setSelCluster(Cluster *c);
	Cluster *getSelCluster();


};


#define bubbleManager Singleton<Bubbles>::getInstance()

#endif // BT_Bubbles_h__

