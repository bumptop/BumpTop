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
#include "BT_Find.h"
#include "BT_BumpObject.h"
#include "BT_Actor.h"
#include "BT_StatsManager.h"
#include "BT_Selection.h"
#include "BT_Pile.h"
#include "BT_OverlayComponent.h"
#include "BT_Util.h"
#include "BT_AnimationManager.h"
#include "BT_TextManager.h"
#include "BT_StickyNoteActor.h"
#include "BT_WindowsOS.h"

void FinderTool::search() 
{
	if (Finder->searchString.isEmpty()) cancel();

	if (!_active) sel->clear();
	_active = true;

	//Record whether objects have active alpha or not
	vector<BumpObject *> searchObjs = scnManager->getVisibleBumpActorsAndPiles(true);

	for (int i = 0; i < searchObjs.size(); i++) {
		if (!_nameVisibleMap.contains(searchObjs[i]))
			_nameVisibleMap.insert(searchObjs[i], !searchObjs[i]->isTextHidden());
	}
	highlightBumpObjectsWithSubstring(Finder->searchString, true);
}

void FinderTool::cancel() 
{	
	_active = false;	
	_fadeOut = true;
	clearHighlightBumpObjectsWithSubstring();
	searchString.clear();
}

bool FinderTool::isActive()
{
	return _active;
}

void FinderTool::showText(BumpObject * obj)
{	
	//We don't want to show the filename for sticky note actors
	if (obj->getObjectType() != ObjectType(BumpActor, FileSystem, StickyNote))
		obj->showText();
}

void FinderTool::hideText(BumpObject * obj)
{	
	obj->hideText();
}


FinderTool::FinderTool() :_alphaAnim(0), _onHighlightBumpObjectsWithSubstring(NULL), _active(false)
{
}
void FinderTool::setOnHighlightBumpObjectsWithSubstring( boost::function<void(QString)> onHighlightBumpObjectsWithSubstring )
{
	_onHighlightBumpObjectsWithSubstring = onHighlightBumpObjectsWithSubstring;
}

void FinderTool::onAnimTick() 
{
	static const float tDiff = 1.0f / 60.0f;

	//If less than GLOBAL(maxTime) ramp up alpha anim
	if (elapsedTime.elapsed() < GLOBAL(maxDelayBetweenKeyDown) && !_fadeOut && _active)
	{
		_alphaAnim = min(0.8f, _alphaAnim + 3.0f*tDiff);		
	}

	//Else bring it down slowly
	else {
		_alphaAnim = max(0.0f, _alphaAnim - 2.0f*tDiff);
	}

	if (_alphaAnim == 0.0f)
	{
		_fadeOut = false;
		_active = false;
	}
}
void FinderTool::killAnimation() 
{
	onAnimFinished();
}
void FinderTool::finishAnimation() 
{
	onAnimFinished();
}

//During a shutdown in the middle of a FAYT, the objects alpha may be incorrect
//This sets the alpha to its correct value bypassing animations
void FinderTool::shutdown()
{
	vector<BumpObject *> objs = scnManager->getVisibleBumpActorsAndPiles(true);
	for (int i = 0; i < objs.size(); i++) {
		BumpObject *obj = objs[i];

		if (_nameVisibleMap.contains(obj)) {
			if (_nameVisibleMap.value(obj))
				obj->getNameableOverlay()->setAlpha(1.0f);
			//The sel->onRemove handles the case where thumbnails filenames are hidden
			else if  (obj->getObjectType() != ObjectType(BumpActor, FileSystem, Thumbnail) )			
				obj->getNameableOverlay()->setAlpha(0.0f);
		}
	}
}

void FinderTool::onAnimFinished() 
{
	_alphaAnim = 0.0f;
	_fadeOut = false;
	_active = false;
	
	//Resets the objects
	vector<BumpObject *> objs = scnManager->getVisibleBumpActorsAndPiles(true);
	for (int i = 0; i < objs.size(); i++) {
		BumpObject *obj = objs[i];

		if (_nameVisibleMap.contains(obj)) {
			if (_nameVisibleMap.value(obj))
				obj->showText();
			//The sel->onRemove handles the case where thumbnails filenames are hidden
			else if  (obj->getObjectType() != ObjectType(BumpActor, FileSystem, Thumbnail) )			
				obj->hideText();
		}
	}
	
	_nameVisibleMap.clear();
}
bool FinderTool::isAnimating(uint animType) 
{
	return (_active || _fadeOut);
}

//Renders a fullscreen gray box on top of current objects
//but below selected objects to make FAYT better
void FinderTool::renderGrayScreen()
{
	//Dont draw a complete translucent gray screen
	if (_alphaAnim == 0.0f) return;

	dxr->switchToOrtho();
	dxr->beginRenderBillboard();
	
	dxr->device->SetTransform(D3DTS_WORLD, &dxr->identity);
	
	// This sets the color of the billboard
	D3DMATERIAL9 material = {0};
	material.Diffuse = D3DXCOLOR(0.0f, 0.0f, 0.0f, _alphaAnim);
	material.Ambient = D3DXCOLOR(0, 0, 0, 1);
	dxr->device->SetMaterial(&material);	

	dxr->device->SetTextureStageState(0,D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	dxr->device->SetTextureStageState(0,D3DTSS_COLORARG1, D3DTA_DIFFUSE);	
	dxr->device->SetTextureStageState(0,D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	dxr->device->SetTextureStageState(0,D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);	

	dxr->renderBillboard(Vec3(0,0,0), Vec3(winOS->GetWindowWidth(), winOS->GetWindowHeight(), 0), D3DXCOLOR(1,1,1,1));

	dxr->device->SetTextureStageState(0,D3DTSS_COLOROP, D3DTOP_MODULATE);
	dxr->device->SetTextureStageState(0,D3DTSS_COLORARG1, D3DTA_TEXTURE);
	dxr->device->SetTextureStageState(0,D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	dxr->device->SetTextureStageState(0,D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

	dxr->endRenderBillboard();
	dxr->switchToPerspective();
}

int FinderTool::fuzzyMatch(const QString& haystack, const QString& needle)
{
	// see http://ginstrom.com/scribbles/2007/12/01/fuzzy-substring-matching-with-levenshtein-distance-in-python/
	if (haystack.size() == 0 || needle.size() == 0)
		return -1;

	const int nlen = needle.size(),
		hlen = haystack.size();
	if (hlen == 0) return -1;
	if (nlen == 1) return haystack.indexOf(needle);
	vector<int> row1(hlen+1, 0);
	for (int i = 0; i < nlen; ++i) {
		vector<int> row2(1, i+1);
		for (int j = 0; j < hlen; ++j) {
			const int cost = needle[i] != haystack[j];
			row2.push_back(std::min(row1[j+1]+1,
				std::min(row2[j]+1,
				row1[j]+cost)));
		}
		row1.swap(row2);
	}
	return *std::min_element(row1.begin(), row1.end());
}


int FinderTool::getObjectsMatchingSubString(QString query, 
								const vector<BumpObject *>& searchObjectsList,
								const vector<BumpObject *>& existingObjectsList,
								QSet<BumpObject *>& newlyMatchedObjectsOut,
								QSet<BumpObject *>& newlyDiscardedObjectsOut,
								QSet<BumpObject *>& matchedObjectsOut,
								bool matchSubString, bool isFuzzy)
{
	// fuzzy matching only for substring searches
	if (!matchSubString && isFuzzy)
		return 0;

	// make the query lower case
	query = query.toLower();

	// convert list of existing objects into a set
	QSet<BumpObject *> existingObjects;
	for (int i = 0; i < existingObjectsList.size(); ++i)
	{
		BumpObject * obj = existingObjectsList[i];
		existingObjects.insert(obj);

		// flatten piles
		if (obj->isBumpObjectType(BumpPile)) {
			vector<BumpObject *> pileItems = ((Pile *) obj)->getPileItems();
			for (int j = 0; j < pileItems.size(); ++j) {
				existingObjects.insert(pileItems[j]);
			}
		}
	}

	// for each item in the search list	
	QList<BumpObject *> closestMatches;
	int closestMatchesError = (1 << 10);
	for (int i = 0; i < searchObjectsList.size(); ++i)
	{
		BumpObject * obj = searchObjectsList[i];
		Actor * actor = obj->isBumpObjectType(BumpActor) ? (Actor *) obj : NULL;
		Pile * pile = obj->isBumpObjectType(BumpPile) ? (Pile *) obj : NULL;

		StickyNoteActor *sticky = NULL;
		if (obj->getObjectType() == ObjectType(BumpActor, FileSystem, StickyNote))
		{		
			sticky = (StickyNoteActor*)obj;
		}

		QString fileName;
		if (sticky)		
			fileName = sticky->getStickyText().toLower();

		else if (actor)
			fileName = actor->getFullText().toLower();
		else if (pile)
			fileName = pile->getFullText().toLower();
		else
			continue;

		if (matchSubString)
		{
			if (isFuzzy)
			{				
				int error = fuzzyMatch(fileName, query);
				// consoleWrite(QString("Compare: %1 => %2, %3/%4\n").arg(query).arg(fileName).arg(error));
				if (error > -1 && error < (query.size() / 2))
				{
					if (error < closestMatchesError)
					{
						closestMatches.clear();
						closestMatches.append(obj);
						closestMatchesError = error;
					}
					else if (error == closestMatchesError)
					{
						closestMatches.append(obj);
					}						
				}
			}
			else // !isFuzzy
			{
				if (fileName.contains(query))
					matchedObjectsOut.insert(obj);
			}
		}
		else // !matchSubString
		{
			if (fileName.startsWith(query))
				matchedObjectsOut.insert(obj);
		}
	}

	// insert any fuzzy matches
	QListIterator<BumpObject *> closestMatchesIter(closestMatches);
	while (closestMatchesIter.hasNext())
	{
		BumpObject * obj = closestMatchesIter.next();
		matchedObjectsOut.insert(obj);
	}

	// properly mark each items in the matched set
	QSetIterator<BumpObject *> matchedObjectsOutIter(matchedObjectsOut);
	while (matchedObjectsOutIter.hasNext())
	{
		BumpObject * obj = matchedObjectsOutIter.next();
		if (!existingObjects.contains(obj))
			newlyMatchedObjectsOut.insert(obj);
	}
	// properly mark each item in the previous set
	for (int i = 0; i < existingObjectsList.size(); ++i)
	{
		BumpObject * obj = existingObjectsList[i];
		if (!matchedObjectsOut.contains(obj))
		{
			newlyDiscardedObjectsOut.insert(obj);
		}
	}

	return matchedObjectsOut.size();
}

// Called after a new query results in no matches, or a user
//   hits the Escape key during a search.  Closes all selected
//   leaf piles resulting from the search, fades all actors
//   back into view, dismisses the search message and clears 
//	 the selection.
void FinderTool::clearHighlightBumpObjectsWithSubstring()
{	
	// if there are leafed pile items selected from the search
	// then we can try closing the pile
	vector<BumpObject *> selectedObjs = sel->getBumpObjects();
	for (int i = 0; i < selectedObjs.size(); ++i)
	{
		BumpObject * obj = selectedObjs[i];
		BumpObject * objParent = obj->getParent();
		if (objParent && objParent->isBumpObjectType(BumpPile)) 
		{
			Pile * parent = (Pile *) objParent;
			if (parent->getActiveLeafItem() == obj)
				parent->close();
		}
	}

	// clear the find-as-you-type message
	dismiss("HighlightBumpObjectsWithSubstring");

	// clear the selection if there are no matches
	sel->clear();
}

void FinderTool::highlightBumpObjectsWithSubstring(QString query, bool matchAnySubString)
{
	const int fadeLength = 400;
	const int textFadeLength = 600;
	const float fadeStart = 0.08f;
	int numFilesMatching = 0;
	bool hasFuzzyMatches = false;
	QSet<BumpObject *> newlyMatchedObjs, newlyDiscardedObjs, matchedObjs;
	vector<BumpObject *> searchObjs = scnManager->getVisibleBumpActorsAndPiles(true);

	if (!query.isEmpty())
	{
		// clear the selection if it's the first char
		// increment the stats (only on the first char)
		if (query.size() == 1) 
		{
			vector<BumpObject *> curObjs = sel->getBumpObjects();
			for (int i = 0; i < curObjs.size(); ++i)
				animManager->finishAnimation(curObjs[i]);
			sel->clear();
			statsManager->getStats().bt.interaction.search.findAsYouType++;
		}

		// make callback if one exists
		if (!_onHighlightBumpObjectsWithSubstring.empty())
			_onHighlightBumpObjectsWithSubstring(query);

		// try and match some files	
		vector<BumpObject *> curObjs = sel->getBumpObjects();
		numFilesMatching = getObjectsMatchingSubString(query, 
			searchObjs, curObjs, newlyMatchedObjs, newlyDiscardedObjs, 
			matchedObjs, matchAnySubString, false);

		/*
		consoleWrite(QString("----\nquery: %1\nnumFilesMatching: %2\nnewlyMatchedObjs: %3\nnewlyDiscardedObjs: %4\nmatchedObjs: %5\ncurObjs: %6\n")
		.arg(query).arg(numFilesMatching).arg(newlyMatchedObjs.size()).arg(newlyDiscardedObjs.size())
		.arg(matchedObjs.size()).arg(curObjs.size()));

		consoleWrite("matchedObjs\n");
		QSetIterator<BumpObject *> iter(matchedObjs);
		while (iter.hasNext()) {
		consoleWrite(iter.next()->getFullText() + "\n");
		}
		consoleWrite("newlyMatchedObjs\n");
		QSetIterator<BumpObject *> iterNew(newlyMatchedObjs);
		while (iterNew.hasNext()) {
		consoleWrite(iterNew.next()->getFullText() + "\n");
		}
		consoleWrite("newlyDiscardedObjs\n");
		QSetIterator<BumpObject *> iterOld(newlyDiscardedObjs);
		while (iterOld.hasNext()) {
		consoleWrite(iterOld.next()->getFullText() + "\n");
		}
		*/

		/*
		// if no files match and we can fuzzy match, do so
		if (numFilesMatching <= 0 && matchAnySubString)
		{
		hasFuzzyMatches = true;
		numFilesMatching = GetObjectsMatchingSubString(query, 
		searchObjs, curObjs, newlyMatchedObjs, newlyDiscardedObjs, 
		matchedObjs, matchAnySubString, true);

		consoleWrite(QString("FUZZY - numFilesMatching: %1\nnewlyMatchedObjs: %2\nnewlyDiscardedObjs: %3\nmatchedObjs: %4\n")
		.arg(numFilesMatching).arg(newlyMatchedObjs.size()).arg(newlyDiscardedObjs.size())
		.arg(matchedObjs.size()));
		}
		*/
	}

	// add/remove from the selection
	assert(numFilesMatching == matchedObjs.size());
	if (numFilesMatching == 0)
	{
		_fadeOut = true;
		clearHighlightBumpObjectsWithSubstring();		
		
	}
	else // numFilesMatching > 0
	{
		if (!animManager->isObjAnimating(this))
		{
			AnimationEntry ae(this);
			animManager->addAnimation(ae);		
			
		}
		_fadeOut = false;

		// disable text relayout when selecting a large group of items
		textManager->disableForceUpdates();

		
		for (int i = 0; i < searchObjs.size(); i++) 
		{
			if (!matchedObjs.contains(searchObjs[i]))
			{
				hideText(searchObjs[i]);				
			}
		}

		// special case: scroll to make that file visible if it is in a gridded pile
		if (numFilesMatching == 1)
		{
			BumpObject * obj = *(matchedObjs.begin());
			BumpObject * parent = obj->getParent();
			if (parent && parent->isObjectType(ObjectType(BumpPile, NULL, Grid)))
			{			
				if (((Pile *)parent)->scrollToGridItem(obj))
				{
					sel->add(obj);
					showText(obj);
				}
			}
		}

		// bounce the newly matched files
		QSetIterator<BumpObject *> newlyMatchedObjsIter(newlyMatchedObjs);
		while (newlyMatchedObjsIter.hasNext())
		{
			BumpObject * obj = newlyMatchedObjsIter.next();
			showText(obj);
			sel->add(obj);			
		}

		// finish animations of discarded objects
		QSetIterator<BumpObject *> newlyDiscardedObjsIter(newlyDiscardedObjs);
		while (newlyDiscardedObjsIter.hasNext())
		{
			// check if the pile should be selected
			bool removeFromSelection = true;
			BumpObject * obj = newlyDiscardedObjsIter.next();
			if (obj->isBumpObjectType(BumpPile)) {
				bool hasChildSelected = false;
				vector<BumpObject *> pileItems = ((Pile *) obj)->getPileItems();
				for (int j = 0; j < pileItems.size(); ++j) {
					if (matchedObjs.contains(pileItems[j]))
						hasChildSelected = true;
				}
				removeFromSelection = !hasChildSelected;
			}

			if (removeFromSelection)
			{
				sel->remove(obj);
				animManager->finishAnimation(obj);	// need to finish animation after removing from selection?				
				hideText(obj);		

			}
		}

		// disable text relayout when selecting a large group of items
		textManager->enableForceUpdates();
	}

	// notify the user if the search of their query string resulted in no results
	if (numFilesMatching > 0)
	{
		// notify how many files found
		MessageClearPolicy clearPolicy;
		clearPolicy.setTimeout(1);
		QString msg;
		if (hasFuzzyMatches)
			msg = QString(BtUtilStr->getString("SimilarFilesFound")).arg(numFilesMatching).arg(query);
		else
			msg = QString(BtUtilStr->getString("FilesFound")).arg(numFilesMatching).arg(query);
		scnManager->messages()->addMessage(new Message("HighlightBumpObjectsWithSubstring", msg, Message::Ok, clearPolicy));
	}
	else if (!query.isEmpty())
	{
		MessageClearPolicy clearPolicy;
		clearPolicy.setTimeout(1);
		QString msg = QString(BtUtilStr->getString("NoFilesFound")).arg(query);
		scnManager->messages()->addMessage(new Message("HighlightBumpObjectsWithSubstring", msg, Message::Ok, clearPolicy));
	}
}
