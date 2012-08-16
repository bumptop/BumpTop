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
#include "BT_LeafForwardMenuActionCustomizer.h"
#include "BT_SceneManager.h"
#include "BT_MenuAction.h"
#include "BT_FileSystemManager.h"

LeafForwardMenuActionCustomizer::LeafForwardMenuActionCustomizer()
{}

void LeafForwardMenuActionCustomizer::update( MenuAction *action )
{
	if (GLOBAL(settings).freeOrProLevel == AL_FREE && action->getAdditionalLabel() != "PRO") {
		action->setEnabled(false);
		action->setAdditionalLabel("PRO");
		_toolTip = action->getToolTip();
		action->setToolTip(QT_TR_NOOP("Flip Page is a PRO feature"));
	} else if (GLOBAL(settings).freeOrProLevel == AL_PRO && action->getAdditionalLabel() == "PRO") {
		action->setEnabled(true);
		action->setAdditionalLabel(QString());
		action->setToolTip(_toolTip);
	}
}

void LeafForwardMenuActionCustomizer::execute( MenuAction *action )
{
	if (GLOBAL(settings).freeOrProLevel == AL_FREE)
		launchBumpTopProPage("leaf");
	else if (GLOBAL(settings).freeOrProLevel == AL_PRO) 
		Key_LeafForwardSelection();

}