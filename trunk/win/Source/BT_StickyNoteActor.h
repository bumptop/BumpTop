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

#ifndef BT_STICKYNOTEACTOR
#define BT_STICKYNOTEACTOR

#include "BT_FileSystemActor.h"
#include "BT_ThemeManager.h"

// ----------------------------------------------------------------------------

class BumpObject;

// ----------------------------------------------------------------------------

void EditStickyNoteAfterDrop(BumpObject * obj);
void EditStickyNoteAfterAnim(AnimationEntry& entry);
void FinishEditStickyNoteAfterAnim(AnimationEntry& entry);

// ----------------------------------------------------------------------------

class StickyNoteTextEdit : public QTextEdit
{
	Q_OBJECT 

public:
	virtual void focusOutEvent(QFocusEvent * event);
	virtual void keyPressEvent(QKeyEvent * event);

signals:
	void accept();
};

class StickyNoteEditDialog : public QDialog
{
	Q_OBJECT

	QTextEdit * _edit;	

	// painting members
	QImage _background;
	const int _noteInset;

public:
	StickyNoteEditDialog();
	virtual ~StickyNoteEditDialog();

	int getNoteInset();

	// operations
	void syncWithTextDocument(QTextDocument& document, const QImage& documentBackground, bool selectExistingText);

	// overrides
	virtual void focusInEvent(QFocusEvent * event);
	virtual void focusOutEvent(QFocusEvent * event);
	virtual void paintEvent(QPaintEvent * event);
	virtual void moveEvent(QMoveEvent * event);
	virtual bool winEvent(MSG * message, long * result);

public slots:
	void onEditAccept();
};

// ----------------------------------------------------------------------------

class StickyNoteActor : public QObject,
						public FileSystemActor,
						public ThemeEventHandler
{	
	Q_OBJECT
	Q_DECLARE_TR_CONTEXT(StickyNoteActor);

	weak_ptr<ThemeManager> themeManagerRef;

	// temp animation actor
	Actor * _tmpAnimationActor;

	// cached sticky note texture
	static QImage _stickyNoteBackground;
	static qint64 _stickyNoteBackgroundFileSize;

	// predefined strings
	static QStringList _predefinedPhrases;
	
	// qt editing dialog
	static StickyNoteEditDialog * _editDialog;
		
	// qt document layout
	QTextDocument _stickyNoteText;
	float _prevDocumentMargin;
	bool _selectExistingText;

	// texture
#ifdef DXRENDER
	IDirect3DTexture9 * _stickyNoteTextureId;
#else
	unsigned int _stickyNoteTextureId;
#endif

protected:
	QString generateRandomStickyNotePhrase() const;
	void toGrayScale(QImage& image, int bias) const;
	void calculateDialogPoseDims(Mat34& poseOut, Vec3& dimsOut);
	
public:
	StickyNoteActor();
	virtual ~StickyNoteActor();

	// operations
	void syncStickyNoteWithFileContents();
	void generateRandomStickyNote();
	void modifyWithString(QString msg);

	//getters
	QString getStickyText();
	
	// dialog operations
	void launchEditDialog(bool selectExistingText);
	void launchEditDialogImmediate();
	void dismissEditDialog();
	
	// overrides
	virtual void onLaunch();
	virtual void onRender(uint flags = RenderSideless);
#ifdef DXRENDER
	virtual void onRelease();
#endif
	virtual void onThemeChanged();

public slots:
	void finishedEditing();
};

#endif // BT_STICKYNOTEACTOR