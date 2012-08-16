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

#ifndef BT_AUTOMATEDTESTS
#define BT_AUTOMATEDTESTS

// -----------------------------------------------------------------------------

#include "BT_FiniteStateMachine.h"
#include "BT_Replayable.h"
#include "BT_OverlayComponent.h"
#include "BT_AutomatedDemo.h"

// -----------------------------------------------------------------------------

/*
* A series of tests
*/
class AutomatedTests : public AutomatedDemo
{
	// the state machine
// 	FiniteStateMachine _demoStates;
// 	FiniteState * _firstState;
// 	FiniteState * _finalizeState;
// 	Stopwatch _timeSinceLastChange;
// 	unsigned int _durationUntilNextChange;
	bool success;


// the AutomatedDemo specific states
  	FSM_STATE(PrepareScene);
  	FSM_STATE(FinalizeScene);

protected:

// 	OverlayLayout* overlay; // for HUD
// 	TextOverlay* HUDtext;
// 	VerticalOverlayLayout* textLayout;


public:
	AutomatedTests();

	// Replayable
	virtual void	play();
	//virtual void	pause();
	virtual void	stop();
	//virtual void	update();


};

#endif // BT_AUTOMATEDTESTS