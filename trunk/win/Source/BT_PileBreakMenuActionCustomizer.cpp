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
#include "BT_FileSystemPile.h"
#include "BT_MenuAction.h"
#include "BT_Selection.h"
#include "BT_PileBreakMenuActionCustomizer.h"
#include "BT_Util.h"

PileBreakMenuActionCustomizer::PileBreakMenuActionCustomizer()
{

}

void PileBreakMenuActionCustomizer::update( MenuAction *action )
{
	if (!canBreakPile())
	{
		action->setEnabled(false);
		_toolTip = action->getToolTip();
		action->setToolTip(QT_TR_NOOP("Can not break a removable device pile."));
	}
	else
	{
		action->setEnabled(true);
		action->setToolTip(_toolTip);
	}
}

void PileBreakMenuActionCustomizer::execute( MenuAction *action )
{
	if (canBreakPile())
		Key_BreakPile();
}

bool PileBreakMenuActionCustomizer::canBreakPile()
{
	vector<Pile *> piles = sel->getFullPiles();
	unsigned int size = piles.size();
	for (unsigned int i=0; i<size; i++)
	{
		FileSystemPile * fsPile = dynamic_cast<FileSystemPile *>(piles[i]);
		if (fsPile && fsPile->getOwner()->isFileSystemType(Removable))
			return false;
	}
	return true;
}