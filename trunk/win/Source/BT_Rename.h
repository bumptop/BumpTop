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

#ifndef BUMPTOP_RENAME_H_
#define BUMPTOP_RENAME_H_

#include "BT_Common.h"
#include "BT_Singleton.h"

class CustomQLineEdit;
class BumpObject;

class Rename : public QObject
{
	Q_OBJECT

private:
	QTimer* _timer;
	CustomQLineEdit* _lineEdit;
	BumpObject *_obj;
	bool _active;
	int _mx, _my;
	bool _timerActive;

public:
	Rename();
	void rename(BumpObject *obj, bool printErrorMsg); 	
	void textBoxUpdate();
	void mouseDown(BumpObject *obj, int x, int y);
	void mouseUp(BumpObject *obj,  int x, int y);
	void setTimerActive(bool active);
	void deleted(BumpObject *obj);
	friend class Singleton<Rename>;

public slots: 
	void textBoxChanged(const QString & text);
	void editingComplete();
	void editingCanceled();
	void rename();
};

#define Renamer Singleton<Rename>::getInstance()

#endif  // BUMPTOP_RENAME_H_