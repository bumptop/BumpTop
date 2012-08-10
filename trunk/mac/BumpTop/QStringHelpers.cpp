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

#include "BumpTop/QStringHelpers.h"

#include <string>

std::ostream& operator<<(std::ostream& s, const QString& str) {
  s << utf8(str.toUtf8().data());
  return s;
}

std::string utf8(const QString& str) {
  return std::string(str.toUtf8().data());
}

QString QStringFromUtf8(std::string str) {
  return QString::fromUtf8(str.c_str());
}

QString QStringFromUtf8(char * str) {
  return QString::fromUtf8(str);
}

#if defined(OS_MACOSX)
QString QStringFromNSString(NSString *str) {
  return QStringFromUtf8([str cStringUsingEncoding:NSUTF8StringEncoding]);
}

const char* utf8(NSString *str) {
  return [str cStringUsingEncoding:NSUTF8StringEncoding];  // NOLINT
}

NSString* NSStringFromQString(const QString& str) {
  return [NSString stringWithUTF8String:utf8(str).c_str()];  // NOLINT
}

NSString* NSStringFromUtf8(std::string str) {
  return NSStringFromQString(QStringFromUtf8(str));
}

CFStringRef CFStringFromUtf8(std::string str) {
  return CFStringCreateWithCString(NULL, str.c_str(), kCFStringEncodingUTF8);
};

CFStringRef CFStringFromQString(QString str) {
  return CFStringCreateWithCString(NULL, utf8(str).c_str(), kCFStringEncodingUTF8);
};

QString QStringFromStringPtr(StringPtr str) {
  size_t length = str[0];
  return QString::fromUtf8(reinterpret_cast<char*>(&str[1]), length);
}

bool versionStringLessThanVersionString(QString v1, QString v2) {
  if (v1 == v2) {
    return false;
  }

  QStringList l1 = v1.split(".");
  QStringList l2 = v2.split(".");

  int i = 0;
  for_each(QString segment, l1) {
    if (l2.count() < (i+1)) {
      return false;
    }

    if (segment.toInt() > l2[i].toInt()) {
      return false;
    }
    i++;
  }
  return true;
}

#endif
