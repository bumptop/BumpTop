/*
 *  Copyright 2012 Google Inc. All Rights Reserved.
 *  
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  
 *      http://www.apache.org/licenses/LICENSE-2.0
 *  
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <gtest/gtest.h>

#include "BumpTop/FileSystemEventDispatcher.h"
#include "BumpTop/FileSystemEventTestHelper.h"
#include "BumpTop/QStringHelpers.h"

namespace {
  class FileSystemEventDispatcherTest : public FileSystemEventTest {
    Q_OBJECT
  };


  TEST_F(FileSystemEventDispatcherTest, Test_adding_and_deleting_paths) {
    // Shouldn't be able to add a file before its parent path is added
    createFileWithName("test1.txt");
    ASSERT_FALSE(FileSystemEventDispatcher::singleton()->addPathToWatch(test_dir_.filePath("test1.txt"), this, false));

    // Test what happens when you add a path that doesn't exist
    ASSERT_FALSE(FileSystemEventDispatcher::singleton()->addPathToWatch(test_dir_.filePath("test2.txt"), this, false));

    ASSERT_TRUE(FileSystemEventDispatcher::singleton()->addPathToWatch(test_dir_.absolutePath(), this, true));

    // Shouldn't be able to add same path twice
    ASSERT_FALSE(FileSystemEventDispatcher::singleton()->addPathToWatch(test_dir_.absolutePath(), this, true));
    ASSERT_FALSE(FileSystemEventDispatcher::singleton()->addPathToWatch(test_dir_.absolutePath() + "/", this, true));

    // And, now we should be able to add that original file
    ASSERT_TRUE(FileSystemEventDispatcher::singleton()->addPathToWatch(test_dir_.filePath("test1.txt"), this, false));

    // Test what happens when you add a path that doesn't exist
    ASSERT_FALSE(FileSystemEventDispatcher::singleton()->addPathToWatch(test_dir_.filePath("test2.txt"), this, false));
    createFileWithName("test2.txt");
    ASSERT_TRUE(FileSystemEventDispatcher::singleton()->addPathToWatch(test_dir_.filePath("test2.txt"), this, false));

    ASSERT_TRUE(FileSystemEventDispatcher::singleton()->removePathToWatch(test_dir_.filePath("test1.txt"), this));
    ASSERT_FALSE(FileSystemEventDispatcher::singleton()->removePathToWatch(test_dir_.filePath("test1.txt"), this));
    ASSERT_TRUE(FileSystemEventDispatcher::singleton()->removePathToWatch(test_dir_.filePath("test2.txt"), this));
    ASSERT_FALSE(FileSystemEventDispatcher::singleton()->removePathToWatch(test_dir_.filePath("test2.txt"), this));
    ASSERT_TRUE(FileSystemEventDispatcher::singleton()->removePathToWatch(test_dir_.absolutePath(), this));
    ASSERT_FALSE(FileSystemEventDispatcher::singleton()->removePathToWatch(test_dir_.absolutePath(), this));
  }

  TEST_F(FileSystemEventDispatcherTest, Test_adding_and_deleting_paths_in_directory_with_trailing_slash) {
    // Basically the previous test case, but add the directory path with a "/" at the end

    // Shouldn't be able to add a file before its parent path is added
    createFileWithName("test1.txt");
    ASSERT_FALSE(FileSystemEventDispatcher::singleton()->addPathToWatch(test_dir_.filePath("test1.txt"), this, false));

    // Test what happens when you add a path that doesn't exist
    ASSERT_FALSE(FileSystemEventDispatcher::singleton()->addPathToWatch(test_dir_.filePath("test2.txt"), this, false));

    ASSERT_TRUE(FileSystemEventDispatcher::singleton()->addPathToWatch(test_dir_.absolutePath() + "/", this, true));

    // Shouldn't be able to add same path twice
    ASSERT_FALSE(FileSystemEventDispatcher::singleton()->addPathToWatch(test_dir_.absolutePath() + "/", this, true));
    ASSERT_FALSE(FileSystemEventDispatcher::singleton()->addPathToWatch(test_dir_.absolutePath() + "//", this, true));
    ASSERT_FALSE(FileSystemEventDispatcher::singleton()->addPathToWatch(test_dir_.absolutePath(), this, true));

    // And, now we should be able to add that original file
    ASSERT_TRUE(FileSystemEventDispatcher::singleton()->addPathToWatch(test_dir_.filePath("test1.txt"), this, false));

    // Test what happens when you add a path that doesn't exist
    ASSERT_FALSE(FileSystemEventDispatcher::singleton()->addPathToWatch(test_dir_.filePath("test2.txt"), this, false));
    createFileWithName("test2.txt");
    ASSERT_TRUE(FileSystemEventDispatcher::singleton()->addPathToWatch(test_dir_.filePath("test2.txt"), this, false));

    ASSERT_TRUE(FileSystemEventDispatcher::singleton()->removePathToWatch(test_dir_.filePath("test1.txt"), this));
    ASSERT_FALSE(FileSystemEventDispatcher::singleton()->removePathToWatch(test_dir_.filePath("test1.txt"), this));
    ASSERT_TRUE(FileSystemEventDispatcher::singleton()->removePathToWatch(test_dir_.filePath("test2.txt"), this));
    ASSERT_FALSE(FileSystemEventDispatcher::singleton()->removePathToWatch(test_dir_.filePath("test2.txt"), this));
    ASSERT_TRUE(FileSystemEventDispatcher::singleton()->removePathToWatch(test_dir_.absolutePath(), this));
    ASSERT_FALSE(FileSystemEventDispatcher::singleton()->removePathToWatch(test_dir_.absolutePath(), this));
  }

  TEST_F(FileSystemEventDispatcherTest, Test_events_are_fired) {
    ASSERT_TRUE(FileSystemEventDispatcher::singleton()->addPathToWatch(test_dir_.absolutePath(), this, true));

    //
    // ADDING
    //
    createFileWithName("test1.txt");

    QString file_path = runEventLoopUntilReceiveEventOfType("FILE_ADDED");
    bool file_was_added = !file_path.isNull();
    EXPECT_TRUE(file_was_added);
    EXPECT_EQ(test_dir_.filePath("test1.txt"), file_path);
    EXPECT_EQ(1, numEventsOfType("FILE_ADDED"));

    ASSERT_TRUE(FileSystemEventDispatcher::singleton()->addPathToWatch(test_dir_.filePath("test1.txt"), this, false));

    renameFile("test1.txt", "test2.txt");

    //
    // RENAMING
    //
    file_path = runEventLoopUntilReceiveEventOfType("FILE_RENAMED");
    bool file_was_renamed = !file_path.isNull();
    EXPECT_TRUE(file_was_renamed);
    EXPECT_EQ(test_dir_.filePath("test1.txt"), file_path);

    // we expect 2 events; the containing directory gets one, as well as the file itself
    EXPECT_EQ(2, numEventsOfType("FILE_RENAMED"));

    //
    // MODIFYING
    //
    sleep(1);  // for some reason we don't get file modified if we don't sleep here...
    modifyFileWithName("test2.txt");

    file_path = runEventLoopUntilReceiveEventOfType("FILE_MODIFIED");
    bool file_was_modified = !file_path.isNull();
    EXPECT_TRUE(file_was_modified);
    EXPECT_EQ(test_dir_.filePath("test2.txt"), file_path);
    EXPECT_EQ(2, numEventsOfType("FILE_MODIFIED"));

    //
    // REMOVING
    //
    removeFile("test2.txt");

    file_path = runEventLoopUntilReceiveEventOfType("FILE_REMOVED");
    bool file_was_removed = !file_path.isNull();
    EXPECT_TRUE(file_was_removed);
    EXPECT_EQ(test_dir_.filePath("test2.txt"), file_path);
    EXPECT_EQ(2, numEventsOfType("FILE_REMOVED"));

    createFileWithName("test1.txt");
    runEventLoopUntilReceiveEventOfType("FILE_ADDED");
    ASSERT_TRUE(FileSystemEventDispatcher::singleton()->addPathToWatch(test_dir_.filePath("test1.txt"), this, false));

    renameFile("test1.txt", "test2.txt");
    file_path = runEventLoopUntilReceiveEventOfType("FILE_RENAMED");
    file_was_renamed = !file_path.isNull();
    EXPECT_TRUE(file_was_renamed);
    EXPECT_EQ(test_dir_.filePath("test1.txt"), file_path);

    // we expect 2 events; the containing directory gets one, as well as the file itself
    EXPECT_EQ(2, numEventsOfType("FILE_RENAMED"));

    ASSERT_FALSE(FileSystemEventDispatcher::singleton()->removePathToWatch(test_dir_.filePath("test1.txt"), this));
    ASSERT_TRUE(FileSystemEventDispatcher::singleton()->removePathToWatch(test_dir_.filePath("test2.txt"), this));
    ASSERT_TRUE(FileSystemEventDispatcher::singleton()->removePathToWatch(test_dir_.absolutePath(), this));
  }
}

#include "moc/moc_FileSystemEventDispatcherTest.cpp"

