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

#include "BumpTop/UsageTracker.h"

#include <utility>

#import <Sparkle/Sparkle.h>

#include "BumpTop/AppSettings.h"
#include "BumpTop/BumpTopApp.h"
#include "BumpTop/QStringHelpers.h"

#include "ThirdParty/GetPrimaryMacAddress.h"

UsageTracker::UsageTracker() {
}

UsageTracker::~UsageTracker() {
}

const int kNumMillisecondsToWaitForFirstAttempt = 1000;   // 1 sec = 1000ms
const int kNumMillisecondsToWaitBetweenAttempts = 60000;  // 1 min = 60 secs = 60 * 1000 ms

void UsageTracker::init() {
  QDateTime today = QDateTime::currentDateTime();
  today.setTime(QTime(0, 0));
  assert(QObject::connect(&next_attempt_timer_, SIGNAL(onTick(Timer*)),  // NOLINT
                          this, SLOT(attemptUploadToBumpTop(Timer*))));  // NOLINT
  next_attempt_timer_.start(kNumMillisecondsToWaitForFirstAttempt);
  assert(QObject::connect(&network_access_manager_, SIGNAL(finished(QNetworkReply*)),  // NOLINT
                          this, SLOT(replyFinished(QNetworkReply*))));  // NOLINT
}

void UsageTracker::attemptUploadToBumpTop(Timer* timer) {
  today_ = QDateTime::currentDateTime();
  today_.setTime(QTime(0, 0));
  today_string_ = today_.toString("yyyy-MM-dd%20hh:mm:ss");
  if ([[SUUpdater sharedUpdater] sendsSystemProfile]) {
    if (AppSettings::singleton()->last_usage_tracking_upload_date() != utf8(today_string_) ||
        AppSettings::singleton()->last_usage_tracking_version_number() != utf8(BumpTopApp::singleton()->bumptopVersion())) {  // NOLINT
      QNetworkRequest request;
      QString url = "http://" + BumpTopApp::singleton()->bumptopVersion() + ".macstats.bumptop.com/usage_tracker.php?";
      std::pair<bool, QString> bool_and_mac_address = GetPrimaryMacAddressString();
      if (bool_and_mac_address.first) {
        QCryptographicHash sha1_hasher(QCryptographicHash::Sha1);
        sha1_hasher.addData(bool_and_mac_address.second.toUtf8());

        url += QString("id=") + sha1_hasher.result().toHex();
        url += QString("&bumptop_version=") + BumpTopApp::singleton()->bumptopVersion();
        url += "&timestamp=" + today_string_;
        request.setUrl(url);

        request.setRawHeader("User-Agent", "BumpTop");

        network_access_manager_.get(request);
      }
    }
  }
}

void UsageTracker::replyFinished(QNetworkReply* reply) {
  if (reply->error() == QNetworkReply::NoError && reply->readAll() == "ok") {
    AppSettings::singleton()->set_last_usage_tracking_upload_date(utf8(today_string_));
    AppSettings::singleton()->set_last_usage_tracking_version_number(utf8(BumpTopApp::singleton()->bumptopVersion()));
    AppSettings::singleton()->saveSettingsFile();
    QDateTime tomorrow = today_.addDays(1);
    int num_secs_till_tomorrow = QDateTime::currentDateTime().secsTo(tomorrow);
    next_attempt_timer_.start(std::max(kNumMillisecondsToWaitBetweenAttempts, (num_secs_till_tomorrow + 1)*1000));
  } else {
    next_attempt_timer_.start(kNumMillisecondsToWaitBetweenAttempts);
  }
  reply->close();
}

#include "moc/moc_UsageTracker.cpp"
