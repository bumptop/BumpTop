// Copyright 2011 Google Inc. All Rights Reserved.
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
#ifndef _BT_FILEMD5HASHER_
#define _BT_FILEMD5HASHER_

class MD5Hasher
{
	std::string _filename;
	std::string _hexDigest;
	md5_state_t _state;
public:
	MD5Hasher();
	std::string hexDigest();
	void update(char *byte_array, int len);
	void updateWithFile(std::string filename);
};

#endif // _BT_FILEMD5HASHER_
