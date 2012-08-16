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

#ifndef BT_WEBPAGE
#define BT_WEBPAGE

#ifdef ENABLE_WEBKIT

class JavaScriptAPI;

// In order to log JavaScript errors, we have to subclass QWebPage and
// override javaScriptConsoleMessage.
class ConsoleLoggingQWebPage : public QWebPage
{
	QWebPage::WebAction _lastAction;

protected:
	virtual void javaScriptConsoleMessage(const QString & message, int lineNumber, const QString & sourceID);
	virtual void javaScriptAlert(QWebFrame * frame, const QString & msg);
	virtual void triggerAction(QWebPage::WebAction action, bool checked = false);
	virtual QString userAgentForUrl(const QUrl & url) const;
public:
	ConsoleLoggingQWebPage() : QWebPage(NULL), _lastAction(QWebPage::Forward) {};
	ConsoleLoggingQWebPage(QWidget *parent) : QWebPage(parent), _lastAction(QWebPage::Forward) {};

	bool testAndClearIfActionIs(QWebPage::WebAction action);
};

class WebView;

class WebPage : public QObject
{
	Q_OBJECT;
	Q_DECLARE_TR_CONTEXT(WebPage);

	bool _loaded;			// whether the page has loaded yet
	QStringList _executeOnLoaded;
	bool _isDirty;			// whether the page has been updated	
	bool _isBufferDirty;	// whether the next update should invalidate the whole buffer
	bool _enableProgressNotifications;	// whether to show progress
	bool _isExternal;		// whether QWebView is shown, which does event processing and rendering etc.
	QRect _dirtyRect;
	WebView * _widget;
	ConsoleLoggingQWebPage _consoleLoggingQWebPage;
	QSize _bufferSize; // Should be _contentSize or bigger if gfx card requires POT
	QSize _contentSize; // The expected full screen size of page, matches aspect ratio or square & power-of-2
	QSize _requestedSize; // If not zero, web actor will resize to _requestedSize, else it will be almost full window
	QImage _buffer;
	JavaScriptAPI *_apiForNextPageLoad;
	bool _wantBumpTopMouseEvents;
	bool _executeCode;

public:
	WebPage();
	virtual ~WebPage();

	void init(const QSize& contentSize, bool autoUpdate=false);
	void setContentSize(const QSize & contentSize, const QSize & bufferSize);
	void load(const QString& url, JavaScriptAPI* api=NULL, bool executeQueuedCodeOnThisPage = false);
	void loadHTML(const QString& html);
	void hideScrollBars();
	void invalidateBuffer();
	void setProgressVisible(bool enable);
	const QImage & getBuffer() const;
	const QRect & getDirtyRect() const;
	bool isLoaded() const;
	bool wantsBumpTopMouseEvents() const;
	void addExecuteCode(QString code);

	QString getTitle() const;
	QSize getContentSize() const;
	QSize getRequestedSize() const;
	QWebView * getView() const;

	bool isExternal();
	void setIsExternal(bool external);

	QVariant evaluateJavaScript(const QString &javascriptCode);

	// update
	void onUpdate();

private slots:
	void pageLoadStarted();
	void pageLoading(int);
	void pageLoaded(bool);
	void pageUpdated(const QRect&);
	void pageRepainted();
	void geometryChangeRequested(const QRect&);
	void scrollRequested(int, int, const QRect&);
	void pageLinkHovered(const QString&, const QString&, const QString&); 
	void javaScriptWindowObjectCleared();

signals:
	void pageUpdate(const QImage&, const QRect&);
};

class WebCache : public QObject
{
	Q_OBJECT

private:
	friend class Singleton<WebCache>;
	WebCache();
	QNetworkDiskCache *_cache;

public:
	
	QNetworkDiskCache* cache();
};



#define NetworkDiskCache Singleton<WebCache>::getInstance()

#endif
#endif
