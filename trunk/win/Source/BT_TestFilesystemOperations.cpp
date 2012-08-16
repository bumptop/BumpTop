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
#ifdef BT_UTEST

#include "BT_TestFilesystemOperations.h"

using namespace CppUnit;

// register the test suite with the anonymous registry
CPPUNIT_TEST_SUITE_REGISTRATION(FilesystemOperationsTest);

#endif


bool my_exists(string path)
{
	WIN32_FILE_ATTRIBUTE_DATA fileAttributes = { 0 };

	// Get the file attributes of this file
	GetFileAttributesEx(path.c_str(), GetFileExInfoStandard, &fileAttributes);

	return fileAttributes.dwFileAttributes != 0;
}

string my_branch_path(string path)
{
	size_t pos = path.rfind('\\');
	if (pos != string::npos)
		return path.substr(0,pos);
	else
		return "";

}

string my_leaf(string path)
{
	size_t pos = path.rfind('\\');
	if (pos != string::npos && path.size() > 1 && pos < path.size()-2)
		return path.substr(pos+1);
	else if (path == "\\" || path == "/" || (path.size() == 3 && path[1] == ':' && path[2] == '\\')) // e.g. C:\ E:\ 
		return "/";
	else if (path.size() >= 2 && path[path.size()-1] == ':')
		return path;
	else if (path == "")
		return "";
	else
		return ".";

}

void FilesystemOperationsTest::test_exists()
{
	// BOOST
	// absolute paths
	CPPUNIT_ASSERT(exists(current_path() / path("BT_TestFilesystemOperations.cpp", native)));
	CPPUNIT_ASSERT(!exists(current_path() / path("this_file_does_not_exist.cpp", native)));

	// relative paths
	CPPUNIT_ASSERT(exists(path("BT_TestFilesystemOperations.cpp", native)));
	CPPUNIT_ASSERT(!exists(path("this_file_does_not_exist", native)));

	// our implementation
	// absolute paths
	CPPUNIT_ASSERT(my_exists(current_path().native_directory_string() + "\\" + "BT_TestFilesystemOperations.cpp"));
	CPPUNIT_ASSERT(!my_exists(current_path().native_directory_string() + "\\" + "this_file_does_not_exist.cpp"));

	// relative paths
	CPPUNIT_ASSERT(my_exists("BT_TestFilesystemOperations.cpp"));
	CPPUNIT_ASSERT(!my_exists("this_file_does_not_exist.cpp"));


}

void FilesystemOperationsTest::test_branch_path()
{
	const uint num_test_paths = 8;
	string test_paths[num_test_paths] = {
		"C:\\Users\\Folder", 
		"C:\\Users\\Folder\\",
		"C:\\Users\\Folder\\file.txt",
		"\\",
		"C:",
		"C:\\",
		"relative\\path",
		""
	};

	string test_paths_branch_paths[num_test_paths] = {
		"C:\\Users", 
		"C:\\Users\\Folder",
		"C:\\Users\\Folder",
		"",
		"",
		"C:",
		"relative",
		""
	};

	for (int i = 0; i < num_test_paths; i++)
	{
		string test_path = test_paths[i];
		string test_path_branch_path = test_paths_branch_paths[i];

		path test_boost_path = path(test_path, native);

		// BOOST
		CPPUNIT_ASSERT_EQUAL_MESSAGE("\"" + test_path + "\".branch_path()", test_path_branch_path, test_boost_path.branch_path().native_directory_string());
		// our implementation
		CPPUNIT_ASSERT_EQUAL_MESSAGE("my_branch_path(\"" + test_path + "\")", test_path_branch_path, my_branch_path(test_path));
	}
}

void FilesystemOperationsTest::test_leaf()
{
	const uint num_test_paths = 11;
	string test_paths[num_test_paths] = {
		"E:\\Users\\Folder", 
		"E:\\Users\\Folder\\",
		"E:\\Users\\Folder\\file.txt",
		"\\",
		"E:",
		"E:\\",
		"relative\\path",
		"",
		"/",
		"oogly:",
		"oogly:\\"
	};

	string test_paths_leafs[num_test_paths] = {
		"Folder", 
		".",
		"file.txt",
		"/",
		"E:",
		"/",
		"path",
		"",
		"/",
		"oogly:",
		"."
	};

	for (int i = 0; i < num_test_paths; i++)
	{
		string test_path = test_paths[i];
		string test_path_leaf = test_paths_leafs[i];

		path test_boost_path = path(test_path, native);

		// BOOST
		CPPUNIT_ASSERT_EQUAL_MESSAGE("\"" + test_path + "\".leaf()", test_path_leaf, test_boost_path.leaf());
		// our implementation
		CPPUNIT_ASSERT_EQUAL_MESSAGE("my_leaf(\"" + test_path + "\")", test_path_leaf, my_leaf(test_path));
	}

}