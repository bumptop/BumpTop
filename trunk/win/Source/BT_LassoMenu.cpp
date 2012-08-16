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
#include "BT_Actor.h"
#include "BT_Camera.h"
#include "BT_FileSystemActor.h"
#include "BT_LassoMenu.h"
#include "BT_Logger.h"
#include "BT_MarkingMenu.h"
#ifdef DXRENDER
	#include "BT_DXRender.h"
	#include "BT_Vertex.h"
#endif
#include "BT_OverlayComponent.h"
#include "BT_Pile.h"
#include "BT_RenderManager.h"
#include "BT_SceneManager.h"
#include "BT_SceneManager.h"
#include "BT_Selection.h"
#include "BT_StatsManager.h"
#include "BT_TextManager.h"
#include "BT_Util.h"
#include "BT_WindowsOS.h"
#include "BT_WindowsOS.h"

LassoMenu::LassoMenu()
: themeManagerRef(themeManager)
{
	// This color is windows Blue	
	innerCircleSize = 25;
	hasLeftInnerCircle = false;
	lastLassoPtsAmt = 0;
	themeManager->registerThemeEventHandler(this);
}

LassoMenu::~LassoMenu()
{
	if (shared_ptr<ThemeManager> tm = themeManagerRef.lock())
		tm->unregisterThemeEventHandler(this);
}

void LassoMenu::onThemeChanged()
{
	LOG("LassoMenu::onThemeChanged");
	lassoColor = themeManager->getValueAsColor("ui.lasso.color.overlay",ColorVal());
}

bool LassoMenu::onMouseUp(Vec3 &pt, MouseButtons button)
{
	if (button == MouseButtonLeft)
	{
		if (!markingMenu->isEnabled())
		{
			// On mouse up, clear the lasso but keep the selection
			reset();
			return true;
		}
	}

	return false;
}

bool LassoMenu::onMouseDown(Vec3 &pt, MouseButtons button)
{
	 	
	bool rc = false;

	if (button == MouseButtonLeft && !markingMenu->isEnabled())
	{
		// See if anything is picked
		pickAndSet(int(pt.x), int(pt.y));

		// No picked Actor means that we just clicked on the floor
		if (!sel->getPickedActor())
		{
			// Clear the Lasso so we can start drawing again
			actualLassoPoints.clear();
			boundingLassoPoints.clear();
			boundingLassoBounds.setEmpty();
			lastLassoPtsAmt = 0;
			stackIcon.setEnable(false);
			innerCircle = true;
			centroid = pt;

			// Add the first point
			updateBoundingLasso(pt);
			actualLassoPoints.push_back(pt);
			rc = true;
		}

		return rc;
	}

	return false;
}

bool isCCW(const Vec3& p0, const Vec3& p1, const Vec3& p2)
{
	float m = (p1.y - p0.y) / (p1.x - p0.x);
	float b = p0.y - (p0.x * m);
	float expectedP2y = (p2.x * m) + b;
	if (p1.x <= p0.x)
		return (p2.y >= expectedP2y);
	else
		return (p2.y <= expectedP2y);
}

void LassoMenu::onMouseMove(Vec3 &pt)
{
	Vec3 distVec, nextPt, dir;
	float dist;

	// If the lasso has a size, it means were dragging
	if ((actualLassoPoints.size() > 0) && !markingMenu->isEnabled())
	{	
		if (pt.distanceSquared(actualLassoPoints.back()) > 5.0f)
		{			
			// Add More Points
			actualLassoPoints.push_back(pt);
			updateBoundingLasso(pt);

			// Check which actors are inside the Lasso (using Bounding Lasso)
			determineSelectedActors();

			// Check to see how far the mouse cursor is form the origin
			const Vec3& origin = actualLassoPoints[0];		
			dist = origin.distanceSquared(pt);

			if ((dist > (innerCircleSize * innerCircleSize)) && !hasLeftInnerCircle)
			{
				// We have left the inner circle
				hasLeftInnerCircle = true;
			}else if ((dist < (innerCircleSize * innerCircleSize)) && hasLeftInnerCircle)
			{
				// Invoke the Marking Menu and turn off the stack Icon
				stackIcon.setEnable(false);
				markingMenu->invoke(Vec2(pt.x, pt.y, 0.0f));	
		
				// record the mm invocation
				statsManager->getStats().bt.interaction.markingmenu.invokedByLasso++;
			}
		}
	}
}

void LassoMenu::onRender(uint flags)
{
	if (actualLassoPoints.size() < 3) 
		return;
#ifdef DXRENDER
	dxr->device->SetTransform(D3DTS_WORLD, &dxr->identity);
	dxr->device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG2);
	dxr->device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
	dxr->device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
	dxr->device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
	// Use texture factor to set primitive colour
	dxr->device->SetRenderState(D3DRS_TEXTUREFACTOR, D3DXCOLOR(lassoColor));
	D3DXVECTOR3 vertices [64];
	dxr->device->SetFVF(D3DFVF_XYZ);
	dxr->device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	unsigned int index = 0;
#else
	if (!(flags & RenderSkipModelViewChange))
		switchToOrtho();

	glPushAttribToken token(GL_ENABLE_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);

	glDisable(GL_CULL_FACE);

	// Lasso Geometry Drawing
	glBegin(GL_TRIANGLE_FAN);
	{
		// Set the color of the Polygons
		lassoColor.setAsOpenGLColor();
#endif
		// determine the ccw order of the lasso by checking which side of the 
		// original line between p0, p1 that p2 falls on
		const Vec3& p0 = actualLassoPoints.front();
		bool ccw = isCCW(p0, actualLassoPoints[1], actualLassoPoints[2]);
		float windowHeight = (float) winOS->GetWindowHeight();
#ifdef DXRENDER
		for (unsigned int i = 0; i < actualLassoPoints.size(); ++i)
		{
			if (index + 3 > _countof(vertices))
			{	//draw existing tris if buffer full
				dxr->device->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, index - 2, vertices, sizeof(*vertices));
				index = 0;
				vertices[index++] = D3DXVECTOR3(p0.x, windowHeight - p0.y, 0);//, D3DXCOLOR(lassoColor));
				vertices[index++] = D3DXVECTOR3(actualLassoPoints[i - 1].x, windowHeight - actualLassoPoints[i - 1].y, 0);//, D3DXCOLOR(lassoColor));
			}
			vertices[index++] = D3DXVECTOR3(actualLassoPoints[i].x, windowHeight - actualLassoPoints[i].y, 0);//, D3DXCOLOR(lassoColor));
		}
#else
		for (unsigned int i = 0; i < actualLassoPoints.size(); ++i)
			glVertex2f(actualLassoPoints[i].x, windowHeight - actualLassoPoints[i].y);
#endif
#ifdef DXRENDER
		if (index)
			dxr->device->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, index - 2, vertices, sizeof(*vertices));
		
		dxr->device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		dxr->device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
		dxr->device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
		dxr->device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		dxr->device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
		dxr->device->SetFVF(PositionTextured::GetFlexibleVertexFormat());
#else
	}
	glEnd();
	glEnable(GL_CULL_FACE);
#endif
	// Lasso Line Drawing
	RenderLasso(actualLassoPoints);
	
	// Draw the small circle that represents a marking menu
	drawInnerCircle();
#ifdef DXRENDER
#else
	if (!(flags & RenderSkipModelViewChange))
		switchToPerspective();
#endif
}

void LassoMenu::drawInnerCircle()
{
	float radians = PI / 180, aRad, miniDeadzone = 7;
	const int complexity = 18; // Must divide by 360 evenly
	ColorVal cVal = lassoColor;

	if (actualLassoPoints.size() > 1 && innerCircle)
	{
		// Set new Color
		cVal.bigEndian.a = unsigned char(0.8f * 255.0f);

		ColorVal outerColor = markingMenu->getOuterColor();
		ColorVal innerColor = markingMenu->getInnerColor();

#ifdef DXRENDER
		dxr->device->SetFVF(PositionColoured::GetFlexibleVertexFormat());
		dxr->device->SetTexture(0, NULL);
		dxr->device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
		dxr->device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
		dxr->device->SetRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_COLOR1);
		PositionColoured vertices [360 / complexity * 2 + 2]; //triangle strip
		unsigned int index = 0;
#else
		cVal.setAsOpenGLColor();
		glBegin(GL_TRIANGLE_STRIP);
		{
#endif
			for (float i = 0; i <= 360; i += complexity)
			{
				// Radians of the next step
				aRad = i * radians;
#ifdef DXRENDER
				vertices[index++] = PositionColoured(centroid.x + (innerCircleSize * sin(aRad)), 
					winOS->GetWindowHeight() - centroid.y + (innerCircleSize * cos(aRad)), 0, D3DXCOLOR(outerColor));
				vertices[index++] = PositionColoured(centroid.x + (miniDeadzone * sin(aRad)), 
						   winOS->GetWindowHeight() - centroid.y + (miniDeadzone * cos(aRad)), 0, D3DXCOLOR(innerColor));
			}
			dxr->device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, index - 2, vertices, sizeof(*vertices));
			dxr->device->SetFVF(PositionTextured::GetFlexibleVertexFormat());
			dxr->device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
			dxr->device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
			dxr->device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
			dxr->device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
#else
				// Draw the points around the center
				outerColor.setAsOpenGLColor();
				glVertex2f(centroid.x + (innerCircleSize * sin(aRad)), 
					winOS->GetWindowHeight() - centroid.y + (innerCircleSize * cos(aRad)));

				// Back to the center
				innerColor.setAsOpenGLColor();
				glVertex2f(centroid.x + (miniDeadzone * sin(aRad)), 
						   winOS->GetWindowHeight() - centroid.y + (miniDeadzone * cos(aRad)));
			}
		}
		glEnd();
#endif

		// set border color
		float blackLineAlphaPerc = 0.15f;
		ColorVal borderColor = markingMenu->getBorderColor();
		borderColor.bigEndian.a = (unsigned int) (innerColor.bigEndian.a * blackLineAlphaPerc);
#ifdef DXRENDER
		// TODO DXR
#else
		borderColor.setAsOpenGLColor();
		// Draw a black inner circle
		glBegin(GL_LINE_LOOP);
		{
			for (float i = 0; i <= 360; i += complexity)
			{
				// Radians of the next step
				aRad = i * radians;

				// Draw the outside points
				glVertex2f(centroid.x + (innerCircleSize * sin(aRad)), winOS->GetWindowHeight() - centroid.y + (innerCircleSize * cos(aRad)));
			}
		}
		glEnd();

		// Draw a black outer circle
		glBegin(GL_LINE_LOOP);
		{
			for (float i = 0; i <= 360; i += complexity)
			{
				// Radians of the next step
				aRad = i * radians;

				// Draw the outside points
				glVertex2f(centroid.x + (miniDeadzone * sin(aRad)), winOS->GetWindowHeight() - centroid.y + (miniDeadzone * cos(aRad)));
			}
		}
		glEnd();
#endif
	}
}

ColorVal LassoMenu::getLassoColor() const
{
	return lassoColor;
}

void LassoMenu::setLassoColor(ColorVal &newColor)
{
	lassoColor = newColor;
}

bool LassoMenu::isVisible() const
{
	return actualLassoPoints.size() > 0 ? true : false;
}

void LassoMenu::determineSelectedActors()
{
	int selCount = sel->getBumpObjects().size();
	vector<BumpObject *> bumpObjects = scnManager->getBumpObjects();
	Vec3 eye = cam->getEye();

	// disable text relayout when selecting a large group of items
	textManager->disableForceUpdates();

	// Check Which Actors are Inside/Crossed by Lasso Selection
	for (int i = 0; i < bumpObjects.size(); i++)
	{
		if (bumpObjects[i]->isPinned())
			continue; 

		//Skip clusters for now
		if (bumpObjects[i]->isObjectType(BumpCluster)) continue;

		if (!bumpObjects[i]->isSelected())
		{	
			Vec3 pos = bumpObjects[i]->getGlobalPosition();
			Vec3 dims = bumpObjects[i]->getDims();
			Box objBox(pos, dims, bumpObjects[i]->getGlobalOrientation());
			Vec3 objBoxPts[8];
			objBox.computePoints(objBoxPts);

			QList<Vec3> pointsToCheck;
			for (int j = 0; j < 8; ++j)
				pointsToCheck.append(objBoxPts[j]);
			pointsToCheck.append(pos);

			for (int j = 0; j < pointsToCheck.size(); ++j)
			{
				if (boundingLassoBounds.contain(WorldToClient(pointsToCheck[j], true, true)) &&
					isPointInPolygon(pointsToCheck[j], boundingLassoPoints))
				{
					sel->add(bumpObjects[i]);
					break;
				}
			}
		}
	}

	// disable text relayout when selecting a large group of items
	textManager->enableForceUpdates();

	// don't bother doing anything if the selection has not changed
	if (!((sel->getBumpObjects().size()) != selCount))
		return;

	lastLassoPtsAmt = actualLassoPoints.size();

	// Get the relevant selection
	bumpObjects = sel->getBumpObjects();

	// Check to see if the items selected are pilable
	bool isPileable = false;
	for (uint i = 0; i < bumpObjects.size(); i++)
		isPileable = isPileable || bumpObjects[i]->isPilable(SoftPile);
	if (!isPileable)
	{
		stackIcon.setEnable(false);
		return;
	}

	// Turn on the Stack Icon if we have something selected
	uint seltype = sel->getSelectionType();
	if (!markingMenu->isEnabled() && 
		seltype & MultipleFreeItems)
	{
		stackIcon.setEnable(sel->getSize() > 2);
	}else{
		// If the Marking Menu is up, we don't want a Stack Icon
		stackIcon.setEnable(false);
	}
}

void LassoMenu::updateBoundingLasso(Vec3 newPt)
{
	if (!stackIcon.isActive())
	{
		// Add it to our Bounds because we just want the outer bounds
		boundingLassoPoints.push_back(newPt);
		boundingLassoBounds.include(newPt);
	}
}

void LassoMenu::update()
{
	stackIcon.update();
}

float LassoMenu::getInnerCircleSize() const
{
	return innerCircleSize;
}

void LassoMenu::disableInnerCircle()
{
	innerCircle = false;
}

void LassoMenu::reset()
{
	hasLeftInnerCircle = false;
	actualLassoPoints.clear();
	boundingLassoPoints.clear();
	boundingLassoBounds.setEmpty();
	stackIcon.setEnable(false);
}	

void LassoMenu::init()
{
	// Register input from the mouse handler
	stackIcon.registerMouseHandler();
	registerMouseHandler(); 
}

Vec3 LassoMenu::getCentroid() const
{
	return centroid;
}

MakeStackIcon& LassoMenu::getStackIcon()
{
	return stackIcon;
}

bool LassoMenu::isTailVectorPointingToStackIcon(float delta) const
{
	// returns the angle between the tail vector and the vector to the stack icon
	if (!actualLassoPoints.empty())
	{
		Vec3 back = actualLassoPoints.back();
		Vec3 fiveBack = actualLassoPoints[NxMath::max((int) 0, (int) actualLassoPoints.size()-5)];
		Vec3 lassoDir = back - fiveBack;
		Vec3 stackIconPos = WorldToClient(stackIcon.getPosition(), true, true);
		Vec3 stackIconDir = stackIconPos - fiveBack;
		lassoDir.normalize();
		stackIconDir.normalize();
		return (acos(stackIconDir.dot(lassoDir)) * 180 / PI) < delta;
	}
	return false;
}

float LassoMenu::getCurvatureRatio() const
{
	// ensure that there are enough points
	if (actualLassoPoints.size() < 2)
		return 0.0f;

	// return the ratio of the arc length / distance from endpoint to endpoint
	float distance = actualLassoPoints.back().distance(actualLassoPoints.front());
	float arcLength = 0.0f;
	for (int i = 1; i < actualLassoPoints.size(); ++i)
	{
		arcLength += (actualLassoPoints[i].distance(actualLassoPoints[i-1]));
	}

	return (arcLength / distance);
}
