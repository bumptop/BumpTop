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

ObjectType BumpObject::getObjectType()
{
	return type;
}

bool BumpObject::isBumpObjectType(BumpObjectType inType)
{
	// Return the Primary Mask of the Object Type (See struct ObjectType)
	return (type.primaryType == inType);
}

bool BumpObject::isObjectType(ObjectType type)
{
	return (this->type == type);
}

bool BumpObject::isParentType(ObjectType type)
{
	return (parent && parent->isObjectType(type));
}

BumpObject *BumpObject::getParent() const
{
	return parent;
}

void BumpObject::setParent(BumpObject *p)
{
	parent = p;
}

void BumpObject::setObjectType(ObjectType t)
{
	type = t;
}

void BumpObject::setBumpObjectType(BumpObjectType t)
{
	type.primaryType = t;
}

void BumpObject::fadeOut(uint numTicks)
{
	setAlphaAnim(getAlpha(), 0.0f, numTicks);
}

void BumpObject::fadeIn(uint numTicks)
{
	setAlphaAnim(getAlpha(), 1.0f, numTicks);
}

bool BumpObject::isPinned()
{
	return pinned;
}

bool BumpObject::isAnimating(uint animType)
{
	// NOTE: since this is called often, we are going to return early

	// Check the different types of anims
	if (animType & AlphaAnim && alphaAnim.size() > 0) return true;
	if (animType & PoseAnim && poseAnim.size() > 0) return true;
	if (animType & SizeAnim && sizeAnim.size() > 0) return true;
	if (animType & FreshnessAnim && freshnessAlphaAnim.size() > 0) return true;
	return false;
}

bool BumpObject::isSelected()
{
	return selected;
}

bool BumpObject::getEnforcePinning()
{
	return enforcePinning;
}

Vec3 BumpObject::getPinPointInSpace()
{
	return pinPointInSpace;
}

Vec3 BumpObject::getPinPointOnActor()
{
	return pinPointOnActor;
}

NxJoint * BumpObject::getPinJoint() const
{
	return pinJoint;
}

void BumpObject::setPinPointInSpace(Vec3 newPinInGlobalSpace)
{
	pinPointInSpace = newPinInGlobalSpace;
}

void BumpObject::setPinPointOnActor(Vec3 newPinInActorSpace)
{
	pinPointOnActor = newPinInActorSpace;
}

void BumpObject::setPinJoint(NxJoint * joint)
{
	pinJoint = joint;
}

float BumpObject::getFreshnessAlpha()
{
	if (!freshnessAlphaAnim.empty())
	{
		if (getAnimationState() == NoAnim)
			freshnessAlphaAnim.clear();
		else
			return freshnessAlphaAnim.front();
	}
	return 0.0f;
}

// Set the dims of the object based to its default dims
void BumpObject::setDimsToDefault()
{
	setDims(getDefaultDims());
}
