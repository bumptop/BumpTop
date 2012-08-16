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
#include "BT_CustomActor.h"
#include "BT_DialogManager.h"
#include "BT_FileSystemManager.h"
#include "BT_FileTransferManager.h"
#include "BT_GLTextureManager.h"
#include "BT_Logger.h"
#include "BT_SceneManager.h"
#include "BT_StatsManager.h"
#include "BT_SVNRevision.h"
#include "BT_SystemTray.h"
#include "BT_ThemeManager.h"
#include "BT_ThreadableUnit.h"
#include "BT_Util.h"
#include "BT_WindowsOS.h"


StatsManager::StatsManager()
: _statsUploadCheckIntervalInMin(30)
, _statsUploadingThread(NULL)
, _statsUploadingThreadSemaphore(1)
#ifdef DISABLE_PHONING
, _suspendUploading(true)
#else
, _suspendUploading(false)
#endif
{
	_reportedMultipleGuids = false;

	// clear the stats data
	ZeroMemory(&_statsData, sizeof(StatsPersistentData));

}

StatsManager::~StatsManager()
{
	SAFE_DELETE(_statsUploadingThread);
}

QString StatsManager::getStatFileName()
{
	if (_statFileName == "")
	{
		updateStatFileName();
	}
	return _statFileName;
}

QString StatsManager::addRecordingSuffix(QString path)
{
	return path + ".recording";
}

QString StatsManager::dropRecordingSuffix(QString path)
{
	return path.mid(0, path.size() - QString(".recording").size());
}

void StatsManager::updateStatFileName()
{
	_statFileGuid = winOS->GetGUID();
	QString wdDir = native(scnManager->getWorkingDirectory());
	_statFileDirHash = QString(QCryptographicHash::hash(wdDir.toUtf8(), QCryptographicHash::Md5).toHex());

	// return the name in the form 'stats_guid_dirhash_year_month_dayofmonth.xml'
	time_t rawtime;
	time(&rawtime);
	_statFileDate = localtime ( &rawtime );
	_statFileDate->tm_hour = _statFileDate->tm_min = _statFileDate->tm_sec = 0;

	QString statFileStream = QString("stats_%1_%2_%3_%4_%5.xml")
		.arg(_statFileGuid)
		.arg(_statFileDirHash)
		.arg(1900 + _statFileDate->tm_year)
		.arg(1 + _statFileDate->tm_mon)
		.arg(_statFileDate->tm_mday);
	_statFileName = addRecordingSuffix(statFileStream);
	return;
}

QString StatsManager::getScreenshotFileName()
{
	QString guid = winOS->GetGUID();
	QString wdDir = native(scnManager->getWorkingDirectory());
	QString dirHash(QCryptographicHash::hash(wdDir.toUtf8(), QCryptographicHash::Md5).toHex());

	// return the name in the form 'stats_guid_hash.xml'
	QString statFileStream = QString("screenshot_%1_%2.jpg")
		.arg(guid).arg(dirHash);
	return statFileStream;
}

StatsPersistentData& StatsManager::getStats()
{
	return _statsData;
}

void StatsManager::startTimer( float& persistentData )
{
	int addrKey = (int)&persistentData;

	// check if we are already timing this value, if so, then we should throw an error
	// XXX: currently we don't allow hierarchical timing
	hash_map<int, StopwatchInSeconds>::const_iterator iter = _timers.find(addrKey);
	if (iter == _timers.end())
	{
		// start the timer otherwise
		_timers[addrKey].restart();
	}
}

void StatsManager::finishTimer( float& persistentData )
{
	int addrKey = (int)&persistentData;

	// check if we were timing this value at all, if not, then throw an error
	hash_map<int, StopwatchInSeconds>::iterator iter = _timers.find(addrKey);
	if (iter != _timers.end())
	{
		// add the elapsed time to the persistent data
		persistentData += float(iter->second.elapsed());

		// remove the timer from the map
		_timers.erase(iter);	
	}
}

void StatsManager::updateBumpTopStateStats()
{
	
	getHardwareStats();

	// actor/pilec counts
	getStats().bt.interaction.actors.fs_types.folders = 0;
	getStats().bt.interaction.actors.fs_types.images = 0;
	getStats().bt.interaction.actors.fs_types.documents = 0;
	getStats().bt.interaction.actors.fs_types.executables = 0;
	getStats().bt.interaction.actors.fs_types.postitnotes = 0;
	const vector<BumpObject *>& bumpObjects = scnManager->getBumpObjects();
	for (int i = 0; i < bumpObjects.size(); ++i)
	{
		if (bumpObjects[i]->getObjectType() == ObjectType(BumpActor, FileSystem))
		{
			FileSystemActor * fsActor = (FileSystemActor *)bumpObjects[i];
			if (fsActor->isFileSystemType(Image))
				getStats().bt.interaction.actors.fs_types.images++;
			else if (fsActor->isFileSystemType(StickyNote))
				getStats().bt.interaction.actors.fs_types.postitnotes++;
			else if (fsActor->isFileSystemType(Folder))
				getStats().bt.interaction.actors.fs_types.folders++;
			else if (fsActor->isFileSystemType(Document))
				getStats().bt.interaction.actors.fs_types.documents++;
			else if (fsActor->isFileSystemType(Executable))
				getStats().bt.interaction.actors.fs_types.executables++;
			else if (fsActor->isFileSystemType(WebThumbnail))
				getStats().bt.interaction.actors.fs_types.webthumbnails++;

		}
	}

	// custom actors
	CustomActor * emailActor = scnManager->getCustomActor<EmailActorImpl>();
	CustomActor * printerActor = scnManager->getCustomActor<PrinterActorImpl>();
	CustomActor * facebookActor = scnManager->getCustomActor<FacebookActorImpl>();
	CustomActor * twitterActor = scnManager->getCustomActor<TwitterActorImpl>();
	getStats().bt.interaction.actors.custom.email.enabled = (emailActor != NULL) ? 1 : 0;
	getStats().bt.interaction.actors.custom.printer.enabled = (printerActor != NULL) ? 1 : 0;
	getStats().bt.interaction.actors.custom.facebook.enabled = (facebookActor != NULL) ? 1 : 0;
	getStats().bt.interaction.actors.custom.twitter.enabled = (twitterActor != NULL) ? 1 : 0;
	statsManager->getStats().bt.launchOnStartup = (int) sysTray->isLaunchingBumpTopOnStartup();

	// theme stats
	getStats().bt.theme.version = stdString(themeManager->getValueAsQString("header.version",""));
	getStats().bt.theme.name = stdString(themeManager->getValueAsQString("header.name",""));
	getStats().bt.theme.url = stdString(themeManager->getValueAsQString("header.url",""));
	getStats().bt.theme.schema = stdString(themeManager->getValueAsQString("header.schema",""));

#ifdef HWMANFDEMOMODE
	getStats().bt.hwManParticipantID = GLOBAL(settings).hwManParticipantID;
#endif

	getStats().bt.inviteCode = stdString(GLOBAL(settings).inviteCode);
	getStats().bt.bumptopEdition = stdString(winOS->BumpTopEditionName(winOS->GetBumpTopEdition()));
	
	finishTimer(statsManager->getStats().bt.window.activeTime);
	startTimer(statsManager->getStats().bt.window.activeTime);
}

// drops the .recording suffix from any .recording file that is more than 2 days old, or
// any .recording file with the same guid and dirhash as this instance of bumptop
void StatsManager::renameOldRecordingFiles()
{
	getStatFileName(); // need to initialize the stat file name, if it already hasn't happened

	if (exists(winOS->GetStatsDirectory()))
	{
		// rename all the old .recording files to .xml

		StrList allRecordingStatFiles = fsManager->getDirectoryContents(native(winOS->GetStatsDirectory()), "*.recording");

		for_each(QString statFile, allRecordingStatFiles)
		{
			QString statFileName = QFileInfo(statFile).fileName();

			QStringList parsedStatFileName;

			// parsing stat filename: 'stats_guid_dirhash_year_month_dayofmonth.xml.recording'
			// check if it ends with .xml.recording
			if (statFileName.endsWith(".xml.recording"))
				parsedStatFileName = statFileName.mid(0, statFileName.size() - QString(".xml.recording").size()).split("_");

			if (parsedStatFileName.size() == 6)
			{
				QString guid  = parsedStatFileName[1];
				QString dirhash = parsedStatFileName[2];
				tm timeinfo = { 0 };
				timeinfo.tm_year = parsedStatFileName[3].toInt();
				timeinfo.tm_year -= 1900;
				timeinfo.tm_mon = parsedStatFileName[4].toInt();
				timeinfo.tm_mon -= 1;
				timeinfo.tm_mday = parsedStatFileName[5].toInt();
				timeinfo.tm_isdst = -1;


				time_t stat_file_date = mktime(&timeinfo);
				time_t current_stat_file_date = mktime(_statFileDate);

				time_t now;
				time(&now);


				//    stats file is not from today AND...
				if (current_stat_file_date != stat_file_date && 
				//     1. this belongs to our instance
					((_statFileGuid == guid && _statFileDirHash == dirhash) ||
				// or  2. we've passed the upload interval time (to be sure we're not renaming another 
				//        bumptop instance's file while it's running)
					 difftime(now, stat_file_date) >= (getStatsUploadCheckIntervalInMin() + 1) * 60.0))
				{
					QFile p(statFile);
					p.rename(native(make_file(winOS->GetStatsDirectory(), dropRecordingSuffix(statFileName))));
				}
			}
			else
			{
				QFile::remove(statFile);
			}
		}

	}
}

int StatsManager::getStatsUploadCheckIntervalInMin()
{
	return _statsUploadCheckIntervalInMin;
}


void StatsManager::saveStats()
{
	if (GLOBAL(settings).captureStatistics)
	{
		statsManager->updateBumpTopStateStats();

		QFileInfo statsFilePath = make_file(winOS->GetStatsDirectory(), getStatFileName());
		std::wofstream ofs(native(statsFilePath).utf16());
		if (ofs.good())
		{
			xml_woarchive oa(ofs);
			oa << BOOST_SERIALIZATION_NVP(_statsData);
		}
	}

	updateStatFileName();

	renameOldRecordingFiles();
}

void StatsManager::loadStats()
{
	if (GLOBAL(settings).captureStatistics)
	{
		try
		{
			renameOldRecordingFiles();

			QFileInfo statsFilePath = make_file(winOS->GetStatsDirectory(), getStatFileName());
			if (exists(statsFilePath))
			{
				// load the stats file if it exists
				wifstream ifs(native(statsFilePath).utf16());
				if (ifs.good())
				{
					xml_wiarchive ia(ifs);
					ia >> BOOST_SERIALIZATION_NVP(_statsData);
				}
			}
		}
		catch(...)
		{
			// XXX: we may fail if we try an old stats file, which shouldn't occur at all if we are 
			//      sending stats information with each update (which invalidates the old stats file
			//		if new metrics are added).  So we ignore it for now.
		}
	}
}

void StatsManager::takeScreenshot()
{
	if (_suspendUploading)
		return;
	if (GLOBAL(settings).captureStatistics)
	{
		QString screenshotFileJPG = getScreenshotFileName();
		texMgr->takeScreenshot(native(make_file(winOS->GetStatsDirectory(), screenshotFileJPG)));
	}
}

QString StatsManager::getScreenshot()
{
	QFileInfo screenShotPath = make_file(winOS->GetStatsDirectory(), getScreenshotFileName());
	return native(screenShotPath);
}

bool StatsManager::hasScreenshot()
{
	if (!GLOBAL(settings).captureStatistics)
		return false;

	// return whether we already have a screenshot taken?
	QFileInfo screenShotPath = make_file(winOS->GetStatsDirectory(), getScreenshotFileName());
	return exists(screenShotPath);
}

size_t readLocalFile(char *bufptr, size_t size, size_t nitems, void *userp)
{	
	return fread(bufptr, size, nitems, (FILE *) userp);
}

void StatsManager::uploadFiles( QString filter, bool forceUpload, bool blocking )
{
	if (_suspendUploading)
		return;
	if (!GLOBAL(settings).captureStatistics)
		return;

	// iterate the listing and upload files as necessary
	QString statsDir = native(winOS->GetStatsDirectory());
	vector<QString> statsDirListing = fsManager->getDirectoryContents(statsDir);
	for (int i = 0; i < statsDirListing.size(); ++i)
	{
		// upload the file
		QString localPath = statsDirListing[i];
		QString fileName = QFileInfo(localPath).fileName();
		QString fileNameWithoutExt = fileName.mid(0, fileName.lastIndexOf("."));
		QString fileNameExt = fileName.mid(fileName.lastIndexOf(".")+1);
		if (fileNameExt == filter)
		{
			QString remoteFile = QString("%1_%2.%3").arg(fileNameWithoutExt).arg(time(NULL)).arg(fileNameExt);

			// set the params
			QString fileName = QFileInfo(localPath).fileName();
			QSet<QString> fileParamKeys;
				fileParamKeys.insert("userfile");
			QHash<QString, QString> params;
				params.insert("userfile", localPath);
				params.insert("submit", "send");

			// transfer the stats file
#ifdef HWMANFDEMOMODE
			ftManager->addPostTransfer(
				FileTransfer(FileTransfer::Upload, this)
				.setRelativeUrl(QString("http://%1.stats.bumptop.com/stats_upload/upload_hwmanf.php").arg(SVN_VERSION_NUMBER), remoteFile.str())
				.setParams(params)
				.setFileParamKeys(fileParamKeys)
				.setLogin("bumptop_stats_upload")
				.setPassword("free57love")
				.setTimeout(30)
				.setTemporaryString(), blocking
				);
#else
				QString destination = QString("http://%1.stats.bumptop.com/stats_upload/upload.php").arg(SVN_VERSION_NUMBER);
			ftManager->addPostTransfer(
				FileTransfer(FileTransfer::Upload, this)
				.setUrl(destination)
				.setParams(params)
				.setFileParamKeys(fileParamKeys)
				.setLogin("bumptop_stats_upload")
				.setPassword("free57love")
				.setTimeout(30)
				.setTemporaryString(), blocking
				);
#endif 
		}
	}
}

bool StatsManager::pUploadStatsThread()
{	
	// just upload the stats and return
	if (ftManager->hasInternetConnection())
		uploadFiles("xml");
	_statsUploadingThreadSemaphore.release();
	return false;
}

void StatsManager::uploadStats()
{
	if (_suspendUploading)
		return;
	/*
	// try to detect the multiple GUID problem
	if (!_reportedMultipleGuids && checkForMultipleGuids())
	{
		// present a dialog box?
		// show dialog box
		dlgManager->clearState();
		dlgManager->setCaption("BumpTop Reporter");
		dlgManager->setPrompt("Oops! BumpTop has noticed a serious non-fatal error, and will send information to BumpTop HQ.");
		
		if (dlgManager->promptDialog(DialogOK))
		{
			dlgManager->setPrompt("Please enter your email so BumpTop can follow-up on the error:");

			string email = winOS->getRegistryStringValue("UserEmail");
			dlgManager->setText(email);

			if (dlgManager->promptDialog(DialogInput))
			{
				string filepath = guidLogger->getFilepath();

				email = dlgManager->getText();
				// save the email address for the future if one is provided
				if (!email.empty())
					winOS->setRegistryStringValue("UserEmail", email);

				guidLogger->closeLog(); // we can't send it if we have an open write handle on it
				if (!EmailCrashReport("feedback@bumptop.com", "Multiple GUIDS found", "", email,  guidLogger->getFilepath()))
				{
					dlgManager->setPrompt("An error occurred while trying to send your feedback.\nPlease alert BumpTop at feedback@bumptop.com");
					dlgManager->promptDialog(DialogOK);

				}
			}
		}
	}
	*/

	// only allow one background upload thread to run at a time
	if (_statsUploadingThreadSemaphore.tryAcquire())
	{
		// upload the stats on a separate thread
		if (!_statsUploadingThread) 
		{
			_statsUploadingThread = new ThreadableUnit;
			_statsUploadingThread->run(boost::bind(&StatsManager::pUploadStatsThread, this), ThreadableUnit::StatsManagerUploadThread);
		}
		else
			_statsUploadingThread->reRun();
	}
}

void StatsManager::clearScreenshots()
{
	if (!GLOBAL(settings).captureStatistics)
		return;

	// iterate the listing and remove the screenshots
	QString statsDir = native(winOS->GetStatsDirectory());
	vector<QString> statsDirListing = fsManager->getDirectoryContents(statsDir);
	for (int i = 0; i < statsDirListing.size(); ++i)
	{
		// upload the file
		QFileInfo localPath(statsDirListing[i]);
		QString fileName = localPath.fileName();
		QString fileNameWithoutExt = fileName.mid(0, fileName.lastIndexOf("."));
		QString fileNameExt = fileName.mid(fileName.lastIndexOf(".")+1);
		if (fileNameExt == "jpg")
		{
			QFile::remove(native(localPath));
		}
	}
}

void StatsManager::uploadScreenshots()
{
	if (_suspendUploading)
		return;
	uploadFiles("jpg", true, true);
}

bool StatsManager::shouldUploadStats(bool& shouldTakeScreenshot)
{
	if (_suspendUploading)
		return false;

	if (!GLOBAL(settings).captureStatistics)
	{
		shouldTakeScreenshot = false;
		return false;
	}

	// determine if today is screenshot day (every three days) and whether we already have one or not
	shouldTakeScreenshot = !hasScreenshot();

	// if we have XML files, we should upload them

	StrList allXmlFiles = fsManager->getDirectoryContents(native(winOS->GetStatsDirectory()), "*.xml");
	return allXmlFiles.size() > 0;
}


void StatsManager::getHardwareStats()
{
	TCHAR lszValue[MAX_PATH + 1];
	HKEY hKey;
	DWORD dwTypeStr = REG_SZ, dwTypeDWORD = REG_DWORD;
	DWORD dwSize = MAX_PATH;
	OSVERSIONINFOEX osVer = {0};
	SYSTEM_INFO sysInfo = {0};
	string windowsType;
	DWORD dwType;

	uint dwSizeUint = sizeof(uint);
	MEMORYSTATUS memStatus = {0};

	// Set the size of the structure as per MSDN
	osVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	memStatus.dwLength = sizeof(MEMORYSTATUS);

	// Retrieve info about the Hardware and OS
	GlobalMemoryStatus((LPMEMORYSTATUS) &memStatus);

	typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);
	typedef BOOL (WINAPI *PGPI)(DWORD, DWORD, DWORD, DWORD, PDWORD);

	PGNSI pGNSI = (PGNSI) GetProcAddress(
		GetModuleHandle(TEXT("kernel32.dll")), 
		"GetNativeSystemInfo");
	if(NULL != pGNSI)
		pGNSI(&sysInfo);
	else
		GetSystemInfo(&sysInfo);

	GetVersionEx((LPOSVERSIONINFO) &osVer);

	// Processor Architecture
	switch (sysInfo.wProcessorArchitecture)
	{
		case PROCESSOR_ARCHITECTURE_AMD64: _statsData.hardware.cpuArch = "AMD64"; break;
		case PROCESSOR_ARCHITECTURE_IA64: _statsData.hardware.cpuArch = "IA64"; break;
		case PROCESSOR_ARCHITECTURE_INTEL: _statsData.hardware.cpuArch = "x86"; break;
		case PROCESSOR_ARCHITECTURE_UNKNOWN: _statsData.hardware.cpuArch = "Unknown"; break;
	}


	// Processor Brand
	switch (sysInfo.dwProcessorType)
	{
		case PROCESSOR_INTEL_386: _statsData.hardware.cpuType = "386"; break;
		case PROCESSOR_INTEL_486: _statsData.hardware.cpuType = "486"; break;
		case PROCESSOR_INTEL_PENTIUM: _statsData.hardware.cpuType = "Pentium"; break;
		case PROCESSOR_INTEL_IA64: _statsData.hardware.cpuType = "IA64"; break;
		case PROCESSOR_AMD_X8664: _statsData.hardware.cpuType = "AMDx64"; break;
	}

	// Identify OS product type


	typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);
	typedef BOOL (WINAPI *PGPI)(DWORD, DWORD, DWORD, DWORD, PDWORD);

	if ( osVer.dwMajorVersion == 6 && osVer.dwMinorVersion == 0 )
	{
		if( osVer.wProductType == VER_NT_WORKSTATION )
			windowsType += "Windows Vista ";
		else windowsType += "Windows Server 2008 ";

		PGPI pGPI = (PGPI) GetProcAddress(
			GetModuleHandle(TEXT("kernel32.dll")), 
			"GetProductInfo");

		if (pGPI)
		{
			pGPI( 6, 0, 0, 0, &dwType);

			switch( dwType )
			{
			case PRODUCT_ULTIMATE:
				windowsType += "Ultimate Edition";
				break;
			case PRODUCT_HOME_PREMIUM:
				windowsType += "Home Premium Edition";
				break;
			case PRODUCT_HOME_BASIC:
				windowsType += "Home Basic Edition";
				break;
			case PRODUCT_ENTERPRISE:
				windowsType += "Enterprise Edition";
				break;
			case PRODUCT_BUSINESS:
				windowsType += "Business Edition";
				break;
			case PRODUCT_STARTER:
				windowsType += "Starter Edition";
				break;
			case PRODUCT_CLUSTER_SERVER:
				windowsType += "Cluster Server Edition";
				break;
			case PRODUCT_DATACENTER_SERVER:
				windowsType += "Datacenter Edition";
				break;
			case PRODUCT_DATACENTER_SERVER_CORE:
				windowsType += "Datacenter Edition (core installation)";
				break;
			case PRODUCT_ENTERPRISE_SERVER:
				windowsType += "Enterprise Edition";
				break;
			case PRODUCT_ENTERPRISE_SERVER_CORE:
				windowsType += "Enterprise Edition (core installation)";
				break;
			case PRODUCT_ENTERPRISE_SERVER_IA64:
				windowsType += "Enterprise Edition for Itanium-based Systems";
				break;
			case PRODUCT_SMALLBUSINESS_SERVER:
				windowsType += "Small Business Server";
				break;
			case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM:
				windowsType += "Small Business Server Premium Edition";
				break;
			case PRODUCT_STANDARD_SERVER:
				windowsType += "Standard Edition";
				break;
			case PRODUCT_STANDARD_SERVER_CORE:
				windowsType += "Standard Edition (core installation)";
				break;
			case PRODUCT_WEB_SERVER:
				windowsType += "Web Server Edition";
				break;
			}
			_statsData.hardware.osProductType = windowsType;
		}
	}


	// Number of Processors
	_statsData.hardware.cpuNumProcessors = sysInfo.dwNumberOfProcessors;

	// CPU Features
	_statsData.hardware.cpuFeatures = "";
	_statsData.hardware.cpuFeatures.append(IsProcessorFeaturePresent(PF_3DNOW_INSTRUCTIONS_AVAILABLE) ? "3DNOW " : "");
	_statsData.hardware.cpuFeatures.append(IsProcessorFeaturePresent(PF_MMX_INSTRUCTIONS_AVAILABLE) ? "MMX " : "");
	_statsData.hardware.cpuFeatures.append(IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE) ? "SSE2 " : "");
	_statsData.hardware.cpuFeatures.append(IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE) ? "SSE " : "");

	// OS Version
	_statsData.hardware.osVersion = osVer.dwMajorVersion;
	_statsData.hardware.osBuildNumber = osVer.dwBuildNumber;
	_statsData.hardware.osServicePack = stdString(QString::fromUtf16((const ushort *) osVer.szCSDVersion));
	_statsData.hardware.osSrvPckMajorVer = osVer.wServicePackMajor;
	_statsData.hardware.osSrvPckMinorVer = osVer.wServicePackMinor;
	_statsData.hardware.isTabletPC = GetSystemMetrics(SM_TABLETPC);

	// Read the registry for more CPU values
	if (SUCCEEDED(RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", NULL, KEY_READ, &hKey)))
	{
		// Get the name of the processor
		memset(lszValue, NULL, dwSize);
		if (SUCCEEDED(RegQueryValueEx(hKey, L"ProcessorNameString", NULL, &dwTypeStr, (LPBYTE) &lszValue, &dwSize)))
		{
			_statsData.hardware.cpuNameString = stdString(QString::fromUtf16((const ushort *) lszValue));
		}

		// Get the Processor Speed
		memset(lszValue, NULL, dwSize);
		RegQueryValueEx(hKey, L"~MHZ", NULL, &dwTypeDWORD, (LPBYTE) &_statsData.hardware.cpuSpeed, (LPDWORD) &dwSizeUint);
		RegCloseKey(hKey);
	}

	// Physical and Virtual Memory
	_statsData.hardware.ramPhysicalTotal = memStatus.dwTotalPhys;
	_statsData.hardware.ramPhysicalAvailable = memStatus.dwAvailPhys;
	_statsData.hardware.ramVirtualTotal = memStatus.dwTotalVirtual;
	_statsData.hardware.ramVirtualAvailable = memStatus.dwAvailVirtual;
	
	_statsData.bt.freeOrPro = (GLOBAL(settings).freeOrProLevel == AL_PRO) ? "Pro" : "Free";
	
	_statsData.bt.performance.averageFps = GLOBAL(frameCounter) ? GLOBAL(avgFramesPerSecond) / GLOBAL(frameCounter) : 0;
	_statsData.bt.performance.maxAverageFps = GLOBAL(maxFramesPerSecond);
#ifdef HWMANFDEMOMODE
	_statsData.bt.hwManParticipantID = GLOBAL(settings).hwManParticipantID;
#endif
	
	_statsData.bt.GUID = stdString(winOS->GetGUID()); // uniquely identify an installation
	_statsData.bt.inviteCode = stdString(GLOBAL(settings).inviteCode);
}

void StatsManager::onTransferError(const FileTransfer& transfer)
{
	// do nothing, just leave the file so that we can re-upload it later
}

void StatsManager::onTransferComplete(const FileTransfer& transfer)
{
	QString serverReturnString = transfer.getTemporaryString();

	if (serverReturnString.contains("uploaded successfully"))
	{
		// remove the file
		if (!transfer.getLocalPath().isEmpty())
		{
			QFile filePath(transfer.getLocalPath());
			if (exists(native(filePath)))
				filePath.remove();
		}

		// remove all the stats file params
		QSet<QString> fileParamKeys = transfer.getFileParamKeys();
		QHash<QString, QString> params = transfer.getParams();
		QSetIterator<QString> iter(fileParamKeys);
		while (iter.hasNext())
		{
			QString key = iter.next();
			QFile filePath(params[key]);
			if (exists(native(filePath)))
			{
				filePath.remove();
			}
		}
	}
}

bool StatsManager::checkForMultipleGuids()
{
	QString guid = "";

	QString statsDir = native(winOS->GetStatsDirectory());
	vector<QString> statsDirListing = fsManager->getDirectoryContents(statsDir, "stats_*");
	for (int i = 0; i < statsDirListing.size(); ++i)
	{
		// upload the file
		QString localPath = statsDirListing[i];
		QString fileName = QFileInfo(localPath).fileName();
		if (fileName.size() > 44)
		{
			QString a_guid = fileName.mid(7, 36);
			if (guid == "") guid = a_guid;
			if (a_guid != guid)
			{
				GUID_LOG("found more than one GUID! " + guid + " and " + a_guid);
				for (int j = 0; j < statsDirListing.size(); ++j)
				{
					GUID_LOG("\t" + statsDirListing[j]);
				}
				return true;
			}
		}
	}
	return false;
}

void StatsManager::onPowerSuspend()
{
	_suspendUploading = true;

	// join the stats uploading thread and release the semaphore
	if (_statsUploadingThread)
	{
		_statsUploadingThread->join(1024);
		_statsUploadingThreadSemaphore.release(_statsUploadingThreadSemaphore.available());
	}
}

void StatsManager::onPowerResume()
{
#ifndef DISABLE_PHONING
	_suspendUploading = false;
#endif
}

StatsPersistentData::StatsPersistentData()
{
	ZeroMemory(&bt, sizeof(bt));
	ZeroMemory(&hardware, sizeof(hardware));
}