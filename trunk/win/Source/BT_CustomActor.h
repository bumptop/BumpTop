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

#ifndef BT_CUSTOMACTOR
#define BT_CUSTOMACTOR

// -----------------------------------------------------------------------------

#include "BT_Actor.h"
#include "BT_Singleton.h"

// -----------------------------------------------------------------------------

class TossTarget;
class LaunchTarget;
class CustomActorImpl;
class Selectable;
class FacebookClient;
class TwitterClient;
class TwitpicClient;
class FlickrClient;
class PbBumpObject;

// -----------------------------------------------------------------------------

/*
 * A set of properties that varies between CustomActors
 */
class CustomActorInfo
{
public:
	QString uniqueObjectId;
	QString textureId;
	QString description;
	QString name;

	float sizeMultiplier;

	shared_ptr<TossTarget> tossHandler;
	shared_ptr<BtDragObject> dragHandler;
	shared_ptr<DropObject> dropHandler;
	shared_ptr<LaunchTarget> launchHandler;
	shared_ptr<Selectable> selectHandler;

public:
	CustomActorInfo();
	~CustomActorInfo();
};

// -----------------------------------------------------------------------------

/* 
 * A CustomActor which represents an actor which has no FileSystem equivalent
 * resource, yet behaves nearly the same.  Note that custom actors are usually
 * represent unique objects in the scene (ie. the printer icon, email icon, etc.)
 */
class CustomActor : public Actor
{
	// 
	CustomActorInfo * _customInfo;

public:
	CustomActor();
	CustomActor(CustomActorInfo * info);
	virtual ~CustomActor();

	// Actor
	void			setCustomActorInfo(CustomActorInfo * info);
	CustomActorInfo * getCustomActorInfo() const;

	bool			isCustomImplementationType(CustomActorImpl * tmpImpl) const;
	
	// TossTarget
	virtual void	onTossRecieve(vector<BumpObject *> tossedObjs);
	virtual bool	isValidTossTarget();
	virtual bool	isValidToss(vector<BumpObject *> tossedObjs);

	// BtDragObject
	virtual void	onDragBegin(FinishedDragCallBack func = NULL);
	virtual void	onDragEnd();
	virtual void	onDragMove();
	virtual void	onDragHover();

	// DropObject
	virtual QString resolveDropOperationString(vector<BumpObject *>& objList);
	virtual void	onDropEnter(vector<BumpObject *> &objList);
	virtual void	onDropExit();
	virtual vector<BumpObject *> onDrop(vector<BumpObject *> &objList);
	virtual void	onDropHover(vector<BumpObject *> &objList);
	virtual bool	isValidDropTarget();
	virtual bool	isSourceValid();

	// LaunchTarget
	virtual void	onLaunch();

	// Selectable
	virtual void	onClick();
	virtual void	onDoubleClick();
	virtual void	onSelect();
	virtual void	onDeselect();

	// protocol buffers
	virtual bool	serializeToPb(PbBumpObject * pbObject);
	virtual bool	deserializeFromPb(const PbBumpObject * pbObject);
};

// -----------------------------------------------------------------------------

/*
 * An interface that allows the CustomActorRegistry to recreate arbitrary 
 * custom actors. 
 */
class CustomActorImpl
{
	// saved REFERENCE of the custom actor info 
	CustomActorInfo * _customInfo;

public:
	CustomActorImpl(CustomActorInfo * info);

	CustomActorInfo *			getCustomActorInfo() const;
	virtual QString				getUniqueObjectId() const = 0;
	virtual CustomActorImpl *	clone() = 0;
};

// -----------------------------------------------------------------------------

/*
 * A registry of CustomActors so that they can be recreated without explicitly
 * comparing against their unique ids.
 */
class CustomActorRegistry
{
	typedef QHash<QString, CustomActorImpl *> CustomActorImplRegistryMap;
	CustomActorImplRegistryMap _customActorImplMapping;

private:
	friend class Singleton<CustomActorRegistry>;
	CustomActorRegistry();

	bool contains(QString uniqueId);

public:
	~CustomActorRegistry();

	bool			registerCustomActorImplementation(CustomActorImpl * impl);
	// bool			unregisterCustomActorImplementation(QString uniqueId);
	// bool			unregisterCustomActorImplementation(CustomActorImpl * impl);
	CustomActorInfo * buildCustomActorInfo(QString uniqueId);
};

// -----------------------------------------------------------------------------

/*
 * A helper class to registry CustomActorImpl.ementations easily.
 */
template<class T>
class CustomActorImplAutoRegister
{
public:
	CustomActorImplAutoRegister()
	{
		CustomActorInfo * info = new CustomActorInfo;
		CustomActorImpl * impl = new T(info);
		Singleton<CustomActorRegistry>::getInstance()->registerCustomActorImplementation(impl);
	}

	// don't bother unregistering for now
};

// -----------------------------------------------------------------------------

class PrinterActorImpl : public CustomActorImpl,
						 public TossTarget,
						 public DropObject,
						 public LaunchTarget,
						 public Selectable
{
	Q_DECLARE_TR_FUNCTIONS(PrinterActorImpl)

	vector<BumpObject *> _prevDropObjList;

public:
	// creates this print actor implementation and fills in the custom actor info
	PrinterActorImpl(CustomActorInfo * info);

	// CustomActorImpl
	virtual QString				getUniqueObjectId() const;
	virtual CustomActorImpl *	clone();

	// TossTarget
	virtual void	onTossRecieve(vector<BumpObject *> tossedObjs);
	virtual bool	isValidTossTarget();
	virtual bool	isValidToss(vector<BumpObject *> tossedObjs);

	// DropObject
	virtual QString resolveDropOperationString(vector<BumpObject *>& objList);
	virtual vector<BumpObject *> onDrop(vector<BumpObject *> &objList);
	virtual void	onDropExit();
	virtual bool	isSourceValid();
	virtual bool	isValidDropTarget();

	// LaunchTarget
	virtual void	onLaunch();

	// Selectable
	virtual void	onClick();
	virtual void	onDeselect();
};

// -----------------------------------------------------------------------------

class FacebookActorImpl : public CustomActorImpl,
	public TossTarget,
	public DropObject,
	public LaunchTarget,
	public Selectable
{
	Q_DECLARE_TR_FUNCTIONS(FacebookActorImpl)

	vector<BumpObject *> _prevDropObjList;
	shared_ptr<FacebookClient> _client;

public:
	// creates this facebook actor implementation and fills in the custom actor info
	FacebookActorImpl(CustomActorInfo * info);

	// CustomActorImpl
	virtual QString				getUniqueObjectId() const;
	virtual CustomActorImpl *	clone();

	// TossTarget
	virtual void	onTossRecieve(vector<BumpObject *> tossedObjs);
	virtual bool	isValidTossTarget();
	virtual bool	isValidToss(vector<BumpObject *> tossedObjs);

	// DropObject
	virtual QString resolveDropOperationString(vector<BumpObject *>& objList);
	virtual vector<BumpObject *> onDrop(vector<BumpObject *> &objList);
	virtual void	onDropExit();
	virtual bool	isSourceValid();
	virtual bool	isValidDropTarget();

	// LaunchTarget
	virtual void	onLaunch();

	// Selectable
	virtual void	onClick();
	virtual void	onDeselect();
};

// -----------------------------------------------------------------------------

class TwitterActorImpl : public CustomActorImpl,
	public TossTarget,
	public DropObject,
	public LaunchTarget,
	public Selectable
{
	Q_DECLARE_TR_FUNCTIONS(TwitterActorImpl)

	vector<BumpObject *> _prevDropObjList;

	shared_ptr<TwitterClient> _twitterClient;
	shared_ptr<TwitpicClient> _twitpicClient;

public:
	// creates this twitter actor implementation and fills in the custom actor info
	TwitterActorImpl(CustomActorInfo * info);

	// CustomActorImpl
	virtual QString				getUniqueObjectId() const;
	virtual CustomActorImpl *	clone();

	// TossTarget
	virtual void	onTossRecieve(vector<BumpObject *> tossedObjs);
	virtual bool	isValidTossTarget();
	virtual bool	isValidToss(vector<BumpObject *> tossedObjs);

	// DropObject
	virtual QString resolveDropOperationString(vector<BumpObject *>& objList);
	virtual vector<BumpObject *> onDrop(vector<BumpObject *> &objList);
	virtual void	onDropExit();
	virtual bool	isSourceValid();
	virtual bool	isValidDropTarget();

	// LaunchTarget
	virtual void	onLaunch();

	// Selectable
	virtual void	onClick();
	virtual void	onDeselect();
};

// -----------------------------------------------------------------------------

class EmailActorImpl : public CustomActorImpl,
					   public TossTarget,
					   public DropObject,
					   public LaunchTarget,
					   public Selectable
{
	Q_DECLARE_TR_FUNCTIONS(EmailActorImpl)

	vector<BumpObject *> _prevDropObjList;

public:
	// creates this email actor implementation and fills in the custom actor info
	EmailActorImpl(CustomActorInfo * info);

	// CustomActorImpl
	virtual QString				getUniqueObjectId() const;
	virtual CustomActorImpl *	clone();

	// TossTarget
	virtual void	onTossRecieve(vector<BumpObject *> tossedObjs);
	virtual bool	isValidTossTarget();
	virtual bool	isValidToss(vector<BumpObject *> tossedObjs);

	// DropObject
	virtual QString resolveDropOperationString(vector<BumpObject *>& objList);
	virtual vector<BumpObject *> onDrop(vector<BumpObject *> &objList);
	virtual void	onDropExit();
	virtual bool	isSourceValid();
	virtual bool	isValidDropTarget();

	// LaunchTarget
	virtual void	onLaunch();

	// Selectable
	virtual void	onClick();
	virtual void	onDeselect();
};

bool CreateEmailWithSelectionAsAttachments(const vector<BumpObject *>& objects);

// -----------------------------------------------------------------------------

class StickyNotePadActorImpl : public CustomActorImpl,
							   public TossTarget,
							   public BtDragObject,
							   public DropObject,
							   public LaunchTarget,
							   public Selectable
{
	Q_DECLARE_TR_FUNCTIONS(StickyNotePadActorImpl)

	vector<BumpObject *> _prevDropObjList;
	BumpObject * _dragBeginObject;

public:
	// creates this email actor implementation and fills in the custom actor info
	StickyNotePadActorImpl(CustomActorInfo * info);

	// CustomActorImpl
	virtual QString				getUniqueObjectId() const;
	virtual CustomActorImpl *	clone();

	// TossTarget
	virtual void	onTossRecieve(vector<BumpObject *> tossedObjs);
	virtual bool	isValidTossTarget();
	virtual bool	isValidToss(vector<BumpObject *> tossedObjs);
	
#ifdef ALLOW_DRAG_CREATE_STICKY_WC_JAN10
	// BtDragObject
	virtual void	onDragBegin(BumpObject * obj, FinishedDragCallBack func = NULL);
	virtual void	onDragEnd();
#endif

	// DropObject
	virtual QString resolveDropOperationString(vector<BumpObject *>& objList);
	virtual vector<BumpObject *> onDrop(vector<BumpObject *> &objList);
	virtual void	onDropExit();
	virtual bool	isSourceValid();
	virtual bool	isValidDropTarget();

	// LaunchTarget
	virtual void	onLaunch();

	// Selectable
	virtual void	onSelect();
	virtual void	onDeselect();
};

// -----------------------------------------------------------------------------

class FlickrActorImpl : public CustomActorImpl,
	public TossTarget,
	public DropObject,
	public LaunchTarget,
	public Selectable
{
	Q_DECLARE_TR_FUNCTIONS(FlickrActorImpl)

	vector<BumpObject *> _prevDropObjList;
	shared_ptr<FlickrClient> _flickrClient;

public:
	// creates this email actor implementation and fills in the custom actor info
	FlickrActorImpl(CustomActorInfo * info);

	// CustomActorImpl
	virtual QString				getUniqueObjectId() const;
	virtual CustomActorImpl *	clone();

	// TossTarget
	virtual void	onTossRecieve(vector<BumpObject *> tossedObjs);
	virtual bool	isValidTossTarget();
	virtual bool	isValidToss(vector<BumpObject *> tossedObjs);

	// DropObject
	virtual QString resolveDropOperationString(vector<BumpObject *>& objList);
	virtual vector<BumpObject *> onDrop(vector<BumpObject *> &objList);
	virtual void	onDropExit();
	virtual bool	isSourceValid();
	virtual bool	isValidDropTarget();

	// LaunchTarget
	virtual void	onLaunch();

	// Selectable
	virtual void	onClick();
	virtual void	onDeselect();
};

// -----------------------------------------------------------------------------

class DummyActorImpl : public CustomActorImpl,
					   public TossTarget,
					   public DropObject,
					   public LaunchTarget
{
	Q_DECLARE_TR_FUNCTIONS(DummyActorImpl)

public:
	// creates this dummy actor implementation and fills in the custom actor info
	DummyActorImpl(CustomActorInfo * info);

	// CustomActorImpl
	virtual QString				getUniqueObjectId() const;
	virtual CustomActorImpl *	clone();

	// TossTarget
	virtual void	onTossRecieve(vector<BumpObject *> tossedObjs);
	virtual bool	isValidTossTarget();
	virtual bool	isValidToss(vector<BumpObject *> tossedObjs);

	// DropObject
	virtual QString resolveDropOperationString(vector<BumpObject *>& objList);
	virtual vector<BumpObject *> onDrop(vector<BumpObject *> &objList);
	virtual bool	isSourceValid();
	virtual bool	isValidDropTarget();

	// LaunchTarget
	virtual void	onLaunch();
};

// -----------------------------------------------------------------------------

class HISLogoActorImpl : public CustomActorImpl,
				 		 public TossTarget,
						 public DropObject,
						 public LaunchTarget,
						 public Selectable
{
	Q_DECLARE_TR_FUNCTIONS(HISLogoActorImpl)

	vector<BumpObject *> _prevDropObjList;

public:
	// creates this his actor implementation and fills in the custom actor info
	HISLogoActorImpl(CustomActorInfo * info);

	// CustomActorImpl
	virtual QString				getUniqueObjectId() const;
	virtual CustomActorImpl *	clone();

	// TossTarget
	virtual void	onTossRecieve(vector<BumpObject *> tossedObjs);
	virtual bool	isValidTossTarget();
	virtual bool	isValidToss(vector<BumpObject *> tossedObjs);

	// DropObject
	virtual QString resolveDropOperationString(vector<BumpObject *>& objList);
	virtual vector<BumpObject *> onDrop(vector<BumpObject *> &objList);
	virtual void	onDropExit();
	virtual bool	isSourceValid();
	virtual bool	isValidDropTarget();

	// LaunchTarget
	virtual void	onLaunch();

	// Selectable
	virtual void	onClick();
	virtual void	onDeselect();
};

#else
	class CustomActorInfo;
	class CustomActor;
#endif // BT_CUSTOMACTOR