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

#ifndef _MOCK_FILETRANSFEREVENTHANDLER_
#define _MOCK_FILETRANSFEREVENTHANDLER_
#ifdef BT_UTEST

#include "BT_TestCommon.h"
#include "BT_FileTransferManager.h"

using CppUnit::TestFixture;

/*
* Mock FileTransferEventHandler 
*/
class MockFileTransferEventHandler : public FileTransferEventHandler
{
public:
	// NOTE that success/unsuccess-ful downloads differ from
	// transfer completion
	typedef map<FileTransfer, bool, CompareFileTransfer> ExpectedTransfersContainer;
	ExpectedTransfersContainer _expectedTransfers;

public:
	~MockFileTransferEventHandler();

	// file transfer event handling
	virtual void onTransferComplete(const FileTransfer& transfer);
	virtual void onTransferError(const FileTransfer& transfer);

	// reset the mock instance
	void reset();

	// appen/query the mock instance with/for expected transfer information
	void expectTransfer(const FileTransfer& ft, bool expectedToSucceed);
	bool isExpectingTransfers() const;
};

#endif // BT_UTEST
#endif // _MOCK_FILETRANSFEREVENTHANDLER_