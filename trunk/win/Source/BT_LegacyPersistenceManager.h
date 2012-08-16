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

#ifndef BT_LEGACYPERSISTENCEMANAGER_H
#define BT_LEGACYPERSISTENCEMANAGER_H

#include "BT_Singleton.h"

#define SCENE_VERSION_NUMBER	15

// forward declares
class Pile;
class Actor;

class LegacyPersistenceManager 
{
private:
	Pile * unserializePile(unsigned int versionNumber, unsigned char * pileData, unsigned int& pileDataSize);
	Pile * unserializePile_v1(unsigned int versionNumber, unsigned char * pileData, unsigned int& pileDataSize);
	Pile * unserializePile_v2(unsigned int versionNumber, unsigned char * pileData, unsigned int& pileDataSize);
	Actor * unserializeActor(unsigned int versionNumber, unsigned char * actorData, unsigned int& actorDataSize);
	Actor * unserializeActor_v1(unsigned int versionNumber, unsigned char * actorData, unsigned int& actorDataSize);
	Actor * unserializeActor_v2(unsigned int versionNumber, unsigned char * actorData, unsigned int& actorDataSize);
	Actor * unserializeActor_v3(unsigned int versionNumber, unsigned char * actorData, unsigned int& actorDataSize);
	Actor * unserializeActor_v4(unsigned int versionNumber, unsigned char * actorData, unsigned int& actorDataSize);
	Actor * unserializeActor_v5(unsigned int versionNumber, unsigned char * actorData, unsigned int& actorDataSize);
	Actor * unserializeActor_v6(unsigned int versionNumber, unsigned char * actorData, unsigned int& actorDataSize);
	Actor * unserializeActor_v7(unsigned int versionNumber, unsigned char * actorData, unsigned int& actorDataSize);
	Actor * unserializeActor_v8(unsigned int versionNumber, unsigned char * actorData, unsigned int& actorDataSize);

	friend class Singleton<LegacyPersistenceManager>;
	LegacyPersistenceManager();	

public:
	bool loadScene(const QString& filePath);
	bool loadSceneData(unsigned char * dataStream, unsigned int dataStreamSize);

	static LegacyPersistenceManager * getInstance();
};

#endif // BT_LEGACYPERSISTENCEMANAGER_H