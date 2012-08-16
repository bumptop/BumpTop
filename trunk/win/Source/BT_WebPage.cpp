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

#ifdef ENABLE_WEBKIT

#include "BumpTop.h"
#include "BT_JavaScriptAPI.h"
#include "BT_OverlayComponent.h"
#include "BT_SceneManager.h"
#include "BT_Util.h"
#include "BT_WebPage.h"
#include "BT_WindowsOS.h"
#include "BT_Windows7Multitouch.h"
#include "NetworkAccessManager.h"

#include "moc/moc_BT_WebPage.cpp"

class WebView : public QWebView
{
	void closeEvent(QCloseEvent *event) { event->ignore(); } // WebView can only be closed by BumpTop calling hide
	void keyPressEvent ( QKeyEvent * event ) { 
		if(event->key() == Qt::Key_Escape)  
			KeyboardCallback(KeyEscape,0,0); // We want BumpTop to handle the ESC keypress
		else if (event->modifiers() == Qt::CTRL && event->key() == Qt::Key_C)
		{
			QString text = selectedText();
			if (!text.isEmpty())
				QApplication::clipboard()->setText(text);
		}
		else
			QWebView::keyPressEvent(event); // Otherwise go on as normal
	}
	void mouseReleaseEvent ( QMouseEvent  * event ) {
		if (event->button() == Qt::XButton1) {
			back();

		}
		else if (event->button() == Qt::XButton2) {
			forward();
		}
		else
			QWebView::mouseReleaseEvent(event);	//default case
	}
	virtual void dragEnterEvent(QDragEnterEvent *){}
    virtual void dragLeaveEvent(QDragLeaveEvent *){}
    virtual void dragMoveEvent(QDragMoveEvent *){}
    virtual void dropEvent(QDropEvent *){}
};

WebPage::WebPage()
: _widget(NULL)
, _consoleLoggingQWebPage()
, _loaded(false)
, _isDirty(false)
, _isBufferDirty(false)
, _isExternal(false)
, _requestedSize(0, 0)
, _enableProgressNotifications(false)
, _apiForNextPageLoad(NULL)
, _wantBumpTopMouseEvents(true)
{}

WebPage::~WebPage()
{
	_widget->page()->mainFrame()->disconnect(this);
	_widget->page()->disconnect(this);
	_widget->disconnect(this);
	SAFE_DELETE(_widget);
}

// Load a URL, and optionally add an API object which will be accessible
// as a var named "BumpTop" in the global scope.
void WebPage::load(const QString& url, JavaScriptAPI* api/*=NULL*/, bool executeQueuedCodeOnThisPage/*=false*/)
{
	_executeCode = executeQueuedCodeOnThisPage;
	_loaded = false;
	_apiForNextPageLoad = api;
	_widget->load(url);
}

void WebPage::loadHTML(const QString& html)
{
	_widget->setHtml(html);
}

void WebPage::init(const QSize & contentSize, bool autoUpdate)
{
	// create the webview if not already created
	if (!_widget)
	{
		_widget = new WebView();

		// Widget must use our subclass of QWebPage so we can get JS console logging
		_widget->setPage(&_consoleLoggingQWebPage);

		// set the properties
		/* Test transparency
		QPalette palette = _widget->palette();
		palette.setBrush(QPalette::Base, Qt::transparent);
		_widget->page()->setPalette(palette);
		_widget->setAttribute(Qt::WA_OpaquePaintEvent, false);
		_widget->setAttribute(Qt::WA_NoSystemBackground, true);
		_widget->setAttribute(Qt::WA_TranslucentBackground, true);
		_widget->setWindowFlags(Qt::FramelessWindowHint);
		_widget->setAutoFillBackground(false);
		*/
		_widget->settings()->setAttribute(QWebSettings::AutoLoadImages, true);
		_widget->settings()->setAttribute(QWebSettings::JavascriptEnabled, true);
		_widget->settings()->setAttribute(QWebSettings::JavascriptCanOpenWindows, true);
		_widget->settings()->setAttribute(QWebSettings::JavascriptCanAccessClipboard, true);		
		_widget->settings()->setAttribute(QWebSettings::LinksIncludedInFocusChain, true);
		_widget->settings()->setAttribute(QWebSettings::PluginsEnabled, false);
		_widget->settings()->setAttribute(QWebSettings::JavaEnabled, true);

		// setup the cache directories
		QDir cacheDir = (winOS->GetCacheDirectory() / QT_NT("Webkit"));
		create_directory(cacheDir);
		QString cachePath = native(cacheDir);
		_widget->settings()->setOfflineStoragePath(cachePath);
		_widget->settings()->setIconDatabasePath(cachePath);

		// connect all the update signals
		connect(_widget->page(), SIGNAL(selectionChanged()), this, SLOT(pageRepainted()));
		connect(_widget->page(), SIGNAL(contentsChanged()), this, SLOT(pageRepainted()));
		connect(_widget->page(), SIGNAL(scrollRequested(int, int, const QRect&)), this, SLOT(scrollRequested(int, int, const QRect&)));
		connect(_widget->page(), SIGNAL(geometryChangeRequested(const QRect&)), this, SLOT(geometryChangeRequested(const QRect&)));
		connect(_widget->page(), SIGNAL(loadStarted()), this, SLOT(pageLoadStarted()));	
		connect(_widget->page(), SIGNAL(loadProgress(int)), this, SLOT(pageLoading(int)));	
		connect(_widget->page(), SIGNAL(loadFinished(bool)), this, SLOT(pageLoaded(bool)));	
		// connect(_widget->page(), SIGNAL(linkHovered(const QString&, const QString&, const QString&)), this, SLOT(pageLinkHovered(const QString&, const QString&, const QString&)));
		if (autoUpdate)
			connect(_widget->page(), SIGNAL(repaintRequested(const QRect&)), this, SLOT(pageUpdated(const QRect&)));
		connect(_widget->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(javaScriptWindowObjectCleared()));
				
		// setup the cookies and what not
		NetworkAccessManager * nam = new NetworkAccessManager(this, Singleton<NetworkCookieJar>::getInstance());		
		nam->setCache(NetworkDiskCache->cache());		

		NetworkDiskCache->cache()->setParent(NULL);
		Singleton<NetworkCookieJar>::getInstance()->setParent(NULL); //Set parent to NULL so that the shared cookie jar wont be deleted when one NetworkAccessManager is deleted
		// nam->setProxy(QNetworkProxy(QNetworkProxy::HttpProxy, 
		_widget->page()->setNetworkAccessManager(nam);
	}
	setContentSize(contentSize, contentSize);
}

void WebPage::setContentSize(const QSize & contentSize, const QSize & bufferSize)
{
	_contentSize = contentSize;
	_bufferSize = bufferSize;

	_widget->page()->setViewportSize(_contentSize);

	// create the new buffers (image/temporary)
	_buffer = QImage(_bufferSize, QImage::Format_ARGB32_Premultiplied);
	invalidateBuffer();
}

void WebPage::onUpdate()
{
	if (!_loaded || !_isDirty || _isExternal)
		return;

	QPainter p;
	p.begin(&_buffer);	
		/* Test transparency
		p.fillRect(QRect(QPoint(0, 0), _contentSize), Qt::transparent);
		*/
		if (_isBufferDirty)
			_dirtyRect = _widget->page()->mainFrame()->geometry();
		p.setClipRect(_dirtyRect);
		_widget->page()->mainFrame()->render(&p);
	p.end();

	// reset the flags
	_isDirty = false;
	_isBufferDirty = false;

	emit pageUpdate(_buffer, _dirtyRect);
	_dirtyRect = QRect(); // Reset the rect
}

const QImage & WebPage::getBuffer() const
{
	return _buffer;
}

const QRect & WebPage::getDirtyRect() const
{
	return _dirtyRect;
}

bool WebPage::isLoaded() const
{
	return _loaded;
}

bool WebPage::wantsBumpTopMouseEvents() const 
{
	return _wantBumpTopMouseEvents;
}

void WebPage::pageLoadStarted()
{
	if (_enableProgressNotifications)
		printTimedUnique("WebPage::pageLoading", 30, QT_TR_NOOP("Loading (0%)"));
}

void WebPage::pageLoading(int progress)
{
	if (!_widget)
		return;

	if (_enableProgressNotifications)
	{
		if (scnManager->messages()->hasMessage("WebPage::pageLoading"))
			printTimedUnique("WebPage::pageLoading", 10, QT_TR_NOOP("Loading (%1%)").arg(progress));
	}
}

void WebPage::pageLoaded(bool)
{
	if (!_widget)
		return;

	if (_enableProgressNotifications)
	{
		dismiss("WebPage::pageLoading");
	}

	// Try to determine the ideal size of the widget.
	// First, check if it's a Google Gadget. If not, see if it's a BumpTop widget.
	QString jsCode(QT_NT(
		"(function () {"
		"	var desiredSize = null;"
		"	try {"
		"		var tables = document.getElementsByClassName('gadget');"
		"		if (1 == tables.length) {"
		"			var googleGadgetBody = tables[0];"
		"			tables[0].style.borderSpacing = '0';"
		"			var tableBody = tables[0].childNodes;"
		"			if (tableBody.length > 0) {"
		"				var tableRows = tableBody[0].childNodes;"
		"				if (tableRows.length == 4) {"
		"					document.body.style.margin = '0';"
		"					desiredSize = { width: googleGadgetBody.offsetWidth, height: googleGadgetBody.offsetHeight }; "
		"				}"
		"			}"
		"		}"
		"	} catch (err) {};"

		"	if (desiredSize === null) {"
		"		try {"
		"			var elements = document.getElementsByTagName('body')[0].getElementsByClassName('bumpwidget');"
		"			if (elements.length == 1 && elements[0].nodeName == 'DIV') {"
		"				var style = getComputedStyle(elements[0]);"
		"				desiredSize = { width: style.getPropertyValue('width'), height: style.getPropertyValue('height') };"
		"			}"
		"		} catch (err) {};"
		"	}"
		"	return (desiredSize === null) ? { width: 0, height: 0 } : desiredSize;"
		"}());"));

	QVariant jsResult = _widget->page()->mainFrame()->evaluateJavaScript(jsCode);
	QVariantMap jsObj = qVariantValue<QVariantMap>(jsResult);

	assert(jsObj.contains("width") && jsObj.contains("height"));

	QString widthStr = jsObj.value("width").toString();
	QString heightStr = jsObj.value("height").toString();

	if (widthStr.endsWith("px")) widthStr.chop(2);
	if (heightStr.endsWith("px")) heightStr.chop(2);

	int width = widthStr.toInt();
	int height = heightStr.toInt();
	if (width > 0 && height > 0)
	{
		_contentSize.setWidth(width);
		_contentSize.setHeight(height);
		_requestedSize = QSize(width, height);
	}
	else
		_requestedSize = QSize(0, 0);

	QVariant disableMouseEvents = evaluateJavaScript("(window.disableBumpTopMouseEvents == true)");
	_wantBumpTopMouseEvents = !disableMouseEvents.toBool();

	_widget->page()->setViewportSize(_contentSize);
	// in case we want to bump up the default text size later:
	// _widget->page()->mainFrame()->setTextSizeMultiplier(1.5f);

	// also scale the widget if we are focused
	if (width > 0 && height > 0) {
		QRect dims = _widget->geometry();
		_widget->move(dims.left() + (dims.width() - width) / 2,
			dims.top() + (dims.height() - height) / 2);
		_widget->setFixedSize(width, height);
	}

	// flag for next update
	_loaded = true;

	// If we have queued javascript commands we want them to be executed when this page 
	// is loaded
	if (_executeOnLoaded.size() && _executeCode)
	{
		for (uint i = 0; i < _executeOnLoaded.size(); i++)
			evaluateJavaScript(_executeOnLoaded[i]);
		_executeOnLoaded.clear();
		_executeCode = false;
	}
	// we are going to invalidate this page, so that the next update call will
	// fire the pageUpdate() signal
	invalidateBuffer();
}

void WebPage::addExecuteCode(QString code) {
	_executeOnLoaded.push_back(code);
}

void WebPage::pageUpdated( const QRect& newDirtyRect )
{
	if (!_loaded)
		return;

	// we are going to invalidate this page, so that the next update call will
	// fire the pageUpdate() signal

	// flag for next update
	_isDirty = true;
	_dirtyRect = _dirtyRect.united(newDirtyRect); // Calculate union
}

// A generic slot for handling several different signals from the underlying page
void WebPage::pageRepainted()
{
	if (_loaded)
		invalidateBuffer();
}

void WebPage::geometryChangeRequested(const QRect& rect)
{
	// Ignore the arg and just repaint the whole page
	if (_loaded)
		invalidateBuffer();
}

void WebPage::scrollRequested(int, int, const QRect&)
{
	// Ignore the arg and just repaint the whole page
	if (_loaded)
		invalidateBuffer();
}

void WebPage::pageLinkHovered( const QString& link, const QString& title, const QString& text )
{
	/*
	if (link.isEmpty())
		SetCursor(LoadCursor(NULL, IDC_ARROW));
	else
		SetCursor(LoadCursor(NULL, IDC_HAND));
	*/
}

QString WebPage::getTitle() const
{
	return _widget->page()->mainFrame()->title();
}

QSize WebPage::getContentSize() const
{
	return _contentSize;
}

QSize WebPage::getRequestedSize() const
{
	return _requestedSize;
}

QWebView * WebPage::getView() const
{
	return _widget;
}

bool WebPage::isExternal()
{
	return _isExternal;
}

void WebPage::setIsExternal(bool external)
{
	_isExternal = external;
	if (_isExternal)
	{
		_widget->setFixedSize(_contentSize);
		_widget->setWindowFlags(Qt::FramelessWindowHint);
		_widget->show();

		// Make WebView child of BumpTop to hide it in task bar, and reposition it to center of BumpTop
		PWND pwnd = WindowFromDC(_widget->getDC());
		SetParent(pwnd, winOS->GetWindowsHandle());
		SetWindowPos(pwnd, HWND_TOP, (winOS->GetWindowWidth() - _contentSize.width()) / 2, (winOS->GetWindowHeight() - _contentSize.height()) / 2, 0, 0, SWP_NOSIZE);
		// To disable Windows built-in gestures
		winOS->GetWindows7Multitouch()->registerTouchWindow(pwnd);
	}
	else
	{
		_widget->hide();
	}
}

void WebPage::hideScrollBars()
{
	_widget->page()->mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
	_widget->page()->mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);
}

void WebPage::invalidateBuffer()
{
	_isBufferDirty = _isDirty = true;
}

// This is called whenever the javascript Window object is cleared, 
// e.g. before starting a new page load.
// NOTE: By design, subsequent page loads will not automatically get the API.
void WebPage::javaScriptWindowObjectCleared()
{
	if (_apiForNextPageLoad)
	{
		// Add a variable named "BumpTop" to the JavaScript window object
		_widget->page()->mainFrame()->addToJavaScriptWindowObject(_apiForNextPageLoad->javaScriptObjectName, _apiForNextPageLoad);
		if (!_apiForNextPageLoad->isPersistent())
			_apiForNextPageLoad = NULL;
	}
}

void WebPage::setProgressVisible(bool enable)
{
	_enableProgressNotifications = enable;
	if (!enable)
		dismiss("WebPage::pageLoading");
}

// Evaluate the given JavaScript on the current page
// Use this with care!
QVariant WebPage::evaluateJavaScript( const QString &javascriptCode )
{
	return _widget->page()->mainFrame()->evaluateJavaScript(javascriptCode);
}

// We must override this method to see errors from JS
void ConsoleLoggingQWebPage::javaScriptConsoleMessage( const QString & message, int lineNumber, const QString & sourceID )
{
	consoleWrite(QString("JS> %1 (%2:%3)\n").arg(message).arg(sourceID).arg(lineNumber));
}

void ConsoleLoggingQWebPage::javaScriptAlert(QWebFrame * frame, const QString & msg)
{	
	QString key = QString::number(timeGetTime());
	scnManager->messages()->addMessage(new Message(key, msg, Message::Ok)); 
}

void ConsoleLoggingQWebPage::triggerAction(QWebPage::WebAction action, bool checked)
{
	QWebPage::triggerAction(action);
	_lastAction = action;
}

QString ConsoleLoggingQWebPage::userAgentForUrl(const QUrl & url) const
{
	switch (GLOBAL(settings).userAgent)
	{
	case iPhoneUserAgent:		
		return "Mozilla/5.0 (iPhone; U; CPU iPhone OS 2_1 like Mac OS X; en-us) AppleWebKit/525.18.1 (KHTML, like Gecko) Version/3.1.1 Mobile/5F136 Safari/525.20";
	case IE8UserAgent:
		return "Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 5.1; FBSMTWB; InfoPath.2; .NET CLR 1.1.4322; .NET CLR 2.0.50727; .NET CLR 3.0.4506.2152; .NET CLR 3.5.30729; MS-RTC LM 8)";
	default: break;
	}
	return QWebPage::userAgentForUrl(url);
}

bool ConsoleLoggingQWebPage::testAndClearIfActionIs(QWebPage::WebAction action)
{
	bool isAction = (_lastAction == action);
	_lastAction = QWebPage::NoWebAction;
	return isAction;
}

WebCache::WebCache()
{
	_cache = new QNetworkDiskCache();
	QString dir = winOS->GetCacheDirectory().absolutePath() + QT_NT("\\WebKit\\DiskCache");
	_cache->setCacheDirectory(dir);	
}

QNetworkDiskCache* WebCache::cache()
{
	return _cache;
}
#endif


