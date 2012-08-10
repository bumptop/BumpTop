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

#include "BumpTop/PythonRunner.h"

#include <QtCore/QDir>
#include <QtCore/QTextStream>
#include <QtCore/QProcess>

bool runPythonScriptInDirectory(QString script, QString directory_path) {
  //
  // find a filename that's not taken
  //
  QString b = directory_path;
  QDir directory(directory_path);
  if (!directory.exists()) {
    return false;
  }

  QString script_filename;
  int i = 0;
  do {
    script_filename = QString("pyscript%1.py").arg(i);
    i++;
  } while (QFile(directory.filePath(script_filename)).exists());

  //
  // write the script to a file
  //
  QFile script_file(directory.filePath(script_filename));
  if (!script_file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    return false;
  }
  QTextStream script_file_out(&script_file);
  script_file_out << script;
  script_file.close();

  //
  // RUN the script
  //
  QProcess python;
  python.setWorkingDirectory(directory_path);
  python.start("python", QStringList() << directory.filePath(script_filename));

  if (!python.waitForFinished()) {
    directory.remove(script_filename);
    return false;
  }

  bool exited_successfully = python.exitStatus() == QProcess::NormalExit && python.exitCode() == 0;
  if (!exited_successfully) {
    directory.remove(script_filename);
    return false;
  }

  directory.remove(script_filename);
  return true;
}
