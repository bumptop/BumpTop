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

#ifdef ENABLE_SLIDESHOW_CLASS
#include "BT_SlideShow.h"
#include "BT_WindowsOS.h"
#include "BT_Camera.h"
#include "BT_SceneManager.h"
#include "BT_Util.h"

SlideShow::SlideShow()
{
	slideShowEnabled = false;
}

void SlideShow::setSlides(vector<uint> &slideTextureNames)
{
	// Initialization
	slideTextures = slideTextureNames;

	restart(0);
}

void SlideShow::nextSlide()
{
	if (currSlide + 1 < slideTextures.size())
	{
		// Fade in the next slide
		queuedSlide = currSlide + 1;
	}
}

void SlideShow::prevSlide()
{
	if (currSlide - 1 >= 0)
	{
		// Fade in the previous slide
		queuedSlide = currSlide - 1;
	}
}

void SlideShow::restart(int slideNum)
{
	if (slideNum < slideTextures.size() && slideNum >= 0)
	{
		globalAlpha = 1.0;
		globalAlphaInc = 0.0;
		queuedSlideAlpha = 0.0;
		queuedSlideAlphaInc = 0.0;

		currSlide = queuedSlide = slideNum;
		slideShowEnabled = true;
		disableOnFadeOut = false;

		
	}
}

void SlideShow::fadeIn()
{
	if (globalAlpha	== 0.0)
	{
		slideShowEnabled = true;
		globalAlpha = 1.0;
		globalAlphaInc = 0.0; //globalSettings.IsFastMachine ? 0.01 : 0.03;
	}
}
void SlideShow::fadeOut()
{
	// Increment the slides so that the next one fades in on top of the current one
	if (globalAlpha == 1.0)
	{
		disableOnFadeOut = true;
		slideShowEnabled = false;
		globalAlpha = 0.0;
		globalAlphaInc = 0.0; //globalSettings.IsFastMachine ? -0.01 : -0.03;

	}
}

void SlideShow::update(uint elapsedTime)
{
	// Modify the Alpha Channel
	if (globalAlphaInc || globalAlphaInc)
	{
		// Increase the alphas
		globalAlpha += (globalAlphaInc * elapsedTime);
		queuedSlideAlpha += (queuedSlideAlphaInc * elapsedTime);

		// Disable the alpha fading
		if (globalAlpha > 1.0f || globalAlpha < 0.0f)
		{
			globalAlphaInc = 0.0;
			
			if (globalAlpha < 0.0) globalAlpha = 0.0;
			if (globalAlpha > 1.0) globalAlpha = 1.0;

			// Turn off the slide show after this fade out
			if (disableOnFadeOut)
			{
				slideShowEnabled = false;
				disableOnFadeOut = false;
			}
		}

		// For the queued alpha channel, clamp it at 1.0 or 0.0
		if (queuedSlideAlpha > 1.0f || queuedSlideAlpha < 0.0f)
		{
			queuedSlideAlphaInc = 0.0;
			currSlide = queuedSlide;

			if (queuedSlideAlpha < 0.0) queuedSlideAlpha = 0.0;
			if (queuedSlideAlpha > 1.0) queuedSlideAlpha = 1.0;
		}
	}
}

void SlideShow::render()
{
#ifdef DXRENDER
	// TODO DXR
#else
	if (slideShowEnabled && slideTextures.size() > 0)
	{
		// Switch to Ortho Mode
		glPushAttribToken token(GL_ENABLE_BIT);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluOrtho2D(0, winOS->GetWindowWidth(), 0, winOS->GetWindowHeight());
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		// Set the position on the screen
 		glDisable(GL_DEPTH_TEST);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		
		glTranslatef(0.0, 0.0, 0.0);

		// ================================================================

		// Set the current texture for drawing
		glBindTexture(GL_TEXTURE_2D, slideTextures[currSlide]);
		glColor4f(1.0, 1.0, 1.0, globalAlpha);

		// Render
		glBegin(GL_POLYGON);
			glTexCoord2i(0,0); 	glVertex2i(0,0);
			glTexCoord2i(1,0); 	glVertex2i(winOS->GetWindowWidth(),0);
			glTexCoord2i(1,1); 	glVertex2i(winOS->GetWindowWidth(),winOS->GetWindowHeight());
			glTexCoord2i(0,1); 	glVertex2i(0,winOS->GetWindowHeight());
		glEnd();

		// Draw the next Slide on top of the current slide
		if (currSlide != queuedSlide)
		{
			// Set the queued texture for drawing
			glBindTexture(GL_TEXTURE_2D, slideTextures[queuedSlide]);
			glColor4f(1.0, 1.0, 1.0, globalAlpha * queuedSlideAlpha);

			// Render
			glBegin(GL_POLYGON);
				glTexCoord2i(0,0); 	glVertex2i(0,0);
				glTexCoord2i(1,0); 	glVertex2i(winOS->GetWindowWidth() / 2,0);
				glTexCoord2i(1,1); 	glVertex2i(winOS->GetWindowWidth() / 2,winOS->GetWindowHeight() / 2);
				glTexCoord2i(0,1); 	glVertex2i(0,winOS->GetWindowHeight() / 2);
			glEnd();
		}

		// ================================================================

		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);

		// Switch back to Perspective mode
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(60.0f, float(winOS->GetWindowWidth()) / float(winOS->GetWindowHeight()), GLOBAL(nearClippingPlane), GLOBAL(farClippingPlane));
		gluLookAt(cam->getEye().x, cam->getEye().y, cam->getEye().z, cam->getEye().x + cam->getDir().x, cam->getEye().y + cam->getDir().y, cam->getEye().z + cam->getDir().z, cam->getUp().x, cam->getUp().y, cam->getUp().z);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
	}
#endif
}

bool SlideShow::isEnabled()
{
	return slideShowEnabled;
}

void SlideShow::disable()
{
	slideShowEnabled = false;
}
#endif