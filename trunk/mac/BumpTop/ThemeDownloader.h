/*
 *  Copyright 2012 Google Inc. All Rights Reserved.
 *  
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  
 *      http://www.apache.org/licenses/LICENSE-2.0
 *  
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef BUMPTOP_THEMEDOWNLOADER_H_
#define BUMPTOP_THEMEDOWNLOADER_H_

class QLabel;
class QProgressBar;
class QWizard;
class QWebView;

#include <QtGui/QWizard>
#include <QtGui/QProgressBar>
#include <QtGui/QLabel>
#include <QtGui/QVBoxLayout>
#include <QtWebKit/QWebView>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

class ThemeDownloader : public QObject {
  Q_OBJECT
 public:

  explicit ThemeDownloader();
  ~ThemeDownloader();

  void init();
  void launch(); // return non empty string if user chooses to enter URL or HTML, else empty

 protected:
  QWizard* wizard_;

  QWebView* web_view_;
  QProgressBar * progress_bar_;
  QLabel *status_;

  QList<QWizard::WizardButton> * button_layout_;	
  QString text_; // User entered URL or HTML through advanced dialog
  QString default_status_message_;

  void downloadFile(const QUrl & url);

 protected slots:
  void customButtonClicked(int which);
  void loadStarted();
  void loadProgress(int progress);
  void loadFinished(bool ok);
  void downloadRequested(const QNetworkRequest & request);
  void unsupportedContent(QNetworkReply * reply);
  void linkClicked(const QUrl & url);
  void downloadProgress(qint64 received, qint64 total);
  void finished();
  void exitDialog(int result);
};

#endif  // BUMPTOP_THEMEDOWNLOADER_H_