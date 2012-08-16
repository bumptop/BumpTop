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
#include "BT_ObjectType.h"

ObjectType::ObjectType(const ObjectType &in)
{
	primaryType = in.primaryType;
	secondaryType = in.secondaryType;
	ternaryType = in.ternaryType;
}

ObjectType::ObjectType(uint prim, uint secnd, uint tern)
{
	primaryType = prim;
	secondaryType = secnd;
	ternaryType = tern;
}

unsigned int ObjectType::asUInt() const
{
	unsigned int v = primaryType << 28 | secondaryType << 16 | ternaryType;
	return v;
}

ObjectType ObjectType::fromUInt(const unsigned int& fullType)
{
	unsigned int primary = (fullType & 0xF0000000) >> 28;
	unsigned int secondary = (fullType & 0xFFF0000) >> 16;
	unsigned int tertiary = (fullType & 0xFFFF);
	return ObjectType(primary, secondary, tertiary);
}

ObjectType ObjectType::fromUIntReversed(const unsigned int& fullType)
{
	unsigned int primary = (fullType & 0xF);
	unsigned int secondary = (fullType & 0xFFF0) >> 4;
	unsigned int tertiary = (fullType & 0xFFFF0000) >> 16;
	return ObjectType(primary, secondary, tertiary);
}

bool ObjectType::operator==(const ObjectType &in)
{
	bool rc = false;

	// Check primary
	if (primaryType == in.primaryType)
	{
		rc = true;

		// TODO:
		// NOTE: this is WRONG, we aren't checking if all the bits match,
		//		 only that some of the bits match

		// Check secondary types
		if (in.secondaryType > NULL)
		{
			if (!(secondaryType & in.secondaryType))
			{
				rc = false;
			}
		}

		// Check ternary Types
		if (in.ternaryType > NULL)
		{
			if (!(ternaryType & in.ternaryType))
			{
				rc = false;
			}
		}

	}

	return rc;
}

bool ObjectType::operator!=(const ObjectType &in)
{
	return !(*this == in);
}
