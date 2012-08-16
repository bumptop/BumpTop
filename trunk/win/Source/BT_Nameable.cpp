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
#include "BT_AnimationManager.h"
#include "BT_FontManager.h"
#include "BT_Nameable.h"
#include "BT_NxActorWrapper.h"
#include "BT_OverlayComponent.h"
#include "BT_SceneManager.h"
#include "BT_TextManager.h"
#include "BT_Util.h"

Nameable::Nameable(bool registerWithOverlay)
: _useTruncatedText(false)
, _hasExtension(false)
, _overlay(NULL)
, _register(registerWithOverlay)
, _respectIconExtensionVisibility(false)
{
	// show the text by default
	showText(false);
}

Nameable::~Nameable()
{
	// Destroy the actor
	scnManager->nameables()->deleteItem(_overlay);
	_overlay = NULL;
}

void Nameable::showText(bool skipAnimation)
{
	if (!_overlay)
	{
		// register the Nameable 
		_overlay = new NameableOverlay(this);
		_overlay->setAlpha(1.0f);
		if (_register)
		{
			AbsoluteOverlayLayout * nameables = scnManager->nameables();
			nameables->addItem(_overlay);
		}
	}
	else
	{
		if (!skipAnimation)
		{
			// animate the 'show'
			_overlay->finishAnimation();
			_overlay->setAlphaAnim(_overlay->getStyle().getAlpha(), 1.0f, 5);
		}
		else
		{
			_overlay->finishAnimation();
			_overlay->setAlpha(1.0f);
		}
	}
}

void Nameable::showTextAnimated(int duration)
{
	if (!_overlay)
	{
		// register the Nameable 
		_overlay = new NameableOverlay(this);
		_overlay->setAlpha(1.0f);
		if (_register)
		{
			AbsoluteOverlayLayout * nameables = scnManager->nameables();
			nameables->addItem(_overlay);
		}
	}
	else
	{
		// animate the 'show'
		_overlay->finishAnimation();
		_overlay->setAlphaAnim(_overlay->getStyle().getAlpha(), 1.0f, duration);
	}
}

void Nameable::hideText(bool skipAnimation /*=false*/)
{
	if (_overlay)
	{
		if (!skipAnimation)
		{
			// animate the hide
			_overlay->finishAnimation();
			_overlay->setAlphaAnim(_overlay->getStyle().getAlpha(), 0.0f, 5);
		}
		else
		{
			_overlay->finishAnimation();
			_overlay->setAlpha(0.0f);
		}
	}
}

QString Nameable::getText() const
{
	if (_useTruncatedText)
		return _truncatedText;
	return _text;
}

QString Nameable::getFullText() const
{
	return _text;
}

void Nameable::setText( QString text )
{
	// set the text and truncate the truncated version
	_text = _truncatedText = text;
	
	// update the overlay
	if (_overlay) 
		_overlay->updateTextFromNameable();
}

QString Nameable::getTextIcon() const
{
	return _iconTextureId;
}

void Nameable::setTextIcon( QString textureId )
{
	_iconTextureId = textureId;
}

const Vec3& Nameable::getRelativeTextPosition() const
{
	return _relativeTextPosition;
}

void Nameable::setRelativeTextPosition( const Vec3& relPos )
{
	_relativeTextPosition = relPos;
}

bool Nameable::hasExtension() const
{
	return _hasExtension;
}

void Nameable::hasExtension(bool hasExtension) 
{
	_hasExtension = hasExtension;
}

bool Nameable::isTextTruncated() const
{
	return _useTruncatedText;
}

void Nameable::setTextTruncation( bool truncated )
{
	_useTruncatedText = truncated;

	// update the overlay
	if (_overlay) 
		_overlay->updateTextFromNameable();
}

bool Nameable::isTextHidden() const
{
	return (_overlay == NULL || (_overlay->getStyle().getAlpha() <= 0.0f));
}

NameableOverlay * Nameable::getNameableOverlay() const
{
	return _overlay;
}

void Nameable::setRespectIconExtensionVisibility(bool respect)
{
	_respectIconExtensionVisibility = respect;
}

void Nameable::onRenderText()
{
	const Vec3& pos = getNameableOverlay()->getPosition();
#ifdef DXRENDER
	Bounds bounds;
	bounds.setInfinite();
	getNameableOverlay()->onRender(pos, bounds);
#else
	//switchToOrtho();
	glPushMatrix();
		glTranslatef(pos.x, pos.y, pos.z);
		getNameableOverlay()->onRender();
	glPopMatrix();
	//switchToPerspective();
#endif
}