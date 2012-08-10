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

#include "BumpTop/BumpTopApp.h"
#include "BumpTop/FileSystemEventTestHelper.h"
#include "BumpTop/FileSystemWatcher.h"
#include "BumpTop/FSEventsWatcher.h"
#include "BumpTop/QStringHelpers.h"
#include "BumpTop/Stopwatch.h"

namespace {
  class FileSystemWatcherTest : public FileSystemEventTest {
    Q_OBJECT

  protected:
    FileSystemWatcher *watcher_;

    bool received_fsevent_callback_;

    void resetHandlerFlags() {
      FileSystemEventTest::resetHandlerFlags();
      received_fsevent_callback_ = false;
    }

    virtual void SetUp() {
      FileSystemEventTest::SetUp();

      watcher_ = new FileSystemWatcher();
      watcher_->init(test_dir_.absolutePath());


      ASSERT_TRUE(QObject::connect(watcher_, SIGNAL(onFileAdded(const QString&)),
                                   this, SLOT(fileAdded(const QString&))));

      ASSERT_TRUE(QObject::connect(watcher_, SIGNAL(onFileRenamed(const QString&, const QString&)),
                                   this, SLOT(fileRenamed(const QString&, const QString&))));

      ASSERT_TRUE(QObject::connect(watcher_, SIGNAL(onFileRemoved(const QString&)),
                                   this, SLOT(fileRemoved(const QString&))));

      ASSERT_TRUE(QObject::connect(watcher_, SIGNAL(onFileModified(const QString&)),
                                   this, SLOT(fileModified(const QString&))));
    }

    virtual void TearDown() {
      FileSystemEventTest::TearDown();

      delete watcher_;
    }

    public slots:

    virtual void fsEventsDirectoryChangedHandler(QString path) {
      received_fsevent_callback_ = true;
    }
  };

  TEST_F(FileSystemWatcherTest, Test_file_added_removed_and_renamed) {
    // running this multiple times to check that there aren't any non-deterministic behaviors; this
    // has been tested to work reliably all the time even if i = 10,000 or greater
    for (int i = 0; i < 1; i++) {
      createFileWithName("test.txt");
      QString file_path = runEventLoopUntilReceiveEventOfType("FILE_ADDED");
      bool file_was_added = !file_path.isNull();
      EXPECT_TRUE(file_was_added);
      EXPECT_EQ(test_dir_.filePath("test.txt"), file_path);
      EXPECT_EQ(1, numEventsOfType("FILE_ADDED"));


      removeFile("test.txt");
      file_path = runEventLoopUntilReceiveEventOfType("FILE_REMOVED");
      bool file_was_removed = !file_path.isNull();
      EXPECT_TRUE(file_was_removed);
      EXPECT_EQ(test_dir_.filePath("test.txt"), file_path);
      EXPECT_EQ(1, numEventsOfType("FILE_REMOVED"));


      createFileWithName("test.txt");
      file_path = runEventLoopUntilReceiveEventOfType("FILE_ADDED");
      file_was_added = !file_path.isNull();


      renameFile("test.txt", "test2.txt");
      file_path = runEventLoopUntilReceiveEventOfType("FILE_RENAMED");
      bool file_was_renamed = !file_path.isNull();

      EXPECT_TRUE(file_was_renamed);
      EXPECT_EQ(test_dir_.filePath("test.txt"), file_path);


      removeFile("test2.txt");
      file_path = runEventLoopUntilReceiveEventOfType("FILE_REMOVED");
      file_was_removed = !file_path.isNull();
      EXPECT_TRUE(file_was_removed);
      EXPECT_EQ(test_dir_.filePath("test2.txt"), file_path);
    }
  }

  TEST_F(FileSystemWatcherTest, Test_more_files_added_removed_and_renamed) {
    // running this multiple times to check that there aren't any non-deterministic behaviors; this
    // has been tested to work reliably all the time even if i = 10,000 or greater
    for (int i = 0; i < 1; i++) {
      createFileWithName("test.txt");
      QString file_path = runEventLoopUntilReceiveEventOfType("FILE_ADDED");
      bool file_was_added = !file_path.isNull();
      EXPECT_TRUE(file_was_added);
      EXPECT_EQ(test_dir_.filePath("test.txt"), file_path);
      EXPECT_EQ(1, numEventsOfType("FILE_ADDED"));

      createFileWithName("a.txt");
      file_path = runEventLoopUntilReceiveEventOfType("FILE_ADDED");
      file_was_added = !file_path.isNull();
      EXPECT_TRUE(file_was_added);
      EXPECT_EQ(test_dir_.filePath("a.txt"), file_path);
      EXPECT_EQ(1, numEventsOfType("FILE_ADDED"));

      createFileWithName("z.txt");
      file_path = runEventLoopUntilReceiveEventOfType("FILE_ADDED");
      file_was_added = !file_path.isNull();
      EXPECT_TRUE(file_was_added);
      EXPECT_EQ(test_dir_.filePath("z.txt"), file_path);
      EXPECT_EQ(1, numEventsOfType("FILE_ADDED"));


      removeFile("test.txt");
      file_path = runEventLoopUntilReceiveEventOfType("FILE_REMOVED");
      bool file_was_removed = !file_path.isNull();
      EXPECT_TRUE(file_was_removed);
      EXPECT_EQ(test_dir_.filePath("test.txt"), file_path);
      EXPECT_EQ(1, numEventsOfType("FILE_REMOVED"));


      createFileWithName("test.txt");
      file_path = runEventLoopUntilReceiveEventOfType("FILE_ADDED");
      file_was_added = !file_path.isNull();

      createFileWithName("x.txt");
      file_path = runEventLoopUntilReceiveEventOfType("FILE_ADDED");
      file_was_added = !file_path.isNull();
      EXPECT_TRUE(file_was_added);
      EXPECT_EQ(test_dir_.filePath("x.txt"), file_path);


      renameFile("test.txt", "test2.txt");
      file_path = runEventLoopUntilReceiveEventOfType("FILE_RENAMED");
      bool file_was_renamed = !file_path.isNull();

      EXPECT_TRUE(file_was_renamed);
      EXPECT_EQ(test_dir_.filePath("test.txt"), file_path);


      removeFile("test2.txt");
      file_path = runEventLoopUntilReceiveEventOfType("FILE_REMOVED");
      file_was_removed = !file_path.isNull();
      EXPECT_TRUE(file_was_removed);
      EXPECT_EQ(test_dir_.filePath("test2.txt"), file_path);

      removeFile("a.txt");
      removeFile("z.txt");
      removeFile("x.txt");
    }
  }

  TEST_F(FileSystemWatcherTest, Test_file_modified_SLOW_TEST) {
    // running this multiple times to check that there aren't any non-deterministic behaviors; this
    // has been tested to work reliably all the time even if i = 10,000 or greater
    for (int i = 0; i < 1; i++) {
      createFileWithName("test.txt");
      QString file_path = runEventLoopUntilReceiveEventOfType("FILE_ADDED");
      bool file_was_added = !file_path.isNull();
      EXPECT_TRUE(file_was_added);
      EXPECT_EQ(test_dir_.filePath("test.txt"), file_path);

      sleep(1);  // for some reason we don't get file modified if we don't sleep here...
      modifyFileWithName("test.txt");
      file_path = runEventLoopUntilReceiveEventOfType("FILE_MODIFIED");
      bool file_was_modified = !file_path.isNull();
      EXPECT_TRUE(file_was_modified);
      EXPECT_EQ(test_dir_.filePath("test.txt"), file_path);

      removeFile("test.txt");
      file_path = runEventLoopUntilReceiveEventOfType("FILE_REMOVED");
      bool file_was_removed = !file_path.isNull();
      EXPECT_TRUE(file_was_removed);
      EXPECT_EQ(test_dir_.filePath("test.txt"), file_path);
    }
  }

  TEST_F(FileSystemWatcherTest, Test_fsevents_works) {
    FSEventsWatcher *fsevents_watcher = new FSEventsWatcher(test_dir_.absolutePath());
    ASSERT_TRUE(QObject::connect(fsevents_watcher, SIGNAL(directoryChanged(QString)),
                                 this, SLOT(fsEventsDirectoryChangedHandler(QString))));

    createFileWithName("test.txt");

    Stopwatch s;
    while (s.elapsed() < 1500 && !received_fsevent_callback_)
      bumptop_->processOneEvent(0);

    EXPECT_TRUE(received_fsevent_callback_);
  }
}

#include "moc/moc_FileSystemWatcherTest.cpp"
