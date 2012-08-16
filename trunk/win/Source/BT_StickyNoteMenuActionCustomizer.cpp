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
#include "BT_Authorization.h"
#include "BT_MenuAction.h"
#include "BT_SceneManager.h"
#include "BT_StickyNoteMenuActionCustomizer.h"
#include "BT_Util.h"
#include "BT_FileSystemManager.h"

StickyNoteMenuActionCustomizer::StickyNoteMenuActionCustomizer()
{

}

void StickyNoteMenuActionCustomizer::update( MenuAction *action )
{
	if (hasExceededMaxNumStickyNotes() && action->getAdditionalLabel() != "PRO")
	{
		action->setEnabled(false);
		action->setAdditionalLabel("PRO");
		_toolTip = action->getToolTip();
		action->setToolTip(QT_TR_NOOP("More than two sticky notes is a PRO feature"));
	} else if (!hasExceededMaxNumStickyNotes() && action->getAdditionalLabel() == "PRO") {
		action->setEnabled(true);
		action->setAdditionalLabel(QString());
		action->setToolTip(_toolTip);
	}
}

void StickyNoteMenuActionCustomizer::execute( MenuAction *action )
{
	if (hasExceededMaxNumStickyNotes())
		launchBumpTopProPage("stickyNote");
	else 
		Key_CreateStickyNote();
}

bool StickyNoteMenuActionCustomizer::hasExceededMaxNumStickyNotes()
{
	return GLOBAL(settings).freeOrProLevel == AL_FREE && GLOBAL(settings).curNumStickyNotes >= GLOBAL(settings).maxNumFreeStickyNotes;
}