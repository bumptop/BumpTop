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

#pragma once

class SpinnerWidget;
class JavaScriptAPI;

class FacebookWizard : QDialog
{
	Q_OBJECT

	QWebView * _webView;
	QString _loginUrl;
	QString _successUrl;
	QString _failureUrl;
	QString _result;
	SpinnerWidget * _spinner;
	JavaScriptAPI * _jsApi;
	
private:
	QUrl getDisplayUrl();

public:
	FacebookWizard(const QString& loginUrl, const QString& successUrl, const QString& failureUrl, JavaScriptAPI * jsApiRef);

	// returns empty string if fails
	QString loginExec();

public slots:
	void urlChanged(const QUrl& url);
	void loadFinished(bool);
	void loadStarted();
	void reloadLoginPage();
	void javaScriptWindowObjectCleared();
	void closeWindow();
};
