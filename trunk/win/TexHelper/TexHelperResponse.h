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

// 1. When TexHelper starts up, it will write a response containing only Ready
// 2. When BumpTop needs to use TexHelper, it reads the ready message, and writes the command line
// 3. TexHelper reads the command line, does operation, writes a response containing Finish and other info
// 4. BumpTop will read the response containing Finish and use any followed image data or load image from hard drive
// 4. Then TexHelper writes another response containing only Ready, and repeat from 2.
// 5. If TexHelper gets an unrecoverable exception, it will write a response containing Restart in 3., 
//		then TexHelperManager needs to restart TexHelper.

struct TexHelperResponse // Must be the same in BumpTop and TexHelper projects
{
	bool Ready; // Whether the TexHelper is ready for commands from parent process, must be sent after a response containing Finish
	bool Finish; // Whether TexHelper has finished last command, check Failure for result
	bool Fail; // If used with Finish, indicates last command failed
	bool Restart; // Whether teh current process should be restarted due to unrecoverable errors
	bool ImageData; // Whether result contains pixel data.

	unsigned int ImageWidth;
	unsigned int ImageHeight;
	unsigned int ImageBytePerPixel;
	unsigned int ImageFormat; // DevIL format 

	unsigned int CheckSum;

	TexHelperResponse(bool ready, bool finish, bool fail)
	{
		Ready = ready;
		Finish = finish;
		Fail = fail;
		Restart = false;
		ImageData = false;
		ImageWidth = ImageHeight = ImageBytePerPixel = ImageFormat = 0;

		CheckSum = calculateCheckSum();
	}

	TexHelperResponse()
	{
		Ready = Finish = Fail = Restart = ImageData = false;
		ImageWidth = ImageHeight = ImageBytePerPixel = ImageFormat = 0;

		CheckSum = calculateCheckSum();
	}

	unsigned int calculateCheckSum()
	{
		unsigned int cs = Ready << 31;
		cs |= Finish << 30;
		cs |= Fail << 29;
		cs |= Restart << 28;
		cs |= ImageData << 27;

		cs ^= ImageWidth;
		cs ^= ImageHeight;
		cs ^= ImageBytePerPixel;
		cs ^= ImageFormat;

		return cs;
	}
};