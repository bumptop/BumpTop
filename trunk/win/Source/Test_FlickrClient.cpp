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
#include "FlickrClient.h"


class Test_FlickrClient : public testing::Test 
{
	protected:
		QString groupFeed, userFeed, favouriteFeed;
		QString userId, groupId, favouriteId, invalidId;
		FlickrClient flickrClient;
		vector<QString> urls;
		
		Test_FlickrClient() :
		flickrClient(),
		groupFeed(""),
		userFeed(""),
		favouriteFeed(""),
		userId("26799028@N06"), // BumptopFlickr photos
		groupId("806251@N24"),  // bumptop group
		favouriteId("26799028@N06"), // Bumptop flickr favourites
		invalidId("123@N00") // Random invalid id
		{}
};

// FlickrClient tests
/*

TEST_F(Test_FlickrClient, test_requestAFrob) 
{
	// Test the getApiSignature function
	EXPECT_NE(flickrClient.requestAFrob(), "");
}

TEST_F (Test_FlickrClient, test_getAPISignature) 
{
	QHash<QString, QString> params;
	params.append("flickr.auth.getFrob");
	params.append(FLICKR_API_KEY);
	EXPECT_EQ(FlickrClient.getAPISignature(&params), "53557cb1aa8769bc2b8170e6d79fab90");
}

// Test get photos from a tag
TEST_F (Test_FlickrClient, test_getPhotosByTag)
{
	EXPECT_TRUE(flickrClient.getPhotosByTag("bumptop", &urls));
	EXPECT_TRUE(flickrClient.getPhotosByTag("photoroot", &urls));
	EXPECT_TRUE(flickrClient.getPhotosByTag("chicken", &urls));
	EXPECT_TRUE(flickrClient.getPhotosByTag("simpsons", &urls));

	// This function will only fail if ExecuteAPI or GetHttpRequest fails
	EXPECT_TRUE(flickrClient.getPhotosByTag("aksjd;flk2lnvm", &urls));
}

TEST_F (Test_FlickrClient, test_getPhotosByInvalidTag)
{
	// getPhotosByTag will only fail in two cases
	// 1. We try to make a call using empty/null parameters
	// 2. executeAPI, or getHttpRequest fails
	EXPECT_FALSE(flickrClient.getPhotosByTag("", &urls));
	EXPECT_FALSE(flickrClient.getPhotosByTag(NULL, &urls));
}

// Test get photos from User Id
TEST_F (Test_FlickrClient, test_getPhotosFromUser)
{
	EXPECT_TRUE(flickrClient.getPhotosFromId(userId, &urls, false));
}

TEST_F (Test_FlickrClient, test_getPhotosFromInvalidUser)
{
	EXPECT_FALSE(flickrClient.getPhotosFromId(invalidId, &urls, false));
	EXPECT_FALSE(flickrClient.getPhotosFromId("", &urls, false));
	EXPECT_FALSE(flickrClient.getPhotosFromId(NULL, &urls, false));
}


// Test get photos from Group id
TEST_F (Test_FlickrClient, test_getPhotosFromGroup)
{
	EXPECT_TRUE(flickrClient.getPhotosFromId(groupId, &urls, true));
}

TEST_F (Test_FlickrClient, testGetPhotosFromInvalidGroup)
{
	EXPECT_FALSE(flickrClient.getPhotosFromId(invalidId, &urls, true));
	EXPECT_FALSE(flickrClient.getPhotosFromId("", &urls, true));
	EXPECT_FALSE(flickrClient.getPhotosFromId(NULL, &urls, true));
}

// Test get photos from invalid id
TEST_F (Test_FlickrClient, test_getFavouritesFromValidId)
{
	EXPECT_TRUE(flickrClient.getFavouritePhotosFromId(favouriteId, &urls));

	// Even sending in empty/null user ID's will return a list of the 
	// last 20 photos that were marked as favourites
	EXPECT_TRUE(flickrClient.getFavouritePhotosFromId("", &urls));
	EXPECT_TRUE(flickrClient.getFavouritePhotosFromId(NULL, &urls));
}

TEST_F (Test_FlickrClient, test_getFavouritesFromInvalidId)
{
	EXPECT_FALSE(flickrClient.getFavouritePhotosFromId(invalidId, &urls));
}

// Test get information about a user
TEST_F (Test_FlickrClient, test_getUserNameFromValidId)
{
	EXPECT_EQ(flickrClient.getUsername(userId).toStdString(), "bumptop");
}

TEST_F (Test_FlickrClient, test_getUserNameFromInvalidId)
{
	EXPECT_EQ(flickrClient.getUsername(invalidId).toStdString(), "");
	EXPECT_EQ(flickrClient.getUsername("").toStdString(), "");
	EXPECT_EQ(flickrClient.getUsername(NULL).toStdString(), "");
}

// Test get information about a group
TEST_F (Test_FlickrClient, test_getGroupNameFromValidId)
{
	EXPECT_EQ(flickrClient.getGroupName(groupId).toStdString(), "Peep my BumpTop");
}

TEST_F (Test_FlickrClient, test_getGroupNameFromInvalidId)
{
	EXPECT_EQ(flickrClient.getGroupName(invalidId).toStdString(), "");
	EXPECT_EQ(flickrClient.getGroupName("").toStdString(), "");
	EXPECT_EQ(flickrClient.getGroupName(NULL).toStdString(), "");
}
*/