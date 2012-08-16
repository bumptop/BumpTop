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

#ifndef _NAMEABLE_
#define _NAMEABLE_

// -----------------------------------------------------------------------------

class NameableOverlay;

// -----------------------------------------------------------------------------

class Nameable
{
	QString _iconTextureId;
	QString _text;
	QString _truncatedText;
	bool _useTruncatedText;
	bool _hasExtension;
	bool _register;
	bool _respectIconExtensionVisibility;
	Vec3 _relativeTextPosition;
	NameableOverlay * _overlay;
	
public:
	Nameable(bool registerWithOverlay=true);
	virtual ~Nameable();

	void onRenderText();

	// operations
	void showText(bool skipAnimation=false);
	void showTextAnimated(int duration);
	void hideText(bool skipAnimation=false);
	void setRespectIconExtensionVisibility(bool respect);

	// accessors
	QString getText() const;
	QString getFullText() const;
	void setText(QString text);

	QString getTextIcon() const;
	void setTextIcon(QString textureId);

	const Vec3& getRelativeTextPosition() const;
	void setRelativeTextPosition(const Vec3& relPos);

	void hasExtension(bool hasExtension);
	bool hasExtension() const;
	
	bool isTextTruncated() const;
	void setTextTruncation(bool truncated); 
	
	bool isTextHidden() const;
	NameableOverlay * getNameableOverlay() const;
};

// -----------------------------------------------------------------------------

#include "BT_Nameable.inl"

// -----------------------------------------------------------------------------

#else
	class Nameable;
#endif