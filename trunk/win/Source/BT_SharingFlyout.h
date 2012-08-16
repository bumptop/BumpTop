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

#ifndef _BT_SHARINGFLYOUT_H_
#define _BT_SHARINGFLYOUT_H_

#include "BT_Common.h"
#include "BT_Flyout.h"

class SharedFolderAPI;

class SharingFlyoutButton : public QPushButton
{
	Q_OBJECT

private:
	QImage _image;
	QLabel *_label;

	QPainterPath _background;
	QColor _backgroundFill;
	int _shift_distance;
	qreal _opacity;

	int BACKGROUND_RADIUS;

protected:
	virtual void paintEvent (QPaintEvent *e);
	virtual void enterEvent (QEvent *e);
	virtual void leaveEvent (QEvent *e);

public:
	SharingFlyoutButton(QWidget *parent=0);
	~SharingFlyoutButton();

	static const qreal OPACITY_SELECTED;
	static const qreal OPACITY_NORMAL;
	static const qreal OPACITY_FADED;

	void setSize(int w, int h);
	void setImage(QImage image);
	void setText(QString text);
	void setBackgroundFill(QColor fill);
	void setOpacity(qreal opacity);

public slots:
	void onPressed();
	void onReleased();
	void onToggle(bool checked);
};

// ---------------------------------------------------------------------------

class SelectAllLineEdit : public QLineEdit
{
	Q_OBJECT

public:
	SelectAllLineEdit(QWidget *parent=0);

	public slots:
		void forceSelectAll(int a, int b);
		void forceSelectNone();
};

// ---------------------------------------------------------------------------

class SharingFlyout : public Flyout
{
	Q_OBJECT
	Q_DECLARE_TR_CONTEXT(SharingFlyout)

	SharedFolderAPI *_sharedFolderAPI;

	QVariantMap *_data;
	QWidget *_parent;

	int BUTTON_ID_EMAIL;
	int BUTTON_ID_FACEBOOK;
	int BUTTON_ID_TWITTER;

	QButtonGroup *_buttonGroup;
	bool _firstButtonClick;

	QLabel *_infoLabel;
	QLabel *_filesLabel;
	QWidget *_uploading;
	QWidget *_link;
	SelectAllLineEdit *_linkField;
	QWidget *_bottomFrame;

	QWidget *_email;
	QPlainTextEdit *_emailBody;

	QWidget *_facebook;
	QWidget *_twitter;

	QSize _sizeHint;

	void hideAll();

	QWidget *initLinkView();
	QWidget *initUploadingView();
	QWidget *initBottomFrame();

private slots:
	void showEmail(QVariantMap *data);
	void showFacebook(QVariantMap *data);
	void showTwitter(QVariantMap *data);

	void cancelClicked();
	void copyClicked();
	void emailClicked();
	void facebookClicked();
	void twitterClicked();
	void sendClicked();
	void browseButtonClicked();

public:
	enum FlyoutState {
		Default,
		Uploading,
		Link,
		Email,
		Facebook,
		Twitter
	};

	SharingFlyout(SharedFolderAPI *api, const QRect& originRect);

	FlyoutState _state;

	void changeState(QString newState, QVariantMap *data);
	void changeState(FlyoutState newState, QVariantMap *data);

	virtual QSize sizeHint() const;

	virtual void setData(QVariantMap *data);
	
public slots:
	void close();
	void buttonClicked(int id);

signals:
	void done();
};

#endif
