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
#include "BT_QtUtil.h"
#include "BT_SceneManager.h"
#include "BT_WindowsOS.h"
#include "BT_Util.h"

#include "moc/moc_BT_Flyout.cpp"

static const double BACKGROUND_OPACITY = 0.85;
static const int POINTER_HEIGHT = 10;
static const int FADE_INTERVAL_MS = 33; // 33 msec == 30 fps
static const int FADE_DURATION_MS = 300;
static const int MINIMUM_VISIBLE_TIME_MS = 2000;
static const int SCREEN_EDGE_MARGIN = 5;

Flyout::Flyout(const QRect& rect) : 
	QWidget(),
	_currentWindowFlags(0),
	_originRect(rect),
	_originalContentMargins(QMargins()),
	_fadeTimer(this),
	_currentOpacity(0.0)
{
	setAttribute(Qt::WA_TranslucentBackground, true);
	setStyleSheet(QT_NT("QLabel { color: white; }"));
	connect(&_fadeTimer, SIGNAL(timeout()), this, SLOT(updateOpacity()));
}

// Override the paint event to have a translucent window with rounded corners.
// TODO: This currently will always draw the pointer on the top.
void Flyout::paintEvent( QPaintEvent *event )
{
	QRect renderRect = rect();
	int middle = renderRect.center().x();

	// The points for a triangle, assuming that it points upwards
	QPoint trianglePoints[3] = 
	{
		QPoint(middle, renderRect.top()), 
		QPoint(middle - POINTER_HEIGHT, renderRect.top() + POINTER_HEIGHT),
		QPoint(middle + POINTER_HEIGHT, renderRect.top() + POINTER_HEIGHT) 
	};

	QRect roundedRect = renderRect.adjusted(0, POINTER_HEIGHT, 0, 0);
	const int CORNER_RADIUS = 5;

	QPainter p(this);
	p.setRenderHint(QPainter::Antialiasing);
	p.setRenderHint(QPainter::HighQualityAntialiasing);
	p.fillRect(event->rect(), Qt::transparent);
	p.setOpacity(BACKGROUND_OPACITY);
	// Use coordinate transformation to render the pointer on the bottom
	if (!_pointerOnTop)
	{
		p.rotate(180.0);
		p.translate(-renderRect.width(), -renderRect.height());
	}

	QBrush fillBrush(QColor::fromRgb(32, 32, 32)); // Dark grey

	// First, draw the filled parts
	p.setPen(Qt::NoPen);
	p.setBrush(fillBrush);
	p.drawRoundedRect(roundedRect, CORNER_RADIUS, CORNER_RADIUS);
	p.drawPolygon(trianglePoints, 3);

	// Now draw the outline
	QLinearGradient gradient(QPointF(0.0, POINTER_HEIGHT), QPointF(0.0, renderRect.height()));
	gradient.setColorAt(0.0, Qt::darkGray);
	gradient.setColorAt(1.0, QColor::fromRgb(64, 64, 64));

	QPen pen;
	pen.setWidth(1);
	pen.setBrush(QBrush(gradient));
	p.setPen(pen);
	p.setBrush(Qt::NoBrush);

	// We don't want an outline across the base of the pointer, so mask that out
	QRect pointerArea(QPoint(trianglePoints[1].x(), trianglePoints[0].y()), trianglePoints[2]);
	pointerArea.adjust(1, 0, -2, 0); // Small tweak to improve the look
	p.setClipRegion(QRegion(rect()) - pointerArea);
	p.drawRoundedRect(roundedRect, CORNER_RADIUS, CORNER_RADIUS);
	p.setClipping(false);

	// Finally, draw the outline on the sides of the pointer
	p.drawLine(trianglePoints[0], trianglePoints[1]);
	pen.setColor(QColor::fromRgb(80, 80, 80));
	p.setPen(pen);
	p.drawLine(trianglePoints[0], trianglePoints[2]);

	p.end();
}

void Flyout::calculatePosition()
{
	// Remember what the original margins were, or restore them if they were
	// previously modified.
	if (_originalContentMargins.isNull())
	{
		int left, top, right, bottom;
		layout()->getContentsMargins(&left, &top, &right, &bottom);
		_originalContentMargins = QMargins(left, top, right, bottom);
	}
	else
	{
		layout()->setContentsMargins(_originalContentMargins);
	}

	QSize size = sizeHint();
	int fullHeight = size.height() + POINTER_HEIGHT;

	// Decide whether the pointer should appear on the top or on the bottom,
	// and set the margins to make room for it.

	bool fitsOnTop = (fullHeight + _originRect.bottom() + SCREEN_EDGE_MARGIN < winOS->GetWindowHeight());

	// When the pointer location switches, we need to repaint the entire widget
	if (fitsOnTop != _pointerOnTop)
		update();
	_pointerOnTop = fitsOnTop;

	QMargins margins = _originalContentMargins;
	if (_pointerOnTop)
		margins.setTop(margins.top() + POINTER_HEIGHT);
	else
		margins.setBottom(margins.bottom() + POINTER_HEIGHT);
	layout()->setContentsMargins(margins);

	// Start with the flyout centered horizontally, above or below the originRect
	int xPos = _originRect.center().x() - (size.width() / 2);
	int yPos = _pointerOnTop ? _originRect.bottom() : _originRect.top() - fullHeight;

	// Make sure it's within the horizontal bounds of the screen
	xPos = max(xPos, SCREEN_EDGE_MARGIN);
	xPos = min(xPos, winOS->GetWindowWidth() - size.width() - SCREEN_EDGE_MARGIN);

	// Translate to screen coordinates
	POINT windowPos = {xPos, yPos};
	ClientToScreen(winOS->GetWindowsHandle(), &windowPos);

	move(windowPos.x, windowPos.y);
}

// Set the Z-order of the flyout's window, based on the state of the BumpTop window
// Ideally, the flyout's window would be a child of the BumpTop window, which
// would make this unnecessary. But Windows doesn't allow child windows to have
// the WS_EX_LAYERED attribute, which is how we achieve transparency. So this
// widget's window is a top-level window, but we play some tricks to make it
// act like it's a child of the BumpTop window.
void Flyout::setWindowZOrder(bool keepOnBottomHint/*=false*/)
{
	Qt::WindowFlags flags = Qt::SubWindow | Qt::FramelessWindowHint;

	// The combination of Qt::WindowStaysOnBottomHint and the call to 
	// SetWindowPos (see below) achieves what we want, EXCEPT when we 
	// are in "show desktop" mode. But in that case, we're ok as long as
	// the window appears on top of BumpTop, because there are no other
	// windows that it could obscure.
	if (keepOnBottomHint || (winOS->GetWindowState() == WorkArea && !winOS->IsInShowDesktopMode()))
		flags |= Qt::WindowStaysOnBottomHint;

	// Setting the window flags causes the window to be hidden, so only
	// do it if the flags have changed.
	if (flags != _currentWindowFlags)
	{
		bool wasVisible = isVisible();
		setWindowFlags(flags);
		_currentWindowFlags = flags;

		if (wasVisible)
			show();
	}
	// Place this window above the BumpTop window
	HWND widgetHwnd = WindowFromDC(getDC());
	SetWindowPos(widgetHwnd, winOS->GetWindowsHandle(), 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
}

// The widget is about to be shown. Figure out where to place it relative
// to the originRect.
void Flyout::showEvent( QShowEvent *event )
{
	calculatePosition();
}

void Flyout::updateOpacity()
{
	qreal increment = 1.0 * _fadeTimer.interval() / FADE_DURATION_MS;
	if (_fadeDirection == FADE_OUT)
		increment *= -1.0;
	_currentOpacity += increment;
	setWindowOpacity(_currentOpacity);
	update(); // Queue a paint event

	if (_currentOpacity < 0.0)
	{
		_currentOpacity = 0.0;
		_fadeTimer.stop();
		hide();
	}
	else if (_currentOpacity > 1.0)
	{
		_currentOpacity = 1.0;
		_fadeTimer.stop();
	}
}

void Flyout::fadeIn()
{
	if (!isVisible())
		_currentOpacity = 0.0;
	setWindowZOrder();
	show();

	_fadeDirection = FADE_IN;
	setWindowOpacity(_currentOpacity);
	_fadeTimer.start(FADE_INTERVAL_MS);
}

void Flyout::fadeOut()
{
	_fadeTimer.stop();
	_fadeDirection = FADE_OUT;
	_fadeTimer.start(FADE_INTERVAL_MS);
}

// A flyout is always anchored to an origin rect (usually the bounds of its
// associated actor). Calling this method (e.g. when the actor moves)
// causes the flyout to update its position accordingly.
// The rect is in client coordinates.
void Flyout::setOriginRect( const QRect& rect )
{
	if (_originRect != rect)
	{
		_originRect = rect;
		calculatePosition();
	}
}

bool Flyout::winEvent( MSG * message, long * result )
{
	// Detect when we are leaving "show desktop" mode, and fix the z-order accordingly
	if (winOS->IsInShowDesktopMode() && message->message == WM_ACTIVATEAPP && !message->wParam)
	{
		setWindowZOrder(true);
	}
	return false;
}

// Set the data to be displayed in the flyout.
// Subclasses should override this.
void Flyout::setData( QVariantMap *data )
{}
