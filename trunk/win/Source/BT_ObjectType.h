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

#ifndef _BT_OBJECT_TYPE_
#define _BT_OBJECT_TYPE_

// -----------------------------------------------------------------------------

class ObjectType
{
public:

	unsigned int primaryType;
	unsigned int secondaryType;
	unsigned int ternaryType;

	// Constructors
	ObjectType(const ObjectType &in);
	ObjectType(uint prim = 0, uint secnd = 0, uint tern = 0);

	unsigned int asUInt() const;
	static ObjectType fromUInt(const unsigned int& fullType);
	static ObjectType fromUIntReversed(const unsigned int& fullType);

	// Operator Overloads
	bool operator==(const ObjectType &in);
	bool operator!=(const ObjectType &in);
};

// -----------------------------------------------------------------------------

#else
	class ObjectType;
#endif