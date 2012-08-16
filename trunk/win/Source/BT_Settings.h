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

#ifndef _BT_SETTINGS_
#define _BT_SETTINGS_

// -----------------------------------------------------------------------------

#include "BT_ProxySettings.h"
#include "BT_QtUtil.h"

enum AuthorizationLevel;
// -----------------------------------------------------------------------------

enum VisualSettings
{
	DefaultVisuals	= 0,
	LowVisuals		= 1,
	MediumVisuals	= 2,
	HighVisuals		= 3
};

// -----------------------------------------------------------------------------

enum UserAgent
{
	DefaultUserAgent	= 0,
	iPhoneUserAgent		= 1,
	IE8UserAgent		= 2
};

// -----------------------------------------------------------------------------

class GlobalSettings
{
private:
	ProxySettings proxySettings;
	void syncGlobalValues(Json::Value& root, bool fromRoot);
	void updateProxySettings();

	bool loadInto(QFileInfo p, Json::Value& val);
	void mergeInto(const Json::Value& fromVal, Json::Value& toVal);

	inline void syncPropertyT(bool fromValue, Json::Value& node, std::string valueKey, int& value);
	inline void syncPropertyT(bool fromValue, Json::Value& node, std::string valueKey, float& value);
	inline void syncPropertyT(bool fromValue, Json::Value& node, std::string valueKey, QString& value);
	inline void syncPropertyT(bool fromValue, Json::Value& node, std::string valueKey, QList<QString>& value);
	inline void syncPropertyT(bool fromValue, Json::Value& node, std::string valueKey, bool& value);
	inline void syncPropertyT(bool fromValue, Json::Value& node, std::string valueKey, VisualSettings& value);

public:
	GlobalSettings();
	~GlobalSettings();
	
	void load(QFileInfo p);
	void save(QFileInfo p);

	void initFirstTime(); // for the first time you create a settings file

	UserAgent userAgent;

	int prevProxyMode;
	QString prevAutoProxyConfigurationUrl;
	QString prevHttpProxyUrl;

	bool dropItemsIntoSceneOnStartup;	// false
	bool enableTossing;					// false

	bool useRightClickMenuInvocation;		// true
	bool enableCameraPanning;		// false

	QString cameraPreset;			// empty string

	bool captureStatistics;			// true

	float topDownViewBufferSize;	// = 1.12f
	int photoFrameSourceDuration;	// = 1200000	[20min]
	int photoFrameImageDuration;	// = 30000	[30secs]

	int shiftOn;//=40; //shift when shiftTally has reached this number
	int maxLinearPileHeight;//=5;
	float EndTossVelocitySq;//=10*10; //once below this threshold, item is no longer considered being tossed
	float MinStartTossVelocitySq;//=100*100; //min velocity to be considered a toss
	float zDist;//=7;
	float yDist;//=1; //height of objects
	float xDist;//=7;
	float CAMWALLS_MAX_ANGLE; //55

	bool useAntiAliasing;
	bool useAnisotropicFiltering;
	bool repositionIconsIfOutsideWalls;
	bool hideInTaskBar;
	int undoLevels;
	int maxTextureSize; //max length/width to scaleup a texture too.  
	float maxImageSize;
	bool manualArrowOverlay;
	bool showToolTips; // = true; //true == display pie menu tool tips
	bool enableHiResIcons;
	bool scaleIconBasedOnFileSize;
	bool scaleIconsBasedOnUsage;
	float maxFileSizeForScale;
	float minFileSizeForScale;
	float minFileSizeThickness;
	float maxFileSizeThickness;
	bool AxisAlignedMode;
	bool enableTouchGestureBrowse;
	bool showConsoleWindow;
	bool useThemeIconOverrides;
	bool camWallsEnabled; //enable invisible walls surrounding camera so things can't be thrown offscreen
	bool useCPUOptimizations;
	bool drawFramesPerSecond;
	bool useWindowsBackgroundImage; //If true, use background image windows has as background.  If false load filepath in settings.BackgroundImage.  
	bool useThumbsDb;
	bool enableMultimouse; //false // if true, multiple mice will work, and so will smartboard if it's compiled in (see first line of BT_SmartBoard.h)
	bool RenderText; //true
	bool LoadHiddenFiles;
	float RotationLimitDegrees; // = 10.0;
	bool enableDebugKeys; // = false; 
	bool DrawOtherBumpTops; // = false;
	bool PrintMode; // = false; //if true, don't draw walls, background, and make clear color white (g.good for figure creation)
	int proxyMode;
	QString autoProxyConfigurationUrl;
	bool overrideIEProxySettings;
	QString httpProxyUrl;
#ifdef HWMANFDEMOMODE
	int hwManParticipantID;
#endif
	QString inviteCode;
	QString guid;
	QString authCode;
	QString monitor;
	bool showIconExtensions;
	VisualSettings visuals;
	bool launchFoldersAsInGrid;

	bool hasRunOnce;
	bool completedTutorial;
	QString proAuthCode;
	QString proInviteCode;
	bool disableLibSquish;

	// facebook
	QString fbc_uid;
	QString fbc_session;
	QString fbc_secret;

	// twitter
	QString tw_login;
	QString tw_password;
	QString tw_oauth_key;
	QString tw_oauth_secret;

	// Flickr
	QString flickr_auth_token;

	// languages
	QString languageOverride;

	bool scrollingGrowsItems;
	
	// not saved
	int maxNumFreeStickyNotes;
	int maxNumFreeWebWidgets;
	int curNumStickyNotes;
	AuthorizationLevel freeOrProLevel;
	bool isTrialVersion;

	// Battery settings
	bool disablePhotoframesOnBattery;
	bool disableAnimationsOnBattery;

	// Feedback user email
	QString userEmail;

	QString currentLibrary;
	QList<QString> otherLibraries;
	bool enableLibraryOverlay;
};

// -----------------------------------------------------------------------------

#define QUOTE(token) #token
#define syncProperty(condition, name) syncPropertyT(condition, root, QUOTE(name), name)

void GlobalSettings::syncPropertyT(bool fromValue, Json::Value& node, std::string valueKey, int& value)
{
	if (fromValue)
	{
		if (node.isMember(valueKey))
			value = node[valueKey].asInt();
	}
	else
		node[valueKey] = value;		
}

void GlobalSettings::syncPropertyT(bool fromValue, Json::Value& node, std::string valueKey, float& value)
{
	if (fromValue)
	{
		if (node.isMember(valueKey))
			value = (float) node[valueKey].asDouble();
	}
	else
		node[valueKey] = value;		
}

void GlobalSettings::syncPropertyT(bool fromValue, Json::Value& node, std::string valueKey, QString& value)
{
	if (fromValue)
	{
		if (node.isMember(valueKey))
			value = qstringFromValue(node[valueKey]);
	}
	else
		node[valueKey] = stdString(value);		
}

void GlobalSettings::syncPropertyT(bool fromValue, Json::Value& node, std::string valueKey, QList<QString>& value)
{
	if (fromValue)
	{
		if (node.isMember(valueKey) && node[valueKey].isArray())
		{
			value.clear();
			for (int i = 0; i < node[valueKey].size(); i++)
			{
				value.append(qstringFromValue(node[valueKey][i]));
			}
		}
	}
	else
	{
		node[valueKey].clear();
		QListIterator<QString> iter(value);
		while (iter.hasNext())
		{
			node[valueKey].append(stdString(iter.next()));
		}
	}
}

void GlobalSettings::syncPropertyT(bool fromValue, Json::Value& node, std::string valueKey, bool& value)
{
	if (fromValue)
	{
		if (node.isMember(valueKey))
			value = node[valueKey].asBool();
	}
	else
		node[valueKey] = value;		
}

void GlobalSettings::syncPropertyT(bool fromValue, Json::Value& node, std::string valueKey, VisualSettings& value)
{
	if (fromValue)
	{
		if (node.isMember(valueKey))
		{
			int val = node[valueKey].asInt();
			switch (val)
			{
			case 0:
				value = DefaultVisuals;
				break;
			case 1:
				value = LowVisuals;
				break;
			case 2:
				value = MediumVisuals;
				break;
			case 3:
				value = HighVisuals;
				break;
			default:
				assert(false);
				break;
			}
		}
	}
	else
	{
		switch (value)
		{
		case DefaultVisuals:
			node[valueKey] = 0;
			break;
		case LowVisuals:
			node[valueKey] = 1;
			break;
		case MediumVisuals:
			node[valueKey] = 2;
			break;
		case HighVisuals:
			node[valueKey] = 3;
			break;
		default:
			assert(false);
			break;
		}
	}
}

#endif