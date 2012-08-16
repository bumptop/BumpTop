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
#include "BT_SceneManager.h"
#include "BT_FileSystemActor.h"
#include "BT_FileSystemActorFactory.h"
#include "BT_FileSystemManager.h"
#include "BT_Flyout.h"
#include "BT_Util.h"
#include "BT_WebPage.h" // For ConsoleLoggingQWebPage
#include "BT_WindowsOS.h"
#include "BT_SharingFlyout.h"
#include "BT_OverlayComponent.h"

#include "BT_JavaScriptAPI.h"
#include "moc/moc_BT_JavaScriptAPI.cpp"

JavaScriptAPI::JavaScriptAPI(WebActor *actor, WebPage *page, bool persistent)
: _actor(actor)
, _page(page)
, _stringData()
, _httpImpl()
, javaScriptObjectName("BumpTopNative")
, _isPersistent(persistent)
{}

JavaScriptAPI::~JavaScriptAPI()
{
	abortAllUploads();
	_httpImpl.disconnect(this);
	_page->disconnect(this);
}

bool JavaScriptAPI::isDebugEnabled()
{
#ifdef DEBUG
	return true;
#else
	return false;
#endif
}

bool JavaScriptAPI::isPersistent() const
{
	return _isPersistent;
}

// Allow JS code to trigger a hard assertion in C++
void JavaScriptAPI::hard_assert( bool javaScriptExpressionValue, QString message/*=QString()*/ )
{
#ifdef DEBUG
	if (!javaScriptExpressionValue)
	{
		consoleWrite(QString("JS assertion failure: ") + message);
		assert(javaScriptExpressionValue);
	}
#endif
}

// Helper function for downloading a file -- will follow redirects if applicable.
// If the desired filename is null, it will be determined from the URL.
// desiredFilename must be ONLY a filename, not an absolute or relative path.
void JavaScriptAPI::downloadFile(QUrl &url, QString callbackName/*=QString::null*/, QString desiredFilename/*=QString::null*/)
{
	QString filename = desiredFilename.isNull() ? QFileInfo(url.path()).fileName() : desiredFilename;

	QNetworkAccessManager *manager = _page->getView()->page()->networkAccessManager();
	QNetworkRequest request(url);
	request.setAttribute(QNetworkRequest::User, QVariant(callbackName));
	request.setAttribute(QNetworkRequest::UserMax, QVariant(filename));
	QNetworkReply *reply = manager->get(request);
	connect(reply, SIGNAL(finished()), this, SLOT(downloadFinished()));
//	connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(downloadProgress(qint64,qint64)));
	connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(downloadError(QNetworkReply::NetworkError)));
}

// Launch a file. fullHandle is either a URL or a filename. If it is a URL,
// it will be downloaded to a temp dir first.
bool JavaScriptAPI::launch(QString fullHandle, QString callbackName/*=QString::null*/, QString fileName/*=QString::null*/)
{
	bool result = false;

	// First, see if it looks like a URL
	QUrl url(fullHandle, QUrl::StrictMode);
	if (url.scheme().startsWith("http"))
	{
		downloadFile(url, callbackName, fileName);
		return true;
	}
	// Otherwise, assume it's a local file
	return fsManager->launchFile(fullHandle);
}

// This slot is connected to a QNetworkReply, probably started by downloadFile()
void JavaScriptAPI::downloadProgress( qint64 received, qint64 total )
{
	// TODO: Do something useful to indicate progress 
	consoleWrite(QString("Download %1/%2\n").arg(received).arg(total));
}

// This slot is connected to a QNetworkReply, probably started by downloadFile()
void JavaScriptAPI::downloadFinished()
{
	QNetworkReply *reply = (QNetworkReply *)sender();
	QString callbackName = reply->request().attribute(QNetworkRequest::User).toString();
	QString origFilename = reply->request().attribute(QNetworkRequest::UserMax).toString();
	QUrl redirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
	if (redirectUrl.isEmpty())
	{
		QDir downloadDir = QDir::temp();
		downloadDir.mkdir("BumpTop Downloads");
		downloadDir.cd("BumpTop Downloads");

		// Save the download to a temp file
		QFileInfo fileInfo(origFilename);
		QString filename = fileInfo.baseName() + "_XXXXXX." + fileInfo.completeSuffix();
		QTemporaryFile destFile(downloadDir.absoluteFilePath(filename));
		destFile.setAutoRemove(false);
		if (destFile.open())
		{
			destFile.write(reply->readAll());
			destFile.close();
		}
		// Try to rename it to the original filename. Might fail, but that's ok.
		destFile.rename(downloadDir.absoluteFilePath(origFilename));
		destFile.setPermissions(QFile::ReadUser); // Make the file read-only
		fsManager->launchFile(destFile.fileName());
		if (!callbackName.isNull())
			_page->evaluateJavaScript(callbackName + "(true);");
	}
	else
	{
		// Follow the redirect
		downloadFile(redirectUrl, callbackName, origFilename);
	}
}

// This slot is connected to a QNetworkReply, probably started by downloadFile()
void JavaScriptAPI::downloadError( QNetworkReply::NetworkError error)
{
	consoleWrite(QString("Download error!\n"));
	QNetworkReply *reply = (QNetworkReply *)sender();
	QString callbackName = reply->request().attribute(QNetworkRequest::User).toString();
	if (!callbackName.isNull())
		_page->evaluateJavaScript(callbackName + "(false);");
}

// Upload one or more files using an HTTP post request.
// 'opts' is a JS object where the keys and values represent the non-file parameters of the POST request.
// 'fileParams' is a JS object of the files to upload, where the key is the name of the parameter
// and the value is a filedata: URL referring to the file.
// 'callbackName' is the name of a JS function in the global scope that will be called when
// the operation is complete, with the args (data, success)
void JavaScriptAPI::sendHttpPostFileUpload(QString urlString, QVariantMap opts, QVariantMap fileParams, QString callbackName)
{
	QUrl url(urlString);
	assert(url.scheme() == "http" || url.scheme() == "https");

	QSet<QString> fileParamKeys;
	QHash<QString, QString> params;
	for_each(QString key, fileParams.keys())
	{
		fileParamKeys.insert(key);
		QString fileDataUrl = fileParams[key].toString();
		assert(_fileDataUrlsToPaths.contains(fileDataUrl));
		params.insert(key, _fileDataUrlsToPaths[fileDataUrl]);
	}
	for_each(QString key, opts.keys())
	{
		params.insert(key, opts[key].toString());
	}

	FileTransfer ft(FileTransfer::Upload, this);
	ft.setUrl(urlString);
	ft.setParams(params);
	ft.setFileParamKeys(fileParamKeys);
	ft.setTemporaryString();
	ft.setTimeout(60);
	ft.setUserData(new QString(callbackName));
	bool succeeded = ftManager->addPostTransfer(ft, false, this);
	if (!succeeded)
		onTransferError(ft);
}

// Progress notifications for sendHttpPostFileUpload()
void JavaScriptAPI::uploadProgress(qint64 bytesSent, qint64 bytesTotal)
{
	// get the filename associated with this upload progress by checking who sent the signal
	QNetworkReply* signalSender = (QNetworkReply*)QObject::sender();
	QString fileNameAttr = signalSender->request().attribute(QNetworkRequest::User).toString();

	_page->evaluateJavaScript(QString_NT("updateProgress(\"%1\", %2, %3);").arg(fileNameAttr).arg(bytesSent).arg(bytesTotal));
}

// terminate any uploads in progress
void JavaScriptAPI::abortAllUploads()
{
	ftManager->removeTransfersToHandler(this);
}

// Helper function for transfers started with sendHttpPostFileUpload()
void JavaScriptAPI::transferFinished(const FileTransfer& transfer, bool success)
{
	QString *callbackName = (QString *)transfer.getUserData();
	assert(_stringData.isNull());
	_stringData = transfer.getTemporaryString();
	_page->evaluateJavaScript(QString("%1(BumpTopNative.internal_getStringData(), %2);")
		.arg(*callbackName).arg(success ? "true" : "false"));
	delete callbackName;
}

// Callback for transfers that were enqueued with the FileTransferManager
void JavaScriptAPI::onTransferComplete( const FileTransfer& transfer )
{
	transferFinished(transfer, true);
}

// Callback for transfers that were enqueued with the FileTransferManager
void JavaScriptAPI::onTransferError( const FileTransfer& transfer )
{
	transferFinished(transfer, false);
}

// It is assumed that the URL string is properly percent encoded
void JavaScriptAPI::sendXmlHttpRequest(QString method, QString urlString, QString callbackName, QString data/*=QString()*/, QVariantMap customHeaders/*=QVariantMap()*/)
{
	QUrl url(urlString);
	assert(url.scheme() == "http" || url.scheme() == "https");

	int port = url.port();
	if (port == -1)
		port = (url.scheme() == "http") ? 80 : 443; 

	_httpImpl.setHost(url.host(), port);
	connect(&_httpImpl, SIGNAL(requestFinished(int, bool)), this, SLOT(httpRequestFinished(int, bool)));

	// These signals are helpful for debugging
//	connect(&_httpImpl, SIGNAL(requestStarted(int)), this, SLOT(httpRequestStarted(int)));
//	connect(&_httpImpl, SIGNAL(readyRead(const QHttpResponseHeader&)), this, SLOT(httpReadyRead(const QHttpResponseHeader&)));
//	connect(&_httpImpl, SIGNAL(responseHeaderReceived(const QHttpResponseHeader&)), this, SLOT(httpResponseHeaderReceived(const QHttpResponseHeader&)));
//	connect(&_httpImpl, SIGNAL(dataReadProgress(int,int)), this, SLOT(httpDataReadProgress(int,int)));
//	connect(&_httpImpl, SIGNAL(stateChanged(int)), this, SLOT(httpStateChanged(int)));

	QString path = url.toString(QUrl::RemoveAuthority|QUrl::RemoveScheme);

	QHttpRequestHeader header(method, path.toUtf8());
	header.addValue("Host", url.host());

	// Set the custom headers
	for_each(QString key, customHeaders.keys())
	{
		header.setValue(key, customHeaders[key].toString());
	}

	int id = _httpImpl.request(header, data.toUtf8());
	_pendingHttpRequests.insert(id, callbackName);
}

// This slot is triggered from an HTTP request in sendXmlHttpRequest().
// Not connected by default, but left here because it's useful for debugging.
void JavaScriptAPI::httpRequestStarted( int id )
{
	if (_pendingHttpRequests.contains(id))
	{
		QHttpRequestHeader header = _httpImpl.currentRequest();
		consoleWrite(QString("Starting HTTP request --\n"));
		for_each(QString key, header.keys())
		{
			consoleWrite(QString("    %1: %2\n").arg(key).arg(header.value(key)));
		}
	}
}

// This slot is triggered from an HTTP request in sendXmlHttpRequest().
void JavaScriptAPI::httpRequestFinished( int id, bool error )
{
	if (_pendingHttpRequests.contains(id))
	{
		assert(_stringData.isNull());
		_stringData = _httpImpl.readAll();

		// If we close the connection here, we don't seem to be able to make any
		// further requests with this object.
//		_httpImpl.close();

		QString callbackCode = QString("%1(BumpTopNative.internal_getStringData(), %2);")
			.arg(_pendingHttpRequests[id])
			.arg(error ? "false" : "true");
		_page->evaluateJavaScript(callbackCode);

		// Remove the object from the list, and let Qt delete it when signal processing is done
		_pendingHttpRequests.remove(id);
	}
}

// This slot is triggered from an HTTP request in sendXmlHttpRequest().
// Not connected by default, but left here because it's useful for debugging.
void JavaScriptAPI::httpDataReadProgress( int done, int total )
{
	consoleWrite(QString("httpDataReadProgress, %1 done, %2 total\n").arg(done).arg(total)); 
}

// This slot is triggered from an HTTP request in sendXmlHttpRequest().
// Not connected by default, but left here because it's useful for debugging.
void JavaScriptAPI::httpResponseHeaderReceived(const QHttpResponseHeader& resp)
{
	consoleWrite(QString("httpResponseHeaderReceived\n"));
	for_each(QString key, resp.keys())
	{
		consoleWrite(QString("    %1: %2\n").arg(key).arg(resp.value(key))); 		
	}
}

// This slot is triggered from an HTTP request in sendXmlHttpRequest().
// Not connected by default, but left here because it's useful for debugging.
void JavaScriptAPI::httpReadyRead( const QHttpResponseHeader& resp )
{
	consoleWrite(QString("httpReadyRead\n"));
}

// This slot is triggered from an HTTP request in sendXmlHttpRequest().
// Not connected by default, but left here because it's useful for debugging.
void JavaScriptAPI::httpStateChanged(int state)
{
	consoleWrite(QString("HTTP state changed: %1\n").arg(state));
}

// An internal method that is used for marshaling a QString from C++ to JS
// External JavaScript (i.e., widgets) should not call this directly.
QString JavaScriptAPI::internal_getStringData()
{
	QString result = _stringData;
	_stringData = QString();
	return result;
}

// Build an HTML5 DragEvent to pass in to the page, to determine whether it
// will handle a drag-and-drop operation. The limitation is that the body of
// the page (rather than one of its children) must respond to the drag events.
// See http://www.whatwg.org/specs/web-apps/current-work/multipage/dnd.html#dnd
// Returns true if the event was canceled, meaning the operation was accepted.
// Note: If the page is still loading, this will return false but the event will
// be injected when the load completes.
bool JavaScriptAPI::injectDragAndDropEvent( QString eventName )
{
	// Here we construct a fake DragEvent and inject it into the DOM.
	// We try to make it look as much as possible like a true DragEvent.
	// We can't actually construct a DragEvent though, because it's not supported
	// in the version of QtWebKit that we're using.
	QString jsCode = QString(
		"(function () {"
		"	var evt = document.createEvent('UIEvents');"
		"	evt.initUIEvent('%1', true, true, window, null);"
		"	evt.screenX = evt.screenY = evt.clientX = evt.clientY = 0;"
		"	evt.ctrlKey = evt.shiftKey = evt.altKey = evt.metaKey = false;"
		"	evt.button = 0;"
		"	evt.relatedTarget = null;"
		"	evt.dataTransfer = { types: ['Files'], files: [%2] };"
		"	var canceled = !document.getElementsByTagName('body').item(0).dispatchEvent(evt);"
		"	return canceled;"
		"}());").arg(eventName).arg(_dragAndDropData);

	if(_page->isLoaded()) {
		return _page->evaluateJavaScript(jsCode).toBool();
	} else {
		_page->addExecuteCode(jsCode);
		return false;
	}
}

// Return true if the page supports having the given items dropped on it.
// We use a simplified version of the HTML5 drag and drop API here.
bool JavaScriptAPI::onDropEnter(vector<BumpObject *> &objList)
{
	// Make sure that there are only files in the list, and for each file,
	// create an associate JavaScript 'File' object.
	// See http://dev.w3.org/2006/webapi/FileUpload/publish/FileAPI.html#idl-if-File
	// and http://dev.w3.org/2006/webapi/FileUpload/publish/FileAPI.html#FileData-if
	// for more information on this interface.
	// The drag and drop data is an inline JS representation of an HTML5 'FileList'
	_dragAndDropData = QString();

	for_each(BumpObject *bumpObj, objList)
	{
		Pile *pile = (bumpObj->getObjectType() == ObjectType(BumpPile)) ? (Pile *)bumpObj : NULL;

		// Loop in order to upload all items contained in a pile
		// If item is not a pile, then complete the loop just once
		int pileIndex = 0;
		int numItems = pile ? pile->getNumItems() : 1;

		do
		{
			BumpObject *objToCopy = pile ? (*pile)[pileIndex] : bumpObj;

			if (!(objToCopy->getObjectType() == ObjectType(BumpActor, FileSystem)))
				return false;

			FileSystemActor *fsActor = (FileSystemActor *)objToCopy;

			if (fsActor->isFileSystemType(FileSystemActorType(Virtual)))
				return false;

			if (fsActor->isFileSystemType(FileSystemActorType(Folder)))
			{
				// show a tooltip saying "no folders..." for sharing only
				if (_actor->isSharingWidget())
					printUnique("SharingNoFolders", BtUtilStr->getString("SharingNoFolders"));
				
				return false;
			}
			
			QString path = fsActor->getFullPath();
			QString shortcutPath = path;

			if(path.endsWith(".lnk"))
				path = fsManager->getShortcutTarget(path, &shortcutPath);

			buildUploadString(shortcutPath, _dragAndDropData);
		} while (++pileIndex < numItems);
	}

	return injectDragAndDropEvent("dragenter");
}

// Return true if the drop event was accepted.
// This uses the HTML5 drag and drop event model
bool JavaScriptAPI::onDrop()
{
	return injectDragAndDropEvent("drop");
}

// Calls a focus/blur widget event handling callback if exists
void JavaScriptAPI::onWidgetFocus()
{	
	_page->evaluateJavaScript("jQuery.event.trigger('widgetfocus');");
}

void JavaScriptAPI::onWidgetBlur()
{
	_page->evaluateJavaScript("jQuery.event.trigger('widgetblur');");
}

// Add class object to javascript
void JavaScriptAPI::dialogJavaScriptWindowObjectCleared()
{
	((QWebFrame *)sender())->addToJavaScriptWindowObject(javaScriptObjectName, this);
}

QVariantMap JavaScriptAPI::getDialogUserData()
{
	return _dialogUserData;
}

// This method is called from JavaScript to open a dialog window in order to prompt the user for various settings related to setting up BumpShares
// The opened window populates an object to return to JavaScript with the results of the fields filled in from the dialog box
QVariantMap JavaScriptAPI::openDialogWindow(QString dialogBoxName, QVariantMap dialogUserData /*= QVariantMap()*/)
{
	// Used to pass information to the ShareCreatedDialog window
	if(!dialogUserData.empty())
		_dialogUserData = dialogUserData;

	// This is getting the parent directory and then going into sharing
	// Change this so that the path is not automatically constructed
	QDir dialogDir(QFileInfo(winOS->GetSharingWidgetPath()).absolutePath() / "dialogs");

	QString dialogPath = dialogDir.absoluteFilePath(dialogBoxName);
	assert(QFileInfo(dialogPath).exists());
	QUrl dialogUrl = QUrl::fromLocalFile(dialogPath);

	// Display html dialog window in a modified wizard window
	_dialog = new QDialog();
	_dialog->setModal(true);
	_dialog->resize(500, 450); // TODO: find a way to pass in a smart height for the dialog
	QWebView *webView = new QWebView(_dialog);
	QWebPage *consoleLoggingPage = new ConsoleLoggingQWebPage(_dialog);
	webView->setPage(consoleLoggingPage);
	webView->setUrl(dialogUrl);

	// This connection allows loading of C++ objects to the javascript on the dialog page
	connect(webView->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(dialogJavaScriptWindowObjectCleared()));

	QVBoxLayout *layout = new QVBoxLayout(_dialog);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(webView);
	_dialog->setLayout(layout);
	_dialog->exec();
	SAFE_DELETE(_dialog);

	// The result is set by calling setDialogResult from the dialog's JavaScript
	QVariantMap result = _dialogBoxResult;
	_dialogBoxResult.clear();
	return result;
}

// This method is called from a dialog box, to set the value that will be returned
// to the caller of openDialogBox().
void JavaScriptAPI::setDialogBoxResult(QVariantMap dialogBoxResult)
{
	_dialogBoxResult = dialogBoxResult;
}

// This method is called from JavaScript in order to close the dialog window
void JavaScriptAPI::closeDialogWindow()
{
	_dialog->close();
}

// Open a file for reading or writing.
// 'mode' is a string of length 1 indicating the access mode. Valid values
// are "r" (read), "w" (truncate), "a" (append).
// Returns the file handle on success, or -1 on failure.
int JavaScriptAPI::openFile(QString fileName, QString mode)
{
	QString filePath(winOS->GetUserApplicationDataPath().absoluteFilePath(fileName));

	QFile* file = new QFile(native(filePath));
	QIODevice::OpenMode flags;
	if (mode == "r")
		flags = QFile::ReadOnly;
	else if (mode == "w")
		flags = QFile::WriteOnly | QFile::Truncate;
	else if (mode == "a")
		flags = QFile::Append;

	if (file->open(flags))
	{
		_fileMap.insert(file->handle(), file);
		return file->handle();
	}
	SAFE_DELETE(file);
	return -1;
}

QString JavaScriptAPI::readFile(int fileHandle)
{
	if(_fileMap.contains(fileHandle))
	{
		QFile * file = _fileMap.value(fileHandle);
		QTextStream stream(file);
		stream.setCodec("UTF-8");
		assert(stream.status() == QTextStream::Ok);

		// read and return the contents
		return stream.readAll();
	}

	return QString::null;
}

bool JavaScriptAPI::writeFile(int fileHandle, QString contents)
{
	if(_fileMap.contains(fileHandle))
	{
		QFile * file = _fileMap.value(fileHandle);
		QTextStream stream(file);
		stream.setCodec("UTF-8");
		assert(stream.status() == QTextStream::Ok);

		// write the contents
		stream << contents;

		return true;
	}

	return false;
}

// This function renames the file to newFileName and replaces any existing files. The file is closed when renamed
bool JavaScriptAPI::renameAndCloseFile(int fileHandle, QString newFileName)
{
	bool result = false;
	if(_fileMap.contains(fileHandle))
	{
		QFile * file = _fileMap.take(fileHandle);
		QFileInfo fileInfo(file->fileName());
		QFileInfo newFilePath(make_file(fileInfo.absolutePath(), newFileName));

		if(newFilePath.exists())
			QFile::remove(newFilePath.absoluteFilePath());

		// File is automatically closed when renamed
		result = file->rename(newFilePath.absoluteFilePath());
		SAFE_DELETE(file);
	}

	return result;
}

void JavaScriptAPI::closeFile(int fileHandle)
{
	if(_fileMap.contains(fileHandle))
	{
		QFile * file = _fileMap.take(fileHandle);
		file->close();
		SAFE_DELETE(file);
	}
}

bool JavaScriptAPI::renameFile(QString oldFileName, QString newFileName)
{
	QFile oldFile(winOS->GetUserApplicationDataPath().absoluteFilePath(oldFileName));
	QFile newFile(winOS->GetUserApplicationDataPath().absoluteFilePath(newFileName));

	if(oldFile.rename(newFile.fileName()))
		return true;

	return false;
}

// Launch the given URL in a web browser
void JavaScriptAPI::openInBrowser( QString url )
{
	assert(QUrl(url, QUrl::StrictMode).scheme().startsWith("http"));
	fsManager->launchFileAsync(url);
}

// Open a file selection dialog and return the string representation of a Javascript object to be uploaded
QString JavaScriptAPI::openFileBrowser()
{
	QStringList selectedFilePaths = QFileDialog::getOpenFileNames();
	QString files = QString();
	int listLength = selectedFilePaths.size();
	for(int i = 0; i < listLength; i++)
	{
		buildUploadString(selectedFilePaths[i], files);
	}
	return QString_NT("[%1]").arg(files);
}

void JavaScriptAPI::buildUploadString(QString fullFilePath, QString& stringToBuild)
{
	if (!_fileDataPathsToUrls.contains(fullFilePath))
	{
		// Construct a filedata: URL, which is basically just a UUID (aka GUID)
		// Internally, we use this as a handle on the actual file.
		QString uuid = QUuid::createUuid().toString();
		uuid.remove('{').remove('}');
		QString fileDataUrl = QString_NT("filedata:") + uuid;

		// Keep a mapping in both directions (URL -> path and path -> URL)
		_fileDataUrlsToPaths.insert(fileDataUrl, fullFilePath);
		_fileDataPathsToUrls.insert(fullFilePath, fileDataUrl);
	}
	QString fileDataUrl = _fileDataPathsToUrls.value(fullFilePath);
	QFileInfo fileInfo(fullFilePath);

	// An inline representation of an HTML5 'File' object. It does not have any of the
	// required methods because we don't need those yet.
	stringToBuild += QString_NT("%1 { \"name\": \"%2\", \"mediaType\": \"%3\", \"size\": %4, \"url\": \"%5\" }")
		.arg(stringToBuild.isEmpty() ? "" : ",")
		.arg(fileInfo.fileName())
		.arg(QT_NT("application/octet-stream"))
		.arg(fileInfo.size())
		.arg(fileDataUrl);
}

QString JavaScriptAPI::translate(const QString& key)
{
	return Singleton<Translations>::getInstance()->translateWeb(QT_NT("JavaScriptAPI"), key.toAscii().constData());
}

QString JavaScriptAPI::translateFormat(const QString& key, const QStringList& arguments)
{
	// Looks funky but allows us not to overload translate format N times
	QString formatStr = Singleton<Translations>::getInstance()->translateWeb(QT_NT("JavaScriptAPI"), key.toAscii().constData());
	switch (arguments.size())
	{
	case 0:
		break;
	case 1:
		formatStr = formatStr.arg(arguments[0]);
		break;
	case 2:
		formatStr = formatStr.arg(arguments[0], arguments[1]);
		break;
	case 3:
		formatStr = formatStr.arg(arguments[0], arguments[1], arguments[2]);
		break;
	case 4:
		formatStr = formatStr.arg(arguments[0], arguments[1], arguments[2], arguments[3]);
		break;
	case 5:
		formatStr = formatStr.arg(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4]);
		break;
	case 6:
		formatStr = formatStr.arg(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5]);
		break;
	case 7:
		formatStr = formatStr.arg(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6]);
		break;
	case 8:
		formatStr = formatStr.arg(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7]);
		break;
	case 9:
		formatStr = formatStr.arg(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8]);
		break;
	default:
		assert(false);
		break;
	}
	return formatStr;
}

void JavaScriptAPI::copyTextToClipboard( QString text )
{
	QApplication::clipboard()->setText(text);
}

// Subclasses may want to override this to perform cleanup
void JavaScriptAPI::deleteFlyout() 
{}

// ---------------------------------------------------------------------------

SharedFolderAPI::SharedFolderAPI(WebActor *actor, WebPage *page) :
	JavaScriptAPI(actor, page, false)
{}

void SharedFolderAPI::showFlyout(QVariantMap data /*= QVariantMap()*/)
{
	_actor->showFlyout(data);
}

void SharedFolderAPI::deleteFlyout()
{
	abortAllUploads();
	_page->evaluateJavaScript(QT_NT("uploadCancelled();"));
}

// Call into the widget javascript to initiate an upload of the given files.
// 'files' is a string description as returned by openFileBrowser(), or as
// used in the drag and drop events.
void SharedFolderAPI::uploadFiles( QString files )
{
	_page->evaluateJavaScript(QString_NT("uploadFiles(%1);").arg(files));
}
