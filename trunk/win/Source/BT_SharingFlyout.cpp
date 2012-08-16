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
#include "BT_JavaScriptAPI.h"
#include "BT_SharingFlyout.h"

#include "moc/moc_BT_SharingFlyout.cpp"

const qreal SharingFlyoutButton::OPACITY_SELECTED = 1.0;
const qreal SharingFlyoutButton::OPACITY_NORMAL = 0.8;
const qreal SharingFlyoutButton::OPACITY_FADED = 0.5;

SharingFlyoutButton::SharingFlyoutButton(QWidget *parent)
: QPushButton(parent)
{
	_label = new QLabel(this);
	_label->move(0, 3);
	_label->setAlignment(Qt::AlignCenter);
	_background = QPainterPath();
	
	BACKGROUND_RADIUS = 5;

	_shift_distance = 0;
	_opacity = OPACITY_NORMAL;

	setCheckable(true);

	connect(this, SIGNAL(pressed()), this, SLOT(onPressed()));
	connect(this, SIGNAL(released()), this, SLOT(onReleased()));
	connect(this, SIGNAL(toggled(bool)), this, SLOT(onToggle(bool)));
}

SharingFlyoutButton::~SharingFlyoutButton()
{
	SAFE_DELETE(_label);
}

void SharingFlyoutButton::setSize(int w, int h)
{
	setMinimumSize(w, h);
	setMaximumSize(w, h);
	_label->setFixedWidth(w);
}

void SharingFlyoutButton::setImage(QImage image)
{
	_image = image;
	_label->move(0, _image.height());
}

void SharingFlyoutButton::setText(QString text)
{
	_label->setText(text);
}

void SharingFlyoutButton::setBackgroundFill(QColor fill)
{
	_background.addRoundedRect(1, 1, width()-3, height()-3, BACKGROUND_RADIUS, BACKGROUND_RADIUS);
	_backgroundFill = fill;
}

void SharingFlyoutButton::setOpacity(qreal opacity)
{
	_opacity = opacity;
	repaint();
}

void SharingFlyoutButton::onPressed()
{
	_shift_distance = 1;
	_label->move(_label->x() + _shift_distance, _label->y() + _shift_distance);
	repaint();
}

void SharingFlyoutButton::onReleased()
{
	_label->move(_label->x() - _shift_distance, _label->y() - _shift_distance);
	_shift_distance = 0;
	repaint();
}

void SharingFlyoutButton::onToggle(bool checked)
{
	if (checked)
		_opacity = OPACITY_SELECTED;
	else
		_opacity = OPACITY_FADED;
}

void SharingFlyoutButton::enterEvent(QEvent *e)
{
	_opacity = OPACITY_SELECTED;
}

void SharingFlyoutButton::leaveEvent(QEvent *e)
{
	if (isChecked())
		_opacity = OPACITY_SELECTED;
	else if (group() == NULL || group()->checkedButton() == NULL)
		_opacity = OPACITY_NORMAL;
	else
		_opacity = OPACITY_FADED;
}

void SharingFlyoutButton::paintEvent(QPaintEvent *e)
{
	QPainter painter(this);
	painter.setOpacity(_opacity);
	painter.translate(_shift_distance, _shift_distance);

	painter.setRenderHint(QPainter::Antialiasing);
	painter.setRenderHint(QPainter::HighQualityAntialiasing);

	painter.fillPath(_background, QBrush(QColor(_backgroundFill)));
	painter.drawPath(_background);
	painter.drawImage(QPoint(0, 0), _image);
}

// ---------------------------------------------------------------------------

SelectAllLineEdit::SelectAllLineEdit(QWidget *parent)
: QLineEdit(parent)
{
	setStyleSheet(QT_NT("border: 1px solid #666666"));
	connect(this, SIGNAL(cursorPositionChanged(int, int)), this, SLOT(forceSelectAll(int, int)));
	connect(this, SIGNAL(editingFinished()), this, SLOT(forceSelectNone()));
}

void SelectAllLineEdit::forceSelectAll(int a, int b)
{
	selectAll();
}

void SelectAllLineEdit::forceSelectNone()
{
	setSelection(0, 0);
}

// ---------------------------------------------------------------------------

SharingFlyout::SharingFlyout(SharedFolderAPI *api, const QRect& originRect)
: Flyout(originRect)
, _sharedFolderAPI(api)
, _data(NULL)
{
	// init constants
	BUTTON_ID_EMAIL = 1;
	BUTTON_ID_FACEBOOK = 2;
	BUTTON_ID_TWITTER = 3;

	_firstButtonClick = true;

	setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Maximum));

	// overall flyout layout
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	setLayout(mainLayout);

	_infoLabel = new QLabel(QT_TR_NOOP(
		"This widget lets you share files with friends. We'll upload them to a "
		"server and give you a link you can share via email or IM."));
	_infoLabel->setWordWrap(true);
	mainLayout->addWidget(_infoLabel);

	// The label that shows the name of the file,
	// only visible when uploadingView or linkView is visible
	_filesLabel = new QLabel(this);
	_filesLabel->setStyleSheet(QT_NT("font-weight: bold;"));
	mainLayout->addWidget(_filesLabel);

	// The view that shows the link to the uploaded file
	_link = initLinkView();
	mainLayout->addWidget(_link);

	// Set the width of the entire flyout based on the width of the link view
	_sizeHint = Flyout::sizeHint();

	// Word wrapping on the label seems to cause layout issues.
	// The fix is to set a fixed size on the label so it doesn't keep trying
	// to reflow the text.
	int labelWidth = _link->sizeHint().width();
	int labelHeight = _infoLabel->heightForWidth(labelWidth);
	_infoLabel->setFixedSize(labelWidth, labelHeight);

	// The view that is shown while the upload is in progress
	_uploading = initUploadingView();
	mainLayout->addWidget(_uploading);

	// On the default & link screens, the area that shows the browse button
	_bottomFrame = initBottomFrame();
	mainLayout->addWidget(_bottomFrame);

	int bottomMargin;
	mainLayout->getContentsMargins(NULL, &bottomMargin, NULL, NULL);
	mainLayout->setSpacing(bottomMargin);

	_email = NULL;
	_facebook = NULL;
	_twitter = NULL;

	changeState(Default, NULL);
}

QWidget *SharingFlyout::initUploadingView()
{
	QWidget *container = new QWidget();

	QLabel *uploadingInstruction = new QLabel(QT_TR_NOOP("Uploading..."));

	SharingFlyoutButton *uploadingCancel = new SharingFlyoutButton();
	uploadingCancel->setSize(55, 22);
	uploadingCancel->setText(QT_TR_NOOP("Cancel"));
	uploadingCancel->setBackgroundFill(QColor(QT_NT("dodgerblue")));
	connect(uploadingCancel, SIGNAL(clicked()), this, SLOT(cancelClicked()));

	QHBoxLayout *uploadingLayout = new QHBoxLayout();
	uploadingLayout->addWidget(uploadingInstruction);
	uploadingLayout->addWidget(uploadingCancel);
	uploadingLayout->setAlignment(uploadingCancel, Qt::AlignRight);
	uploadingLayout->setContentsMargins(0, 0, 0, 0);
	container->setLayout(uploadingLayout);

	return container;
}

QWidget *SharingFlyout::initLinkView()
{
	QWidget *container = new QWidget();

	QLabel *linkInstruction = new QLabel(QT_TR_NOOP("Share this link:"));

	_linkField = new SelectAllLineEdit();
	_linkField->setMinimumWidth(150);
	_linkField->setReadOnly(true);

	SharingFlyoutButton *copy = new SharingFlyoutButton();
	copy->setSize(45, 22);
	copy->setText(QT_TR_NOOP("Copy"));
	copy->setBackgroundFill(QColor(QT_NT("dodgerblue")));
	connect(copy, SIGNAL(clicked()), this, SLOT(copyClicked()));

	QHBoxLayout *linkLayout = new QHBoxLayout();
	linkLayout->setContentsMargins(0, 0, 0, 0);
	linkLayout->addWidget(linkInstruction);
	linkLayout->addWidget(_linkField);
	linkLayout->addWidget(copy);
	container->setLayout(linkLayout);

	return container;
}

QWidget *SharingFlyout::initBottomFrame()
{
	QFrame *frame = new QFrame();
	frame->setFrameStyle(QFrame::Box);
	frame->setStyleSheet(QT_NT(".QFrame { border: 0; border-top: 1px solid #606060; }"));

	// Make the spacing on top the same as below
	int bottomMargin;
	layout()->getContentsMargins(NULL, NULL, NULL, &bottomMargin);

	QHBoxLayout *frameLayout = new QHBoxLayout();
	frameLayout->setContentsMargins(0, bottomMargin, 0, 0);
	frame->setLayout(frameLayout);
	QLabel *label = new QLabel(QT_TR_NOOP("Choose files to share:"));
	frameLayout->addWidget(label, 1, Qt::AlignRight);

	QPushButton *browseButton = new QPushButton();
	browseButton->setText(QT_TR_NOOP("Browse..."));
	connect(browseButton, SIGNAL(clicked()), this, SLOT(browseButtonClicked()));

	frameLayout->addWidget(browseButton, 0, Qt::AlignRight);
	return frame;
}

// Hide all of the top-level widgets, in preparation to show one of them
void SharingFlyout::hideAll()
{
	_infoLabel->hide();
	_filesLabel->hide();
	_uploading->hide();
	_link->hide();
	_bottomFrame->hide();

	if (_email != NULL)
		_email->hide();

	if (_facebook != NULL)
		_facebook->hide();

	if (_twitter != NULL)
		_twitter->hide();
}

void SharingFlyout::changeState(QString newState, QVariantMap *data)
{
	FlyoutState state;

	if (newState == "uploading")
		state = Uploading;
	else if (newState == "link")
		state = Link;
	else if (newState == "email")
		state = Email;
	else if (newState == "facebook")
		state = Facebook;
	else if (newState == "twitter")
		state = Twitter;

	changeState(state, data);
}

void SharingFlyout::changeState(FlyoutState newState, QVariantMap *data)
{
	if (_state == newState)
		return;

	_state = newState;

	hideAll();

	if (newState == Default)
	{
		_infoLabel->show();
		_bottomFrame->show();
	}
	else if (newState == Uploading)
	{
		_filesLabel->show();
		_uploading->show();
	}
	else if (newState == Link)
	{
		_filesLabel->show();
		_linkField->setText(data->value(QString_NT("url")).toString());
		_link->show();
		_bottomFrame->show();
	}
	else if (newState == Email)
	{
		showEmail(data);
	}
	else if (newState == Facebook)
	{
		showFacebook(data);
	}
	else if (newState == Twitter)
	{
		showTwitter(data);
	}

	// Argh! Can't get Qt to re-layout this widget properly.
	// This seems to be the only way to get it to work.
	if (isVisible())
	{
		hide();
		show();
	}
}

void SharingFlyout::showEmail(QVariantMap *data)
{
	QString sampleEmailQ = QT_TR_NOOP("Hi,\n\nI've shared a file with you using BumpTop. You can get it here: ");
	if (_email == NULL)
	{
		// construct email widgets if not already done
		_email = new QWidget(this);

		QWidget *emailFrom = new QWidget(_email);
		QLabel *emailFromInstruction = new QLabel(QT_TR_NOOP("From:"), _email);
		
		QLineEdit *emailFromField = new QLineEdit(_email);

		QHBoxLayout *emailFromLayout = new QHBoxLayout(_email);
		emailFromLayout->setContentsMargins(0, 0, 0, 0);
		emailFromLayout->addWidget(emailFromInstruction);
		emailFromLayout->addWidget(emailFromField);
		emailFrom->setLayout(emailFromLayout);

		QWidget *emailTo = new QWidget(_email);
		QLabel *emailToInstruction = new QLabel(QT_TR_NOOP("To:"), _email);
		emailToInstruction->setFixedWidth(emailFromInstruction->sizeHint().width());

		QLineEdit *emailToField = new QLineEdit(_email);

		QHBoxLayout *emailToLayout = new QHBoxLayout(_email);
		emailToLayout->setContentsMargins(0, 0, 0, 0);
		emailToLayout->addWidget(emailToInstruction);
		emailToLayout->addWidget(emailToField);
		emailTo->setLayout(emailToLayout);

		_emailBody = new QPlainTextEdit(_email);
		int layoutMarginLeft = 0;
		int layoutMarginRight = 0;
//		_layout->getContentsMargins(&layoutMarginLeft, NULL, &layoutMarginRight, NULL);
		_emailBody->setFixedSize(size().width() - layoutMarginLeft - layoutMarginRight, 100);

		SharingFlyoutButton *emailSend = new SharingFlyoutButton(_email);
		emailSend->setSize(45, 18);
		emailSend->setText(QT_TR_NOOP("Send"));
		emailSend->setBackgroundFill(QColor(QT_NT("dodgerblue")));
		connect(emailSend, SIGNAL(clicked()), this, SLOT(sendClicked()));

		QVBoxLayout *emailLayout = new QVBoxLayout(_email);
		emailLayout->addWidget(emailFrom);
		emailLayout->addWidget(emailTo);
		emailLayout->addWidget(_emailBody);
		emailLayout->addWidget(emailSend);
		emailLayout->setAlignment(emailSend, Qt::AlignRight);
		emailLayout->setContentsMargins(0, 0, 0, 0);
		_email->setLayout(emailLayout);

//		_layout->addWidget(_email);
	}
	if (data)
		_emailBody->setPlainText(sampleEmailQ + data->value(QString_NT("url")).toString());
	_email->show();
}

void SharingFlyout::showFacebook(QVariantMap *data)
{
	if (_facebook == NULL)
	{

	}
	_facebook->show();
}

void SharingFlyout::showTwitter(QVariantMap *data)
{
	if (_twitter == NULL)
	{

	}
	_twitter->show();
}

void SharingFlyout::cancelClicked()
{
	close();
} 

void SharingFlyout::copyClicked()
{
	QApplication::clipboard()->setText(_linkField->text());
}

void SharingFlyout::buttonClicked(int id)
{
	if (id == BUTTON_ID_EMAIL)
		emailClicked();
	else if (id == BUTTON_ID_FACEBOOK)
		facebookClicked();
	else if (id == BUTTON_ID_TWITTER)
		twitterClicked();

	// need to explicitly fade out the buttons that weren't clicked, only on the first click
	if (_firstButtonClick == true) {
		((SharingFlyoutButton*)_buttonGroup->button(BUTTON_ID_EMAIL))->setOpacity(SharingFlyoutButton::OPACITY_FADED);
		((SharingFlyoutButton*)_buttonGroup->button(BUTTON_ID_FACEBOOK))->setOpacity(SharingFlyoutButton::OPACITY_FADED);
		((SharingFlyoutButton*)_buttonGroup->button(BUTTON_ID_TWITTER))->setOpacity(SharingFlyoutButton::OPACITY_FADED);
		((SharingFlyoutButton*)_buttonGroup->button(id))->setOpacity(SharingFlyoutButton::OPACITY_SELECTED);
		_firstButtonClick = false;
	}
}

void SharingFlyout::emailClicked()
{
	changeState(SharingFlyout::Email, _data);
}

void SharingFlyout::sendClicked()
{
	close();
}

void SharingFlyout::facebookClicked()
{
	changeState(SharingFlyout::Facebook, _data);
}

void SharingFlyout::twitterClicked()
{
	changeState(SharingFlyout::Twitter, _data);
}

void SharingFlyout::browseButtonClicked()
{
	QString files = _sharedFolderAPI->openFileBrowser();

	// Check if the user cancelled the dialog
	if (files != "[]")
		_sharedFolderAPI->uploadFiles(files);
}

void SharingFlyout::close()
{
	emit done();
}

// Return the preferred size for this widget.
QSize SharingFlyout::sizeHint() const
{
	// Use a constant width which is set on construction of the flyout
	QSize hint = Flyout::sizeHint();
	hint.setWidth(_sizeHint.width());
	return hint;
}

void SharingFlyout::setData( QVariantMap *data )
{
	QString filesStr = data->value(QString_NT("toStr"), QT_TR_NOOP("Unknown file(s)")).toString();
	int maxChars = 40;
	if (filesStr.length() > maxChars)
	{
		filesStr = filesStr.left(maxChars - 3);
		filesStr.append(QT_NT("..."));
	}
	_filesLabel->setText(filesStr);

	changeState(data->value(QString_NT("status")).toString(), data);
	_data = data;
}

