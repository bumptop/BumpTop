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
#include "BT_MD5Hasher.h"

MD5Hasher::MD5Hasher(  )
{
	md5_init(&_state);
}

std::string MD5Hasher::hexDigest()
{
	md5_byte_t digest[16];
	md5_finish(&_state, digest);

	char hex_output[16*2 + 1];
	for (int di = 0; di < 16; ++di)
		sprintf_s(hex_output + di * 2,(size_t)3, "%02x", digest[di]);

	_hexDigest = string(hex_output);

	return _hexDigest;
}

void MD5Hasher::updateWithFile( string filename )
{
	char buffer[100];
	ifstream myFile (filename.c_str(), ios::in | ios::binary);
	int numBytes = 0;
	myFile.read (buffer, 100);
	while (! myFile.eof() && !myFile.fail())
	{
		numBytes+= myFile.gcount();
		update(buffer, myFile.gcount());
		myFile.read (buffer, 100);
	}
	numBytes+= myFile.gcount();
	update(buffer, myFile.gcount());
	myFile.close();
}

void MD5Hasher::update( char *byte_array, int len )
{
	md5_append(&_state, (const md5_byte_t *)byte_array, len);
}