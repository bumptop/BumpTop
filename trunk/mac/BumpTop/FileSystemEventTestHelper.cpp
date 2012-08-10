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

#include "BumpTop/FileSystemEventTestHelper.h"

#include "BumpTop/BumpTopApp.h"
#include "BumpTop/PythonRunner.h"
#include "BumpTop/Stopwatch.h"
#include "BumpTop/TestHelpers.h"

void FileSystemEventTest::resetHandlerFlags() {
  events_.clear();
}

void FileSystemEventTest::clearTestDirectory() {
  test_dir_ = getTestDataDirectory();
  test_dir_.setPath(test_dir_.filePath("FileSystemWatcherTest"));

  if (test_dir_.exists()) {
    for_each(QFileInfo file_info, test_dir_.entryInfoList()) {
      test_dir_.remove(file_info.fileName());
    }
  }

  getTestDataDirectory().rmdir(test_dir_.dirName());
  ASSERT_TRUE(!test_dir_.exists());
}


void FileSystemEventTest::SetUp() {
  bumptop_ = BumpTopApp::singleton();

  resetHandlerFlags();
  clearTestDirectory();
  getTestDataDirectory().mkdir(test_dir_.dirName());

  ASSERT_TRUE(test_dir_.exists());
}

void FileSystemEventTest::TearDown() {
  resetHandlerFlags();
  clearTestDirectory();
}

void FileSystemEventTest::createFileWithName(QString filename) {
  QFile test_file(test_dir_.filePath(filename));

  if (!test_file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    return;
  }

  QTextStream test_file_out(&test_file);
  test_file_out << "Test data\n";
  test_file.close();
}

void FileSystemEventTest::modifyFileWithName(QString filename) {
  // for some reason this has to be done by an external application
  QStringList script;
  script << "import sys"
  << QString("test_file = file('%1', 'w')").arg(filename)
  << "test_file.write('ftw!')"
  << "test_file.close()"
  << QString("test_file = file('%1', 'r')").arg(filename)
  << "if test_file.read() != 'ftw!':"
  << "    sys.exit(1)";

  ASSERT_TRUE(runPythonScriptInDirectory(script.join("\n"), test_dir_.absolutePath()));
}

void FileSystemEventTest::renameFile(QString filename, QString new_filename) {
  EXPECT_TRUE(test_dir_.rename(filename, new_filename));
}

void FileSystemEventTest::removeFile(QString filename) {
  EXPECT_TRUE(test_dir_.remove(filename));
}

void FileSystemEventTest::fileAdded(const QString& path) {
  events_.append("FILE_ADDED:" + path);
}

void FileSystemEventTest::fileRenamed(const QString& old_path, const QString& new_path) {
  events_.append("FILE_RENAMED:" + old_path);
}

void FileSystemEventTest::fileRemoved(const QString& path) {
  events_.append("FILE_REMOVED:" + path);
}

void FileSystemEventTest::fileModified(const QString& path) {
  events_.append("FILE_MODIFIED:" + path);
}

QString FileSystemEventTest::runEventLoopUntilReceiveEventOfType(QString event_type) {
  events_.clear();
  Stopwatch stopwatch;
  while (stopwatch.elapsed() < 1500) {
    bumptop_->renderTick();  // required for qfilesystemwatcher notifications to register
    bumptop_->processOneEvent(0);  // required for fsevents notifications to register
    for_each(QString event, events_) {
      if (event.startsWith(event_type)) {
        return event.split(QChar(':'))[1];
      }
    }
  }
  return QString();
}

int FileSystemEventTest::numEventsOfType(QString event_type) {
  int i = 0;
  for_each(QString event, events_) {
    if (event.startsWith(event_type)) {
      i++;
    }
  }
  return i;
}

#include "moc/moc_FileSystemEventTestHelper.cpp"
