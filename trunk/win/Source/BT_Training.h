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

#ifndef _BT_TRAINING_
#define _BT_TRAINING_

#include "BT_FiniteStateMachine.h"

class TextOverlay;
class ImageOverlay;
class OverlayLayout;
class FileSystemActor;
class Pile;
class Timer;
class TrainingFiniteState;

class TrainingIntroRunner
{
	// the state machine
	FiniteStateMachine _trainingIntroStates;
	FiniteState * _firstState, * _finishingState;
	FiniteState ** _sectionBeginnings;


public:
	TrainingIntroRunner();
	~TrainingIntroRunner();
	bool _awaitingCurrentStateFinalize;

	void			start();
	void			update();
	void			jumpToSection(int partNumber);
	void			jumpToFinish();
};

class TrainingSharedVariables
{
public:
	TrainingSharedVariables();
	OverlayLayout * _overlay, *_restartPartNButton, *_skipTutorialButton;
	TextOverlay * _overlayText;
	ImageOverlay * _overlayImage;
	FileSystemActor *_actor1, *_actor2, *_actor3;
	FileSystemActor **_actorArray;

	Pile * _pile;
	MouseOverlayEventHandler *_restartPartNMouseEventHandler, *_skipTutorialMouseEventHandler;
};

class TrainingFiniteState : public FiniteState
{
protected:
	TrainingSharedVariables *_s;
	bool _isStateCompleted;
	bool _isCleanedUp;
public:

	TrainingFiniteState(TrainingSharedVariables *shared, unsigned int duration =  -1); 
	void			markStateAsCompleted();
	virtual bool	finalizeStateChanged();
	virtual void	cleanup();
	virtual void	onFinalizeStateChanged();
};

class InitializationState : public TrainingFiniteState
{
	Q_DECLARE_TR_FUNCTIONS(InitializationState)

public:
	InitializationState(TrainingSharedVariables *shared);
	virtual void	onStateChanged();
	virtual bool	finalizeStateChanged();
	virtual void	onFinalizeStateChanged();
	virtual void	cleanup();
};

class StartPart1 : public TrainingFiniteState
{
	Q_DECLARE_TR_FUNCTIONS(StartPart1)

	Timer * _dropTimer;
public:
	StartPart1(TrainingSharedVariables *shared);
	virtual void	onStateChanged();
	virtual void	cleanup();
};

class EnterSlideshowMode : public TrainingFiniteState
{
public:
	EnterSlideshowMode(TrainingSharedVariables *shared);
	virtual void	onStateChanged();
	void			onFileSystemActorLaunch(FileSystemActor* theActor);
	virtual void	cleanup();
};

class SwipeNextInSlideshowMode : public TrainingFiniteState
{
	Q_DECLARE_TR_FUNCTIONS(SwipeNextInSlideshowMode)

public:
	SwipeNextInSlideshowMode(TrainingSharedVariables *shared);
	virtual void	onStateChanged();
	void			onHighlightNextWatchedActor(bool forward);
	virtual void	cleanup();
};

class SwipeNextAgainInSlideshowMode : public TrainingFiniteState
{
	Q_DECLARE_TR_FUNCTIONS(SwipeNextAgainInSlideshowMode)

public:
	SwipeNextAgainInSlideshowMode(TrainingSharedVariables *shared);
	virtual void	onStateChanged();
	void			onHighlightNextWatchedActor(bool forward);
	virtual void	cleanup();
};

class ExitSlideshowMode : public TrainingFiniteState
{
	Q_DECLARE_TR_FUNCTIONS(ExitSlideshowMode)

public:
	ExitSlideshowMode(TrainingSharedVariables *shared);
	virtual void	onStateChanged();
	virtual void	cleanup();
};


// needs to be initialized with a duration of 1000
class StartPart2 : public TrainingFiniteState
{
	Q_DECLARE_TR_FUNCTIONS(StartPart2)

public:
	StartPart2(TrainingSharedVariables *shared);
	virtual void	onStateChanged();
};

class MoveItem : public TrainingFiniteState
{
	Q_DECLARE_TR_FUNCTIONS(MoveItem)

	NxReal _distanceMoved;
	Vec3 _actor3lastPosition;
public:
	MoveItem(TrainingSharedVariables *shared);
	virtual void	onStateChanged();
	virtual bool	finalizeStateChanged();
	virtual void	cleanup();
	virtual void	onFinalizeStateChanged();
};

class BounceItemAgainstWalls : public TrainingFiniteState
{
	Q_DECLARE_TR_FUNCTIONS(BounceItemAgainstWalls)

	Vec3 _actor3velocity;
	Vec3 _actor3acceleration;
public:
	BounceItemAgainstWalls(TrainingSharedVariables *shared);
	virtual void	onStateChanged();
	virtual bool	finalizeStateChanged();
	virtual void	cleanup();
};

class GrowItem : public TrainingFiniteState
{
	Q_DECLARE_TR_FUNCTIONS(GrowItem)

public:
	GrowItem(TrainingSharedVariables *shared);
	virtual void	onStateChanged();
	virtual void	cleanup();
	virtual void	onFinalizeStateChanged();
};

class DeleteItem : public TrainingFiniteState
{
	Q_DECLARE_TR_FUNCTIONS(DeleteItem)

public:
	DeleteItem(TrainingSharedVariables *shared);
	virtual void	onStateChanged();
	void			checkIfFileDeleted();
	virtual void	cleanup();
	virtual void	onFinalizeStateChanged();
};

class CreatePile : public TrainingFiniteState
{
	Q_DECLARE_TR_FUNCTIONS(CreatePile)

public:
	CreatePile(TrainingSharedVariables *shared);
	virtual void	onStateChanged();
	void			onCrossed(Pile* pile);
	virtual void	cleanup();
	virtual void	onFinalizeStateChanged();
};

class GridView : public TrainingFiniteState
{
	Q_DECLARE_TR_FUNCTIONS(GridView)

public:
	GridView(TrainingSharedVariables *shared);
	virtual void	onStateChanged();
	virtual void	cleanup();
};

class CloseGridView : public TrainingFiniteState
{
	Q_DECLARE_TR_FUNCTIONS(CloseGridView)

public:
	CloseGridView(TrainingSharedVariables *shared);
	virtual void	onStateChanged();
	virtual void	cleanup();
};

class StartPart4 : public TrainingFiniteState
{
public:
	StartPart4(TrainingSharedVariables *shared);
	virtual void	onStateChanged();
};

class DragToWalls : public TrainingFiniteState
{
	Q_DECLARE_TR_FUNCTIONS(DragToWalls)

public:
	DragToWalls(TrainingSharedVariables *shared);
	virtual void	onStateChanged();
	virtual void	cleanup();
};

class ZoomToWalls : public TrainingFiniteState
{
	Q_DECLARE_TR_FUNCTIONS(ZoomToWalls)

public:
	ZoomToWalls(TrainingSharedVariables *shared);
	virtual void	onStateChanged();
	virtual void	cleanup();
};

class ZoomToFloor : public TrainingFiniteState
{
	Q_DECLARE_TR_FUNCTIONS(ZoomToFloor)

public:
	ZoomToFloor(TrainingSharedVariables *shared);
	virtual void	onStateChanged();
	virtual void	cleanup();
};

class StartPart5 : public TrainingFiniteState
{
public:
	StartPart5(TrainingSharedVariables *shared);
	virtual void	onStateChanged();
};

class FindAsYouType : public TrainingFiniteState
{
	Q_DECLARE_TR_FUNCTIONS(FindAsYouType)

public:
	FindAsYouType(TrainingSharedVariables *shared);
	virtual void	onStateChanged();
	void			onHighlightBumpObjectsWithSubstring( QString query );
};

class OrganizeByType : public TrainingFiniteState
{
	Q_DECLARE_TR_FUNCTIONS(OrganizeByType)

public:
	OrganizeByType(TrainingSharedVariables *shared);
	virtual void	onStateChanged();
	virtual void	cleanup();
};

class FinishTrainingIntro : public TrainingFiniteState
{
	Q_DECLARE_TR_FUNCTIONS(FinishTrainingIntro)

	Timer * _resetSceneTimer;
public:
	FinishTrainingIntro(TrainingSharedVariables *shared);
	virtual void	onStateChanged();
	virtual void	cleanup();
};

class FinishedTrainingIntro : public TrainingFiniteState
{
	Q_DECLARE_TR_FUNCTIONS(FinishedTrainingIntro)

public:
	FinishedTrainingIntro(TrainingSharedVariables *shared);
	virtual void	onStateChanged();
};

class RemoveSkipTutorialButton : public TrainingFiniteState
{
public:
	RemoveSkipTutorialButton(TrainingSharedVariables *shared);
	virtual void	onStateChanged();
};


class CreateSkipTutorialButton : public TrainingFiniteState, public MouseOverlayEventHandler
{
	Q_DECLARE_TR_FUNCTIONS(CreateSkipTutorialButton)

protected:
	bool		_mouseIsDown;
	QString		_buttonLabel;
public:
	CreateSkipTutorialButton(TrainingSharedVariables *shared);
	virtual void	onStateChanged();

	virtual bool onMouseDown(MouseOverlayEvent& mouseEvent);
	virtual bool onMouseUp(MouseOverlayEvent& mouseEvent);
	virtual bool onMouseMove(MouseOverlayEvent& mouseEvent);
};

class RemoveRestartSectionNButton : public TrainingFiniteState
{
public:
	RemoveRestartSectionNButton(TrainingSharedVariables *shared);
	virtual void	onStateChanged();
};


class CreateRestartSectionNButton : public TrainingFiniteState, public MouseOverlayEventHandler
{
protected:
	bool		_mouseIsDown;
	QString		_buttonLabel;
	int			_partNumber;
public:
	CreateRestartSectionNButton(TrainingSharedVariables *shared);
	virtual void	onStateChanged();

	virtual bool onMouseDown(MouseOverlayEvent& mouseEvent);
	virtual bool onMouseUp(MouseOverlayEvent& mouseEvent);
	virtual bool onMouseMove(MouseOverlayEvent& mouseEvent);
};

class CreateRestartSection1Button : public CreateRestartSectionNButton
{
	Q_DECLARE_TR_FUNCTIONS(CreateRestartSection1Button)
public:
	CreateRestartSection1Button(TrainingSharedVariables *shared);
};

class CreateRestartSection2Button : public CreateRestartSectionNButton
{
	Q_DECLARE_TR_FUNCTIONS(CreateRestartSection2Button)
public:
	CreateRestartSection2Button(TrainingSharedVariables *shared);
};

class CreateRestartSection3Button : public CreateRestartSectionNButton
{
	Q_DECLARE_TR_FUNCTIONS(CreateRestartSection3Button)
public:
	CreateRestartSection3Button(TrainingSharedVariables *shared);
};

class CreateRestartSection4Button : public CreateRestartSectionNButton
{
	Q_DECLARE_TR_FUNCTIONS(CreateRestartSection4Button)
public:
	CreateRestartSection4Button(TrainingSharedVariables *shared);
};

class CreateRestartSection5Button : public CreateRestartSectionNButton
{
	Q_DECLARE_TR_FUNCTIONS(CreateRestartSection5Button)
public:
	CreateRestartSection5Button(TrainingSharedVariables *shared);
};


#endif // _BT_TRAINING_
