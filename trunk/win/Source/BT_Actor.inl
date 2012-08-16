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

bool Actor::isActorType(uint actorType)
{
	// Return a BitMask that is located on the Tertiary Section of the ObjectType (See struct ObjectType)
	return (type.secondaryType & actorType) ? true : false;
}

bool Actor::isMaximized()
{
	return maximized;
}

QString Actor::getTextureString()
{
	return textureID;
}

#ifdef DXRENDER
IDirect3DVertexBuffer9 * Actor::getDisplayListId() const
#else
uint Actor::getDisplayListId() const
#endif
{
	return _displayListId;
}

BumpObject *Actor::getObjectToMimic() const
{
	return actorToMimic;
}

void Actor::setObjectToMimic(BumpObject *obj)
{
	// NOTE: This is a pointer to an actor that this actor tries to represent. One
	//       of the places we use this is when dragging items out of a pile. This
	//       actor gains similar properties to the one it represents, but with 0.5f
	//       alpha. It acts as a ghosted copy of the item its mimicking. 
	actorToMimic = obj;
}