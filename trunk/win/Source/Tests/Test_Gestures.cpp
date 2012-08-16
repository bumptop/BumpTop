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
#include "BT_GestureManager.h"

class Test_Gestures : public testing::Test {
protected:
	GestureManager _gestureManager;

	Test_Gestures() : _gestureManager()
	{}
};

// GestureManager tests

TEST_F(Test_Gestures, test_GestureManager) {
	// Very basic tests for the GestureManager

	EXPECT_FALSE(_gestureManager.isGestureActive());
}

// GestureContext tests

TEST_F(Test_Gestures, test_GestureContext) {
	// Very basic tests for the GestureContext

	GestureContext *gestureContext = _gestureManager.getGestureContext();
	EXPECT_TRUE(gestureContext->isEmpty());
	EXPECT_EQ(gestureContext->getNumActiveTouchPoints(), 0);
}
