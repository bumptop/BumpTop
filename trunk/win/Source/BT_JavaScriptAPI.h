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

#ifndef _BT_JAVASCRIPTAPI_H_
#define _BT_JAVASCRIPTAPI_H_

#include "BT_Common.h"
#include "BT_BumpObject.h"
#include "BT_Pile.h"
#include "BT_FileTransferManager.h"
#include "BT_WebActor.h"
#include "BT_WebPage.h"

#if ENABLE_WEBKIT

// These objects provide the API that is available in the web page.
//
// The types we can actually return are quite limited. Here's what seems to
// work, with C++ => JavaScript mapping:
//     QVariantMap => Object
//     int => int
//     QString => QString
// QVariantList maps to an Array in JavaScript, however as soon as the JS
// env is reset (e.g. when the page is reloaded), we get a memory fault.
// The workaround is to put the results in an object, like this:
//     { "1": <result_1>, "2": <result_2> }
// and when we iterate over them in JavaScript, it seems to automatically
// happen in the correct order.

class JavaScriptAPI : 
	public QObject, 
	public FileTransferEventHandler // For HTTP Post notifications
{
	Q_OBJECT

	QMap<int, QFile *> _fileMap;
	QDialog * _dialog;
	QVariantMap _dialogBoxResult;
	QVariantMap _dialogUserData;
	QString _dragAndDropData;
	bool _isPersistent;

	QHttp _httpImpl;
	QHash<int,QString> _pendingHttpRequests;

	bool injectDragAndDropEvent(QString eventName);
	void downloadFile(QUrl &url, QString callbackName=QString::null, QString desiredFilename=QString::null);
	void transferFinished(const FileTransfer& transfer, bool success);
	void buildUploadString(QString fullFilePath, QString& stringToBuild);

protected:	
	WebActor *_actor;
	WebPage *_page;
	QString _stringData;

	// Maintain a two-way mapping between filedata: URLs and the absolute path
	// Given a filedata URL, we can determine the absolute path to the file;
	// and we can also ensure that we don't create multiple URLs for the same file.
	QHash<QString,QString> _fileDataUrlsToPaths;
	QHash<QString,QString> _fileDataPathsToUrls;

public:
	JavaScriptAPI(WebActor *actor, WebPage *page, bool persistent);
	virtual ~JavaScriptAPI();

	const QString javaScriptObjectName;

	bool isPersistent() const;

	// Callbacks for HTTP Post uploads
	virtual void onTransferComplete(const FileTransfer& transfer);
	virtual void onTransferError(const FileTransfer& transfer);

	virtual bool onDropEnter(vector<BumpObject *> &objList);
	virtual bool onDrop();

	virtual void onWidgetFocus();
	virtual void onWidgetBlur();

private slots:
	void httpRequestStarted(int id);
	void httpRequestFinished(int id, bool error);
	void httpDataReadProgress(int done, int total);
	void httpResponseHeaderReceived(const QHttpResponseHeader& resp);
	void httpReadyRead(const QHttpResponseHeader& resp); 
	void httpStateChanged(int state); 
	void dialogJavaScriptWindowObjectCleared();

	void downloadProgress( qint64 received, qint64 total );
	void downloadFinished();
	void downloadError(QNetworkReply::NetworkError);

public slots:
	// All public slots will be exposed as methods in javascript

	QString internal_getStringData();

	QString translate(const QString& key);
	QString translateFormat(const QString& key, const QStringList& arguments);

	// For debugging purposes
	bool isDebugEnabled();
	void hard_assert(bool javaScriptExpressionValue, QString message=QString());

	// Methods for working with dialogs
	QVariantMap openDialogWindow(QString dialogBoxName, QVariantMap dialogUserData = QVariantMap());
	QVariantMap getDialogUserData();
	void setDialogBoxResult(QVariantMap dialogResult);
	void closeDialogWindow();

	// JavaScript file access API
	int openFile(QString fileName, QString mode);
	QString readFile(int fileHandle);
	bool writeFile(int fileHandle, QString contents);
	bool renameAndCloseFile(int fileHandle, QString newFileName);
	bool renameFile(QString oldFileName, QString newFileName);
	void closeFile(int fileHandle);

	bool launch(QString fullPathOrUrl, QString callbackName=QString::null, QString desiredFilename=QString::null);
	void sendXmlHttpRequest(QString method, QString urlString, QString callbackName, QString data=QString(), QVariantMap customHeaders=QVariantMap());
	void sendHttpPostFileUpload(QString urlString, QVariantMap opts, QVariantMap fileParams, QString callbackName);
	void uploadProgress(qint64 bytesSent, qint64 bytesTotal);
	void abortAllUploads();

	void openInBrowser(QString url);
	QString openFileBrowser();

	void copyTextToClipboard(QString text);

	virtual void deleteFlyout();
};

// ---------------------------------------------------------------------------

class SharedFolderAPI : public JavaScriptAPI
{
	Q_OBJECT
	QString _dirPath;

public:
	SharedFolderAPI(WebActor *actor, WebPage *page);

	void uploadFiles(QString files);

public slots:
	// All public slots will be exposed as methods in javascript

	void showFlyout(QVariantMap data /*= QVariantMap()*/);
	void deleteFlyout();
};

#endif /* ENABLE_WEBKIT */
#endif /* _BT_JAVASCRIPTAPI_H_ */
