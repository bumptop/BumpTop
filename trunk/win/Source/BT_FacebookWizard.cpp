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
#include "BT_FacebookWizard.h"
#include "BT_FileTransferManager.h"
#include "BT_JavaScriptAPI.h"
#include "BT_QtUtil.h"
#include "BT_SpinnerWidget.h"
#include "BT_WindowsOS.h"
#include "moc/moc_BT_FacebookWizard.cpp"

FacebookWizard::FacebookWizard(const QString& loginUrl, const QString& successUrl, const QString& failureUrl, JavaScriptAPI * jsApiRef)
: QDialog(0)
, _jsApi(jsApiRef)
, _loginUrl(loginUrl)
, _successUrl(successUrl)
, _failureUrl(failureUrl)
{
	// setup dialog
	resize(800, 600);

	// setup the web view
	_webView = new QWebView(this);

	// setup spinner
	_spinner = new SpinnerWidget(QT_NT(native(make_file(winOS->GetTexturesDirectory(), "throbber.gif"))), this);

	connect(_webView, SIGNAL(urlChanged(const QUrl &)), this, SLOT(urlChanged(const QUrl &)));
	connect(_webView, SIGNAL(loadFinished(bool)), this, SLOT(loadFinished(bool)));
	connect(_webView->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(javaScriptWindowObjectCleared()));
	connect(_webView, SIGNAL(loadStarted()), this, SLOT(loadStarted()));
}

QString FacebookWizard::loginExec()
{
	_webView->load(getDisplayUrl());
	int result = exec();
	if (result)
		return _result;
	return QString();
}

QUrl FacebookWizard::getDisplayUrl()
{
	if(ftManager->hasInternetConnection(_loginUrl))
		return QUrl(_loginUrl);
	else
	{
		// Display a page to the user notifying of them of no connection
		QDir facebookWidgetDir(::parent(winOS->GetLanguagesDirectory()) / "Facebook");
		QString dialogPath = facebookWidgetDir.absoluteFilePath("facebook_no_connection.html");
		assert(QFileInfo(dialogPath).exists());
		return QUrl::fromLocalFile(dialogPath);
	}
}

void FacebookWizard::loadFinished(bool)
{
	// we do some extra handling to show the login fields automatically
	if (_webView->url().toString().startsWith(_loginUrl, Qt::CaseInsensitive)) {
		if (_webView->page() && _webView->page()->mainFrame())
			_webView->page()->mainFrame()->evaluateJavaScript("FBConnectLoginBootstrap.showRegularLoginUI();");
	}
	_spinner->hide();
}

void FacebookWizard::loadStarted()
{
	_spinner->show();
}

void FacebookWizard::urlChanged(const QUrl & url)
{
	QString urlStr = url.toString();
	if (urlStr.startsWith(_successUrl, Qt::CaseInsensitive)) 
	{
		_result = url.queryItemValue("session");
		accept(); // success
	}
	else if (urlStr.startsWith(_failureUrl, Qt::CaseInsensitive)) 
		reject(); // cancel
	else if (urlStr.startsWith(_loginUrl, Qt::CaseInsensitive))
	{
		// ignore
	}
	else
	{
		// some other page? ie. signup
		// TODO: fix
	}
}

// Add class object to javascript
void FacebookWizard::javaScriptWindowObjectCleared()
{
	((QWebFrame *)sender())->addToJavaScriptWindowObject(QT_NT("BumpTopNative"), _jsApi);
	((QWebFrame *)sender())->addToJavaScriptWindowObject(QT_NT("BumpTopNativeNoConnectionWizard"), this);	
}

void FacebookWizard::reloadLoginPage()
{
	_webView->load(getDisplayUrl());
}

void FacebookWizard::closeWindow()
{
	close();
}