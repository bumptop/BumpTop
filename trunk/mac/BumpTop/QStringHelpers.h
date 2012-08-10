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

#ifndef BUMPTOP_QSTRINGHELPERS_H_
#define BUMPTOP_QSTRINGHELPERS_H_

#include <QtCore/QString>
#include <string>

static const QString EMPTY_QSTRING;
std::ostream& operator<<(std::ostream& s, const QString& str);
std::string utf8(const QString& str);
QString QStringFromUtf8(std::string str);
QString QStringFromUtf8(char * str);
#if defined(OS_MACOSX)
QString QStringFromNSString(NSString *str);
const char* utf8(NSString *str);
NSString* NSStringFromQString(const QString& str);
NSString* NSStringFromUtf8(std::string str);
CFStringRef CFStringFromUtf8(std::string str);
CFStringRef CFStringFromQString(QString str);
QString QStringFromStringPtr(StringPtr str);  // Carbon
bool versionStringLessThanVersionString(QString v1, QString v2);
#endif
#endif  // BUMPTOP_QSTRINGHELPERS_H_
