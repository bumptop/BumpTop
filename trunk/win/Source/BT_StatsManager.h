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

#ifndef BT_STATS_MANAGER
#define BT_STATS_MANAGER

// -----------------------------------------------------------------------------

#include "BT_SceneManager.h"	
#include "BT_FileTransferManager.h"
#include "BT_Settings.h"

// -----------------------------------------------------------------------------

// stats data
class StatsPersistentData
{
public: 
	StatsPersistentData();
	struct _bt_
	{
#ifdef HWMANFDEMOMODE
		int hwManParticipantID;
#endif
		string inviteCode;
		string GUID;
		string build;	
		// this is hashed (ascii chars)
		string workingDirectory;
		string bumptopEdition;
		string freeOrPro;

		int resetLayout;
		int crashes;

		int launchOnStartup;

		struct _window_
		{
			int instantiations;

			int activatedFromSystemTray;
			int activatedFromDoubleClick;
			int activatedFromClick;
			int activatedFromNonClick;
			int activatedFromShowDesktop;

			float activeTime;
			float focusedTime;
			float interactiveTime;
			float slideshowTime;
		} window;

		struct _settings_
		{
			int instantiations;
		} settings;

		struct _theme_
		{
			string version;
			string name;
			string url;
			string schema;
			int timesChanged;
		} theme;

		struct _shellExt_
		{
			// XXX: isn't this 1 per different stats file?
			int foldersBumped;
			int foldersUnbumped;

		} shellExt;

		struct _performance_
		{
			int averageFps;
			int maxAverageFps;
		} performance;

		struct _interaction_
		{
			struct _piles_
			{
				int created;		// including pilization
				// int pilesCreatedByLassoAndCross
				int destroyed;		// including folderization
				int folderized;
				int pilized;
				int gridView;
				int gridScrolled;
				int fanoutView;
				int pileByType;
				int leafed;
			} piles;

			struct _actors_
			{
				int pinned; 
				int unpinned;

				int addedToPile;		// explicitly? ie. via drag
				int removedFromPile;

				int launchedFile;
				int highlightedImage;

				struct _fs_types_
				{
					int folders;
					int images;
					int documents;
					int executables;
					int postitnotes;
					int postitnotesCreated;
					int webthumbnails;
					int webthumbnailsCreated;
				} fs_types;

				struct _custom_
				{
					struct _email_
					{
						int enabled;
						int tossedTo;
						int droppedOn;
						int launched;
					} email;

					struct _printer_
					{
						int enabled;
						int tossedTo;
						int droppedOn;
						int launched;
					} printer;

					struct _facebook_
					{
						int enabled;
						int tossedTo;
						int droppedOn;
						int launched;
					} facebook;

					struct _twitter_
					{
						int enabled;
						int tossedTo;
						int droppedOn;
						int launched;
					} twitter;

					struct _flickr_
					{
						int enabled;
						int tossedTo;
						int droppedOn;
						int launched;
					} flickr;
				} custom;
			} actors;

			struct _layout_
			{
				int laidOutInGrid;
			} layout;

			struct _clipboard_
			{
				int cut;
				int copy;
				int paste;
			} clipboard;

			struct _dragAndDrop_
			{
				int toExplorer;
				int fromExplorer;

				int toActor;
				int toPile;
			} dragAndDrop;

			struct _markingmenu_
			{
				int invokedByLasso;
				int invokedByClick;
				int executedCommandByClick;
				int executedCommandByHotkey;
			} markingmenu;

			struct _search_
			{
				int findAsYouType;
				int searchSubstring;
			} search;

		} interaction;

	} bt;

	struct _hardware_
	{
		// CPU Info
		string cpuNameString; // ie. AMD Turion(tm) 64 X2 Mobile Technology TL-60
		string cpuArch; // x86, x64, Itanium, etc
		string cpuType; // 386, 486, Pentium, X8664, IA64, etc
		uint cpuNumProcessors;
		string cpuFeatures; // MMX, SSE, 3DNOW, SSE2, etc
		uint cpuSpeed;

		// Windows Info
		uint osVersion; // 5=XP, 6=Vista.
		string osProductType;  // Home Premium, Ultimate, etc.
		uint osBuildNumber;
		string osServicePack;
		uint osSrvPckMajorVer;
		uint osSrvPckMinorVer;
		int isTabletPC;

		// Video Card
		string vidRenderer;

		// Physical Memory
		uint ramPhysicalTotal;
		uint ramPhysicalAvailable;
		uint ramVirtualTotal;
		uint ramVirtualAvailable;


	} hardware;
};

/*
-Capture for Top 10 interactions in the system.  
	- moving items, 
	- tossing itmes.  
	- How BT is brought to front (sysTray, Hotkey Start+D, click to focus).  		
	- Lasso selections made with how many # of objects.  
	- Find as you type.  
	- Ctrl+F. 
*/

class ThreadableUnit;

// Stats Manager Class
class StatsManager : public FileTransferEventHandler
{

	ThreadableUnit * _statsUploadingThread;
	QSemaphore _statsUploadingThreadSemaphore;
	bool _suspendUploading;

	// stats data
	StatsPersistentData _statsData;
	hash_map<int, StopwatchInSeconds> _timers;
	QString _statFileName;
	tm *_statFileDate;
	QString _statFileDirHash;
	QString _statFileGuid;
	const int _statsUploadCheckIntervalInMin;
	bool _reportedMultipleGuids;

	// Constructor
	friend class Singleton<StatsManager>;	

	// Hardware stats (CPU, OS, RAM, etc)
	void getHardwareStats();

	// filenames
	void updateStatFileName();
	void renameOldRecordingFiles();

	QString getScreenshotFileName();

	QString addRecordingSuffix(QString path);
	QString dropRecordingSuffix(QString path);

	// curl
	bool pUploadStatsThread();
	void uploadFiles(QString filter,  bool forceUpload=false, bool blocking=true);
	bool checkForMultipleGuids();

public:
	StatsManager();
	~StatsManager();

	// stats
	StatsPersistentData& getStats();

	// persistence
	void saveStats();
	void loadStats();
	QString getStatFileName();

	// uploading
	void clearScreenshots();
	void uploadScreenshots();
	void uploadStats();
	bool shouldUploadStats(bool& shouldTakeScreenshot);

	// screenshot
	bool hasScreenshot();
	QString getScreenshot();
	void takeScreenshot();
	
	// Starts the timer for specified persistent data value, using it's address as a 	 
	// key, accumulating the elapsed to the variable when the timer is finished.
	void startTimer(float& persistentData);
	void finishTimer(float& persistentData);

	void updateBumpTopStateStats();
	int getStatsUploadCheckIntervalInMin();

	// file transfer handler
	virtual void onTransferComplete(const FileTransfer& transfer);
	virtual void onTransferError(const FileTransfer& transfer);

	// system events
	void onPowerSuspend();
	void onPowerResume();
};

// -----------------------------------------------------------------------------

#define statsManager Singleton<StatsManager>::getInstance()

// -----------------------------------------------------------------------------

// Scoped stat timer
class ScopedStatsTimer
{
	float& _data;

public:
	ScopedStatsTimer(float& f)
	: _data(f)
	{
		if (GLOBAL(settings).captureStatistics)
			statsManager->startTimer(_data);
	}
	~ScopedStatsTimer()
	{
		if (GLOBAL(settings).captureStatistics)
			statsManager->finishTimer(_data);
	}
};

// -----------------------------------------------------------------------------

namespace boost {
	namespace serialization {
		template<class Archive>
		void serialize(Archive & arc, StatsPersistentData & d, const uint version)
		{
			// Be sure to add new variables to the *end*... otherwise stats will be misreported
			arc & BOOST_SERIALIZATION_NVP(d.bt.build);	
			arc & BOOST_SERIALIZATION_NVP(d.bt.workingDirectory);
			arc & BOOST_SERIALIZATION_NVP(d.bt.resetLayout);
			arc & BOOST_SERIALIZATION_NVP(d.bt.window.instantiations);
			arc & BOOST_SERIALIZATION_NVP(d.bt.window.activatedFromSystemTray);
			arc & BOOST_SERIALIZATION_NVP(d.bt.window.activatedFromDoubleClick);
			arc & BOOST_SERIALIZATION_NVP(d.bt.window.activeTime);
			arc & BOOST_SERIALIZATION_NVP(d.bt.window.focusedTime);
			arc & BOOST_SERIALIZATION_NVP(d.bt.window.interactiveTime);
			arc & BOOST_SERIALIZATION_NVP(d.bt.window.slideshowTime);

			arc & BOOST_SERIALIZATION_NVP(d.bt.shellExt.foldersBumped);
			arc & BOOST_SERIALIZATION_NVP(d.bt.shellExt.foldersUnbumped);

			arc & BOOST_SERIALIZATION_NVP(d.hardware.cpuNameString);
			arc & BOOST_SERIALIZATION_NVP(d.hardware.cpuArch);
			arc & BOOST_SERIALIZATION_NVP(d.hardware.cpuType);
			arc & BOOST_SERIALIZATION_NVP(d.hardware.cpuNumProcessors);
			arc & BOOST_SERIALIZATION_NVP(d.hardware.cpuFeatures);
			arc & BOOST_SERIALIZATION_NVP(d.hardware.cpuSpeed);
			arc & BOOST_SERIALIZATION_NVP(d.hardware.osVersion);
			arc & BOOST_SERIALIZATION_NVP(d.hardware.osBuildNumber);
			arc & BOOST_SERIALIZATION_NVP(d.hardware.osServicePack);
			arc & BOOST_SERIALIZATION_NVP(d.hardware.osSrvPckMajorVer);
			arc & BOOST_SERIALIZATION_NVP(d.hardware.osSrvPckMinorVer);
			arc & BOOST_SERIALIZATION_NVP(d.hardware.vidRenderer);
			arc & BOOST_SERIALIZATION_NVP(d.hardware.ramPhysicalTotal);
			arc & BOOST_SERIALIZATION_NVP(d.hardware.ramPhysicalAvailable);
			arc & BOOST_SERIALIZATION_NVP(d.hardware.ramVirtualTotal);
			arc & BOOST_SERIALIZATION_NVP(d.hardware.ramVirtualAvailable);

			arc & BOOST_SERIALIZATION_NVP(d.bt.interaction.piles.created);
			arc & BOOST_SERIALIZATION_NVP(d.bt.interaction.piles.destroyed);
			arc & BOOST_SERIALIZATION_NVP(d.bt.interaction.piles.folderized);
			arc & BOOST_SERIALIZATION_NVP(d.bt.interaction.piles.pilized);
			arc & BOOST_SERIALIZATION_NVP(d.bt.interaction.piles.gridView);
			arc & BOOST_SERIALIZATION_NVP(d.bt.interaction.piles.fanoutView);

			arc & BOOST_SERIALIZATION_NVP(d.bt.interaction.actors.pinned); 
			arc & BOOST_SERIALIZATION_NVP(d.bt.interaction.actors.unpinned);
			arc & BOOST_SERIALIZATION_NVP(d.bt.interaction.actors.addedToPile);
			arc & BOOST_SERIALIZATION_NVP(d.bt.interaction.actors.removedFromPile);
			arc & BOOST_SERIALIZATION_NVP(d.bt.interaction.actors.launchedFile);
			arc & BOOST_SERIALIZATION_NVP(d.bt.interaction.actors.highlightedImage);

			arc & BOOST_SERIALIZATION_NVP(d.bt.interaction.actors.fs_types.folders);
			arc & BOOST_SERIALIZATION_NVP(d.bt.interaction.actors.fs_types.images);
			arc & BOOST_SERIALIZATION_NVP(d.bt.interaction.actors.fs_types.documents);
			arc & BOOST_SERIALIZATION_NVP(d.bt.interaction.actors.fs_types.executables);

			arc & BOOST_SERIALIZATION_NVP(d.bt.interaction.dragAndDrop.toExplorer);
			arc & BOOST_SERIALIZATION_NVP(d.bt.interaction.dragAndDrop.fromExplorer);
			arc & BOOST_SERIALIZATION_NVP(d.bt.interaction.dragAndDrop.toActor);
			arc & BOOST_SERIALIZATION_NVP(d.bt.interaction.dragAndDrop.toPile);

			arc & BOOST_SERIALIZATION_NVP(d.bt.interaction.markingmenu.invokedByLasso);
			arc & BOOST_SERIALIZATION_NVP(d.bt.interaction.markingmenu.invokedByClick);
			arc & BOOST_SERIALIZATION_NVP(d.bt.interaction.markingmenu.executedCommandByClick);
			arc & BOOST_SERIALIZATION_NVP(d.bt.interaction.markingmenu.executedCommandByHotkey);

			// new variables... add to the end!
			arc & BOOST_SERIALIZATION_NVP(d.bt.interaction.piles.pileByType);

			arc & BOOST_SERIALIZATION_NVP(d.bt.interaction.actors.custom.email.enabled);
			arc & BOOST_SERIALIZATION_NVP(d.bt.interaction.actors.custom.email.tossedTo);
			arc & BOOST_SERIALIZATION_NVP(d.bt.interaction.actors.custom.email.droppedOn);
			arc & BOOST_SERIALIZATION_NVP(d.bt.interaction.actors.custom.email.launched);
			arc & BOOST_SERIALIZATION_NVP(d.bt.interaction.actors.custom.printer.enabled);
			arc & BOOST_SERIALIZATION_NVP(d.bt.interaction.actors.custom.printer.tossedTo);
			arc & BOOST_SERIALIZATION_NVP(d.bt.interaction.actors.custom.printer.droppedOn);
			arc & BOOST_SERIALIZATION_NVP(d.bt.interaction.actors.custom.printer.launched);
			arc & BOOST_SERIALIZATION_NVP(d.bt.interaction.layout.laidOutInGrid);
			arc & BOOST_SERIALIZATION_NVP(d.bt.interaction.clipboard.cut);
			arc & BOOST_SERIALIZATION_NVP(d.bt.interaction.clipboard.copy);
			arc & BOOST_SERIALIZATION_NVP(d.bt.interaction.clipboard.paste);

			arc & BOOST_SERIALIZATION_NVP(d.bt.window.activatedFromClick);
			arc & BOOST_SERIALIZATION_NVP(d.bt.window.activatedFromNonClick);
			arc & BOOST_SERIALIZATION_NVP(d.bt.window.activatedFromShowDesktop);

			arc & BOOST_SERIALIZATION_NVP(d.bt.launchOnStartup);

			arc & BOOST_SERIALIZATION_NVP(d.bt.interaction.piles.gridScrolled);
			arc & BOOST_SERIALIZATION_NVP(d.bt.interaction.piles.leafed);
			arc & BOOST_SERIALIZATION_NVP(d.bt.theme.version);
			arc & BOOST_SERIALIZATION_NVP(d.bt.theme.name);
			arc & BOOST_SERIALIZATION_NVP(d.bt.theme.url);
			arc & BOOST_SERIALIZATION_NVP(d.bt.theme.schema);

			arc & BOOST_SERIALIZATION_NVP(d.bt.crashes);

			arc & BOOST_SERIALIZATION_NVP(d.bt.performance.averageFps);
			arc & BOOST_SERIALIZATION_NVP(d.bt.performance.maxAverageFps);

			arc & BOOST_SERIALIZATION_NVP(d.hardware.isTabletPC);
			
			arc & BOOST_SERIALIZATION_NVP(d.bt.interaction.search.findAsYouType);
			arc & BOOST_SERIALIZATION_NVP(d.bt.interaction.search.searchSubstring);
			// add new variables to the end!
#ifdef HWMANFDEMOMODE
			arc & BOOST_SERIALIZATION_NVP(d.bt.hwManParticipantID);
#endif

			arc & BOOST_SERIALIZATION_NVP(d.bt.interaction.actors.fs_types.postitnotes);
			arc & BOOST_SERIALIZATION_NVP(d.bt.interaction.actors.fs_types.postitnotesCreated);
			arc & BOOST_SERIALIZATION_NVP(d.bt.settings.instantiations);
			arc & BOOST_SERIALIZATION_NVP(d.bt.theme.timesChanged);
			arc & BOOST_SERIALIZATION_NVP(d.bt.inviteCode);
			arc & BOOST_SERIALIZATION_NVP(d.bt.interaction.actors.fs_types.webthumbnails);
			arc & BOOST_SERIALIZATION_NVP(d.bt.interaction.actors.fs_types.webthumbnailsCreated);

			arc & BOOST_SERIALIZATION_NVP(d.bt.interaction.actors.custom.facebook.enabled);
			arc & BOOST_SERIALIZATION_NVP(d.bt.interaction.actors.custom.facebook.tossedTo);
			arc & BOOST_SERIALIZATION_NVP(d.bt.interaction.actors.custom.facebook.droppedOn);
			arc & BOOST_SERIALIZATION_NVP(d.bt.interaction.actors.custom.facebook.launched);

			arc & BOOST_SERIALIZATION_NVP(d.bt.interaction.actors.custom.twitter.enabled);
			arc & BOOST_SERIALIZATION_NVP(d.bt.interaction.actors.custom.twitter.tossedTo);
			arc & BOOST_SERIALIZATION_NVP(d.bt.interaction.actors.custom.twitter.droppedOn);
			arc & BOOST_SERIALIZATION_NVP(d.bt.interaction.actors.custom.twitter.launched);

			arc & BOOST_SERIALIZATION_NVP(d.bt.interaction.actors.custom.flickr.enabled);
			arc & BOOST_SERIALIZATION_NVP(d.bt.interaction.actors.custom.flickr.tossedTo);
			arc & BOOST_SERIALIZATION_NVP(d.bt.interaction.actors.custom.flickr.droppedOn);
			arc & BOOST_SERIALIZATION_NVP(d.bt.interaction.actors.custom.flickr.launched);
			arc & BOOST_SERIALIZATION_NVP(d.bt.bumptopEdition);
			arc & BOOST_SERIALIZATION_NVP(d.hardware.osProductType); // Home Premium, Ultimate, etc.
			arc & BOOST_SERIALIZATION_NVP(d.bt.GUID); // Install GUID to uniquely identify an install
			arc & BOOST_SERIALIZATION_NVP(d.bt.freeOrPro); // Free or Pro
		}
	}
}

#else
	// class StatsPersistentData;
	class StatsManager;
#endif