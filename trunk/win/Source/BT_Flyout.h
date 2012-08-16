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

#ifndef BUMPTOP_FLYOUT_H_
#define BUMPTOP_FLYOUT_H_

// A Flyout is a kind of notification widget.
// The general idea is a cross between a non-modal dialog box, and tooltips.
// Subclass this widget to provide the actual content for the flyout.

class Flyout : public QWidget
{
	Q_OBJECT

private:
	enum FadeDirection { FADE_IN, FADE_OUT };

	Qt::WindowFlags _currentWindowFlags;
	QRect _originRect;
	QMargins _originalContentMargins;
	QTimer _fadeTimer; // Used for implementing fadeIn/fadeOut
	qreal _currentOpacity;
	FadeDirection _fadeDirection;
	bool _pointerOnTop; // If true, the flyout's pointer appears on the top

	void calculatePosition();

private slots:
	void updateOpacity();

protected:
	virtual void paintEvent(QPaintEvent *event);
	virtual void showEvent(QShowEvent *event);
	virtual bool winEvent ( MSG * message, long * result );

public:
	Flyout(const QRect& rect);

	void setOriginRect(const QRect& rect);
	void fadeIn();
	void fadeOut();
	virtual void setData(QVariantMap *data);

	// Public, but shouldn't be called externally except from WindowsSystem::BringWindowToForeground
	void setWindowZOrder(bool keepOnBottomHint=false);
};

#endif // BUMPTOP_FLYOUT_H_
