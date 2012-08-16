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

#ifndef _BT_FILE_SYSTEM_ACTOR_
#define _BT_FILE_SYSTEM_ACTOR_

// -----------------------------------------------------------------------------

#include "BT_Actor.h"
#include "BT_AnimatedTextureSource.h"
#include "BT_GLTextureManager.h"
#include "BT_GLTextureObject.h"

// -----------------------------------------------------------------------------

class FileSystemPile;
class PbBumpObject;

// -----------------------------------------------------------------------------

// Summarizes the different types of FileSystem Attributes [tertiaryType - (1 << 0) to (1 << 15)]
enum FileSystemActorType
{
	Folder		= (1 << 0),
	Virtual		= (1 << 1),
	File		= (1 << 2),
	Image		= (1 << 3),
	Document	= (1 << 4),
	Link		= (1 << 5),
	Thumbnail	= (1 << 6),
	DeadLink	= (1 << 7),
	Executable	= (1 << 8),
	PhotoFrame	= (1 << 9),
	StickyNote	= (1 << 10),
	WebThumbnail = (1 << 11),
	LogicalVolume = (1 << 12),
	Removable = (1 << 13)
};

// -----------------------------------------------------------------------------

class FileSystemActor : public Actor,
						public TextureEventHandler
{
	Q_DECLARE_TR_FUNCTIONS(FileSystemActor)

private:
	weak_ptr<ThemeManager> themeManagerRef;

protected:
	// File Path
	QString filePath;
	QString shortPath;
	QString lnkFullPath;
	bool _mounted;

	// Pileization for Folder Types
	FileSystemPile *pileizedPile;
	// this hash map holds the previous actor dimensions of the folderized items
	// so that their dimensions can be restored when pilelized again
	QHash<QString, Vec3> _prevPileizedActorDims;

	// Thumbnail for Image Types
	QString thumbnailID;
	QString _alternateThumbnailId;
	QString _overrideTexturePath;
	GLTextureDetail _minThumbnailDetail;

	bool useThumbnail; // Should this actor be thumbnailed?

	// These two flags really only apply if useThumbnail is true
	bool _loadThumbnailGLTexture; // Should the thumbnail be loaded into the actor's GL texture?
	bool _serializeThumbnail; // Should the thumbnail image be serialized to disk?

	// tracking for auto-grow functionality
	int numTimesLaunched;
	void			incrementNumTimesLaunched();

	// animated actor?
	bool _isAnimatedTexture;
	AnimatedTextureSource _animatedTextureSource;

	boost::function<void(FileSystemActor* actor)> _onLaunchHandler;

	bool loadThumbnailTexture(GLTextureObject& obj);

public:
	FileSystemActor(); // Generally, should use FileSystemActorFactory instead
	virtual ~FileSystemActor();

	// Member Functions
	void			pushFileSystemType(FileSystemActorType fsType);
	void			popFileSystemType(FileSystemActorType fsType);
	virtual void	onLaunch();
	void			enableThumbnail(bool useThumbnail=true, bool loadThumbnail=true);
	FileSystemPile	*pileize();
	virtual bool	allowNativeContextMenu() const;

	// Events
	virtual vector<BumpObject *> onDrop(vector<BumpObject *> &objList);
	virtual void	onRender(uint flags = RenderSideless);
	virtual void	onTossRecieve(vector<BumpObject *> tossedObjs);

	// Setters
	void			setPileizedPile(FileSystemPile *p);
	virtual void	setFilePath(QString fullPath, bool skipTextureResolution=false);
	void			setNumTimesLaunched(int numTimesLaunched);
	void			setThumbnailID(QString id);
	virtual void	setMinThumbnailDetail(GLTextureDetail detail);
	void			setTextureOverride(QString path);
	void			setOnLaunchHandler(boost::function<void(FileSystemActor* actor)> onLaunchHandler);
	void			refreshThumbnail();
	void			addFolderContentDimensions (QString filePath, Vec3 dim);
	void			clearFolderContentDimensions ();
	void			setSizeAnim(Vec3 &startSize, Vec3 &lastSize, uint steps);
	void			setSizeAnim(deque<Vec3> &customSizeAnim);
	inline void		setLoadThumbnailGLTexture(bool value) { _loadThumbnailGLTexture = value; };
	inline void		setSerializeThumbnail(bool value) { _serializeThumbnail = value; };

	// Getters
#ifdef DXRENDER
	virtual IDirect3DTexture9 * getTextureNum();
#else
	virtual uint	getTextureNum();
#endif
	FileSystemPile *getPileizedPile();
	QString			getFullPath() const;
	QString			getShortPath();
	QString			getLinkTarget() const;
	QString			getTargetPath() const; // returns path for links, or just the path if not
	QString			getFileName(bool truncExt = true);
	QString			getThumbnailID() const; 
	GLTextureDetail	getMinThumbnailDetail() const;
	QString			getAlternateThumbnailId() const;
	virtual Vec3	getDefaultDims();
	bool			getTextureOverride(QString& path);
	bool			isFileSystemType(FileSystemActorType fsType) const;
	bool			isCopyIntoActor(const vector<BumpObject *> &objList) const;
	bool			isPilable(uint pileType);
	bool			isPileized();
	bool			isThumbnailized();
	virtual QString resolveDropOperationString(vector<BumpObject *>& objList);
	virtual bool	isValidDropTarget();
	virtual bool	isSourceValid();
	virtual bool	isValidTossTarget();
	virtual bool	isValidToss(vector<BumpObject *> tossedObjs);

	// texture event handling
	virtual void onTextureLoadComplete(QString textureKey);
	virtual void onTextureLoadError(QString textureKey);

	// 
	bool			hasAnimatedTexture() const;
	void			updateAnimatedTexture();
	void			playAnimatedTexture(bool play);
	bool			isPlayingAnimatedTexture() const;

	// mountable drives
	bool			isMounted() const;
	void			setMounted(bool mount, QString name="");
	
	// protocol buffers
	virtual bool	serializeToPb(PbBumpObject * pbObject);
	virtual bool	deserializeFromPb(const PbBumpObject * pbObject);
};

// -----------------------------------------------------------------------------

#else
	class FileSystemActor;
	enum FileSystemActorType;
#endif
