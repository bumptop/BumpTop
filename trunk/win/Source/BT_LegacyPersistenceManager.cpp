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
#include "BT_Authorization.h"
#include "BT_Camera.h"
#include "BT_CustomActor.h"
#include "BT_FileSystemActorFactory.h"
#include "BT_FileSystemPile.h"
#include "BT_FileSystemManager.h"
#include "BT_FlickrPhotoFrameSource.h"
#include "BT_LegacyPersistenceManager.h"
#include "BT_LocalPhotoFrameSource.h"
#include "BT_Macros.h"
#include "BT_PhotoFrameActor.h"
#include "BT_Pile.h"
#include "BT_RSSPhotoFrameSource.h"
#include "BT_SceneManager.h"
#include "BT_Selection.h"
#include "BT_StickyNoteActor.h"
#include "BT_Util.h"
#include "BT_WebActor.h"
#include "BT_WebThumbnailActor.h"

struct ExtendedHeader
{
	uint versionNumber;
	bool isInfiniteDesktopTurnedOn;
};

// ----------------------------------------------------------------------------

LegacyPersistenceManager::LegacyPersistenceManager()
{}

LegacyPersistenceManager * LegacyPersistenceManager::getInstance()
{
	return Singleton<LegacyPersistenceManager>::getInstance();
}

bool LegacyPersistenceManager::loadScene(const QString& filePath)
{
	if (!exists(filePath))
		return false;

	FILE * fp = _wfopen((LPCWSTR) filePath.utf16(), L"r+b");
	if (fp)
	{
		uint sz = 0;

		// Get File Size
		fseek(fp , 0 , SEEK_END);
		sz = ftell(fp);
		rewind(fp);

		if (sz > 0)
		{
			// Allocate the buffer space
			unsigned char * buf = new unsigned char[sz];
			if (buf)
			{
				// Read out the buffer
				fread(buf, 1, sz, fp);
				fclose(fp);
				bool rc = false;

				// Load the scene from buffer
				try
				{
					rc = LegacyPersistenceManager::getInstance()->loadSceneData(buf, sz);
				} 
				catch(int)
				{
					rc = false;
				}

				// Delete what was read because after this point it gets regenerated
				SAFE_DELETE_ARRAY(buf);

				// Check to see if the Load scene was successful
				if (rc)
				{
					LOG("LegacyPersistenceManager::loadScene successful");
					return rc;
				}
				else
				{
					// Load failed for some reason, delete actors
					for (int i = 0; i < GLOBAL(activeNxActorList).size(); i++)
					{
						if (GetBumpActor(GLOBAL(activeNxActorList)[i]))
						{
							Actor * obj = GetBumpActor(GLOBAL(activeNxActorList)[i]);
							SAFE_DELETE(obj);
							i--;
						}
					}
				}					
			}
		}
	}

	return false;
}

bool LegacyPersistenceManager::loadSceneData(unsigned char * dataStream, unsigned int dataStreamSize)
{
	if (dataStream)
	{
		// ===========================================================
		//					    HEADER [16 BYTES]
		// ===========================================================

		int numActors = 0;
		if (!SERIALIZE_READ(&dataStream, &numActors, sizeof(int), dataStreamSize)) return false;

		int numPiles = 0;
		if (!SERIALIZE_READ(&dataStream, &numPiles, sizeof(int), dataStreamSize)) return false;


		// ===========================================================
		//			    EXTENDED HEADER [ 4 + 1024 BYTES]
		// ===========================================================

		// the header was written as SERIALIZE_MAX_SIZE bytes, so read that
		// much into the header struct
		unsigned char extendedHeaderData[SERIALIZE_MAX_SIZE];
		if (!SERIALIZE_READ(&dataStream, &extendedHeaderData, SERIALIZE_MAX_SIZE, dataStreamSize)) return false;

		ExtendedHeader * header = (ExtendedHeader *) extendedHeaderData;
		if (header->versionNumber > SCENE_VERSION_NUMBER)
			return false;
		if (header->versionNumber > 3)
		{			
			if (header->isInfiniteDesktopTurnedOn)
				Key_ToggleInfiniteDesktopMode();
		}

		// ===========================================================
		//  PILE [8 + (4 + 1024) + (#items * (4 + 1024 + 120)) BYTES]
		// ===========================================================
		
		// load each of the soft piles (hard piles are stored as filesystem actors
		// which are then pilelized upon loading)
		for (int i = 0; i < numPiles; ++i)
		{
			unsigned int pileDataSize = 0;
			if (SERIALIZE_READ(&dataStream, &pileDataSize, sizeof(unsigned int), dataStreamSize))
			{
				unsigned char * pileData = new unsigned char[pileDataSize];
				if (SERIALIZE_READ(&dataStream, pileData, pileDataSize, dataStreamSize))
				{
					if (!unserializePile(header->versionNumber, pileData, pileDataSize))
					{
						SAFE_DELETE_ARRAY(pileData);
						return false;
					}
				}
				else
				{
					SAFE_DELETE_ARRAY(pileData);
					return false;
				}
				SAFE_DELETE_ARRAY(pileData);
			}
			else
			{
				return false;
			}
		}	

		// ===========================================================
		//			  	     ACTOR DATA [1024 BYTES]
		// ===========================================================

		// load each of the actors
		for (int i = 0; i < numActors; ++i)
		{
			unsigned char actorData[SERIALIZE_MAX_SIZE];
			if (SERIALIZE_READ(&dataStream, &actorData, SERIALIZE_MAX_SIZE, dataStreamSize))
			{
				// Create the Actors
				unsigned int actorDataSize = SERIALIZE_MAX_SIZE;
				if (!unserializeActor(header->versionNumber, actorData, actorDataSize))
					return false;
			}
			else
			{
				return false;
			}
		}

		// ===========================================================
		//			  	     CAMERA DATA [72 BYTES]
		// ===========================================================

		// load the camera data
		cam->unserialize(dataStream, dataStreamSize, header->versionNumber);

		// LEGACY PERSISTENCY TEMP
		scnManager->crossReference();
		sel->clear();
		// END TEMP

		return true;
	}
	return false;
}

Pile * LegacyPersistenceManager::unserializePile(unsigned int versionNumber, unsigned char * pileData, unsigned int& pileDataSize)
{
	// deserialize the different versions of the piles
	if (versionNumber < 2)
		return unserializePile_v1(versionNumber, pileData, pileDataSize);
	else if (versionNumber <= SCENE_VERSION_NUMBER)
		return unserializePile_v2(versionNumber, pileData, pileDataSize);
	else
		return NULL;
}

Pile * LegacyPersistenceManager::unserializePile_v1(unsigned int versionNumber, unsigned char * pileData, unsigned int& pileDataSize)
{
	Pile * pile = NULL;
	FileSystemPile * fsPile = NULL;

	// ---- HEADER (1024 bytes) ----

	int dummyInt = 0;
	if (!SERIALIZE_READ(&pileData, &dummyInt, sizeof(int), pileDataSize)) return NULL;

	unsigned int numActors = 0;
	if (!SERIALIZE_READ(&pileData, &numActors, sizeof(unsigned int), pileDataSize)) return NULL;

	ObjectType pileType = 0;
	if (!SERIALIZE_READ(&pileData, &pileType, sizeof(unsigned int), pileDataSize)) return NULL;
	if (pileType.primaryType == BumpPile)
	{
		if (pileType.secondaryType == HardPile)
		{
			fsPile = new FileSystemPile();
			pile = fsPile;
		}
		else
			pile = new Pile();
	}
	else
		return NULL;

	Vec3 pileCentroid(0.0f);
	if (!SERIALIZE_READ(&pileData, &pileCentroid, sizeof(Vec3), pileDataSize)) return NULL;

	// ---- ACTORS (numActors * 1160 bytes) ----	

	for (unsigned i = 0; i < numActors; ++i)
	{
		unsigned int actorDataSize = SERIALIZE_MAX_SIZE;
		unsigned char actorData[SERIALIZE_MAX_SIZE];

		// read the actor data
		if (!SERIALIZE_READ(&pileData, actorData, actorDataSize, pileDataSize)) return NULL;
		Actor * actor = unserializeActor(versionNumber, actorData, actorDataSize);
		if (actor)
		{
			Mat34 actorPoses, dummyMat;
			Vec3 actorDims, dummyVec;

			// Load Supporting Data [2 * (4 + 12) + 2 * (4 + 48) = 136 bytes]
			if (!SERIALIZE_READ(&pileData, &actorPoses, sizeof(Mat34), pileDataSize)) return NULL;
			if (!SERIALIZE_READ(&pileData, &actorDims, sizeof(Vec3), pileDataSize)) return NULL;
			if (!SERIALIZE_READ(&pileData, &dummyMat, sizeof(Mat34), pileDataSize)) return NULL;
			if (!SERIALIZE_READ(&pileData, &dummyVec, sizeof(Vec3), pileDataSize)) return NULL;

			actor->setGlobalPose(actorPoses);
			actor->setDims(actorDims);
			pile->addToPile(actor);
		}
		else
		{
			// LEGACY PERSISTENCE TEMP
			SAFE_DELETE(pile);
			return NULL;
		}
	}

	if (pile->getNumItems() > 0)
	{
		pile->stack(pileCentroid);
		animManager->finishAnimation(pile);
		pile->updatePhantomActorDims();
		return pile;
	}
	else
	{
		SAFE_DELETE(pile);
		return NULL;
	}
}

Pile * LegacyPersistenceManager::unserializePile_v2(unsigned int versionNumber, unsigned char * pileData, unsigned int& pileDataSize)
{
	Pile * pile = NULL;
	FileSystemPile * fsPile = NULL;

	// ---- HEADER (1024 bytes) ----

	unsigned int objectTypeBytes = 0;
	if (!SERIALIZE_READ(&pileData, &objectTypeBytes, sizeof(unsigned int), pileDataSize)) return NULL;
	ObjectType pileType = ObjectType::fromUIntReversed(objectTypeBytes);
	if (pileType.primaryType == BumpPile)
	{
		if (pileType.secondaryType == HardPile)
		{
			fsPile = new FileSystemPile();
			pile = fsPile;
		}
		else
			pile = new Pile();
	}
	else
		return NULL;

	unsigned int numActors = 0;
	if (!SERIALIZE_READ(&pileData, &numActors, sizeof(unsigned int), pileDataSize)) return NULL;	

	Vec3 pileCentroid(0.0f);
	if (!SERIALIZE_READ(&pileData, &pileCentroid, sizeof(Vec3), pileDataSize)) return NULL;

	// ---- ACTORS (numActors * 1096 bytes) ----	

	for (int i = 0; i < numActors; ++i)
	{	
		unsigned int actorDataSize = SERIALIZE_MAX_SIZE;
		unsigned char actorData[SERIALIZE_MAX_SIZE];

		if (!SERIALIZE_READ(&pileData, actorData, actorDataSize, pileDataSize)) return NULL;
		Actor * actor = unserializeActor(versionNumber, actorData, actorDataSize);
		if (actor)
		{
			// Save Supporting Data [(4 + 12) + (4 + 48) = 68 bytes]
			Mat34 actorPose;
			if (!SERIALIZE_READ(&pileData, &actorPose, sizeof(Mat34), pileDataSize)) return NULL;
			if (versionNumber < 5)
			{
				Vec3 dummyVec(0.0f);
				if (!SERIALIZE_READ(&pileData, &dummyVec, sizeof(NxVec3), pileDataSize)) return NULL;
			}

			actor->setGlobalPose(actorPose);
			pile->addToPile(actor);
		}
		else
		{
			// LEGACY PERSISTENCE TEMP
			SAFE_DELETE(pile);
			return NULL;
		}
	}

	if (pile->getNumItems() > 0)
	{
		pile->stack(pileCentroid);
		animManager->finishAnimation(pile);
		pile->updatePhantomActorDims();
		return pile;
	}
	else
	{
		SAFE_DELETE(pile);
		return NULL;
	}
}

Actor * LegacyPersistenceManager::unserializeActor(unsigned int versionNumber, unsigned char * actorData, unsigned int& actorDataSize)
{
	// deserialize the different versions of actors
	if (versionNumber == 0)
		return unserializeActor_v1(versionNumber, actorData, actorDataSize);
	else if (versionNumber <= 6)
		return unserializeActor_v2(versionNumber, actorData, actorDataSize);
	else if (versionNumber == 7)
		return unserializeActor_v3(versionNumber, actorData, actorDataSize);
	else if (versionNumber == 8)
		return unserializeActor_v4(versionNumber, actorData, actorDataSize);
	else if (versionNumber <= 11)
		return unserializeActor_v5(versionNumber, actorData, actorDataSize);
	else if (versionNumber == 12)
		return unserializeActor_v6(versionNumber, actorData, actorDataSize);
	else if (versionNumber <= 14)
		return unserializeActor_v7(versionNumber, actorData, actorDataSize);
	else if (versionNumber <= SCENE_VERSION_NUMBER)
		return unserializeActor_v8(versionNumber, actorData, actorDataSize);
	else
		return NULL;	
}

Actor * LegacyPersistenceManager::unserializeActor_v1(unsigned int versionNumber, unsigned char * actorData, unsigned int& actorDataSize)
{
	Actor * actor = NULL;
	FileSystemActor * fsActor = NULL;

	QString texturePath;
	if (!SERIALIZE_READ_STRING(&actorData, texturePath, actorDataSize)) return false;

	QString dummyString;
	if (!SERIALIZE_READ_STRING(&actorData, dummyString, actorDataSize)) return false;
	if (!SERIALIZE_READ_STRING(&actorData, dummyString, actorDataSize)) return false;

	unsigned int actorType = 0;
	if (!SERIALIZE_READ(&actorData, &actorType, sizeof(int), actorDataSize)) return false;
	if (actorType & 2)
	{
		fsActor = FileSystemActorFactory::createFileSystemActor(texturePath);
		actor = fsActor;
	}
	else
		actor = new Actor();

	bool isInvisible = false;
	if (!SERIALIZE_READ(&actorData, &isInvisible, sizeof(bool), actorDataSize)) return false;
	if (isInvisible) 
		actor->pushActorType(Invisible);

	bool isVirtualFolder = false;
	if (!SERIALIZE_READ(&actorData, &isVirtualFolder, sizeof(bool), actorDataSize)) return false;

	bool isPilelized = false;
	if (!SERIALIZE_READ(&actorData, &isPilelized, sizeof(bool), actorDataSize)) return false;
	if (isPilelized && fsActor) 
		fsActor->pileize();

	bool isNameAutoCreated = false;
	if (!SERIALIZE_READ(&actorData, &isNameAutoCreated, sizeof(bool), actorDataSize)) return false;

	bool isPinned = false;
	if (!SERIALIZE_READ(&actorData, &isPinned, sizeof(bool), actorDataSize)) return false;

	bool isNameless = false;
	if (!SERIALIZE_READ(&actorData, &isNameless, sizeof(bool), actorDataSize)) return false;

	bool isCrumpled = false;
	if (!SERIALIZE_READ(&actorData, &isCrumpled, sizeof(bool), actorDataSize)) return false;

	bool isPeeled = false;
	if (!SERIALIZE_READ(&actorData, &isPeeled, sizeof(bool), actorDataSize)) return false;

	float crumpleX = 0.0f, crumpleY = 0.0f;
	if (!SERIALIZE_READ(&actorData, &crumpleX, sizeof(float), actorDataSize)) return false;
	if (!SERIALIZE_READ(&actorData, &crumpleY, sizeof(float), actorDataSize)) return false;

	Mat34 actorPose;
	if (!SERIALIZE_READ(&actorData, &actorPose, sizeof(Mat34), actorDataSize)) return false;
	actor->setGlobalPose(actorPose);

	Vec3 worldPinPoint(0.0f);
	if (!SERIALIZE_READ(&actorData, &worldPinPoint, sizeof(Vec3), actorDataSize)) return false;
	if (isPinned)
	{
		GLOBAL(PinPoint) = worldPinPoint;
		PinItemToWall(actor);
	}

	Vec3 actorDims(1.0f);
	if (!SERIALIZE_READ(&actorData, &actorDims, sizeof(Vec3), actorDataSize)) return false;
	actor->setDims(actorDims);

	Vec3 peelStartPoint(0.0f);
	Vec3 peelEndPoint(0.0f);
	if (!SERIALIZE_READ(&actorData, &peelStartPoint, sizeof(Vec3), actorDataSize)) return false;
	if (!SERIALIZE_READ(&actorData, &peelEndPoint, sizeof(Vec3), actorDataSize)) return false;

	Vec3 originalDims(0.0f);
	Vec3 initialDims(0.0f);
	if (!SERIALIZE_READ(&actorData, &originalDims, sizeof(Vec3), actorDataSize)) return false;
	if (!SERIALIZE_READ(&actorData, &initialDims, sizeof(Vec3), actorDataSize)) return false;

	bool isTextHidden = false;
	if (!SERIALIZE_READ(&actorData, &isTextHidden, sizeof(bool), actorDataSize)) return false;
	if (isTextHidden)
		actor->hideText(true);

	return actor;
}

Actor * LegacyPersistenceManager::unserializeActor_v2(unsigned int versionNumber, unsigned char * actorData, unsigned int& actorDataSize)
{
	PhotoFrameActor * pfActor = NULL;
	WebThumbnailActor * wtActor = NULL;
	FileSystemActor * fsActor = NULL;
	Actor * actor = NULL;

	ObjectType actorType;
	if (!SERIALIZE_READ(&actorData, &actorType, sizeof(unsigned int), actorDataSize)) return false;
	if (actorType.primaryType == BumpActor)
	{
		bool isProBuild = (GLOBAL(settings).freeOrProLevel == AL_PRO);
		if (actorType.ternaryType & PhotoFrame)
			actor = fsActor = pfActor = new PhotoFrameActor();
		else if (isProBuild && (actorType.ternaryType & WebThumbnail))
			actor = fsActor = wtActor = new WebThumbnailActor();
		else if (actorType.secondaryType & FileSystem && (actorType.ternaryType & StickyNote))
			actor = fsActor = new StickyNoteActor();
		else if (actorType.secondaryType & FileSystem)
			actor = fsActor = new FileSystemActor();
		else
			actor = new Actor();

		actor->setObjectType(actorType);
	}
	else
		return NULL;

	QString text;
	if (!SERIALIZE_READ_STRING(&actorData, text, actorDataSize)) return false;
	actor->setText(text);

	bool isTextVisible = false;
	if (!SERIALIZE_READ(&actorData, &isTextVisible, sizeof(bool), actorDataSize)) return false;
	if (!isTextVisible)
		actor->hideText(true);
		
	bool isPinned = false;
	if (!SERIALIZE_READ(&actorData, &isPinned, sizeof(bool), actorDataSize)) return false;

	bool isCrumpled = false;
	if (!SERIALIZE_READ(&actorData, &isCrumpled, sizeof(bool), actorDataSize)) return false;
	
	bool isPeeled = false;
	if (!SERIALIZE_READ(&actorData, &isPeeled, sizeof(bool), actorDataSize)) return false;

	Mat34 actorPose;
	if (!SERIALIZE_READ(&actorData, &actorPose, sizeof(Mat34), actorDataSize)) return false;
	actor->setGlobalPose(actorPose);

	Vec3 actorDims(1.0f);
	if (!SERIALIZE_READ(&actorData, &actorDims, sizeof(Vec3), actorDataSize)) return false;
	actor->setDims(actorDims);

	Vec3 worldPinPoint(0.0f);
	if (!SERIALIZE_READ(&actorData, &worldPinPoint, sizeof(Vec3), actorDataSize)) return false;

	Vec3 actorPinPoint(0.0f);
	if (!SERIALIZE_READ(&actorData, &actorPinPoint, sizeof(Vec3), actorDataSize)) return false;
	if (isPinned)
	{
		actor->setPinPointInSpace(worldPinPoint);
		actor->setPinPointOnActor(actorPinPoint);
		actor->onPin(false);
	}

	Vec3 peelStartPoint(0.0f);
	Vec3 peelEndPoint(0.0f);
	if (!SERIALIZE_READ(&actorData, &peelStartPoint, sizeof(Vec3), actorDataSize)) return false;
	if (!SERIALIZE_READ(&actorData, &peelEndPoint, sizeof(Vec3), actorDataSize)) return false;

	// general filesystem actor properties
	if (fsActor)
	{
		QString filePath;
		if (!SERIALIZE_READ_STRING(&actorData, filePath, actorDataSize)) return false;
		if (!filePath.isEmpty())
			fsActor->setFilePath(filePath);

		bool isPilelized = false;
		if (!SERIALIZE_READ(&actorData, &isPilelized, sizeof(bool), actorDataSize)) return false;
		if (isPilelized) 
			fsActor->pileize();

		bool isThumbnailized = false;
		if (!SERIALIZE_READ(&actorData, &isThumbnailized, sizeof(bool), actorDataSize)) return false;
		if (isThumbnailized)
			fsActor->enableThumbnail(true, false);

		if (versionNumber >= 4)
		{
			int actorLaunchCount = 0;
			if (!SERIALIZE_READ(&actorData, &actorLaunchCount, sizeof(int), actorDataSize)) return false;
			fsActor->setNumTimesLaunched(actorLaunchCount);
		}
	}

	// photoframe actors
	if (pfActor)
	{
		RSSPhotoFrameSource * rssSource = NULL;
		PhotoFrameSource * source = NULL;

		unsigned int photoFrameType = 0;
		if (!SERIALIZE_READ(&actorData, &photoFrameType, sizeof(unsigned int), actorDataSize)) return false;
		if (photoFrameType == LocalImageSource)
		{
			source = new LocalPhotoFrameSource();
			if (!source->unserialize(actorData, actorDataSize)) return false;
		}
		else if (photoFrameType == RemoteImageSource)
		{	
			source = rssSource = new RSSPhotoFrameSource();
			source->unserialize(actorData, actorDataSize);
			if (rssSource->isFlickrPhotoFrame())
			{				
				PhotoFrameSource * flickrSource = new FlickrPhotoFrameSource(rssSource->getRSSFeedUrl());
				SAFE_DELETE(source);
				source = flickrSource;
			}
		}

		if (source)
		{
			pfActor->setUninitializedSource(source);
		}
	}

	return actor;
}

Actor * LegacyPersistenceManager::unserializeActor_v3(unsigned int versionNumber, unsigned char * actorData, unsigned int& actorDataSize)
{
	PhotoFrameActor * pfActor = NULL;
	WebThumbnailActor * wtActor = NULL;
	FileSystemActor * fsActor = NULL;
	CustomActor * csActor = NULL;
	Actor * actor = NULL;

	unsigned int objectTypeBytes = 0;
	if (!SERIALIZE_READ(&actorData, &objectTypeBytes, sizeof(unsigned int), actorDataSize)) return false;	
	ObjectType objectType = ObjectType::fromUIntReversed(objectTypeBytes);
	if (objectType.primaryType == BumpActor)
	{
		bool isProBuild = (GLOBAL(settings).freeOrProLevel == AL_PRO);
		if (isProBuild && (objectType.ternaryType & WebThumbnail))
			actor = fsActor = wtActor = new WebThumbnailActor();
		else if (objectType.ternaryType & PhotoFrame)
			actor = fsActor = pfActor = new PhotoFrameActor();
		else if (objectType.secondaryType & FileSystem && (objectType.ternaryType & StickyNote))
			actor = fsActor = new StickyNoteActor();
		else if (objectType.secondaryType & FileSystem)
			actor = fsActor = new FileSystemActor();
		else if (objectType.secondaryType & Custom)
			actor = csActor = new CustomActor();
		else
			actor = new Actor();

		actor->setObjectType(objectType);
	}
	else
		return false;

	QString text;
	if (!SERIALIZE_READ_STRING(&actorData, text, actorDataSize)) return false;
	actor->setText(text);

	bool isTextVisible = false;
	if (!SERIALIZE_READ(&actorData, &isTextVisible, sizeof(bool), actorDataSize)) return false;
	if (!isTextVisible)
		actor->hideText(true);
	
	bool isPinned = false;
	if (!SERIALIZE_READ(&actorData, &isPinned, sizeof(bool), actorDataSize)) return false;

	bool isCrumpled = false;
	if (!SERIALIZE_READ(&actorData, &isCrumpled, sizeof(bool), actorDataSize)) return false;
	
	bool isPeeled = false;
	if (!SERIALIZE_READ(&actorData, &isPeeled, sizeof(bool), actorDataSize)) return false;
	
	Mat34 actorPose;
	if (!SERIALIZE_READ(&actorData, &actorPose, sizeof(Mat34), actorDataSize)) return false;
	actor->setGlobalPose(actorPose);

	Vec3 actorDims;
	if (!SERIALIZE_READ(&actorData, &actorDims, sizeof(Vec3), actorDataSize)) return false;
	actor->setDims(actorDims);

	Vec3 worldPinPoint(0.0f);
	Vec3 actorPinPoint(0.0f);
	if (!SERIALIZE_READ(&actorData, &worldPinPoint, sizeof(Vec3), actorDataSize)) return false;
	if (!SERIALIZE_READ(&actorData, &actorPinPoint, sizeof(Vec3), actorDataSize)) return false;
	if (isPinned)
	{
		actor->setPinPointInSpace(worldPinPoint);
		actor->setPinPointOnActor(actorPinPoint);
		actor->onPin(false);
	}

	Vec3 peelStartPoint(0.0f);
	Vec3 peelEndPoint(0.0f);
	if (!SERIALIZE_READ(&actorData, &peelStartPoint, sizeof(Vec3), actorDataSize)) return false;
	if (!SERIALIZE_READ(&actorData, &peelEndPoint, sizeof(Vec3), actorDataSize)) return false;

	// filesystem actor properties
	if (fsActor)
	{
		QString filePath;
		if (!SERIALIZE_READ_STRING(&actorData, filePath, actorDataSize)) return false;
		if (!filePath.isEmpty())
			fsActor->setFilePath(filePath);

		QString launchOverridePath;
		if (!SERIALIZE_READ_STRING(&actorData, launchOverridePath, actorDataSize)) return false;
		if (!launchOverridePath.isEmpty())
			fsActor->setLaunchOverride(launchOverridePath);

		bool isPilelized = false;
		if (!SERIALIZE_READ(&actorData, &isPilelized, sizeof(bool), actorDataSize)) return false;
		if (isPilelized)
			fsActor->pileize();

		bool isThumbnailized = false;
		if (!SERIALIZE_READ(&actorData, &isThumbnailized, sizeof(bool), actorDataSize)) return false;
		if (isThumbnailized)
			fsActor->enableThumbnail(true, false);

		if (versionNumber >= 4)
		{
			int actorLaunchCount = 0;
			if (!SERIALIZE_READ(&actorData, &actorLaunchCount, sizeof(int), actorDataSize)) return false;
			fsActor->setNumTimesLaunched(actorLaunchCount);
		}
	}

	// photoframe actors
	if (pfActor)
	{
		LocalPhotoFrameSource * localSource = NULL;
		RSSPhotoFrameSource * rssSource = NULL;
		PhotoFrameSource * source = NULL;

		unsigned int photoFrameType = 0;
		if (!SERIALIZE_READ(&actorData, &photoFrameType, sizeof(unsigned int), actorDataSize)) return false;		// PhotoFrameSourceType
		if (photoFrameType == LocalImageSource)
		{
			source = localSource = new LocalPhotoFrameSource();
			if (!source->unserialize(actorData, actorDataSize)) 
				return false;
		}
		else if (photoFrameType == RemoteImageSource)
		{	
			source = rssSource = new RSSPhotoFrameSource();
			source->unserialize(actorData, actorDataSize);
			if (rssSource->isFlickrPhotoFrame())
			{
				PhotoFrameSource * flickrSource = new FlickrPhotoFrameSource(rssSource->getRSSFeedUrl());
				SAFE_DELETE(source);
				source = flickrSource;
			}
		}

		if (source)
			pfActor->setUninitializedSource(source);
	}

	// custom actors
	if (csActor)
	{	
		QString uniqueObjectId;
		if (!SERIALIZE_READ_STRING(&actorData, uniqueObjectId, actorDataSize)) return false;
		CustomActorInfo * info = Singleton<CustomActorRegistry>::getInstance()->buildCustomActorInfo(uniqueObjectId);
		if (!info)
		{
			animManager->addAnimation(AnimationEntry(actor, (FinishedCallBack) DeleteActorAfterAnim));
		}
		else
		{
			/*
			QString name;
			if (!SERIALIZE_READ_STRING(&actorData, name, actorDataSize)) return false;
			info->name = name;

			QString textureId;
			if (!SERIALIZE_READ_STRING(&actorData, textureId, actorDataSize)) return false;
			if (info->textureId.isEmpty())
				info->textureId = winOS->GetUniqueID();

			QString texturePath;
			if (!SERIALIZE_READ_STRING(&actorData, texturePath, actorDataSize)) return false;
			if (!texturePath.isEmpty())
			{
				// set a new texture and put that into the info
				info->textureId = winOS->GetUniqueID();
				texMgr->loadTexture(GLTextureObject(Load|Compress, info->textureId, texturePath, HiResImage, HighPriority));
			}
			*/

			csActor->setCustomActorInfo(info);
		}
	}

	return actor;
}

Actor * LegacyPersistenceManager::unserializeActor_v4(unsigned int versionNumber, unsigned char * actorData, unsigned int& actorDataSize)
{
	PhotoFrameActor * pfActor = NULL;
	WebThumbnailActor * wtActor = NULL;
	FileSystemActor * fsActor = NULL;
	CustomActor * csActor = NULL;
	Actor * actor = NULL;

	unsigned int objectTypeBytes = 0;
	if (!SERIALIZE_READ(&actorData, &objectTypeBytes, sizeof(unsigned int), actorDataSize)) return false;	
	ObjectType objectType = ObjectType::fromUIntReversed(objectTypeBytes);
	if (objectType.primaryType == BumpActor)
	{
		bool isProBuild = (GLOBAL(settings).freeOrProLevel == AL_PRO);
		if (isProBuild && (objectType.ternaryType & WebThumbnail))
			actor = fsActor = wtActor = new WebThumbnailActor();
		else if (objectType.ternaryType & PhotoFrame)
			actor = fsActor = pfActor = new PhotoFrameActor();
		else if (objectType.secondaryType & FileSystem && (objectType.ternaryType & StickyNote))
			actor = fsActor = new StickyNoteActor();
		else if (objectType.secondaryType & FileSystem)
			actor = fsActor = new FileSystemActor();
		else if (objectType.secondaryType & Custom)
			actor = csActor = new CustomActor();
		else
			actor = new Actor();

		actor->setObjectType(objectType);
	}
	else
		return false;

	QString text;
	if (!SERIALIZE_READ_STRING(&actorData, text, actorDataSize)) return false;
	actor->setText(text);

	bool isTextVisible = false;
	if (!SERIALIZE_READ(&actorData, &isTextVisible, sizeof(bool), actorDataSize)) return false;
	if (!isTextVisible)
		actor->hideText(true);
	
	bool isPinned = false;
	if (!SERIALIZE_READ(&actorData, &isPinned, sizeof(bool), actorDataSize)) return false;

	bool isCrumpled = false;
	if (!SERIALIZE_READ(&actorData, &isCrumpled, sizeof(bool), actorDataSize)) return false;
	
	bool isPeeled = false;
	if (!SERIALIZE_READ(&actorData, &isPeeled, sizeof(bool), actorDataSize)) return false;
	
	Mat34 actorPose;
	if (!SERIALIZE_READ(&actorData, &actorPose, sizeof(Mat34), actorDataSize)) return false;
	actor->setGlobalPose(actorPose);

	Vec3 actorDims;
	if (!SERIALIZE_READ(&actorData, &actorDims, sizeof(Vec3), actorDataSize)) return false;
	actor->setDims(actorDims);

	Vec3 worldPinPoint(0.0f);
	Vec3 actorPinPoint(0.0f);
	if (!SERIALIZE_READ(&actorData, &worldPinPoint, sizeof(Vec3), actorDataSize)) return false;
	if (!SERIALIZE_READ(&actorData, &actorPinPoint, sizeof(Vec3), actorDataSize)) return false;
	if (isPinned)
	{
		actor->setPinPointInSpace(worldPinPoint);
		actor->setPinPointOnActor(actorPinPoint);
		actor->onPin(false);
	}

	Vec3 peelStartPoint(0.0f);
	Vec3 peelEndPoint(0.0f);
	if (!SERIALIZE_READ(&actorData, &peelStartPoint, sizeof(Vec3), actorDataSize)) return false;
	if (!SERIALIZE_READ(&actorData, &peelEndPoint, sizeof(Vec3), actorDataSize)) return false;

	// filesystem actor properties
	if (fsActor)
	{
		QString filePath;
		if (!SERIALIZE_READ_STRING(&actorData, filePath, actorDataSize)) return false;
		if (!filePath.isEmpty())
			fsActor->setFilePath(filePath);

		QString launchOverridePath;
		if (!SERIALIZE_READ_STRING(&actorData, launchOverridePath, actorDataSize)) return false;
		if (!launchOverridePath.isEmpty())
			fsActor->setLaunchOverride(launchOverridePath);
		
		QString textureOverridePath;
		if (!SERIALIZE_READ_STRING(&actorData, textureOverridePath, actorDataSize)) return false;
		if (!textureOverridePath.isEmpty())
			fsActor->setTextureOverride(textureOverridePath);

		bool isPilelized = false;
		if (!SERIALIZE_READ(&actorData, &isPilelized, sizeof(bool), actorDataSize)) return false;
		if (isPilelized)
			fsActor->pileize();

		bool isThumbnailized = false;
		if (!SERIALIZE_READ(&actorData, &isThumbnailized, sizeof(bool), actorDataSize)) return false;
		if (isThumbnailized)
			fsActor->enableThumbnail(true, false);

		if (versionNumber >= 4)
		{
			int actorLaunchCount = 0;
			if (!SERIALIZE_READ(&actorData, &actorLaunchCount, sizeof(int), actorDataSize)) return false;
			fsActor->setNumTimesLaunched(actorLaunchCount);
		}
	}

	// photoframe actors
	if (pfActor)
	{
		LocalPhotoFrameSource * localSource = NULL;
		RSSPhotoFrameSource * rssSource = NULL;
		PhotoFrameSource * source = NULL;

		unsigned int photoFrameType = 0;
		if (!SERIALIZE_READ(&actorData, &photoFrameType, sizeof(unsigned int), actorDataSize)) return false;		// PhotoFrameSourceType
		if (photoFrameType == LocalImageSource)
		{
			source = localSource = new LocalPhotoFrameSource();
			if (!source->unserialize(actorData, actorDataSize)) 
				return false;
		}
		else if (photoFrameType == RemoteImageSource)
		{	
			source = rssSource = new RSSPhotoFrameSource();
			source->unserialize(actorData, actorDataSize);
			if (rssSource->isFlickrPhotoFrame())
			{
				PhotoFrameSource * flickrSource = new FlickrPhotoFrameSource(rssSource->getRSSFeedUrl());
				SAFE_DELETE(source);
				source = flickrSource;
			}
		}

		if (source)
			pfActor->setUninitializedSource(source);
	}

	// custom actors
	if (csActor)
	{	
		QString uniqueObjectId;
		if (!SERIALIZE_READ_STRING(&actorData, uniqueObjectId, actorDataSize)) return false;
		CustomActorInfo * info = Singleton<CustomActorRegistry>::getInstance()->buildCustomActorInfo(uniqueObjectId);
		if (!info)
		{
			animManager->addAnimation(AnimationEntry(actor, (FinishedCallBack) DeleteActorAfterAnim));
		}
		else
		{
			/*
			QString name;
			if (!SERIALIZE_READ_STRING(&actorData, name, actorDataSize)) return false;
			info->name = name;

			QString textureId;
			if (!SERIALIZE_READ_STRING(&actorData, textureId, actorDataSize)) return false;
			if (info->textureId.isEmpty())
				info->textureId = winOS->GetUniqueID();

			QString texturePath;
			if (!SERIALIZE_READ_STRING(&actorData, texturePath, actorDataSize)) return false;
			if (!texturePath.isEmpty())
			{
				// set a new texture and put that into the info
				info->textureId = winOS->GetUniqueID();
				texMgr->loadTexture(GLTextureObject(Load|Compress, info->textureId, texturePath, HiResImage, HighPriority));
			}
			*/

			csActor->setCustomActorInfo(info);
		}
	}

	return actor;
}

Actor * LegacyPersistenceManager::unserializeActor_v5(unsigned int versionNumber, unsigned char * actorData, unsigned int& actorDataSize)
{
	PhotoFrameActor * pfActor = NULL;
	WebThumbnailActor * wtActor = NULL;
	FileSystemActor * fsActor = NULL;
	CustomActor * csActor = NULL;
	Actor * actor = NULL;

	unsigned int objectTypeBytes = 0;
	if (!SERIALIZE_READ(&actorData, &objectTypeBytes, sizeof(unsigned int), actorDataSize)) return false;	
	ObjectType objectType = ObjectType::fromUIntReversed(objectTypeBytes);
	if (objectType.primaryType == BumpActor)
	{
		bool isProBuild = (GLOBAL(settings).freeOrProLevel == AL_PRO);
		if (isProBuild && (objectType.ternaryType & WebThumbnail))
			actor = fsActor = wtActor = new WebThumbnailActor();
		else if (objectType.ternaryType & PhotoFrame)
			actor = fsActor = pfActor = new PhotoFrameActor();
		else if (objectType.secondaryType & FileSystem && (objectType.ternaryType & StickyNote))
			actor = fsActor = new StickyNoteActor();
		else if (objectType.secondaryType & FileSystem)
			actor = fsActor = new FileSystemActor();
		else if (objectType.secondaryType & Custom)
			actor = csActor = new CustomActor();
		else
			actor = new Actor();

		actor->setObjectType(objectType);
	}
	else
		return false;

	QString text;
	if (!SERIALIZE_READ_QSTRING(&actorData, text, actorDataSize)) return false;
	actor->setText(text);

	bool isTextVisible = false;
	if (!SERIALIZE_READ(&actorData, &isTextVisible, sizeof(bool), actorDataSize)) return false;
	if (!isTextVisible)
		actor->hideText(true);
	
	bool isPinned = false;
	if (!SERIALIZE_READ(&actorData, &isPinned, sizeof(bool), actorDataSize)) return false;

	bool isCrumpled = false;
	if (!SERIALIZE_READ(&actorData, &isCrumpled, sizeof(bool), actorDataSize)) return false;
	
	bool isPeeled = false;
	if (!SERIALIZE_READ(&actorData, &isPeeled, sizeof(bool), actorDataSize)) return false;
	
	Mat34 actorPose;
	if (!SERIALIZE_READ(&actorData, &actorPose, sizeof(Mat34), actorDataSize)) return false;
	actor->setGlobalPose(actorPose);

	Vec3 actorDims;
	if (!SERIALIZE_READ(&actorData, &actorDims, sizeof(Vec3), actorDataSize)) return false;
	actor->setDims(actorDims);

	Vec3 worldPinPoint(0.0f);
	Vec3 actorPinPoint(0.0f);
	if (!SERIALIZE_READ(&actorData, &worldPinPoint, sizeof(Vec3), actorDataSize)) return false;
	if (!SERIALIZE_READ(&actorData, &actorPinPoint, sizeof(Vec3), actorDataSize)) return false;
	if (isPinned)
	{
		actor->setPinPointInSpace(worldPinPoint);
		actor->setPinPointOnActor(actorPinPoint);
		actor->onPin(false);
	}

	Vec3 peelStartPoint(0.0f);
	Vec3 peelEndPoint(0.0f);
	if (!SERIALIZE_READ(&actorData, &peelStartPoint, sizeof(Vec3), actorDataSize)) return false;
	if (!SERIALIZE_READ(&actorData, &peelEndPoint, sizeof(Vec3), actorDataSize)) return false;

	// filesystem actor properties
	if (fsActor)
	{	
		// NOTE: we'll update the type to a post-it if setFilePath detects it, so for now
		// assume that everything is _not_ a post it
		fsActor->popFileSystemType(StickyNote);

		// assume a removable volume is unmounted first
		if (fsActor->isFileSystemType(Removable))
			fsActor->setMounted(false);

		QString filePath;
		if (!SERIALIZE_READ_QSTRING(&actorData, filePath, actorDataSize)) return false;
		if (!filePath.isEmpty())
			fsActor->setFilePath(filePath, fsActor->isFileSystemType(LogicalVolume));

		QString launchOverridePath;
		if (!SERIALIZE_READ_QSTRING(&actorData, launchOverridePath, actorDataSize)) return false;
		if (!launchOverridePath.isEmpty())
			fsActor->setLaunchOverride(launchOverridePath);
		
		QString textureOverridePath;
		if (!SERIALIZE_READ_QSTRING(&actorData, textureOverridePath, actorDataSize)) return false;
		if (!textureOverridePath.isEmpty())
			fsActor->setTextureOverride(textureOverridePath);

		bool isPilelized = false;
		if (!SERIALIZE_READ(&actorData, &isPilelized, sizeof(bool), actorDataSize)) return false;
		if (isPilelized)
			fsActor->pileize();

		bool isThumbnailized = false;
		if (!SERIALIZE_READ(&actorData, &isThumbnailized, sizeof(bool), actorDataSize)) return false;
		if (isThumbnailized)
			fsActor->enableThumbnail(true, false);

		if (versionNumber >= 4)
		{
			int actorLaunchCount = 0;
			if (!SERIALIZE_READ(&actorData, &actorLaunchCount, sizeof(int), actorDataSize)) return false;
			fsActor->setNumTimesLaunched(actorLaunchCount);
		}
	}

	// photoframe actors
	if (pfActor)
	{
		LocalPhotoFrameSource * localSource = NULL;
		RSSPhotoFrameSource * rssSource = NULL;
		PhotoFrameSource * source = NULL;

		unsigned int photoFrameType = 0;
		if (!SERIALIZE_READ(&actorData, &photoFrameType, sizeof(unsigned int), actorDataSize)) return false;		// PhotoFrameSourceType
		if (photoFrameType == LocalImageSource)
		{
			source = localSource = new LocalPhotoFrameSource();
			if (!source->unserialize(actorData, actorDataSize)) 
				return false;
		}
		else if (photoFrameType == RemoteImageSource)
		{	
			source = rssSource = new RSSPhotoFrameSource();
			source->unserialize(actorData, actorDataSize);
			if (rssSource->isFlickrPhotoFrame())
			{
				PhotoFrameSource * flickrSource = new FlickrPhotoFrameSource(rssSource->getRSSFeedUrl());
				SAFE_DELETE(source);
				source = flickrSource;
			}
		}

		if (source)
			pfActor->setUninitializedSource(source);
	}

	// custom actors
	if (csActor)
	{	
		QString uniqueObjectId;
		if (!SERIALIZE_READ_QSTRING(&actorData, uniqueObjectId, actorDataSize)) return false;
		CustomActorInfo * info = Singleton<CustomActorRegistry>::getInstance()->buildCustomActorInfo(uniqueObjectId);
		if (!info)
		{
			animManager->addAnimation(AnimationEntry(actor, (FinishedCallBack) DeleteActorAfterAnim));
		}
		else
		{
			/*
			QString name;
			if (!SERIALIZE_READ_QSTRING(&actorData, name, actorDataSize)) return false;
			info->name = name;

			QString textureId;
			if (!SERIALIZE_READ_QSTRING(&actorData, textureId, actorDataSize)) return false;
			if (info->textureId.isEmpty())
				info->textureId = winOS->GetUniqueID();

			QString texturePath;
			if (!SERIALIZE_READ_QSTRING(&actorData, texturePath, actorDataSize)) return false;
			if (!texturePath.isEmpty())
			{
				// set a new texture and put that into the info
				info->textureId = winOS->GetUniqueID();
				texMgr->loadTexture(GLTextureObject(Load|Compress, info->textureId, texturePath, HiResImage, HighPriority));
			}
			*/

			csActor->setCustomActorInfo(info);
		}
	}

	return actor;
}

Actor * LegacyPersistenceManager::unserializeActor_v6(unsigned int versionNumber, unsigned char * actorData, unsigned int& actorDataSize)
{
	PhotoFrameActor * pfActor = NULL;
	WebThumbnailActor * wtActor = NULL;
	FileSystemActor * fsActor = NULL;
	CustomActor * csActor = NULL;
	Actor * actor = NULL;

	unsigned int objectTypeBytes = 0;
	if (!SERIALIZE_READ(&actorData, &objectTypeBytes, sizeof(unsigned int), actorDataSize)) return false;	
	ObjectType objectType = ObjectType::fromUIntReversed(objectTypeBytes);
	if (objectType.primaryType == BumpActor)
	{
		bool isProBuild = (GLOBAL(settings).freeOrProLevel == AL_PRO);
		if (isProBuild && (objectType.ternaryType & WebThumbnail))
			actor = fsActor = wtActor = new WebThumbnailActor();
		else if (objectType.ternaryType & PhotoFrame)
			actor = fsActor = pfActor = new PhotoFrameActor();
		else if (objectType.secondaryType & FileSystem && (objectType.ternaryType & StickyNote))
			actor = fsActor = new StickyNoteActor();
		else if (objectType.secondaryType & FileSystem)
			actor = fsActor = new FileSystemActor();
		else if (objectType.secondaryType & Custom)
			actor = csActor = new CustomActor();
		else
			actor = new Actor();

		actor->setObjectType(objectType);
	}
	else
		return false;

	QString text;
	if (!SERIALIZE_READ_QSTRING(&actorData, text, actorDataSize)) return false;
	actor->setText(text);

	bool isTextVisible = false;
	if (!SERIALIZE_READ(&actorData, &isTextVisible, sizeof(bool), actorDataSize)) return false;
	if (!isTextVisible)
		actor->hideText(true);
	
	bool isPinned = false;
	if (!SERIALIZE_READ(&actorData, &isPinned, sizeof(bool), actorDataSize)) return false;

	bool isCrumpled = false;
	if (!SERIALIZE_READ(&actorData, &isCrumpled, sizeof(bool), actorDataSize)) return false;
	
	bool isPeeled = false;
	if (!SERIALIZE_READ(&actorData, &isPeeled, sizeof(bool), actorDataSize)) return false;
	
	Mat34 actorPose;
	if (!SERIALIZE_READ(&actorData, &actorPose, sizeof(Mat34), actorDataSize)) return false;
	actor->setGlobalPose(actorPose);

	Vec3 actorDims;
	if (!SERIALIZE_READ(&actorData, &actorDims, sizeof(Vec3), actorDataSize)) return false;
	actor->setDims(actorDims);

	Vec3 worldPinPoint(0.0f);
	Vec3 actorPinPoint(0.0f);
	if (!SERIALIZE_READ(&actorData, &worldPinPoint, sizeof(Vec3), actorDataSize)) return false;
	if (!SERIALIZE_READ(&actorData, &actorPinPoint, sizeof(Vec3), actorDataSize)) return false;
	if (isPinned)
	{
		actor->setPinPointInSpace(worldPinPoint);
		actor->setPinPointOnActor(actorPinPoint);
		actor->onPin(false);
	}

	Vec3 peelStartPoint(0.0f);
	Vec3 peelEndPoint(0.0f);
	if (!SERIALIZE_READ(&actorData, &peelStartPoint, sizeof(Vec3), actorDataSize)) return false;
	if (!SERIALIZE_READ(&actorData, &peelEndPoint, sizeof(Vec3), actorDataSize)) return false;

	// filesystem actor properties
	if (fsActor)
	{	
		// NOTE: we'll update the type to a post-it if setFilePath detects it, so for now
		// assume that everything is _not_ a post it
		fsActor->popFileSystemType(StickyNote);

		// assume a removable volume is unmounted first
		if (fsActor->isFileSystemType(Removable))
			fsActor->setMounted(false);

		QString filePath;
		if (!SERIALIZE_READ_QSTRING(&actorData, filePath, actorDataSize)) return false;
		if (!filePath.isEmpty())
			fsActor->setFilePath(filePath, fsActor->isFileSystemType(LogicalVolume));

		QString launchOverridePath;
		if (!SERIALIZE_READ_QSTRING(&actorData, launchOverridePath, actorDataSize)) return false;
		if (!launchOverridePath.isEmpty())
			fsActor->setLaunchOverride(launchOverridePath);
		
		QString textureOverridePath;
		if (!SERIALIZE_READ_QSTRING(&actorData, textureOverridePath, actorDataSize)) return false;
		if (!textureOverridePath.isEmpty())
			fsActor->setTextureOverride(textureOverridePath);

		bool isPilelized = false;
		if (!SERIALIZE_READ(&actorData, &isPilelized, sizeof(bool), actorDataSize)) return false;
		if (isPilelized)
			fsActor->pileize();

		bool isThumbnailized = false;
		if (!SERIALIZE_READ(&actorData, &isThumbnailized, sizeof(bool), actorDataSize)) return false;
		if (isThumbnailized)
			fsActor->enableThumbnail(true, false);

		if (versionNumber >= 4)
		{
			int actorLaunchCount = 0;
			if (!SERIALIZE_READ(&actorData, &actorLaunchCount, sizeof(int), actorDataSize)) return false;
			fsActor->setNumTimesLaunched(actorLaunchCount);
		}
	}

	// photoframe actors
	if (pfActor)
	{
		LocalPhotoFrameSource * localSource = NULL;
		RSSPhotoFrameSource * rssSource = NULL;
		FlickrPhotoFrameSource * flickrSource = NULL;
		PhotoFrameSource * source = NULL;

		unsigned int photoFrameType = 0;
		if (!SERIALIZE_READ(&actorData, &photoFrameType, sizeof(unsigned int), actorDataSize)) return false;		// PhotoFrameSourceType
		if (photoFrameType == LocalImageSource)
		{
			source = localSource = new LocalPhotoFrameSource();
			if (!source->unserialize(actorData, actorDataSize)) 
				return false;
		}
		else if (photoFrameType == RemoteImageSource)
		{	
			source = rssSource = new RSSPhotoFrameSource();
			source->unserialize(actorData, actorDataSize);
			if (rssSource->isFlickrPhotoFrame())
			{
				flickrSource = new FlickrPhotoFrameSource(rssSource->getRSSFeedUrl());
				SAFE_DELETE(source);
				source = flickrSource;
			}
		}
		else if (photoFrameType == FlickrImageSource)
		{
			source = flickrSource = new FlickrPhotoFrameSource();
			source->unserialize(actorData, actorDataSize);
		}

		if (source)
			pfActor->setUninitializedSource(source);
	}

	// custom actors
	if (csActor)
	{	
		QString uniqueObjectId;
		if (!SERIALIZE_READ_QSTRING(&actorData, uniqueObjectId, actorDataSize)) return false;
		CustomActorInfo * info = Singleton<CustomActorRegistry>::getInstance()->buildCustomActorInfo(uniqueObjectId);
		if (!info)
		{
			animManager->addAnimation(AnimationEntry(actor, (FinishedCallBack) DeleteActorAfterAnim));
		}
		else
		{
			/*
			QString name;
			if (!SERIALIZE_READ_QSTRING(&actorData, name, actorDataSize)) return false;
			info->name = name;

			QString textureId;
			if (!SERIALIZE_READ_QSTRING(&actorData, textureId, actorDataSize)) return false;
			if (info->textureId.isEmpty())
				info->textureId = winOS->GetUniqueID();

			QString texturePath;
			if (!SERIALIZE_READ_QSTRING(&actorData, texturePath, actorDataSize)) return false;
			if (!texturePath.isEmpty())
			{
				// set a new texture and put that into the info
				info->textureId = winOS->GetUniqueID();
				texMgr->loadTexture(GLTextureObject(Load|Compress, info->textureId, texturePath, HiResImage, HighPriority));
			}
			*/

			csActor->setCustomActorInfo(info);
		}
	}

	return actor;
}

Actor * LegacyPersistenceManager::unserializeActor_v7(unsigned int versionNumber, unsigned char * actorData, unsigned int& actorDataSize)
{
	PhotoFrameActor * pfActor = NULL;
	WebThumbnailActor * wtActor = NULL;
	FileSystemActor * fsActor = NULL;
	CustomActor * csActor = NULL;
	Actor * actor = NULL;

	unsigned int objectTypeBytes = 0;
	if (!SERIALIZE_READ(&actorData, &objectTypeBytes, sizeof(unsigned int), actorDataSize)) return false;	
	ObjectType objectType = ObjectType::fromUIntReversed(objectTypeBytes);
	if (objectType.primaryType == BumpActor)
	{
		bool isProBuild = (GLOBAL(settings).freeOrProLevel == AL_PRO);
		if (isProBuild && (objectType.ternaryType & WebThumbnail))
			actor = fsActor = wtActor = new WebThumbnailActor();
		else if (objectType.ternaryType & PhotoFrame)
			actor = fsActor = pfActor = new PhotoFrameActor();
		else if (objectType.secondaryType & FileSystem && (objectType.ternaryType & StickyNote))
			actor = fsActor = new StickyNoteActor();
		else if (objectType.secondaryType & FileSystem)
			actor = fsActor = new FileSystemActor();
		else if (objectType.secondaryType & Custom)
			actor = csActor = new CustomActor();
		else
			actor = new Actor();

		actor->setObjectType(objectType);
	}
	else
		return false;

	QString text;
	if (!SERIALIZE_READ_QSTRING(&actorData, text, actorDataSize)) return false;
	actor->setText(text);

	bool isTextVisible = false;
	if (!SERIALIZE_READ(&actorData, &isTextVisible, sizeof(bool), actorDataSize)) return false;
	if (!isTextVisible)
		actor->hideText(true);

	bool isPinned = false;
	if (!SERIALIZE_READ(&actorData, &isPinned, sizeof(bool), actorDataSize)) return false;

	bool isCrumpled = false;
	if (!SERIALIZE_READ(&actorData, &isCrumpled, sizeof(bool), actorDataSize)) return false;
	
	bool isPeeled = false;
	if (!SERIALIZE_READ(&actorData, &isPeeled, sizeof(bool), actorDataSize)) return false;
	
	Mat34 actorPose;
	if (!SERIALIZE_READ(&actorData, &actorPose, sizeof(Mat34), actorDataSize)) return false;
	actor->setGlobalPose(actorPose);

	Vec3 actorDims;
	if (!SERIALIZE_READ(&actorData, &actorDims, sizeof(Vec3), actorDataSize)) return false;
	actor->setDims(actorDims);

	Vec3 worldPinPoint(0.0f);
	Vec3 actorPinPoint(0.0f);
	if (!SERIALIZE_READ(&actorData, &worldPinPoint, sizeof(Vec3), actorDataSize)) return false;
	if (!SERIALIZE_READ(&actorData, &actorPinPoint, sizeof(Vec3), actorDataSize)) return false;
	if (isPinned)
	{
		actor->setPinPointInSpace(worldPinPoint);
		actor->setPinPointOnActor(actorPinPoint);
		actor->onPin(false);
	}

	Vec3 peelStartPoint(0.0f);
	Vec3 peelEndPoint(0.0f);
	if (!SERIALIZE_READ(&actorData, &peelStartPoint, sizeof(Vec3), actorDataSize)) return false;
	if (!SERIALIZE_READ(&actorData, &peelEndPoint, sizeof(Vec3), actorDataSize)) return false;

	// filesystem actor properties
	if (fsActor)
	{	
		// NOTE: we'll update the type to a post-it if setFilePath detects it, so for now
		// assume that everything is _not_ a post it
		fsActor->popFileSystemType(StickyNote);

		// assume a removable volume is unmounted first
		if (fsActor->isFileSystemType(Removable))
			fsActor->setMounted(false);

		QString filePath;
		if (!SERIALIZE_READ_QSTRING(&actorData, filePath, actorDataSize)) return false;
		if (!filePath.isEmpty())
			fsActor->setFilePath(filePath, fsActor->isFileSystemType(LogicalVolume));

		QString launchOverridePath;
		if (!SERIALIZE_READ_QSTRING(&actorData, launchOverridePath, actorDataSize)) return false;
		if (!launchOverridePath.isEmpty())
			fsActor->setLaunchOverride(launchOverridePath);
		
		QString textureOverridePath;
		if (!SERIALIZE_READ_QSTRING(&actorData, textureOverridePath, actorDataSize)) return false;
		if (!textureOverridePath.isEmpty())
			fsActor->setTextureOverride(textureOverridePath);

		bool isPilelized = false;
		if (!SERIALIZE_READ(&actorData, &isPilelized, sizeof(bool), actorDataSize)) return false;
		if (isPilelized)
			fsActor->pileize();

		bool isThumbnailized = false;
		if (!SERIALIZE_READ(&actorData, &isThumbnailized, sizeof(bool), actorDataSize)) return false;
		if (isThumbnailized)
			fsActor->enableThumbnail(true, false);

		if (versionNumber >= 4)
		{
			int actorLaunchCount = 0;
			if (!SERIALIZE_READ(&actorData, &actorLaunchCount, sizeof(int), actorDataSize)) return false;
			fsActor->setNumTimesLaunched(actorLaunchCount);
		}

		if (fsActor->isFileSystemType(Folder))
		{
			int folderContentDimensions = 0;
			if (SERIALIZE_READ(&actorData, &folderContentDimensions, sizeof(int), actorDataSize))
			{
				QString filePath;
				Vec3 fileDims(0.0f);
				for (int i = 0; i < folderContentDimensions; ++i)
				{
					if (SERIALIZE_READ_QSTRING(&actorData, filePath, actorDataSize) && 
						SERIALIZE_READ_VEC3(&actorData, fileDims, actorDataSize))
						fsActor->addFolderContentDimensions(filePath, fileDims);
				}
			}
		}
	}

	// photoframe actors
	if (pfActor)
	{
		LocalPhotoFrameSource * localSource = NULL;
		RSSPhotoFrameSource * rssSource = NULL;
		FlickrPhotoFrameSource * flickrSource = NULL;
		PhotoFrameSource * source = NULL;

		unsigned int photoFrameType = 0;
		if (!SERIALIZE_READ(&actorData, &photoFrameType, sizeof(unsigned int), actorDataSize)) return false;		// PhotoFrameSourceType
		if (photoFrameType == LocalImageSource)
		{
			source = localSource = new LocalPhotoFrameSource();
			if (!source->unserialize(actorData, actorDataSize)) 
				return false;
		}
		else if (photoFrameType == RemoteImageSource)
		{	
			source = rssSource = new RSSPhotoFrameSource();
			source->unserialize(actorData, actorDataSize);
			if (rssSource->isFlickrPhotoFrame())
			{
				flickrSource = new FlickrPhotoFrameSource(rssSource->getRSSFeedUrl());
				SAFE_DELETE(source);
				source = flickrSource;
			}
		}
		else if (photoFrameType == FlickrImageSource)
		{
			source = flickrSource = new FlickrPhotoFrameSource();
			source->unserialize(actorData, actorDataSize);
		}

		if (source)
			pfActor->setUninitializedSource(source);
	}

	// custom actors
	if (csActor)
	{	
		QString uniqueObjectId;
		if (!SERIALIZE_READ_QSTRING(&actorData, uniqueObjectId, actorDataSize)) return false;
		CustomActorInfo * info = Singleton<CustomActorRegistry>::getInstance()->buildCustomActorInfo(uniqueObjectId);
		if (!info)
		{
			animManager->addAnimation(AnimationEntry(actor, (FinishedCallBack) DeleteActorAfterAnim));
		}
		else
		{
			/*
			QString name;
			if (!SERIALIZE_READ_QSTRING(&actorData, name, actorDataSize)) return false;
			info->name = name;

			QString textureId;
			if (!SERIALIZE_READ_QSTRING(&actorData, textureId, actorDataSize)) return false;
			if (info->textureId.isEmpty())
				info->textureId = winOS->GetUniqueID();

			QString texturePath;
			if (!SERIALIZE_READ_QSTRING(&actorData, texturePath, actorDataSize)) return false;
			if (!texturePath.isEmpty())
			{
				// set a new texture and put that into the info
				info->textureId = winOS->GetUniqueID();
				texMgr->loadTexture(GLTextureObject(Load|Compress, info->textureId, texturePath, HiResImage, HighPriority));
			}
			*/

			csActor->setCustomActorInfo(info);
		}
	}

	return actor;
}

Actor * LegacyPersistenceManager::unserializeActor_v8(unsigned int versionNumber, unsigned char * actorData, unsigned int& actorDataSize)
{
	PhotoFrameActor * pfActor = NULL;
	WebThumbnailActor * wtActor = NULL;
	FileSystemActor * fsActor = NULL;
	CustomActor * csActor = NULL;
	WebActor * webActor = NULL;
	Actor * actor = NULL;

	unsigned int objectTypeBytes = 0;
	if (!SERIALIZE_READ(&actorData, &objectTypeBytes, sizeof(unsigned int), actorDataSize)) return false;	
	ObjectType objectType = ObjectType::fromUIntReversed(objectTypeBytes);
	if (objectType.primaryType == BumpActor)
	{
		bool isProBuild = (GLOBAL(settings).freeOrProLevel == AL_PRO);
		if (isProBuild && (objectType.ternaryType & WebThumbnail))
			actor = fsActor = wtActor = new WebThumbnailActor();
		else if (objectType.ternaryType & PhotoFrame)
			actor = fsActor = pfActor = new PhotoFrameActor();
		else if (objectType.secondaryType & FileSystem && (objectType.ternaryType & StickyNote))
			actor = fsActor = new StickyNoteActor();
		else if (objectType.secondaryType & FileSystem)
			actor = fsActor = new FileSystemActor();
		else if (objectType.secondaryType & Custom)
			actor = csActor = new CustomActor();
		else if (objectType.secondaryType & Webpage)
			actor = webActor = new WebActor();
		else
			actor = new Actor();

		actor->setObjectType(objectType);
	}
	else
		return false;

	QString text;
	if (!SERIALIZE_READ_QSTRING(&actorData, text, actorDataSize)) return false;
	actor->setText(text);

	bool isTextVisible = false;
	if (!SERIALIZE_READ(&actorData, &isTextVisible, sizeof(bool), actorDataSize)) return false;
	if (!isTextVisible)
		actor->hideText(true);
	
	bool isPinned = false;
	if (!SERIALIZE_READ(&actorData, &isPinned, sizeof(bool), actorDataSize)) return false;

	bool isCrumpled = false;
	if (!SERIALIZE_READ(&actorData, &isCrumpled, sizeof(bool), actorDataSize)) return false;
	
	bool isPeeled = false;
	if (!SERIALIZE_READ(&actorData, &isPeeled, sizeof(bool), actorDataSize)) return false;
	
	Mat34 actorPose;
	if (!SERIALIZE_READ(&actorData, &actorPose, sizeof(Mat34), actorDataSize)) return false;
	actor->setGlobalPose(actorPose);

	Vec3 actorDims;
	if (!SERIALIZE_READ(&actorData, &actorDims, sizeof(Vec3), actorDataSize)) return false;
	actor->setDims(actorDims);

	Vec3 worldPinPoint(0.0f);
	Vec3 actorPinPoint(0.0f);
	if (!SERIALIZE_READ(&actorData, &worldPinPoint, sizeof(Vec3), actorDataSize)) return false;
	if (!SERIALIZE_READ(&actorData, &actorPinPoint, sizeof(Vec3), actorDataSize)) return false;
	if (isPinned)
	{
		actor->setPinPointInSpace(worldPinPoint);
		actor->setPinPointOnActor(actorPinPoint);
		actor->onPin(false);
	}

	Vec3 peelStartPoint(0.0f);
	Vec3 peelEndPoint(0.0f);
	if (!SERIALIZE_READ(&actorData, &peelStartPoint, sizeof(Vec3), actorDataSize)) return false;
	if (!SERIALIZE_READ(&actorData, &peelEndPoint, sizeof(Vec3), actorDataSize)) return false;

	// filesystem actor properties
	if (fsActor)
	{	
		// NOTE: we'll update the type to a post-it if setFilePath detects it, so for now
		// assume that everything is _not_ a post it
		fsActor->popFileSystemType(StickyNote);

		// assume a removable volume is unmounted first
		if (fsActor->isFileSystemType(Removable))
			fsActor->setMounted(false);

		QString filePath;
		if (!SERIALIZE_READ_QSTRING(&actorData, filePath, actorDataSize)) return false;
		if (!filePath.isEmpty())
			fsActor->setFilePath(filePath, fsActor->isFileSystemType(LogicalVolume));

		QString launchOverridePath;
		if (!SERIALIZE_READ_QSTRING(&actorData, launchOverridePath, actorDataSize)) return false;
		if (!launchOverridePath.isEmpty())
			fsActor->setLaunchOverride(launchOverridePath);
		
		QString textureOverridePath;
		if (!SERIALIZE_READ_QSTRING(&actorData, textureOverridePath, actorDataSize)) return false;
		if (!textureOverridePath.isEmpty())
			fsActor->setTextureOverride(textureOverridePath);

		bool isPilelized = false;
		if (!SERIALIZE_READ(&actorData, &isPilelized, sizeof(bool), actorDataSize)) return false;
		if (isPilelized)
			fsActor->pileize();

		bool isThumbnailized = false;
		if (!SERIALIZE_READ(&actorData, &isThumbnailized, sizeof(bool), actorDataSize)) return false;
		if (isThumbnailized)
			fsActor->enableThumbnail(true, false);

		if (versionNumber >= 4)
		{
			int actorLaunchCount = 0;
			if (!SERIALIZE_READ(&actorData, &actorLaunchCount, sizeof(int), actorDataSize)) return false;
			fsActor->setNumTimesLaunched(actorLaunchCount);
		}

		if (fsActor->isFileSystemType(Folder))
		{
			int folderContentDimensions = 0;
			if (SERIALIZE_READ(&actorData, &folderContentDimensions, sizeof(int), actorDataSize))
			{
				QString filePath;
				Vec3 fileDims(0.0f);
				for (int i = 0; i < folderContentDimensions; ++i)
				{
					if (SERIALIZE_READ_QSTRING(&actorData, filePath, actorDataSize) && 
						SERIALIZE_READ_VEC3(&actorData, fileDims, actorDataSize))
						fsActor->addFolderContentDimensions(filePath, fileDims);
				}
			}
		}
	}

	// photoframe actors
	if (pfActor)
	{
		LocalPhotoFrameSource * localSource = NULL;
		RSSPhotoFrameSource * rssSource = NULL;
		FlickrPhotoFrameSource * flickrSource = NULL;
		PhotoFrameSource * source = NULL;

		unsigned int photoFrameType = 0;
		if (!SERIALIZE_READ(&actorData, &photoFrameType, sizeof(unsigned int), actorDataSize)) return false;
		if (photoFrameType == LocalImageSource)
		{
			source = localSource = new LocalPhotoFrameSource();
			if (!source->unserialize(actorData, actorDataSize)) 
				return false;
		}
		else if (photoFrameType == RemoteImageSource)
		{	
			source = rssSource = new RSSPhotoFrameSource();
			source->unserialize(actorData, actorDataSize);
			if (rssSource->isFlickrPhotoFrame())
			{
				flickrSource = new FlickrPhotoFrameSource(rssSource->getRSSFeedUrl());
				SAFE_DELETE(source);
				source = flickrSource;
			}
		}
		else if (photoFrameType == FlickrImageSource)
		{
			source = flickrSource = new FlickrPhotoFrameSource();
			source->unserialize(actorData, actorDataSize);
		}

		if (source)
			pfActor->setUninitializedSource(source);
	}

	// custom actors
	if (csActor)
	{	
		QString uniqueObjectId;
		if (!SERIALIZE_READ_QSTRING(&actorData, uniqueObjectId, actorDataSize)) return false;
		if (!uniqueObjectId.isEmpty())
		{
			CustomActorInfo * info = Singleton<CustomActorRegistry>::getInstance()->buildCustomActorInfo(uniqueObjectId);
			if (!info)
				animManager->addAnimation(AnimationEntry(actor, (FinishedCallBack) DeleteActorAfterAnim));
			else
				csActor->setCustomActorInfo(info);
		}

		/*
		QString name;
		if (!SERIALIZE_READ_QSTRING(&actorData, name, actorDataSize)) return false;
		info->name = name;

		QString textureId;
		if (!SERIALIZE_READ_QSTRING(&actorData, textureId, actorDataSize)) return false;
		if (info->textureId.isEmpty())
			info->textureId = winOS->GetUniqueID();

		QString texturePath;
		if (!SERIALIZE_READ_QSTRING(&actorData, texturePath, actorDataSize)) return false;
		if (!texturePath.isEmpty())
		{
			// set a new texture and put that into the info
			info->textureId = winOS->GetUniqueID();
			texMgr->loadTexture(GLTextureObject(Load|Compress, info->textureId, texturePath, HiResImage, HighPriority));
		}
		*/
	}

	// web actors
	if (webActor)
	{
		bool isPageDataUrl = true;
		if (!SERIALIZE_READ(&actorData, &isPageDataUrl, sizeof(bool), actorDataSize)) return false;

		QString pageData;
		if (!SERIALIZE_READ_QSTRING(&actorData, pageData, actorDataSize)) return false;

		// If we can make more Web Actors (pro version or free version with 2 or less)
		if(WebActor::canMakeMore()) {
			if (isPageDataUrl)
				webActor->load(pageData);
			else
				webActor->loadHTML(pageData);
		} else { // Otherwise, downgrade the actors to .URL or .html files
			if ( (isPageDataUrl) && (!pageData.isEmpty()) ) {
				QUrl pageUrl = QUrl(pageData);
				QString pageDomain = pageUrl.host();
				QString ext = ".URL";

				pageDomain.append(ext);

				QFileInfo newUrlFile = fsManager->getUniqueNewFilePathInWorkingDirectory(pageDomain);
				QString contents = "[InternetShortcut]\r\nURL=";
				contents.append(pageData);

				write_file_utf8(contents,native(newUrlFile));
			} else {
				QString HTMLFile = "WebWidget.html";
				QFileInfo newHTMLFile = fsManager->getUniqueNewFilePathInWorkingDirectory(HTMLFile);
				write_file_utf8(pageData,native(newHTMLFile));
			}
			FadeAndDeleteActor(actor);
		}
	}

	return actor;
}

/*
bool LegacyPersistenceManager::saveScene(const QString& filePath)
{
	assert(false);
	return false;
}

bool LegacyPersistenceManager::saveScene(unsigned char **dataStream, uint &sz)
{
	// not going to be used any more

	// SCENE VERSION NUMBER
	// Everytime the scene file changes, increment this variable.
	// Also, handle loading for different types in unserializeActor();
	// DO A SEARCH FOR 'verNum' and make sure all references are updated.
	// THEN UPDATE SCENE_VERSION_NUMBER inside BT_Util.h
	uint versionNumber = SCENE_VERSION_NUMBER;
	
	if (dataStream)
	{
		//DEBUG_TIMER_RESET;
		vector<unsigned char *> saveList;
		vector<uint> saveListSize;
		vector<NxActorWrapper *> actors;
		vector<Pile *> pileList;
		uint numActors = 0, numPiles = 0, totalSize = (16 + sizeof(int) + 1024);
		unsigned char extHeaderDummyData[SERIALIZE_MAX_SIZE], *dataBuffer;

		// Figure out how many SAVABLE piles there are
		for (int i = 0; i < GLOBAL(getPiles()).size(); i++)
		{
			Pile *pile = GLOBAL(getPiles())[i];

			if (pile)
			{
				if (pile->getPileType() == SoftPile)
				{
					numPiles++;
					pileList.push_back(pile);

					totalSize += sizeof(int) + sizeof(int) + sizeof(int) + (SERIALIZE_MAX_SIZE + (pile->getNumItems() * (SERIALIZE_MAX_SIZE + sizeof(int) + 68)));
				}

				// Remove piled Actors from the List
				for (int j = 0; j < pile->getNumItems(); j++)
				{
					// Remove this item from being saved later on in the actor data save cycle
					for (uint k = 0; k < actors.size(); k++)
					{
						if (pile->isInPile(GetBumpObject(actors[k])) > -1)
						{
							actors.erase(actors.begin() + k);
						}
					}
				}
			}
		}

		// Figure out how mane SAVABLE actors we have
		vector<NxActorWrapper *> activeNxActorList = GLOBAL(activeNxActorList);
		for (int i = 0; i < activeNxActorList.size(); i++)
		{
			Actor *data = GetBumpActor(activeNxActorList[i]);

			if (data && 
				data->isBumpObjectType(BumpActor) && 
				!data->getParent() && 
				(data->isActorType(Webpage) || data->isActorType(Custom) || data->isActorType(FileSystem) || data->isActorType(Logical)) && !data->isActorType(Temporary))
			{
				// Add only Actors with a Data component
				numActors++;
				actors.push_back(activeNxActorList[i]);
				totalSize += sizeof(int) + (SERIALIZE_MAX_SIZE);
			}
		}
		// Determine how many bytes serializing the camera will take
		totalSize += cam->getSizeOfCameraSerialize();

		sz = totalSize;
		*dataStream = new unsigned char[totalSize];
		dataBuffer = *dataStream;

		// ===========================================================
		//						HEADER [16 BYTES]
		// ===========================================================

		// Save the number of actors and piles in the scene
		if (!SERIALIZE_WRITE(&dataBuffer, sizeof(int), totalSize, &numActors)) return false;
		if (!SERIALIZE_WRITE(&dataBuffer, sizeof(int), totalSize, &numPiles)) return false;


		// ===========================================================
		//			       EXTENDED HEADER [1024 BYTES]
		// ===========================================================

		// This is dummy Data
		memset(extHeaderDummyData, NULL, SERIALIZE_MAX_SIZE);
		
		// This saves the extended header struct into the extended header dummy data array (which is padded with 0's to fill 1024 bytes)
		ExtendedHeader extHead;
		extHead.versionNumber = versionNumber; // present in version 2

		extHead.isInfiniteDesktopTurnedOn = GLOBAL(isInInfiniteDesktopMode); // present in version 3 

		assert(sizeof(ExtendedHeader) <= SERIALIZE_MAX_SIZE); // very important
		*((ExtendedHeader*)extHeaderDummyData) = extHead;
		if (!SERIALIZE_WRITE(&dataBuffer, SERIALIZE_MAX_SIZE, totalSize, &extHeaderDummyData[0])) return false;


		// ===========================================================
		//	   PILE [12 + (1024 + (#items * (1024 + 68))) BYTES]
		// ===========================================================
		for (int i = 0; i < pileList.size(); i++)
		{
			Pile *pile = pileList[i];

			// This is a Soft Pile    |      Pile Data     + (     Pile Items       * (   Actor Data     +  AD Header  + Footer))
			uint pileDataSize = SERIALIZE_MAX_SIZE + (pile->getNumItems() * (SERIALIZE_MAX_SIZE + sizeof(int) + 68));
			uint pileWriteSize = pileDataSize;
			unsigned char *buf = new unsigned char[pileDataSize];
			memset(buf, NULL, pileDataSize);

			// Save the Pile Header & Actors
			pile->serialize(buf, pileDataSize);
			if (!SERIALIZE_WRITE(&dataBuffer, sizeof(int), totalSize, &pileWriteSize)) return false;
			if (!SERIALIZE_WRITE(&dataBuffer, pileWriteSize, totalSize, buf)) return false;

			SAFE_DELETE_ARRAY(buf);
		}



		// ===========================================================
		//			  	     ACTOR DATA [1024 BYTES]
		// ===========================================================
		for (int i = 0; i < actors.size(); i++)
		{
			Actor *data = GetBumpActor(actors[i]);

			// skip the actor if it's no longer in the scene
			if (!scnManager->containsObject(data))
				continue;

			// Save actors with Data
			if (data)
			{
				unsigned char buf[SERIALIZE_MAX_SIZE];
				uint bufSz = SERIALIZE_MAX_SIZE;
				memset(buf, NULL, SERIALIZE_MAX_SIZE);

				// Serialize
				data->serialize((unsigned char *) &buf, bufSz);
				if (!SERIALIZE_WRITE(&dataBuffer, SERIALIZE_MAX_SIZE, totalSize, buf)) return false;
			}
		}
		
		// ===========================================================
		//			  	     CAMERA DATA [72 BYTES]
		// ===========================================================
		cam->serialize(dataBuffer, totalSize);

		return (totalSize == 0);
	}

	return false;
}
*/