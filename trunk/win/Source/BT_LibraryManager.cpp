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
#include "BT_LibraryManager.h"
#include "BT_Library.h"
#include "BT_WindowsOS.h"
#include "BT_Util.h"

LibraryManager::LibraryManager()
: _hMod(NULL)
, _pSHGetKnownFolderItem(NULL)
, _pSHGetKnownFolderIDList(NULL)
, _pSHCreateItemFromParsingName(NULL)
, _pSHGetIDListFromObject(NULL)
, _pSHGetNameFromIDList(NULL)
, _pSHCreateItemFromIDList(NULL)
, _comInitialized(false)
, _notifyRegisterId(0)
, _libraryFolder(NULL)
, _libraries()
, _desktopLibraryName(QT_NT("Desktop"))
{
	_hMod = LoadLibrary(QT_NT(L"Shell32.dll"));
	if (_hMod)
	{
		_pSHGetKnownFolderItem = (SHGetKnownFolderItemSignature) GetProcAddress(_hMod, QT_NT("SHGetKnownFolderItem"));
		_pSHGetKnownFolderIDList = (SHGetKnownFolderIDListSignature) GetProcAddress(_hMod, QT_NT("SHGetKnownFolderIDList"));
		_pSHCreateItemFromParsingName = (SHCreateItemFromParsingNameSignature) GetProcAddress(_hMod, QT_NT("SHCreateItemFromParsingName"));
		_pSHGetIDListFromObject = (SHGetIDListFromObjectSignature) GetProcAddress(_hMod, QT_NT("SHGetIDListFromObject"));
		_pSHGetNameFromIDList = (SHGetNameFromIDListSignature) GetProcAddress(_hMod, QT_NT("SHGetNameFromIDList"));
		_pSHCreateItemFromIDList = (SHCreateItemFromIDListSignature) GetProcAddress(_hMod, QT_NT("SHCreateItemFromIDList"));
	}
	
	_comInitialized = (bool)SUCCEEDED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED));
	
	init();

	initializeLibraryList();
}

LibraryManager::~LibraryManager()
{
	SAFE_RELEASE(_libraryFolder);

	notifyLibraryChanges(false);
	
	if (_comInitialized)
		CoUninitialize();

	if (_hMod)
		FreeLibrary(_hMod);
}

bool LibraryManager::init()
{
	if (!_comInitialized || !_pSHGetKnownFolderItem || !_pSHGetIDListFromObject)
		return false;

	if (_libraryFolder)
		return true;

	HRESULT hr = S_OK;
	bool result = false;

	IShellItem* libDirItem;
	hr = _pSHGetKnownFolderItem(FOLDERID_Libraries, KF_FLAG_DEFAULT_PATH, NULL, IID_PPV_ARGS(&libDirItem));
	if (SUCCEEDED(hr))
	{
		IShellFolder* libDirFolder;
		hr = libDirItem->BindToHandler(NULL, BHID_SFObject, IID_PPV_ARGS(&libDirFolder));
		if (SUCCEEDED(hr))
		{
			_libraryFolder = libDirFolder;
			result = true;
		}
		libDirItem->Release();
	}
	return result;
}

const QList< QSharedPointer<Library> > LibraryManager::initializeLibraryList()
{
	HRESULT hr = S_OK;
	
	_libraries.clear();

	// Create and insert the default Desktop 'Library' into the list
	// This library is always created and will work on Vista and XP
	QSharedPointer<Library> desktopLib = createDesktopLibrary();
	_libraries.insert(desktopLib->getHashKey(), desktopLib);
	
	if (!createWin7Libraries())
	{
		// If we could not create the Win7 Libraries, try Vista KnownFolders
		createVistaLibraries();
	}	
	
	return _libraries.values();
}

bool LibraryManager::createWin7Libraries()
{
	if (!_comInitialized || !_pSHCreateItemFromParsingName || !_libraryFolder)
		return false;
	
	IEnumIDList* itemEnum;
	HRESULT hr = _libraryFolder->EnumObjects(NULL, SHCONTF_FOLDERS, &itemEnum);
	if (SUCCEEDED(hr))
	{
		ITEMIDLIST* itemIdList;
		while (itemEnum->Next(1, &itemIdList, NULL) == S_OK)
		{
			STRRET strret;
			hr = _libraryFolder->GetDisplayNameOf(itemIdList, SHGDN_FORPARSING, &strret);
			if (SUCCEEDED(hr))
			{
				LPWSTR parsingName;
				StrRetToStr(&strret, NULL, &parsingName);
				
				IShellItem* item;
				hr = _pSHCreateItemFromParsingName(parsingName, NULL, IID_PPV_ARGS(&item));
				if (SUCCEEDED(hr))
				{		
					QSharedPointer<Library> library = createLibrary(item);
					if (library)
						_libraries.insert(library->getHashKey(), library);
					item->Release();
				}
				CoTaskMemFree(parsingName);
			}
			CoTaskMemFree(itemIdList);
		}
		itemEnum->Release();
	}
	return true;
}

bool LibraryManager::createVistaLibraries()
{
	if (!winOS->IsWindowsVersionGreaterThanOrEqualTo(WindowsVista))
		return false;

	QList<KNOWNFOLDERID> documentFolders;
	documentFolders.append(FOLDERID_Documents);
	documentFolders.append(FOLDERID_PublicDocuments);
	QSharedPointer<Library> documentLibrary = createLibraryFromKnownFolders(documentFolders);
	if (documentLibrary)
		_libraries.insert(documentLibrary->getHashKey(), documentLibrary);

	QList<KNOWNFOLDERID> musicFolders;
	musicFolders.append(FOLDERID_Music);
	musicFolders.append(FOLDERID_PublicMusic);
	QSharedPointer<Library> musicLibrary = createLibraryFromKnownFolders(musicFolders);
	if (musicLibrary)
		_libraries.insert(musicLibrary->getHashKey(), musicLibrary);
	
	QList<KNOWNFOLDERID> pictureFolders;
	pictureFolders.append(FOLDERID_Pictures);
	pictureFolders.append(FOLDERID_PublicPictures);
	QSharedPointer<Library> pictureLibrary = createLibraryFromKnownFolders(pictureFolders);
	if (pictureLibrary)
		_libraries.insert(pictureLibrary->getHashKey(), pictureLibrary);
	
	QList<KNOWNFOLDERID> videoFolders;
	videoFolders.append(FOLDERID_Videos);
	videoFolders.append(FOLDERID_PublicVideos);
	QSharedPointer<Library> videoLibrary = createLibraryFromKnownFolders(videoFolders);
	if (videoLibrary)
		_libraries.insert(videoLibrary->getHashKey(), videoLibrary);
	
	return true;
}

QSharedPointer<Library> LibraryManager::createLibraryFromKnownFolders(QList<KNOWNFOLDERID>& kfList)
{
	QSharedPointer<LibraryImpl> library(NULL);
	if (!_comInitialized || !_pSHGetKnownFolderIDList || !_pSHCreateItemFromIDList)
		return library;
	
	QString libraryName;
	QString iconPath;
	QList<QString> directories;
	QListIterator<KNOWNFOLDERID> iter(kfList);
	while (iter.hasNext())
	{
		const KNOWNFOLDERID& id = iter.next();
		PIDLIST_ABSOLUTE pidl;
		if (SUCCEEDED(_pSHGetKnownFolderIDList(id, KF_FLAG_DEFAULT_PATH, NULL, &pidl)))
		{
			IShellItem* item;
			if (SUCCEEDED(_pSHCreateItemFromIDList(pidl, IID_PPV_ARGS(&item))))
			{
				LPWSTR path;
				if (SUCCEEDED(item->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &path)))
				{
					directories.append(QString::fromUtf16((const ushort*) path));
					CoTaskMemFree(path);

					if (id == kfList.front())
					{
						// This is the first directory, so use it's folder name as the name of the Library
						LPWSTR name;
						if (SUCCEEDED(item->GetDisplayName(SIGDN_NORMALDISPLAY, &name)))
						{
							libraryName = QString::fromUtf16((const ushort*) name);
							CoTaskMemFree(name);
						}
					}
				}
				item->Release();
			}
			CoTaskMemFree(pidl);
		}
	}
	
	if (directories.isEmpty())
		return library;
	
	library = QSharedPointer<LibraryImpl>(new LibraryImpl(libraryName));
	library->setTextureKey(QString_NT("library_default.folder.icon"));
	library->setFolderPaths(directories);
	library->setHashKey(QString_NT("lib_%1").arg(directories.front()));
	library->setValid(true);
	return library;
}

QSharedPointer<Library> LibraryManager::createDesktopLibrary()
{
	QList<QString> folders;
	folders.append(winOS->GetSystemPath(DesktopDirectory));		// Default Save directory
	folders.append(winOS->GetSystemPath(AllUsersDesktopDir));
	
	// Create the Desktop Library
	QSharedPointer<LibraryImpl> library(new LibraryImpl(_desktopLibraryName));
	library->setFolderPaths(folders);
	library->setTextureKey(QString_NT("library_default.desktop.icon"));
	library->setValid(true);
	library->setHashKey(QString_NT("def_%1").arg(_desktopLibraryName));
	return library;
}

QSharedPointer<Library> LibraryManager::createLibrary(IShellItem* item)
{
	if (!_comInitialized)
		return QSharedPointer<Library>();

	QSharedPointer<LibraryImpl> lib;
	LPWSTR name;
	HRESULT hr = item->GetDisplayName(SIGDN_NORMALDISPLAY, &name);
	if (SUCCEEDED(hr))
	{
		QString libraryName(QString::fromUtf16((const ushort*)name));
		QString iconPath = getLibraryIconPath(item);
		
		IShellLibrary* library;
		hr = SHLoadLibraryFromItem(item, STGM_READ, IID_PPV_ARGS(&library));
		if (SUCCEEDED(hr))
		{
			// Get the folders that this library uses
			QList<QString> folderPaths = getLibraryFolderPaths(library);
			if (!folderPaths.isEmpty())
			{
				lib = QSharedPointer<LibraryImpl>(new LibraryImpl(libraryName));
				lib->setFolderPaths(folderPaths);
				lib->setIconPath(iconPath);
				lib->setValid(true);
				lib->setHashKey(QString_NT("lib_%1").arg(libraryName));
			}
			library->Release();
		}
		CoTaskMemFree(name);
	}
	return lib;
}

QSharedPointer<Library> LibraryManager::addFolderAsLibrary(PIDLIST_ABSOLUTE pidl)
{
	QSharedPointer<LibraryImpl> library(NULL);

	if (!_comInitialized)
		return library;
	
	IShellItem* folderItem;
	if (SUCCEEDED(SHCreateShellItem(NULL, NULL, pidl, &folderItem)))
	{
		PWSTR filePath = NULL;
		if (SUCCEEDED(folderItem->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &filePath)))
		{
			QList<QString> directories;
			directories.append(QString::fromUtf16((const ushort*)filePath));
			CoTaskMemFree(filePath);

			PWSTR displayName = NULL;
			if (SUCCEEDED(folderItem->GetDisplayName(SIGDN_PARENTRELATIVEFORADDRESSBAR, &displayName)))
			{
				QString name = QString::fromUtf16((const ushort*)displayName);
				CoTaskMemFree(displayName);
				
				library = QSharedPointer<LibraryImpl>(new LibraryImpl(name));
				library->setFolderPaths(directories);
				library->setHashKey(QString_NT("usr_%1").arg(directories.front()));
				library->setTextureKey(QString_NT("library_default.folder.icon"));
				_libraries.insert(library->getHashKey(), library);
			}
		}
		folderItem->Release();
	}
	return library;
}

QSharedPointer<Library> LibraryManager::addFolderAsLibrary(QString& folderPath)
{
	QSharedPointer<Library> library(NULL);
	PIDLIST_ABSOLUTE pidl = winOS->GetAbsolutePidlFromAbsFilePath(folderPath);
	if (pidl)
	{
		library = addFolderAsLibrary(pidl);
	}
	return library;
}

void LibraryManager::removeLibrary(QSharedPointer<Library>& library)
{
	removeLibrary(library->getHashKey());
}

void LibraryManager::removeLibrary(QString& key)
{
	QMap< QString, QSharedPointer<Library> >::iterator iter = _libraries.find(key);
	if (iter != _libraries.end())
		_libraries.erase(iter);
}

QString LibraryManager::getLibraryIconPath(IShellItem* item)
{
	QString path;
	if (!_comInitialized || !item || !_pSHGetIDListFromObject)
		return path;
	
	IShellItem* parent;
	if (SUCCEEDED(item->GetParent(&parent)))
	{
		IShellIcon* iconEnum;
		if (SUCCEEDED(parent->BindToHandler(NULL, BHID_SFObject, IID_PPV_ARGS(&iconEnum))))
		{
			IShellFolder* parentFolder;
			if (SUCCEEDED(parent->BindToHandler(NULL, BHID_SFObject, IID_PPV_ARGS(&parentFolder))))
			{
				PIDLIST_ABSOLUTE absIdList;
				HRESULT hr = _pSHGetIDListFromObject(item, &absIdList);
				if (SUCCEEDED(hr))
				{
					// Extract the relative item id list so that we can use the icon enumerator
					// to extract the icon path of the item
					PIDLIST_RELATIVE itemIdList = relativePIDLFromAbsolute(parentFolder, absIdList);
					if (itemIdList)
					{
						int index = 0;
						hr = iconEnum->GetIconOf(itemIdList, GIL_FORSHELL, &index);
						if (SUCCEEDED(hr))
							path = QString_NT("*;%1").arg(index);
						CoTaskMemFree(itemIdList);
					}
					CoTaskMemFree(absIdList);
				}
				parentFolder->Release();
			}
			iconEnum->Release();
		}
		parent->Release();
	}
	return path;
}

QList<QString> LibraryManager::getLibraryFolderPaths(IShellLibrary* library)
{
	QList<QString> folderList;
	if (!_comInitialized || !library)
		return folderList;

	HRESULT hr = S_OK;
	IShellItem* defaultSaveFolder;
	hr = library->GetDefaultSaveFolder(DSFT_DETECT, IID_PPV_ARGS(&defaultSaveFolder));
	if (SUCCEEDED(hr))
	{
		IShellItemArray* dirItemArray;
		hr = library->GetFolders(LFF_FORCEFILESYSTEM, IID_PPV_ARGS(&dirItemArray));
		if (SUCCEEDED(hr))
		{
			IEnumShellItems* itemEnum;
			hr = dirItemArray->EnumItems(&itemEnum);
			if (SUCCEEDED(hr))
			{
				IShellItem* dirItem;
				while (itemEnum->Next(1, &dirItem, NULL) == S_OK)
				{
					LPWSTR name;
					hr = dirItem->GetDisplayName(SIGDN_FILESYSPATH, &name);
					if (SUCCEEDED(hr))
					{
						QString dirPath = QString::fromUtf16((const ushort *)name);
						
						int different;
						if (defaultSaveFolder->Compare(dirItem, SICHINT_ALLFIELDS, &different) == S_OK)
						{
							// This is the default save folder, so have it at the top of the list
							folderList.prepend(dirPath);
						}
						else
						{
							folderList.append(dirPath);
						}
						CoTaskMemFree(name);
					}
					dirItem->Release();
				}
				itemEnum->Release();
			}
			dirItemArray->Release();
		}
		defaultSaveFolder->Release();
	}
	return folderList;
}

const QList< QSharedPointer<Library> > LibraryManager::getLibraries() const
{
	return _libraries.values();
}

QSharedPointer<Library> LibraryManager::getLibraryByKey(QString &key) const
{
	QSharedPointer<Library> lib = _libraries.value(key);
	if (!lib)
	{
		lib = getDesktopLibrary();
	}
	return lib;
}

QSharedPointer<Library> LibraryManager::getDesktopLibrary() const
{
	return getLibraryByKey(QString_NT("def_%1").arg(_desktopLibraryName));
}

QList< QSharedPointer<Library> > LibraryManager::getFolderLibraries()
{
	QList< QSharedPointer<Library> > libs;
	QList<QString> keys = _libraries.keys();
	QListIterator<QString> keyIter(keys);
	while (keyIter.hasNext())
	{
		QString key = keyIter.next();
		if (key.startsWith(QT_NT("usr_")))
		{
			libs.append(_libraries[key]);
		}
	}
	return libs;
}

QList<QString> LibraryManager::getFolderLibraryDirectories()
{
	QList<QString> libDirs;
	QList< QSharedPointer<Library> >& folderLibs = getFolderLibraries();
	QListIterator< QSharedPointer<Library> > iter(folderLibs);
	while (iter.hasNext())
	{
		const QList<QString>& dirList = iter.next()->getFolderPaths();
		assert(dirList.size() > 0);
		libDirs.append(dirList.front());
	}
	return libDirs;
}

PIDLIST_RELATIVE LibraryManager::relativePIDLFromAbsolute(IShellFolder* folder, PIDLIST_ABSOLUTE absList)
{
	if (!_comInitialized || !_pSHGetIDListFromObject)
		return NULL;

	HRESULT hr = S_OK;
	PIDLIST_RELATIVE relativeItem = NULL;
	PIDLIST_ABSOLUTE folderList;
	hr = _pSHGetIDListFromObject(folder, &folderList);
	if (SUCCEEDED(hr))
	{
		bool valid = true;
		LPITEMIDLIST currentFolder = folderList;
		LPITEMIDLIST currentItem = absList;
		while (currentFolder->mkid.cb)
		{
			if (currentFolder->mkid.cb != currentItem->mkid.cb)
			{
				valid = false;
				break;
			}
			
			UINT length = currentFolder->mkid.cb - sizeof(currentFolder->mkid.cb);
			if (strncmp((CHAR*)currentFolder->mkid.abID, (CHAR*)currentItem->mkid.abID, length) != 0)
			{
				valid = false;
				break;
			}

			currentFolder = (LPITEMIDLIST)((BYTE*)currentFolder + currentFolder->mkid.cb);
			currentItem = (LPITEMIDLIST)((BYTE*)currentItem + currentItem->mkid.cb);
		}
		
		if (valid && currentItem->mkid.cb)
		{
			LPITEMIDLIST sizeCountList = currentItem;
			UINT size = sizeof(sizeCountList->mkid.cb);
			while (sizeCountList->mkid.cb)
			{
				size += sizeCountList->mkid.cb;
				sizeCountList = (LPITEMIDLIST)((BYTE*)sizeCountList + sizeCountList->mkid.cb);
			}
			relativeItem = (LPITEMIDLIST)CoTaskMemAlloc(size);
			memcpy((void*)relativeItem, (void*)currentItem, size);	
		}
		CoTaskMemFree(folderList);
	}
	return relativeItem;
}

bool LibraryManager::notifyLibraryChanges(bool value)
{
	if (!_comInitialized || !_libraryFolder)
		return false;

	bool result = false;
	if (value && !_notifyRegisterId)
	{
		ITEMIDLIST* folderPIDL;
		HRESULT hr = _pSHGetIDListFromObject(_libraryFolder, &folderPIDL);
		if (SUCCEEDED(hr))
		{
			SHChangeNotifyEntry libraryNotify;
			libraryNotify.fRecursive = FALSE;
			libraryNotify.pidl = folderPIDL;
			int notifyRegisterId = SHChangeNotifyRegister(winOS->GetWindowsHandle(), SHCNRF_ShellLevel | SHCNRF_InterruptLevel | SHCNRF_NewDelivery, SHCNE_RMDIR | SHCNE_CREATE | SHCNE_RENAMEFOLDER | SHCNE_UPDATEDIR, WM_LIBRARYCHANGE, 1, &libraryNotify);
			if (notifyRegisterId > 0)
			{
				_notifyRegisterId = notifyRegisterId;
				result = true;
			}
			CoTaskMemFree(folderPIDL);
		}
	}
	else if (!value && _notifyRegisterId)
		result = SHChangeNotifyDeregister(_notifyRegisterId);
	return result;
}

void LibraryManager::onLibraryChangeEvent(long eventId, ITEMIDLIST* itemOne, ITEMIDLIST* itemTwo)
{
	if (!_comInitialized || !_pSHGetNameFromIDList || !_pSHCreateItemFromIDList)
		return;
	
	bool reload = false;
	HRESULT hr = S_OK;
	QString firstItemName, firstItemKey;
	QString secondItemName, secondItemKey;
	
	if (itemOne)
	{
		PWSTR name = NULL;
		if (SUCCEEDED(_pSHGetNameFromIDList(itemOne, SIGDN_NORMALDISPLAY, &name)))
		{
			firstItemName = QString::fromUtf16((const ushort*) name);
			firstItemKey = QString_NT("lib_%1").arg(firstItemName);
			CoTaskMemFree(name);
		}
	}

	if (itemTwo)
	{
		PWSTR name = NULL;
		if (SUCCEEDED(_pSHGetNameFromIDList(itemTwo, SIGDN_NORMALDISPLAY, &name)))
		{
			secondItemName = QString::fromUtf16((const ushort*) name);
			secondItemKey = QString_NT("lib_%1").arg(secondItemName);
			CoTaskMemFree(name);
		}
	}

	switch (eventId)
	{
	case SHCNE_CREATE:
		{
			LOG(QString_NT("Library '%1' created\n").arg(firstItemName));
			IShellItem* item;
			hr = _pSHCreateItemFromIDList(itemOne, IID_PPV_ARGS(&item));
			if (SUCCEEDED(hr))
			{
				QSharedPointer<Library> newLibrary = createLibrary(item);
				if (newLibrary)
				{
					if (_libraries.value(firstItemKey))
						_libraries.remove(firstItemKey);
					_libraries.insert(newLibrary->getHashKey(), newLibrary);
				}
				item->Release();
			}
		}
		break;
		
	case SHCNE_RENAMEFOLDER:
		{	
			LOG(QString_NT("Library '%1' renamed to '%2'\n").arg(firstItemName).arg(secondItemName));
			QSharedPointer<Library> oldLibrary = _libraries.value(firstItemKey);
			if (oldLibrary)
			{
				_libraries.remove(firstItemKey);
				QSharedPointer<LibraryImpl> libImpl = oldLibrary.dynamicCast<LibraryImpl>();
				libImpl->setName(secondItemName);
				libImpl->setHashKey(secondItemKey);
				_libraries.insert(secondItemKey, oldLibrary);
			}
		}
		break;

	case SHCNE_UPDATEDIR:	
		{
			LOG(QString_NT("Library '%1' folders updated\n").arg(firstItemName));
			QSharedPointer<Library> oldLibrary = _libraries.value(firstItemKey);
			if (oldLibrary)
			{
				oldLibrary.dynamicCast<LibraryImpl>()->setValid(false);
				_libraries.remove(firstItemKey);
			}
				
			IShellItem* item;
			hr = _pSHCreateItemFromIDList(itemOne, IID_PPV_ARGS(&item));
			if (SUCCEEDED(hr))
			{
				QSharedPointer<Library> newLibrary = createLibrary(item);
				if (newLibrary)
					_libraries.insert(newLibrary->getHashKey(), newLibrary);
				item->Release();
			}
		}
		break;
	
	case SHCNE_RMDIR:	
		{
			LOG(QString_NT("Library '%1' deleted\n").arg(firstItemName));
			QSharedPointer<Library> library = _libraries.value(firstItemKey);
			if (library)
			{
				library.dynamicCast<LibraryImpl>()->setValid(false);
				_libraries.remove(firstItemKey);
			}
		}
		break;
	}
}