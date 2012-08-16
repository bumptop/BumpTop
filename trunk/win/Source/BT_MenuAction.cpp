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
#include "BT_BumpObject.h"
#include "BT_FileSystemActor.h"
#include "BT_MarkingMenu.h"
#include "BT_MenuAction.h"
#include "BT_MenuActionCustomizer.h"
#include "BT_OverlayComponent.h"
#include "BT_Selection.h"
#include "BT_SceneManager.h"
#include "BT_Util.h"
#include "BT_WebActor.h"

MenuAction::MenuAction()
{
}

MenuAction::MenuAction(	KeyCombo combo, bool isEnabled, QString newLabel, 
						QString newToolTip, float newAngSpan, float newRadThickness, 
						float newDegPref, uint selection, ObjectType objTypeCond,
						VoidCallback exeAction, MenuActionCustomizer *cust, bool isSubMenu, 
						bool isCheckBox, BoolCallback myCheckCondition)
{
	enabled = isEnabled;
	angularSpan = newAngSpan;
	radiusThickness = newRadThickness;
	degreePreference = newDegPref;
	selectionCondition = selection;
	executeAction = exeAction;
	customizer = cust;
	label = newLabel;
	toolTip = newToolTip;
	hotKey = combo;
	objectTypeCondition = objTypeCond;
	useObjectTypeCondition = *((uint *) &objectTypeCondition) > 0 ? true : false;
	animating = false;
	_isSubMenu = isSubMenu;
	_isCheckBox = isCheckBox;
	checkCondition = myCheckCondition;

	// setup the text buffers	
	if (newDegPref > Ellipsis) 
	{
		_primaryTextBuffer.setMaxBounds(QSize(256, 0));
		_primaryTextBuffer.setText(newLabel);
		_primaryTextBuffer.pushFlag(TextPixmapBuffer::GradientColor);
		_primaryTextBuffer.setGradientColors(QColor(255, 255, 255), QColor(210, 210, 210));
		_primaryTextBuffer.update();
		_secondaryTextBuffer.setMaxBounds(QSize(256, 0));
		_secondaryTextBuffer.setText(getHotkeyString());
		_secondaryTextBuffer.pushFlag(TextPixmapBuffer::GradientColor);
		_secondaryTextBuffer.setGradientColors(QColor(245, 245, 245), QColor(200, 200, 200));
		_secondaryTextBuffer.update();
	}
}

MenuAction::~MenuAction()
{
}

bool MenuAction::isSubMenu()
{
	return _isSubMenu;
}

bool MenuAction::isCheckBox()
{
	return _isCheckBox;
}

bool MenuAction::checkConditionMet() {
	if(checkCondition == NULL)
		return false;
	return checkCondition();
}

void MenuAction::execute()
{
	if (customizer)
	{
		// Let the customizer decide what to do
		customizer->execute(this);
	}else{
		// Execute either the callback or the Customizer's callback
		if (executeAction)
		{
			executeAction();
		}
	}
	dismiss("MarkingMenu_Tooltip");
}

float MenuAction::getAngularSpan() const
{
	return angularSpan;
}

QString MenuAction::getLabel() const
{
	return label;
}

QString MenuAction::getHotkeyString() const
{
	if (hotKey.subKeys.key > 0)
		return QString("(%1)").arg(hotKey.toString());
	return QString();
}


float MenuAction::getDegreePreference() const
{
	return degreePreference;
}

float MenuAction::getRadiusThickness() const
{
	return radiusThickness;
}

QString MenuAction::getToolTip() const
{
	return toolTip;
}

MenuActionCustomizer *MenuAction::getCustomAction() const
{
	return customizer;
}

KeyCombo MenuAction::getHotKey() const
{
	return hotKey;
}

bool MenuAction::isEnabled()
{
	return enabled;
}

void MenuAction::setEnabled(bool en)
{
	enabled = en;
}

void MenuAction::setLabel(QString newLabel)
{
	label = newLabel;
	if (degreePreference > Ellipsis) 
	{
		_primaryTextBuffer.setText(newLabel);
		_primaryTextBuffer.update();
	}
}

void MenuAction::setCustomAction(MenuActionCustomizer *cust)
{
	customizer = cust;

	if (cust)
	{
		// The customizer should update this class
		cust->update(this);
	}
}

void MenuAction::setHotKey(KeyCombo &combo)
{
	hotKey = combo;
	_secondaryTextBuffer.setText(getHotkeyString());
	_secondaryTextBuffer.update();
}

void MenuAction::setToolTip(QString tip)
{
	toolTip = tip;
}

void MenuAction::setAngularSpan(float val)
{
	angularSpan = val;
}

void MenuAction::setRadiusThickness(float val)
{
	radiusThickness = val;
}

void MenuAction::setDegreePreference(float val)
{
	degreePreference = val;
}

void MenuAction::setExecutionCallback(VoidCallback callback)
{
	executeAction = callback;
}

void MenuAction::update()
{
	
}

VoidCallback MenuAction::getExecuteAction() const
{
	return executeAction;
}

bool MenuAction::operator<(const MenuAction &left) const
{
	return hotKey.subKeys.key < left.getHotKey().subKeys.key;
}

bool MenuAction::isAnimating() const
{
	return animating;
}

void MenuAction::refreshCustomizer()
{
	if (customizer)
	{
		// Notify the customizer that we are registering with it
		customizer->update(this);
	}
}

#ifdef DXRENDER
void MenuAction::onRelease()
{
	_primaryTextBuffer.onRelease();
	_secondaryTextBuffer.onRelease();
}
#endif

bool MenuAction::isSelectionValid(uint cond) const
{
	vector<BumpObject *> selection = sel->getBumpObjects();
	uint selSize = selection.size();
	ObjectType objType;
	uint tempSel = selectionCondition;

	// Exclude all but the type specified
	if (selectionCondition & UseOnlyThisType)
	{
		// Remove this tag for comparison purposes
		tempSel &= ~UseOnlyThisType;

		if (cond & ~tempSel)
		{
			return false;
		}
	}

	// Check if condition of selection meets this menu action
	if (cond & selectionCondition)
	{
		if (useObjectTypeCondition)
		{
			// Try to find an object in the selection that matches the ObjectType condition
			for (uint i = 0; i < selSize; i++)
			{
				objType = selection[i]->getObjectType();

				// SPECIAL CASE for expanding virtual folders
				if (objType == ObjectType(BumpActor, FileSystem, Virtual) &&
					executeAction == Key_ExpandToPile)
					continue;

#ifndef BTDEBUG
				// hide "Reload page" action for sharing and facebook widget (but leave for web pages)
				if (objType == ObjectType(BumpActor, Webpage) &&
					executeAction == Key_ReloadWebActor)
				{
					WebActor* tempWebActor = (WebActor*) selection[i];
					if (tempWebActor->isSharingWidget() || tempWebActor->isFacebookWidgetUrl())
						return false;
				}
#endif

				// This object matches the ObjectType specified
				if (objType == objectTypeCondition)
				{
					return true;
				}
			}
		}else{
			return true;
		}
	}

	// Condition not Matched
	return false;
}

void MenuAction::setAdditionalLabel( QString label )
{
	additionalLabel = label;
	if (!label.isEmpty())
		_secondaryTextBuffer.setText(label);
	else
		_secondaryTextBuffer.setText(getHotkeyString());
	_secondaryTextBuffer.update();
}

QString MenuAction::getAdditionalLabel() const
{
	return additionalLabel;
}

TextPixmapBuffer * MenuAction::getPrimaryTextBuffer()
{
	return &_primaryTextBuffer;
}

TextPixmapBuffer * MenuAction::getSecondaryTextBuffer()
{
	return &_secondaryTextBuffer;
}

void MenuAction::setPrimaryTextFont(const QFont& font)
{
	if (degreePreference > Ellipsis) 
	{
		_primaryTextBuffer.setFont(font);
		_primaryTextBuffer.update();
	}
}

void MenuAction::setSecondaryTextFont(const QFont& font)
{
	if (degreePreference > Ellipsis) 
	{
		_secondaryTextBuffer.setFont(font);
		_secondaryTextBuffer.update();
	}
}