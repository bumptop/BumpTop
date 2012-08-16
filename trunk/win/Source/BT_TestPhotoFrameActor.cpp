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

#include "BT_TestPhotoFrameActor.h"

using namespace CppUnit;

// register the test suite with the anonymous registery
CPPUNIT_TEST_SUITE_REGISTRATION(PhotoFrameActorTest);

/*
* PhotoFrameActorTest implementation
*/
void PhotoFrameActorTest::setUp()
{}

void PhotoFrameActorTest::tearDown()
{}

void PhotoFrameActorTest::testConstructor()
{
	return;
	// XXX: problem: actors need the scnee to be initialized, which means greater
	// startup time for all?
	
	// ensure that the default photo frame ctor does not install a source
	// PhotoFrameActor pfActor;
	// CPPUNIT_ASSERT_THROW(pfActor.getSource(), runtime_error);

	/*
	// standard local directory
	const QString rootDirectory("C:\\");
	PhotoFrameActor pfLocalActor(rootDirectory);

	const QString invalidDirectory("C:\\_probably_an_invalid_directory_unless_you_are_crazy");
	PhotoFrameActor pfInvalidLocalActor(invalidDirectory); 

	const QString rssFeed("http://api.flickr.com/services/feeds/photos_public.gne?tags=portugal&lang=en-us&format=atom");
	PhotoFrameActor pfFeedActor(rssFeed);

	// XXX: empty in terms of valid images?
	const QString rssEmptyFeed("");
	PhotoFrameActor pfEmptyFeedActor(rssEmptyFeed);
	// assert: source items is none

	const QString flickrFeed("flickr://testtag");
	PhotoFrameActor pfFlickrFeedActor(flickrFeed);

	const QString flickrInvalidFeed("flickr://no_such_tag_exists_probably_unless_they_are_crazy");
	PhotoFrameActor pfFlickrInvalidFeedActor(flickrInvalidFeed);

	const QString invalidProtocol("asdf://what?");
	PhotoFrameActor pfInvalidActor(invalidProtocol);
	*/

	// creating a photo frame actor with a source should throw and exception if the 
	// source is invalid.
}

#endif // BT_UTEST