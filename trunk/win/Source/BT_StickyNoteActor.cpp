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
#include "BT_Camera.h"
#include "BT_AnimationManager.h"
#include "BT_Authorization.h"
#include "BT_BumpObject.h"
#include "BT_CustomActor.h"
#ifdef DXRENDER
#include "BT_DXRender.h"
#endif
#include "BT_FontManager.h"
#include "BT_SceneManager.h"
#include "BT_Settings.h"
#include "BT_StickyNoteActor.h"
#include "BT_StickyNoteMenuActionCustomizer.h"
#include "BT_WindowsOS.h"
#include "BlitzBlur.h"
#include "BumpTop.h"
#include "moc/moc_BT_StickyNoteActor.cpp"

// Some animation constants
const int StickyNoteActorNumTempActorSteps = 13;
const int StickyNoteActorNumStickySteps = 8;

void EditStickyNoteAfterDrop(BumpObject * object) 
{
	// ensure that it is a sticky note
	if (object->isObjectType(ObjectType(BumpActor, FileSystem, StickyNote)))
	{
		StickyNoteActor * actor = (StickyNoteActor *) object;
		actor->launchEditDialog(true);
	}
}

void EditStickyNoteAfterAnim(AnimationEntry& entry)
{
	assert(entry.getObject());

	Actor * tempActor = (Actor *) entry.getObject();
	StickyNoteActor * stickyNote = (StickyNoteActor *) tempActor->getActorToMimic();
	stickyNote->launchEditDialogImmediate();
	tempActor->pushActorType(Invisible);
}

void FinishEditStickyNoteAfterAnim(AnimationEntry& entry)
{
	assert(entry.getObject());

	Actor * tempActor = (Actor *) entry.getObject();
	StickyNoteActor * stickyNote = (StickyNoteActor *) tempActor->getActorToMimic();
	stickyNote->syncStickyNoteWithFileContents();
	
	// delete the temporary actor
	SAFE_DELETE(tempActor);
}

void StickyNoteTextEdit::focusOutEvent(QFocusEvent * event)
{
	if (event->reason() != Qt::PopupFocusReason)
		emit accept();
	QTextEdit::focusOutEvent(event);
}

void StickyNoteTextEdit::keyPressEvent(QKeyEvent * event)
{
	// handle ctrl+enter since that's what is commonly used to commit a piece of text
	if (event->modifiers() == Qt::ControlModifier &&
		(event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter))
	{
		emit accept();
	}
	QTextEdit::keyPressEvent(event);
}

StickyNoteEditDialog::StickyNoteEditDialog()
: QDialog(NULL, Qt::FramelessWindowHint|Qt::SubWindow)
, _edit(NULL)
, _noteInset(10)
{
	// make this window a child of the BumpTop window to hide it in the task bar
	// HWND hwnd = WindowFromDC(getDC());
	// SetParent(hwnd, winOS->GetWindowsHandle());

	// resize and center the window
	resize(320, 320);
	move(winOS->GetWindowWidth() / 2 - width() / 2,
		winOS->GetWindowHeight() / 2 - height() / 2);
	setStyleSheet(QT_NT("QDialog {")
		QT_NT("border: 0px;")
	QT_NT("}"));
	setAttribute(Qt::WA_TranslucentBackground);

	// create the dialog content
	QLayout * layout = NULL;
	layout = new QHBoxLayout(this);
	layout->setContentsMargins(20, 20, 25, 30);	
		_edit = new StickyNoteTextEdit();
		_edit->setStyleSheet(QT_NT("QTextEdit {")
			QT_NT("background: rgb(0,0,0,0);")
			QT_NT("border: 0px;")
			QT_NT("selection-color: rgb(240,240,240);")
			QT_NT("selection-background-color: rgb(60,60,60);")
		QT_NT("}"));
		layout->addWidget(_edit);
	connect(_edit, SIGNAL(accept()), this, SLOT(onEditAccept()));

	// try and forward focus events to the edit field
	setFocusProxy(_edit);
}

StickyNoteEditDialog::~StickyNoteEditDialog()
{
	SAFE_DELETE(_edit);
}

int StickyNoteEditDialog::getNoteInset()
{
	return _noteInset;
}

void StickyNoteEditDialog::syncWithTextDocument(QTextDocument& document, const QImage& documentBackground, bool selectExistingText)
{
	assert(_edit);
	
	// update the background
	_background = documentBackground.scaled(width() - (2 * _noteInset), height() - (2 * _noteInset), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

	// use the same text document
	_edit->setDocument(&document);
	if (selectExistingText)
	{
		_edit->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
		_edit->moveCursor(QTextCursor::Start, QTextCursor::KeepAnchor);
	}
	else
	{
		_edit->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
	}
	repaint();
}

void StickyNoteEditDialog::focusInEvent(QFocusEvent * event)
{
	assert(_edit);
	// forward the event onto the edit dialog
	_edit->setFocus();
}

void StickyNoteEditDialog::focusOutEvent(QFocusEvent * event)
{
	// just accept this dialog to close and save it
	accept();
}

void StickyNoteEditDialog::paintEvent(QPaintEvent * event)
{
	// override the widget background
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setRenderHint(QPainter::HighQualityAntialiasing);
	painter.setPen(Qt::NoPen);

	// draw the note background
	painter.drawImage(_noteInset, _noteInset, _background);
}

void StickyNoteEditDialog::moveEvent(QMoveEvent * event)
{
	// refresh the qt widget
	repaint();
	return QDialog::moveEvent(event);
}

bool StickyNoteEditDialog::winEvent(MSG * msg, long * result)
{
	POINT p = {0};
	bool evtResult = QDialog::winEvent(msg, result);
	if (msg->message == WM_NCHITTEST)
	{
		GetCursorPos(&p);
		p.x -= pos().x();
		p.y -= pos().y();
		QWidget * widget = childAt(p.x, p.y);
		if (!widget)
		{
			// if we are not clicking over a child widget, fool windows into 
			// thinking we are clicking a caption bar (to allow it to move
			// as if we were)
			if (msg->hwnd == WindowFromDC(getDC()))
			{
				*result = HTCAPTION;
				return true;
			}
		}
	}
	return evtResult;
}

void StickyNoteEditDialog::onEditAccept()
{
	accept();
}

// ----------------------------------------------------------------------------

// static initialization
QStringList StickyNoteActor::_predefinedPhrases;
StickyNoteEditDialog * StickyNoteActor::_editDialog = NULL;
QImage StickyNoteActor::_stickyNoteBackground;
qint64 StickyNoteActor::_stickyNoteBackgroundFileSize = 0;

StickyNoteActor::StickyNoteActor()
: FileSystemActor()
, themeManagerRef(themeManager)
, _stickyNoteTextureId(0)
, _tmpAnimationActor(NULL)
, _selectExistingText(false)
{
	// populate the predefined strings if necessary
	if (_predefinedPhrases.isEmpty())
	{
		//const QString doubleClickMsg("\n(Double click it later to edit)");
		_predefinedPhrases.append(QT_TR_NOOP("- Open another sticky note to read these"));
		if (winOS->GetLocaleLanguage().startsWith("en"))
		{
			_predefinedPhrases.append(QT_TR_NOOP("- Do something nice"));
			_predefinedPhrases.append(QT_TR_NOOP("- Bump around"));
			_predefinedPhrases.append(QT_TR_NOOP("- Do one thing everyday that scares you"));
			_predefinedPhrases.append(QT_TR_NOOP("- Buy a pen"));
			_predefinedPhrases.append(QT_TR_NOOP("- Take a nap"));
			_predefinedPhrases.append(QT_TR_NOOP("- Take a deep breath"));
			_predefinedPhrases.append(QT_TR_NOOP("- Buy a goldfish"));
			_predefinedPhrases.append(QT_TR_NOOP("- Pick up groceries"));
			_predefinedPhrases.append(QT_TR_NOOP("- Call Mom"));
#ifndef DISABLE_PHONING
			_predefinedPhrases.append(QT_TR_NOOP("- Visit BumpTop.com"));
#endif
			_predefinedPhrases.append(QT_TR_NOOP("- Think about life"));
			_predefinedPhrases.append(QT_TR_NOOP("- Reflect"));
			_predefinedPhrases.append(QT_TR_NOOP("- Read a book"));
			_predefinedPhrases.append(QT_TR_NOOP("- Take up Yoga"));
			_predefinedPhrases.append(QT_TR_NOOP("- Walk the dog"));
			_predefinedPhrases.append(QT_TR_NOOP("- Bake some bread"));
			_predefinedPhrases.append(QT_TR_NOOP("- Say something nice about someone"));
			_predefinedPhrases.append(QT_TR_NOOP("- Mow the lawn"));
			_predefinedPhrases.append(QT_TR_NOOP("- Plant a tree"));
			_predefinedPhrases.append(QT_TR_NOOP("- Save the planet"));
			_predefinedPhrases.append(QT_TR_NOOP("- Change the world"));
			_predefinedPhrases.append(QT_TR_NOOP("- Make a difference"));
			_predefinedPhrases.append(QT_TR_NOOP("- Fix global warming"));
			_predefinedPhrases.append(QT_TR_NOOP("- Do the dishes"));
			_predefinedPhrases.append(QT_TR_NOOP("- Write a blog entry"));
			_predefinedPhrases.append(QT_TR_NOOP("- Donate to a good cause"));
			_predefinedPhrases.append(QT_TR_NOOP("- Clean up my desktop"));
			_predefinedPhrases.append(QT_TR_NOOP("- Tell a friend about BumpTop"));
			_predefinedPhrases.append(QT_TR_NOOP("- Get some exercise"));
			_predefinedPhrases.append(QT_TR_NOOP("- Go for a bike ride"));
		}
	}

	pushFileSystemType(StickyNote);
	setText(QT_NT(""));
	hideText(true);
	++GLOBAL(settings).curNumStickyNotes;

	// change the sticky note pad texture to reflect the sticky note pad state
	if (StickyNoteMenuActionCustomizer::hasExceededMaxNumStickyNotes())
	{
		CustomActor * actor = scnManager->getCustomActor<StickyNotePadActorImpl>();
		if (actor)
			actor->setTextureID("icon.custom.stickyNotePadDisabled");
	}

	// enable theme event notifications
	themeManager->registerThemeEventHandler(this);
}

StickyNoteActor::~StickyNoteActor()
{
	bool prevHasExceededMaxNumStickyNotes = StickyNoteMenuActionCustomizer::hasExceededMaxNumStickyNotes();

	if ((GLOBAL(settings).curNumStickyNotes > 0))
		--GLOBAL(settings).curNumStickyNotes;
	
	// change the sticky note pad texture to reflect the sticky note pad state
	if (prevHasExceededMaxNumStickyNotes && 
		!StickyNoteMenuActionCustomizer::hasExceededMaxNumStickyNotes())
	{
		CustomActor * actor = scnManager->getCustomActor<StickyNotePadActorImpl>();
		if (actor)
			actor->setTextureID("icon.custom.stickyNotePad");
	}

	// disable theme event notifications
	if (shared_ptr<ThemeManager> tm = themeManagerRef.lock())
		tm->unregisterThemeEventHandler(this);

	// destroy the temporary actor
	SAFE_DELETE(_tmpAnimationActor);

	// dismiss the edit dialog
	dismissEditDialog();

	// destroy the dialog
	SAFE_DELETE(_editDialog);
	
	// free the sticky note texture
#ifdef DXRENDER
	onRelease();
#endif
}

void StickyNoteActor::syncStickyNoteWithFileContents()
{
	reauthorize(false);

	// clear the old texture
#ifdef DXRENDER
	SAFE_RELEASE(_stickyNoteTextureId);
#else
	if (_stickyNoteTextureId)
	{
		glDeleteTextures(1, &_stickyNoteTextureId);
		_stickyNoteTextureId = 0;
	}
#endif

	// get the font
	int fontSize = NxMath::max(20, themeManager->getValueAsInt("ui.stickyNote.font.size",0));
	FontDescription desc(themeManager->getValueAsFontFamilyName("ui.stickyNote.font.family",""), fontSize);
	QFont stickyNoteFont = fontManager->getFont(desc);

	// get the sticky note text
	QString stickyNoteStr;
	getStickyNote(getFullPath(), &stickyNoteStr);

	// layout the sticky note text
	QTextOption option;
		option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
	_stickyNoteText.setDefaultTextOption(option);
	_stickyNoteText.setDefaultFont(stickyNoteFont);
	_stickyNoteText.setDocumentMargin(12.5f);
	_stickyNoteText.setTextWidth(256);
	_stickyNoteText.setPlainText(stickyNoteStr);

	QSizeF textSize = _stickyNoteText.size();
	const int minStickyNoteFontSize = 10;
	while (textSize.height() > 256 && 
		   stickyNoteFont.pointSize() > minStickyNoteFontSize)
	{
		stickyNoteFont.setPointSize(stickyNoteFont.pointSize() - 1);
		_stickyNoteText.setDefaultFont(stickyNoteFont);
		textSize = _stickyNoteText.size();
	}

	// render the sticky note to a qimage
	const unsigned int bufferSize = 256;
	QImage image(bufferSize, bufferSize, QImage::Format_ARGB32);
	image.fill(Qt::red);
	QPainter p;
	p.begin(&image);
	p.setRenderHint(QPainter::Antialiasing, true);
	p.setRenderHint(QPainter::TextAntialiasing, true);
		p.drawImage(0, 0, _stickyNoteBackground);
		p.setPen(QColor(50, 50, 75));
		_stickyNoteText.drawContents(&p, QRect(0, 0, bufferSize, int(bufferSize - _stickyNoteText.documentMargin())));
	p.end();
#ifdef DXRENDER
	_stickyNoteTextureId = dxr->createTextureFromData(image.width(), image.height(), image.bits(), image.bytesPerLine());
#else
	// load the qimage into open gl
	glGenTextures(1, &_stickyNoteTextureId);
	glBindTexture(GL_TEXTURE_2D, _stickyNoteTextureId);

	float maximumAnisotropy = 0.0f;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maximumAnisotropy);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maximumAnisotropy);
	if (GLEW_ARB_texture_border_clamp)
	{
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER_ARB);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER_ARB);
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

	// copy the full image (glTexSubImage2D does not work well with GL_GENERATE_MIPMAP)
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width(), image.height(),
		0, GL_BGRA, GL_UNSIGNED_BYTE, image.bits());
#endif
}

void StickyNoteActor::generateRandomStickyNote()
{
	modifyWithString(generateRandomStickyNotePhrase());
	syncStickyNoteWithFileContents();
}

void StickyNoteActor::modifyWithString(QString msg)
{
	if (!isFileSystemType(StickyNote))
		return;
	writeStickyNote(getFullPath(), msg);
}

void StickyNoteActor::calculateDialogPoseDims(Mat34& poseOut, Vec3& dimsOut)
{
	POINT windowPos = {0, 0};
	ClientToScreen(winOS->GetWindowsHandle(), &windowPos);
	float dist = 0.0f;
	Vec3 v, w, dir, centroidPt, cornerPt;
	Ray ray;
	NxPlane plane(cam->getDir(), -80.0f);

	// get the centroid point on the plane
	window2world(_editDialog->pos().x() - windowPos.x + _editDialog->width() / 2.0f,
		_editDialog->pos().y() - windowPos.y + _editDialog->height() / 2.0f, v, w);
	dir = w - v;
	dir.normalize();
	ray = Ray(v, dir);
	NxRayPlaneIntersect(ray, plane, dist, centroidPt);

	// get the corner point
	window2world(_editDialog->pos().x() - windowPos.x + _editDialog->width() - _editDialog->getNoteInset(),
		_editDialog->pos().y() - windowPos.y + _editDialog->height() - _editDialog->getNoteInset(), v, w);
	dir = w - v;
	dir.normalize();
	ray = Ray(v, dir);
	NxRayPlaneIntersect(ray, plane, dist, cornerPt);

	// calculate the final dims
	Vec3 diff = centroidPt - cornerPt;
	dimsOut = Vec3 (diff.x, diff.z, getDims().z);
			
	// calculate the final orientation/position
	Mat33 ori;
	Vec3 zaxis = cam->getDir() - cam->getEye();
	zaxis.normalize();
	Vec3 xaxis = cam->getUp().cross(zaxis);
	xaxis.normalize();
	Vec3 yaxis = zaxis.cross(xaxis);
	
	ori.setColumn(0, xaxis);
	ori.setColumn(1, yaxis);
	ori.setColumn(2, zaxis);
	poseOut = Mat34(ori, centroidPt);
}

void StickyNoteActor::launchEditDialog(bool selectExistingText)
{
	_selectExistingText = selectExistingText;

	// NOTE: for the future, we may want QuickLook-like behaviour where
	// we follow the sticky notes as the user clicks around on the screen.
	// If so, then we should keep a reference to the existing dialog some
	// where and pass it into this launch process?
	
	// if one does not exist, create a new sticky not dialog
	// and center it
	if (!_editDialog)
		_editDialog = new StickyNoteEditDialog();
	else
	{
		// close the dialog and disconnect all signals
		_editDialog->accept();
		_editDialog->disconnect();
	}
	
	// setup all the connections
	connect(_editDialog, SIGNAL(finished(int)), this, SLOT(finishedEditing()));
	connect(_editDialog, SIGNAL(accepted()), this, SLOT(finishedEditing()));
	connect(_editDialog, SIGNAL(rejected()), this, SLOT(finishedEditing()));

	// move it to the appropriate place
	const int marginLeftRight = 20;
	const int marginTopBottom = -20;
	Bounds b = this->getScreenBoundingBox();
	POINT windowPos = {0, 0};
	ClientToScreen(winOS->GetWindowsHandle(), &windowPos);
	Vec3 screenMin = b.getMin() + Vec3(windowPos.x, windowPos.y, 0);
	Vec3 screenMax = b.getMax() + Vec3(windowPos.x, windowPos.y, 0);
	Vec3 screenDims = Vec3(winOS->GetWindowWidth(), winOS->GetWindowHeight(), 0);
	Vec3 tr(NxMath::max(screenMin.x, screenMax.x), NxMath::min(screenMin.y, screenMax.y), 0);
	Vec3 tl(NxMath::min(screenMin.x, screenMax.x), NxMath::min(screenMin.y, screenMax.y), 0);
	int yBound = NxMath::min(NxMath::max((int) tr.y + marginTopBottom, windowPos.y + abs(marginTopBottom)), windowPos.y + (int) screenDims.y - abs(marginTopBottom) - _editDialog->height());
	if ((tr.x + marginLeftRight + _editDialog->width()) <= (windowPos.x + screenDims.x - marginLeftRight))
	{
		// put this note to the right of this actor
		_editDialog->move(tr.x + marginLeftRight, yBound);
	}
	else if ((tl.x - marginLeftRight - _editDialog->width()) >= marginLeftRight)
	{
		// put this note to the left of this actor
		_editDialog->move((tl.x - marginLeftRight - _editDialog->width()), yBound);
	}

	Mat34 finalPose;
	Vec3 finalDims;
	calculateDialogPoseDims(finalPose, finalDims);
	const int StickyNoteActorNumTempActorSteps = 13;
	const int StickyNoteActorNumStickySteps = 8;
	
	// create a new proxy actor
	assert(!_tmpAnimationActor);
	_tmpAnimationActor = new Actor();
	_tmpAnimationActor->pushActorType(Temporary);
	_tmpAnimationActor->setObjectToMimic(this);
	_tmpAnimationActor->setFrozen(false);
	_tmpAnimationActor->setGravity(false);
	_tmpAnimationActor->setCollisions(false);
	_tmpAnimationActor->setAlphaAnim(0.0f, 1.0f, StickyNoteActorNumTempActorSteps);
	_tmpAnimationActor->setSizeAnim(lerpRange(getDims(), finalDims, StickyNoteActorNumTempActorSteps, SoftEase));
	_tmpAnimationActor->setPoseAnim(slerpPose(getGlobalPose(), finalPose, StickyNoteActorNumTempActorSteps, SoftEase), (FinishedCallBack) EditStickyNoteAfterAnim, NULL);
	setAlphaAnim(getAlpha(), 0.4f, StickyNoteActorNumStickySteps);
}

void StickyNoteActor::launchEditDialogImmediate()
{
	// update the dialog and show it	
	_stickyNoteText.setModified(false);
	_editDialog->syncWithTextDocument(_stickyNoteText, _stickyNoteBackground, _selectExistingText);
	_editDialog->activateWindow();
	_editDialog->show();
	_editDialog->setFocus();
}

void StickyNoteActor::dismissEditDialog()
{
	if (_editDialog)
	{
		SAFE_DELETE(_editDialog);
	}
}

QString StickyNoteActor::generateRandomStickyNotePhrase() const
{
	assert(!_predefinedPhrases.isEmpty());
	return _predefinedPhrases[randomInt(0, _predefinedPhrases.size())];
}

void StickyNoteActor::onLaunch()
{
	launchEditDialog(false);
}

void StickyNoteActor::onRender(uint flags)
{
	if (!isActorType(Invisible))
	{
		if (_stickyNoteTextureId == 0)
			syncStickyNoteWithFileContents();
		
		setTextureID("icon.custom.stickyNote");
		Actor::onRender(flags);
#ifdef DXRENDER
		dxr->device->SetRenderState(D3DRS_ZENABLE, false);
		dxr->renderSideLessBox(getGlobalPosition(), getGlobalOrientation(), getDims(), _stickyNoteTextureId);
		dxr->device->SetRenderState(D3DRS_ZENABLE, true);
#endif
	}
}

#ifdef DXRENDER
void StickyNoteActor::onRelease()
{
	SAFE_RELEASE(_stickyNoteTextureId);

	// Call the base class
	FileSystemActor::onRelease();
}
#endif

void StickyNoteActor::toGrayScale(QImage& image, int bias) const
{
	// naive grayscale conversion, since it is only done once when 
	// the theme is loaded/changed
	// http://en.wikipedia.org/wiki/Grayscale
	QRgb pixel;
	int gray = 0;
	int width = image.width();
	int height = image.height();
	for (int i = 0; i < height; ++i)
	{
		for (int j = 0; j < width; ++j)
		{
			pixel = image.pixel(j, i);
			gray = (11 * qRed(pixel) + 16 * qGreen(pixel) + 5 * qBlue(pixel)) / 32 + bias;
			image.setPixel(j, i, qRgba(gray, gray, gray, qAlpha(pixel)));
		}
	}
}

void StickyNoteActor::onThemeChanged()
{
	// reload all the necessary textures
	QDir texPath = winOS->GetUserThemesDirectory(true) / themeManager->getValueAsQString("textures.relativeRoot","");
	QDir iconRelPath = native(texPath / themeManager->getValueAsQString("textures.icon.relativeRoot","")).append('\\');
	QString iconCustomRelPath = native(iconRelPath / themeManager->getValueAsQString("textures.icon.custom.relativeRoot","")).append('\\');
	QString stickyNoteTexturePath = iconCustomRelPath + themeManager->getValueAsQString("textures.icon.custom.stickyNote","");
	if (exists(stickyNoteTexturePath))
	{
		qint64 filesize = QFileInfo(stickyNoteTexturePath).size();
		if (_stickyNoteBackgroundFileSize != filesize)
		{
			QImage tmp = _stickyNoteBackground = QImage(stickyNoteTexturePath);
			toGrayScale(_stickyNoteBackground, -120);
			_stickyNoteBackground = Blitz::blur(_stickyNoteBackground, 2);
			QPainter p;
				p.begin(&_stickyNoteBackground);
				p.drawImage(0, 0, tmp);
				p.end();
			_stickyNoteBackgroundFileSize = filesize;
		}
	}
}

void StickyNoteActor::finishedEditing()
{
	// update the file
	if (_stickyNoteText.isModified())
	{
		writeStickyNote(getFullPath(), _stickyNoteText.toPlainText());
		
		// notify the user with a little animation
		setFreshnessAlphaAnim(1.0f, 25);
	}

	// animate back and then finish up the visuals
	if (_tmpAnimationActor)
	{
		Mat34 startPose;
		Vec3 startDims;
		int numTempActorSteps = StickyNoteActorNumTempActorSteps;
		int numStickySteps = StickyNoteActorNumStickySteps;
		if (_tmpAnimationActor->isAnimating())
		{
			animManager->removeAnimation(_tmpAnimationActor);
			startPose = _tmpAnimationActor->getGlobalPose();
			startDims = _tmpAnimationActor->getDims();
			numTempActorSteps = max(1, (int)((_tmpAnimationActor->getAlpha() / 1.0f) * numTempActorSteps));
			numStickySteps = max(1, (int)(((1.0f - getAlpha()) / 1.0f) * numStickySteps));
		}
		else
		{
			calculateDialogPoseDims(startPose, startDims);
		}
		
		_tmpAnimationActor->popActorType(Invisible);
		_tmpAnimationActor->setAlphaAnim(_tmpAnimationActor->getAlpha(), 0.0f, numTempActorSteps);
		_tmpAnimationActor->setSizeAnim(lerpRange(startDims, getDims(), numTempActorSteps, SoftEase));
		_tmpAnimationActor->setPoseAnim(slerpPose(startPose, getGlobalPose(), numTempActorSteps, SoftEase), (FinishedCallBack) FinishEditStickyNoteAfterAnim, NULL);
		_tmpAnimationActor = NULL;
		setAlphaAnim(getAlpha(), 1.0f, numStickySteps);
	}
}

QString StickyNoteActor::getStickyText()
{
	return _stickyNoteText.toPlainText();
}