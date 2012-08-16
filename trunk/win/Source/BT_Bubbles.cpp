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
#include "BT_Bubbles.h"
#include "BT_Cluster.h"
#include "BT_SceneManager.h"
#include "BT_Util.h"
#include "BT_BumpObject.h"
#include "BT_Nameable.h"
#include "BT_OverlayComponent.h"
#include "BT_Pile.h"
#include "BT_Actor.h"
#include "BT_Vertex.h"
#include "BT_Selection.h"
#include "BT_QuadNode.h"
#include "BT_Box2.h"
#include "BT_ClusterItem.h"




BubbleOverlayControl::BubbleOverlayControl()
{
	scnManager->registerOverlay(this);
}

BubbleOverlayControl::~BubbleOverlayControl()
{
	scnManager->unregisterOverlay(this);
}

bool BubbleOverlayControl::onMouseDown(MouseOverlayEvent& mouseEvent)
{		
	//HAX : Do not change
	//this fixes the overlay adjustment to get the original mouse position (ie with 0,0 in the top left)
	return bubbleManager->onMouseDown(mouseEvent.getPosition().x, -(mouseEvent.getPosition().y - getSize().y), mouseEvent);
	
}

Bubbles::Bubbles():
_mode(0),	//BEFORE COMMIT SET BACK TO 0
_enabled(false),
_highQuality(false),
_drawWire(false),
_smoothCurve(false),
_allowClusterRecalc(true),
_root(NULL),
_state(BUB_NORMAL),
_selCluster(NULL)
{
	
}

const QVector<ClusterItem *> & Bubbles::getObjList()
{
	return _objList;
}



void Bubbles::setAllowClusterRecalc(bool value)
{
	_allowClusterRecalc = value;
}

bool Bubbles::allowSmoothCurve()
{
	return _smoothCurve;
}

void Bubbles::collapseSpreadedCluster()
{
	if (_state != BUB_SPREADOUT) return;
	_selCluster->spreadBack();
}

bool Bubbles::onMouseDown(int x, int y, MouseOverlayEvent& mouseEvent)
{
	//Reponsible for collapsing the cluster automatically when the user clicks on something outside the cluster
	
	if (_state != BUB_SPREADOUT) return false;

	tuple<NxActorWrapper*, BumpObject*, Vec3> t = pick(x,y);	
	BumpObject* pickedBumpObject = t.get<1>();
	

	//picked an object in the spreaded cluster
	foreach (ClusterItem *k, _selCluster->_items)
	{
		if (k->obj == pickedBumpObject)
			return false;		
	}

	//picked the spreaded cluster itself
	if (_selCluster == pickedBumpObject)
		return false;
	

	//picked something outside the spreaded cluster, spread the cluster back automatically
	_selCluster->spreadBack();

	return false;	//catch the input
}



void Bubbles::cycleMode()
{

	//mode 0 - disabled, 1 - low, full, 2 - high,wire, 3 - high, full, 
	_mode++;
	_mode = _mode % 5;

	switch(_mode)
	{

	case 0:
		printUnique(QT_TR_NOOP("Bubbles::cycleMode"), QT_TR_NOOP("Disabled Bubble Clusters.  WHY????"));
		
		_enabled = false;
		break;
	case 1:
		printUnique(QT_TR_NOOP("Bubbles::cycleMode"), QT_TR_NOOP("Hit Ctrl-B again to see different visualizations"));
	
		_enabled = true;
		_highQuality = false;
		_drawWire = false;
		_smoothCurve = false;
		break;
	case 2:
		_enabled = true;
		_highQuality = false;
		_drawWire = false;
		_smoothCurve = true;
		break;

	case 3:
		_highQuality = true;
		_drawWire = false;	
		_smoothCurve = false;
		break;
	case 4:
		
		_highQuality = true;
		_drawWire = false;
		_smoothCurve = true;
		break;
	}

}

void Bubbles::update()
{
	
	if (!_enabled) return;
	if (_allowClusterRecalc) calcClusters();
	else
	{
		foreach(ClusterItem *k, _objList)
			k->update();
	}
	
	//update quad tree

	Box desktopBox = GetDesktopBox(GLOBAL(ZoomBuffer));

	Box2 desktop;
	desktop.min.x = desktopBox.GetCenter().x - desktopBox.GetExtents().x;
	desktop.max.x = desktopBox.GetCenter().x + desktopBox.GetExtents().x;									
	desktop.min.y = desktopBox.GetCenter().z - desktopBox.GetExtents().z;
	desktop.max.y = desktopBox.GetCenter().z + desktopBox.GetExtents().z;

	if (_root) delete _root;

	_root = new QuadNode(desktop, 0);

	for (int i = 0; i < _objList.size(); i++)
	{
		_root->addObject(_objList[i], _objList[i]->pos);
	}

}

QuadNode * Bubbles::getQuadTree()
{
	return _root;
}

Cluster* Bubbles::getCluster(BumpObject *obj)
{
		return _clusterMap[obj];
}

//n^2 algorithm.  This is computed each frame and may become expensive when n > 100
void Bubbles::calcClusters()	
{

	assert(_allowClusterRecalc);
	
	
	//This grabs all objects we are allowed to work with (essentially those on the ground)
	vector<BumpObject*> tempList = scnManager->getVisibleBumpActorsAndPiles(false);
	
	//delete all old kernels
	foreach (ClusterItem *k, _objList)
	{
		delete k;
	}
	_objList.clear();

	for (int i = 0; i < tempList.size(); i++)
	{
		if (tempList[i]->isPinned()) continue;
		//mouse over wall

		//check it has no parent (Except bubble cluster)
		if (tempList[i]->getParent() != NULL && !tempList[i]->getParent()->isBumpObjectType(BumpCluster)) continue;

		//set the parent to null
		tempList[i]->setParent(NULL);
		_objList.append(new ClusterItem(tempList[i]));
		
	}
	

	
	//Creates your cluster map, maps objects to clusters, and gets all bumpobjects in a cluster
	QVector<int> clusterMap(_objList.size());	
	QVector<QVarLengthArray<int>> clusters(_objList.size());

	//Initialize clusters
	for (int i = 0; i < _objList.size(); i++) 
	{	
		clusters[i].append(i);
		clusterMap[i] = i;
	}

	for (int i = 0; i < _objList.size(); i++) {
		for (int j = i + 1; j < _objList.size(); j++)
		{

			//compute the 2d distance squared
			const NxReal distSq = _objList[i]->distSq(_objList[j]);
			//NxReal distSq = _objList[i]->getGlobalPosition().distanceSquared(_objList[j]->getGlobalPosition());
			NxReal threshold = BubbleClusters::nearRangeSq;// * (getCluster(_objList[i]) == getCluster(_objList[j])) ? thresholdFarMulti : thresholdNearMulti;
			
			if (!_clusterMap.empty())
			{
				if (getCluster(_objList[i]->obj) == getCluster(_objList[j]->obj))
					threshold = BubbleClusters::farRangeSq;
				else
					threshold = BubbleClusters::nearRangeSq;
			}
		

			if (distSq < threshold)
			{
				//merge cluster for obj[j] into cluster obj[i]
				int liveCluster = clusterMap[j];
				int tempCluster = clusterMap[i];

				//assert(oldCluster != NULL);
				//assert(newCluster != NULL);

				if (liveCluster == tempCluster) continue;

				for (int k = 0; k < clusters[tempCluster].size(); k++)
				{
					clusterMap[clusters[tempCluster][k]] = liveCluster;
					clusters[liveCluster].append(clusters[tempCluster][k]);
				}

				clusters[tempCluster].clear();			
				
			}
		}
		
	}
	
	//print out the current cluster lists
	//cout << "Calculating Clusters\n";
	//for (int i = 0; i < objs.size(); i++) {
	//	if (clusters[i].size() > 1)
	//	{
	//		cout << "Cluster " << i << ":\n";
	//		for (int j = 0; j < clusters[i].size(); j++) {
	//			consoleWrite(clusters[i].at(j)->getNameableOverlay()->getTextOverlay()->getText());
	//			consoleWrite("\n");
	//		}
	//		
	//	}
	//}
	//cout << "\n\n";

	//Create the real clusters

	_clusterMap.clear();
	//uses a cluster pool to be cheap
	int curCluster = 0;
	for (int i = 0; i < clusters.size(); i++)
	{
		if (clusters[i].size() == 0) continue;

		for (;curCluster < _clusters.size(); curCluster++)
		{
			if (_clusters[curCluster]->_allowRecalc) break;
		}
	
		//If the pool is too small, add to it
		if (_clusters.size() <= curCluster)
		{
			//cout << "created new cluster " << _clusters.size() << endl;
			_clusters.append(new Cluster);
		}

		

		Cluster *c = _clusters[curCluster++];

		c->reset();

		for (int j = 0; j < clusters[i].size(); j++)
		{
			
			ClusterItem *k = _objList[clusters[i][j]];
			
			
			c->add(k);
			_clusterMap.insert(k->obj, c);
			k->obj->setParent(c);
			
			

		}


	}

	for (;curCluster < _clusters.size(); curCluster++)
	{
		if (_clusters[curCluster]->_allowRecalc)
			_clusters[curCluster]->reset();
	}

		

	//print out the current cluster lists
	/*for (int i = 0; i < _clusters.size(); i++) {
		if (_clusters[i]->_items.size() > 1)
		{
			cout << "Cluster " << i << ":\n";
			for (int j = 0; j < _clusters[i]->_items.size(); j++) {
				consoleWrite(_clusters[i]->_items.at(j)->getNameableOverlay()->getTextOverlay()->getText());
				consoleWrite("\n");
			}

		}
	}
	cout << "\n\n";*/
	

	//delete [] clusterId;
	//delete [] clusters;
}

Cluster *Bubbles::pickCluster(int x, int y)
{
	if (!_enabled) return NULL;

	Vec3 point = pointOnFloor(x,y);

	//scan all clusters and check if the point is "inside" the boundary
	//clusters should store polygon boundary information
	foreach (Cluster *c, _clusters)
	{
		if (c->containsPoint(point.x, point.z))
		{

			return c;
		}
	}
	return NULL;

}


void Bubbles::enterSpreadState()
{
	assert(_state == BUB_NORMAL);
	_state = BUB_SPREADOUT;
	setAllowClusterRecalc(false);
	
}

void Bubbles::exitSpreadState()
{
	assert(_state == BUB_SPREADOUT);
	_state = BUB_NORMAL;
	setAllowClusterRecalc(true);
	_selCluster = NULL;

}

void Bubbles::setSelCluster(Cluster *c)
{
	_selCluster = c;
}
Cluster *Bubbles::getSelCluster()
{
	return _selCluster;
}

void Bubbles::drawVolumeBoundary(QVector<Vec2> boundary, D3DXCOLOR color)
{

	D3DMATERIAL9 material = {0};
	material.Diffuse = color;
	material.Ambient = material.Diffuse;
	dxr->device->SetMaterial(&material);


	dxr->device->SetTexture(0,  texMgr->getGLTextureId("volumeline.glow"));
	dxr->device->SetTextureStageState(0,D3DTSS_COLOROP, D3DTOP_MODULATE);
	dxr->device->SetTextureStageState(0,D3DTSS_COLORARG1, D3DTA_TEXTURE);
	dxr->device->SetTextureStageState(0,D3DTSS_COLORARG2, D3DTA_DIFFUSE);

	dxr->device->SetTextureStageState(0,D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	dxr->device->SetTextureStageState(0,D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	dxr->device->SetTextureStageState(0,D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

	dxr->device->SetFVF(PositionTextured::GetFlexibleVertexFormat());

	for (int i = 0; i < boundary.size(); i++)
	{

		drawVolumeLine(Vec2(boundary[i].x, 0, boundary[i].y), Vec2(boundary[(i + 1) % boundary.size()].x, 0, boundary[(i + 1) % boundary.size()].y));

	}

	dxr->device->SetTexture(0, NULL);

	dxr->device->SetFVF(D3DFVF_XYZ);
}

//Only on x and z plane
void Bubbles::drawVolumeLine(Vec2 start, Vec2 end)
{
	//project line to screen coordinates

	Vec2 dir = end - start;

	Vec2 perp(dir.z, 0, -dir.x);
	//normalize perp
	perp.normalize();
	perp.multiply(1.0f, perp);

	dir.normalize();
	dir.multiply(1.0f, dir);

	//calc offset
	start = start - dir;
	end = end + dir;	


	//rendered textured quad

	PositionTextured *vertices = new PositionTextured[6];

	int curVertex = 0;

	PositionTextured a(start.x + perp.x, 0.05f, start.z + perp.z, 0.0f, 0.0f);  //point 0
	PositionTextured b(end.x + perp.x, 0.05f, end.z + perp.z, 1.0f, 0.0f ); 
	PositionTextured c(end.x - perp.x, 0.05f, end.z - perp.z, 1.0f, 1.0f);
	PositionTextured d(start.x - perp.x, 0.05f, start.z - perp.z, 0.0f, 1.0f);


	vertices[curVertex++] = a;  //point 0
	vertices[curVertex++] = b; 
	vertices[curVertex++] = c;
	vertices[curVertex++] = a;
	vertices[curVertex++] = c;
	vertices[curVertex++] = d;

	dxr->device->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 2, vertices, sizeof(PositionTextured));

	delete [] vertices;



}

void Bubbles::drawClusters()
{	
	if (!_enabled) return;

	foreach (Cluster *c, _clusters)
	{
		c->adjustFieldStrength();
	}

	QColor colorList[] = {QColor(Qt::red), QColor(Qt::green), QColor(Qt::blue), QColor(Qt::cyan), QColor(Qt::magenta), QColor(Qt::yellow)};
	int numColors = 6;
	int colorId = 0;

	dxr->switchToPerspective();
	dxr->device->SetTexture(0, NULL);
	dxr->device->SetTransform(D3DTS_WORLD, &dxr->identity);
	
	if (_drawWire)
		dxr->device->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );

	dxr->device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);	
	dxr->device->SetFVF(D3DFVF_XYZ);
	dxr->device->SetRenderState(D3DRS_ZENABLE, false);


	for (int i = 0; i < _clusters.size(); i++)
	{		
		Cluster *c = _clusters[i];
		
		c->draw(colorList[colorId], _highQuality ? BubbleClusters::highResolutionStepSize : BubbleClusters::lowResolutionStepSize);

		colorId++;
		colorId = colorId % numColors;		
	}	

	dxr->device->SetFVF(PositionNormalTextured::GetFlexibleVertexFormat());
	dxr->device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
	dxr->device->SetRenderState(D3DRS_ZENABLE, true);
	dxr->device->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
}

void Bubbles::reset()
{
	_clusters.clear();
}

void Bubbles::drawDebugClusters()
{
	if (!_enabled) return;

	dxr->switchToPerspective();
	dxr->device->SetTexture(0, NULL);
	dxr->device->SetTransform(D3DTS_WORLD, &dxr->identity);

	//if (_drawWire)
	//	dxr->device->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );

	dxr->device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);	
	dxr->device->SetFVF(D3DFVF_XYZ);
	dxr->device->SetRenderState(D3DRS_ZENABLE, false);

	for (int i = 0; i < _clusters.size(); i++)
	{		
		Cluster *c = _clusters[i];

		c->drawDebug();
		

	}	

	dxr->device->SetFVF(PositionNormalTextured::GetFlexibleVertexFormat());
	dxr->device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
	dxr->device->SetRenderState(D3DRS_ZENABLE, true);
	dxr->device->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
}



