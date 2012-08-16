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
#include "BT_StackIcon.h"
#include "BT_Pile.h"
#include "BT_BumpObject.h"
#include "BT_SceneManager.h"
#include "BT_Selection.h"	
#include "BT_Util.h"
#include "BT_WindowsOS.h"
#include "BT_GLTextureManager.h"
#include "BT_OverlayComponent.h"
#include "BT_LassoMenu.h"

#ifdef DXRENDER
#include "BT_DXRender.h"
#endif

MakeStackIcon::MakeStackIcon()
{
	vertStackPile = NULL;

	reset();
}

MakeStackIcon::~MakeStackIcon()
{
	if (vertStackPile)
	{
		DeletePile(vertStackPile);
	}
}

void MakeStackIcon::undoCross()
{
	sel->clear();

	for (uint i = 0; i < vertStackPile->getNumItems(); i++)
	{
		sel->add((*vertStackPile)[i]);
	}
}

void MakeStackIcon::doCross()
{
	Bounds bounds;
	Vec3 cent;

	vector<BumpObject *> actorList = sel->getBumpObjects();

	// Crossed to Make Pile
	vertStackPile = new Pile();

	if (!_onCrossedHandler.empty())
		_onCrossedHandler(vertStackPile);

	// Save this as a significant Action
	winOS->SetConditionalFlag(SignificantChange, true);

	// Add and Stack
	for (uint i = 0; i < actorList.size(); i++)
	{
		if (actorList[i]->isPilable(SoftPile))
			vertStackPile->addToPile(actorList[i]);
	}

	vertStackPile->stack(position);
	vertStackPile->sortBySize();

	sel->clear();
	sel->add((BumpObject *) vertStackPile);

	printUnique("MakeStackIcon::doCross", QT_TR_NOOP("Pile created via Lasso'n'Cross gesture"));

	active = true;
}

void MakeStackIcon::update()
{
	// Update the alpha
	if (alpha < 1.0f)
	{
		alpha += alphaInc;
		alpha = clampVals(alpha, 0.0f, 1.0f);
	}
}

bool MakeStackIcon::isEnabled()
{
	return enabled;
}

void MakeStackIcon::setEnable(bool e)
{
	vector<BumpObject *> objectList;
	float avgX = 0.0f, avgZ = 0.0f;

	if (e)
	{
		objectList = sel->getBumpObjects();

		// Get the center of the selection
		for (uint i = 0; i < objectList.size(); i++)
		{
			avgX += objectList[i]->getActor()->getGlobalPosition().x;
			avgZ += objectList[i]->getActor()->getGlobalPosition().z;
		}

		// account for the lasso centroid (we can pull towards the centroid by setting the weight)
		int lassoWeight = 1;
		if (lassoWeight > 0)
		{
			Vec3 lassoCentroid = lassoMenu->getCentroid();
			Vec3 worldLassoCentroid = ClientToWorld((int)lassoCentroid.x, (int)lassoCentroid.y, 0);
			for (int i = 0; i < lassoWeight; ++i)
			{
				avgX += worldLassoCentroid.x;
				avgZ += worldLassoCentroid.z;
			}
		}

		// Set the position of the stack Icon
		setPosition(Vec3(avgX / (objectList.size() + lassoWeight), 0.0f, avgZ / (objectList.size() + lassoWeight)));

		// Fade In
		alphaInc = 0.05f;
	}else{
		// Fade Out
		alphaInc = -0.05f;

		reset();
	}

	enabled = e;
}

void MakeStackIcon::reset()
{
	numTimesCrossed = 0;
	enabled = false;
	size = 20;
	alpha = 0.0f;
	alphaInc = 0.05f;
	vertStackPile = NULL;
	active = false;
}

void MakeStackIcon::onRender(uint flags)
{
	if (!enabled)
		return;
#ifdef DXRENDER
	D3DMATERIAL9 stackIconMaterial = dxr->textureMaterial;
	stackIconMaterial.Diffuse = D3DXCOLOR(1, 1, 1, ease(alpha));
	dxr->device->SetMaterial(&stackIconMaterial);
	Mat33 orientation(NxQuat(90, Vec3(1,0,0)));
	dxr->renderSideLessBox(Vec3(position.x, 10, position.z), orientation, Vec3(size / 2.0f, -size / 2.0f, 1), texMgr->getGLTextureId("widget.vertStack"));
	dxr->device->SetMaterial(&dxr->textureMaterial);
#else
	glPushAttribToken token(GL_ENABLE_BIT);

	//glColor4f(72.0f/255., 149.0f/255., 234.0f/255., 1.0);
	if (alpha == 1.0)
		glColor4d(1, 1, 1, 1);
	else glColor4d(1, 1, 1, ease(alpha));

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texMgr->getGLTextureId("widget.vertStack"));
	glEnable(GL_BLEND);

	glPushMatrix();
	glTranslated(position.x, 0, position.z);
	glScalef(size / 2.0f, 0.2f, size / 2.0f);
	glBegin(GL_TRIANGLE_FAN);
	{
		glNormal3f(0,1,0);
		glTexCoord2f(1,1); 	glVertex3f(-1,1,1);
		glTexCoord2f(0,1); 	glVertex3f(1,1,1);
		glTexCoord2f(0,0); 	glVertex3f(1,1,-1);
		glTexCoord2f(1,0); 	glVertex3f(-1,1,-1);
	}
	glEnd();
	glPopMatrix();
#endif
}

Vec3 MakeStackIcon::getCorner(int i)
{
	if (!inRange(i, 1, 4))
	{
		return Vec3(0.0f);
	}

	// 1 = bottomR, 2 = topL, 3 = topR, 4 = bottomL
	switch (i)
	{
		case 1:
			return Vec3(position.x - (size / 2.0f), 0, position.z - (size / 2.0f));

		case 2:
			return Vec3(position.x + (size / 2.0f), 0, position.z + (size / 2.0f));

		case 3:
			return Vec3(position.x - (size / 2.0f), 0, position.z + (size / 2.0f));

		default:
			return Vec3(position.x + (size / 2.0f), 0, position.z - (size / 2.0f));
	}
}

//Check for cross with Diagonal line-segment defined from the top-L to bottom-R corner of square.
//If it crosses, perform actions with Cross()
bool MakeStackIcon::checkForCross(Vec3 currPoint, Vec3 prevPoint)
{
	Vec3 target[4];

	if (enabled)
	{
		// Create a bounding box where the StackIcon resides
		for (uint i = 0; i < 4; i++)
		{
			target[i] = WorldToClient(getCorner(i + 1));
			target[i].z = 0.0f;

			// Corner Index:
			// 1          2
			//  +--------+
			//  |        |
			//  |  ICON  |
			//  |        |
			//  +--------+
			// 3          0
		}

		// Crossing target is defined as -size/2 to size/2 on the z-axis, centered at 'position'
		if (currPoint.x >= target[3].x && currPoint.x <= target[0].x &&
			currPoint.y <= target[3].y && currPoint.y >= target[1].y)
		{
			// ensure that we're not hitting a straight line
			if (lassoMenu->getCurvatureRatio() > 1.2f)
			{
				return lassoMenu->isTailVectorPointingToStackIcon(20.0f);
			}
		}
	}

	return false;
}

void MakeStackIcon::onMouseMove(Vec2 &pt)
{
	if (enabled)
	{
		// Check to see if we have crossed the center
		if (checkForCross(pt, lastMousePt))
		{
			// First time entering the icon bounding box
			if (!prevPointEntered)
			{
				if (!vertStackPile)
				{
					doCross();
				}else{
					undoCross();
				}
			}

			// Return true if there was a cross
			prevPointEntered = true;
		}else{
			prevPointEntered = false;
		}
	}

	lastMousePt = pt;
}

void MakeStackIcon::setPosition(Vec3 val)
{
	position = val;
}

Vec3 MakeStackIcon::getPosition() const
{
	return position;
}

bool MakeStackIcon::isActive() const
{
	// This signifies that the StackIcon has been crossed atleast once
	return active;
}

void MakeStackIcon::setActive(bool val)
{
	active = val;
}

void MakeStackIcon::setOnCrossedHandler( boost::function<void(Pile*)> onCrossedHandler )
{
	_onCrossedHandler = onCrossedHandler;
}