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

template<class T>
CustomActor * SceneManager::getCustomActor()
{
	CustomActorInfo * tmpInfo = new CustomActorInfo;
	CustomActorImpl * tmpImpl = new T(tmpInfo);

	CustomActor * actor = NULL;
	const vector<BumpObject *>& objects = getBumpObjects();
	for (int i = 0; i < objects.size(); ++i)
	{
		if (objects[i]->getObjectType() == ObjectType(BumpActor, Custom))
		{
			CustomActor * customActor = (CustomActor *) objects[i];
			if (customActor->isCustomImplementationType(tmpImpl))
			{
				actor = customActor;
			}
		}
	}	

	SAFE_DELETE(tmpInfo);		// implicitly deletes the handler
	return actor;
}