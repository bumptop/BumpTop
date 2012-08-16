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

#ifndef BT_AUTOMATEDDEMO
#define BT_AUTOMATEDDEMO

// -----------------------------------------------------------------------------

#include "BT_FiniteStateMachine.h"
#include "BT_Replayable.h"
#include "BT_OverlayComponent.h"

// -----------------------------------------------------------------------------

/*
 * An pre-recorded replayable demo.
 */
class AutomatedDemo : public Replayable
{
	Q_DECLARE_TR_FUNCTIONS(AutomatedDemo);

	// the state machine

	// the AutomatedDemo specific states
	FSM_STATE(PrepareScene);
	FSM_STATE(FinalizeScene);

protected:

	OverlayLayout* overlay; // for HUD
	TextOverlay* HUDtext;
	VerticalOverlayLayout* textLayout;

	FiniteStateMachine _demoStates;
	FiniteState * _firstState;
	FiniteState * _finalizeState;
	Stopwatch _timeSinceLastChange;
	unsigned int _durationUntilNextChange;


	class InitPerformanceData : public FiniteState
	{	
		QString _perfDescription;

	public: 
		InitPerformanceData(unsigned int duration, QString perfDescription);
		~InitPerformanceData();
		virtual bool prepareStateChanged();
		virtual void onStateChanged();
		virtual bool finalizeStateChanged();
	};

	class CollectPerformanceData : public FiniteState
	{	
		QString _perfDescription;

	public: 
		CollectPerformanceData(unsigned int duration, QString perfDescription);
		~CollectPerformanceData();
		virtual bool prepareStateChanged();
		virtual void onStateChanged();
		virtual bool finalizeStateChanged();
	};

public:
	AutomatedDemo();
	~AutomatedDemo();

	// Replayable
	virtual void	play();
	virtual void	pause();
	virtual void	stop();
	virtual Replayable::State		update();

	
};

#endif // BT_AUTOMATEDDEMO