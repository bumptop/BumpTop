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
#include "BT_AnimationManager.h"
#include "BT_CustomActor.h"
#include "BT_OverlayComponent.h"
#include "BT_Pile.h"
#include "BT_SceneManager.h"
#include "BT_WindowsOS.h"
#include "BT_Util.h"
#include "BT_Macros.h"
#include "BT_FileSystemManager.h"
#include "BT_FileSystemPile.h"
#include "BT_DialogManager.h"
#include "BT_Selection.h"
#include "BT_StatsManager.h"
#include "BT_StickyNoteMenuActionCustomizer.h"
#include "FacebookClient.h"
#include "TwitterClient.h"
#include "FlickrClient.h"
#include "BumpTop.pb.h"

// -----------------------------------------------------------------------------

CustomActorInfo::CustomActorInfo()
: sizeMultiplier(1.0f)
{}

CustomActorInfo::~CustomActorInfo()
{}

// -----------------------------------------------------------------------------

CustomActor::CustomActor()
: Actor()
, _customInfo(NULL)
{
	pushActorType(Custom);
	setDims(Vec3(GLOBAL(settings).xDist, GLOBAL(settings).zDist, GLOBAL(settings).yDist));
	setGlobalOrientation(GLOBAL(straightIconOri));
	setGlobalPosition(Vec3(0.0f));
}

CustomActor::CustomActor(CustomActorInfo * info)
: Actor()
, _customInfo(NULL)
{
	pushActorType(Custom);
	setDims(Vec3(info->sizeMultiplier * GLOBAL(settings).xDist, info->sizeMultiplier * GLOBAL(settings).zDist, info->sizeMultiplier * GLOBAL(settings).yDist));
	setGlobalOrientation(GLOBAL(straightIconOri));
	setGlobalPosition(Vec3(0.0f));
	setCustomActorInfo(info);
}

CustomActor::~CustomActor()
{
	SAFE_DELETE(_customInfo);
}

bool CustomActor::isCustomImplementationType(CustomActorImpl * tmpImpl) const
{
	if (_customInfo)
	{
		return (_customInfo->uniqueObjectId == tmpImpl->getCustomActorInfo()->uniqueObjectId);
	}
	return false;
}

void CustomActor::setCustomActorInfo(CustomActorInfo * info)
{
	SAFE_DELETE(_customInfo);
	_customInfo = info;

	// Set up the state of the Actor
	setTextureID(info->textureId);
	setText(info->name);
	if (info->name.isEmpty())
		hideText(true);	
}

CustomActorInfo * CustomActor::getCustomActorInfo() const
{
	return _customInfo;
}

void CustomActor::onTossRecieve(vector<BumpObject *> tossedObjs)
{
	Actor::onTossRecieve(tossedObjs);
	if (_customInfo && _customInfo->tossHandler)
		_customInfo->tossHandler->onTossRecieve(tossedObjs);
}
bool CustomActor::isValidTossTarget()
{	
	if (_customInfo && _customInfo->tossHandler)
		return _customInfo->tossHandler->isValidTossTarget();
	return Actor::isValidTossTarget();
}
bool CustomActor::isValidToss(vector<BumpObject *> tossedObjs)
{
	if (_customInfo && _customInfo->tossHandler)
		return _customInfo->tossHandler->isValidToss(tossedObjs);
	return Actor::isValidToss(tossedObjs);
}

void CustomActor::onDragBegin(FinishedDragCallBack func)
{
	if (_customInfo && _customInfo->dragHandler)
		return _customInfo->dragHandler->onDragBegin(this, func);
	return Actor::onDragBegin(func);
}

void CustomActor::onDragEnd()
{
	if (_customInfo && _customInfo->dragHandler)
		return _customInfo->dragHandler->onDragEnd();
	return Actor::onDragEnd();
}

void CustomActor::onDragMove()
{
	if (_customInfo && _customInfo->dragHandler)
		return _customInfo->dragHandler->onDragMove();
	return Actor::onDragMove();
}

void CustomActor::onDragHover()
{
	if (_customInfo && _customInfo->dragHandler)
		return _customInfo->dragHandler->onDragHover();
	return Actor::onDragHover();
}

QString CustomActor::resolveDropOperationString(vector<BumpObject *>& objList)
{
	if (_customInfo && _customInfo->dropHandler)
		return _customInfo->dropHandler->resolveDropOperationString(objList);
	return Actor::resolveDropOperationString(objList);
}

void CustomActor::onDropEnter(vector<BumpObject *> &objList)
{
	if (_customInfo && _customInfo->dropHandler)
		_customInfo->dropHandler->onDropEnter(objList);
	Actor::onDropEnter(objList);
}

void CustomActor::onDropExit()
{
	if (_customInfo && _customInfo->dropHandler)
		_customInfo->dropHandler->onDropExit();
	Actor::onDropExit();
}

void CustomActor::onDropHover(vector<BumpObject *> &objList)
{
	if (_customInfo && _customInfo->dropHandler)
		_customInfo->dropHandler->onDropHover(objList);
	Actor::onDropHover(objList);
}

vector<BumpObject *> CustomActor::onDrop(vector<BumpObject *> &objList)
{
	if (_customInfo && _customInfo->dropHandler)
		return _customInfo->dropHandler->onDrop(objList);
	return Actor::onDrop(objList);
}

bool CustomActor::isValidDropTarget()
{
	if (_customInfo && _customInfo->dropHandler)
		return _customInfo->dropHandler->isValidDropTarget();
	return Actor::isValidDropTarget();
}

bool CustomActor::isSourceValid()
{
	if (_customInfo && _customInfo->dropHandler)
		return _customInfo->dropHandler->isSourceValid();
	return Actor::isSourceValid();
}

void CustomActor::onLaunch()
{
	// handle the launch override if there is one
	if (!getLaunchOverride().isEmpty())
	{
		fsManager->launchFileAsync(getLaunchOverride());
		return;
	}	

	if (_customInfo && _customInfo->launchHandler)
		_customInfo->launchHandler->onLaunch();
}

void CustomActor::onClick()
{
	if (_customInfo && _customInfo->selectHandler)
		_customInfo->selectHandler->onClick();
	Actor::onClick();
}

void CustomActor::onDoubleClick()
{
	if (_customInfo && _customInfo->selectHandler)
		_customInfo->selectHandler->onDoubleClick();
	Actor::onDoubleClick();
}

void CustomActor::onSelect()
{
	if (_customInfo && _customInfo->selectHandler)
		_customInfo->selectHandler->onSelect();
	Actor::onSelect();
}

void CustomActor::onDeselect()
{
	if (_customInfo && _customInfo->selectHandler)
		_customInfo->selectHandler->onDeselect();
	Actor::onDeselect();
}

bool CustomActor::serializeToPb(PbBumpObject * pbObject)
{
	assert(pbObject);

	// serialize the core actor properties
	if (!Actor::serializeToPb(pbObject))
		return false;

	if (_customInfo)
	{
		// write the implementation object class name
		pbObject->SetExtension(PbCustomActor::impl_class_name, stdString(_customInfo->uniqueObjectId));
	}

	return pbObject->IsInitialized();
}

bool CustomActor::deserializeFromPb(const PbBumpObject * pbObject)
{
	assert(pbObject);

	// deserialize the core actor properties
	if (!Actor::deserializeFromPb(pbObject))
		return false;

	// read the implementation name so that we can build it
	if (pbObject->HasExtension(PbCustomActor::impl_class_name))
	{
		QString uniqueObjectId = qstring(pbObject->GetExtension(PbCustomActor::impl_class_name));
		CustomActorInfo * info = Singleton<CustomActorRegistry>::getInstance()->buildCustomActorInfo(uniqueObjectId);
		if (!info)
			animManager->addAnimation(AnimationEntry(this, (FinishedCallBack) DeleteActorAfterAnim));
		else
			setCustomActorInfo(info);
	}

	return true;
}

// -----------------------------------------------------------------------------

CustomActorImpl::CustomActorImpl(CustomActorInfo * info)
: _customInfo(info)
{}

CustomActorInfo * CustomActorImpl::getCustomActorInfo() const
{
	return _customInfo;
}

// -----------------------------------------------------------------------------

CustomActorRegistry::CustomActorRegistry()
{}

CustomActorRegistry::~CustomActorRegistry()
{
	// delete all the implementations
	CustomActorImplRegistryMap::iterator iter = _customActorImplMapping.begin();
	while (iter != _customActorImplMapping.end())
	{
		delete iter.value();
		iter++;
	}
}

bool CustomActorRegistry::contains(QString uniqueId)
{
	return _customActorImplMapping.find(uniqueId) != _customActorImplMapping.end();
}

bool CustomActorRegistry::registerCustomActorImplementation(CustomActorImpl * impl)
{
	assert(impl);
	QString id = impl->getUniqueObjectId();
	assert(!contains(id));
	_customActorImplMapping.insert(id, impl);
	return true;
}

CustomActorInfo * CustomActorRegistry::buildCustomActorInfo(QString uniqueId)
{
	if (contains(uniqueId)) {
		CustomActorImpl * impl = _customActorImplMapping.find(uniqueId).value()->clone();
		return impl->getCustomActorInfo();
	}
	return NULL;
}

// -----------------------------------------------------------------------------

// register the FacebookActorImpl
// CustomActorImplAutoRegister<FacebookActorImpl> facebookActorImplRegistration;

FacebookActorImpl::FacebookActorImpl(CustomActorInfo * info)
: CustomActorImpl(info)
{
	info->uniqueObjectId = FacebookActorImpl::getUniqueObjectId();
	info->textureId = QString(QT_NT("icon.custom.facebook"));
	info->name = QT_TR_NOOP("Facebook");
	info->description = QT_TR_NOOP("Drop images to here to upload them to Facebook\nand double-click to go to your profile!");
	info->sizeMultiplier = 2.0f;
	shared_ptr<FacebookActorImpl> thisRef(this);
	info->tossHandler = thisRef;
	info->dropHandler = thisRef;
	info->launchHandler = thisRef;
	info->selectHandler = thisRef;

	_client = shared_ptr<FacebookClient>(new FacebookClient());
}

void FacebookActorImpl::onTossRecieve(vector<BumpObject *> tossedObjs)
{
	if (onDrop(tossedObjs).empty())
	{
		// statsManager->getStats().bt.interaction.actors.custom.printer.tossedTo++;
		// since we are using the same drop code, decrement the count again
		// statsManager->getStats().bt.interaction.actors.custom.printer.droppedOn--;
	}

	animateObjectsBackToPreDropPose(tossedObjs);
}

bool FacebookActorImpl::isValidTossTarget()
{	
	return true;
}

bool FacebookActorImpl::isValidToss(vector<BumpObject *> tossedObjs)
{
	if (tossedObjs.empty())
		return false;

	// check if they are filesystem actors
	vector<FacebookPhoto> photos;
	for (int i = 0; i < tossedObjs.size(); ++i)
	{
		ObjectType type = tossedObjs[i]->getObjectType();
		if (type == ObjectType(BumpActor, FileSystem, Image))
		{
			// mark this as a photo
			FileSystemActor * fsActor = (FileSystemActor *) tossedObjs[i];
			photos.push_back(FacebookPhoto(fsActor->getFullPath()));
		}
		else if (type == ObjectType(BumpPile))
		{
			// concat all the pile items
			Pile * p = (Pile *) tossedObjs[i];
			vector<BumpObject *> pileItems = p->getPileItems();
			for (int j = 0; j < pileItems.size(); ++j)
				tossedObjs.push_back(pileItems[j]);
		}
		else
		{
			return false;
		}
	}

	// check if the photos can be validated
	QString err;
	if (!_client->validate_photos_upload(photos, err))
	{
		return false;
	}

	return true;
}


QString FacebookActorImpl::resolveDropOperationString(vector<BumpObject *>& objList)
{
	return QT_TR_NOOP("Upload");
}

vector<BumpObject *> FacebookActorImpl::onDrop(vector<BumpObject *> &objList)
{
	if (!ftManager->hasInternetConnection())
	{
		printUniqueError("FacebookActorImpl", QT_TR_NOOP("No internet connection detected"));
		return objList;
	}

	_prevDropObjList = objList;

	// upload the photos
	bool usingSavedSession = false;
	if (_client->initialize(usingSavedSession))
	{
		bool continueUpload = !usingSavedSession;

		if (usingSavedSession)
		{
			dlgManager->clearState();
			if (dlgManager->promptDialog(DialogFacebookConfirm))
			{
				continueUpload = true;
			}
		}

		if (continueUpload)
		{
			vector<FacebookPhoto> photos;
			for (int i = 0; i < objList.size(); ++i)
			{
				ObjectType type = objList[i]->getObjectType();
				if (type == ObjectType(BumpActor, FileSystem, Image))
				{
					// mark this as a photo
					FileSystemActor * fsActor = (FileSystemActor *) objList[i];
					photos.push_back(FacebookPhoto(fsActor->getFullPath()));
				}
				else if (type == ObjectType(BumpPile))
				{
					// concat all the pile items
					Pile * p = (Pile *) objList[i];
					vector<BumpObject *> pileItems = p->getPileItems();
					for (int j = 0; j < pileItems.size(); ++j)
						objList.push_back(pileItems[j]);
				}
				else
				{
					// checked for in isValidToss
					assert(false);
				}
			}

			if (_client->photos_upload(photos))
			{
				statsManager->getStats().bt.interaction.actors.custom.facebook.droppedOn++;
				return vector<BumpObject *>();
			}
		}
	}
	return objList;
}

void FacebookActorImpl::onDropExit()
{
	if (!_prevDropObjList.empty())
	{
		// workaround to prevent the pinning of items on the wall when dropping onto this actor
		sel->setPickedActor(NULL);
		animateObjectsBackToPreDropPose(_prevDropObjList);
		_prevDropObjList.clear();
	}
}

bool FacebookActorImpl::isSourceValid()
{
	if(GLOBAL(settings.enableTossing))
		return isValidToss(source);
	else {
		if (source.empty())
		return false;

		// check if they are filesystem actors
		vector<FacebookPhoto> photos;
		for (int i = 0; i < source.size(); ++i)
		{
			ObjectType type = source[i]->getObjectType();
			if (type == ObjectType(BumpActor, FileSystem, Image))
			{
				// mark this as a photo
				FileSystemActor * fsActor = (FileSystemActor *) source[i];
				photos.push_back(FacebookPhoto(fsActor->getFullPath()));
			}
			else if (type == ObjectType(BumpPile))
			{
				// concat all the pile items
				Pile * p = (Pile *) source[i];
				vector<BumpObject *> pileItems = p->getPileItems();
				for (int j = 0; j < pileItems.size(); ++j)
				source.push_back(pileItems[j]);
			}
			else
			{
				return false;
			}
		}

		// check if the photos can be validated
		QString err;
		if (!_client->validate_photos_upload(photos, err))
		{
			return false;
		}
		return true;
	}
}

bool FacebookActorImpl::isValidDropTarget()
{
	return true;
}

QString FacebookActorImpl::getUniqueObjectId() const
{
	return QT_NT("FacebookActorImpl");
}

CustomActorImpl * FacebookActorImpl::clone()
{
	// we know that the FacebookActorImpl defines the CustomActorInfo's 
	// properties, so we can just create a new FacebookActorImpl and 
	// copy over the other CustomActorInfo.  (No other information is 
	// stored in the FacebookActorImpl)
	CustomActorInfo * newCAI = new CustomActorInfo;
	*newCAI = *(this->getCustomActorInfo());
	return new FacebookActorImpl(newCAI);
}

void FacebookActorImpl::onLaunch()
{
	dismiss("CustomActorImpl_description");
	QString url = QT_NT("http://www.facebook.com/profile.php");
	fsManager->launchFileAsync(url);
	statsManager->getStats().bt.interaction.actors.custom.facebook.launched++;
}

void FacebookActorImpl::onClick()
{
	// notify the user
	MessageClearPolicy policy; 
		policy.setTimeout(5); 
	Message * message = new Message("CustomActorImpl_description", getCustomActorInfo()->description, Message::Ok | Message::ForceTruncate, policy);
	scnManager->messages()->addMessage(message); 
}

void FacebookActorImpl::onDeselect()
{
	BumpObject * obj = sel->getPickedActor();
	dismiss("CustomActorImpl_description");
}

// -----------------------------------------------------------------------------

// register the TwitterActorImpl
CustomActorImplAutoRegister<TwitterActorImpl> twitterActorImplRegistration;

TwitterActorImpl::TwitterActorImpl(CustomActorInfo * info)
: CustomActorImpl(info)
{
	info->uniqueObjectId = TwitterActorImpl::getUniqueObjectId();
	info->textureId = QString(QT_NT("icon.custom.twitter"));
	info->name = QT_TR_NOOP("Twitter");
	info->description = QT_TR_NOOP("Drop images here to update your Twitter\nand double-click to send a tweet!");
	info->sizeMultiplier = 2.0f;
	shared_ptr<TwitterActorImpl> thisRef(this);
	info->tossHandler = thisRef;
	info->dropHandler = thisRef;
	info->launchHandler = thisRef;
	info->selectHandler = thisRef;

	_twitterClient = shared_ptr<TwitterClient>(new TwitterClient(TWITTER_SERVER));
	_twitpicClient = shared_ptr<TwitpicClient>(new TwitpicClient(TWITPIC_SERVER));
}

void TwitterActorImpl::onTossRecieve(vector<BumpObject *> tossedObjs)
{
	if (onDrop(tossedObjs).empty())
	{
		// statsManager->getStats().bt.interaction.actors.custom.printer.tossedTo++;
		// since we are using the same drop code, decrement the count again
		// statsManager->getStats().bt.interaction.actors.custom.printer.droppedOn--;
	}

	animateObjectsBackToPreDropPose(tossedObjs);
}
bool TwitterActorImpl::isValidTossTarget()
{	
	return true;
}
bool TwitterActorImpl::isValidToss(vector<BumpObject *> tossedObjs)
{
	if (tossedObjs.empty())
		return false;

	if (tossedObjs.size() > 1)
	{
		printUnique("TwitterActorImpl", QT_TR_NOOP("One tweet at a time!"));
		return false;
	}

	// check if it is an actor
	QString photoPath;
	for (int i = 0; i < tossedObjs.size(); ++i)
	{
		ObjectType type = tossedObjs[i]->getObjectType();
		if (type == ObjectType(BumpActor, FileSystem, Image))
		{
			FileSystemActor * fsActor = (FileSystemActor *) tossedObjs[i];
			photoPath = fsActor->getFullPath();
		}
		else
		{
			return false;
		}
	}

	// check if the photo can be validated
	if (!_twitpicClient->validate_uploadAndPost("", photoPath))
	{
		return false;
	}

	return true;
}


QString TwitterActorImpl::resolveDropOperationString(vector<BumpObject *>& objList)
{
	return QT_TR_NOOP("Tweet");
}

vector<BumpObject *> TwitterActorImpl::onDrop(vector<BumpObject *> &objList)
{
	if (!ftManager->hasInternetConnection())
	{
		printUniqueError("TwitterActorImpl", QT_TR_NOOP("No internet connection detected"));
		return objList;
	}

	_prevDropObjList = objList;

	if (_twitterClient->initialize() &&
		_twitpicClient->initialize())
	{
		// upload the photos
		// NOTE:	This code looks like it supports multiple photo uploads, however the twitter actor is restricted to one photo at a time.
		//			Although it looks like the code below is logically broken (photoPath is being overwritten), it works for single photos.
		QString photoPath;
		for (int i = 0; i < objList.size(); ++i)
		{
			ObjectType type = objList[i]->getObjectType();
			if (type == ObjectType(BumpActor, FileSystem, Image))
			{
				FileSystemActor * fsActor = (FileSystemActor *) objList[i];
				photoPath = fsActor->getFullPath();
			}
			else
			{
				// non-files are rejected
				printUniqueError("TwitterActorImpl", QT_TR_NOOP("Unsupported image file type"));
				return objList;
			}
		}

		// tweet
		if (_twitpicClient->uploadAndPostDeferMessage(photoPath))
			return vector<BumpObject *>();
	}
	return objList;
}

void TwitterActorImpl::onDropExit()
{
	if (!_prevDropObjList.empty())
	{
		// workaround to prevent the pinning of items on the wall when dropping onto this actor
		sel->setPickedActor(NULL);
		animateObjectsBackToPreDropPose(_prevDropObjList);
		_prevDropObjList.clear();
	}
}


bool TwitterActorImpl::isSourceValid()
{
	if(GLOBAL(settings.enableTossing))
		return isValidToss(source);
	else {
		if (source.empty())
			return false;

		if (source.size() > 1)
		{
			printUnique("TwitterActorImpl", QT_TR_NOOP("One tweet at a time!"));
			return false;
		}

		// check if it is an actor
		QString photoPath;
		for (int i = 0; i < source.size(); ++i)
		{
			ObjectType type = source[i]->getObjectType();
			if (type == ObjectType(BumpActor, FileSystem, Image))
			{
				FileSystemActor * fsActor = (FileSystemActor *) source[i];
				photoPath = fsActor->getFullPath();
			}
			else
			{
				return false;
			}
		}

		// check if the photo can be validated
		if (!_twitpicClient->validate_uploadAndPost("", photoPath))
		{
			return false;
		}

		return true;
	}
}

bool TwitterActorImpl::isValidDropTarget()
{
	return true;
}

QString TwitterActorImpl::getUniqueObjectId() const
{
	return QT_NT("TwitterActorImpl");
}

CustomActorImpl * TwitterActorImpl::clone()
{
	// we know that the TwitterActorImpl defines the CustomActorInfo's 
	// properties, so we can just create a new TwitterActorImpl and 
	// copy over the other CustomActorInfo.  (No other information is 
	// stored in the TwitterActorImpl)
	CustomActorInfo * newCAI = new CustomActorInfo;
	*newCAI = *(this->getCustomActorInfo());
	return new TwitterActorImpl(newCAI);
}

void TwitterActorImpl::onLaunch()
{
	dismiss("CustomActorImpl_description");
	if (_twitterClient->initialize())
	{	
		// tweet
		dlgManager->clearState();
		dlgManager->setCaption(QT_TR_NOOP("Update Twitter as %1").arg(GLOBAL(settings).tw_login));
		if (dlgManager->promptDialog(DialogTwitterTweet))
		{
			QString msg = dlgManager->getText();
			if (_twitterClient->statuses_update(msg))
			{
				statsManager->getStats().bt.interaction.actors.custom.twitter.launched++;
				printUnique("TwitterActorImpl", QT_TR_NOOP("Tweet sent!"));
				fsManager->launchFileAsync(_twitterClient->getProfileLink());
			}
			else
			{
				printUnique("TwitterActorImpl", QT_TR_NOOP("Tweet failed :("));
			}
		}
	}
}

void TwitterActorImpl::onClick()
{
	// notify the user
	{
		MessageClearPolicy policy; 
			policy.setTimeout(5);	
		Message * message = new Message("CustomActorImpl_description", getCustomActorInfo()->description, Message::Ok | Message::ForceTruncate, policy);
		scnManager->messages()->addMessage(message); 
	}
}

void TwitterActorImpl::onDeselect()
{
	dismiss("CustomActorImpl_description");
}

// -----------------------------------------------------------------------------

// register the PrinterActorImpl
CustomActorImplAutoRegister<PrinterActorImpl> printerActorImplRegistration;

PrinterActorImpl::PrinterActorImpl(CustomActorInfo * info)
: CustomActorImpl(info)
{
	info->uniqueObjectId = PrinterActorImpl::getUniqueObjectId();
	info->textureId = QString(QT_NT("icon.custom.printer"));
	info->name = QT_TR_NOOP("Send to Printer");
	info->description = QT_TR_NOOP("Drop an item on this printer to\nsend it to your default printer!");
	info->sizeMultiplier = 2.0f;
	shared_ptr<PrinterActorImpl> thisRef(this);
		info->tossHandler = thisRef;
		info->dropHandler = thisRef;
		info->launchHandler = thisRef;
		info->selectHandler = thisRef;

	/*
	// try and find the name of the default printer
	{
		DWORD len = 1024;
		TCHAR printerName[1024];
		GetDefaultPrinter(printerName, &len);
		info->name = printerName;
	}
	*/
}

void PrinterActorImpl::onTossRecieve(vector<BumpObject *> tossedObjs)
{
	if (onDrop(tossedObjs).empty())
	{
		statsManager->getStats().bt.interaction.actors.custom.printer.tossedTo++;
		// since we are using the same drop code, decrement the count again
		statsManager->getStats().bt.interaction.actors.custom.printer.droppedOn--;
	}

	animateObjectsBackToPreDropPose(tossedObjs);
}
bool PrinterActorImpl::isValidTossTarget()
{	
	return true;
}
bool PrinterActorImpl::isValidToss(vector<BumpObject *> tossedObjs)
{
	return true;
}


QString PrinterActorImpl::resolveDropOperationString(vector<BumpObject *>& objList)
{
	return QT_TR_NOOP("Print");
}

vector<BumpObject *> PrinterActorImpl::onDrop(vector<BumpObject *> &objList)
{
	_prevDropObjList = objList;

	// only one item can be printed at a time
	if (objList.size() > 1)
	{
		MessageClearPolicy clearPolicy;
		clearPolicy.setTimeout(2);
		scnManager->messages()->addMessage(new Message("PrinterActorImpl::onDrop", QT_TR_NOOP("One print job at a time please."), Message::Ok, clearPolicy));
		return objList;
	}
	else if (objList.size() == 1)
	{
		// ensure is proper file system file
		if (!(objList.front()->getObjectType() == ObjectType(BumpActor, FileSystem, File)))
		{
			MessageClearPolicy clearPolicy;
				clearPolicy.setTimeout(2);
			scnManager->messages()->addMessage(new Message("PrinterActorImpl::onDrop", QT_TR_NOOP("You can only print files!"), Message::Ok, clearPolicy));
			return objList;
		}	

		// confirm if we actually want to print
		dlgManager->clearState();
		dlgManager->setPrompt(QT_TR_NOOP("Print these file(s) on your default printer?"));
		if (!dlgManager->promptDialog(DialogYesNo))
		{
			dismiss("PrinterActorImpl::onDrop");
			return objList;
		}

		// send to the printer
		FileSystemActor * fsActor = dynamic_cast<FileSystemActor *>(objList.front());
		QString filePath = fsActor->getFullPath();
		int result = (int) ShellExecute(winOS->GetWindowsHandle(), QT_NT(L"print"), (LPCTSTR) filePath.utf16(), NULL, NULL, SW_SHOWNORMAL);
		if (result <= 32)
		{
			MessageClearPolicy clearPolicy;
				clearPolicy.setTimeout(2);
			scnManager->messages()->addMessage(new Message("PrinterActorImpl::onDrop", QT_TR_NOOP("Could not print this file!"), Message::Ok, clearPolicy));
		}
		else
		{
			// notify the user
			MessageClearPolicy clearPolicy;
				clearPolicy.setTimeout(3);
			Message * message = new Message("PrinterActorImpl::onDrop", QT_TR_NOOP("Printing file"), Message::Ok, clearPolicy);
			scnManager->messages()->addMessage(message);

			statsManager->getStats().bt.interaction.actors.custom.printer.droppedOn++;
			return vector<BumpObject *>();
		}
	}
	return objList;
}

void PrinterActorImpl::onDropExit()
{
	if (!_prevDropObjList.empty())
	{
		// workaround to prevent the pinning of items on the wall when dropping onto this actor
		sel->setPickedActor(NULL);
		animateObjectsBackToPreDropPose(_prevDropObjList);
		_prevDropObjList.clear();
	}
}


bool PrinterActorImpl::isSourceValid()
{
	if(GLOBAL(settings.enableTossing))
		return isValidToss(source);
	else
		return true;
}


bool PrinterActorImpl::isValidDropTarget()
{
	return true;
}

QString PrinterActorImpl::getUniqueObjectId() const
{
	return QT_NT("PrinterActorImpl");
}

CustomActorImpl * PrinterActorImpl::clone()
{
	// we know that the PrinterActorImpl defines the CustomActorInfo's 
	// properties, so we can just create a new PrinterActorImpl and 
	// copy over the other CustomActorInfo.  (No other information is 
	// stored in the PrinterActorImpl)
	CustomActorInfo * newCAI = new CustomActorInfo;
	*newCAI = *(this->getCustomActorInfo());
	return new PrinterActorImpl(newCAI);
}

void PrinterActorImpl::onLaunch()
{
	dismiss("CustomActorImpl_description");
	// {2227A280-3AEA-1069-A2DE-08002B30309D} - Printers uuid
	statsManager->getStats().bt.interaction.actors.custom.printer.launched++;
	ShellExecute(winOS->GetWindowsHandle(), NULL, QT_NT(L"::{2227A280-3AEA-1069-A2DE-08002B30309D}"), NULL, NULL, SW_SHOWNORMAL);
	printUnique("PrinterActorImpl", QT_TR_NOOP("Launching Printers control panel"));
}

void PrinterActorImpl::onClick()
{
	// notify the user
	{
		MessageClearPolicy policy; 
		policy.setTimeout(5); 
		Message * message = new Message("CustomActorImpl_description", getCustomActorInfo()->description, Message::Ok | Message::ForceTruncate, policy);
		scnManager->messages()->addMessage(message); 
	}
}

void PrinterActorImpl::onDeselect()
{
	dismiss("CustomActorImpl_description");
}

// -----------------------------------------------------------------------------

// register the EmailActorImpl
CustomActorImplAutoRegister<EmailActorImpl> emailActorImplRegistration;

EmailActorImpl::EmailActorImpl(CustomActorInfo * info)
: CustomActorImpl(info)
{
	info->uniqueObjectId = EmailActorImpl::getUniqueObjectId();
	info->textureId = QString(QT_NT("icon.custom.email"));
	info->name = QT_TR_NOOP("New Email");
	info->description = QT_TR_NOOP("Drop items on this email outbox\nto create a new email with the items\nas attachments!");
	info->sizeMultiplier = 2.0f;
	shared_ptr<EmailActorImpl> thisRef(this);
		info->tossHandler = thisRef;
		info->dropHandler = thisRef;
		info->launchHandler = thisRef;
		info->selectHandler = thisRef;
}

bool CreateEmailWithSelectionAsAttachments(const vector<BumpObject *>& objects)
{
	IShellFolder * desktopFolder = NULL;
	IShellFolder2 * psf = NULL;
	IShellView * psv = NULL;
	bool result = false;

	// get the shell folder
	if (SUCCEEDED(SHGetDesktopFolder(&desktopFolder))) 
	{
		// create a list of all the items to be cut
		vector<BumpObject *> selection = objects;
		vector<QString> filePaths = GetFilteredSetForIDataObject(selection, &psf);
		if (filePaths.empty())
			return true;

		// build the set of pidls that are to be acted upon
		uint numPidls = filePaths.size();
		LPCITEMIDLIST * pidlArray = new LPCITEMIDLIST[numPidls];
		for (int i = 0; i < numPidls; ++i)
		{
			pidlArray[i] = winOS->GetRelativePidlFromAbsFilePath(filePaths[i]);
		}

		// get the IDataObject representing these files
		IDataObject * pdo = NULL;
		IDropTarget * pdt = NULL;
		HRESULT hr = psf->GetUIObjectOf(NULL, numPidls, pidlArray, IID_IDataObject, NULL, (void **) &pdo);
		if (SUCCEEDED(hr))
		{
			GUID sendMailId = {0x9e56be60, 0xc50f, 0x11cf, {0x9a, 0x2c, 0x00,0xa0, 0xc9, 0x0a, 0x90, 0xce}};

			// create an instance, and mimic drag-and-drop
			hr = ::CoCreateInstance( sendMailId, NULL, CLSCTX_INPROC_SERVER, IID_IDropTarget,	(void **) &pdt);
			if (SUCCEEDED(hr))
			{
				POINTL pt = {0,0};
				DWORD dwEffect = 0;
				pdt->DragEnter(pdo, MK_LBUTTON, pt, &dwEffect);
				pdt->Drop(pdo, MK_LBUTTON, pt, &dwEffect);
				pdt->Release();
				result = true;
			}
			pdo->Release();
		}
		desktopFolder->Release();
	}
	
	return result;
}

void EmailActorImpl::onTossRecieve(vector<BumpObject *> tossedObjs)
{	
	if (onDrop(tossedObjs).empty())
	{
		statsManager->getStats().bt.interaction.actors.custom.email.tossedTo++;
		// since we are using the same drop code, decrement the count again
		statsManager->getStats().bt.interaction.actors.custom.email.droppedOn--;
	}

	animateObjectsBackToPreDropPose(tossedObjs);
}
bool EmailActorImpl::isValidTossTarget()
{	
	return true;
}
bool EmailActorImpl::isValidToss(vector<BumpObject *> tossedObjs)
{
	return true;
}


QString EmailActorImpl::resolveDropOperationString(vector<BumpObject *>& objList)
{
	return QT_TR_NOOP("Email");
}

vector<BumpObject *> EmailActorImpl::onDrop(vector<BumpObject *> &objList)
{
	_prevDropObjList = objList;

	// try and attach all filesystem actors
	if (objList.size() > 0)
	{
		vector<BumpObject *> tmpObjList = objList;

		// ensure only filesystem actors can be sent
		for (int i = 0; i < tmpObjList.size();)
		{
			BumpObject * obj = tmpObjList[i];
			if (obj->getObjectType() == ObjectType(BumpActor, FileSystem))
			{
				FileSystemActor * fsActor = dynamic_cast<FileSystemActor *>(obj);
				if (!fsActor->isFileSystemType(File))
				{
					MessageClearPolicy clearPolicy;
						clearPolicy.setTimeout(2);
					scnManager->messages()->addMessage(new Message("EmailActorImpl::onDrop", QT_TR_NOOP("Can't Email non-file attachments!"), Message::Ok, clearPolicy));
					return objList;
				}
				else
				{
					++i;
				}
			}
			else if (obj->getObjectType() == ObjectType(BumpPile, SoftPile))
			{
				// add each of these items to the set of objects to be attached
				tmpObjList.erase(tmpObjList.begin() + i);
				Pile * p = dynamic_cast<Pile *>(obj);
				vector<BumpObject *> pileItems = p->getPileItems();
				for (int i = 0; i < pileItems.size(); ++i)
				{
					tmpObjList.push_back(pileItems[i]);
				}
			}
			else if (obj->getObjectType() == ObjectType(BumpPile, HardPile))
			{
				FileSystemPile * fp = dynamic_cast<FileSystemPile *>(obj);
				tmpObjList.erase(tmpObjList.begin() + i);
				tmpObjList.push_back(fp->getOwner());
			}
			else if (obj->getObjectType() == ObjectType(BumpActor,Webpage))
			{
				MessageClearPolicy clearPolicy;
				clearPolicy.setTimeout(2);
				scnManager->messages()->addMessage(new Message("EmailActorImpl::onDrop", QT_TR_NOOP("Can't Email non-file attachments!"), Message::Ok, clearPolicy));
				return objList;
			}
			else
			{
				 ++i;
			}
		}

		// try and do the toss
		if (!CreateEmailWithSelectionAsAttachments(tmpObjList))
		{
			MessageClearPolicy clearPolicy;
			clearPolicy.setTimeout(2);
			scnManager->messages()->addMessage(new Message("EmailActorImpl::onDrop", QT_TR_NOOP("Could not create email with these files\nas attachments!"), Message::Ok, clearPolicy));
		}
		else
		{
			// notify the user
			MessageClearPolicy clearPolicy;
				clearPolicy.setTimeout(3);
			Message * message = new Message("EmailActorImpl::onDrop", QT_TR_NOOP("Creating new email"), Message::Ok, clearPolicy);
			scnManager->messages()->addMessage(message);

			statsManager->getStats().bt.interaction.actors.custom.email.droppedOn++;
			return vector<BumpObject *>();
		}
	}
	return objList;
}

void EmailActorImpl::onDropExit()
{
	if (!_prevDropObjList.empty())
	{
		// workaround to prevent the pinning of items on the wall when dropping onto this actor
		sel->setPickedActor(NULL);
		animateObjectsBackToPreDropPose(_prevDropObjList);
		_prevDropObjList.clear();
	}
}


bool EmailActorImpl::isSourceValid()
{
	if(GLOBAL(settings.enableTossing))
		return isValidToss(source);
	else 
		return true;
}


bool EmailActorImpl::isValidDropTarget()
{
	return true;
}

QString EmailActorImpl::getUniqueObjectId() const
{
	return QT_NT("EmailActorImpl");
}

CustomActorImpl * EmailActorImpl::clone()
{
	// we know that the EmailActorImpl defines the CustomActorInfo's 
	// properties, so we can just create a new EmailActorImpl and 
	// copy over the other CustomActorInfo.  (No other information is 
	// stored in the EmailActorImpl)
	CustomActorInfo * newCAI = new CustomActorInfo;
	*newCAI = *(this->getCustomActorInfo());
	return new EmailActorImpl(newCAI);
}

void EmailActorImpl::onLaunch()
{
	// launch a new window
	statsManager->getStats().bt.interaction.actors.custom.email.launched++;
	ShellExecute(winOS->GetWindowsHandle(), NULL, QT_NT(L"mailto:"), NULL, NULL, SW_SHOWNORMAL);
}

void EmailActorImpl::onClick()
{
	// notify the user
	{
		MessageClearPolicy policy; 
			policy.setTimeout(5); 
		Message * message = new Message("CustomActorImpl_description", getCustomActorInfo()->description, Message::Ok | Message::ForceTruncate, policy);
		scnManager->messages()->addMessage(message);
	}
}

void EmailActorImpl::onDeselect()
{
	dismiss("CustomActorImpl_description");
}


// -----------------------------------------------------------------------------

// register the StickyNotePadActorImpl
CustomActorImplAutoRegister<StickyNotePadActorImpl> stickyNotePadActorImplRegistration;

StickyNotePadActorImpl::StickyNotePadActorImpl(CustomActorInfo * info)
: CustomActorImpl(info)
, _dragBeginObject(NULL)
{
	info->uniqueObjectId = StickyNotePadActorImpl::getUniqueObjectId();
	// change the sticky note pad texture to reflect the sticky note pad state
	if (StickyNoteMenuActionCustomizer::hasExceededMaxNumStickyNotes())
		info->textureId = QString(QT_NT("icon.custom.stickyNotePadDisabled"));
	else
		info->textureId = QString(QT_NT("icon.custom.stickyNotePad"));
	info->name = QT_NT("");
	info->description = QT_TR_NOOP("Double click to add a new sticky note!");
	info->sizeMultiplier = 2.0f;
	shared_ptr<StickyNotePadActorImpl> thisRef(this);
		info->tossHandler = thisRef;
		info->dropHandler = thisRef;
		info->launchHandler = thisRef;
		info->selectHandler = thisRef;
#ifdef ALLOW_DRAG_CREATE_STICKY_WC_JAN10
		info->dragHandler = thisRef;
#endif
}

void StickyNotePadActorImpl::onTossRecieve(vector<BumpObject *> tossedObjs)
{	
	onDrop(tossedObjs);
	animateObjectsBackToPreDropPose(tossedObjs);
}
bool StickyNotePadActorImpl::isValidTossTarget()
{	
	return false;
}
bool StickyNotePadActorImpl::isValidToss(vector<BumpObject *> tossedObjs)
{
	return false;
}

#ifdef ALLOW_DRAG_CREATE_STICKY_WC_JAN10
	void StickyNotePadActorImpl::onDragBegin(BumpObject * obj, FinishedDragCallBack func)
	{
		_dragBeginObject = obj;
		BtDragObject::setFinishedDragCallback(func);
		return;
	}

	void StickyNotePadActorImpl::onDragEnd()
	{
		BtDragObject::execFinishedDragCallback(_dragBeginObject);	
		_dragBeginObject = NULL;
		return;
	}
#endif

QString StickyNotePadActorImpl::resolveDropOperationString(vector<BumpObject *>& objList)
{
	// STICKYNOTE_TODO: check if only one sticky note is in the selection
	return QT_NT("");
}

vector<BumpObject *> StickyNotePadActorImpl::onDrop(vector<BumpObject *> &objList)
{
	_prevDropObjList = objList;

	// STICKYNOTE_TODO: if there is only one sticky note in the the selection, then
	// we just push another sticky note text instance onto the top of the sticky note pad

	return objList;
}

void StickyNotePadActorImpl::onDropExit()
{
	if (!_prevDropObjList.empty())
	{
		// workaround to prevent the pinning of items on the wall when dropping onto this actor
		sel->setPickedActor(NULL);
		animateObjectsBackToPreDropPose(_prevDropObjList);
		_prevDropObjList.clear();
	}
}


bool StickyNotePadActorImpl::isSourceValid()
{
	if(GLOBAL(settings.enableTossing))
		return isValidToss(source);
	else 
		return false;
}


bool StickyNotePadActorImpl::isValidDropTarget()
{
	return false;
}

QString StickyNotePadActorImpl::getUniqueObjectId() const
{
	return QT_NT("StickyNotePadActorImpl");
}

CustomActorImpl * StickyNotePadActorImpl::clone()
{
	// we know that the StickyNotePadActorImpl defines the CustomActorInfo's 
	// properties, so we can just create a new StickyNotePadActorImpl and 
	// copy over the other CustomActorInfo.  (No other information is 
	// stored in the StickyNotePadActorImpl)
	CustomActorInfo * newCAI = new CustomActorInfo;
	*newCAI = *(this->getCustomActorInfo());
	return new StickyNotePadActorImpl(newCAI);
}

void StickyNotePadActorImpl::onLaunch()
{
	// create a new sticky note in the center of the scene
	Key_CreateStickyNote();
}

void StickyNotePadActorImpl::onSelect()
{
#ifndef ALLOW_DRAG_CREATE_STICKY_WC_JAN10
	// notify the user
	{
		MessageClearPolicy policy; 
			policy.setTimeout(5); 
		Message * message = new Message("CustomActorImpl_description", getCustomActorInfo()->description, Message::Ok | Message::ForceTruncate, policy);
		scnManager->messages()->addMessage(message);
	}
#endif
}

void StickyNotePadActorImpl::onDeselect()
{
#ifndef ALLOW_DRAG_CREATE_STICKY_WC_JAN10
	dismiss("CustomActorImpl_description");
#endif
}

// -----------------------------------------------------------------------------

// register the FlickrActorImpl
CustomActorImplAutoRegister<FlickrActorImpl> flickrActorImplRegistration;

FlickrActorImpl::FlickrActorImpl(CustomActorInfo * info)
: CustomActorImpl(info)
{
	info->uniqueObjectId = FlickrActorImpl::getUniqueObjectId();
	info->textureId = QString(QT_NT("icon.custom.flickr"));
	info->name = QT_TR_NOOP("Flickr");
	info->description = QT_TR_NOOP("Drop items on this Flickr widget\nto upload them to your\nFlickr Acccount!");
	info->sizeMultiplier = 2.0f;
	shared_ptr<FlickrActorImpl> thisRef(this);
	info->tossHandler = thisRef;
	info->dropHandler = thisRef;
	info->launchHandler = thisRef;
	info->selectHandler = thisRef;

	_flickrClient = shared_ptr<FlickrClient>(new FlickrClient());
}

void FlickrActorImpl::onTossRecieve(vector<BumpObject *> tossedObjs)
{	
	if (onDrop(tossedObjs).empty())
	{
		// Record stats for flickr 
	}

	animateObjectsBackToPreDropPose(tossedObjs);
}
bool FlickrActorImpl::isValidTossTarget()
{	
	return true;
}
bool FlickrActorImpl::isValidToss(vector<BumpObject *> tossedObjs)
{
	return true;
}


QString FlickrActorImpl::resolveDropOperationString(vector<BumpObject *>& objList)
{
	return QT_TR_NOOP("Flickr");
}

vector<BumpObject *> FlickrActorImpl::onDrop(vector<BumpObject *> &objList)
{
	_prevDropObjList = objList;
	// Determine list of file system actors
	// Pass those to flickrClient

	vector<FileSystemActor *> fsActors;
	for (int i = 0; i < objList.size(); ++i)
	{
		ObjectType type = objList[i]->getObjectType();
		if (type == ObjectType(BumpActor, FileSystem, Image))
		{
			// mark this as a photo
			FileSystemActor * fsActor = (FileSystemActor *) objList[i];
			fsActors.push_back(fsActor);
		}
		else if (type == ObjectType(BumpPile))
		{
			// concat all the pile items
			Pile * p = (Pile *) objList[i];
			vector<BumpObject *> pileItems = p->getPileItems();
			for (int j = 0; j < pileItems.size(); ++j)
				objList.push_back(pileItems[j]);
		}
	}

	_flickrClient->uploadPhotos(fsActors);
	statsManager->getStats().bt.interaction.actors.custom.flickr.droppedOn++;
	return objList;
}

void FlickrActorImpl::onDropExit()
{
	if (!_prevDropObjList.empty())
	{
		// workaround to prevent the pinning of items on the wall when dropping onto this actor
		sel->setPickedActor(NULL);
		animateObjectsBackToPreDropPose(_prevDropObjList);
		_prevDropObjList.clear();
	}
}


bool FlickrActorImpl::isSourceValid()
{
	if(GLOBAL(settings.enableTossing))
		return isValidToss(source);
	else
		return true;
}


bool FlickrActorImpl::isValidDropTarget()
{
	return true;
}

QString FlickrActorImpl::getUniqueObjectId() const
{
	return QT_NT("FlickrActorImpl");
}

CustomActorImpl * FlickrActorImpl::clone()
{
	// we know that the EmailActorImpl defines the CustomActorInfo's 
	// properties, so we can just create a new EmailActorImpl and 
	// copy over the other CustomActorInfo.  (No other information is 
	// stored in the EmailActorImpl)
	CustomActorInfo * newCAI = new CustomActorInfo;
	*newCAI = *(this->getCustomActorInfo());
	return new FlickrActorImpl(newCAI);
}

void FlickrActorImpl::onLaunch()
{
	QString flickrAuthToken = GLOBAL(settings).flickr_auth_token;
	if (flickrAuthToken.isEmpty())
	{
		if (!_flickrClient->authenticate())
		{
			printUniqueError("FlickrActorImpl", QT_TR_NOOP("Failed to authenticate"));
		}
	}
	else
	{
		printUniqueError("FlickrActorImpl", QT_TR_NOOP("You already authenticated. Simply drop photos to upload"));
	}
	statsManager->getStats().bt.interaction.actors.custom.flickr.launched++;
}

void FlickrActorImpl::onClick()
{
	// Do something
}

void FlickrActorImpl::onDeselect()
{
	// do something
}

// -----------------------------------------------------------------------------

// register the EmailActorImpl
CustomActorImplAutoRegister<DummyActorImpl> dummyActorImplRegistration;

DummyActorImpl::DummyActorImpl(CustomActorInfo * info)
: CustomActorImpl(info)
{
	info->uniqueObjectId = DummyActorImpl::getUniqueObjectId();
	info->sizeMultiplier = 2.0f;
	shared_ptr<DummyActorImpl> thisRef(this);
		info->tossHandler = thisRef;
		info->dropHandler = thisRef;
		info->launchHandler = thisRef;
}

void DummyActorImpl::onTossRecieve(vector<BumpObject *> tossedObjs)
{	
	onDrop(tossedObjs);
}
bool DummyActorImpl::isValidTossTarget()
{	
	return true;
}

QString DummyActorImpl::resolveDropOperationString(vector<BumpObject *>& objList)
{
	return QString();
}

bool DummyActorImpl::isValidToss(vector<BumpObject *> tossedObjs)
{
	return true;
}

vector<BumpObject *> DummyActorImpl::onDrop(vector<BumpObject *> &objList)
{
	printUnique("DummyActorImpl::onDrop", QT_TR_NOOP("File(s) Received"));

	// Animate back to the original starting pose
	animateObjectsBackToPreDropPose(objList);
	return vector<BumpObject *>();
}


bool DummyActorImpl::isSourceValid()
{
	if(GLOBAL(settings.enableTossing))
		return isValidToss(source);
	else
		return true;
}


bool DummyActorImpl::isValidDropTarget()
{
	return true;
}

QString DummyActorImpl::getUniqueObjectId() const
{
	return QT_NT("DummyActorImpl");
}

CustomActorImpl * DummyActorImpl::clone()
{
	// we know that the DummyActorImpl defines the CustomActorInfo's 
	// properties, so we can just create a new DummyActorImpl and 
	// copy over the other CustomActorInfo.  (No other information is 
	// stored in the DummyActorImpl)
	CustomActorInfo * newCAI = new CustomActorInfo;
	*newCAI = *(this->getCustomActorInfo());
	return new DummyActorImpl(newCAI);
}

void DummyActorImpl::onLaunch()
{
	printUnique("DummyActorImpl::onLaunch", QT_TR_NOOP("Launched"));
}

// -----------------------------------------------------------------------------

// register the HISLogoActorImpl
CustomActorImplAutoRegister<HISLogoActorImpl> hisLogoActorImplRegistration;

HISLogoActorImpl::HISLogoActorImpl(CustomActorInfo * info)
: CustomActorImpl(info)
{
	info->uniqueObjectId = HISLogoActorImpl::getUniqueObjectId();
	info->textureId = QString(QT_NT("icon.custom.logo_his"));
	info->name = QT_TR_NOOP("HIS Digital");
	info->description = QT_TR_NOOP("Double click this icon to visit the HIS Digital webpage");
	info->sizeMultiplier = 2.0f;
	shared_ptr<HISLogoActorImpl> thisRef(this);
	info->tossHandler = thisRef;
	info->dropHandler = thisRef;
	info->launchHandler = thisRef;
	info->selectHandler = thisRef;
}

void HISLogoActorImpl::onTossRecieve(vector<BumpObject *> tossedObjs)
{
	// do nothing
}
bool HISLogoActorImpl::isValidTossTarget()
{	
	return false;
}
bool HISLogoActorImpl::isValidToss(vector<BumpObject *> tossedObjs)
{
	return false;
}

QString HISLogoActorImpl::resolveDropOperationString(vector<BumpObject *>& objList)
{
	return QString();
}

vector<BumpObject *> HISLogoActorImpl::onDrop(vector<BumpObject *> &objList)
{	
	return objList;
}

void HISLogoActorImpl::onDropExit()
{
	// do nothing
}


bool HISLogoActorImpl::isSourceValid()
{
	return false;
}


bool HISLogoActorImpl::isValidDropTarget()
{
	return false;
}

QString HISLogoActorImpl::getUniqueObjectId() const
{
	return QT_NT("HISLogoActorImpl");
}

CustomActorImpl * HISLogoActorImpl::clone()
{
	// we know that the HISLogoActorImpl defines the CustomActorInfo's 
	// properties, so we can just create a new HISLogoActorImpl and 
	// copy over the other CustomActorInfo.  (No other information is 
	// stored in the HISLogoActorImpl)
	CustomActorInfo * newCAI = new CustomActorInfo;
	*newCAI = *(this->getCustomActorInfo());
	return new HISLogoActorImpl(newCAI);
}

void HISLogoActorImpl::onLaunch()
{
	dismiss("CustomActorImpl_description");
	fsManager->launchFileAsync(QT_NT("http://www.hisdigital.com"));
	printUnique("HISLogoActorImpl", QT_TR_NOOP("Launching HIS Digital's Website"));
}

void HISLogoActorImpl::onClick()
{}

void HISLogoActorImpl::onDeselect()
{}

// -----------------------------------------------------------------------------