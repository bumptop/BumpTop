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
#include "BT_PhotoFrameDialog.h"
#include "BT_WindowsOS.h"
#include "BT_QtUtil.h"


/*
 * PhotoFrameDialog implementation
 */
PhotoFrameDialog::PhotoFrameDialog()
{
	resetToDefault();
}

PhotoFrameDialog::~PhotoFrameDialog()
{}

bool PhotoFrameDialog::onInit(HWND hwnd)
{

	// set the window strings
	SetWindowText(hwnd, (LPCTSTR) QT_TR_NOOP("Select the Photo Frame source").utf16());
	SetDlgItemText(hwnd, IDC_PHOTO_FRAME, (LPCTSTR) QT_TR_NOOP("Photo Frame Source").utf16());
	SetDlgItemText(hwnd, IDC_LOCAL_RADIO, (LPCTSTR) QT_TR_NOOP("Local Directory or Image File:").utf16());
	SetDlgItemText(hwnd, IDC_FLICKR_RADIO, (LPCTSTR) QT_TR_NOOP("Flickr tag:").utf16());
	SetDlgItemText(hwnd, IDC_RSS_RADIO, (LPCTSTR) QT_TR_NOOP("Photo RSS Feed (ie, from Flickr or Picasa Web):").utf16());
	SetDlgItemText(hwnd, IDOK, (LPCTSTR) QT_TR_NOOP("Ok").utf16());
	SetDlgItemText(hwnd, IDCANCEL, (LPCTSTR) QT_TR_NOOP("Cancel").utf16());

	// set the default values
	switch (_type)
	{
	case LocalDirectory:
		//Ordering of the dialog text setting matters because the last set item will be selected
		SetDlgItemText(hwnd, IDC_PHOTO_FLICKR_TEXT, (LPCWSTR) _tag.utf16());
		SetDlgItemText(hwnd, IDC_PHOTO_RSSFEED_TEXT, (LPCWSTR) _feed.utf16());
		SetDlgItemText(hwnd, IDC_PHOTO_LOCALDIR_TEXT, (LPCWSTR) _directory.utf16());
		SetFocus(GetDlgItem(hwnd, IDC_BROWSE_LOCALDIR));
		break;

	case FlickrTags:
		//Ordering of the dialog text setting matters because the last set item will be selected
		SetDlgItemText(hwnd, IDC_PHOTO_RSSFEED_TEXT, (LPCWSTR) _feed.utf16());
		SetDlgItemText(hwnd, IDC_PHOTO_LOCALDIR_TEXT, (LPCWSTR) _directory.utf16());
		SetDlgItemText(hwnd, IDC_PHOTO_FLICKR_TEXT, (LPCWSTR) _tag.utf16());
		SetFocus(GetDlgItem(hwnd, IDC_PHOTO_FLICKR_TEXT));
		break;

	case RSSFeed:
		//Ordering of the dialog text setting matters because the last set item will be selected
		SetDlgItemText(hwnd, IDC_PHOTO_LOCALDIR_TEXT, (LPCWSTR) _directory.utf16());
		SetDlgItemText(hwnd, IDC_PHOTO_FLICKR_TEXT, (LPCWSTR) _tag.utf16());
		SetDlgItemText(hwnd, IDC_PHOTO_RSSFEED_TEXT, (LPCWSTR) _feed.utf16());
		SetFocus(GetDlgItem(hwnd, IDC_PHOTO_RSSFEED_TEXT));
		break;

	default:
		break;
	}

	// The above code will set the _type variable to the correct type. 
	// This function will sync the radio buttons to that variable
	syncRadioButtonState(hwnd);

	// caption
	SetWindowText(hwnd, (LPCTSTR) QT_TR_NOOP("Select the Photo Frame source").utf16());

	return true;
}

bool PhotoFrameDialog::onCommand(HWND hwnd, WPARAM wParam, LPARAM lParam )
{
	uint maxChar = 1024;
	TCHAR buf[1024];

	// Process Buttons
	switch (LOWORD(wParam))
	{
		// update the type
	case IDC_LOCAL_RADIO:
		_type = LocalDirectory;
		// XXX: DOES NOT WORK, select the field
		// SendMessage(GetDlgItem(hwnd, LOWORD(wParam)), EM_SETSEL, 0, MAKELPARAM(-1, 0));
		break;
	case IDC_FLICKR_RADIO:
		_type = FlickrTags;
		// XXX: DOES NOT WORK, select the field
		// SendMessage(GetDlgItem(hwnd, LOWORD(wParam)), EM_SETSEL, 0, MAKELPARAM(-1, 0));
		break;
	case IDC_RSS_RADIO:
		_type = RSSFeed;
		// XXX: DOES NOT WORK, select the field
		// SendMessage(GetDlgItem(hwnd, LOWORD(wParam)), EM_SETSEL, 0, MAKELPARAM(-1, 0));
		break;

		// update the info
	case IDC_PHOTO_RSSFEED_TEXT:
		GetWindowText(GetDlgItem(hwnd, IDC_PHOTO_RSSFEED_TEXT), buf, maxChar);
		_feed = QString::fromUtf16((const ushort *) buf);
		_type = RSSFeed;
		break;
	case IDC_PHOTO_FLICKR_TEXT:		
		GetWindowText(GetDlgItem(hwnd, IDC_PHOTO_FLICKR_TEXT), buf, maxChar);
		_tag = QString::fromUtf16((const ushort *) buf);
		_type = FlickrTags;
		break;
	case IDC_PHOTO_LOCALDIR_TEXT:
		GetWindowText(GetDlgItem(hwnd, IDC_PHOTO_LOCALDIR_TEXT), buf, maxChar);
		_directory = QString::fromUtf16((const ushort *) buf);
		_type = LocalDirectory;
		break;
		// browse button
	case IDC_BROWSE_LOCALDIR:
		// Use the windows Folder Browse Dialog
		if (dlgManager->promptDirListing())
		{
			SetDlgItemText(hwnd, IDC_PHOTO_LOCALDIR_TEXT, (LPCWSTR) dlgManager->getText().utf16());
			_type = LocalDirectory;
		}
		break;

	default:
		return false;
	}
	// sync up radio button
	syncRadioButtonState(hwnd);
	return false;
}

void PhotoFrameDialog::resetToDefault()
{
	// reset internal vars
	_type = LocalDirectory;
	_directory = winOS->GetSystemPath(MyPictures);
	_feed.clear();
	_tag = "lego";
}

void PhotoFrameDialog::setSelectedType( PhotoFrameSourceSelectedType type )
{
	_type = type;
}

void PhotoFrameDialog::setDirectory( QString dir )
{
	_directory = dir;
}

void PhotoFrameDialog::setRawFeed( QString feed )
{
	_feed = feed;
}

void PhotoFrameDialog::setFlickrTag( QString tag )
{
	_tag = tag;
}

PhotoFrameDialog::PhotoFrameSourceSelectedType PhotoFrameDialog::getSelectedType() const
{
	return _type;
}

QString PhotoFrameDialog::getDirectory() const
{
	return _directory;
}

QString PhotoFrameDialog::getRawFeed() const
{
	return _feed;
}

QString PhotoFrameDialog::getFlickrTag() const
{
	return _tag;
}

void PhotoFrameDialog::syncRadioButtonState(HWND hwnd)
{
	// uncheck all non-type radio buttons
	SendMessage(GetDlgItem(hwnd, IDC_LOCAL_RADIO), BM_SETCHECK, (_type == LocalDirectory) ? BST_CHECKED : BST_UNCHECKED, 0);
	SendMessage(GetDlgItem(hwnd, IDC_FLICKR_RADIO), BM_SETCHECK, (_type == FlickrTags) ? BST_CHECKED : BST_UNCHECKED, 0);
	SendMessage(GetDlgItem(hwnd, IDC_RSS_RADIO), BM_SETCHECK, (_type == RSSFeed) ? BST_CHECKED : BST_UNCHECKED, 0);
}

QString PhotoFrameDialog::getSourceString() const
{
	// set the default values
	switch (_type)
	{
	case LocalDirectory:
		return _directory;
	case FlickrTags:
		return QString("flickr://") + _tag;
	case RSSFeed:
		return _feed;
	default:
		return QString();
	}
}