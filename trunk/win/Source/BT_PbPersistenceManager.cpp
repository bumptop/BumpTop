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
#include "BT_Actor.h"
#include "BT_Authorization.h"
#include "BT_BumpObject.h"
#include "BT_CustomActor.h"
#include "BT_FileSystemActor.h"
#include "BT_PbPersistenceManager.h"
#include "BT_PhotoFrameActor.h"
#include "BT_Pile.h"
#include "BT_QtUtil.h"
#include "BT_Selection.h"
#include "BT_SceneManager.h"
#include "BT_StickyNoteActor.h"
#include "BT_SVNRevision.h"
#include "BT_WebActor.h"
#include "BT_WebThumbnailActor.h"
#include "BT_WindowsOS.h"
#include "BumpTop.pb.h"

PbPersistenceManager::PbPersistenceManager()
{
	GOOGLE_PROTOBUF_VERIFY_VERSION;
}

PbPersistenceManager::~PbPersistenceManager()
{
	// OnUnload:
	google::protobuf::ShutdownProtobufLibrary();
}

bool PbPersistenceManager::loadScene(const QString& filePath)
{	
	if (!exists(filePath))
		return false;

	PbBumpTop bumptop;

	// read the bumptop root from disk
	std::ifstream fileIn(filePath.utf16(), ios::binary);
	if (!bumptop.ParseFromIstream(&fileIn))
	{
		fileIn.close();
		return false;
	}
	fileIn.close();

	// deserialize the header (for debugging mostly)
	if (bumptop.has_header())
	{
		const PbHeader& header = bumptop.header();
		QString buildStr;
		if (header.has_build())
			buildStr = qstring(header.build());
		unsigned int versionNumber = 0;
		if (header.has_version())
			versionNumber = header.version();
	}

	// deserialize the scene data
	if (bumptop.has_scene())
	{
		if (!scnManager->deserializeFromPb(&bumptop.scene()))
		{
			assert(false);
			return false;
		}
	}

	// cross reference the files in the scene data with the files actually
	// in the working directory
	scnManager->crossReference();
	sel->clear();

	LOG("PbPersistenceManager::loadScene successful");
	return true;
}

bool PbPersistenceManager::saveScene(const QString& filePath)
{
	PbBumpTop bumptop;

	// save the header
	if (bumptop.mutable_header())
	{
		PbHeader * header = bumptop.mutable_header();

		QString buildStr;
		QTextStream stream(&buildStr);
		stream << "BumpTop,";
		stream << winOS->BumpTopEditionName(winOS->GetBumpTopEdition()) << ",";
		stream << ((GLOBAL(settings).freeOrProLevel == AL_PRO) ? "Pro," : "Free,");	
		stream << winOS->GetBuildNumber() << ",";
		stream << winOS->GetLocaleLanguage() << ",";
		stream << "win32";

		header->set_build(stdString(buildStr));
		header->set_version(atoi(SVN_VERSION_NUMBER));
	}

	// save the scene data
	if (!scnManager->serializeToPb(bumptop.mutable_scene()))
	{
		assert(false);
		return false;
	}

	// write the bumptop root to disk
	std::ofstream fileOut(filePath.utf16(), ios::trunc | ios::binary);
	if (!bumptop.SerializeToOstream(&fileOut))
	{
		fileOut.close();
		return false;
	}
	fileOut.close();
	
	bool ret = bumptop.IsInitialized();
	if (ret)
		LOG("PbPersistenceManager::saveScene successful");
	return ret;
}

BumpObject * PbPersistenceManager::deserializeBumpObjectFromPb(const PbBumpObject * object)
{
	BumpObject * newObject = NULL;

	// NOTE: in the future, we can also use the class type factory to 
	// build the correct bump object type

	// create the object according to the type
	if (object->has_type())
	{
		ObjectType type = ObjectType::fromUInt(object->type());

		if (type.primaryType == BumpActor)
		{
			if (type.secondaryType & FileSystem)
			{
				if (type.ternaryType & PhotoFrame)
					newObject = new PhotoFrameActor();
				else if (type.ternaryType & WebThumbnail)
					newObject = new WebThumbnailActor();
				else if (type.ternaryType & StickyNote)
					newObject = new StickyNoteActor();
				else
					newObject = new FileSystemActor();
			}
			else if (type.secondaryType & Custom)
				newObject = new CustomActor();
			else if (type.secondaryType & Webpage)
				newObject = new WebActor();
			else
				assert(false);
		}
		else if (type.primaryType == BumpPile)
		{
			newObject = new Pile();
		}
		else if (type.primaryType == BumpCluster)
		{
			
			//do not deserialize bumpclusters right now
			//TODO double check on commit
			assert(false);
		}
		else
			assert(false);
	}

	// deserialize the object
	if (newObject)
	{
		if (!newObject->deserializeFromPb(object))
		{
			SAFE_DELETE(newObject);
			return NULL;
		}
	}

	return newObject;
}

PbPersistenceManager * PbPersistenceManager::getInstance()
{
	return Singleton<PbPersistenceManager>::getInstance();
}