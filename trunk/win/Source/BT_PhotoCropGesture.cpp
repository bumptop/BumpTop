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
#include "BT_Camera.h"
#include "BT_FileSystemActor.h"
#include "BT_GestureContext.h"
#include "BT_GLTextureManager.h"
#include "BT_OverlayComponent.h"
#include "BT_SceneManager.h"
#include "BT_QtUtil.h"
#include "BT_Util.h"
#include "BT_WindowsOS.h"

#include "BT_PhotoCropGesture.h"

const QColor PhotoCropGesture::CROP_COLOR = QColor(153, 153, 153, 204);
const float PhotoCropGesture::CROP_LINE_WIDTH = 6.0f;
const float PhotoCropGesture::CROP_LINE_PATTERN_SCALE = 15.0f;

PhotoCropGesture::PhotoCropGesture() :
	Gesture("PhotoCrop", 2, 2, true)
{
	clearGesture();
}

PhotoCropGesture::Detected PhotoCropGesture::isRecognizedImpl(GestureContext *gestureContext)
{
	// Photo crop can only be done in slideshow mode
	if (!isSlideshowModeActive())
		return gestureRejected("Not in slideshow mode");

	_gestureTouchPaths = gestureContext->getActiveTouchPaths();

	// Find the stationary finger which "holds" the photo. If it can't be found,
	// the gesture is not recognized yet.
	// XXX: Should check that this finger is actually on the photo
	if (_gestureTouchPaths[0]->isStationary())
	{
		_stationaryPath = _gestureTouchPaths[0];
		_cropPath = _gestureTouchPaths[1];
	} 
	else if (_gestureTouchPaths[1]->isStationary())
	{
		_stationaryPath = _gestureTouchPaths[1];
		_cropPath = _gestureTouchPaths[0];
	}
	else 
	{
		return Maybe;
	}
	
	// Check if the second finger is swiping horizontally or vertically
	if (isCropPathValid())
	{
		printUnique(QString("MT_Gesture"), QT_TR_NOOP("Photo Crop Mode"));
		return gestureAccepted();
	}
	return Maybe;
}

bool PhotoCropGesture::processGestureImpl(GestureContext *gestureContext)
{
	uint numTouchPoints = gestureContext->getNumActiveTouchPoints();
	if (numTouchPoints <= 2)
	{ 
		QList<Path*> activeTouchPaths = gestureContext->getActiveTouchPaths();
		// The gesture is active as long as the "holding" finger is still down
		if (activeTouchPaths.contains(_stationaryPath))
		{
			// See if a new crop line is being drawn
			if ((numTouchPoints == 2) && !activeTouchPaths.contains(_cropPath))
			{
				if (activeTouchPaths[0] == _stationaryPath)
					_cropPath = activeTouchPaths[1];
				else
					_cropPath = activeTouchPaths[0];
			}

			return true; // Continue processing this gesture
		}
		else if (isCropPathValid())
		{
			ObjectType watchedObjectType = cam->getHighlightedWatchedActor()->getObjectType();		

			// Only crop on an image
			if (watchedObjectType != ObjectType(BumpActor, FileSystem, Image))
			{
				printUnique(QString("MT_Gesture"), QT_TR_NOOP("Only images can be cropped"));
				return false;
			}

			// Don't crop a photo frame
			if (watchedObjectType == ObjectType(BumpActor, FileSystem, PhotoFrame))
			{
				printUnique(QString("MT_Gesture"), QT_TR_NOOP("Photo frames cannot be cropped"));
				return false;
			}
			
			// We assume that the middle of the line is on the photo
			Vec3 winPoint, actorPercentage;
			winPoint.x = (_cropPath->getFirstTouchPoint().x + _cropPath->getLastTouchPoint().x) / 2;
			winPoint.y = (_cropPath->getFirstTouchPoint().y + _cropPath->getLastTouchPoint().y) / 2;
			winPoint.z = 0;

			actorPercentage = windowToActorPercent(winPoint);
			
			// Check if crop path is on the image
			if (actorPercentage.x == 0.0f && actorPercentage.y == 0.0f)
			{
				return false;
			}
			
			Vec3 midCropPoint = (_cropPath->getTotalDisplacementVector() / 2.0f) + _cropPath->getFirstTouchPoint().getPositionVector();
			Vec3 midStationaryPoint = _stationaryPath->getFirstTouchPoint().getPositionVector();
			bool result = true;
			
			float topCrop = 0.0f;
			float rightCrop = 0.0f;
			float bottomCrop = 0.0f;
			float leftCrop = 0.0f;

			if (_cropPath->isApproximatelyHorizontal())
			{
				// If the crop is happening below the stationary point, crop the bottom portion
				// Otherwise crop the top portion
				if (midCropPoint.y > midStationaryPoint.y)
					bottomCrop = actorPercentage.y;
				else
					topCrop = 1.0f - actorPercentage.y;
			}
			else
			{
				// If the crop is happening to the left of the stationary point, crop the left portion
				// Otherwise crop the right portion
				if (midCropPoint.x < midStationaryPoint.x)
					leftCrop = actorPercentage.x;
				else
					rightCrop = 1.0f - actorPercentage.x;
			}
			
			// Check if an original version of this photo exists in the hidden directory
			FileSystemActor *actor = dynamic_cast<FileSystemActor *>(cam->getHighlightedWatchedActor());
			
			if (!hasOriginalPhoto(actor))
			{
				result = backupOriginalPhoto(actor);
			}

			if (result)
			{			
				if (texMgr->cropPhoto(actor->getFullPath(), actor->getFullPath(), topCrop, rightCrop, bottomCrop, leftCrop))
				{
					cam->getRestoreOriginalPhotoControl()->show();
					printUnique(QString("MT_Gesture"), QT_TR_NOOP("Cropping..."));
					return false;
				}
			}
			
			printStr(QT_TR_NOOP("Crop failed!"));
		}
	}
	return false;
}

void PhotoCropGesture::onRender()
{
	BumpObject* photo = cam->getHighlightedWatchedActor();
	
	if (_cropPath && isCropPathValid() && photo)
	{
		QList<Vec3> pointList;
		TouchPoint& firstPoint = _cropPath->getFirstTouchPoint();
		TouchPoint& lastPoint = _cropPath->getLastTouchPoint();
		Vec3 midCropPoint = (_cropPath->getTotalDisplacementVector() / 2.0f) + _cropPath->getFirstTouchPoint().getPositionVector();
		Vec3 midStationaryPoint = _stationaryPath->getFirstTouchPoint().getPositionVector();
		
		bool horizontal, aboveOrRight;
		if (_cropPath->isApproximatelyVertical())
		{
			horizontal = false;
			
			// Find out if we are cropping the right or left side of the image
			aboveOrRight = midCropPoint.x > midStationaryPoint.x;

			Vec3 start(firstPoint.x, 0.0f, 0.0f);
			Vec3 end(firstPoint.x, lastPoint.y, 0.0f);
			if (lastPoint.y < firstPoint.y)
			{
				start.y = winOS->GetWindowHeight();
			}
			
			pointList.push_back(start);
			pointList.push_back(end);
		}
		else
		{
			horizontal = true;

			// Find out if we are cropping the top or bottom side of the image
			aboveOrRight = midStationaryPoint.y > midCropPoint.y;
			
			Vec3 start(0.0f, firstPoint.y, 0.0f);
			Vec3 end(lastPoint.x, firstPoint.y, 0.0f);
			if (lastPoint.x < firstPoint.x)
			{
				start.x = winOS->GetWindowWidth();
			}

			pointList.push_back(start);
			pointList.push_back(end);
		}
		
		// Get the world space bounding box surrounding the image
		Box box = photo->getBox();
		Vec3 points[8];
		
		float pixelsCut;
		Vec3 shiftDisplacement(0.0f);
		
		// Set the crop point to the middle of the image/screen and then adjust either the x
		// or y value of this point to the correct value determined by the crop line.
		Vec3 cropPoint(winOS->GetWindowWidth() / 2.0f, winOS->GetWindowHeight() / 2.0f, 0.0f);
		if (horizontal)
			cropPoint.sety(pointList.front().y);
		else
			cropPoint.setx(pointList.front().x);

		// Calculate the position of the mid point of the crop line, in percent of the image.
		Vec3 percent = windowToActorPercent(cropPoint);

		if (horizontal)
		{
			if (aboveOrRight)
			{
				// Determine the amount of pixels, in world space, we need to 
				// cut off the image.
				pixelsCut = box.extents.y * percent.y;
				shiftDisplacement.sety(pixelsCut);	
			}
			else
			{
				// Determine the amount of pixels, in world space, we need to 
				// cut off the image.
				pixelsCut = box.extents.y * (1.0f - percent.y);
				shiftDisplacement.sety(-pixelsCut);
			}
			
			// Shrink the dimensions of the bounding box to reflect a crop.
			box.extents.y -= pixelsCut;	
		}
		else
		{
			if (aboveOrRight)
			{
				// Determine the amount of pixels, in world space, we need to 
				// cut off the image.
				pixelsCut = box.extents.x * percent.x;
				shiftDisplacement.setx(-pixelsCut);
			}
			else
			{
				// Determine the amount of pixels, in world space, we need to 
				// cut off the image.
				pixelsCut = box.extents.x * (1.0f - percent.x);
				shiftDisplacement.setx(pixelsCut);
			}
			
			// Shrink the dimensions of the bounding box to reflect a crop.
			box.extents.x -= pixelsCut;
		}

		// When the bounding box's dimensions are changed, they shrink or grow
		// from both sides, about the middle. This means we need to shift the box
		// as well to properly cut off a piece of the bounding box.
		box.center += box.GetRot() * shiftDisplacement;
		
		// Find the points of all the vertices in the bounding box
		box.computePoints(points);
		
		// Only the first four points make up the top plane, so only
		// calculate their client coordinates.
		for (int n = 0; n < 4; n++)
		{
			points[n] = WorldToClient(points[n]);

			// Invert the Y coordinates
			points[n].y = winOS->GetWindowHeight() - points[n].y;
		}

		// Draw an overlay to represent the area to be cropped.
#ifdef DXRENDER
		
		// Due to imprecisions when projecting the box to the screen, add a 1 pixel border around the
		// crop 'selection'
		Vec3 size(abs(points[0].x - points[1].x) + 2.0f, abs(points[1].y - points[2].y) + 2.0f, 0.0f);
		points[1] = points[1] - Vec3(1.0f, 1.0f, 0.0f);

		dxr->device->SetTransform(D3DTS_WORLD, &dxr->identity);
		dxr->device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG2);
		dxr->device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
		
		dxr->renderBillboard(points[1], size, D3DXCOLOR(CROP_COLOR.red() / 255.0f, CROP_COLOR.green() / 255.0f, CROP_COLOR.blue() / 255.0f, CROP_COLOR.alpha() / 255.0f));
		
		dxr->device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		dxr->device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
		
		dxr->renderLine(pointList, QColor(0, 0, 0, 255), CROP_LINE_WIDTH, CROP_LINE_PATTERN_SCALE, CROP_COLOR);
#else
		glPushAttribToken token(GL_ENABLE_BIT);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);

		glColor4f(0.6f, 0.6f, 0.6f, 0.8f);
		glBegin(GL_QUADS);
			glVertex2f(points[1].x, points[1].y);
			glVertex2f(points[0].x, points[0].y);
			glVertex2f(points[3].x, points[3].y);
			glVertex2f(points[2].x, points[2].y);
 		glEnd();

		RenderLine(pointList, true, 15, 6);
#endif
	}
}

void PhotoCropGesture::clearGesture()
{
	_stationaryPath = NULL;
	_cropPath = NULL;
}

bool PhotoCropGesture::isCropPathValid()
{
	return ((NULL != _cropPath) 
		&& (_cropPath->getTotalDisplacementVector().magnitude() >= MINIMUM_CROP_LINE_LENGTH)
		&& (_cropPath->isApproximatelyHorizontal() || _cropPath->isApproximatelyVertical()));
}

Vec3 PhotoCropGesture::windowToActorPercent(Vec3& windowPoint)
{
	tuple<NxActorWrapper*, BumpObject*, Vec3> t = pick((int)windowPoint.x, (int)windowPoint.y);
	BumpObject *pickedObject = t.get<1>();
	if (pickedObject == NULL)
		return Vec3(0.0f);
	Vec3 stabPointActorSpace = t.get<2>();
	Vec3 dims = pickedObject->getDims();

	Vec3 actorPercentage;
	actorPercentage.x = (-stabPointActorSpace.x + dims.x) / (2.0f * dims.x);
	actorPercentage.y = (stabPointActorSpace.y + dims.y) / (2.0f * dims.y);
	actorPercentage.z = 0;
	return actorPercentage;
}
