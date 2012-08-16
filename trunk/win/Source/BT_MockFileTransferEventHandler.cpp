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
#ifdef BT_UTEST
#include "BT_MockFileTransferEventHandler.h"

using namespace CppUnit;

/*
 * MockFileTransferEventHandler implementation
 */
MockFileTransferEventHandler::~MockFileTransferEventHandler()
{
	CPPUNIT_ASSERT(!isExpectingTransfers());
}

void MockFileTransferEventHandler::reset()
{
	_expectedTransfers.clear();
}

void MockFileTransferEventHandler::onTransferComplete( const FileTransfer& transfer )
{
	ExpectedTransfersContainer::iterator iter = _expectedTransfers.find(transfer);
	CPPUNIT_ASSERT(iter != _expectedTransfers.end());		// ensure that this transfer result was expected
	CPPUNIT_ASSERT(iter->second);							// ensure that this transfer result was expected to be successful
	_expectedTransfers.erase(iter);
}

void MockFileTransferEventHandler::onTransferError(const FileTransfer& transfer)
{
	ExpectedTransfersContainer::iterator iter = _expectedTransfers.find(transfer);
	CPPUNIT_ASSERT(iter != _expectedTransfers.end());		// ensure that this transfer result was expected
	CPPUNIT_ASSERT(!iter->second);							// ensure that this transfer result was expected to be successful
	_expectedTransfers.erase(iter);
}

void MockFileTransferEventHandler::expectTransfer( const FileTransfer& ft, bool expectedToSucceed )
{
	_expectedTransfers.insert(make_pair(ft, expectedToSucceed));
}

bool MockFileTransferEventHandler::isExpectingTransfers() const
{
	return !_expectedTransfers.empty();
}

#endif // BT_UTEST