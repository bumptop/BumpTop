// Copyright 2012 Google Inc. All Rights Reserved.
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

#ifndef BT_QTUTIL
#define BT_QTUTIL

#include "BT_Singleton.h"

// generates a new seed for the random number generator based on the current time
void randomSeed();
// for generating a random integer between the specified ranges [..)
int randomInt(int lowerBound, int upperBound);

// for rounding to the nearest step (such that (value % step == 0))
float roundToNearest(float value, float step);
float roundToNext(float value, float step);

// for concatenating qdirectories like in boost::filesystem
QDir operator/(const QDir& lhs, const QDir& rhs) ;
QDir operator/(const QDir& lhs, const QString& rhs) ;
QDir& operator/=(QDir& lhs, const QDir& rhs) ;

// native qstring representation of a qdir/qfileinfo
QString native(const QDir& dir);
QString native(const QFile& f);
QString native(const QFileInfo& p);
QString native(const QString& p);

// convenience functions to check file existance
bool exists(QString path);
bool exists(QFile path);
bool exists(QFileInfo path);
bool exists(QDir path);

// convenience functions to get the parent
QDir parent(const QDir& dir);
QDir parent(const QFileInfo& path);
QDir parent(const QString& path);

// convenience functions to get the filename
QString filename(const QString& path);

// add quotes to a path string if it is not already quoted
void ensureQuoted(QString& pathStr);
void ensureUnquoted(QString& pathStr);

// add final path slash if it is not already there
void ensurePathSlash(QString& pathStr);

// quick shorthand for converting std string to qstring
QString qstring(const std::string& str);
std::string stdString(const QString& str);

// convenience function to convert a json string value to qstring
QString qstringFromValue(const Json::Value& val);

// creates a QFileInfo object from a parent directory and a filename
QFileInfo make_file(const QDir& dir, const QString& name);
QFileInfo make_file(const QString& dir, const QString& name);

// creates a new directory with the given QDir
void create_directory(const QDir& dir);

// returns whether a QDir is empty
bool empty(const QDir& dir);

// loads a file into a unicode QString
QString read_file(QString filename, const char * codec);
QString read_file_utf8(QString filename);

// ensures a file starts with a specific header
bool validate_file_startswith(QString filename, const char * codec, QString header);
bool validate_file_startswith_utf8(QString filename, QString header);

// writes a file from a unicode QString
bool write_file_utf8(QString contents, QString filename);
bool append_file_utf8(QString appendContents, QString filename);

// converts a string into proper case
QString make_proper_case(QString& str);

QRect boundsToQRect(Bounds bounds);

// -----------------------------------------------------------------------------
#endif