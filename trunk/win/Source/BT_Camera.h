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

#pragma once

#ifndef _BT_CAMERA_
#define _BT_CAMERA_

// -----------------------------------------------------------------------------

#include "BT_Animatable.h"
#include "BT_AnimationManager.h"
#include "BT_WatchedObjects.h"
#include "BT_WebActor.h"
#include "BT_Singleton.h"
#include "BT_GLTextureManager.h"
#include "BT_OverlayEvent.h"
#include "TwitterClient.h"
#include "FacebookClient.h"

class NxActorWrapper;
class BumpObject;
class OverlayLayout;
class HorizontalOverlayLayout;
class VerticalOverlayLayout;
class TextOverlay;
class ImageOverlay;
class PbCamera;


// -----------------------------------------------------------------------------

#define MOVING_FLAG_TIMEOUT 100
#define CAMERA_FOVY 60.0f

// -----------------------------------------------------------------------------

class Actor;

// -----------------------------------------------------------------------------

class RestoreOriginalPhotoControl : public MouseOverlayEventHandler
{
	Q_DECLARE_TR_FUNCTIONS(RestoreOriginalPhotoControl)

	OverlayLayout * _layout;
	HorizontalOverlayLayout * _hLayout;
	TextOverlay * _label;
	bool _enabled;

	void init();

	// animation
	void fadeIn();
	void fadeOut();

public:
	RestoreOriginalPhotoControl();
	virtual ~RestoreOriginalPhotoControl();

	void show();
	void hide(); 

	// event handlers
	virtual bool onMouseDown(MouseOverlayEvent& mouseEvent);
	virtual bool onMouseUp(MouseOverlayEvent& mouseEvent);
	virtual bool onMouseMove(MouseOverlayEvent& mouseEvent);
};

class SlideShowControls : public MouseOverlayEventHandler
{
	Q_DECLARE_TR_FUNCTIONS(SlideShowControls)

	OverlayLayout * _slideshowMenuContainer;
	HorizontalOverlayLayout * _slideshowMenuHLayout;

	//Twitter menu element
	OverlayLayout * _twitterContainer;
		VerticalOverlayLayout * _twitterLayout;
		ImageOverlay * _twitterImage;
		TextOverlay * _twitterLabel;

	//Facebook menu element
	OverlayLayout * _facebookContainer;
		VerticalOverlayLayout * _facebookLayout;
		ImageOverlay * _facebookImage;
		TextOverlay * _facebookLabel;

	//Email menu element
	OverlayLayout * _emailContainer;
		VerticalOverlayLayout * _emailLayout;
		ImageOverlay * _emailImage;
		TextOverlay * _emailLabel;

	//Divider
	ImageOverlay * _dividerImage1;
	ImageOverlay * _dividerImage2;
	ImageOverlay * _dividerImage3;
	ImageOverlay * _dividerImage4;
	ImageOverlay * _dividerImage5;
	ImageOverlay * _dividerImage6;
	ImageOverlay * _dividerImage7;

	//Printer menu element
	OverlayLayout * _printerContainer;
		VerticalOverlayLayout * _printerLayout;
		ImageOverlay * _printerImage;
		TextOverlay * _printerLabel;

	//Edit menu element
	OverlayLayout * _editContainer;
		VerticalOverlayLayout * _editLayout;
		ImageOverlay * _editImage;
		TextOverlay * _editLabel;

	//Close menu element
	OverlayLayout * _closeContainer;
		VerticalOverlayLayout * _closeLayout;
		ImageOverlay * _closeImage;
		TextOverlay * _closeLabel;

	//Close menu element
	OverlayLayout * _nextContainer;
		VerticalOverlayLayout * _nextLayout;
		ImageOverlay * _nextImage;

	//Close menu element
	OverlayLayout * _prevContainer;
		VerticalOverlayLayout * _prevLayout;
		ImageOverlay * _prevImage;


	TwitterClient * twitterClient;
	TwitpicClient * twitpicClient;
	FacebookClient * facebookClient;

	bool _enabled;

	// animation
	void fadeIn();
	void fadeOut();

public:
	enum actionFlag {
		TWITTER,
		FACEBOOK,
		EMAIL,
		PRINTER,
		NEXT,
		PREVIOUS,
		CLOSE,
		EDIT,
		GOFACEBOOK,
		NOPRESS
	};

	WebActor* _facebookActor;

	SlideShowControls();
	virtual ~SlideShowControls();

	actionFlag pressed;

	void twitterAction();
	void facebookAction();
	void printerAction();
	void closeAction();
	void emailAction();
	void nextAction();
	void prevAction();
	void editAction();
	void goFacebookAction();
	int getHeight();
	WebActor * getFacebookActor(bool createIfNotExists);

	//
	void init();
	void disable(); 

	// event handlers
	virtual bool onMouseDown(MouseOverlayEvent& mouseEvent);
	virtual bool onMouseUp(MouseOverlayEvent& mouseEvent);
	virtual bool onMouseMove(MouseOverlayEvent& mouseEvent);
};



// -----------------------------------------------------------------------------

class ZoomOutControl : public MouseOverlayEventHandler
{
	Q_DECLARE_TR_FUNCTIONS(ZoomOutControl)

	OverlayLayout * _layout;
	HorizontalOverlayLayout * _hLayout;
	TextOverlay * _zoomOutLabel;
	bool _enabled;
	QString _defaultText;

	void init();
	
	// animation
	void fadeIn();
	void fadeOut();

public:
	ZoomOutControl();
	virtual ~ZoomOutControl();

	void show();
	void hide(); 

	void setText(QString text, Vec3 offset); // If text is empty, assume text is "Reset Camera" and offset accordingly
    
	// event handlers
	virtual bool onMouseDown(MouseOverlayEvent& mouseEvent);
	virtual bool onMouseUp(MouseOverlayEvent& mouseEvent);
	virtual bool onMouseMove(MouseOverlayEvent& mouseEvent);

	bool isEnabled();
};

// -----------------------------------------------------------------------------

class CornerInfoControl : public MouseOverlayEventHandler
{
	Q_DECLARE_TR_FUNCTIONS(CornerInfoControl)

	OverlayLayout * _layout;
	HorizontalOverlayLayout * _hLayout;
	TextOverlay * _zoomOutLabel;
	bool _enabled;

	// animation
	void fadeIn();
	void fadeOut();

public:
	CornerInfoControl();
	virtual ~CornerInfoControl();

	void init();
	void disable(); 
	bool isEnabled();
	void setMessageText(QString text);

	// event handlers
	virtual bool onMouseDown(MouseOverlayEvent& mouseEvent);
	virtual bool onMouseUp(MouseOverlayEvent& mouseEvent);
	virtual bool onMouseMove(MouseOverlayEvent& mouseEvent);
};

// -----------------------------------------------------------------------------

class Camera : public Animatable
{
public:
	enum PredefinedCameraViews 
	{
		DefaultView,
		TopDownView,
		BottomRightCornerView,
		WallView,
		UserDefinedView
	};

private:
	Q_DECLARE_TR_FUNCTIONS(Camera)

	static const float SAME_POSITION_THRESHOLD;

	Vec3 orig;				// current values
	Vec3 eye;
	Vec3 dir;
	Vec3 up;
	Vec3 right;
	deque<Vec3> pathEye;	// animation paths
	deque<Vec3> pathDir;
	deque<Vec3> pathUp;
	PredefinedCameraViews _currentCameraView;

	QList<BumpObject*> zoomedObjects;	// objects that we are zoomed into
	bool isSlideShow;

	void (* _animationFinishCallback)(void *);	// callback for when the camera finishes it's animation
	void * _animationFinishData;				// custom data for the finished animation callback
	
	// Camera settings loaded from Scene file
	Vec3 savedEye;
	Vec3 savedDir;
	Vec3 savedUp;
	bool cameraIsFreeForm;

	// tutorial callbacks
	boost::function<void(bool)> _onHighlightNextWatchedActorHandler;
	boost::function<void()> _onPopWatchActorsHandler;

	// keep track of previously visible nameables
	set<BumpObject *> _prevVisibleNameables;

	// visible bounds of the camera
	Bounds visibleBounds;

	// set of actors to watch
	stack<WatchedObjects> watchedActors;

	// Singleton
	friend class Singleton<Camera>;
	Camera();

	// Private Actions
	Vec3 getEyePositionToFitBounds(const Bounds& bounds);
	void updateWatchedActorVisibleBounds();

	// Camera overlay controls
	SlideShowControls slideShowControls;
	ZoomOutControl zoomOutControl;
	CornerInfoControl cornerInfoControl;
	RestoreOriginalPhotoControl restoreOriginalPhotoControl;

#ifndef DXRENDER
	// open gl cached values
	GLdouble * _glProjMatrix;
	GLdouble * _glMVMatrix;
	GLint * _glViewport;
	bool _isglDataDirty;
#endif

public:
	~Camera();
	
	Vec3 getEye();
	Vec3 getUp();
	Vec3 getDir();
	Vec3 getOrigin();
	Vec3 getDefaultEye();
	PredefinedCameraViews getCurrentCameraView();
	void setCurrentCameraView(PredefinedCameraViews view, bool makePersistent = false);
	void setOrientation(Vec3 eye, Vec3 up, Vec3 dir);
	void setDir(Vec3 newDir);
	void setUp(Vec3 newUp);
	void setEye(Vec3 newEye);
	void setOrigin(Vec3 newOrig);

	bool inSlideshow();
	void setSlideshow(bool active);

	Vec3 zoomToTopDownView();
	Vec3 zoomToAngledView();
	Vec3 zoomToBottomRightCornerView();
	// void zoomToWall(int i);

	// Camera settings loaded from Scene file
	Vec3 getSavedEye();
	Vec3 getSavedDir();
	Vec3 getSavedUp();
	void storeCameraPosition(Vec3 newEye, Vec3 newDir, Vec3 newUp);
	bool isCameraFreeForm();
	void setIsCameraFreeForm(bool isCameraFreeForm, bool showMessage = true);
	void loadCameraFromPreset(QString cameraPreset);

	bool adjustPointInDesktop(Vec3& origin, Vec3& destination, NxPlane& plane);
	void setAnimationFinishCallback(void (* animationFinishCallback)(void *), void * animationFinishData);
	void animateToImpl(Vec3 eyeDest, Vec3 dirDest, Vec3 upDest, int timeStep, bool easeIn, bool boundsCheck, bool projectTail);
	void animateTo(Vec3 eyeDest, Vec3 dirDest, Vec3 upDest = Vec3(0, 1, 0), int timeStep = 30, bool easeIn = true);
	void animateToWithBounds(Vec3 eyeDest, Vec3 dirDest, Vec3 upDest = Vec3(0, 1, 0), int timeStep = 30, bool easeIn = true);
	void animateToWithSliding(Vec3 eyeDest, Vec3 dirDest, Vec3 upDest = Vec3(0, 1, 0), int timeStep = 30, bool easeIn = true);
	void animateQuadratic (Vec3 eyeDest, Vec3 topDest, Vec3 dirDest, Vec3 upDest = Vec3(0, 1, 0), int timeStep = 30);
	void animationFinishCallback();
	void scrollToPoint(Vec3 loc);
	void scrollToOrigin();
	void pointCameraTo(Vec3 pt);
	void pointCameraTo(Vec3 pt, bool executeWindowToWorld);
	void customAnimation(deque<Vec3> &eyeDest, deque<Vec3> &dirDest, deque<Vec3> &upDest);
	void update(float elapsed);
	void restorePreviousVisibleNameables(bool skipAnimation, bool temporary = false);
	void storePreviousVisibleNameables(bool skipAnimation, bool temporary = false);
	
	QList<BumpObject*>& getZoomedObjects();
	void zoomToIncludeObjects(vector<BumpObject*> objs);
	
	// Call with a screenSize to make camera zoom to object to match the screenSize
	void zoomToOrientedActorBoundsAttributes(BumpObject * actor, Vec3& eye, Vec3& up, Vec3& dir, float buffer = 0.0f, const Vec3 & screenSize = Vec3(0,0,0));
	Bounds getActorsBounds(const vector<BumpObject *>& objects, bool flatten=false);
	int cameraIsFacingWall();
	void lookAtWall(NxActorWrapper* wall, Vec3& positionOut, Vec3& directionOut);

	void helperSetThumbnailDetail(BumpObject *actor, GLTextureDetail textureDetail, GLTextureLoadPriority priority);

	// watched actors
	void watchActor(BumpObject * object);
	void unwatchActor(BumpObject * object);

	void pushWatchActors(const vector<BumpObject *>& actors, bool groupPileItems = false);
	void popWatchActors();
	void popAllWatchActors();

	bool isWatchingActors() const;
	bool isTrackingWatchedActors() const;
	void setTrackingWatchedActors(bool state);
	bool hasWatchedActorHighlighted() const;
	bool isWatchedActorHighlighted(BumpObject * object) const;

	void rehighlightWatchedActor();
	void highlightWatchedActor(BumpObject * actor, BumpObject * nextActor=NULL);
	void highlightNextWatchedActor(bool forward);
	BumpObject * getHighlightedWatchedActor() const;

	// called from mouse handlers
	ZoomOutControl* getZoomOutControl();
	RestoreOriginalPhotoControl* getRestoreOriginalPhotoControl();

	// animatable implementation
	virtual void onAnimFinished();
	virtual void onAnimTick();
	virtual void killAnimation();
	virtual void finishAnimation();
	virtual void revertAnimation();
	virtual void skipAnimation();
	virtual bool isAnimating(uint animType = SizeAnim | AlphaAnim | PoseAnim);

	void setOnHighlightNextWatchedActorHandler(boost::function<void(bool)> onHighlightNextWatchedActorHandler);
	void setOnPopWatchActorsHandler(boost::function<void()> onPopWatchActorsHandler);

	// Corner Message
	CornerInfoControl * getCornerInfoControl();

#ifndef DXRENDER
	// open gl cached values
	GLdouble * glModelViewMatrix() const;
	GLdouble * glProjectionMatrix() const;
	GLint * glViewport() const;
	void markglDataDirty();
	void readOGLMatrices();
#endif

	// legacy deserialization
	void unserialize(unsigned char *buf, uint &bufSz, int versionNumber);

	// protocol buffers serialization
	bool serializeToPb(PbCamera * pbCamera);
	bool deserializeFromPb(const PbCamera * pbCamera);
};

// -----------------------------------------------------------------------------

#define cam Singleton<Camera>::getInstance()

// -----------------------------------------------------------------------------

#else
	class Camera;
#endif
