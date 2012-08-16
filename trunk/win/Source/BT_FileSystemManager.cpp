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
#include "BT_FileSystemManager.h"
#include "BT_FileSystemPile.h"
#include "BT_DialogManager.h"
#include "BT_SceneManager.h"
#include "BT_OverlayComponent.h"
#include "BT_WindowsOS.h"
#include "BT_Util.h"
#include "BT_QtUtil.h"
#include "BT_WindowsHelper.h"
#include "BT_LibraryManager.h"

#ifdef UNICODE
	#define ANSI_UNICODE(ansi, unicode)  unicode
#else
	#define ANSI_UNICODE(ansi, unicode)  ansi
#endif

#define myGetProcAddress(hDLL, functionName) \
	((PFN_##functionName)GetProcAddress(hDLL, (#functionName ANSI_UNICODE("A", "W"))))

FileSystemManager::FileSystemManager()
: _hMSIDll(0)
, pfnMsiGetShortcutTarget(NULL)
, pfnMsiGetComponentPath(NULL)
{	
	_hMSIDll = LoadLibrary(L"Msi.dll");
	if (_hMSIDll)
	{
		pfnMsiGetShortcutTarget = myGetProcAddress(_hMSIDll, MsiGetShortcutTarget);
		pfnMsiGetComponentPath = myGetProcAddress(_hMSIDll, MsiGetComponentPath);
	}
}

FileSystemManager::~FileSystemManager()
{
	if (_hMSIDll)
		FreeLibrary(_hMSIDll);
}

TCHAR * FileSystemManager::allocateStringFromPathsArray(const vector<FileSystemActor *>& objList)
{
	// calculate how long the fromPaths string has to be (+2 initially for the final extra \0 char)
	int fromPathsSize = 1;
	for (int i = 0; i < objList.size(); ++i)
	{
		// (+1 for the \0 char after each path)
		fromPathsSize += ((FileSystemActor *) objList[i])->getFullPath().size() + 1;
	}
	
	TCHAR * fromPaths = new TCHAR[fromPathsSize];
	ZeroMemory(fromPaths, fromPathsSize * sizeof(TCHAR));
	return fromPaths;
}

TCHAR * FileSystemManager::allocateStringFromPathsArray(const vector<QString>& filePaths)
{
	// calculate how long the fromPaths string has to be (+2 initially for the final extra \0 char)
	int fromPathsSize = 1;
	for (int i = 0; i < filePaths.size(); ++i)
	{
		// (+1 for the \0 char after each path)
		fromPathsSize += filePaths[i].size() + 1;
	}

	TCHAR * fromPaths = new TCHAR[fromPathsSize];	
	ZeroMemory(fromPaths, fromPathsSize * sizeof(TCHAR));
	return fromPaths;
}

bool FileSystemManager::addObject(Watchable *listener)
{
	uint changeFilter = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_ATTRIBUTES;

	// Check if this item is not part of the listener list
	if (vecContains(listenerList, listener) == -1)
	{
		StrList watchDirs = listener->getWatchDir();

		// Add this directory to the watch list
		for (uint i = 0; i < watchDirs.size(); i++)
		{
			dirWatcher.WatchDirectory((LPCTSTR) watchDirs[i].utf16(), changeFilter, this);
		}

		listenerList.push_back(listener);

		return true;
	}

	// Item is already in the list
	return false;
}

bool FileSystemManager::removeObject(Watchable *listener)
{
	int indx = vecContains(listenerList, listener);

	if (indx != -1)
	{
		StrList watchDirs = listener->getWatchDir();

		// Remove it from the directory watcher
		for (uint i = 0; i < watchDirs.size(); i++)
			dirWatcher.UnwatchDirectory((LPCTSTR) watchDirs[i].utf16());

		// Kill off any events that belong to this directory
		for (int i = 0; i < eventList.size(); i++)
		{
			if (eventList[i].obj == listener)
			{
				eventList.erase(eventList.begin() + i);
				i--;
			}
		}

		// Remove the watchable item from our own list
		listenerList.erase(listenerList.begin() + indx);
		return true;
	}

	return false;
}

//Analogous to getUniqueNewFolderPathInWorkingDirectory, this function returns a unique file path using the parameter as a template
QFileInfo FileSystemManager::getUniqueNewFilePathInWorkingDirectory(QString fileName) {
	QFileInfo newFile;
	int file_extension_division = fileName.lastIndexOf(".");
	QString ext = fileName.mid(file_extension_division,fileName.size()-file_extension_division);
	QString fileBase = fileName.mid(0,file_extension_division);
	QString oldBase = fileBase;
	int x = 1;
	do
	{
		QString counter = QString::number(x);
		fileBase.append(counter);
		fileBase.append(ext);
		newFile = make_file(GLOBAL(getWorkingDirectory()), fileBase);
		fileBase = oldBase;
		x++;
	} 
	while (exists(newFile));

	return newFile;
}

QDir FileSystemManager::getUniqueNewFolderPathInWorkingDirectory(QString folderName)
{
	QDir piledStuffPath(scnManager->getWorkingDirectory());
	QString tempRelFolder(folderName);

	// Find a suitable Name for this new pile
	int i = 1;
	while (piledStuffPath.exists(tempRelFolder))
	{
		tempRelFolder = QString("%1 %2").arg(folderName).arg(i);
		i++;
	}
	return (piledStuffPath / tempRelFolder);
}

void FileSystemManager::update()
{
	if (_memberMutex.tryLock())
	{
		// filter out duplicates
		QList<WatchableEvent> uniqueEventsList;
		WatchableEvent prevEvent(NULL, FileAdded, "", "");
		for (unsigned int i = 0; i < eventList.size(); ++i)
		{
			if (eventList[i] != prevEvent)
				uniqueEventsList.append(eventList[i]);
		}

		// loop through each of the events in the unique list and 
		// handle them
		QListIterator<WatchableEvent> iter(uniqueEventsList);
		while (iter.hasNext())
		{
			const WatchableEvent& event = iter.next();
			switch (event.watchEvent)
			{
			case FileAdded:
				event.obj->onFileAdded(event.param1);
				break;

			case FileDeleted:
				event.obj->onFileRemoved(event.param1);
				break;

			case FileRenamed:
				event.obj->onFileNameChanged(event.param1, event.param2);
				break;

			case FileModified:
				event.obj->onFileModified(event.param1);
				break;
			}
		}

		// clear the list of events
		eventList.clear();
		_memberMutex.unlock();
	}
}

bool FileSystemManager::deleteFiles(vector<FileSystemActor *> objList, vector<FileSystemActor *> &failedObj, bool confirm, bool skipRecycleBin)
{
	if (objList.empty())
		return true;
	
	FileSystemActor *data;
	SHFILEOPSTRUCT fileOperation = { 0 };
	vector<FileSystemActor *> delList;
	int numVirtualIconsDeleted = 0;

	TCHAR * fromPaths = allocateStringFromPathsArray(objList);
	TCHAR * fromPathsOffset = fromPaths;

	// build the paths of files to delete
	for (int i = 0; i < objList.size(); i++)
	{
		// Check if its a filesystem actor
		if (objList[i]->getObjectType() == ObjectType(BumpActor, FileSystem))
		{
			data = (FileSystemActor *) objList[i];

			// Exclude Virtual folders
			if (!data->isFileSystemType(Virtual))
			{
				// Check to see if this file is actually on the hard disk or not
				if (isValidFileName(data->getFullPath()))
				{
					// build the from-path string
					lstrcpy(fromPathsOffset, (LPCTSTR) data->getFullPath().utf16());
					fromPathsOffset += data->getFullPath().size() + 1;

					delList.push_back(data);
				}
			}
			else
			{
				// prompt the user
				dlgManager->clearState();
				dlgManager->setCaption(QT_TR_NOOP("Remove from desktop?"));
				dlgManager->setPrompt(QT_TR_NOOP("Are you sure you want to remove %1 from your desktop?\n(You can re-enable it in Windows' Desktop Properties dialog)").arg(data->getFullPath()));
				if (dlgManager->promptDialog(DialogYesNo))
				{
					// we try and set the associated registry values for these icons
					winOS->SetIconAvailability(winOS->GetIconTypeFromFileName(data->getFullPath()), false);
					// but fade and delete the actor anyways
					FadeAndDeleteActor(data);
				}
				++numVirtualIconsDeleted;
			}
		}
	}

	if ((delList.size() + numVirtualIconsDeleted) != objList.size())
	{
		// Alert the user that one of more VIRTUAL files are in the selection
		MessageClearPolicy clearPolicy;
			clearPolicy.setTimeout(4);
		scnManager->messages()->addMessage(new Message("deleteFiles", QT_TR_NOOP("Some items cannot be deleted (ie. My Computer, Recycle Bin)\nPlease remove them through Windows' Desktop properties"), Message::Warning, clearPolicy));
	}

	if (!delList.empty())
	{
		// Delete the specific files
		fileOperation.hwnd = winOS->GetWindowsHandle();
		fileOperation.wFunc = FO_DELETE;
		fileOperation.pFrom = fromPaths;
		fileOperation.pTo = L"\0\0";
		fileOperation.fFlags = (skipRecycleBin ? 0 : FOF_ALLOWUNDO) | (confirm ? NULL : FOF_NOCONFIRMATION);

		// Remove the item form the listener before deleting
		for (uint i = 0; i < delList.size(); i++)
		{
			if (delList[i]->isPileized())
			{
				removeObject(delList[i]->getPileizedPile());
			}
		}

		SHFileOperation(&fileOperation);

		// See if there were any items that could not be deleted
		for (uint i = 0; i < delList.size(); i++)
		{
			if (isValidFileName(delList[i]->getFullPath()))
			{
				failedObj.push_back(delList[i]);
			}else{
				delList[i]->setLinearVelocity(Vec3(0.0f));
				delList[i]->setAngularVelocity(Vec3(0.0f));
				delList[i]->putToSleep();
			}
		}
	}

	delete fromPaths;
	return failedObj.size() == 0;
}

bool FileSystemManager::deleteFileByName(QString filePath, bool silent, bool skipRecycleBin, bool confirm)
{
	TCHAR filep[MAX_PATH] = {0};
	lstrcpy(filep, (LPCTSTR) filePath.utf16());
	
	SHFILEOPSTRUCT fileOp = { 0 };
		fileOp.hwnd = winOS->GetWindowsHandle();
		fileOp.wFunc = FO_DELETE;
		fileOp.pFrom = filep;
		fileOp.pTo = L"\0\0";
		fileOp.fFlags = (skipRecycleBin ? 0 : FOF_ALLOWUNDO) | (confirm ? NULL : FOF_NOCONFIRMATION) | (silent ? FOF_SILENT : 0);
	bool result = (SHFileOperation(&fileOp) == 0) && !fileOp.fAnyOperationsAborted;

	return result;
}

bool FileSystemManager::renameFile(FileSystemActor *obj, QString newName, bool confirm)
{
	bool result = false;

	// Sanity check
	if (obj->getFileName(false) == newName) return false;

	// Check to see if the file is not VRTUAL and then commence a rename
	if (!obj->isFileSystemType(Virtual))
	{
		// remove the object from the watch list, so that it can be renamed
		if (obj->isPileized())
			removeObject(obj->getPileizedPile());

		QDir p(obj->getFullPath());
		result = renameFile(p, newName, confirm);

		// Add the pile again to reset the Listener
		if (obj->isPileized())
			addObject(obj->getPileizedPile());
	}

	return result;
}

bool FileSystemManager::renameFile(QDir old, QString newName, bool confirm)
{
	QString oldPathString(native(old));
	QDir parentPath = parent(old);
	QString newPathString(native((parentPath / newName)));

	TCHAR oldp[MAX_PATH] = {0};
	TCHAR newp[MAX_PATH] = {0};
	lstrcpy(oldp, (LPCTSTR) oldPathString.utf16());
	lstrcpy(newp, (LPCTSTR) newPathString.utf16());

	SHFILEOPSTRUCT fileOperation = { 0 };
	fileOperation.hwnd = winOS->GetWindowsHandle();
	fileOperation.wFunc = FO_RENAME;
	fileOperation.pFrom = oldp;
	fileOperation.pTo = newp;
	fileOperation.fFlags = FOF_ALLOWUNDO | (confirm ? NULL : FOF_NOCONFIRMATION);

	// Do the windows file operations for Rename
	return (SHFileOperation(&fileOperation) == 0) && !fileOperation.fAnyOperationsAborted;
}

bool FileSystemManager::moveFiles(vector<FileSystemActor *>& objList, QString destDir, vector<FileSystemActor *>& failedObj, bool confirm)
{
	return moveFiles(objList, destDir, failedObj, vector<FileSystemActor*>(), vector<FileSystemActorRename>(), confirm);
}

bool FileSystemManager::moveFiles(vector<FileSystemActor *>& objList, QString destDir, vector<FileSystemActor *>& failedObj, vector<FileSystemActor*>& replacedObj, vector<FileSystemActorRename>& renamedObj, bool confirm)
{
	// Ask the user if they want to move these items, prevents accidental moves
	if (confirm)
	{
		dlgManager->clearState();
		dlgManager->setCaption(QT_TR_NOOP("Move Files?"));
		dlgManager->setPrompt(QT_TR_NOOP("Are you sure you want to move the selected items to '%1'? ").arg(filename(destDir)));

		if (!dlgManager->promptDialog(DialogYesNo))
		{
			failedObj = objList;
			return false;
		}
	}

	// ensure that there are no virtual folders
	for (uint i = 0; i < objList.size(); i++)
	{
		FileSystemActor *fsData = objList[i];

		// Virtual folders should warn the user
		if (fsData->isFileSystemType(Virtual))
		{
			failedObj = objList;

			MessageClearPolicy clearPolicy;
			clearPolicy.setTimeout(4);
			scnManager->messages()->addMessage(new Message("moveFiles", QT_TR_NOOP("Virtual Folders, such as My Computer and the Recycle Bin, cannot be moved!"), Message::Warning, clearPolicy));
			return false;
		}
	}

	if (winOS->IsWindowsVersionGreaterThanOrEqualTo(WindowsVista))
	{
		return moveFilesVista(objList, destDir, failedObj, replacedObj, renamedObj);
	}	
	else
	{
		return moveFilesXP(objList, destDir, failedObj, replacedObj);
	}
}

bool FileSystemManager::moveFilesVista(vector<FileSystemActor*>& objList, QString destDir, vector<FileSystemActor*>& failedObj, vector<FileSystemActor*>& replacedObj, vector<FileSystemActorRename>& renamedObj)
{
	QList<LPWSTR> sourceFiles;
	QList<FileOperationResult> results;
	
	vector<FileSystemActor*>::iterator fileIter;
	for (fileIter = objList.begin(); fileIter != objList.end(); fileIter++)
	{
		sourceFiles.push_back((LPWSTR) (*fileIter)->getFullPath().utf16());
	}

	if (moveFilesVistaHelper(sourceFiles, (LPWSTR) destDir.utf16(), results))
	{
		assert(results.size() >= objList.size());
		for (int i = 0; i < objList.size(); i++)
		{
			// A Compatibility hack. Need to treat replacing files differently
			if (results[i].action == Replaced)
				replacedObj.push_back(objList[i]);
			else if (results[i].action == NotMoved || results[i].action == Error)
				failedObj.push_back(objList[i]);
			else if (results[i].action == Renamed)
			{
				FileSystemActorRename fsRename;
				fsRename.actor = objList[i];
				fsRename.newFileName = results[i].newFileName;
				renamedObj.push_back(fsRename);
			}
		}
		return true;
	}
	else
	{
		failedObj = objList;
		return false;
	}
}

bool FileSystemManager::moveFilesXP(vector<FileSystemActor *>& objList, QString& destDir, vector<FileSystemActor *>& failedObj, vector<FileSystemActor*>& replacedObj)
{
	SHFILEOPSTRUCT fileOperation = { 0 };
	QString toPath(destDir);
	set<FileSystemActor *> duplicateExists;

	TCHAR * fromPaths = allocateStringFromPathsArray(objList);
	TCHAR * fromPathsOffset = fromPaths;

	for (uint i = 0; i < objList.size(); i++)
	{
		FileSystemActor *fsData = objList[i];
		
		if (!isValidFileName(fsData->getFullPath()))
		{
			failedObj = objList;
			delete fromPaths;
			return false;
		}

		// build the from-path string
		lstrcpy(fromPathsOffset, (LPCTSTR) fsData->getFullPath().utf16());
		fromPathsOffset += fsData->getFullPath().size() + 1;

		// save the set of items that have duplicate names, so that we
		// can reference them if the move fails
		QDir destP(destDir);
		if (isValidFileName(native(make_file(destDir, filename(fsData->getFullPath())))))
			duplicateExists.insert(fsData);
	}

	TCHAR newp[MAX_PATH] = {0};
	lstrcpy(newp, (LPCTSTR) toPath.utf16());

	fileOperation.hwnd = winOS->GetWindowsHandle();
	fileOperation.wFunc = FO_MOVE;
	fileOperation.pFrom = fromPaths;
	fileOperation.pTo = newp;
	fileOperation.fFlags = FOF_ALLOWUNDO;

	// Do the windows file operations for Move
	bool succeeded = (SHFileOperation(&fileOperation) == 0) && !fileOperation.fAnyOperationsAborted;

	for (uint i = 0; i < objList.size(); i++)
	{
		FileSystemActor * fsData = objList[i];

		QFileInfo p(fsData->getFullPath());
		QFileInfo destP = make_file(destDir, p.fileName());

		// Determine which files failed and which replaced existing ones
		if (isValidFileName(fsData->getFullPath()))
		{
			failedObj.push_back(fsData);
		}
		else if (duplicateExists.find(fsData) != duplicateExists.end())
		{
			replacedObj.push_back(fsData);
		}
	}

	delete fromPaths;
	return succeeded;
}

bool FileSystemManager::moveFiles(const vector<QString>& filePaths, QString destDir)
{
	SHFILEOPSTRUCT fileOperation = { 0 };
	QString toPath(destDir);
	bool succeeded = false;

	TCHAR * fromPaths = allocateStringFromPathsArray(filePaths);
	TCHAR * fromPathsOffset = fromPaths;

	// build the string of files to copy from
	for (uint i = 0; i < filePaths.size(); i++)
	{
		lstrcpy(fromPathsOffset, (LPCTSTR) filePaths[i].utf16());
		fromPathsOffset += filePaths[i].size() + 1;
	}

	// build the string of the directory to copy to
	TCHAR newp[MAX_PATH] = {0};
	lstrcpy(newp, (LPCTSTR) toPath.utf16());

	fileOperation.hwnd = winOS->GetWindowsHandle();
	fileOperation.wFunc = FO_MOVE;
	fileOperation.pFrom = fromPaths;
	fileOperation.pTo = newp;
	fileOperation.fFlags = FOF_ALLOWUNDO;

	// Do the windows file operations for Move
	succeeded = (SHFileOperation(&fileOperation) == 0) && !fileOperation.fAnyOperationsAborted;

	delete fromPaths;

	// NOTE: it is possible that the succeeded return value may not match 
	// the final state of the move operation, so we should still use the set
	// of failed objects to determine the result of the move
	return succeeded;
}

bool FileSystemManager::copyFiles(vector<FileSystemActor *> &objList, QString destDir, vector<FileSystemActor *> &failedObj, bool confirm)
{
	SHFILEOPSTRUCT fileOperation = { 0 };
	QString toPath(destDir), tmpStr;
	bool rc = false;
	vector<BumpObject *> objFailed;

	TCHAR * fromPaths = allocateStringFromPathsArray(objList);
	TCHAR * fromPathsOffset = fromPaths;

	// ensure that there are no virtual folders in the set and build the
	// files to copy from
	for (uint i = 0; i < objList.size(); i++)
	{
		FileSystemActor * fsData = objList[i];

		// Virtual folders should warn the user
		if (fsData->isFileSystemType(Virtual))
		{
			MessageClearPolicy clearPolicy;
				clearPolicy.setTimeout(2);
			scnManager->messages()->addMessage(new Message("copyFiles", QT_TR_NOOP("Virtual Folders, such as My Computer and the Recycle Bin, cannot be copied!"), Message::Warning, clearPolicy));
			delete fromPaths;
			return false;
		}

		// build the string
		lstrcpy(fromPathsOffset, (LPCTSTR) fsData->getFullPath().utf16());
		fromPathsOffset += fsData->getFullPath().size() + 1;
	}

	// build the string of the path to copy to
	TCHAR newp[MAX_PATH] = {0};
	lstrcpy(newp, (LPCTSTR) toPath.utf16());

	fileOperation.hwnd = winOS->GetWindowsHandle();
	fileOperation.wFunc = FO_COPY;
	fileOperation.pFrom = fromPaths;
	fileOperation.pTo = newp;
	fileOperation.fFlags = FOF_ALLOWUNDO | (confirm ? NULL : FOF_NOCONFIRMATION);

	// do the windows file operations for Copy
	int result = SHFileOperation(&fileOperation);
	if (result != 0)	
		failedObj = objList;

	delete fromPaths;
	return result == 0;
}

bool FileSystemManager::copyFileByName(QString oldPath, QString destDir, QString newFileName, bool confirm, bool silent, bool allowUndo)
{
	SHFILEOPSTRUCT fileOperation = { 0 };
	QString fromPath(oldPath), toPath(destDir);
	toPath = native(destDir / newFileName);

	TCHAR oldp[MAX_PATH] = {0};
	TCHAR newp[MAX_PATH] = {0};
	lstrcpy(oldp, (LPCTSTR) fromPath.utf16());
	lstrcpy(newp, (LPCTSTR) toPath.utf16());

	fileOperation.hwnd = winOS->GetWindowsHandle();
	fileOperation.wFunc = FO_COPY;
	fileOperation.pFrom = (LPCTSTR) oldp;
	fileOperation.pTo = (LPCTSTR) newp;
	fileOperation.fFlags = FOF_RENAMEONCOLLISION | (allowUndo ? FOF_ALLOWUNDO : NULL) | (confirm ? NULL : FOF_NOCONFIRMATION) | (silent ? FOF_SILENT : NULL);

	// do the windows file operations for copy
	return (SHFileOperation(&fileOperation) == 0) && !fileOperation.fAnyOperationsAborted;
}

bool FileSystemManager::createFile(QString filePath)
{
	// Create a file on the disk
	HANDLE hFile = CreateFile((LPCTSTR) filePath.utf16(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile)
	{
		// Close it because the creation process automatically opens it for writing
		CloseHandle(hFile);
		return true;
	}

	return false;
}

bool FileSystemManager::createDirectory(QString dirPath)
{
	// Create a directory
	return CreateDirectory((LPCTSTR) dirPath.utf16(), NULL) ? true : false;
}

void FileSystemManager::createShortcut(QString shortcutFilePath, QString pathToTarget, QString desc)
{
	HRESULT hres = NULL;
	IShellLink * psl = NULL;
	IPersistFile * ppf = NULL;
	
	QDir workingDirectory = scnManager->getWorkingDirectory();
	QDir linkPath = workingDirectory / shortcutFilePath;

	// Get a pointer to the IShellLink interface.
	hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void **) &psl);

	if (SUCCEEDED(hres))
	{
		// Set the path to the shortcut target
		psl->SetPath((LPCWSTR) pathToTarget.utf16());
		psl->SetDescription((LPCWSTR) desc.utf16());

		// Query IShellLink for the IPersistFile interface for
		// saving the shortcut in persistent storage.
		hres = psl->QueryInterface(IID_IPersistFile, (void **) &ppf);

		if (SUCCEEDED(hres))
		{
			// Save the link by calling IPersistFile::Save.
			hres = ppf->Save((LPCOLESTR) native(linkPath).utf16(), TRUE);
			ppf->Release();
		}

		psl->Release();
	}
}

void FileSystemManager::onFileAdded(LPCWSTR strFileName)
{
	QMutexLocker m(&_memberMutex);
	QString wideFileName = QString::fromUtf16((const ushort *)strFileName);
	QDir parentPath = parent(wideFileName);
	Watchable * obj = getWatchObjFromPath(native(parentPath));

	// If we actually have a watcher on this directory, notify it on the next tick
	if (obj)
	{
		// Add it to the List
		eventList.push_back(WatchableEvent(obj, FileAdded, wideFileName, QString()));
	}
}

void FileSystemManager::onFileRemoved(LPCWSTR strFileName)
{
	QMutexLocker m(&_memberMutex);
	QString wideFileName = QString::fromUtf16((const ushort *)strFileName);
	QDir parentPath = parent(wideFileName);
	Watchable * obj = getWatchObjFromPath(native(parentPath));

	// If we actually have a watcher on this directory, notify it on the next tick
	if (obj)
	{
		// Add it to the List
		eventList.push_back(WatchableEvent(obj, FileDeleted, wideFileName, QString()));
	}
}

void FileSystemManager::onFileNameChanged(LPCWSTR strOldFileName, LPCWSTR strNewFileName)
{
	QMutexLocker m(&_memberMutex);
	QString wideOldFileName = QString::fromUtf16((const ushort *)strOldFileName);
	QString wideNewFileName = QString::fromUtf16((const ushort *)strNewFileName);
	QDir parentPath = parent(wideOldFileName);
	Watchable * obj = getWatchObjFromPath(native(parentPath));

	// If we actually have a watcher on this directory, notify it on the next tick
	if (obj)
	{
		// Add it to the List
		eventList.push_back(WatchableEvent(obj, FileRenamed, wideOldFileName, wideNewFileName));
	}
}

void FileSystemManager::onFileModified(LPCWSTR strFileName)
{
	QMutexLocker m(&_memberMutex);
	QString wideFileName = QString::fromUtf16((const ushort *) strFileName);
	QDir parentPath = parent(wideFileName);
	Watchable * obj = getWatchObjFromPath(native(parentPath));

	// If we actually have a watcher on this directory, notify it on the next tick
	if (obj)
	{
		// Add it to the List
		eventList.push_back(WatchableEvent(obj, FileModified, wideFileName, QString()));
	}
}

StrList	FileSystemManager::getWorkingDirectoryContents(QString filter)
{
	if (!scnManager->getCurrentLibrary())
		return getDirectoryContents(native(scnManager->getWorkingDirectory()), filter);
	
	StrList files;
	QListIterator<QString> dirIt(scnManager->getCurrentLibrary()->getFolderPaths());
	while (dirIt.hasNext())
	{
		StrList list = getDirectoryContents(dirIt.next(), filter);
		files.insert(files.end(), list.begin(), list.end());
	}
	return files;
}

StrList FileSystemManager::getDirectoryContents(QString dirPath, QString filter)
{
#ifdef WIN32
	WIN32_FIND_DATA ffd;
	vector<QString> listing;
	HANDLE hFind = NULL;

	// Return early if no directory
	if (!isValidFileName(dirPath))
	{
		return listing;
	}

	if (!dirPath.endsWith("\\"))
	{
		// Add a slash to the end of the Path
		dirPath.append("\\");
	}

	hFind = FindFirstFile((LPCTSTR) (dirPath + filter).utf16(), &ffd);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			// Add the filename to the list
			bool isHidden = (ffd.dwFileAttributes & Hidden) > 0;
			if (!isHidden || GLOBAL(settings).LoadHiddenFiles)
			{
				QString fileName = QString::fromUtf16((const ushort *) ffd.cFileName);
				if (fileName != "." && 
					fileName != "..")
				{
					listing.push_back(dirPath + fileName);
				}
			}
		} while (FindNextFile(hFind, &ffd));

		FindClose(hFind);
	}

	return listing;
#else
	vector<QString> fileListing;

	QStringList filters;
	filters << filter;
	QDir directory(dirPath);
	if (exists(directory))
	{
		directory.setNameFilters(filters);
		directory.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
		directory.setSorting(QDir::Name | QDir::DirsFirst);

		QFileInfoList list = directory.entryInfoList();
		for (int i = 0; i < list.size(); ++i)
		{
			fileListing.push_back(native(list[i]));
		}
	}

	return fileListing;
#endif
}

bool FileSystemManager::getShortcutTarget(QString shortcutFileName, QString * targetOut, QString * argsOut, QString * workingDirOut)
{
	assert(targetOut);

	// return true if _any_ of the attributes can be resolved

	bool targetResolved = false;
	bool result = false;
	IShellLink * psl = NULL; 

	// Accounting for the newer "Advertised Shortcuts" which refer to MSI operations which may pre-empt the 
	// launching of the specified shortcut
	DWORD targetPathLen = MAX_PATH;
	TCHAR targetPath[MAX_PATH];
	TCHAR productCode[MAX_PATH];
	TCHAR featureId[MAX_PATH];
	TCHAR componentCode[MAX_PATH];
	if (pfnMsiGetShortcutTarget && pfnMsiGetComponentPath)
	{
		if (ERROR_SUCCESS == pfnMsiGetShortcutTarget((LPCTSTR) shortcutFileName.utf16(), productCode, featureId, componentCode)) 
		{
			if (INSTALLSTATE_LOCAL == pfnMsiGetComponentPath(productCode, componentCode, targetPath, &targetPathLen))
			{
				*targetOut = QString::fromUtf16((const ushort *) targetPath);
				targetResolved = true;
				result = true;
			}
		}
	}

	// Get a pointer to the IShellLink interface. 
	TCHAR args[MAX_PATH];
	TCHAR workingDir[MAX_PATH];

	HRESULT hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID *) &psl); 
	if (SUCCEEDED(hres)) 
	{ 
		IPersistFile * ppf = NULL; 

		// Get a pointer to the IPersistFile interface. 
		if (SUCCEEDED(psl->QueryInterface(IID_IPersistFile, (LPVOID *) &ppf))) 
		{ 
			// Load the shortcut. 
			if (SUCCEEDED(ppf->Load((LPCOLESTR) shortcutFileName.utf16(), STGM_READ))) 
			{ 
				// Resolve the link. 
				if (SUCCEEDED(psl->Resolve(winOS->GetWindowsHandle(), SLR_NOUPDATE | SLR_NO_UI))) 
				{ 
					// Get the path to the link target. 
					if (!targetResolved && SUCCEEDED(psl->GetPath(targetPath, MAX_PATH, NULL, 0))) 
					{
						*targetOut = QString::fromUtf16((const ushort *) targetPath);
						result = true;
					}
					if (argsOut && SUCCEEDED(psl->GetArguments(args, MAX_PATH)))
					{
						*argsOut = QString::fromUtf16((const ushort *) args);
						result = true;
					}
					if (workingDirOut && SUCCEEDED(psl->GetWorkingDirectory(workingDir, MAX_PATH)))
					{
						*workingDirOut = QString::fromUtf16((const ushort *) workingDir);
						result = true;
					}
				} 
			} 

			// Release the pointer to the IPersistFile interface. 
			ppf->Release(); 
		} 

		// Release the pointer to the IShellLink interface. 
		psl->Release(); 
	} 
	return result;
}

uint FileSystemManager::getFileAttributes(QString fileName)
{
	WIN32_FILE_ATTRIBUTE_DATA fileAttributes = { 0 };

	// Get the file attributes of this file
	GetFileAttributesEx((LPCTSTR) fileName.utf16(), GetFileExInfoStandard, &fileAttributes);

	return (uint) fileAttributes.dwFileAttributes;
}

QString FileSystemManager::getFileExtension(QString filePath)
{
	QFileInfo info(filePath);
	if (info.filePath().contains(".") && !info.suffix().isEmpty())
		return QString(".") + info.suffix().toLower();
	return QString();
}

Watchable *FileSystemManager::getWatchObjFromPath(QString filePath)
{
	for (uint i = 0; i < listenerList.size(); i++)
	{
		StrList watchDirs = listenerList[i]->getWatchDir();
		for (uint k = 0; k < watchDirs.size(); k++)
		{
			// If this directory query is contained in our listener list, return true
			if ((watchDirs[k] == filePath) ||
				(watchDirs[k] == (filePath + "\\")))
			{
				return listenerList[i];
			}
		}
	}

	return NULL;
}

bool FileSystemManager::isDirectoryWatched(QString dir)
{
	return (getWatchObjFromPath(dir) != NULL);
}

bool FileSystemManager::isValidFileName(QString filePath)
{
	// If the file attributes don't return anything, this file is invalid
	return getFileAttributes(filePath) != 0;
}

bool FileSystemManager::hasCommonRoots(const FileSystemActor * actor, const vector<BumpObject *>& dropObjects) const
{
	// XXX: see http://msdn.microsoft.com/en-us/library/aa364952.aspx
	vector<BumpObject *> dropObjs = dropObjects;
	FileSystemActor * firstValid = NULL;
	for (int i = 0; i < dropObjs.size() && !firstValid; ++i)
	{
		if (dropObjs[i]->isBumpObjectType(BumpActor))
		{
			Actor * a = (Actor *) dropObjs[i];
			if (a->isActorType(FileSystem))
				firstValid = (FileSystemActor *) a;
		}
		if (dropObjs[i]->isBumpObjectType(BumpPile))
		{
			Pile * p = (Pile *) dropObjs[i];
			vector<BumpObject *> items = p->getPileItems();
			for (int j = 0; j < items.size(); ++j)
			{
				dropObjs.push_back(items[j]);
			}
		}
	}

	if (firstValid)
	{
#ifdef WIN32
		int d1 = PathGetDriveNumber((LPCTSTR) actor->getTargetPath().utf16());
		int d2 = PathGetDriveNumber((LPCTSTR) firstValid->getTargetPath().utf16());
		return	(d1 == d2);
#else
	#error NOT IMPLEMENTED
#endif
	}
	return false;
}

void FileSystemManager::setFileAttributes(QString fileName, uint attribute)
{
	if (isValidFileName(fileName))
	{
		// Set the file attributes on this file
		SetFileAttributes((LPCTSTR) fileName.utf16(), attribute);
	}
}

uint FileSystemManager::getFileSize(QString filePath)
{
	LARGE_INTEGER fileSize;
	HANDLE hFile;

	// Open the File for reading
	hFile = CreateFile((LPCTSTR) filePath.utf16(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

	if (hFile)
	{
		// Get the file size
		fileSize.LowPart = 0;
		GetFileSizeEx(hFile, &fileSize);

		CloseHandle(hFile);
		return fileSize.LowPart;
	}

	return 0;
}

time_t FileSystemManager::fileTimeToUnixTime(const FILETIME& filetime)
{
	LONGLONG longlong = filetime.dwHighDateTime; 

	longlong <<= 32; 
	longlong |= filetime.dwLowDateTime; 
	longlong -= 116444736000000000; 

	return longlong / 10000000; 
}

time_t FileSystemManager::getFileCreationTime(QString filePath)
{
	HANDLE hFile = CreateFile((LPCTSTR) filePath.utf16(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile)
	{
		// Get the file modification time
		FILETIME fileCreationTime;
		GetFileTime(hFile, &fileCreationTime, NULL, NULL);

		CloseHandle(hFile);
		return fileTimeToUnixTime(fileCreationTime);
	}
	return 0;
}

bool FileSystemManager::launchFileAsync(QString filePath, QString arguments, QString workingDir, bool runAsElevated, bool quoteArgs)
{
	return launchFile(filePath, arguments, workingDir, runAsElevated, quoteArgs, true);
}

bool FileSystemManager::launchFile(SHELLEXECUTEINFO sei, QString filePath) {
	
	QString errMsg;

	if(!ShellExecuteEx(&sei))
	{
		// for all intents and purposes, we will return true after an
		// async execution

		int rcVal = (int) sei.hInstApp;

		errMsg = QT_TR_NOOP("BumpTop failed to launch file: %1.\n\n").arg(filePath);

		// Select the appropriate Message
		switch (rcVal)
		{
		case 0: errMsg.append("Out of Memory or Resources."); break;
		case ERROR_BAD_FORMAT: errMsg.append("The .exe file is invalid (non-Microsoft Win32 .exe or error in .exe image). "); break;
		case SE_ERR_ACCESSDENIED: errMsg.append("The operating system denied access to the specified file or link target. "); break;
		case SE_ERR_ASSOCINCOMPLETE: errMsg.append("The file name association is incomplete or invalid. "); break;
		case SE_ERR_DDEBUSY: errMsg.append("The Dynamic Data Exchange (DDE) transaction could not be completed because other DDE transactions were being processed. "); break;
		case SE_ERR_DDEFAIL: errMsg.append("The DDE transaction failed. "); break;
		case SE_ERR_DDETIMEOUT: errMsg.append("The DDE transaction could not be completed because the request timed out. "); break;
		case SE_ERR_DLLNOTFOUND: errMsg.append("The specified DLL was not found. "); break;
		case SE_ERR_FNF: errMsg.append("The specified file was not found. "); break;
		case SE_ERR_NOASSOC: errMsg.append("There is no application associated with the given file name extension. "); break;
		case SE_ERR_OOM: errMsg.append("There was not enough memory to complete the operation. "); break;
		case SE_ERR_PNF: errMsg.append("The specified path was not found. "); break;
		case SE_ERR_SHARE: errMsg.append("A sharing violation occurred. "); break;
		}

		printUnique("FileSystemManager::launchFile", errMsg);
		return false;

	}

	return true;
}

bool FileSystemManager::launchFile(QString filePath, LPWSTR verb)
{
	int rcVal = 0;

	// check if we should use the pidl
	LPCITEMIDLIST pidl = NULL;	
	int virtualIconIndex = winOS->GetIconTypeFromFileName(filePath);
	if (virtualIconIndex > -1)
		pidl = winOS->GetPidlFromName(virtualIconIndex);

	// ensure that all the arguments are quoted
	QString wPath(filePath);
	ensureQuoted(wPath);

	SHELLEXECUTEINFO sei = {0};

	sei.cbSize = sizeof(sei);
	sei.hwnd = winOS->GetWindowsHandle();
	sei.lpVerb = verb;

	if (pidl)
		sei.lpIDList = (LPVOID) pidl;
	else
		sei.lpFile = (LPCTSTR) wPath.utf16();

	sei.nShow = SW_SHOWNORMAL;

	return launchFile(sei,filePath);
}

bool FileSystemManager::launchFile(QString filePath, QString arguments, QString workingDir, bool runAsElevated, bool quoteArgs, bool async)
{
	QString errMsg;
	int rcVal = 0;

	// check if we should use the pidl
	LPCITEMIDLIST pidl = NULL;	
	int virtualIconIndex = winOS->GetIconTypeFromFileName(filePath);
	if (virtualIconIndex > -1)
		pidl = winOS->GetPidlFromName(virtualIconIndex);

	// ensure that all the arguments are quoted
	QString wPath(filePath), wArgs(arguments), wDir(workingDir);
	ensureQuoted(wPath);
	if (quoteArgs)
		ensureQuoted(wArgs);
	ensureQuoted(wDir);
	
	// Try executing the file
	// Trying with ShellExecuteEx to see if we can fix the problems with .lnk not launching
	SHELLEXECUTEINFO sei = {0};
	

	sei.cbSize = sizeof(sei);
	// #define SEE_MASK_NOZONECHECKS      0x00800000
	sei.fMask = SEE_MASK_FLAG_LOG_USAGE | (pidl ? SEE_MASK_IDLIST : 0) | (async ? SEE_MASK_ASYNCOK : 0) | 0x00800000;
	sei.hwnd = winOS->GetWindowsHandle();
	if (winOS->IsWindowsVersionGreaterThanOrEqualTo(WindowsVista) && runAsElevated)
		sei.lpVerb =  L"runas"; // 'secret' verb to prompt for elevation on Vista
	else
		sei.lpVerb = NULL; // giving a null value for the verb forces it to use the default verb
	if (pidl)
		sei.lpIDList = (LPVOID) pidl;
	else
		sei.lpFile = (LPCTSTR) wPath.utf16();
	sei.lpParameters = (LPCTSTR) wArgs.utf16();
	sei.lpDirectory = (LPCTSTR) wDir.utf16();
	sei.nShow = SW_SHOWNORMAL;

	return launchFile(sei,filePath);
}

bool FileSystemManager::resolveVolume( DEV_BROADCAST_HDR * data, QList<QString>& volumesOut )
{
	// only handle logical volumes
	if (data->dbch_devicetype == DBT_DEVTYP_VOLUME)
	{
		DEV_BROADCAST_VOLUME * volume = (DEV_BROADCAST_VOLUME *) data;
		if (volume->dbcv_flags != DBTF_NET)
		{
			// not a network volume
			// so get the drive letter
			static QString driveLetters("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
			for (int i = 0; i < driveLetters.size(); ++i)
			{
				if (volume->dbcv_unitmask & (1 << i))
				{
					QString driveLetter(1, driveLetters[i]);
					driveLetter += ":\\";
					volumesOut.append(driveLetter);
				}
			}
			if (!volumesOut.isEmpty())
				return true;
		}
	}
	return false;
}

bool FileSystemManager::resolveVolumeName( QString volume, QString& volumeNameOut )
{
	if (!exists(volume))
		return false;

	// derive the drive name from the drive letter
	TCHAR volumeName[MAX_PATH];
	volumeNameOut = QString("Removable Disk (%1)").arg(volume);
	if (GetVolumeInformation((LPCTSTR) volume.utf16(), volumeName, MAX_PATH, NULL, NULL, NULL, NULL, 0))
	{
		QString tmp = QString::fromUtf16((const ushort *) volumeName);
		if (!tmp.isEmpty())
			volumeNameOut = tmp;
	}	
	return true;
}

bool FileSystemManager::isValidVolume( QString volume )
{
	if (!exists(volume))
		return false;

	unsigned int driveType = GetDriveType((LPCTSTR) volume.utf16());
	if (driveType > DRIVE_NO_ROOT_DIR)
	{
		DWORD spc, bps, nfc, tnc;
		if (GetDiskFreeSpace((LPCTSTR) volume.utf16(),
			&spc, &bps, &nfc, &tnc))
			return true;
		return false;
	}
	return false;
}

bool FileSystemManager::isVolumeADisc( QString volume )
{
	if (!exists(volume))
		return false;

	unsigned int driveType = GetDriveType((LPCTSTR) volume.utf16());
	return (driveType == DRIVE_CDROM);
}

void FileSystemManager::onPowerSuspend()
{
	// unwatch all the directories
	for (int i = 0; i < listenerList.size(); ++i)
	{
		Watchable * w = listenerList[i];

		StrList watchDirs = w->getWatchDir();
		for (int j = 0; j < watchDirs.size(); j++)
			dirWatcher.UnwatchDirectory((LPCTSTR) watchDirs[j].utf16());
	}
}

void FileSystemManager::onPowerResume()
{
	// re-watch all the directories
	for (int i = 0; i < listenerList.size(); ++i)
	{
		Watchable * w = listenerList[i];

		uint changeFilter = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_ATTRIBUTES;
		StrList watchDirs = w->getWatchDir();
		for (int j = 0; j < watchDirs.size(); j++)
			dirWatcher.WatchDirectory((LPCTSTR) watchDirs[j].utf16(), changeFilter, this);
	}
}

bool FileSystemManager::isIdenticalPath( const QString& pathA, const QString& pathB )
{
	QString pa = pathA.trimmed();
	QString pb = pathB.trimmed();
	return (pa.compare(pb, Qt::CaseInsensitive) == 0);
}