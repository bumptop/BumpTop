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
#include "BT_Settings.h"
#include "BT_WindowsOS.h"
#include "BT_Logger.h"
#include "BT_FileTransferManager.h"
#include "BT_Authorization.h"
#include "BT_Camera.h"

GlobalSettings::GlobalSettings()
{
	// Default Values
	userAgent = DefaultUserAgent;
	dropItemsIntoSceneOnStartup = false;
	cameraPreset = "";
	useRightClickMenuInvocation = true;
	enableCameraPanning = false;
	captureStatistics = false;
	maxNumFreeStickyNotes = 2;
	maxNumFreeWebWidgets = 2;
	curNumStickyNotes = 0;
	bool queryShowIconExtensions = false;
	DWORD tmp = winOS->getRegistryDirectDwordValue(
		"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced", 
		"HideFileExt", queryShowIconExtensions);
	showIconExtensions = queryShowIconExtensions && (tmp == 0);
	disableLibSquish = false;

	visuals = MediumVisuals;
	launchFoldersAsInGrid = true;
	enableTossing = false;

	topDownViewBufferSize = 1.12f;
	photoFrameSourceDuration = 1200;
	photoFrameImageDuration = 30;

	shiftOn=40;
	maxLinearPileHeight=5;
	EndTossVelocitySq=10*10;
	MinStartTossVelocitySq=100*100;
	zDist=7;
	yDist=1;
	xDist=7;
	CAMWALLS_MAX_ANGLE=35;

	repositionIconsIfOutsideWalls = true;
	useAntiAliasing = true;
	useAnisotropicFiltering = true;
	hideInTaskBar = true;
	undoLevels = 20;
	maxTextureSize = 1024;
	maxImageSize = 30;
	manualArrowOverlay = true;
	showToolTips = true;
	enableHiResIcons = true;
	scaleIconBasedOnFileSize = true;
	scaleIconsBasedOnUsage = true;
	minFileSizeForScale = 1000000;
	maxFileSizeForScale = 25000000;
	minFileSizeThickness = 1;
	maxFileSizeThickness = 3;
	AxisAlignedMode = false;
	enableTouchGestureBrowse = true;
	showConsoleWindow = false;
	// NOTE: on XP we have some nice icons that we should override with
	useThemeIconOverrides = winOS->IsWindowsVersion(WindowsXP);
	camWallsEnabled = false;
	useCPUOptimizations = true;

	disablePhotoframesOnBattery = true;
	disableAnimationsOnBattery = true;

#ifdef _DEBUG
	drawFramesPerSecond = true;
#else
	drawFramesPerSecond = false;
#endif
	useWindowsBackgroundImage = false;
	useThumbsDb = true;
	enableMultimouse = false;
	RenderText = true;
	LoadHiddenFiles = false;
	RotationLimitDegrees = 10.0;
	enableDebugKeys = false;
	DrawOtherBumpTops = false;
	PrintMode = false;
	prevProxyMode = NO_PROXY;
	proxyMode = NO_PROXY;
	autoProxyConfigurationUrl = QString();
	overrideIEProxySettings = false;
#ifdef HWMANFDEMOMODE
	hwManParticipantID = -1;
#endif
	hasRunOnce = false;
	completedTutorial = false;
	scrollingGrowsItems = false;
	freeOrProLevel = AL_FREE;
	isTrialVersion = false;
	languageOverride.clear();

	currentLibrary = QT_NT("");
	enableLibraryOverlay = true;
}

GlobalSettings::~GlobalSettings()
{}

bool GlobalSettings::loadInto(QFileInfo p, Json::Value& root)
{
	LOG("GlobalSettings::loadInto");
	QString settingsStr = read_file_utf8(native(p));

	if (!settingsStr.isEmpty())
		GUID_LOG("successfully loaded settings file");
	else
		GUID_LOG("loading settings file failed");

	Json::Reader reader;
	QByteArray tmp = settingsStr.toUtf8();
	return reader.parse(tmp.constData(), root);	 
}

void GlobalSettings::mergeInto(const Json::Value& fromVal, Json::Value& toVal)
{
	// XXX: This assumes that values do not change types!

	// copy each member over
	vector<std::string> members = fromVal.getMemberNames();
	for (int i = 0; i < members.size(); ++i)
	{
		const string& memberName = members[i];
		toVal[memberName] = fromVal[memberName];
		if (fromVal[memberName].isObject())
			mergeInto(fromVal[memberName], toVal[memberName]);
	}
}

void GlobalSettings::load( QFileInfo p )
{
	LOG("GlobalSettings::load");
	Json::Value root;

	if (exists(p))
		GUID_LOG("settings file exists");
	else
		GUID_LOG("settings file does not exists");
	
	if (loadInto(p, root))
	{
		// pull values from the root into this global settings
		LOG("\tsyncGlobalValues");
		GUID_LOG("successfully parsed settings file");
		syncGlobalValues(root, true);
	}
	else
	{
		GUID_LOG("parsing settings file FAILED");
	}

	LOG("\tupdateProxySettings");
	updateProxySettings();
	LOG("~GlobalSettings::load");
}

void GlobalSettings::initFirstTime()
{
	LOG("GlobalSettings::initFirstTime");
	updateProxySettings();
}

void GlobalSettings::save( QFileInfo p )
{
	LOG("GlobalSettings::save");

	// save current settings
	Json::Value currentSettings;
	syncGlobalValues(currentSettings, false);

	// load existing settings in file
	Json::Value fileSettings;
	loadInto(p, fileSettings);

	// merge the two settings
	mergeInto(currentSettings, fileSettings);

	// write the file settings back
	Json::StyledWriter jsonWriter;
	std::string outStr = jsonWriter.write(fileSettings);
	GUID_LOG("saving settings file");
	bool result = write_file_utf8(QString::fromStdString(outStr), native(p));
	if (result)
	{
		GUID_LOG("settings file is saved");
	}
	else
	{
		GUID_LOG("settings file is NOT SAVED");
	}
	LOG("~GlobalSettings::save");
}

void GlobalSettings::syncGlobalValues( Json::Value& root, bool fromRoot)
{
	// value types can be exactly one of the following
	// Int, Bool, Double, String
	syncProperty(fromRoot, dropItemsIntoSceneOnStartup);

	syncProperty(fromRoot, cameraPreset);
	syncProperty(fromRoot, useRightClickMenuInvocation);
	syncProperty(fromRoot, enableCameraPanning);
	captureStatistics = false; syncProperty(fromRoot, captureStatistics); captureStatistics = false;
	syncProperty(fromRoot, topDownViewBufferSize);
	syncProperty(fromRoot, photoFrameImageDuration);
	syncProperty(fromRoot, photoFrameSourceDuration);
	syncProperty(fromRoot, shiftOn);
	syncProperty(fromRoot, maxLinearPileHeight);
	syncProperty(fromRoot, EndTossVelocitySq);
	syncProperty(fromRoot, MinStartTossVelocitySq);
	syncProperty(fromRoot, zDist);
	syncProperty(fromRoot, yDist);
	syncProperty(fromRoot, xDist);
	syncProperty(fromRoot, CAMWALLS_MAX_ANGLE);
	syncProperty(fromRoot, useAntiAliasing);
	syncProperty(fromRoot, useAnisotropicFiltering);
	syncProperty(fromRoot, repositionIconsIfOutsideWalls);
	syncProperty(fromRoot, hideInTaskBar);
	syncProperty(fromRoot, undoLevels);
	syncProperty(fromRoot, maxTextureSize);
	syncProperty(fromRoot, maxImageSize);
	syncProperty(fromRoot, manualArrowOverlay);
	syncProperty(fromRoot, showToolTips);
	syncProperty(fromRoot, enableHiResIcons);
	syncProperty(fromRoot, scaleIconBasedOnFileSize);
	syncProperty(fromRoot, scaleIconsBasedOnUsage);
	syncProperty(fromRoot, maxFileSizeForScale);
	syncProperty(fromRoot, minFileSizeThickness);
	syncProperty(fromRoot, maxFileSizeThickness);
	syncProperty(fromRoot, AxisAlignedMode);
	syncProperty(fromRoot, enableTouchGestureBrowse);
	syncProperty(fromRoot, showConsoleWindow);
	syncProperty(fromRoot, useThemeIconOverrides);
	syncProperty(fromRoot, camWallsEnabled);
	syncProperty(fromRoot, useCPUOptimizations);
	syncProperty(fromRoot, drawFramesPerSecond);
	syncProperty(fromRoot, useWindowsBackgroundImage);
	syncProperty(fromRoot, useThumbsDb);
	syncProperty(fromRoot, enableMultimouse);
	syncProperty(fromRoot, RenderText);
	syncProperty(fromRoot, LoadHiddenFiles);
	syncProperty(fromRoot, RotationLimitDegrees);
	syncProperty(fromRoot, enableDebugKeys);
	syncProperty(fromRoot, DrawOtherBumpTops);
	syncProperty(fromRoot, PrintMode);
	syncProperty(fromRoot, proxyMode);
	syncProperty(fromRoot, autoProxyConfigurationUrl);
	syncProperty(fromRoot, overrideIEProxySettings);
	syncProperty(fromRoot, httpProxyUrl);
	syncProperty(fromRoot, disableAnimationsOnBattery);
	syncProperty(fromRoot, disablePhotoframesOnBattery);
#ifdef HWMANFDEMOMODE
	syncProperty(fromRoot, hwManParticipantID);
#endif
	syncProperty(fromRoot, inviteCode);
	if (fromRoot)
		GUID_LOG(QString("loading guid as ") + qstring(root["guid"].asString()));
	else
		GUID_LOG(QString("setting guid as ") + guid);
	syncProperty(fromRoot, guid);
	GUID_LOG("guid is " + guid);
	syncProperty(fromRoot, authCode);
	syncProperty(fromRoot, monitor);
	monitor.replace("\\", "");
	monitor.replace(".", "");
	syncProperty(fromRoot, showIconExtensions);
	syncProperty(fromRoot, hasRunOnce);
	syncProperty(fromRoot, completedTutorial);
	syncProperty(fromRoot, visuals);
	syncProperty(fromRoot, proAuthCode);
	syncProperty(fromRoot, proInviteCode);
	syncProperty(fromRoot, launchFoldersAsInGrid);

	syncProperty(fromRoot, fbc_uid);	
	syncProperty(fromRoot, fbc_session);
	syncProperty(fromRoot, fbc_secret);

	syncProperty(fromRoot, tw_login);
	syncProperty(fromRoot, tw_password);
	syncProperty(fromRoot, tw_oauth_key);
	syncProperty(fromRoot, tw_oauth_secret);

	syncProperty(fromRoot, flickr_auth_token);

	syncProperty(fromRoot, scrollingGrowsItems);
	syncProperty(fromRoot, disableLibSquish);
	syncProperty(fromRoot, userEmail);
	syncProperty(fromRoot, enableTossing);
	syncProperty(fromRoot, languageOverride);

	syncProperty(fromRoot, currentLibrary);
	syncProperty(fromRoot, otherLibraries);
	syncProperty(fromRoot, enableLibraryOverlay);
}

void GlobalSettings::updateProxySettings()
{
	LOG("GlobalSettings::updateProxySettings");
	if (overrideIEProxySettings)
	{
		// user has settings that override the IE ones
		proxySettings.setProxyManually(httpProxyUrl);
	}
	else
	{
		// get the current proxy values
		proxySettings.copyProxySettingsFromIE();
		proxyMode = proxySettings.getProxyMode();
		autoProxyConfigurationUrl = proxySettings.getAutoProxyConfigurationURL();
		httpProxyUrl = proxySettings.getHttpProxyURL();

		if (proxyMode != prevProxyMode ||
			autoProxyConfigurationUrl != prevAutoProxyConfigurationUrl ||
			httpProxyUrl != prevHttpProxyUrl)
		{
			// check if these settings work
			if (!ftManager->hasInternetConnection())
			{
				// try switching to no proxy if necessary
				QString savedHttpProxyUrl = httpProxyUrl;
				httpProxyUrl = "";
				if (ftManager->hasInternetConnection())
				{
					proxySettings.errorLogWrite(QString("switching to no proxy, proxy URL ") + savedHttpProxyUrl + QString(" was not working"));
					proxyMode = NO_PROXY;
					overrideIEProxySettings = true;
				}
				else
				{
					httpProxyUrl = savedHttpProxyUrl;
				}
			}
			
			// save the previous values
			prevProxyMode = proxyMode;
			prevAutoProxyConfigurationUrl = autoProxyConfigurationUrl;
			prevHttpProxyUrl = httpProxyUrl;
		}
	}
	LOG("~GlobalSettings::updateProxySettings");
}