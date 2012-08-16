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

#ifndef _FILE_SYSTEM_MANAGER_
#define _FILE_SYSTEM_MANAGER_

// -----------------------------------------------------------------------------

#include "BT_Singleton.h"
#include "BT_Watchable.h"
#include "BT_WatchableEvent.h"
#include "BT_FileSystemActor.h"
#include "DirectoryChanges.h"

// -----------------------------------------------------------------------------

enum FileAttributes
{
	NonExistent	= 0x00000000,
	ReadOnly	= 0x00000001,
	Hidden		= 0x00000002,
	System		= 0x00000004,
	Directory	= 0x00000010,  
	Archive		= 0x00000020,  
	Device		= 0x00000040,  
	Normal		= 0x00000080,  
	TempOnly	= 0x00000100,
	SparseFile	= 0x00000200,  
	ReparsePoint= 0x00000400,  
	Compressed	= 0x00000800,  
	Offline		= 0x00001000,  
	NotIndexed	= 0x00002000,  
	Encrypted	= 0x00004000  
};

// -----------------------------------------------------------------------------

struct FileSystemActorRename
{
	FileSystemActor* actor;
	QString newFileName;
};

// -----------------------------------------------------------------------------

class FileSystemManager : public CDirectoryChangeHandler
{
	Q_DECLARE_TR_FUNCTIONS(FileSystemManager)

	DirectoryChangeWatcher	dirWatcher;
	vector<Watchable *>		listenerList;
	vector<WatchableEvent>	eventList;
	QMutex _memberMutex;

	// msi-advertised shortcuts
	HMODULE _hMSIDll;
	typedef UINT (WINAPI *PFN_MsiGetShortcutTarget)(LPCTSTR szShortcutTarget, LPTSTR szProductCode, LPTSTR szFeatureId, LPTSTR szComponentCode);
	typedef UINT (WINAPI *PFN_MsiGetComponentPath)(LPCTSTR szProduct, LPCTSTR szComponent, LPTSTR lpPathBuf, DWORD* pcchBuf);
	PFN_MsiGetShortcutTarget pfnMsiGetShortcutTarget;
	PFN_MsiGetComponentPath pfnMsiGetComponentPath;

	// private functions
	time_t		fileTimeToUnixTime(const FILETIME& filetime);

	// OS specific functions
	bool		moveFilesVista(vector<FileSystemActor *> &objList, QString destDir, vector<FileSystemActor*>& failedObj, vector<FileSystemActor*>& replacedObj, vector<FileSystemActorRename>& renamedObj);
	bool		moveFilesXP(vector<FileSystemActor *> &objList, QString& destDir, vector<FileSystemActor *> &failedObj, vector<FileSystemActor*>& replacedObj);


	// Singleton
	friend class Singleton<FileSystemManager>;
	FileSystemManager();

	TCHAR * allocateStringFromPathsArray(const vector<FileSystemActor *>& objList);
	TCHAR * allocateStringFromPathsArray(const vector<QString>& filePaths);

public:

	~FileSystemManager();

	// Actions
	bool		addObject(Watchable *listener);
	bool		removeObject(Watchable *listener);
	void		update();
	bool		deleteFiles(vector<FileSystemActor *> objList, vector<FileSystemActor *> &failedObj, bool confirm = false, bool skipRecycleBin = false);
	bool		deleteFileByName(QString filePath, bool silent=false, bool skipRecycleBin = false, bool confirm = false);
	bool		renameFile(FileSystemActor *obj, QString newName, bool confirm = false);
	bool		renameFile(QDir oldPath, QString newName, bool confirm = false);
	bool		moveFiles(vector<FileSystemActor *> &objList, QString destDir, vector<FileSystemActor *> &failedObj, bool confirm = false);
	bool		moveFiles(vector<FileSystemActor *> &objList, QString destDir, vector<FileSystemActor *> &failedObj, vector<FileSystemActor*>& replacedObj, vector<FileSystemActorRename>& renamedObj, bool confirm = false);
	
	bool		moveFiles(const vector<QString>& filePaths, QString destDir);
	bool		copyFiles(vector<FileSystemActor *> &objList, QString destDir, vector<FileSystemActor *> &failedObj, bool confirm = false);
	bool		copyFileByName(QString oldPath, QString destDir, QString newFileName, bool confirm = false, bool silent = false, bool allowUndo = true);
	bool		createFile(QString filePath);
	bool		createDirectory(QString dirPath);
	void		createShortcut(QString shortcutFilePath, QString pathToTarget, QString desc);
	bool		launchFile(QString filePath, QString arguments="", QString workingDir="", bool runAsElevated = false, bool quoteArgs = true, bool async = false);
	bool		launchFileAsync(QString filePath, QString arguments="", QString workingDir="", bool runAsElevated = false, bool quoteArgs = true);
	bool		launchFile(QString filePath, LPWSTR verb);
	bool		launchFile(SHELLEXECUTEINFO sei, QString filePath);

	// Events
	void		onFileAdded(LPCWSTR strFileName);
	void		onFileRemoved(LPCWSTR strFileName);
	void		onFileNameChanged(LPCWSTR strOldFileName, LPCWSTR strNewFileName);
	void		onFileModified(LPCWSTR strFileName);
	void		onPowerSuspend();
	void		onPowerResume();

	// Getters
	StrList		getDirectoryContents(QString dirPath, QString filter = "*");
	StrList		getWorkingDirectoryContents(QString filter = "*");
	bool		getShortcutTarget(QString shortcutFileName, QString * targetOut=NULL, QString * argsOut=NULL, QString * workingDirOut=NULL);
	uint		getFileAttributes(QString fileName);
	QString		getFileExtension(QString filePath);
	uint		getFileSize(QString filePath);
	time_t		getFileCreationTime(QString filePath);
	Watchable	*getWatchObjFromPath(QString filePath);
	bool		isDirectoryWatched(QString dir);
	bool		isValidFileName(QString filePath);
	bool		hasCommonRoots(const FileSystemActor * actor, const vector<BumpObject *>& dropObjs) const;
	
	bool		resolveVolume(DEV_BROADCAST_HDR * data, QList<QString>& volumeOut);
	bool		resolveVolumeName(QString volume, QString& volumeNameOut);
	bool		isValidVolume(QString volume);
	bool		isVolumeADisc(QString volume);
	bool		isIdenticalPath(const QString& pathA, const QString& pathB);

	// Additional operations
	QFileInfo	getUniqueNewFilePathInWorkingDirectory(QString fileName);
	QDir		getUniqueNewFolderPathInWorkingDirectory(QString folderName);

	// Setters
	void		setFileAttributes(QString fileName, uint attribute);

};

// -----------------------------------------------------------------------------

#define fsManager Singleton<FileSystemManager>::getInstance()

// -----------------------------------------------------------------------------

#else
	class FileSystemManager;
	enum FileAttributes;
#endif
