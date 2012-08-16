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

#pragma once

#ifndef _BT_MENU_ACTION_
#define _BT_MENU_ACTION_

// -----------------------------------------------------------------------------

#include "BT_KeyCombo.h"
#include "BT_ObjectType.h"
#include "TextPixmapBuffer.h"

class MenuActionCustomizer;

// -----------------------------------------------------------------------------

class MenuAction
{
	bool					enabled;
	bool					animating;
	QString					label;
	QString					toolTip;
	QString					additionalLabel;
	MenuActionCustomizer	*customizer;
	KeyCombo				hotKey;
	float					angularSpan;
	float					radiusThickness;
	float					degreePreference;
	uint					selectionCondition;
	ObjectType				objectTypeCondition;
	bool					useObjectTypeCondition;
	VoidCallback			executeAction;
	BoolCallback			checkCondition;
	TextPixmapBuffer		_primaryTextBuffer;
	TextPixmapBuffer		_secondaryTextBuffer;
	bool					_isSubMenu;
	bool					_isCheckBox;

public:

	// Constructors & Destructor
	MenuAction();
	MenuAction(KeyCombo combo, bool isEnabled, QString newLabel, QString newToolTip, float newAngSpan, float newRadThickness, float newDegPref, uint selection, ObjectType objTypeCond, VoidCallback exeAction = NULL, MenuActionCustomizer *cust = NULL, bool isSubMenu = false, bool isCheckBox = false, BoolCallback myCheckCondition = NULL);
	~MenuAction();

	vector<MenuAction>		subMenu;

	// Actions
	void					execute();
	void					update();
	void					refreshCustomizer();
#ifdef DXRENDER
	void					onRelease();
#endif

	// Operations
	bool					operator<(const MenuAction &left) const;

	// Getters
	float					getAngularSpan() const;
	QString					getLabel() const;
	QString					getHotkeyString() const;
	float					getDegreePreference() const;
	float					getRadiusThickness() const;
	QString					getToolTip() const;
	MenuActionCustomizer	*getCustomAction() const;
	KeyCombo				getHotKey() const;
	VoidCallback			getExecuteAction() const;
	QString					getAdditionalLabel() const;	
	TextPixmapBuffer *		getPrimaryTextBuffer();
	TextPixmapBuffer *		getSecondaryTextBuffer();
	bool					isSelectionValid(uint cond) const;
	bool					isEnabled();
	bool					isAnimating() const;
	bool					isSubMenu();
	bool					isCheckBox();
	bool					checkConditionMet();

	// Setters
	void					setEnabled(bool en);
	void					setLabel(QString newLabel);
	void					setCustomAction(MenuActionCustomizer *cust);
	void					setHotKey(KeyCombo &combo);
	void					setToolTip(QString tip);
	void					setAngularSpan(float val);
	void					setRadiusThickness(float val);
	void					setDegreePreference(float val);
	void					setExecutionCallback(VoidCallback callback);
	void					setAdditionalLabel(QString additionalLabel);
	void					setPrimaryTextFont(const QFont& font);
	void					setSecondaryTextFont(const QFont& font);
};

// -----------------------------------------------------------------------------

#else
	class MenuAction;
#endif