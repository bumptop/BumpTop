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

#ifndef BUMPTOP_USAGETRACKER_H_
#define BUMPTOP_USAGETRACKER_H_

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

#include "BumpTop/Timer.h"

class UsageTracker : public QObject {
  Q_OBJECT

 public:
  UsageTracker();
  ~UsageTracker();

  void init();
 public slots:  // NOLINT
  void attemptUploadToBumpTop(Timer* timer);
  void replyFinished(QNetworkReply* reply);
 protected:
  Timer next_attempt_timer_;
  QNetworkAccessManager network_access_manager_;
  bool success_;
  QDateTime today_;
  QString today_string_;
};

#endif  // BUMPTOP_USAGETRACKER_H_
