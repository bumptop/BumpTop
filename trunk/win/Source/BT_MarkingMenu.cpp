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
#include "BT_BumpObject.h"
#include "BT_MarkingMenu.h"
#include "BT_FontManager.h"
#include "BT_MenuActionManager.h"
#include "BT_RenderManager.h"
#include "BT_SceneManager.h"
#include "BT_Selection.h"
#include "BT_WindowsOS.h"
#include "BT_LassoMenu.h"
#include "BT_Util.h"
#include "BT_MarkingMenu.h"
#include "BT_StatsManager.h"
#include "BT_OverlayComponent.h"
#include "BT_Logger.h"

#ifdef DXRENDER
#include "BT_DXRender.h"
#include "BT_Vertex.h"
#include "BT_WebActor.h"
#endif

MarkingMenu::MarkingMenu()
: themeManagerRef(themeManager)
{
	// Set up all the Menu Actions
	enabled = false;
	maxDeadzone = 35;
	normalThickness = 110;
	maxThickness = 135;
	animating = false;
	ballisticMode = false;
	flyoutHidden = false;
	lastSelectedObject = NULL;
	updateMenu = true;
	currHit = NULL;
	
	// setup the more options menu delay
	// NOTE: this is disabled for now
	// _moreOptionsInvokeDelayTimer.setTimerEventHandler(boost::bind<void>(&MarkingMenu::launchMoreOptionsMenuOnHover, this));

	themeManager->registerThemeEventHandler(this);
}

MarkingMenu::~MarkingMenu()
{
	if (shared_ptr<ThemeManager> tm = themeManagerRef.lock())
		tm->unregisterThemeEventHandler(this);
}

void MarkingMenu::launchMoreOptionsMenuOnHover()
{
	// if we are still waiting on the menu action, then show the more options menu
	/*
	POINT p;
	GetCursorPos(&p);
	if (ballisticMode || isHittingMenuWithCallback(Vec3(p.x, p.y, 0.0f), Key_MoreOptions))
	{
		p.x = centroid.x;
		p.y = centroid.y + curThickness - 5;
		Key_MoreOptionsHelper(p, true);
	}
	*/
}

// Calculates angles, biases, thicknesses, etc. for activeMenu layout. Puts result in currLayout
void MarkingMenu::calculateMenuLayout() 
{
	float sliceSize;
	float startDegreePref = 0;

	// Poll the MenuActionManager and get the Menu Actions that can be used in the current selection
	vector<MenuAction *> activeMenu = menuManager->determineMenuActions(sel);

	for (uint i = 0; i < activeMenu.size(); i++)
	{
		// Sort the top and lower (Ellipsis) level actions into groups
		if (activeMenu[i]->getDegreePreference() != Ellipsis)
		{
			topLevelActions.push_back(activeMenu[i]);
		}else{
			lowerLevelActions.push_back(activeMenu[i]);
		}

		// Have the customizer refresh this MenuAction
		activeMenu[i]->refreshCustomizer();
	}

	// Just in case (this should always pass)
	assert(topLevelActions.size() > 0);

	// Get the slice Size
	sliceSize = 360.0f / float(topLevelActions.size());

	// Find the First Menu Position
	for (uint i = 0; i < topLevelActions.size(); i++)
	{
		if (topLevelActions[i]->getDegreePreference() != NoPref)
		{
			startDegreePref = topLevelActions[i]->getDegreePreference();
			i = topLevelActions.size();
		}
	}

	// Create the Menu Slices
	for (uint i = 0; i < topLevelActions.size(); i++)
	{
		MenuLocation menuPos;

		// Setup the Pieces of the menu, first index is aligned with North, and goes clockwise
		menuPos.startDegrees = startDegreePref - (sliceSize / 2);
		menuPos.endDegrees = startDegreePref + (sliceSize / 2);
		menuPos.startRadius = maxDeadzone;
		menuPos.endRadius = normalThickness;

		// Keep all values below 360 Degrees
		if (menuPos.endDegrees > 360)
		{
			menuPos.startDegrees -= 360;
			menuPos.endDegrees -= 360;
		}

		menuLayout.push_back(menuPos);
		startDegreePref += sliceSize;
	}

	// Put the menuActions where they belong
	pollMenuActionBiases();
}

void MarkingMenu::pollMenuActionBiases()
{
	// Polls the Biases of all activeMenu actions
	for (uint j = 0; j < topLevelActions.size(); j++)
	{
		// Loop through all the actions and attempt to put the priority actions into their places
		for (uint i = 0; i < menuLayout.size(); i++)
		{
			if (topLevelActions[j]->getDegreePreference() >= 0.0f &&
				topLevelActions[j]->getDegreePreference() >= menuLayout[i].startDegrees &&
				topLevelActions[j]->getDegreePreference() <= menuLayout[i].endDegrees &&
				menuLayout[i].menuAction == NULL)
			{
				// If available, put the menuAction in the place where it belongs
				menuLayout[i].menuAction = topLevelActions[j];
				i = menuLayout.size();
			}
		}
	}

	// Put in remaining options (and NoPref Actions) into the left over slots
	for (uint i = 0; i < topLevelActions.size(); i++)
	{
		bool foundAction = false;

		// Find any menuActions that are not included in the menuLayout
		for (uint j = 0; j < menuLayout.size(); j++)
		{
			if (menuLayout[j].menuAction && menuLayout[j].menuAction == topLevelActions[i])
			{
				foundAction = true;
			}
		}

		if (!foundAction)
		{
			// Look for an empty spot to put this remaining action
			for (uint k = 0; k < menuLayout.size(); k++)
			{
				if (menuLayout[k].menuAction == NULL)
				{
					menuLayout[k].menuAction = topLevelActions[i];
					k = menuLayout.size();
				}
			}
		}
	}
}

void MarkingMenu::calculateOnScreenPosition()
{
	// If its too far over on the right side, move it slightly left
	if (centroid.x + maxThickness > winOS->GetWindowWidth())
	{
		centroid.x = centroid.x - (centroid.x + maxThickness - winOS->GetWindowWidth());
	}

	// Its too far left, move it back to the right
	if (centroid.x - maxThickness < 0)
	{
		centroid.x += -(centroid.x - maxThickness);
	}

	// If its too far over on the top side, move it slightly down
	if (centroid.y + maxThickness > winOS->GetWindowHeight())
	{
		centroid.y = centroid.y - (centroid.y + maxThickness - winOS->GetWindowHeight());
	}

	// Its too low, move it slightly upwards
	if (centroid.y - maxThickness < 0)
	{
		centroid.y += -(centroid.y - maxThickness);
	}
}


MenuAction *MarkingMenu::hitTest(Vec2 &in)
{
	float angle;
	Vec2 deg = centroid - in;
	MenuAction *menuAction = NULL;

	if (menuLayout.size() > 0)
	{
		// Get the angle
		angle = AngleBetweenVectors(Vec2(0, 1, 0), deg) * 180 / PI;

		if (deg.distanceSquared(Vec2(0, 0, 0)) >= maxDeadzone * maxDeadzone &&
			(ballisticMode || deg.distanceSquared(Vec2(0, 0, 0)) <= normalThickness * normalThickness))
		{
			while (!menuAction)
			{
				// Set lastHit and currHit for Animation purposes
				for (uint i = 0; i < menuLayout.size(); i++)
				{
					// Find out which menuAction belongs to this click
					if (angle >= menuLayout[i].startDegrees && angle <= menuLayout[i].endDegrees)
					{
						menuAction = menuLayout[i].menuAction;
						break;
					}
				}

				// Adjust the Angle to compensate for Angles that are too far in the negatives
				if (menuAction == NULL)
				{
					if (angle < 0)
					{
						angle += 360;
					}
				}
			}
		}
	}

	return menuAction;
}

void MarkingMenu::init()
{
	float scale = FontManager::getDPIScale();
	normalThickness *= scale;
	maxThickness *= scale;
	maxDeadzone *= scale;
}

// Renderable Derivations
void MarkingMenu::update()
{
	unsigned char alphaInc = 20;

	animating = false;

	if (enabled)
	{
		// Animate the alpha
		if (innerColor.bigEndian.a < 255)
		{
			if (innerColor.bigEndian.a + alphaInc < 255)
			{
				innerColor.bigEndian.a += alphaInc;
				textColor.bigEndian.a += alphaInc;
				subTextColor.bigEndian.a += alphaInc;
				animating = true;
			}else{
				innerColor.bigEndian.a = 255;
				textColor.bigEndian.a = 255;
				subTextColor.bigEndian.a = 255;
				animating = true;
				rndrManager->invalidateRenderer();
			}
		}

		// Animate Thickness Size
		if (!sizeMultiplier.empty())
		{
			// Increase the size of the thicknesses
			curThickness = normalThickness * sizeMultiplier.front();
			curDeadzone = maxDeadzone * sizeMultiplier.front();

			updateMenu = true;
			sizeMultiplier.pop_front();
		}
	}

	if (updateMenu || ballisticMode)
	{
		animating = true;
		updateMenu = false;

		rndrManager->invalidateRenderer();
	}
}

enum
{
	REGULAR_LABEL = 0,
	ADDITIONAL_LABEL = 1
};

#ifdef DXRENDER
void MarkingMenu::renderBackground()
{
	// How many degrees apart each vertex is
	const float complexity = 8.0f;
	const float blackLineAlphaPerc = 0.15f;
	const float sliceThickness = curThickness;

	// Adjust the alpha value of the outline colours
	borderColor.bigEndian.a = (uint)(innerColor.bigEndian.a * blackLineAlphaPerc);

	// Convert all the colours to D3D colours
	D3DCOLOR outerDXColor = D3DCOLOR(outerColor);
	D3DCOLOR innerDXColor = D3DCOLOR(innerColor);
	D3DCOLOR outerHighlightDXColor = D3DCOLOR(outerHighlightColor);
	D3DCOLOR innerHighlightDXColor = D3DCOLOR(innerHighlightColor);
	D3DCOLOR outerDisabledDXColor = D3DCOLOR((outerDXColor & 0xff000000) | ((outerDXColor & 0xfefefe) >> 1));
	D3DCOLOR innerDisabledDXColor = D3DCOLOR((innerDXColor & 0xff000000) | ((innerDXColor & 0xfefefe) >> 1));
	D3DCOLOR borderDXColor = D3DCOLOR(borderColor);
		
	// Set up the necessary render states
	dxr->device->SetFVF(PositionColoured::GetFlexibleVertexFormat());
	dxr->device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
	dxr->device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	dxr->device->SetRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_COLOR1);
	
	// The size here is determined by the amount of vertices it takes to represent a circle
	// given a certain complexity (degrees between vertices). This is rounded up and 1 is added
	// since the circle must close and draw the first vertex again. Because the wedges are drawn
	// independently, the start and end degree positions are shared and as of now there is no
	// method that rejects these redundant vertices.
	uint outlineLen = (int)ceilf(360.0f / complexity) + 1 + (menuLayout.size() * 2);
	D3DXVECTOR2* outerOutlineVertices = new D3DXVECTOR2 [outlineLen];
	// The inner circle could have less vertices since it is smaller
	D3DXVECTOR2* innerOutlineVertices = new D3DXVECTOR2 [outlineLen];
	
	// Index of the line that draws the inner and outer circle outline
	uint outlineIndex = 0;

	for (uint menuIndex = 0; menuIndex < menuLayout.size(); menuIndex++)
	{
		// Calculate the space needed to store all the vertices of the wedge
		uint wedgeFillLen = ((uint)ceilf((menuLayout[menuIndex].endDegrees - menuLayout[menuIndex].startDegrees) / complexity) + 1) * 2;
		PositionColoured* wedgeFillVertices = new PositionColoured [wedgeFillLen];
		
		// Menu separators
		QList<D3DXVECTOR2> leftLine, rightLine;

		// Index of the triangle strip that renders the circle
		uint fillIndex = 0;
		
		D3DCOLOR currentOuterCol = outerDXColor;
		D3DCOLOR currentInnerCol = innerDXColor;

		// If this is a disabled wedge, darken it
		if (!menuLayout[menuIndex].menuAction->isEnabled())
		{
			currentOuterCol = outerDisabledDXColor;
			currentInnerCol = innerDisabledDXColor;
		}
		else if (menuLayout[menuIndex].menuAction == currHit)
		{
			// Use the highlighted colour if this wedge is selected
			currentOuterCol = outerHighlightDXColor;
			currentInnerCol = innerHighlightDXColor;
		}
		
		for (float degree = menuLayout[menuIndex].startDegrees; degree <= menuLayout[menuIndex].endDegrees; degree += complexity)
		{
			float radian = D3DXToRadian(degree);
			
			// Here we generate the coordinates for the inner and outer circle. We can then use them in drawing the outline of the outer circle and
			// inner circle
			// We don't subtract the window height from these coordinates because the renderLine() function and underlying IDirect3DLine interface
			// renders with the correct coordinate system
			outerOutlineVertices[outlineIndex] = D3DXVECTOR2(centroid.x + (sliceThickness * sin(radian)), centroid.y - (sliceThickness * cos(radian)));
			innerOutlineVertices[outlineIndex++] = D3DXVECTOR2(centroid.x + (curDeadzone * sin(radian)), centroid.y - (curDeadzone * cos(radian)));	

			// We also use the very same coordinates as above for the wedge itself (Filling it in)
			// Subtract the window height from these positions because the rendering coordinate system is flipped
			wedgeFillVertices[fillIndex++] = PositionColoured(D3DXVECTOR3(outerOutlineVertices[outlineIndex - 1].x, winOS->GetWindowHeight() - outerOutlineVertices[outlineIndex - 1].y, 0), currentOuterCol);
			wedgeFillVertices[fillIndex++] = PositionColoured(D3DXVECTOR3(innerOutlineVertices[outlineIndex - 1].x, winOS->GetWindowHeight() - innerOutlineVertices[outlineIndex - 1].y, 0), currentInnerCol);

			// If the spacing of the vertices does not fit evenly into the wedge, make sure we close the gap
			if (degree + complexity > menuLayout[menuIndex].endDegrees && degree < menuLayout[menuIndex].endDegrees)
				degree = menuLayout[menuIndex].endDegrees - complexity;
		}

		// Draw the wedge (filled)
		dxr->device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, fillIndex - 2, wedgeFillVertices, sizeof(*wedgeFillVertices));
		
		// Render left separator line
		leftLine.append(innerOutlineVertices[outlineIndex - (fillIndex / 2)]);
		leftLine.append(outerOutlineVertices[outlineIndex - (fillIndex / 2)]);
		dxr->renderLine(leftLine, borderColor.asQColor());

		// Render right separator line
		rightLine.append(innerOutlineVertices[outlineIndex - 1]);
		rightLine.append(outerOutlineVertices[outlineIndex - 1]);
		dxr->renderLine(rightLine, borderColor.asQColor());

		delete [] wedgeFillVertices;
	}
	
	// Draw the circle's outer outline
	dxr->renderLine(outerOutlineVertices, outlineIndex, borderDXColor);

	// Draw the circle's inner outline
	dxr->renderLine(innerOutlineVertices, outlineIndex, borderDXColor);

	delete [] outerOutlineVertices;
	delete [] innerOutlineVertices;

	// Reset the necessary render states
	dxr->device->SetFVF(PositionTextured::GetFlexibleVertexFormat());
	dxr->device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	dxr->device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	dxr->device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	dxr->device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
}
#endif

void MarkingMenu::onRender(uint flags)
{
	float aRad = 0.0f, radians = PI / 180, rad;
	Vec2 textCenter;
#ifdef DXRENDER
	QList<Vec2> ballisticsLine;
#else
	const int complexity = 8; // This controls the size of each Slice
	float incr, endDeg;
	float blackLineAlphaPerc = 0.15f, sliceThickness;
#endif
	if (enabled)
	{
		if (!(flags & RenderSkipModelViewChange))
			switchToOrtho();
#ifdef DXRENDER

		dxr->device->SetTransform(D3DTS_WORLD, &dxr->identity);
		renderBackground();

		if (ballisticMode)
		{
			ballisticsLine.append(centroid);
			ballisticsLine.append(lastMousePt);
			dxr->renderLine(ballisticsLine, outerColor.asQColor());
		}
#else
		// OpenGL stuffs
		glPushAttribToken token(GL_ENABLE_BIT);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
	
		for (uint j = 0; j < menuLayout.size() - 1; j++)
		{
			endDeg = menuLayout[j].endDegrees;
			incr = complexity;
			if (!menuLayout[j].menuAction->isEnabled()) glEnable(GL_POLYGON_STIPPLE);
			glBegin(GL_TRIANGLE_STRIP);
			{
				for (float i = menuLayout[j].startDegrees; i <= endDeg; i += incr)
				{
					// Radians of the next step
					aRad = i * radians;

					//sliceThickness = menuLayout[j].menuAction->getRadiusThickness();
					sliceThickness = curThickness;

					// Hilight the menu chunk that is selected
					if (currHit == menuLayout[j].menuAction)
						outerHighlightColor.setAsOpenGLColor();
					else
						outerColor.setAsOpenGLColor();

					// Draw the outside points
					glVertex2f(centroid.x + (sliceThickness * sin(aRad)), winOS->GetWindowHeight() - centroid.y + (sliceThickness * cos(aRad)));

					// Hilight the menu chunk that is selected
					if (currHit == menuLayout[j].menuAction)
						innerHighlightColor.setAsOpenGLColor();
					else
						innerColor.setAsOpenGLColor();

					// Draw the inside Points
					glVertex2f(centroid.x + (curDeadzone * sin(aRad)), winOS->GetWindowHeight() - centroid.y + (curDeadzone * cos(aRad)));

					if (endDeg - i < incr && endDeg - i > 0.0f)
					{
						// Add a tiny Slice to fill in the gap
						incr = endDeg - i;
					}
				}
			}
			glEnd();
			if (!menuLayout[j].menuAction->isEnabled()) glDisable(GL_POLYGON_STIPPLE);

			// Black Line color
			borderColor.bigEndian.a = (unsigned int) (innerColor.bigEndian.a * blackLineAlphaPerc);

			borderColor.setAsOpenGLColor();
			glBegin(GL_LINE_STRIP);
			{
				for (float i = menuLayout[j].startDegrees; i <= endDeg; i += incr)
				{
					// Radians of the next step
					aRad = i * radians;
					//sliceThickness = menuLayout[j].menuAction->getRadiusThickness();
					sliceThickness = curThickness;

					// Draw the outside points
					glVertex2f(centroid.x + (sliceThickness * sin(aRad)), winOS->GetWindowHeight() - centroid.y + (sliceThickness * cos(aRad)));

					if (endDeg - i < incr && endDeg - i > 0.0f)
					{
						// Add a tiny Slice to fill in the gap
						incr = endDeg - i;
					}
				}
			}
			glEnd();

			// Draw Dividing Lines
			glBegin(GL_LINES);
			{
				//sliceThickness = menuLayout[j].menuAction->getRadiusThickness();
				sliceThickness = curThickness;

				// Left Line
				aRad = menuLayout[j].startDegrees * radians;
				glVertex2f(centroid.x + (curDeadzone * sin(aRad)), winOS->GetWindowHeight() - centroid.y + (curDeadzone * cos(aRad)));
				glVertex2f(centroid.x + (sliceThickness * sin(aRad)), winOS->GetWindowHeight() - centroid.y + (sliceThickness * cos(aRad)));

				// Right Line
				aRad = menuLayout[j].endDegrees * radians;
				glVertex2f(centroid.x + (curDeadzone * sin(aRad)), winOS->GetWindowHeight() - centroid.y + (curDeadzone * cos(aRad)));
				glVertex2f(centroid.x + (sliceThickness * sin(aRad)), winOS->GetWindowHeight() - centroid.y + (sliceThickness * cos(aRad)));

			}
			glEnd();
		}

		// Draw a black inner circle
		glBegin(GL_LINE_LOOP);
		{
			for (float i = 0; i <= 360; i += complexity)
			{
				// Radians of the next step
				aRad = i * radians;

				// Draw the outside points
				glVertex2f(centroid.x + (curDeadzone * sin(aRad)), winOS->GetWindowHeight() - centroid.y + (curDeadzone * cos(aRad)));
			}
		}
		glEnd();

		// Ballistic mode Line Drawing
		if (ballisticMode)
		{
			outerColor.setAsOpenGLColor();

			glBegin(GL_LINES);
			{
				glVertex2f(centroid.x, winOS->GetWindowHeight() - centroid.y);
				glVertex2f(lastMousePt.x, winOS->GetWindowHeight() - lastMousePt.y);
			}
			glEnd();
		}

		glEnable(GL_TEXTURE_2D);
#endif
		// Draw the Text
		for (uint j = 0; j < menuLayout.size(); j++)
		{
			// Get the center of the Menu Slice
			rad = ((menuLayout[j].endDegrees - menuLayout[j].startDegrees) / 2 + menuLayout[j].startDegrees) * radians;
			textCenter = Vec2(centroid.x + (((curThickness - curDeadzone) / 2 + curDeadzone) * sin(rad)), 
						      winOS->GetWindowHeight() - centroid.y + (((curThickness - curDeadzone) / 2 + curDeadzone) * cos(rad)), 0.0f);
			textCenter.x = NxMath::floor(textCenter.x);
			textCenter.y = NxMath::floor(textCenter.y);

			TextPixmapBuffer * primaryText = menuLayout[j].menuAction->getPrimaryTextBuffer();
			TextPixmapBuffer * secondaryText = menuLayout[j].menuAction->getSecondaryTextBuffer();
			int x, y;
			Vec3 maxUVs;
			const QSize& primaryTextSize = primaryText->getActualSize();	
			const QSize& secondaryTextSize = secondaryText->getActualSize();
			int totalHeight = primaryTextSize.height() + secondaryTextSize.height();
			int primaryOffset = (primaryTextSize.height() - (totalHeight / 2));
			float xOffset = 0.0f;
			float yOffset = 0.0f;

#ifdef DXRENDER
			dxr->beginRenderBillboard();
#endif
			// position the additional text (in case of overlap, draw this first)			
			if (menuLayout[j].menuAction->getSecondaryTextBuffer()->getText() != "")
			{
				secondaryText->bindAsGLTexture(maxUVs);
				x = textCenter.x - (secondaryTextSize.width() / 2);
				y = textCenter.y - primaryOffset - (secondaryTextSize.height());
			
#ifdef DXRENDER
				dxr->renderBillboard(roundOffDecimals(Vec3(x, y, 0)),
									 Vec3(secondaryTextSize.width(), secondaryTextSize.height(), 0),
									 D3DXCOLOR(subTextColor),
									 Vec3(0.0f),
									 maxUVs);
#else
				const QSize& primaryBufferSize = primaryText->getBuffer().size();
				const QSize& secondaryBufferSize = secondaryText->getBuffer().size();
				subTextColor.setAsOpenGLColor();
				glPushMatrix();
				glTranslatef(x, y + (secondaryTextSize.height() - secondaryBufferSize.height()), 0);
				glBegin(GL_QUADS);
					glTexCoord2f(1,0); 	glVertex2f(secondaryBufferSize.width() + xOffset,secondaryBufferSize.height() + yOffset);
					glTexCoord2f(0,0); 	glVertex2f(xOffset,secondaryBufferSize.height() + yOffset);
					glTexCoord2f(0,1); 	glVertex2f(xOffset,yOffset);
					glTexCoord2f(1,1);	glVertex2f(secondaryBufferSize.width() + xOffset,yOffset);
				glEnd();
				glPopMatrix();
#endif
			}
			
			// position the primary text
			primaryText->bindAsGLTexture(maxUVs);
			x = textCenter.x - (primaryTextSize.width() / 2);
			y = textCenter.y - primaryOffset;
#ifdef DXRENDER
			dxr->renderBillboard(roundOffDecimals(Vec3(x, y, 0)),
								 Vec3(primaryTextSize.width(), primaryTextSize.height(), 0),
								 D3DXCOLOR(textColor),
								 Vec3(0.0f),
								 maxUVs);
			dxr->endRenderBillboard();
#else
			textColor.setAsOpenGLColor();
			glPushMatrix();
			glTranslatef(x, y + (primaryTextSize.height() - primaryTextSize.height()), 0);
			if (!menuLayout[j].menuAction->isEnabled()) 
				glEnable(GL_POLYGON_STIPPLE);
			glBegin(GL_QUADS);
				glTexCoord2f(1,0); 	glVertex2f(primaryTextSize.width() + xOffset,primaryTextSize.height() + yOffset);
				glTexCoord2f(0,0); 	glVertex2f(xOffset,primaryTextSize.height() + yOffset);
				glTexCoord2f(0,1); 	glVertex2f(xOffset,yOffset);
				glTexCoord2f(1,1);	glVertex2f(primaryTextSize.width() + xOffset,yOffset);
			glEnd();
			if (!menuLayout[j].menuAction->isEnabled()) 
				glDisable(GL_POLYGON_STIPPLE);
			glPopMatrix();
#endif	
		}

		if (!(flags & RenderSkipModelViewChange))
			switchToPerspective();
	}	
}

bool MarkingMenu::isHittingMenuWithCallback(const Vec3& pt, VoidCallback cb)
{
	MenuAction * thisHit = hitTest(Vec2(pt));
	if (thisHit)
		return (thisHit->getExecuteAction() == cb);
	else
		return false;
}

// Marking Menu Actions
void MarkingMenu::onMouseMove(Vec2 &pt)
{
	MenuAction *thisHit = NULL;

	if (enabled)
	{
		// If we are outside the boundary of the menu and our mouse is down, we are in ballistic mode
		if (!ballisticMode && (buttons & MouseButtonRight || buttons & MouseButtonLeft) && 
			pt.distanceSquared(ballisticHitPt) > maxDeadzone * maxDeadzone)
		{
			ballisticMode = true;
		}

		if (buttons == 0 || ballisticMode)
		{
			thisHit = hitTest(pt);
			lastMousePt = pt;

			if (menuLayout.size())
			{
				// Update the way the menu is rendered
				if (thisHit != currHit)
				{
					updateMenu = true;
				}

				lastHit = currHit;
				currHit = thisHit;
			}

			if (thisHit)
			{
				if (thisHit != lastHit)
				{
					if (GLOBAL(settings).showToolTips)
						printTimedUnique("MarkingMenu_Tooltip", 5, thisHit->getToolTip());
				}
			}
			else
			{
				if (scnManager->messages()->hasMessage("MarkingMenu_Tooltip"))
					dismiss("MarkingMenu_Tooltip");
			}
		}
	}
}

bool MarkingMenu::onMouseDown(Vec2 &pt, MouseButtons button)
{
	MenuAction *thisHit = NULL;
	Vec2 deg = centroid - pt;

	buttons |= button;

	if (enabled)
	{
		if (button == MouseButtonLeft)
		{
			thisHit = hitTest(pt);

			lastHit = currHit;
			currHit = thisHit;

			// Destroy if we clicked elsewhere on BumpTop
			if (!thisHit)
				destroy();

			return true;
		}
	}else{
		ballisticHitPt = pt;
	}

	return false;
}

bool MarkingMenu::onMouseUp(Vec2 &pt, MouseButtons button)
{
	buttons &= ~button;

	if (currHit && enabled)
	{
		if (button & MouseButtonLeft ||
			(ballisticMode && button & MouseButtonRight))
		{
			if (currHit->isEnabled() || currHit->getCustomAction() != NULL)
			{
				// Execute whatever currHit is
				currHit->execute();

				// record this command execution
				statsManager->getStats().bt.interaction.markingmenu.executedCommandByClick++;

				// Destroy/Fade out the Menu
				destroy();	
				return true;
			}
		}
	}

	ballisticMode = false;
	return false;
}

bool MarkingMenu::onKeyUp(KeyCombo &key)
{
	return false;
}

bool MarkingMenu::onKeyDown(KeyCombo &key)
{
	// get all the top level menu actions
	vector<MenuAction *> primaryActions = menuManager->determineMenuActions(sel);
	vector<MenuAction *>::iterator iter = primaryActions.begin();
	while (iter != primaryActions.end())
	{
		MenuAction * action = *iter;
		if (!(action->getDegreePreference() != Ellipsis))
			iter = primaryActions.erase(iter);
		else
			iter++;
	}

	// Find the menu option that uses this key
	for (uint i = 0; i < primaryActions.size(); i++)
	{
		// we ignore the ctrl key in comparing the hotkeys so that they 
		// can be handled either way
		// (only do the key check if the ctrl key is pressed and the menus are not showing
		KeyCombo hotkey = primaryActions[i]->getHotKey();
		if ((key.subKeys.isCtrlPressed && !isEnabled()) || isEnabled())
		{
			hotkey.subKeys.isCtrlPressed = key.subKeys.isCtrlPressed;

			if ((hotkey.subKeys.key > 0) && (hotkey == key))
			{
				// Highlight this option as if it was clicked on
				lastHit = currHit;
				currHit = primaryActions[i];

				if (currHit->isEnabled() || currHit->getCustomAction() != NULL)
				{
					// Render to shop the selected body
					// Try taking out the sleep afterwards
					winOS->Render();
					Sleep(50);   // so the menu doesn't disappear too quickly; disorienting to users

					// Execute whatever currHit is
					currHit->execute();

					// Select the "..." menu so it can be rendered
					if (vecContains(lowerLevelActions, currHit) != -1)
					{
						currHit = topLevelActions[0];
					}

					// Destroy the Menu
					destroy();

					ballisticMode = false;

					// record the key hit
					statsManager->getStats().bt.interaction.markingmenu.executedCommandByHotkey++;

					return true;
				}
			}
		}
	}

	// Kill the Menu if escape is pressed
	if (key.subKeys.key == KeyEscape)
		destroy();

	return false;
}

Vec2 MarkingMenu::getCentroid() 
{
	return centroid;
}

float MarkingMenu::getRadius()
{
	return curThickness;
}

void MarkingMenu::prepare()
{
	calculateMenuLayout();
}

void MarkingMenu::invoke(Vec2 &mousePos)
{
	// If we used a Lasso Menu to create the selection, setup an animation
	curDeadzone = 0.0f;
	curThickness = 0.0f;
	sizeMultiplier = lerpRange(0.3f, 1.0f, 10);

	// Create an ActiveMenu Layout
	calculateMenuLayout();

	// Turn off the LassoCircle
	if (lassoMenu->isVisible())
	{
		centroid = lassoMenu->getCentroid();
	}else{
		centroid = mousePos;
		innerColor.bigEndian.a = 0;
		textColor.bigEndian.a = 0;
		subTextColor.bigEndian.a = 0;
	}

	maxDeadzone = lassoMenu->getInnerCircleSize();
	lassoMenu->disableInnerCircle();

	// Move the menu so its always on the screen
	calculateOnScreenPosition();

	// Turn on the Marking Menu
	enabled = true;
	animating = true;
	subTextColor.bigEndian.a = textColor.bigEndian.a = innerColor.bigEndian.a;

	// mouse handling code sets pickedActor to null, so use getBumpObjects instead when selection size is 1
	if (sel->getBumpObjects().size() == 1)
		lastSelectedObject = (BumpObject*) sel->getBumpObjects().front();
	else
		lastSelectedObject = (BumpObject*) sel->getPickedActor();
	
	// if selected actor is a webactor, hide its flyout if it is visible
	if (lastSelectedObject && lastSelectedObject->isObjectType(ObjectType(BumpActor, Webpage)))
	{
		WebActor* tempWebActor = (WebActor*) lastSelectedObject;
		if (tempWebActor->isFlyoutVisible())
		{
			tempWebActor->fadeOutFlyout();
			flyoutHidden = true;
		}
	}

	GLOBAL(timeElapsed).restart();

}

void MarkingMenu::offsetFromMouse(Vec2 &mousePos)
{
	// double check if the mouse position hits the centroid
	Vec3 inward = centroid - mousePos;
	float magnitude = inward.magnitude();
	
	if ((maxDeadzone < magnitude) && (magnitude < maxThickness))
	{
		// slide it over
		float offset = (maxThickness - magnitude + GLOBAL(dblClickSize));
		inward.normalize();
		centroid += (inward * (offset));
	}
}

bool MarkingMenu::isMenuItemSelected(Vec2 &in)
{
	return hitTest(in);
}

void MarkingMenu::destroy()
{
	topLevelActions.clear();
	lowerLevelActions.clear();
	menuLayout.clear();
	enabled = false;
	ballisticMode = false;
	currHit = NULL;
	lastHit = NULL;
	updateMenu = true;

	// Clear the Lasso also
	lassoMenu->reset();

	// if selected actor is a webactor and its flyout was hidden, show it again
	if (flyoutHidden && lastSelectedObject && lastSelectedObject->isObjectType(ObjectType(BumpActor, Webpage)))
	{
		WebActor* tempWebActor = (WebActor*)lastSelectedObject;
		tempWebActor->fadeInFlyout();
		flyoutHidden = false;
	}
}

bool MarkingMenu::isAnimating()
{
	for (uint i = 0; i < menuLayout.size(); i++)
	{
		if (menuLayout[i].menuAction->isAnimating())
			return true;
	}

	return animating;
}

vector<MenuAction *> MarkingMenu::getLowerLevelActions() const
{
	return lowerLevelActions;
}

void MarkingMenu::onThemeChanged()
{
	LOG("MarkingMenu::onThemeChanged");
	innerColor = themeManager->getValueAsColor("ui.markingMenu.color.default.inner",ColorVal());
	outerColor = themeManager->getValueAsColor("ui.markingMenu.color.default.outer",ColorVal());
	innerHighlightColor = themeManager->getValueAsColor("ui.markingMenu.color.highlight.inner",ColorVal());
	outerHighlightColor = themeManager->getValueAsColor("ui.markingMenu.color.highlight.outer",ColorVal());
	borderColor = themeManager->getValueAsColor("ui.markingMenu.color.border",ColorVal());

	textColor = themeManager->getValueAsColor("ui.markingMenu.font.color",ColorVal(255,255,255,255));
	subTextColor = themeManager->getValueAsColor("ui.markingMenu.font.subTextColor",ColorVal(255,255,255,255));
	textColorDisabled = themeManager->getValueAsColor("ui.markingMenu.font.disabledColor",ColorVal(255,255,255,255));	

	QString fontName = themeManager->getValueAsFontFamilyName("ui.markingMenu.font.family","");
	textFont = FontDescription(fontName, themeManager->getValueAsInt("ui.markingMenu.font.size",12));
	subTextFont = FontDescription(fontName, themeManager->getValueAsInt("ui.markingMenu.font.subTextSize",10));
}

const ColorVal& MarkingMenu::getInnerColor() const
{
	return innerColor;
}

const ColorVal& MarkingMenu::getOuterColor() const
{
	return outerColor;
}

const ColorVal& MarkingMenu::getBorderColor() const
{
	return borderColor;
}