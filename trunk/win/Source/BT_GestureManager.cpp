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
#include "BT_Gesture.h"
#include "BT_OverlayComponent.h"
#include "BT_SceneManager.h"
#include "BT_GLTextureManager.h"
#include "BT_Util.h"
#include "BT_WindowsOS.h"
#include "BT_Authorization.h"

#ifdef DXRENDER
#include "BT_DXRender.h"
#include "BT_Vertex.h"
#endif

#include "BT_GestureManager.h"

// Gesture Includes
#include "BT_PinchZoomGesture.h"
#include "BT_CameraPanGesture.h"
#include "BT_CameraZoomGesture.h"
#include "BT_ScrunchGesture.h"
#include "BT_PhotoCropGesture.h"
#include "BT_LeafingGesture.h"
#include "BT_RightClickGesture.h"
#include "BT_TapZoomGesture.h"
#include "BT_RotateDesktopGesture.h"
#include "BT_PullDownWallsGesture.h"
#include "BT_FanOutGesture.h"
#include "BT_ShoveGesture.h"

GestureManager::GestureManager() :
	_gestureContext(),
	_activeGesture(NULL),
	_eligibleGestures(),
	_goProMessages()
{
	// After creating a new gesture, add it to the GestureManager
	// The order that these gestures are added in determine priority,
	// first being highest priority.
	
	// Other than right-click, the gestures are Pro-only
	addGesture(new RightClickGesture());
	
	if (GLOBAL(settings).freeOrProLevel == AL_PRO)
	{
		addGesture(new ScrunchGesture());
		addGesture(new PhotoCropGesture());
		addGesture(new TapZoomGesture());
		addGesture(new LeafingGesture());
		//addGesture(new FanOutGesture());
		addGesture(new PinchZoomGesture());
		addGesture(new RotateDesktopGesture());
		addGesture(new PullDownWallsGesture());
		addGesture(new ShoveGesture());
		addGesture(new CameraPanGesture());
		addGesture(new CameraZoomGesture());
	}
	
	_eligibleGestures = _gestures;
}

GestureManager::~GestureManager()
{
	QListIterator<Gesture *> it(_gestures);
	while(it.hasNext())
	{
		delete it.next();
	}

	QListIterator<TextOverlay *> textIt(_goProMessages);
	while(textIt.hasNext())
	{
		delete textIt.next();
	}
}

void GestureManager::addGesture(Gesture *gesture)
{
	_gestures.append(gesture);
}

void GestureManager::processGestures()
{
	if (!_activeGesture)
	{
		// Iterate over the list of eligible gestures
		QMutableListIterator<Gesture *> it(_eligibleGestures);
		while(it.hasNext())
		{
			Gesture * currentGesture = it.next();
			Gesture::Detected status = currentGesture->isRecognized(&_gestureContext);
			if(status == Gesture::Yes)
			{
				_activeGesture = currentGesture;
				break;
			}
			else if (status == Gesture::No)
			{
				// This gesture is FOR SURE not recognized. Remove it from the list.
				it.remove();
			}
		}
	}
	if (_activeGesture)
	{
		if (_activeGesture->isProcessing())
			_activeGesture->processGesture(&_gestureContext);

		if (!_activeGesture->isProcessing() && _gestureContext.getNumActiveTouchPoints() == 0)
			clear();
	}
}

bool GestureManager::isGestureActive()
{
	return _activeGesture != NULL;
}

// Clear the gesture context and all of the gestures
void GestureManager::clear()
{
	QListIterator<Gesture *> it(_gestures);
	while(it.hasNext())
	{
		it.next()->clear();
	}
	
	if (GLOBAL(settings).freeOrProLevel != AL_PRO)
	{
		// Make sure all "go pro" messages are turned off
		QMutableListIterator<TextOverlay*> textIt(_goProMessages);
		while (textIt.hasNext())
		{
			textIt.next()->getStyle().setVisible(false);
		}
	}
	_gestureContext.clear();
	_eligibleGestures = _gestures;
	_activeGesture = NULL;
}

void GestureManager::onRender()
{
	if (_gestureContext.getNumActiveTouchPoints() > 0)
	{
#ifndef DXRENDER
		glBindTexture(GL_TEXTURE_2D, NULL);
#endif
		QList<Path*> activeTouchPaths = _gestureContext.getActiveTouchPaths();
		QMutableListIterator<TextOverlay*> textIt(_goProMessages);

		// If we are not Pro users, we don't get
		// multi-touch gestures. Show a "Go Pro!" message
		// when more than two fingers are on the touch
		// screen.
		bool showProMessages = GLOBAL(settings).freeOrProLevel != AL_PRO && activeTouchPaths.size() > 1;
		
		// Render the gesture
		if (_activeGesture && _activeGesture->isProcessing())
			_activeGesture->onRender();

		// Render a small blob for every active touch point
		QListIterator<Path*> it(activeTouchPaths);
		while (it.hasNext())
		{
			TouchPoint& tp = it.next()->getLastTouchPoint();
			renderCursor(&tp);
			
			if (showProMessages)
			{
				TextOverlay* message = NULL;
				if (textIt.hasNext())
				{
					message = textIt.next();
				}
				else
				{
					// We do not have enough message overlays,
					// make another one.
					textIt.insert(CreateGoProFingerMessage());
					message = textIt.value();
					printTimedUnique(QString("MT_GO_PRO"), 6, QT_TR_NOOP("Get awesome Multi-Touch gestures in the\nBumpTop Pro version at www.bumptop.com"));
				}
				
				// Set the messages to follow the finger points
				message->getStyle().setVisible(true);
				message->setPosition(Vec3(tp.x - 30.0f, winOS->GetWindowHeight() - tp.y + 50.0f, 0.0f));
			}
		}

		// If we have more messages than finger points,
		// make sure they're turned off
		while (textIt.hasNext())
		{
			textIt.next()->getStyle().setVisible(false);
		}
	}
}

void GestureManager::renderCursor(TouchPoint *touchPoint)
{
#ifdef DXRENDER
	const float ringSizePercent = 1.5f;

	IDirect3DTexture9* touchPointTexture = texMgr->getGLTextureId("multitouch.touchpoint");
	if (!touchPointTexture)
	{
		LOG(QString_NT("'multitouch.touchpoint' not loaded. Not rendering touch points"));
		return;
	}

	dxr->device->SetTexture(0, touchPointTexture);
	dxr->device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	dxr->device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

	Vec3 size(touchPoint->width * ringSizePercent, touchPoint->height * ringSizePercent, 0.0f);
	Vec3 offset(touchPoint->x - (size.x / 2.0f), winOS->GetWindowHeight() - touchPoint->y - (size.y / 2.0f), 0.0f);
	
	dxr->renderBillboard(offset,
						 size,
						 D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f));
	
	dxr->device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	dxr->device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);

#else
	POINT p = {touchPoint->x, touchPoint->y};
	glPushAttribToken token(GL_ENABLE_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	
	glColor4ub(255, 255, 255, 96);

	Vec3 center((GLfloat)p.x, (GLfloat)winOS->GetWindowHeight() - p.y, 0.0f);
	DrawEllipse(center, (float)touchPoint->width, (float)touchPoint->height);
#endif
}

void GestureManager::setLogFileStream( QTextStream *stream )
{
	_logFileStream = stream;

	QListIterator<Gesture *> it(_gestures);
	while(it.hasNext())
		it.next()->setLogFileStream(stream);
}