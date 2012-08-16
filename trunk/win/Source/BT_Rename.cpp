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

#include "BT_Common.h"
#include "BT_Rename.h"
#include "BT_OverlayComponent.h"
#include "BT_BumpObject.h"
#include "BT_Pile.h"
#include "BT_FileSystemActor.h"
#include "BT_FileSystemPile.h"
#include "BT_WindowsOS.h"
#include "BT_TextManager.h"
#include "BT_RenderManager.h"
#include "BT_CustomQLineEdit.h"
#include "BT_Selection.h"
#include "BT_Util.h"
#include "BT_SceneManager.h"
#include "moc/moc_BT_Rename.cpp"


void Rename::mouseDown(BumpObject *obj, int x, int y)
{	
	if (!_timerActive && sel->getSize() == 1 && sel->getBumpObjects()[0] == obj)
	{
		_mx = x;
		_my = y;
		_obj = obj;		
	}
	else
	{
		_timerActive = false;
	}
}

void Rename::mouseUp(BumpObject *obj, int x, int y)
{
	int dx = x - _mx;
	int dy = y - _my;
	if (!_active && (dx*dx + dy*dy)< GLOBAL(sglClickSize))
	{
		_timer->setSingleShot(true);
		_timer->start(600);
		_timerActive = true;				
	}
	else
	{
		_mx = _my = -1;
	}
}

void Rename::textBoxUpdate()
{
	if (_active)
		textBoxChanged(_lineEdit->text());
}

void Rename::textBoxChanged(const QString& str) 
{	
	QRect rect = _lineEdit->fontMetrics().boundingRect(str);
	if (str.length() == 0)
		rect = _lineEdit->fontMetrics().boundingRect("l");	

	const int offWidth = 14, offHeight = 8;
	_lineEdit->setFixedSize(rect.width() + offWidth, rect.height() + offHeight);
	
	int px, py;
	px = (int)(_obj->getNameableOverlay()->getStyle().getOffset().x + _obj->getLinearVelocity().x/2 + (_obj->getNameableOverlay()->getSize().x )/2 - (rect.width() +offWidth)/2);
	py = winOS->GetWindowHeight() - (int)(_obj->getNameableOverlay()->getStyle().getOffset().y + _obj->getLinearVelocity().y);
	
	POINT windowPos = {0,0};
	if (ClientToScreen(winOS->GetWindowsHandle(), &windowPos)) 
	{
		px += windowPos.x;
		py += windowPos.y;
	}	

	_lineEdit->move(px, py);
	_lineEdit->repaint();
}

void Rename::deleted(BumpObject *obj)
{
	if (obj == _obj)
	{
		_obj = NULL;
		editingCanceled();
	}
}

void Rename::editingCanceled()
{
	if (_active)
	{
		//Must be set inactive before close() is called, as _lineEdit->close() calls editingComplete()
		_active = false;
		_lineEdit->close();
		if (_obj)
			_obj->getNameableOverlay()->getStyle().setVisible(true);

		textManager->invalidate();
		rndrManager->invalidateRenderer();
	}
}

void Rename::editingComplete() 
{	
	if (_active)
	{
		_active = false;
		bool success = true;
		if (_lineEdit->text() != _obj->getNameableOverlay()->getNameable()->getText())
		{
			if (_lineEdit->text().size() != 0 && _obj->getObjectType()==ObjectType(BumpActor, FileSystem))
			{				
				success = winOS->RenameFile(dynamic_cast<FileSystemActor *>(_obj), _lineEdit->text());
			}
			else if (_obj->isBumpObjectType(BumpPile))
			{
				Pile *p = (Pile *) _obj;

				if (_lineEdit->text().size() != 0 && p->getPileType() == HardPile)
				{
					FileSystemPile *fsPile = dynamic_cast<FileSystemPile *>(p);
					success = winOS->RenameFile(fsPile->getOwner(), _lineEdit->text());			
				}
				else if (p->getPileType() == SoftPile)
				{
					p->setText(_lineEdit->text());
				}
			}
		}

		if (success)
		{
			_lineEdit->close();
			_obj->getNameableOverlay()->getStyle().setVisible(true);			
			rndrManager->invalidateRenderer();
			textManager->forceUpdate();
		}
		else
		{
			_active = true;
		}
	}	
}

void Rename::setTimerActive(bool active)
{
	_timerActive = active;
}

Rename::Rename()
{	
	_active = false;
	_mx = _my = -1;
	_timer = new QTimer(this);
	assert(connect(_timer, SIGNAL(timeout()), this, SLOT(rename())));
}

void Rename::rename()
{
	if (_timerActive)
		rename(_obj, false);	
	_timerActive = false;
}

void Rename::rename(BumpObject *obj, bool printErrorMsg)
{
	if (_active)
		return;

	QString initialLabel;

	if (obj->getObjectType() == ObjectType(BumpActor, FileSystem))
	{		
		FileSystemActor * fsActor = (FileSystemActor *) obj;
		// Do not allow the ability to rename post-it notes
		if (fsActor->isFileSystemType(StickyNote) || fsActor->isFileSystemType(PhotoFrame))
		{
			if (printErrorMsg)
				printUnique("Key_RenameSelection", BtUtilStr->getString("ErrorRenameObjectType"));		
			return;
		}
		else
			initialLabel = filename(fsActor->getFullPath());

	}
	else if (obj->isBumpObjectType(BumpPile))
	{
		Pile * p = (Pile *) obj;

		if (p->getPileType() == HardPile)
		{
			FileSystemPile * fsPile = (FileSystemPile *) p;
			QString fileName = filename(fsPile->getPileOwnerPath());
			if (fileName.isEmpty())
				fileName = obj->getNameableOverlay()->getNameable()->getText();
			initialLabel = fileName;
		}
		else if (p->getPileType() == SoftPile)
		{
			if (p->getText().isEmpty())
			{
				p->setText(" ");
				textManager->updateRelativeNameablePositions();
				textManager->updateAbsoluteNameablePositions();				
			}
					
			initialLabel = p->getText();
		}
	}
	else
	{
		if (printErrorMsg)
			printUnique("Key_RenameSelection", BtUtilStr->getString("ErrorRenameObjectType"));
		return;
	}

	_obj = obj;
	_active = true;
	_lineEdit = new CustomQLineEdit();
	_lineEdit->setObjectName("RenamerBox");
	_lineEdit->setWindowFlags(Qt::FramelessWindowHint|Qt::SubWindow);
	_lineEdit->setAttribute(Qt::WA_DeleteOnClose);

	FontDescription fd = _obj->getNameableOverlay()->getTextOverlay()->getFont();
	_lineEdit->setFont(QFont(fd.fontName, fd.fontSize));
	_lineEdit->setText(initialLabel);
	_lineEdit->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
	textBoxChanged(_lineEdit->text());

	//Set the cursor to highlight the base file
	_lineEdit->home(false);
	if (obj->getObjectType()==ObjectType(BumpActor, FileSystem))
	{		
		QFileInfo info = QFileInfo((dynamic_cast<FileSystemActor *>(_obj))->getFullPath());
		
		if (!info.completeBaseName().isEmpty() && !info.isDir())
			_lineEdit->setSelection(0, info.completeBaseName().length());
		else
			_lineEdit->end(true);
	}
	else
		_lineEdit->end(true);

	_lineEdit->activateWindow();
	_lineEdit->show();
	_lineEdit->setFocus();
	_lineEdit->grabKeyboard();	

	_obj->getNameableOverlay()->getStyle().setVisible(false);
	rndrManager->invalidateRenderer();
	

	assert(connect(_lineEdit, SIGNAL(textChanged(const QString&)),this, SLOT(textBoxChanged(const QString&))));
	assert(connect(_lineEdit, SIGNAL(textEdited(const QString&)),this, SLOT(textBoxChanged(const QString&))));
	assert(connect(_lineEdit, SIGNAL(editingFinished()),this, SLOT(editingComplete())));
	assert(connect(_lineEdit, SIGNAL(editingCanceled()), this, SLOT(editingCanceled())));

}