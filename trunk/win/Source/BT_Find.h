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

#ifndef BUMPTOP_FIND_H_
#define BUMPTOP_FIND_H_

#include "BT_Common.h"
#include "BT_Singleton.h"
#include "BT_Stopwatch.h"
#include "BT_Animatable.h"

class BumpObject;

class FinderTool : public Animatable
{
private:
	float _alphaAnim;		//The current alpha for the gray screen
	bool _fadeOut;		//The escape key has been pressed, thus the animation should skip to the fading out part
	bool _active;			//Indicates fayt mode is on

	QMap<BumpObject*, bool> _nameVisibleMap;
	boost::function<void(QString)> _onHighlightBumpObjectsWithSubstring;
	
	
	int getObjectsMatchingSubString(QString query, 
		const vector<BumpObject *>& searchObjectsList,
		const vector<BumpObject *>& existingObjectsList,
		QSet<BumpObject *>& newlyMatchedObjectsOut,
		QSet<BumpObject *>& newlyDiscardedObjectsOut,
		QSet<BumpObject *>& matchedObjectsOut,
		bool matchSubString, bool isFuzzy);

	int fuzzyMatch(const QString& s, const QString& t);

	//Show/hide text 
	void showText(BumpObject * obj);
	void hideText(BumpObject * obj);

	void highlightBumpObjectsWithSubstring(QString query, bool matchAnySubString);
	void clearHighlightBumpObjectsWithSubstring();
public:
	FinderTool();
	StopwatchInSeconds elapsedTime; // Time since last key press in search string
	QString searchString;
	
	void setOnHighlightBumpObjectsWithSubstring( boost::function<void(QString)> onHighlightBumpObjectsWithSubstring );
	void renderGrayScreen();
	

	//The two main functions that searches and cancels the search
	void search();
	void cancel();	

	void shutdown();

	bool isActive();

	//Animatable
	virtual void onAnimTick();
	virtual void killAnimation();
	virtual void finishAnimation();
	virtual bool isAnimating(uint animType);
	virtual void onAnimFinished();

};

#define Finder Singleton<FinderTool>::getInstance()


#endif