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
#include "BT_AnimationEntry.h"
#include "BT_AnimationManager.h"
#include "BT_Authorization.h"
#include "BT_Camera.h"
#include "BT_DialogManager.h"
#include "BT_FileSystemActor.h"
#include "BT_FileSystemActorFactory.h"
#include "BT_FileSystemManager.h"
#include "BT_FileSystemPile.h"
#include "BT_FontManager.h"
#include "BT_GLTextureManager.h"
#include "BT_Macros.h"
#include "BT_OverlayComponent.h"
#include "BT_PbPersistenceHelpers.h"
#include "BT_Pile.h"
#include "BT_RenderManager.h"
#include "BT_SceneManager.h"
#include "BT_Selection.h"
#include "BT_StatsManager.h"
#include "BT_StickyNoteMenuActionCustomizer.h"
#include "BT_TextManager.h"
#include "BT_Util.h"
#include "BT_WatchedObjects.h"
#include "BT_WidgetManager.h"
#include "BT_WindowsOS.h"
#include "BumpTop.h"
#include "BumpTop.pb.h"

#ifdef DXRENDER
#include "BT_DXRender.h"
#endif

FileSystemActor::FileSystemActor()
: themeManagerRef(themeManager)
, _minThumbnailDetail(SampledImage)
{
	// This actor is Tied to the File System
	pushActorType(FileSystem);

	pileizedPile = NULL;
	useThumbnail = false;
	numTimesLaunched = 0;
	_isAnimatedTexture = false;
	_mounted = false;
	_loadThumbnailGLTexture = true;
	_serializeThumbnail = false;

	// create an alternate thumbnail id to swap back and forth
	// ensure ID's are unique
	thumbnailID = winOS->GetUniqueID();
	_alternateThumbnailId = winOS->GetUniqueID();
}

FileSystemActor::~FileSystemActor()
{
	if (isPileized())
	{
		// If this icon represents a pile, destroy it
		DeletePile(pileizedPile, false, false, true);
		pileizedPile = NULL;
	}

	if (isThumbnailized())
	{
		// Free some memory by releasing the thumbnail texture
		texMgr->deleteTexture(thumbnailID);

		// free the alternate thumbnail id as well
		texMgr->deleteTexture(_alternateThumbnailId);
	}
}

void FileSystemActor::onLaunch()
{
	assert(!filePath.isNull());

	if (!_onLaunchHandler.empty())
		_onLaunchHandler(this);

	// override for widgets
	Widget * w = widgetManager->getActiveWidgetForFile(getFullPath());
	if (w && w->isWidgetOverrideActor(this))
	{
		w->launchWidgetOverride(this);
		return;
	}
	
	// Do a quick pass to determine what needs to be created or not
	bool isWatchingHighlighted = cam->isWatchedActorHighlighted(this);
	bool zoomIntoImage = isFileSystemType(Image) && !isWatchingHighlighted && texMgr->isTextureState(thumbnailID, TextureLoaded);
	bool launchImage = (isFileSystemType(Image) && isWatchingHighlighted) && !texMgr->isTextureState(thumbnailID, TextureLoaded);
	bool createTemporaryActor = !zoomIntoImage && !launchImage;
	bool createRandomAnimPath = createTemporaryActor;

	Actor * obj = NULL;
	if (createTemporaryActor)
	{
		obj = new Actor();
		Vec3 startPosition;

		// Set up the state of the Actor
		obj->pushActorType(Temporary);
		obj->setDims(getDims());
		obj->setGravity(false);
		obj->setCollisions(false);
		obj->setAlphaAnim(getAlpha(), 0.2f, 40);
		obj->setGlobalPose(getGlobalPose());
		obj->setObjectToMimic(this);
	}

	// Special case for launching a pileized actor
	Vec3 startPosition;
	if (isPileized())
	{
		startPosition = pileizedPile->getGlobalPosition();
	}else{
		startPosition = getGlobalPosition();
	}

	// create random animation path from the icon up to the camera eye
	if (createRandomAnimPath)
	{
		// Set an animation that moves the icon into the camera
 		CreateRandomAnimPath(obj, startPosition, cam->getEye(), 40);

		// Delete the object after the random animation is over.
		animManager->removeAnimation(obj);
		animManager->addAnimation(AnimationEntry(obj, (FinishedCallBack) DeleteActorAfterAnim));
	}

	// handle the launch override if there is one
	if (!getLaunchOverride().isEmpty())
	{
		fsManager->launchFileAsync(getLaunchOverride());
		return;
	}

	// Execute this Icon
	if (!isFileSystemType(Virtual))
	{
		// If this is a folder, then try and browse to it
		if (scnManager->isShellExtension && isFileSystemType(Folder))
		{			
			// try and send a custom message to the proxy window to move to the child
			incrementNumTimesLaunched();
			animManager->finishAnimation(this);
			SaveSceneToFile();
			winOS->ShellExtBrowseToChild(filePath);
			return;
		}
		// This is an image, so zoom to it if we are not already watching it
		else if (zoomIntoImage && isFileSystemType(Image) && texMgr->isTextureState(thumbnailID, TextureLoaded))
		{
			Key_EnableSlideShow();
			this->putToSleep();

			// record this zoom interaction
			statsManager->getStats().bt.interaction.actors.highlightedImage++;
			return;
		}

		// Execute it as normal
		// QString lnkTarget, lnkArgs, lnkWorkingDir;
		bool fileLaunched = false;
		/*
		if (isFileSystemType(Link))
		{
			fsManager->getShortcutTarget(getFullPath(), &lnkTarget, &lnkArgs, &lnkWorkingDir);
			fileLaunched = fsManager->launchFileAsync(lnkTarget, lnkArgs, lnkWorkingDir);
		}
		else
		*/
		fileLaunched = fsManager->launchFileAsync(filePath);

		if (fileLaunched)
		{
			// otherwise, just increment this file launch count and execute it
			// it is decided that images do not auto-grow (was a design decision)
			if (!launchImage)
			{
				incrementNumTimesLaunched();
			}

			// record this launch
			statsManager->getStats().bt.interaction.actors.launchedFile++;
		}
	}
	else
	{
		incrementNumTimesLaunched();
		fsManager->launchFile(filePath);
	}
}

// Sets whether this actor should be thumbnailed at all, and if loadThumbnail
// is true, queues the thumbnail operation on the texture loader.
// If no thumbnail exists, it will use the default filesystem icon.
void FileSystemActor::enableThumbnail(bool enableThumbnail/*=true*/, bool loadThumbnail/*=true*/)
{
	useThumbnail = enableThumbnail;

	// Load this Thumbnail because it wasn't loaded before
	if (useThumbnail)
	{
		GLTextureDetail detail = SampledImage;
		// NOTE: workaround for keeping training images in hires
		// also use hi-res if user wants to skip thumbs db usage
		bool isTrainingImage = getFullPath().startsWith(native(winOS->GetTrainingDirectory()), Qt::CaseInsensitive);
		if (!GLOBAL(settings).useThumbsDb || isTrainingImage)
			detail = HiResImage;

		if (loadThumbnail)
			loadThumbnailTexture(GLTextureObject(Load|Reload, _alternateThumbnailId, getTargetPath(), detail, NormalPriority, true, isFileSystemType(Image)));
	}
}

FileSystemPile *FileSystemActor::pileize()
{
	StrList dirListing;
	QString dirPath;
	vector<Actor *> objListing;
	FileSystemActor *obj = NULL;
	FileSystemPile *p = NULL;

	// Don't allow Piles to be created recursively
	if (isParentType(BumpPile))
	{
		MessageClearPolicy clearPolicy;
			clearPolicy.setTimeout(4);
		scnManager->messages()->addMessage(new Message("pileize_recPiles", QT_TR_NOOP("Sorry, Items within Piles cannot be viewed as Piles at this time.\nThis feature will be implemented in a later version of BumpTop"), Message::Ok, clearPolicy));
		return NULL;
	}

	// If this item has been pileized, then just return its pile
	if (pileizedPile)
	{
		return pileizedPile;
	}

	if (isFileSystemType(Folder))
	{
		// Get a Directory listing of this folder
		dirPath = getTargetPath();
		dirListing = fsManager->getDirectoryContents(dirPath);

		// Check if this Folder has anything in it
		if (dirListing.empty())
		{
			MessageClearPolicy clearPolicy;
				clearPolicy.setTimeout(4);
			scnManager->messages()->addMessage(new Message("pileize_emptyFolder", QT_TR_NOOP("This folder is empty, so it can't be expanded to a pile"), Message::Ok, clearPolicy));
			return NULL;
		}

		// Create a new Pile
		p = new FileSystemPile();
		if (p)
		{
			for (uint i = 0; i < dirListing.size(); i++)
			{
				obj =  FileSystemActorFactory::createFileSystemActor(dirListing[i]);

				// Create new Actors that represent each item in that directory
				// NOTE: we need to set the initial size of the object, since we try and sync the post it
				//		 in the setFilePath call, which means that it will try and fill to the dims of the
				//		 object, which, in it's default size, is not visible text-wise.
				if (_prevPileizedActorDims.contains(dirListing[i].toLower()))
					obj->setDims(Vec3(_prevPileizedActorDims.value(dirListing[i].toLower())));
				else
					obj->setDims(getDims());
				obj->setGlobalPose(getGlobalPose());
				obj->setFilePath(dirListing[i]);

				objListing.push_back(obj);
			}

			// Add items to this Pile
			for (uint i = 0; i < objListing.size(); i++)
			{
				p->addToPile(objListing[i]);
			}

			// Save and setup initial states
			p->setOwner(this);
			p->setText(getFullText());
			p->stack(getGlobalPosition());
			
			// set the icon to be this actor's 
			if (isFileSystemType(Folder))
				p->setTextIcon(getTextureID());

			// Create custom Animations
			for (uint i = 0; i < objListing.size(); i++)
			{
				objListing[i]->setAlphaAnim(0.0f, 1.0f, 15);
			}

			// Make this actor Non-existent
			this->hideAndDisable();

			// Finish up by setting the pile as the current selection
			pileizedPile = p;
			sel->remove((BumpObject *) this);
			sel->add((Pile *) p);

			textManager->invalidate();

			// record this pilization
			statsManager->getStats().bt.interaction.piles.pilized++;
			return p;
		}		
	}

	return NULL;
}

void FileSystemActor::setPileizedPile(FileSystemPile *p)
{
	pileizedPile = p;
}

void FileSystemActor::setFilePath(QString fullPath, bool skipTextureResolution /*=false*/)
{	
	if (_isAnimatedTexture)
		_animatedTextureSource.setPath(fullPath);

	// Save the Path (or Virtual Folder Name)
	filePath = fullPath;
	winOS->GetShortPathName(fullPath, shortPath);
	if (skipTextureResolution)
		return;

	// resolve the texture to load for this file if there is one
	QString ext = fsManager->getFileExtension(fullPath);
	QString texId;
	GLTextureDetail detail = FileIcon;
	GLTextureLoadPriority priority = NormalPriority;

	bool isVista = winOS->IsWindowsVersionGreaterThanOrEqualTo(WindowsVista);
	bool overrideSystemTextures = GLOBAL(settings).useThemeIconOverrides;
	int virtualIconId = winOS->GetIconTypeFromFileName(fullPath);
	if (virtualIconId > -1)
	{
		// mark this is a virtual icon
		pushFileSystemType(Virtual);
		pushFileSystemType(Folder);

		// check if we are overloading any virtual icons (only My Computer for now)
		if (overrideSystemTextures && 
			(virtualIconId == MyComputer) &&
			texMgr->hasTexture("override.virtual.mycomputer"))
		{
			texId = QT_NT("override.virtual.mycomputer");
			detail = HiResImage;
		}
		else
		{
			// otherwise, we will just load the icon later
			texId = fullPath;

			// NOTE: we force load these icons here because we do not do so if the 
			// texture id is set below
			loadThumbnailTexture(GLTextureObject(Load, texId, texId, FileIcon, priority, false));
		}
	}
	else
	{
		// not a virtual icon, just a random icon then
		unsigned int fileAttributes = fsManager->getFileAttributes(fullPath);

		// delete this object if it doesn't exist (and it's not a photo frame or volume)
		if (!fileAttributes &&
			!(isFileSystemType(PhotoFrame) || isFileSystemType(LogicalVolume)))
		{
			animManager->addAnimation(AnimationEntry(this, (FinishedCallBack) DeleteActorAfterAnim));
			setAlpha(0.0f);
			return;
		}

		// make sure there's no lingering animations
		animManager->removeAnimation(this);
		setAlpha(1.0f);

		// XXX: check if we are using animated textures
		// _isAnimatedTexture = (fileExtension == ".gif");

		// check if this is a shortcut
		// NOTE: if it is a valid shortcut, the file attributes and extension
		//		 now refer to the target and not the shortcut itself
		if (fileAttributes && ext == ".lnk")
		{
			// resolve the shortcut target
			fsManager->getShortcutTarget(fullPath, &lnkFullPath);
			if (fsManager->isValidFileName(lnkFullPath))
			{
				pushFileSystemType(Link);
				popFileSystemType(DeadLink);

				fileAttributes = fsManager->getFileAttributes(lnkFullPath);
				ext = fsManager->getFileExtension(lnkFullPath);
			}
			else
			{
				pushFileSystemType(DeadLink);
			}
		}


		// check if it is a folder
		if (fileAttributes & Directory)
		{
			pushFileSystemType(Folder);

			// XXX: only override shortcuts, and not folders?
			/*
			if (!overrideSystemTextures || !enableFileTypeIconsForShortcuts)
			{
				texId = winOS->GetSystemIconInfo(getFullPath());
			}
			else 
			*/
			if (overrideSystemTextures)
			{
				texId = QT_NT("override.ext.folder");
				detail = HiResImage;
			}		
		}
		else
		{
			// normal file
			pushFileSystemType(File);
			hasExtension(true); //only files have extension, so the nameable extension hide only applies here

			// resolve some information about the file
			if (ext.size() > 0)
			{
				if (ext == ".exe")
					pushFileSystemType(Executable);
				else
				{
					// XXX: check if it's a document
					// pushFileSystemType(Document);

					if (overrideSystemTextures)
					{
						QString potentialOverrideTex = QString(QT_NT("override.ext")) + ext;
						if (texMgr->hasTexture(potentialOverrideTex))
						{
							texId = potentialOverrideTex;
							detail = HiResImage;
						}
					}
				}

				// load the thumbnail if this is an image
				// NOTE: we append the period because if the extension is empty
				// the search is always true
				if (GLOBAL(supportedExtensions).contains(ext + "."))
				{
					if (!isThumbnailized())
						enableThumbnail(true, !winOS->IsFileInUse(fullPath));
					pushFileSystemType(Image);
					pushFileSystemType(Thumbnail);
					hideText(true);
				}
			}
		}
	}

	// at this point, resolve the file icon texture id if there was no override
	if (texId.isEmpty())
	{
		texId = winOS->GetSystemIconInfo(fullPath);

		// mark the texture for loading
		loadThumbnailTexture(GLTextureObject(Load, texId, texId, detail, priority,false));
	}
	setTextureID(texId);

	// we also want to try and load thumbnails for normal files if they exist
	// (as long as it's not a widget file)
	Widget * w = widgetManager->getActiveWidgetForFile(fullPath);
	if (!isThumbnailized() && (detail == FileIcon) && !w)
	{
		FileSystemActorType typesToIgnore = FileSystemActorType(Executable | Virtual);
		// on vista, just queue the thumbnail for loading
		if (isVista && !isFileSystemType(typesToIgnore))
		{
			QString ext = fsManager->getFileExtension(getTargetPath());
			loadThumbnailTexture(GLTextureObject(Load|Reload, _alternateThumbnailId, getTargetPath(), SampledImage, IdlePriority));
		}
		// on windows xp, check if the thumbs db has a record first
		else if (winOS->IsWindowsVersion(WindowsXP))
		{
			if (texMgr->hasWinThumbnail(getTargetPath()))
			{
				loadThumbnailTexture(GLTextureObject(Load|Reload, _alternateThumbnailId, getTargetPath(), SampledImage, IdlePriority));
			}
		}
	}

	// XXX: (disabled) set the initial dimensions and weight of this file based on it's size
	// setDimsFromFileSize(this);

	// set the text
	if(!isFileSystemType(PhotoFrame)) {
		if (w && w->isWidgetOverrideActor(this))
		{
			setText(w->getWidgetOverrideLabel(this));
			Vec3 actorDims = getDims();
			float aspect = (actorDims.x / actorDims.y);
			Vec3 dims(GLOBAL(settings).xDist, GLOBAL(settings).zDist / aspect, GLOBAL(settings).yDist);
			float scale = w->getWidgetOverrideScale(this);
			if (scale > 0.0f)
			{
				dims *= scale;
				setSizeAnim(getDims(), dims, 25);
			}
		}
		else
			setText(getFileName(isFileSystemType(Link) || isFileSystemType(DeadLink)));
	}
	setRespectIconExtensionVisibility(!isFileSystemType(Folder));


	// New name was set, invalidate text
	textManager->invalidate();
	rndrManager->invalidateRenderer();
}

void FileSystemActor::setNumTimesLaunched(int timesLaunched)
{
	numTimesLaunched = timesLaunched;
}

void FileSystemActor::incrementNumTimesLaunched()
{
	numTimesLaunched++;
}

void FileSystemActor::pushFileSystemType(FileSystemActorType fsType)
{
	type.ternaryType |= fsType;
}

void FileSystemActor::popFileSystemType(FileSystemActorType fsType)
{
	type.ternaryType &= ~fsType;
}

QString FileSystemActor::getFullPath() const
{
	return filePath;
}

QString FileSystemActor::getShortPath()
{
	return shortPath;
}

QString FileSystemActor::getTargetPath() const
{
	return isFileSystemType(Link) ? getLinkTarget() : getFullPath();
}

QString FileSystemActor::getFileName(bool truncExt)
{
	if (isFileSystemType(Virtual))
	{
		return filePath;
	}else{
		if (truncExt)
		{
			// Return the name minus the file Extension
			QFileInfo fileInfo(filePath);
			return fileInfo.completeBaseName();
		}else{
			// Return just the file name with extension
			return filename(filePath);
		}
	}
}

bool FileSystemActor::isFileSystemType(FileSystemActorType fsType) const
{
	return (type.ternaryType & fsType) ? true : false;
}

QString FileSystemActor::getThumbnailID() const 
{
	return thumbnailID;
}

GLTextureDetail FileSystemActor::getMinThumbnailDetail() const
{
	return _minThumbnailDetail;
}

QString FileSystemActor::getAlternateThumbnailId() const
{
	return _alternateThumbnailId;
}

Vec3 FileSystemActor::getDefaultDims()
{
	Vec3 dims(GLOBAL(settings).xDist, GLOBAL(settings).zDist, GLOBAL(settings).yDist);
	dims *= scnManager->defaultActorGrowFactor;
	return dims;
}

bool FileSystemActor::isPilable(uint pileType)
{
	// Virtual Folders are not allowed to be moved into hard piles
	if ((pileType == HardPile) && isFileSystemType(Virtual))
	{
		return false;
	}else{
		return true;
	}

	return false;
}

bool FileSystemActor::isPileized()
{
	return (pileizedPile != NULL) ? true : false;
}

bool FileSystemActor::isThumbnailized()
{
	return useThumbnail;
}

#ifdef DXRENDER
IDirect3DTexture9 * FileSystemActor::getTextureNum()
#else
uint FileSystemActor::getTextureNum()
#endif
{	
	QString texID = textureID;

	if (useThumbnail && texMgr->isTextureState(thumbnailID, TextureLoaded))
	{
		// Only show the thumbnail if the thumbnail is loaded
		return texMgr->getGLTextureId(thumbnailID);
	}

	return texMgr->getGLTextureId(texID);
}

FileSystemPile * FileSystemActor::getPileizedPile()
{
	return pileizedPile;
}

bool FileSystemActor::isCopyIntoActor(const vector<BumpObject *> &objList) const
{
	bool copyFile = !fsManager->hasCommonRoots(this, objList); // common roots (same partition) defaults to move
	copyFile |= winOS->IsKeyDown(KeyControl); // Shift forces move, Ctrl forces copy
	copyFile &= !winOS->IsKeyDown(KeyShift);
	return copyFile;
}
vector<BumpObject *> FileSystemActor::onDrop(vector<BumpObject *> &objList)
{
	QString fPath;
	FileSystemActor *fsData;
	Pile *pile;
	FileSystemPile *fsPile;
	Vec3 topPt;
	bool moveItem = true;
	bool operationSucceeded = false;
	vector<FileSystemActor *> failedObj, fsObjList;
	vector<BumpObject *> failedBumpObjs;
	vector<FileSystemActor *> hardPileOwners;

	if (!isSourceValid())
	{
		return objList;
	}
	else
	{	
		// Convert Piles to Free Items
		for (int i = 0; i < objList.size(); i++)
		{
			// Do Pile to free Item conversions
			if (objList[i]->getObjectType() == ObjectType(BumpPile, HardPile, Stack))
			{
				// If its a Hard Pile, get its owner and use it instead of pile members
				fsPile = (FileSystemPile *) objList[i];

				objList.erase(objList.begin() + i);
				objList.push_back(fsPile->getOwner());
				hardPileOwners.push_back(fsPile->getOwner());

				fsPile->folderize(false);

				i--;

			}else if (objList[i]->getObjectType() == ObjectType(BumpPile, SoftPile, Stack))
			{
				pile = (Pile *) objList[i];

				objList.erase(objList.begin() + i);
				i--;

				// If its a Soft Pile, use its members instead of the pile
				for (uint j = 0; j < pile->getNumItems(); j++)
				{
					objList.push_back((*pile)[j]);
				}
			}
		}

		if (isFileSystemType(Executable))
		{
			for (uint i = 0; i < objList.size(); i++)
			{
				// Create a parameter list separated by spaces
				fsData = (FileSystemActor *) objList[i];

				if (!fsData || !scnManager->containsObject(fsData))
					continue;

				fPath.append(fsData->getFullPath());
				fPath.append(" ");
			}

			// We just tossed into an executable
			// QString lnkTarget, dummyLnkArgs, lnkWorkingDir;
			QString lnkArgs = fPath;
			/*
			if (isFileSystemType(Link))
			{
				fsManager->getShortcutTarget(getFullPath(), &lnkTarget, &dummyLnkArgs, &lnkWorkingDir);
				fsManager->launchFileAsync(lnkTarget, lnkArgs, lnkWorkingDir);
			}
			else
			*/
				fsManager->launchFileAsync(filePath, lnkArgs);

		}else if (isFileSystemType(Folder))
		{
			bool itemNeedsPrompt = false;
			bool onlyPhotoFrame = true;

			int iconType = winOS->GetIconTypeFromFileName(getFullPath());
			bool isRecycleBin = iconType == RecycleBin;
			bool isMyDocuments = iconType == MyDocuments;

			// Convert BumpObject to FileSystemActors, delete WebActors since they are not FileSystemActors
			for (uint i = 0; i < objList.size(); i++)
			{
				if (objList[i]->isObjectType(ObjectType(BumpActor, Webpage)))
				{
					if (isRecycleBin)
					{
						sel->remove(objList[i]);
						objList[i]->markDragCancelled(true);
						objList[i]->onDragEnd();
						FadeAndDeleteActor((Actor *)objList[i]);
						objList.erase(objList.begin() + i);
					}
					else
						_ASSERT(0);
				}
				else
				{
					fsObjList.push_back((FileSystemActor *) objList[i]);

					if (fsObjList[i]->getObjectType() == ObjectType(BumpActor, FileSystem, PhotoFrame))
						itemNeedsPrompt = true;
					else
						onlyPhotoFrame = false;
				}
			}

			if (isFileSystemType(Virtual))
			{
				// Handle tossing into the recycle bin
				if (isRecycleBin || isMyDocuments)
				{
					if (itemNeedsPrompt)
					{
						dlgManager->clearState();
						dlgManager->setPrompt(QT_TR_NOOP("A Photo Frame was detected in the selection, would you like to delete it?"));
						dlgManager->setCaption(QT_NT("BumpTop"));

						if (dlgManager->promptDialog(DialogYesNo))
						{
							vector<FileSystemActor *>::iterator iter = fsObjList.begin();
							while (iter != fsObjList.end())
							{
								FileSystemActor * fsActor = *iter;
								if (fsActor->getObjectType() == ObjectType(BumpActor, FileSystem, PhotoFrame))
								{
									// Remove the photo frame by deferring deletion until after the anim is over
									fsActor->fadeOut();
									animManager->removeAnimation(fsActor);
									animManager->addAnimation(AnimationEntry(fsActor, (FinishedCallBack) DeleteActorAfterAnim, NULL, true));

									iter = fsObjList.erase(iter);
								}
								else
									iter++;
							}
						}
						else
						{
							vector<FileSystemActor *>::iterator iter = fsObjList.begin();
							while (iter != fsObjList.end())
							{
								FileSystemActor * fsActor = *iter;
								if (fsActor->getObjectType() == ObjectType(BumpActor, FileSystem, PhotoFrame))
								{
									failedObj.push_back(fsActor);
									iter = fsObjList.erase(iter);
								}
								else
									iter++;
							}
						}
					}

					if (!fsObjList.empty())
					{
						if (isRecycleBin)
							operationSucceeded = fsManager->deleteFiles(fsObjList, failedObj, !onlyPhotoFrame);
						else
							operationSucceeded = fsManager->moveFiles(fsObjList, winOS->GetSystemPath(iconType), failedObj, true);
					}
				}
			}else{
				if (!isCopyIntoActor(objList)) // Shift forces move, Ctrl forces copy, common root defaults to move
				{
					// Handle tossing into a regular folder
					operationSucceeded = fsManager->moveFiles(fsObjList, getTargetPath(), failedObj, true);
					if (operationSucceeded)
					{
						printUnique("FileSystemActor::onDrop", QT_TR_NOOP("%1 file(s) moved to %2").arg(fsObjList.size()).arg(getText()));
					}
				}
				else
				{
					// Handle tossing into a regular folder on another resource (will copy instead of move)
					operationSucceeded = fsManager->copyFiles(fsObjList, getTargetPath(), failedObj, true);
					if (operationSucceeded)
					{
						printUnique("FileSystemActor::onDrop", QT_TR_NOOP("%1 file(s) copied to %2").arg(fsObjList.size()).arg(getText()));
					}

					// Animate back to the original starting pose
					animateObjectsBackToPreDropPose(objList);
				}
			}
		}

		if (!operationSucceeded)
		{
			// re-pileize hard piles if move failed
			vector<FileSystemActor *>::iterator iter = hardPileOwners.begin();
			while (iter != hardPileOwners.end())
			{
				(*iter)->pileize();
				iter++;
			}
		}

		// record this drop
		statsManager->getStats().bt.interaction.dragAndDrop.toActor++;
	}

	// Convert from a list of FileSYstemActors to BumpObjects
	for (uint i = 0; i < failedObj.size(); i++)
	{
		failedBumpObjs.push_back(failedObj[i]);
	}

	return failedBumpObjs;
}

QString FileSystemActor::resolveDropOperationString(vector<BumpObject *>& objList)
{
	// if this is a recycle bin, then show "delete files"
	if (isFileSystemType(Virtual) && 
		winOS->GetIconTypeFromFileName(getFullPath()) == RecycleBin)
	{
		return QT_TR_NOOP("Delete");
	}
	else if (isFileSystemType(Executable))
	{
		return QT_TR_NOOP("Launch %1").arg(getFileName(true));
	}
	else if (isFileSystemType(Link))
	{
		// check if we are pointing to a folder or not
		if (!QFileInfo(getTargetPath()).isDir())
		{
			return QT_TR_NOOP("Launch %1").arg(getFileName(true));
		}
	}
	else
	{
		// check if the root of the items in the object list is different
		// than that of this item
		if (isCopyIntoActor(objList)) // Shift forces move, Ctrl forces copy, common root defaults to move
		{
			return QT_TR_NOOP("Copy");
		}
	}
	return Actor::resolveDropOperationString(objList);
}

bool FileSystemActor::isValidDropTarget()
{	
	// Conditional List
	if (isFileSystemType(DeadLink)) return false;
	if (!isFileSystemType(Folder) && !isFileSystemType(Executable) && !isFileSystemType(Link)) return false;

	int iconType = winOS->GetIconTypeFromFileName(getFullPath());
	if (isFileSystemType(Virtual) && (iconType != RecycleBin  && iconType != MyDocuments)) return false;
	if (isFileSystemType(Link) && (isFileSystemType(Executable) || isFileSystemType(Folder))) return true;
	if (isFileSystemType(Removable) && !isMounted()) return false;

	return true;
}

bool FileSystemActor::isSourceValid()
{
	FileSystemActor *fsData = NULL;
	Actor *data = NULL;

	for (uint i = 0; i < source.size(); i++)
	{
		// Check to see if the icon is a filesystem icon
		if (source[i]->isBumpObjectType(BumpActor))
		{
			data = (Actor *) source[i];

			if (data->isActorType(FileSystem))
			{
				fsData = (FileSystemActor *) source[i];

				// Exclude Virtual folders
				if (fsData->isFileSystemType(Virtual)) return false;
				if (fsData->isFileSystemType(LogicalVolume)) return false;
				if (fsData == this) return false;
			}
			else if (data->isActorType(Webpage) && RecycleBin == winOS->GetIconTypeFromFileName(getFullPath()))
				return true; // can drop WebActor into Recycling Bin to delete
			else
				return false;

		}else if (source[i]->isBumpObjectType(BumpPile))
		{
			Pile *pile = (Pile *) source[i];

			// Check for any items that cannot be moved (ie. Virtual folders)
			for (uint i = 0; i < pile->getNumItems(); i++)
			{
				if (!(*pile)[i]->isPilable(HardPile))
				{
					return false;
				}
			}

			// Ignore gridded items because they are on the dynamic plane, above
			// all other items on the floor.
			if (pile->getPileState() == Grid) return false;

			// Allow Piles
			return true;
		
		}else{
			// Logical items are not allowed
			return false;
		}
	}

	return true;
}

QString FileSystemActor::getLinkTarget() const
{
	return lnkFullPath;
}

void FileSystemActor::onRender(uint flags)
{
#ifdef TESTING_ALTERNATE_RENDERING
	
	if (isActorType(Invisible))
		return;

	// _isAnimatedTexture is always false right now since we don't robustly support animated gifs
	if (_isAnimatedTexture)
	{
		bool successful = true;
		
		// if this is an animated texture and we've never loaded the animation
		// then do so now

		if (!_animatedTextureSource.load())
			_isAnimatedTexture = false;

		if (_animatedTextureSource.numFrames() <= 1)
			successful = false;
		else
		{
			AnimatedTextureFrame * frame = _animatedTextureSource.getCurrentTextureFrame();
			Vec3 sz = getDims();

			glPushAttribToken token(GL_ENABLE_BIT);
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_BLEND);
			glEnable(GL_TEXTURE_2D);

			// Conditionals
			if (isSelected() || isActorType(Temporary)) glDisable(GL_DEPTH_TEST);
			if (isActorType(Invisible)) return;

			//transform our unit cube:
			glPushMatrix();

				ShapeVis::setupGLMatrix(getGlobalPosition(), getGlobalOrientation());
				glScaled(sz.x, sz.y, sz.z);

				// render the current index
				glBindTexture(GL_TEXTURE_2D, _animatedTextureSource.GLId());			
				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame->width, frame->height, GL_BGRA, GL_UNSIGNED_BYTE, frame->pixelData);
				
				// Set the Texture
				glColor4f(1, 1, 1, getAlpha());

				// Scale the image
				uint squareSize = closestPowerOfTwo(NxMath::max(frame->width, frame->height));
				float w = (float) frame->width / squareSize;
				float h = (float) frame->height / squareSize;
				h = 1.0f - (h * w);

				glBegin(GL_QUADS);
					glNormal3f(0,0,1);
					glTexCoord2f(w,0); 	glVertex3f(1,1,1);
					glTexCoord2f(0,0); 	glVertex3f(-1,1,1);
					glTexCoord2f(0,h); 	glVertex3f(-1,-1,1);
					glTexCoord2f(w,h); 	glVertex3f(1,-1,1);

					glNormal3f(0,0,-1);
					glTexCoord2f(0,h); 	glVertex3f(1,-1,-1);
					glTexCoord2f(w,h); 	glVertex3f(-1,-1,-1);
					glTexCoord2f(w,0); 	glVertex3f(-1,1,-1);
					glTexCoord2f(0,0); 	glVertex3f(1,1,-1);
				glEnd();

			glPopMatrix();
		}

		// skip the normal rendering routine
		if (successful)
			return;
	}

	// Render the previous actor first
	Actor::onRender(flags);

	// Render an overlay
	bool isLinkOverlay = (isFileSystemType(Link) || isFileSystemType(DeadLink)) && GLOBAL(settings).manualArrowOverlay;
	bool isFileTypeOverlay = !getTextIcon().isEmpty();
	
	if (isLinkOverlay || isFileTypeOverlay)
	{
		glPushAttribToken token(GL_ENABLE_BIT);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glEnable(GL_TEXTURE_2D);

		// Render the shortcut/file type icon Arrow
		QString renderTextureId = QT_NT("icon.linkOverlay");
		Vec3 renderPos = getGlobalPosition();
		Vec3 renderDims = getDims();
		bool shouldScaleIcon = false;
		if (isFileTypeOverlay)
		{
			renderTextureId = getTextIcon();
			shouldScaleIcon = true;
		}
		else if (isLinkOverlay)
		{
			// If the creator of the theme has a bad icon link overlay, we need to re-size it so it does not take up the entire actor
			// A "good" icon link overlay is 256x255; with the actual overlay part taking up 77x75 in the bottom-left corner
			Vec3 dimensions = texMgr->getTextureDims(renderTextureId);
			if (dimensions.x > 0 && dimensions.y > 0) 
			{
				if ((closestPowerOfTwo(dimensions.x) != 256) || (closestPowerOfTwo(dimensions.y) != 256))
				{
					shouldScaleIcon = true;
					
					// position the link icon to the corner of the actual actor (instead of scaling/stretching it)
					float maxSide = max(dimensions.x, dimensions.y);
					renderDims.x = renderDims.y = maxSide;
				}
			}
		}

		if (shouldScaleIcon)
		{
			Vec3 offset(getDims().x, -getDims().y, 0);
			
			// Scale the actor to proper size (256/77 =~ 3.4; 255/75 = 3.4)
			renderDims.x /= 3.4f;
			renderDims.y /= 3.4f;
			
			offset += Vec3(-renderDims.x - 0.25f, renderDims.y + 0.25f, 0);
			offset = getGlobalOrientation() * offset;
			renderPos += offset;
		}
		glBindTexture(GL_TEXTURE_2D, texMgr->getGLTextureId(renderTextureId));
		glColor4f(1, 1, 1, getAlpha());
		ShapeVis::renderSideLessBox(renderPos, getGlobalOrientation(), renderDims);
	}
#elif !defined DXRENDER
	// _isAnimatedTexture is always false right now since we don't robustly support animated gifs
	if (_isAnimatedTexture)
	{
		bool successful = true;
		
		// if this is an animated texture and we've never loaded the animation
		// then do so now

		if (!_animatedTextureSource.load())
			_isAnimatedTexture = false;

		if (_animatedTextureSource.numFrames() <= 1)
			successful = false;
		else
		{
			AnimatedTextureFrame * frame = _animatedTextureSource.getCurrentTextureFrame();
			Vec3 sz = getDims();

			glPushAttribToken token(GL_ENABLE_BIT);
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_BLEND);
			glEnable(GL_TEXTURE_2D);

			// Conditionals
			if (isSelected() || isActorType(Temporary)) glDisable(GL_DEPTH_TEST);
			if (isActorType(Invisible)) return;

			//transform our unit cube:
			glPushMatrix();

				ShapeVis::setupGLMatrix(getGlobalPosition(), getGlobalOrientation());
				glScaled(sz.x, sz.y, sz.z);

				// render the current index
				glBindTexture(GL_TEXTURE_2D, _animatedTextureSource.GLId());			
				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame->width, frame->height, GL_BGRA, GL_UNSIGNED_BYTE, frame->pixelData);
				
				// Set the Texture
				glColor4f(1, 1, 1, getAlpha());

				// Scale the image
				uint squareSize = closestPowerOfTwo(NxMath::max(frame->width, frame->height));
				float w = (float) frame->width / squareSize;
				float h = (float) frame->height / squareSize;
				h = 1.0f - (h * w);

				glBegin(GL_QUADS);
					glNormal3f(0,0,1);
					glTexCoord2f(w,0); 	glVertex3f(1,1,1);
					glTexCoord2f(0,0); 	glVertex3f(-1,1,1);
					glTexCoord2f(0,h); 	glVertex3f(-1,-1,1);
					glTexCoord2f(w,h); 	glVertex3f(1,-1,1);

					glNormal3f(0,0,-1);
					glTexCoord2f(0,h); 	glVertex3f(1,-1,-1);
					glTexCoord2f(w,h); 	glVertex3f(-1,-1,-1);
					glTexCoord2f(w,0); 	glVertex3f(-1,1,-1);
					glTexCoord2f(0,0); 	glVertex3f(1,1,-1);
				glEnd();

			glPopMatrix();
		}

		// skip the normal rendering routine
		if (successful)
			return;
	}

	// Render the previous actor first
	Actor::onRender(flags);

	// Render a link overlay arrow
	if ((isFileSystemType(Link) || isFileSystemType(DeadLink)) && !isActorType(Invisible))
	{
		glPushAttribToken token(GL_ENABLE_BIT);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glEnable(GL_TEXTURE_2D);

		// Render the shortcut Arrow
		if (GLOBAL(settings).manualArrowOverlay)
		{
			glBindTexture(GL_TEXTURE_2D, texMgr->getGLTextureId("icon.linkOverlay"));

			bool scaleOverlayActor = false;

			//If the creator of the theme has a bad icon link overlay, we need to re-size it so it does not take up the entire actor
			Vec3 dimensions = texMgr->getTextureDims("icon.linkOverlay");

			//A "good" icon link overlay is 256x255; with the actual overlay part taking up 77x75 in the bottom-left corner
			if(dimensions.x > 0 && dimensions.y > 0) {
				if((closestPowerOfTwo(dimensions.x) != 256)||(closestPowerOfTwo(dimensions.y) != 256))
					scaleOverlayActor = true;
			}

			glColor4f(1, 1, 1, getAlpha());

			// Render Shape
			Vec3 pos = getGlobalPosition();
			Vec3 dims = getDims();
			Vec3 normalizedDims = dims;

			// position the link icon to the corner of the actual actor (instead of scaling/stretching it)
			float maxSide = max(dims.x, dims.y);
			normalizedDims.x = normalizedDims.y = maxSide;

			// Scale the actor to proper size (256/77 =~ 3.4; 255/75 = 3.4)
			if(scaleOverlayActor){
				normalizedDims.x /= 3.4f;
				normalizedDims.y /= 3.4f;
			}
			
			if(scaleOverlayActor) {
				Vec3 offset(dims.x, -dims.y, 0);
				offset = getGlobalOrientation() * offset;
				pos += offset;
			} else {
				pos.x -= normalizedDims.x - dims.x;
				pos.z += normalizedDims.y - dims.y;
			}

			ShapeVis::renderSideLessBox(pos, getGlobalOrientation(), normalizedDims);
		}
	}
#elif defined DXRENDER
	// Render the previous actor first
	Actor::onRender(flags);

	// Render a link overlay arrow
	if ((isFileSystemType(Link) || isFileSystemType(DeadLink)) && !isActorType(Invisible))
	{
		// Render the shortcut Arrow
		if (GLOBAL(settings).manualArrowOverlay)
		{
			bool scaleOverlayActor = false;

			//If the creator of the theme has a bad icon link overlay, we need to re-size it so it does not take up the entire actor
			Vec3 dimensions = texMgr->getTextureDims("icon.linkOverlay");

			//A "good" icon link overlay is 256x255; with the actual overlay part taking up 77x75 in the bottom-left corner
			if(dimensions.x > 0 && dimensions.y > 0) {
				if((closestPowerOfTwo(dimensions.x) != 256)||(closestPowerOfTwo(dimensions.y) != 256))
					scaleOverlayActor = true;
			}

			// Render Shape
			Vec3 pos = getGlobalPosition();
			Vec3 dims = getDims();
			Vec3 normalizedDims = dims;

			// position the link icon to the corner of the actual actor (instead of scaling/stretching it)
			float maxSide = max(dims.x, dims.y);
			normalizedDims.x = normalizedDims.y = maxSide;

			// Scale the actor to proper size (256/77 =~ 3.4; 255/75 = 3.4)
			if(scaleOverlayActor){
				normalizedDims.x /= 3.4f;
				normalizedDims.y /= 3.4f;
			}
			
			if(scaleOverlayActor) {
				Vec3 offset(dims.x, -dims.y, 0);
				offset = getGlobalOrientation() * offset;
				pos += offset;
			} else {
				pos.x -= normalizedDims.x - dims.x;
				pos.z += normalizedDims.y - dims.y;
			}
			
			dxr->device->SetRenderState(D3DRS_ZENABLE, false);
			dxr->renderSideLessBox(pos, getGlobalOrientation(), normalizedDims, texMgr->getGLTextureId("icon.linkOverlay"));
			dxr->device->SetRenderState(D3DRS_ZENABLE, true);
		}
	}
#endif
}

void FileSystemActor::setThumbnailID(QString id)
{
	thumbnailID = id;
}

void FileSystemActor::setMinThumbnailDetail(GLTextureDetail detail)
{
	_minThumbnailDetail = detail;
}
 
void FileSystemActor::onTossRecieve(vector<BumpObject *> tossedObjs)
{
	vector<BumpObject *> failedObj = onDrop(tossedObjs);
	animateObjectsBackToPreDropPose(failedObj);
}
bool FileSystemActor::isValidTossTarget()
{
	if (isActorType(Invisible))
		return false;

	// Only the recycle bin can be tossed to
	if (isFileSystemType(Virtual))
	{
		int iconType = winOS->GetIconTypeFromFileName(getFullPath());
		return (iconType == RecycleBin) || (iconType == MyDocuments);
	}
	else if (isFileSystemType(LogicalVolume))
		return true;

	return false;
}

bool FileSystemActor::isValidToss(vector<BumpObject *> tossedObjs)
{
	bool rc = false;

	// Use the Drop logic to determine valid tosses
	source = tossedObjs;
	rc = isSourceValid();
	source.clear();

	return rc;
}


bool FileSystemActor::allowNativeContextMenu() const
{
	return !isFileSystemType(LogicalVolume);
}

void FileSystemActor::onTextureLoadComplete(QString textureKey)
{
	QString tmpId = getThumbnailID();
	if (textureKey == tmpId)
		return;

	// swap (free the previous texture id, and set this one as the current)
	texMgr->deleteTexture(tmpId);
	setThumbnailID(_alternateThumbnailId);
	_alternateThumbnailId = tmpId;
}

void FileSystemActor::onTextureLoadError(QString textureKey)
{
	// do nothing (do not swap the current image if loading the next one fails)
}

void FileSystemActor::refreshThumbnail()
{
	if (winOS->IsFileInUse(getFullPath()))
		return;

	if (isThumbnailized())
	{
		Widget * w = widgetManager->getActiveWidgetForFile(getFullPath());
		if (w)
			loadThumbnailTexture(GLTextureObject(Load|Reload, _alternateThumbnailId, filePath, HiResImage, NormalPriority));
		else
			loadThumbnailTexture(GLTextureObject(Load|Compress, _alternateThumbnailId, getTargetPath(), getMinThumbnailDetail(), NormalPriority));
	}
	else
		loadThumbnailTexture(GLTextureObject(Load|Reload, textureID, getTargetPath(), FileIcon, NormalPriority));
}



bool FileSystemActor::hasAnimatedTexture() const
{
	return false;
	// XXX: disable animated textures
	// return _isAnimatedTexture;
}

void FileSystemActor::updateAnimatedTexture()
{
	_animatedTextureSource.onUpdate();
}

void FileSystemActor::playAnimatedTexture( bool play )
{
	if (play)
		_animatedTextureSource.play();
	else
		_animatedTextureSource.pause();
}

bool FileSystemActor::isPlayingAnimatedTexture() const
{
	return _animatedTextureSource.isPlaying();
}

void FileSystemActor::setTextureOverride( QString path )
{
	// set the override file path
	_overrideTexturePath = path;	
	if (!path.isEmpty())
	{
		loadThumbnailTexture(GLTextureObject(Load|Compress|Reload, _alternateThumbnailId, _overrideTexturePath, HiResImage, NormalPriority));

		if (!isThumbnailized())
			setTextureID(_alternateThumbnailId);
	}
}

bool FileSystemActor::getTextureOverride( QString& path )
{
	path = _overrideTexturePath;
	return !path.isEmpty();
}

void FileSystemActor::setOnLaunchHandler( boost::function<void(FileSystemActor* actor)> onLaunchHandler )
{
	_onLaunchHandler = onLaunchHandler;
}

bool FileSystemActor::isMounted() const
{
	return _mounted;
}

void FileSystemActor::setMounted( bool mount, QString name )
{
	_mounted = mount;
	if (_mounted)
	{
		if (isActorType(Invisible))
		{
			popActorType(Invisible);
			setText(name);

			QString msg = QT_TR_NOOP("Device '%1' attached").arg(getText());
			printUnique("FileSystemActor::setMounted", msg);
		}
	}
	else
	{
		if (!isActorType(Invisible))
		{
			pushActorType(Invisible);

			QString msg = QT_TR_NOOP("Device '%1' removed").arg(getText());
			printUnique("FileSystemActor::setMounted", msg);
		}
	}
}

// Add a <String, Vector> to the _prevPileizedActorDims list. The string represents the path
// to the file we are storing the dims for, the Vec3 is a vector representing the dimension
void FileSystemActor::addFolderContentDimensions(QString filePath, Vec3 dim)
{
	_prevPileizedActorDims.insert(filePath.toLower(), dim);
}

void  FileSystemActor::clearFolderContentDimensions()
{
	_prevPileizedActorDims.clear();
}

void FileSystemActor::setSizeAnim( Vec3 &startSize, Vec3 &lastSize, uint steps )
{
	sizeAnim.clear();

	// Create a nice Bounce animation
	sizeAnim = bounceGrow(startSize, lastSize, steps);

	// Add to the animation manager
	if (isObjectType(ObjectType(BumpActor, FileSystem, StickyNote)))
		animManager->addAnimation(AnimationEntry(this, (FinishedCallBack) SyncStickyNoteAfterResize, NULL, true));
	else
		animManager->addAnimation(AnimationEntry(this, NULL, NULL, true));
	bool hasGridPileParent = getParent() && getParent()->isObjectType(ObjectType(BumpPile, NULL, Grid));
 	if (hasGridPileParent && isFileSystemType(Image))
	{
 		animManager->finishAnimation(this);

		// We need to add an animation to make sure the updatePileStateAfterAnim is called
		sizeAnim.push_back(lastSize);
		animManager->addAnimation(AnimationEntry(this, (FinishedCallBack) updatePileStateAfterAnim, NULL, true));
	}
}

void FileSystemActor::setSizeAnim(deque<Vec3> &customSizeAnim)
{
	Actor::setSizeAnim(customSizeAnim);
}

// Private helper method for enqueueing the thumbnail texture to be loaded.
// All internal callers should use this rather than calling the texture
// manager directly, to ensure the proper flags are passed in. 
bool FileSystemActor::loadThumbnailTexture( GLTextureObject& obj )
{
	if (!_loadThumbnailGLTexture) obj.operation |= NoGLLoad;
	if (_serializeThumbnail) obj.operation |= Serialize;
	return texMgr->loadTexture(obj);
}

bool FileSystemActor::serializeToPb(PbBumpObject * pbObject)
{
	assert(pbObject);

	// NOTE: if this actor is pileized, then sync the position of this actor to the pile
	if (isPileized())
	{
		BumpObject * lastPileItem = pileizedPile->getLastItem();
		assert(lastPileItem);
		if (lastPileItem)
		{
			Vec3 pos = lastPileItem->getGlobalPosition();
			pos.y = getGlobalPosition().z;
			setGlobalPosition(pos);
		}
	}

	// serialize the core actor properties
	if (!Actor::serializeToPb(pbObject))
		return false;

	// write the full path
	pbObject->SetExtension(PbFileSystemActor::full_path, stdString(getFullPath()));
	
	// write the launch override path
	pbObject->SetExtension(PbFileSystemActor::launch_override_path, stdString(getLaunchOverride()));

	// write the texture override path
	QString overrideTexturePath;
	getTextureOverride(overrideTexturePath);
	pbObject->SetExtension(PbFileSystemActor::texture_override_path, stdString(overrideTexturePath));

	// write the pilelized state
	bool pileized = (pileizedPile != NULL);
	pbObject->SetExtension(PbFileSystemActor::pileized, pileized);

	// write the thumbnail state
	pbObject->SetExtension(PbFileSystemActor::thumbnailized, useThumbnail);

	// write the launch count
	pbObject->SetExtension(PbFileSystemActor::launch_count, useThumbnail);

	// NOTE: if the actor was pileized, then push it back underground
	if (isPileized())
		PushBelowGround(this);

	// write all the cached previous children sizes if this filesystem actor was pileized
	if (isFileSystemType(Folder))
	{
		QHashIterator<QString, Vec3> iter(_prevPileizedActorDims);
		while (iter.hasNext()) 
		{
			iter.next();
			PbFileSystemActor_PbCachedFilePathDims * dims = pbObject->AddExtension(PbFileSystemActor::prev_pileized_children_dims);
			dims->set_file_path(stdString(iter.key()));
			toPbVec3(iter.value(), dims->mutable_dims());
		}
	}

	return pbObject->IsInitialized();
}

bool FileSystemActor::deserializeFromPb(const PbBumpObject * pbObject)
{
	assert(pbObject);

	// deserialize the core actor properties
	if (!Actor::deserializeFromPb(pbObject))
		return false;

	// NOTE: removable drives are always initially un-mounted
	if (isFileSystemType(Removable))
		setMounted(false);

	// read the full path
	if (pbObject->HasExtension(PbFileSystemActor::full_path))
	{
		QString filePath = qstring(pbObject->GetExtension(PbFileSystemActor::full_path));
		if (!filePath.isEmpty())
			setFilePath(filePath, isFileSystemType(LogicalVolume));
	}

	// read the launch override path
	if (pbObject->HasExtension(PbFileSystemActor::launch_override_path))
	{
		QString launchOverridePath = qstring(pbObject->GetExtension(PbFileSystemActor::launch_override_path));
		setLaunchOverride(launchOverridePath);
	}

	// read the texture override path
	if (pbObject->HasExtension(PbFileSystemActor::texture_override_path))
	{
		QString textureOverridePath = qstring(pbObject->GetExtension(PbFileSystemActor::texture_override_path));
		setTextureOverride(textureOverridePath);
	}

	// read the pilelized state
	if (pbObject->HasExtension(PbFileSystemActor::pileized))
	{
		if (pbObject->GetExtension(PbFileSystemActor::pileized))
			pileize();
	}

	// read the thumbnailized state
	if (pbObject->HasExtension(PbFileSystemActor::thumbnailized))
	{
		if (pbObject->GetExtension(PbFileSystemActor::thumbnailized))
			enableThumbnail(true, false);
	}

	// read the launch count
	if (pbObject->HasExtension(PbFileSystemActor::launch_count))
		setNumTimesLaunched(pbObject->GetExtension(PbFileSystemActor::launch_count));

	// read all the cached previous children sizes if this filesystem actor was pileized
	if (isFileSystemType(Folder))
	{
		int numCachedPileizedActorSizes = pbObject->ExtensionSize(PbFileSystemActor::prev_pileized_children_dims);
		for (int i = 0; i < numCachedPileizedActorSizes; ++i)
		{
			const PbFileSystemActor_PbCachedFilePathDims& actorDims = pbObject->GetExtension(PbFileSystemActor::prev_pileized_children_dims, i);
			addFolderContentDimensions(qstring(actorDims.file_path()), fromPbVec3(actorDims.dims()));
		}
	}

	return true;
}

