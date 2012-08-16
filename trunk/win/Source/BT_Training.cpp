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
#include "BT_DialogManager.h"
#include "BT_FileSystemActor.h"
#include "BT_FileSystemManager.h"
#include "BT_Find.h"
#include "BT_GLTextureManager.h"
#include "BT_LassoMenu.h"
#include "BT_OverlayComponent.h"
#include "BT_Pile.h"
#include "BT_RenderManager.h"
#include "BT_SceneManager.h"
#include "BT_Selection.h"
#include "BT_Timer.h"
#include "BT_Training.h"
#include "BT_Util.h"
#include "BT_WindowsOS.h"
#include "BumpTop.h"


TrainingFiniteState::TrainingFiniteState( TrainingSharedVariables *shared, unsigned int duration )
: FiniteState(duration)
{
	_s = shared;
	_isStateCompleted = false;
}

bool TrainingFiniteState::finalizeStateChanged()
{
	if (_isStateCompleted)
	{
		onFinalizeStateChanged();
		return true;
	}
		
	return false;
}

void TrainingFiniteState::markStateAsCompleted()
{
	_isStateCompleted = true;
}

void TrainingFiniteState::cleanup()
{
	// just a stub, for replacement
	_isStateCompleted = false;

}

void TrainingFiniteState::onFinalizeStateChanged()
{}

TrainingIntroRunner::TrainingIntroRunner()
{
	_sectionBeginnings = new FiniteState*[5];

	// PREPARE SCENE (NOTE, we need some time for the textures to load)
	TrainingSharedVariables *ti = new TrainingSharedVariables();

	FiniteState * tmpState = NULL, * prevState = NULL;
	_firstState = tmpState = new InitializationState(ti);
	_trainingIntroStates.addState(tmpState);


	prevState = tmpState;
	tmpState = new CreateSkipTutorialButton(ti);
	_trainingIntroStates.addState(tmpState);
	_trainingIntroStates.addTransition(prevState, tmpState);

	

	prevState = tmpState;
	tmpState = new CreateRestartSection1Button(ti);
	_trainingIntroStates.addState(tmpState);
	_trainingIntroStates.addTransition(prevState, tmpState);

	_sectionBeginnings[0] = tmpState; // PART 1

	
	// PART 1
	prevState = tmpState;
	tmpState = new StartPart1(ti);
	_trainingIntroStates.addState(tmpState);
	_trainingIntroStates.addTransition(prevState, tmpState);



	prevState = tmpState;
	tmpState = new EnterSlideshowMode(ti);
	_trainingIntroStates.addState(tmpState);
	_trainingIntroStates.addTransition(prevState, tmpState);

	prevState = tmpState;
	tmpState = new SwipeNextInSlideshowMode(ti);
	_trainingIntroStates.addState(tmpState);
	_trainingIntroStates.addTransition(prevState, tmpState);

	prevState = tmpState;
	tmpState = new SwipeNextAgainInSlideshowMode(ti);
	_trainingIntroStates.addState(tmpState);
	_trainingIntroStates.addTransition(prevState, tmpState);

	prevState = tmpState;
	tmpState = new ExitSlideshowMode(ti);
	_trainingIntroStates.addState(tmpState);
	_trainingIntroStates.addTransition(prevState, tmpState);

	// PART 2
	
	prevState = tmpState;
	tmpState = new CreateRestartSection2Button(ti);
	_trainingIntroStates.addState(tmpState);
	_trainingIntroStates.addTransition(prevState, tmpState);

	_sectionBeginnings[1] = tmpState; // PART 2

	prevState = tmpState;
	tmpState = new StartPart2(ti);
	_trainingIntroStates.addState(tmpState);
	_trainingIntroStates.addTransition(prevState, tmpState);


	prevState = tmpState;
	tmpState = new MoveItem(ti);
	_trainingIntroStates.addState(tmpState);
	_trainingIntroStates.addTransition(prevState, tmpState);

	prevState = tmpState;
	tmpState = new BounceItemAgainstWalls(ti);
	_trainingIntroStates.addState(tmpState);
	_trainingIntroStates.addTransition(prevState, tmpState);

	prevState = tmpState;
	tmpState = new GrowItem(ti);
	_trainingIntroStates.addState(tmpState);
	_trainingIntroStates.addTransition(prevState, tmpState);

	prevState = tmpState;
	tmpState = new DeleteItem(ti);
	_trainingIntroStates.addState(tmpState);
	_trainingIntroStates.addTransition(prevState, tmpState);

	
	prevState = tmpState;
	tmpState = new CreateRestartSection3Button(ti);
	_trainingIntroStates.addState(tmpState);
	_trainingIntroStates.addTransition(prevState, tmpState);

	_sectionBeginnings[2] = tmpState; // PART 3

	prevState = tmpState;
	tmpState = new CreatePile(ti);
	_trainingIntroStates.addState(tmpState);
	_trainingIntroStates.addTransition(prevState, tmpState);

	prevState = tmpState;
	tmpState = new GridView(ti);
	_trainingIntroStates.addState(tmpState);
	_trainingIntroStates.addTransition(prevState, tmpState);

	prevState = tmpState;
	tmpState = new CloseGridView(ti);
	_trainingIntroStates.addState(tmpState);
	_trainingIntroStates.addTransition(prevState, tmpState);

	prevState = tmpState;
	tmpState = new CreateRestartSection4Button(ti);
	_trainingIntroStates.addState(tmpState);
	_trainingIntroStates.addTransition(prevState, tmpState);

	_sectionBeginnings[3] = tmpState; // PART 4

	prevState = tmpState;
	tmpState = new StartPart4(ti);
	_trainingIntroStates.addState(tmpState);
	_trainingIntroStates.addTransition(prevState, tmpState);

	prevState = tmpState;
	tmpState = new DragToWalls(ti);
	_trainingIntroStates.addState(tmpState);
	_trainingIntroStates.addTransition(prevState, tmpState);

	prevState = tmpState;
	tmpState = new ZoomToWalls(ti);
	_trainingIntroStates.addState(tmpState);
	_trainingIntroStates.addTransition(prevState, tmpState);

	prevState = tmpState;
	tmpState = new ZoomToFloor(ti);
	_trainingIntroStates.addState(tmpState);
	_trainingIntroStates.addTransition(prevState, tmpState);

	prevState = tmpState;
	tmpState = new CreateRestartSection5Button(ti);
	_trainingIntroStates.addState(tmpState);
	_trainingIntroStates.addTransition(prevState, tmpState);

	_sectionBeginnings[4] = tmpState; // PART 5

	prevState = tmpState;
	tmpState = new StartPart5(ti);
	_trainingIntroStates.addState(tmpState);
	_trainingIntroStates.addTransition(prevState, tmpState);

	prevState = tmpState;
	tmpState = new FindAsYouType(ti);
	_trainingIntroStates.addState(tmpState);
	_trainingIntroStates.addTransition(prevState, tmpState);

	prevState = tmpState;
	tmpState = new OrganizeByType(ti);
	_trainingIntroStates.addState(tmpState);
	_trainingIntroStates.addTransition(prevState, tmpState);

	prevState = tmpState;
	tmpState = new RemoveRestartSectionNButton(ti);
	_trainingIntroStates.addState(tmpState);
	_trainingIntroStates.addTransition(prevState, tmpState);

	_finishingState = tmpState;

	prevState = tmpState;
	tmpState = new RemoveSkipTutorialButton(ti);
	_trainingIntroStates.addState(tmpState);
	_trainingIntroStates.addTransition(prevState, tmpState);

	prevState = tmpState;
	tmpState = new FinishTrainingIntro(ti);
	_trainingIntroStates.addState(tmpState);
	_trainingIntroStates.addTransition(prevState, tmpState);

	prevState = tmpState;
	tmpState = new FinishedTrainingIntro(ti);
	_trainingIntroStates.addState(tmpState);
	_trainingIntroStates.addTransition(prevState, tmpState);

	_awaitingCurrentStateFinalize = true;
}

void TrainingIntroRunner::start()
{
	assert(_firstState);

	// start the demo
	_trainingIntroStates.start(_firstState);
}

void TrainingIntroRunner::update()
{
	_trainingIntroStates.next(_awaitingCurrentStateFinalize);
}

void TrainingIntroRunner::jumpToSection( int partNumber )
{
	assert(1 <= partNumber && partNumber <= 5);
	_trainingIntroStates.jumpToState(_sectionBeginnings[partNumber-1]);
}

void TrainingIntroRunner::jumpToFinish()
{
	_trainingIntroStates.jumpToState(_finishingState);
}

TrainingIntroRunner::~TrainingIntroRunner()
{
	delete _sectionBeginnings;
}


void InitializationState::onStateChanged()
{
	_s->_overlay = new OverlayLayout;
	_s->_overlay->getStyle().setOffset(Vec3(0.5f, 0.8f, 0));

	//_overlayImage = new ImageOverlay(native(winOS->GetTexturesDirectory()) + "\\animated.gif", TEXTURE_ANIMATED_GIF_PATH);//"animated");//
	_s->_overlayText = new TextOverlay(QT_TR_NOOP("Loading, please wait..."));
	_s->_overlayText->getStyle().setCornerRadius(AllCorners, 8.0f);
	_s->_overlayText->getStyle().setBackgroundColor(ColorVal(128, 0, 0, 0));
	_s->_overlayText->getStyle().setPadding(LeftRightEdges, 20);
	_s->_overlayText->getStyle().setPadding(TopBottomEdges, 10);
	_s->_overlayText->getTextBuffer().setMaxBounds(QSize(winOS->GetWindowWidth(), 0));

	// font to use (and fallbacks)
	FontDescription fontDesc(themeManager->getValueAsFontFamilyName("ui.message.font.family",""), 24);
	_s->_overlayText->setFont(fontDesc);
	_s->_overlayText->setAlpha(1.0f);
	_s->_overlay->addItem(_s->_overlayText);
	//_overlay->addItem(_overlayImage);

	scnManager->registerOverlay(_s->_overlay);
}

InitializationState::InitializationState( TrainingSharedVariables *shared )
: TrainingFiniteState(shared)
{}

void InitializationState::cleanup()
{
	TrainingFiniteState::cleanup();
}

bool InitializationState::finalizeStateChanged()
{
	// only move on if we have no more images queued
	return !texMgr->hasTexturesOfState(ImageQueued);
}

void InitializationState::onFinalizeStateChanged()
{
	markStateAsCompleted();
}

void StartPart1::onStateChanged()
{
	Key_DisableSlideShow();
	Key_SetCameraAsAngled();

 
	_s->_overlayText->setAlphaAnim(0.25f, 1.0f, 50);
	_s->_overlayText->setText(QT_TR_NOOP("Welcome to BumpTop!\n\nLet's take a quick tour of BumpTop's features.\nTo start Part 1, double-click one of the photos below."), true);

	_dropTimer = new Timer;
	_dropTimer->setTimerDuration(500);
	_dropTimer->setTimerEventHandler(boost::bind<void>(&StartPart1::markStateAsCompleted, this));
	_dropTimer->start();
}

StartPart1::StartPart1( TrainingSharedVariables *shared )
: TrainingFiniteState(shared)
{}

void StartPart1::cleanup()
{
	SAFE_DELETE(_dropTimer);
	TrainingFiniteState::cleanup();
}
void EnterSlideshowMode::onFileSystemActorLaunch( FileSystemActor* theActor )
{
	_isStateCompleted = true;
}

void EnterSlideshowMode::onStateChanged()
{
	// 1. Create two initial objects
	// 2. Hook up onLaunch handler on the first item

	//scnManager->unregisterOverlay(_prevLayout);
	// Create the new Actor
	//ClearBumpTop();

	SAFE_DELETE(_s->_actor1);
	SAFE_DELETE(_s->_actor2);
	SAFE_DELETE(_s->_actor3);

	QString source_filename1 = native(make_file(winOS->GetTexturesDirectory(), "Slide1-training.JPG"));
	QString filename1 = native(make_file(winOS->GetTrainingDirectory(), "Slide1.JPG"));
	QFileInfo fileInfo1(filename1);

	if (!exists(fileInfo1))
	{
		QFile::copy(source_filename1, filename1);
	}


	_s->_actor1 = new FileSystemActor();
	_s->_actor1->setDims(Vec3(GLOBAL(settings).xDist*6, GLOBAL(settings).zDist*6, GLOBAL(settings).yDist));
	NxF64 l[] = {49.0812, 0.947953, 23.9928};
	Vec3 pos;
	pos.set(l);
	_s->_actor1->setGlobalPosition(pos);
	Mat33 ori;
	NxF64 m[] = {0.994254, 0.107044, 0.000167498, 0.000149778, 0.000179897, -0.999994, -0.107044, 0.994248, 0.000162867};
	ori.setRowMajor(m);
	_s->_actor1->setGlobalOrientation(ori);
	_s->_actor1->setAlpha(1.0f);
	_s->_actor1->setFilePath(filename1);
	_s->_actor1->setLinearVelocity(Vec3(0, -0.75f, 0));
	_s->_actor1->setFreshnessAlphaAnim(1.0f, 150);
	_s->_actor1->setOnLaunchHandler(boost::bind(&EnterSlideshowMode::onFileSystemActorLaunch, this, _1));
	_s->_actor1->setParent(NULL);

	QString source_filename2 = native(make_file(winOS->GetTexturesDirectory(), "Slide2-training.JPG"));
	QString filename2 = native(make_file(winOS->GetTrainingDirectory(), "Slide2.JPG"));
	QFileInfo fileInfo2(filename2);

	if (!exists(fileInfo2))
	{
		QFile::copy(source_filename2, filename2);
	}

	_s->_actor2 =  new FileSystemActor();
	_s->_actor2->setDims(Vec3(GLOBAL(settings).xDist*6, GLOBAL(settings).zDist*6, GLOBAL(settings).yDist));
	NxF64 n[] = {-26.6577, 2.09487, 20.6993};
	pos.set(n);
	_s->_actor2->setGlobalPosition(pos);
	NxF64 o[] = {0.993064, -0.115103, 0.0240037, 0.0235658, -0.00516405, -0.999709, 0.115193, 0.99334, -0.00241574 };
	ori.setRowMajor(o);

	_s->_actor2->setGlobalOrientation(ori);


	_s->_actor2->setAlpha(1.0f);
	_s->_actor2->setFilePath(filename2);
	_s->_actor2->setLinearVelocity(Vec3(0, -0.75f, 0));
	_s->_actor2->setFreshnessAlphaAnim(1.0f, 150);
	_s->_actor2->setOnLaunchHandler(boost::bind(&EnterSlideshowMode::onFileSystemActorLaunch, this, _1));
	_s->_actor2->setParent(NULL);

	// select this new actor	
	sel->clear();
	sel->add(_s->_actor1);

}

EnterSlideshowMode::EnterSlideshowMode( TrainingSharedVariables *shared )
: TrainingFiniteState(shared)
{}

void EnterSlideshowMode::cleanup()
{
	_s->_actor1->setOnLaunchHandler(NULL);
	_s->_actor2->setOnLaunchHandler(NULL);
	TrainingFiniteState::cleanup();
}

void SwipeNextInSlideshowMode::onStateChanged()
{
	// Hook up handler for drag to the right in slideshow mode
	_s->_overlayText->setAlphaAnim(0.25f, 1.0f, 50);
	_s->_overlayText->setText(QT_TR_NOOP("Good job! Now drag quickly to the right OR push the right arrow key."), true);
	cam->setOnHighlightNextWatchedActorHandler(boost::bind(&SwipeNextInSlideshowMode::onHighlightNextWatchedActor, this, _1));
}

void SwipeNextInSlideshowMode::onHighlightNextWatchedActor( bool forward )
{
	_isStateCompleted = true;
}

SwipeNextInSlideshowMode::SwipeNextInSlideshowMode( TrainingSharedVariables *shared )
: TrainingFiniteState(shared)
{}

void SwipeNextInSlideshowMode::cleanup()
{
	cam->setOnHighlightNextWatchedActorHandler(NULL);
	TrainingFiniteState::cleanup();
}

void SwipeNextAgainInSlideshowMode::onStateChanged()
{
	// Hook up handler for drag to the left in slideshow mode
	_s->_overlayText->setAlphaAnim(0.25f, 1.0f, 50);
	_s->_overlayText->setText(QT_TR_NOOP("Sweet! Now try quickly dragging to the left OR push the left arrow key."), true);
	cam->setOnHighlightNextWatchedActorHandler(boost::bind(&SwipeNextAgainInSlideshowMode::onHighlightNextWatchedActor, this, _1));
}

void SwipeNextAgainInSlideshowMode::onHighlightNextWatchedActor( bool forward )
{
	_isStateCompleted = true;
}

SwipeNextAgainInSlideshowMode::SwipeNextAgainInSlideshowMode( TrainingSharedVariables *shared )
: TrainingFiniteState(shared)
{}

void SwipeNextAgainInSlideshowMode::cleanup()
{
	cam->setOnHighlightNextWatchedActorHandler(NULL);
	TrainingFiniteState::cleanup();
}
void ExitSlideshowMode::onStateChanged()
{
	// Hook up handler for zooming out from slideshow mode
	_s->_overlayText->setAlphaAnim(0.25f, 1.0f, 50);
	_s->_overlayText->setText(QT_TR_NOOP("Great! Now click close to zoom back out OR drag down."), true);
	cam->setOnPopWatchActorsHandler(boost::bind(&ExitSlideshowMode::markStateAsCompleted, this));
}

ExitSlideshowMode::ExitSlideshowMode( TrainingSharedVariables *shared )
: TrainingFiniteState(shared)
{}

void ExitSlideshowMode::cleanup()
{
	cam->setOnPopWatchActorsHandler(NULL);
	TrainingFiniteState::cleanup();
}

void StartPart2::onStateChanged()
{
	//ClearBumpTop();
	Key_DisableSlideShow();
	Key_SetCameraAsAngled();

	SAFE_DELETE(_s->_actor1);
	SAFE_DELETE(_s->_actor2);
	SAFE_DELETE(_s->_actor3);

	_s->_overlayText->setAlphaAnim(0.25f, 1.0f, 50);
	_s->_overlayText->setText(QT_TR_NOOP("Part 2/5: Items in BumpTop\n \nYou'll see a small BumpTop logo. Click it, and holding \nyour mouse, move it around."), true);

	QString source_filename = native(make_file(winOS->GetTexturesDirectory(), "bumptoplogo-training.png"));
	QString filename = native(make_file(winOS->GetTrainingDirectory(), "bumptoplogo.png"));
	QFileInfo fileInfo(filename);

	if (!exists(fileInfo))
	{
		QFile::copy(source_filename, filename);
	}
	_s->_actor3 =  new FileSystemActor();
	_s->_actor3->setDims(Vec3(GLOBAL(settings).xDist*3, GLOBAL(settings).zDist*3, GLOBAL(settings).yDist));
	NxF64 l[] = {7.93398, 0.949995, 1.28544};
	Vec3 pos;
	pos.set(l);
	_s->_actor3->setGlobalPosition(pos);
	Mat33 ori;
	NxF64 m[] = {0.993886, 0.110408, 6.61739e-08, 6.46649e-07, -4.95813e-06, -1, -0.110408, 0.993886, -4.99771e-06};
	ori.setRowMajor(m);
	_s->_actor3->setGlobalPosition(pos);
	_s->_actor3->setGlobalOrientation(ori);
	_s->_actor3->setAlpha(1.0f);
	_s->_actor3->setFilePath(filename);
	_s->_actor3->setLinearVelocity(Vec3(0, -0.75f, 0));
	_s->_actor3->setFreshnessAlphaAnim(1.0f, 150);
	_s->_actor3->setParent(NULL);

	_isStateCompleted = true;

	
}

StartPart2::StartPart2( TrainingSharedVariables *shared )
: TrainingFiniteState(shared, 1000)
{}

void MoveItem::onStateChanged()
{
	_actor3lastPosition = _s->_actor3->getGlobalPosition();

}

MoveItem::MoveItem( TrainingSharedVariables *shared )
: TrainingFiniteState(shared), 
_distanceMoved(0.0)
{}


bool MoveItem::finalizeStateChanged()
{
	_distanceMoved += _actor3lastPosition.distance(_s->_actor3->getGlobalPosition());
	_actor3lastPosition = _s->_actor3->getGlobalPosition();
	if (_distanceMoved > 100)
	{
		onFinalizeStateChanged();
		return true;
	}
	return false;
}

void MoveItem::cleanup()
{

	_distanceMoved = 0.0;
	TrainingFiniteState::cleanup();
}

void MoveItem::onFinalizeStateChanged()
{
	printStr(QT_TR_NOOP("Item moved... great!"));
}

BounceItemAgainstWalls::BounceItemAgainstWalls( TrainingSharedVariables *shared )
: TrainingFiniteState(shared),
_actor3velocity(Vec3(0,0,0)),
_actor3acceleration(Vec3(0,0,0))
{}

void BounceItemAgainstWalls::onStateChanged()
{
	_s->_overlayText->setAlphaAnim(0.25f, 1.0f, 50);
	_s->_overlayText->setText(QT_TR_NOOP("In BumpTop, items bump against one another \nand the walls, just like in the real world.\n \nTry throwing the item quickly against \nany of the walls."), true);
}

bool BounceItemAgainstWalls::finalizeStateChanged()
{
	Vec3 newVel = _s->_actor3->getLinearVelocity();
	Vec3 newAcceleration = newVel - _actor3velocity;

	if (sel->getPickedActor() != _s->_actor3 && _actor3acceleration.distanceSquared(newAcceleration) > 50000)
	{
		float dist = _actor3acceleration.distanceSquared(newAcceleration);
		return true;

	}
	_actor3velocity = newVel;
	_actor3acceleration = newAcceleration;
	return false;
}

void BounceItemAgainstWalls::cleanup()
{
	_actor3velocity = Vec3(0,0,0);
	_actor3acceleration = Vec3(0,0,0);
	TrainingFiniteState::cleanup();
}
GrowItem::GrowItem( TrainingSharedVariables *shared )
: TrainingFiniteState(shared)
{}

void GrowItem::onStateChanged()
{
	_s->_overlayText->setAlphaAnim(0.25f, 1.0f, 50);
	_s->_overlayText->setText(QT_TR_NOOP("Now we'll describe how to use the menus in BumpTop.\n \nRight-click the item, and select \"Grow.\""), true);
	_s->_actor3->setOnGrowHandler(boost::bind(&GrowItem::markStateAsCompleted, this));
}


void GrowItem::cleanup()
{
	_s->_actor3->setOnGrowHandler(NULL);
	TrainingFiniteState::cleanup();
}

void GrowItem::onFinalizeStateChanged()
{
	printStr(QT_TR_NOOP("Item grown... good!")); 
}

DeleteItem::DeleteItem( TrainingSharedVariables *shared )
: TrainingFiniteState(shared)
{}

void DeleteItem::onStateChanged()
{
	_s->_overlayText->setAlphaAnim(0.25f, 1.0f, 50);
	_s->_overlayText->setText(QT_TR_NOOP("You can also access the standard Windows right-click menu.\n \nTo delete the item right-click on it, then select \"More...\", and then \"Delete.\""), true);
	_s->_actor3->setOnGrowHandler(NULL);
	setOnInvokeCommandHandler(boost::bind(&DeleteItem::checkIfFileDeleted, this));
}

void DeleteItem::checkIfFileDeleted()
{
	if (_s->_actor3 != NULL)
	{
		QFileInfo fileInfo(_s->_actor3->getFileName());
		if (!exists(fileInfo))
		{
			FadeAndDeleteActor(_s->_actor3);
			_s->_actor3 = NULL;

			setOnInvokeCommandHandler(NULL);
			
			_isStateCompleted = true;
		}
	}
}

void DeleteItem::cleanup()
{
	setOnInvokeCommandHandler(NULL);
	TrainingFiniteState::cleanup();
}

void DeleteItem::onFinalizeStateChanged()
{
	printStr(QT_TR_NOOP("File deleted successfully!"));
}

CreatePile::CreatePile( TrainingSharedVariables *shared )
: TrainingFiniteState(shared)
{}

void CreatePile::onStateChanged()
{
	Key_DisableSlideShow();
	Key_SetCameraAsAngled();

	//ClearBumpTop();
	SAFE_DELETE(_s->_actor1);
	SAFE_DELETE(_s->_actor2);
	SAFE_DELETE(_s->_actor3);

	_s->_overlayText->setAlphaAnim(0.25f, 1.0f, 50);
	_s->_overlayText->setText(QT_TR_NOOP("Part 3/5: Piles\n \nLet's create a pile!\n \nDraw a circle around the items and without letting go\nof the mouse button, drag across the \"Pile\" icon that \nappears in the center\n \nOR Draw a circle around the items, then right-click on\nthem and select \"Create Pile\""), true);

	QString source_filename1 = native(make_file(winOS->GetTexturesDirectory(), "Slide1-training.JPG"));
	QString filename1 = native(make_file(winOS->GetTrainingDirectory(), "Slide1.JPG"));
	QFileInfo fileInfo1(filename1);

	if (!exists(fileInfo1))
	{
		QFile::copy(source_filename1, filename1);
	}

	_s->_actor1 = new FileSystemActor();
	_s->_actor1->setDims(Vec3(GLOBAL(settings).xDist*4, GLOBAL(settings).zDist*4, GLOBAL(settings).yDist));
	NxF64 l[] = {33.88, 0.949995, 26.5467};
	Vec3 pos;
	pos.set(l);
	_s->_actor1->setGlobalPosition(pos);
	Mat33 ori;
	NxF64 m[] = {0.984808, 0.173648, 2.25004e-09, 0, -2.76985e-08, -1, -0.173648, 0.984808, -2.79397e-08};
	ori.setRowMajor(m);
	_s->_actor1->setGlobalOrientation(ori);
	_s->_actor1->setAlpha(1.0f);
	//_actor1->setTextureID("training.slide_1");
	//_actor1->enableThumbnail(true);
	_s->_actor1->setFilePath(filename1);
	_s->_actor1->setLinearVelocity(Vec3(0, -0.75f, 0));
	_s->_actor1->setFreshnessAlphaAnim(1.0f, 150);
	//_i->_actor1->setOnLaunchHandler(boost::bind(&CreatePile::onFileSystemActorLaunch_1_2, this, _1));
	_s->_actor1->setParent(NULL);
	//_actor1->setTextureID("training.slide_1");

	QString source_filename2 = native(make_file(winOS->GetTexturesDirectory(), "Slide2-training.JPG"));
	QString filename2 = native(make_file(winOS->GetTrainingDirectory(), "Slide2.JPG"));
	QFileInfo fileInfo2(filename2);

	if (!exists(fileInfo2))
	{
		QFile::copy(source_filename2, filename2);
	}

	_s->_actor2 =  new FileSystemActor();
	_s->_actor2->setDims(Vec3(GLOBAL(settings).xDist*4, GLOBAL(settings).zDist*4, GLOBAL(settings).yDist));
	NxF64 n[] = {27.3797, 0.949974, -28.4457};
	pos.set(n);
	_s->_actor2->setGlobalPosition(pos);
	NxF64 o[] = {0.984808, -0.173648, -2.25004e-09, 0, -2.76985e-08, -1, 0.173648, 0.984808, -2.79397e-08};
	ori.setRowMajor(o);
	_s->_actor2->setGlobalOrientation(ori);
	_s->_actor2->setAlpha(1.0f);
	_s->_actor2->setFilePath(filename2);
	_s->_actor2->setLinearVelocity(Vec3(0, -0.75f, 0));
	_s->_actor2->setFreshnessAlphaAnim(1.0f, 150);
	_s->_actor2->setParent(NULL);

	QString source_filename3 = native(make_file(winOS->GetTexturesDirectory(), "bumptoplogo-training.png"));
	QString filename3 = native(make_file(winOS->GetTrainingDirectory(), "bumptoplogo.png"));
	QFileInfo fileInfo3(filename3);

	if (!exists(fileInfo3))
	{
		QFile::copy(source_filename3, filename3);
	}


	_s->_actor3 =  new FileSystemActor();
	_s->_actor3->setDims(Vec3(GLOBAL(settings).xDist*2.5, GLOBAL(settings).zDist*2.5, GLOBAL(settings).yDist));
	NxF64 p[] = {-18.0132, 0.950001, 7.95274};
	pos.set(p);
	_s->_actor3->setGlobalPosition(pos);
	NxF64 q[] = {0.999634, 0.0270596, 7.51996e-07, 8.66964e-07, -1.8204e-06, -0.999998, -0.0270596, 0.999631, -1.84232e-06};
	ori.setRowMajor(q);
	_s->_actor3->setGlobalOrientation(ori);
	_s->_actor3->setAlpha(1.0f);
	_s->_actor3->setFilePath(filename3);
	_s->_actor3->setLinearVelocity(Vec3(0, -0.75f, 0));
	_s->_actor3->setFreshnessAlphaAnim(1.0f, 150);
	_s->_actor3->setParent(NULL);

	lassoMenu->getStackIcon().setOnCrossedHandler(boost::bind(&CreatePile::onCrossed, this, _1));
	setMakePileHandler(boost::bind(&CreatePile::onCrossed, this, _1));
}

void CreatePile::onCrossed( Pile* pile )
{
	_s->_pile = pile;
	_isStateCompleted = true;
}

void CreatePile::cleanup()
{
	lassoMenu->getStackIcon().setOnCrossedHandler(NULL);
	setMakePileHandler(NULL);
	TrainingFiniteState::cleanup();
}

void CreatePile::onFinalizeStateChanged()
{
	printStr(QT_TR_NOOP("Great!"));
}
GridView::GridView( TrainingSharedVariables *shared )
: TrainingFiniteState(shared)
{}

void GridView::onStateChanged()
{
	_s->_overlayText->setAlphaAnim(0.25f, 1.0f, 50);
	_s->_overlayText->setText(QT_TR_NOOP("You can also view the piles in different ways. \nAs an example, try double clicking the pile."), true);
	_s->_pile->setOnGridHandler(boost::bind(&GridView::markStateAsCompleted, this));
}

void GridView::cleanup()
{
	_s->_pile->setOnGridHandler(NULL);
	TrainingFiniteState::cleanup();
}

CloseGridView::CloseGridView( TrainingSharedVariables *shared )
:TrainingFiniteState(shared)
{}

void CloseGridView::onStateChanged()
{
	_s->_overlayText->setAlphaAnim(0.25f, 1.0f, 50);
	_s->_overlayText->setText(QT_TR_NOOP("To close the pile, click on the red \"X\" in the upper left corner."), true);
	_s->_pile->setOnGridCloseHandler(boost::bind(&GridView::markStateAsCompleted, this));
}

void CloseGridView::cleanup()
{
	_s->_pile->setOnGridCloseHandler(NULL);
	TrainingFiniteState::cleanup();
}

DragToWalls::DragToWalls( TrainingSharedVariables *shared )
: TrainingFiniteState(shared)
{}

void DragToWalls::onStateChanged()
{
	_s->_overlayText->setAlphaAnim(0.25f, 1.0f, 50);
	_s->_overlayText->setText(QT_TR_NOOP("Part 4/5: Walls.\n \nBumpTop has super-cool walls which you can pin stuff to.\nTry dragging one of the pictures onto the back or side walls."), true);
	setOnPinItemHandler(boost::bind(&DragToWalls::markStateAsCompleted, this));
}


void DragToWalls::cleanup()
{
	setOnPinItemHandler(NULL);
	TrainingFiniteState::cleanup();
}

ZoomToWalls::ZoomToWalls( TrainingSharedVariables *shared )
: TrainingFiniteState(shared)
{}

void ZoomToWalls::onStateChanged()
{
	_s->_overlayText->setAlphaAnim(0.25f, 1.0f, 50);
	_s->_overlayText->setText(QT_TR_NOOP("You can also focus in on the walls for a closer look. \nTry double-clicking one of the walls."), true);
	setOnWallFocusHandler(boost::bind(&ZoomToWalls::markStateAsCompleted, this));
}

void ZoomToWalls::cleanup()
{
	setOnWallFocusHandler(NULL);
	TrainingFiniteState::cleanup();
}


ZoomToFloor::ZoomToFloor( TrainingSharedVariables *shared )
: TrainingFiniteState(shared)
{}

void ZoomToFloor::onStateChanged()
{
	_s->_overlayText->setAlphaAnim(0.25f, 1.0f, 50);
	_s->_overlayText->setText(QT_TR_NOOP("Double-click the floor to return to the default view"), true);
	setOnFloorFocusHandler(boost::bind(&ZoomToWalls::markStateAsCompleted, this));
}

void ZoomToFloor::cleanup()
{
	setOnFloorFocusHandler(NULL);
	TrainingFiniteState::cleanup();
}

FindAsYouType::FindAsYouType( TrainingSharedVariables *shared )
: TrainingFiniteState(shared)
{}

void FindAsYouType::onStateChanged()
{
	_s->_overlayText->setAlphaAnim(0.25f, 1.0f, 50);
	_s->_overlayText->setText(QT_TR_NOOP("Part 5/5: Searching & Automatic organization.\n \nBumpTop makes it super easy to search through your files.\nType 'report' to find all files that have 'report' in their name."), true);
	Finder->setOnHighlightBumpObjectsWithSubstring(boost::bind(&FindAsYouType::onHighlightBumpObjectsWithSubstring, this, _1));	
}

void FindAsYouType::onHighlightBumpObjectsWithSubstring( QString query )
{
	if (query == "rep")
	{
		_isStateCompleted = true;
		Finder->setOnHighlightBumpObjectsWithSubstring(NULL);
	}
}

OrganizeByType::OrganizeByType( TrainingSharedVariables *shared )
: TrainingFiniteState(shared)
{}

void OrganizeByType::onStateChanged()
{
	_s->_overlayText->setAlphaAnim(0.25f, 1.0f, 50);
	_s->_overlayText->setText(QT_TR_NOOP("If you'd like to give some automatic ordering to your files,\nyou can ask BumpTop to organize by type.\n \nRight-click on the desktop, and select \"Pile by Type...\""), true);
	setPileByTypeHandler(boost::bind(&OrganizeByType::markStateAsCompleted, this));
}

void OrganizeByType::cleanup()
{
	setPileByTypeHandler(NULL);
	TrainingFiniteState::cleanup();
}

FinishTrainingIntro::FinishTrainingIntro( TrainingSharedVariables *shared )
: TrainingFiniteState(shared)
{}

void FinishTrainingIntro::onStateChanged()
{
	// Store current camera position
	cam->storeCameraPosition(cam->getEye(), cam->getDir(), cam->getUp());

	scnManager->unregisterOverlay(_s->_overlay);
	SAFE_DELETE(_s->_overlay); // also deletes all children ie. _overlayText



	GLOBAL(settings).completedTutorial = true;
	winOS->SaveSettingsFile();

	printStr(QT_TR_NOOP("Resetting scene... please wait."));
	_resetSceneTimer = new Timer;
	_resetSceneTimer->setTimerDuration(600);
	_resetSceneTimer->setTimerEventHandler(boost::bind<void>(&FinishTrainingIntro::markStateAsCompleted, this));
	_resetSceneTimer->start();

	fsManager->addObject(scnManager);

}

void FinishTrainingIntro::cleanup()
{
	SAFE_DELETE(_resetSceneTimer);
	TrainingFiniteState::cleanup();
}
FinishedTrainingIntro::FinishedTrainingIntro( TrainingSharedVariables *shared )
: TrainingFiniteState(shared)
{}

void FinishedTrainingIntro::onStateChanged()
{	
	if (winOS->IsWindowsVersionGreaterThanOrEqualTo(WindowsVista))
	{
		QString title = QT_TR_NOOP("BumpTop School Complete");
		QString msg = QT_TR_NOOP("That's it, you're all done learning the basics!  We'll now take you to your brand new BumpTop.");
		dlgManager->clearState();
		dlgManager->setCaption(title);
		dlgManager->setPrompt(msg);
		dlgManager->promptDialog(DialogCaptionOnly);
	}
	else
	{
		::MessageBox(NULL, (LPCWSTR) QT_TR_NOOP("That's it, you're all done learning the basics!  We'll now take you to your brand new BumpTop.").utf16(), 
			(LPCWSTR) QT_TR_NOOP("BumpTop School Complete").utf16(), MB_OK);
	}

	Key_ReloadScene();

	if (GLOBAL(isInInfiniteDesktopMode))
	{
		// zoom camera outwards from close in
		cam->skipAnimation();
		cam->setEye(Vec3(0,10,0));

		// re-add the popped actors (from Key_SetCameraAsAngled) if in zui mode
		cam->pushWatchActors(scnManager->getBumpObjects());
	}
	else
	{
		Key_DisableSlideShow();
#ifdef TABLE			
		Key_SetCameraAsTopDown();
#else
		Key_SetCameraAsAngled();
#endif
	}

	printStr(QT_TR_NOOP("Finished training! You're all done."));

	delete scnManager->getTrainingIntroRunner();
	scnManager->setTrainingIntroRunner(NULL);

}

RemoveSkipTutorialButton::RemoveSkipTutorialButton( TrainingSharedVariables *shared )
: TrainingFiniteState(shared)
{}

void RemoveSkipTutorialButton::onStateChanged()
{
	if (_s->_skipTutorialMouseEventHandler)
	{	
		_s->_skipTutorialButton->removeMouseEventHandler(_s->_skipTutorialMouseEventHandler);
		_s->_skipTutorialMouseEventHandler = NULL;
	}

	if (_s->_skipTutorialButton) 
		scnManager->unregisterOverlay(_s->_skipTutorialButton);

	SAFE_DELETE(_s->_skipTutorialButton);

	_isStateCompleted = true;
}

CreateSkipTutorialButton::CreateSkipTutorialButton( TrainingSharedVariables *shared )
: TrainingFiniteState(shared)
{
	_buttonLabel = QT_TR_NOOP("Skip tutorial");
}

void CreateSkipTutorialButton::onStateChanged()
{
	if (_s->_skipTutorialMouseEventHandler)
	{	
		_s->_skipTutorialButton->removeMouseEventHandler(_s->_skipTutorialMouseEventHandler);
		_s->_skipTutorialMouseEventHandler = NULL;
	}

	if (_s->_skipTutorialButton) 
		scnManager->unregisterOverlay(_s->_skipTutorialButton);

	//SAFE_DELETE(_i->_skipTutorialButton);
	// THIS SHOULD NOT BE COMMENTED OUT, FIND OUT
	// WHY ITS CAUSING BUMPTOP TO HAVE HEAP ERRORS

	_s->_skipTutorialButton = new OverlayLayout;
	//_layout->getStyle().setOffset(Vec3(-0.10f, -0.05f, 0));
	_s->_skipTutorialButton->getStyle().setOffset(Vec3(-0.10f, 0.05f, 0));
	TextOverlay *_zoomOutLabel = new TextOverlay(_buttonLabel);
	FontDescription fontDesc(themeManager->getValueAsFontFamilyName("ui.message.font.family",""), 28);
	_zoomOutLabel->setFont(fontDesc);

	HorizontalOverlayLayout *hLayout = new HorizontalOverlayLayout;
	hLayout->getStyle().setBackgroundColor(ColorVal(60, 0, 0, 0));	
	hLayout->getStyle().setPadding(TopBottomEdges, 5.0f);
	hLayout->getStyle().setPadding(LeftRightEdges, 10.0f);
	hLayout->getStyle().setSpacing(2.0f);
	hLayout->addItem(_zoomOutLabel);
	hLayout->addMouseEventHandler(this);
	_s->_skipTutorialMouseEventHandler = this;
	_s->_skipTutorialButton->addItem(hLayout);

	scnManager->registerOverlay(_s->_skipTutorialButton);
	_isStateCompleted = true;
}

bool CreateSkipTutorialButton::onMouseDown( MouseOverlayEvent& mouseEvent )
{
	HorizontalOverlayLayout* hLayout = dynamic_cast<HorizontalOverlayLayout*>(_s->_skipTutorialButton->items()[0]);

	hLayout->getStyle().setBackgroundColor(ColorVal(100, 0, 160, 255));
	rndrManager->invalidateRenderer();

	_mouseIsDown = true;
	return true;
}

bool CreateSkipTutorialButton::onMouseUp( MouseOverlayEvent& mouseEvent )
{
	HorizontalOverlayLayout* hLayout = dynamic_cast<HorizontalOverlayLayout*>(_s->_skipTutorialButton->items()[0]);

	hLayout->getStyle().setBackgroundColor(ColorVal(60, 0, 0, 0));
	rndrManager->invalidateRenderer();

	_mouseIsDown = false;

	TrainingIntroRunner *tir = scnManager->getTrainingIntroRunner();
	tir->jumpToFinish();

	return true;
}

bool CreateSkipTutorialButton::onMouseMove( MouseOverlayEvent& mouseEvent )
{
	return false;
}

RemoveRestartSectionNButton::RemoveRestartSectionNButton( TrainingSharedVariables *shared )
: TrainingFiniteState(shared)
{}

void RemoveRestartSectionNButton::onStateChanged()
{
	if (_s->_restartPartNMouseEventHandler)
	{	
		_s->_restartPartNButton->removeMouseEventHandler(_s->_restartPartNMouseEventHandler);
		_s->_restartPartNMouseEventHandler = NULL;
	}

	if (_s->_restartPartNButton) 
		scnManager->unregisterOverlay(_s->_restartPartNButton);
	
	SAFE_DELETE(_s->_restartPartNButton);

	_isStateCompleted = true;
}

CreateRestartSectionNButton::CreateRestartSectionNButton( TrainingSharedVariables *shared )
: TrainingFiniteState(shared)
{}

void CreateRestartSectionNButton::onStateChanged()
{
	if (_s->_restartPartNMouseEventHandler)
	{	
		_s->_restartPartNButton->removeMouseEventHandler(_s->_restartPartNMouseEventHandler);
		_s->_restartPartNMouseEventHandler = NULL;
	}

	if (_s->_restartPartNButton) 
		scnManager->unregisterOverlay(_s->_restartPartNButton);
	
	//SAFE_DELETE(_i->_restartPartNButton);
	// THIS SHOULD NOT BE COMMENTED OUT, FIND OUT
	// WHY ITS CAUSING BUMPTOP TO HAVE HEAP ERRORS

	_s->_restartPartNButton = new OverlayLayout;
	//_layout->getStyle().setOffset(Vec3(-0.10f, -0.05f, 0));
	_s->_restartPartNButton->getStyle().setOffset(Vec3(0.10f, 0.05f, 0));
	TextOverlay *_zoomOutLabel = new TextOverlay(_buttonLabel);
	FontDescription fontDesc(themeManager->getValueAsFontFamilyName("ui.message.font.family",""), 28);
	_zoomOutLabel->setFont(fontDesc);

	HorizontalOverlayLayout *hLayout = new HorizontalOverlayLayout;
	hLayout->getStyle().setBackgroundColor(ColorVal(60, 0, 0, 0));
	hLayout->getStyle().setPadding(TopBottomEdges, 5.0f);
	hLayout->getStyle().setPadding(LeftRightEdges, 10.0f);
	hLayout->getStyle().setSpacing(2.0f);
	hLayout->addItem(_zoomOutLabel);
	hLayout->addMouseEventHandler(this);
	_s->_restartPartNMouseEventHandler = this;
	_s->_restartPartNButton->addItem(hLayout);

	scnManager->registerOverlay(_s->_restartPartNButton);
	_isStateCompleted = true;
}

bool CreateRestartSectionNButton::onMouseDown( MouseOverlayEvent& mouseEvent )
{
	HorizontalOverlayLayout* hLayout = dynamic_cast<HorizontalOverlayLayout*>(_s->_restartPartNButton->items()[0]);

	hLayout->getStyle().setBackgroundColor(ColorVal(100, 0, 160, 255));
	rndrManager->invalidateRenderer();

	_mouseIsDown = true;
	return true;
}

bool CreateRestartSectionNButton::onMouseUp( MouseOverlayEvent& mouseEvent )
{
	HorizontalOverlayLayout* hLayout = dynamic_cast<HorizontalOverlayLayout*>(_s->_restartPartNButton->items()[0]);

	hLayout->getStyle().setBackgroundColor(ColorVal(60, 0, 0, 0));
	rndrManager->invalidateRenderer();

	_mouseIsDown = false;

	TrainingIntroRunner *tir = scnManager->getTrainingIntroRunner();
	tir->jumpToSection(_partNumber);

	return true;
}

bool CreateRestartSectionNButton::onMouseMove( MouseOverlayEvent& mouseEvent )
{
	return false;
}

CreateRestartSection1Button::CreateRestartSection1Button( TrainingSharedVariables *shared )
: CreateRestartSectionNButton(shared)
{
	_buttonLabel = QT_TR_NOOP("Restart Part 1");
	_partNumber = 1;
}

CreateRestartSection2Button::CreateRestartSection2Button( TrainingSharedVariables *shared )
: CreateRestartSectionNButton(shared)
{
	_buttonLabel = QT_TR_NOOP("Restart Part 2");
	_partNumber = 2;
}

CreateRestartSection3Button::CreateRestartSection3Button( TrainingSharedVariables *shared )
: CreateRestartSectionNButton(shared)
{
	_buttonLabel = QT_TR_NOOP("Restart Part 3");
	_partNumber = 3;
}

CreateRestartSection4Button::CreateRestartSection4Button( TrainingSharedVariables *shared )
: CreateRestartSectionNButton(shared)
{
	_buttonLabel = QT_TR_NOOP("Restart Part 4");
	_partNumber = 4;
}

CreateRestartSection5Button::CreateRestartSection5Button( TrainingSharedVariables *shared )
: CreateRestartSectionNButton(shared)
{
	_buttonLabel = QT_TR_NOOP("Restart Part 5");
	_partNumber = 5;
}

TrainingSharedVariables::TrainingSharedVariables()
{
	_overlay = NULL;
	_restartPartNButton = NULL;
	_skipTutorialButton = NULL;
	_overlayText = NULL;
	_overlayImage = NULL;
	_actor1 = NULL;
	_actor2 = NULL;
	_actor3 = NULL;
	_pile = NULL;
	_restartPartNMouseEventHandler = NULL;
	_skipTutorialMouseEventHandler = NULL;
	_actorArray = NULL;
}


StartPart4::StartPart4( TrainingSharedVariables *shared )
: TrainingFiniteState(shared)
{}

void StartPart4::onStateChanged()
{
	Key_DisableSlideShow();
	Key_SetCameraAsAngled();

	SAFE_DELETE(_s->_actor1);
	SAFE_DELETE(_s->_actor2);
	SAFE_DELETE(_s->_actor3);

	QString source_filename = native(make_file(winOS->GetTexturesDirectory(), "DSCF0534-2-training.jpg"));
	QString filename = native(make_file(winOS->GetTrainingDirectory(), "DSCF0534-2.jpg"));
	QFileInfo fileInfo(filename);

	if (!exists(fileInfo))
	{
		QFile::copy(source_filename, filename);
	}
	_s->_actor1 =  new FileSystemActor();
	_s->_actor1->setDims(Vec3(GLOBAL(settings).xDist*4, GLOBAL(settings).zDist*4, GLOBAL(settings).yDist));
	NxF64 l[] = {-25.6967, 0.949994, -6.17019};
	Vec3 pos;
	pos.set(l);
	_s->_actor1->setGlobalPosition(pos);
	Mat33 ori;
	NxF64 m[] = {0.993857, -0.110676, 9.84832e-07, 6.7695e-07, -7.48708e-07, -0.999998, 0.110676, 0.993854, -6.56582e-07};
	ori.setRowMajor(m);
	_s->_actor1->setGlobalOrientation(ori);
	_s->_actor1->setAlpha(1.0f);
	_s->_actor1->setFilePath(filename);
	_s->_actor1->setLinearVelocity(Vec3(0, -0.75f, 0));
	_s->_actor1->setFreshnessAlphaAnim(1.0f, 150);
	_s->_actor1->setParent(NULL);



	QString source_filename2 = native(make_file(winOS->GetTexturesDirectory(), "DSC_0384-training.JPG"));
	QString filename2 = native(make_file(winOS->GetTrainingDirectory(), "DSC_0384.jpg"));
	QFileInfo fileInfo2(filename2);

	if (!exists(fileInfo2))
	{
		QFile::copy(source_filename2, filename2);
	}

	_s->_actor2 =  new FileSystemActor();
	_s->_actor2->setDims(Vec3(GLOBAL(settings).xDist*4, GLOBAL(settings).zDist*4, GLOBAL(settings).yDist));
	NxF64 n[] = {59.2983, 0.949993, -9.36131};
	pos.set(n);
	_s->_actor2->setGlobalPosition(pos);
	NxF64 o[] = {0.987016, 0.160622, -8.96546e-08, 4.84802e-07, -8.16147e-07, -0.999997, -0.160622, 0.987013, -8.48435e-07};
	ori.setRowMajor(o);
	_s->_actor2->setGlobalOrientation(ori);
	_s->_actor2->setAlpha(1.0f);
	_s->_actor2->setFilePath(filename2);
	_s->_actor2->setLinearVelocity(Vec3(0, -0.75f, 0));
	_s->_actor2->setFreshnessAlphaAnim(1.0f, 150);
	_s->_actor2->setParent(NULL);

	

	//CreateDefaultScenePhotoFrames();

	_isStateCompleted = true;
}


StartPart5::StartPart5( TrainingSharedVariables *shared )
: TrainingFiniteState(shared)
{}

void StartPart5::onStateChanged()
{
	
	Key_DisableSlideShow();
	Key_SetCameraAsAngled();
	

	SAFE_DELETE(_s->_actor1);
	SAFE_DELETE(_s->_actor2);
	SAFE_DELETE(_s->_actor3);

	if (_s->_actorArray != NULL)
	{
		for (int i = 0; i < 31; i++)
			SAFE_DELETE(_s->_actorArray[i]);
		SAFE_DELETE(_s->_actorArray);
	}

	vector<QString> texturesList;

	QString textureBaseNames[] = {"winImage4.png", "winAVI.avi", "winNotes.txt", "winPDF2.pdf", "winWord.doc"};
	

	for_each(QString textureBaseName, textureBaseNames)
	{
		for (int i =0; i < 5; i++)
		{
			QString source_filename = native(make_file(winOS->GetTexturesDirectory(), textureBaseName));
			QString filename = native(make_file(winOS->GetTrainingDirectory(), QString("%1-%2").arg(i).arg(textureBaseName)));
			QFileInfo fileInfo(filename);

			if (!exists(fileInfo))
				QFile::copy(source_filename, filename);

			texturesList.push_back(filename);
		}
	}

	for (int i =0; i < 5; i++)
	{
		QString source_filename = native(make_file(winOS->GetTexturesDirectory(), "winFolder"));
		QString filename = native(make_file(winOS->GetTrainingDirectory(), QString("%1-%2").arg(i).arg("winFolder")));
		QFileInfo fileInfo(filename);

		if (!exists(fileInfo))
			create_directory(filename);

		texturesList.push_back(filename);
	}


	QString source_filename = native(make_file(winOS->GetTexturesDirectory(), "winPDF2.pdf"));
	QString filename = native(make_file(winOS->GetTrainingDirectory(), QString("colbert report.pdf").arg(30)));
	QFileInfo fileInfo(filename);

	if (!exists(fileInfo))
		QFile::copy(source_filename, filename);

	texturesList.push_back(filename);

	random_shuffle(texturesList.begin(), texturesList.end());

	NxF64 positions[31][3] = {6.403070, 2.087294, 24.360008,
						-12.611953, 0.986401, 23.537888,
						-23.669910, 2.450729, 26.051123,
						-43.876873, 2.942572, 9.432054,
						-47.709332, 0.991832, 1.027789,
						-22.723902, 0.990034, -33.222054,
						-46.639923, 0.990027, -24.039465,
						-95.339790, 0.990027, -78.952522,
						-125.189911, 0.986345, -76.603279,
						42.743145, 0.990026, -37.434391,
						110.516960, 0.990000, -38.500946,
						119.872536, 0.987529, -10.353114,
						117.982460, 2.240962, 4.089403,
						91.835747, 0.990026, -18.863251,
						129.318558, 0.990005, -28.183353,
						-19.553080, 0.990025, -10.787930,
						-122.548492, 0.990021, -48.276020,
						121.045074, 0.989957, -50.912422,
						116.877190, 0.990024, -71.625160,
						117.165497, 0.990029, -93.218452,
						-129.354202, 0.990021, -99.430740,
						-126.225151, 0.990031, 100.790894,
						-110.597382, 0.990025, 66.137840,
						-104.437096, 0.990023, -99.265251,
						-107.430962, 0.990022, 88.984360,
						21.753952, 0.990027, -35.933743,
						-81.324348, 0.990023, -97.788429,
						21.167747, 0.990021, 11.894445,
						-4.446814, 0.990020, 1.029145,
						-77.998085, 0.990027, 94.543594,
						-115.894440, 2.826268, -70.358871};

	_s->_actorArray = new FileSystemActor*[31];

	for (int i =0; i < 31; i++)
	{
		QString texture = texturesList[i];

		_s->_actorArray[i] =  new FileSystemActor();
		_s->_actorArray[i]->setDimsToDefault();
		Vec3 pos;
		positions[i][1] *= 200;
		pos.set(positions[i]);
		_s->_actorArray[i]->setGlobalPosition(pos);
		_s->_actorArray[i]->setAlpha(1.0f);
		_s->_actorArray[i]->setFilePath(texture);
		_s->_actorArray[i]->setLinearVelocity(Vec3(0, -0.75f, 0));
		_s->_actorArray[i]->setFreshnessAlphaAnim(1.0f, 150);
		_s->_actorArray[i]->setParent(NULL);

		// randomly choose a texture
		// remove it from the list
		// create a filesystemactor
	}

	// then iterate through all the positions, and randomly choose one from the list of textures
	_isStateCompleted = true;

}
