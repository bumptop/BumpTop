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

#ifndef _BT_LIBRARY_MANAGER_
#define _BT_LIBRARY_MANAGER_

#include "BT_Common.h"
#include "BT_Windows7User32Override.h"
#include "BT_Library.h"
#include "BT_Watchable.h"

#define WM_LIBRARYCHANGE (WM_USER + 117)

class LibraryManager
{
private:
	HMODULE _hMod;
	SHGetKnownFolderItemSignature _pSHGetKnownFolderItem;
	SHGetKnownFolderIDListSignature _pSHGetKnownFolderIDList;
	SHCreateItemFromParsingNameSignature _pSHCreateItemFromParsingName;
	SHGetIDListFromObjectSignature _pSHGetIDListFromObject;
	SHGetNameFromIDListSignature _pSHGetNameFromIDList;
	SHCreateItemFromIDListSignature _pSHCreateItemFromIDList;
	bool _comInitialized;
	ULONG _notifyRegisterId;

	QString _desktopLibraryName;
	
	IShellFolder* _libraryFolder;
	
	QMap< QString, QSharedPointer<Library> > _libraries;
	
private:
	// Loads the Libraries folder and icon enumerator in memory
	bool init();
	
	// Creates Libraries from the Windows7 Libraries
	bool createWin7Libraries();
	
	// Creates Libraries from the common Vista+ KnownFolders
	bool createVistaLibraries();

	// Creates a library from a shell item
	QSharedPointer<Library> createLibrary(IShellItem* item);

	// Creates the default Desktop Library (Works on 7, Vista and XP)
	QSharedPointer<Library> createDesktopLibrary();
	
	// Creates a Folder Library from a Known Folder (Vista)
	QSharedPointer<Library> createLibraryFromKnownFolders(QList<KNOWNFOLDERID>& kfList);

	// Helper function to retrieve the folder paths of a library
	QList<QString> getLibraryFolderPaths(IShellLibrary* library);

	// Helper function to retrieve the icon path of a library
	QString getLibraryIconPath(IShellItem* item);

	PIDLIST_RELATIVE relativePIDLFromAbsolute(IShellFolder* folder, PIDLIST_ABSOLUTE absList);
	
public:
	LibraryManager();
	~LibraryManager();

	const QList< QSharedPointer<Library> > initializeLibraryList();
	
	QSharedPointer<Library> addFolderAsLibrary(PIDLIST_ABSOLUTE folderPidl);
	QSharedPointer<Library> addFolderAsLibrary(QString& folderPath);
	void removeLibrary(QSharedPointer<Library>& library);
	void removeLibrary(QString& key);

	const QList< QSharedPointer<Library> > getLibraries() const;
	QSharedPointer<Library> getLibraryByKey(QString& key) const;
	QSharedPointer<Library> getDesktopLibrary() const;
	QList< QSharedPointer<Library> > getFolderLibraries();
	QList<QString> getFolderLibraryDirectories();

	bool notifyLibraryChanges(bool value);
	void onLibraryChangeEvent(long eventId, ITEMIDLIST* itemOne, ITEMIDLIST* itemTwo);
};
#endif