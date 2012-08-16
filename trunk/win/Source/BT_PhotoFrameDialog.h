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

#ifndef _PHOTO_FRAME_DIALOG_
#define _PHOTO_FRAME_DIALOG_

// -----------------------------------------------------------------------------

#include "BT_DialogManager.h"

// -----------------------------------------------------------------------------

class PhotoFrameDialog : public Win32Dialog
{
	Q_DECLARE_TR_FUNCTIONS(PhotoFrameDialog)
		
public: 
	enum PhotoFrameSourceSelectedType
	{
		LocalDirectory,
		FlickrTags,
		FacebookFeed,
		RSSFeed
	};

private:
	PhotoFrameSourceSelectedType _type;
	QString _directory;			
	QString _feed;
	QString _tag;
	// need to separate these out for default value reasons

private:
	void syncRadioButtonState(HWND hwnd);

public:
	PhotoFrameDialog();
	virtual ~PhotoFrameDialog();

	// dialog overrides
	virtual bool onInit(HWND hwnd);
	virtual bool onCommand(HWND hwnd, WPARAM wParam, LPARAM lParam); 
	
	// reset
	virtual void resetToDefault();

	// accessors
	void setSelectedType(PhotoFrameSourceSelectedType type);
	void setDirectory(QString dir);
	void setRawFeed(QString feed);
	void setFlickrTag(QString tag);

	PhotoFrameSourceSelectedType getSelectedType() const;
	QString getDirectory() const;
	QString getRawFeed() const;
	QString getFlickrTag() const;
	QString getSourceString() const;
};

#endif // _PHOTO_FRAME_DIALOG_