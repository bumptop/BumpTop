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

#ifndef BUMPTOP_AUTHORIZATION_H_
#define BUMPTOP_AUTHORIZATION_H_

#include <string>
#include <utility>

#include "BumpTop/protoc/AllMessages.pb.h"
#include "BumpTop/Singleton.h"

typedef std::pair<bool, QString> BoolAndQString;
class Authorization : public QObject {
  Q_OBJECT

 public:
  Authorization();

  void init();
  BoolAndQString registerOverInternet(QString invite_code);
  bool saveLicenseFile();
  bool loadLicenseFile();
  bool deleteLicenseFile();
  BoolAndQString decipherAndLoadConfigFile();
  bool authorized();
  void set_authorized(bool value);
  QString key();
 public slots:  // NOLINT
  void requestFinished(int id, bool error);
 protected:
  virtual bool parseFromString(std::string data) = 0;
  virtual QString config_file_path() = 0;
  virtual QString license_file_path() = 0;
  virtual std::pair<QString, QString> auth_script_web_host_and_path() = 0;
  virtual QString account_type() = 0;
  bool authorized_;
  QHttp *http_;
  bool download_finished_;
  BoolAndQString download_return_value_;
  int http_request_id_;
  License *license_;
};

class ProAuthorization : public Authorization, public ProConfig {
  SINGLETON_HEADER(ProAuthorization)
 public:
  ProAuthorization();
 protected:
  virtual bool parseFromString(std::string data);
  virtual QString config_file_path();
  virtual QString license_file_path();
  virtual std::pair<QString, QString> auth_script_web_host_and_path();
  virtual QString account_type();
};

#endif  // BUMPTOP_AUTHORIZATION_H_
