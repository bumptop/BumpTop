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

#pragma once

#ifndef _FILE_SYSTEM_PILE_
#define _FILE_SYSTEM_PILE_

// -----------------------------------------------------------------------------

#include "BT_Pile.h"
#include "BT_Watchable.h"

// -----------------------------------------------------------------------------

class BumpObject;
class FileSystemActor;

// -----------------------------------------------------------------------------

class FileSystemPile : public Pile, public Watchable
{
	Q_DECLARE_TR_FUNCTIONS(FileSystemPile)

	FileSystemActor *owner;
	bool _folderizeOnClose;
	Vec3 _folderizeOnClosePosition;

public:

	FileSystemPile();
	virtual ~FileSystemPile();

	// Actions
	virtual bool	addToPile(BumpObject *obj);
	bool			copyToPile(BumpObject *obj);
	virtual bool	removeFromPile(BumpObject *obj, UpdateFlag updateFlag = Update);
	bool			copyFromPile(BumpObject *obj, UpdateFlag updateFlag = Update);
	virtual bool	breakPile();
	FileSystemActor	*folderize(bool animate = true);
	FileSystemActor *convert(Pile *softPile, QString newDirName);
	void			close();
	virtual void	grow(uint numSteps = 25, float growFactor = 1.6f);
	virtual void	shrink(uint numSteps = 25, float shrinkFactor = 0.6f);
	void			setFolderizeOnClose(bool foc, Vec3 pos);

	// Events
	void			onFileAdded(QString strFileName);
	void			onFileRemoved(QString strFileName);
	void			onFileNameChanged(QString strOldFileName, QString strNewFileName);
	void			onFileModified(QString strFileName);
	vector<BumpObject *> onDrop(vector<BumpObject *> &objList);
	void			onTossRecieve(vector<BumpObject *> tossedObjs);
	virtual void	onWidgetClick(BumpObject * widget);

	// Getters
	StrList			getWatchDir();
	QString			getPileOwnerPath();
	FileSystemActor	*getOwner();
	int				isInPile(BumpObject *obj);
	virtual QString resolveDropOperationString(vector<BumpObject *>& objList);
	bool			isSourceValid();
	bool			isValidToss(vector<BumpObject *> tossedObjs);
	bool			isPilable(uint pileType);
	bool			isCopyIntoPile(const vector<BumpObject *>& objList) const;

	// Setters
	void			setOwner(FileSystemActor *newOwner);

};

// -----------------------------------------------------------------------------

#else
	class FileSystemPile;
#endif