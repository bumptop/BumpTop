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
#include "BT_FileTransferManager.h"
#include "BT_SceneManager.h"
#include "BT_SVNRevision.h"
#include "BT_Util.h"
#include "BT_WindowsOS.h"

#include "moc/moc_BT_FileTransferManager.cpp"

/*
 * Static defs
 */
const bool verbose = false;

const QString FileTransfer::commonUrls[] =
{
	"ftp://ftp.bumptop.com/stats/",
	"ftp://ftp.bumptop.com/test/",
	"ftp://ftp.bumptop.com/download/",
	"ftp://ftp.bumptop.com/stats/toshiba/",
	"ftp://ftp.bumptop.com/stats/dell/",
	"ftp://ftp.bumptop.com/",
	"ftp://ftp.bumptop.com/staging/",
	"http://bumptop.com/download/",
	"http://bumptop.com/download/staging/",


};
const QString FileTransfer::commonLogins[] = 
{
	"alpha@omexcards.com",		// ftp://ftp.bumptop.com/stats/
	"alpha@omexcards.com",		// ftp://ftp.bumptop.com/test/
	"alpha@omexcards.com",		// ftp://ftp.bumptop.com/download/
	"alpha@omexcards.com",		// ftp://ftp.bumptop.com/stats/toshiba
	"alpha@omexcards.com",		// ftp://ftp.bumptop.com/stats/dell
	"alpha@omexcards.com",		// ftp://ftp.bumptop.com
	"alpha@omexcards.com",		// ftp://ftp.bumptop.com/staging
	"alpha@omexcards.com",		// http://bumptop.com/download
	"alpha@omexcards.com"		// http://bumptop.com/download/staging
};
const QString FileTransfer::commonPasswords[] = 
{
	"free57love",				// ftp://ftp.bumptop.com/stats/
	"free57love",				// ftp://ftp.bumptop.com/test/
	"free57love",				// ftp://ftp.bumptop.com/download/
	"free57love",				// ftp://ftp.bumptop.com/stats/toshiba
	"free57love",				// ftp://ftp.bumptop.com/stats/dell
	"free57love",				// ftp://ftp.bumptop.com
	"free57love",				// ftp://ftp.bumptop.com/staging
	"free57love",				// http://bumptop.com/download/
	"free57love",				// http://bumptop.com/download/staging
};

/* 
 * FileTransfer implementation
 */
FileTransfer::FileTransfer( FileTransferType type, FileTransferEventHandler * handler )
: _type(type)
, _handler(handler)
, _fileHandle(0)
, _stringBuffer(0)
, _timeout(0)
, _userData(NULL)
{}


FileTransfer::~FileTransfer()
{}

FileTransfer& FileTransfer::setUrl( QString url )
{
	_url = url;
	return *this;
}

FileTransfer& FileTransfer::setParams( QHash<QString, QString> params )
{
	_params = params;
	return *this;
}

FileTransfer& FileTransfer::setFileParamKeys(QSet<QString> paramKeys)
{
	_fileParamKeys = paramKeys;
	return *this;
}

FileTransfer& FileTransfer::setRelativeUrl( FileTransferCommonUrl url, QString file )
{
	_url = (commonUrls[url] + file);
	_login = commonLogins[url];
	_password = commonPasswords[url];
	return *this;
}

FileTransfer& FileTransfer::setLocalPath( QString path )
{
	// disable the string stream if a file is specified
	if (_stringBuffer)
		SAFE_DELETE(_stringBuffer);

	_localPath = path;
	return *this;
}

FileTransfer& FileTransfer::setTemporaryString()
{
	if (_type != FileTransfer::Upload)
	{
		// disable local path if there is one
		if (!_localPath.isEmpty())
		{			
			_localPath.clear();
		}
	}

	if (!_stringBuffer)
	{
		// create new string buffer
		_stringBuffer = new QString;
	}
	else
	{
		// otherwise, clear the stream
		_stringBuffer->clear();
	}
	return *this;
}

FileTransfer& FileTransfer::setLogin( QString login )
{
	_login = login;
	return *this;
}

FileTransfer& FileTransfer::setPassword( QString password )
{
	_password = password;
	return *this;
}

FileTransfer& FileTransfer::setTimeout( int timeout )
{
	_timeout = timeout;
	return *this;
}

FileTransfer& FileTransfer::setRemoteFileName(QString remoteFileName)
{
	_remoteFileName = remoteFileName;
	return *this;
}

FileTransfer& FileTransfer::setUserData( void *userData )
{
	_userData = userData;
	return *this; 
}

QString FileTransfer::getUrl() const
{
	return _url;
}

QHash<QString, QString> FileTransfer::getParams() const
{
	return _params;
}

QSet<QString> FileTransfer::getFileParamKeys() const
{
	return _fileParamKeys;
}

QString FileTransfer::getLocalPath() const
{
	return _localPath;
}

QString FileTransfer::getTemporaryString() const
{
	if (!_stringBuffer)
		throw runtime_error("Expected temporary string output where none was specified");
	return *_stringBuffer;
}

QString FileTransfer::getLogin() const
{
	return _login;
}

QString FileTransfer::getPassword() const
{
	return _password;
}

FileTransfer::FileTransferType FileTransfer::getType() const
{
	return _type;
}

int FileTransfer::getTimeout() const
{
	return _timeout;
}

QString FileTransfer::getRemoteFileName() const
{
	return _remoteFileName;
}

QString FileTransfer::getEncodedParams() const
{
	return ftManager->encodeParams(_params);
}

bool FileTransfer::isValid() const
{
	// only require that a local path and the remote url are set
	return (!_url.isEmpty() && (!_localPath.isEmpty() || (_stringBuffer != NULL)));
}

FileTransferEventHandler * FileTransfer::getHandler() const
{
	return _handler;
}

QString * FileTransfer::openTemporaryString()
{
	if (!_stringBuffer)
		throw runtime_error("Expected string stream");
	// if (_type == FileTransfer::Download)
	{
		_stringBuffer->clear();
	}
	return _stringBuffer;
}

void FileTransfer::clearTemporaryString()
{
	if (!_stringBuffer)
		throw runtime_error("Expected string stream");
	_stringBuffer->clear();
}

bool FileTransfer::shouldSaveToFile() const
{
	return (_stringBuffer == NULL);
}

void FileTransfer::cleanup()
{
	
}

void *FileTransfer::getUserData() const
{
	return _userData;
}

FileTransferWrapper::FileTransferWrapper(FileTransfer &ft, QList<FileTransferWrapper*> *parent, bool isBlocking, int maxNumRedirects)
: _ft(ft)
{
	_postData = NULL;
	_postBuffer = NULL;
	_reply = NULL;
	_aborted = false;
	_isBlocking = isBlocking;
	_maxNumberOfRedirects = maxNumRedirects;
	_parent = parent;
	_manager = new QNetworkAccessManager(this);
}

FileTransferWrapper::~FileTransferWrapper()
{
	_parent->removeOne(this);
	delete _postData;
	delete _postBuffer;
	_ft.cleanup();
	_reply->deleteLater();
	_manager->deleteLater();
	
}

FileTransfer& FileTransferWrapper::getFileTransfer()
{
	return _ft;
}

void FileTransferWrapper::authenticate(QNetworkReply *reply, QAuthenticator *authenticator)
{
	authenticator->setUser(_ft.getLogin());
	authenticator->setPassword(_ft.getPassword());	
}

void FileTransferWrapper::error(QNetworkReply::NetworkError code)
{
	if (verbose) consoleWrite(((QNetworkReply*)sender())->errorString() + "\n\n");
}

void FileTransferWrapper::abort()
{
	_aborted = true;
	_reply->abort();
}

bool FileTransferWrapper::isBlocking() const
{
	return _isBlocking;
}

int FileTransferWrapper::getMaxNumberOfRedirects() const
{
	return _maxNumberOfRedirects;
}

bool FileTransferWrapper::finishedReply(QNetworkReply * reply)
{
	assert(reply == _reply);
	bool hasError = false;
	bool succeeded = false;

	if (_aborted) 
		return false;

	// handle any redirects
	QUrl redirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
	QString redirectUrlStr = redirectUrl.toString();
	if (getMaxNumberOfRedirects() > 0 && !redirectUrl.isEmpty())
	{
		FileTransfer transfer = _ft;
		transfer.setUrl(redirectUrl.toString());
		deleteLater();
		return ftManager->addTransfer(transfer, isBlocking(), getMaxNumberOfRedirects()-1);
	}

	// if the reply has not finished, mark as error
	if (!reply->isFinished()) 
		hasError = true;

	// set the transfer data functions
	QByteArray data = reply->readAll();
	switch (_ft.getType())
	{
	case FileTransfer::Upload:
		// save the upload reply to the temporary string
		if (!_ft.shouldSaveToFile())
		{
			QString *outBuffer = _ft.openTemporaryString();
			outBuffer->append(data);

			if (verbose) consoleWrite("POST " + _ft.getTemporaryString() + "\n\n");
		}
		break;

	case FileTransfer::Download:
		// save the file to disk
		if (_ft.shouldSaveToFile())
		{
			QFile file(_ft.getLocalPath());
			if (!file.open(QIODevice::WriteOnly))
				hasError = true;			
			if (file.write(data) == -1) 
				hasError = true;
			file.close();
		}
		// save the file to the temporary string
		else
		{
			QString * outBuffer = _ft.openTemporaryString();
			outBuffer->append(data);

			if (verbose) consoleWrite("POST " + _ft.getTemporaryString() + "\n\n");
		}
		break;

	default:		
		throw runtime_error("Invalid FileTransfer type encountered");
		break;
	}
	
	// call the callbacks
	if (!hasError && reply->error() == QNetworkReply::NoError)
	{
		if (_ft.getHandler())
			_ft.getHandler()->onTransferComplete(_ft);
		succeeded = true;
	}
	else
	{
		if (_ft.getHandler())
			_ft.getHandler()->onTransferError(_ft);
		succeeded = false;
		
		ftManager->removeIncompleteFile(_ft);	
	}

	// delete this object later
	deleteLater();

	return succeeded;
}

void FileTransferWrapper::finished(QNetworkReply * reply)
{
	finishedReply(reply);
}


/* 
 * FileTransferManager implementation
 */
FileTransferManager::FileTransferManager()
{
	// initialize internet connection check function
	_pfnICC = NULL;
	_hMod = LoadLibrary(_T("Wininet.dll"));
	if (_hMod)
	{
		_pfnICC = (InternetCheckConnectionW) GetProcAddress(_hMod, "InternetCheckConnectionW");
		_pfnIGCC = (InternetGetConnectedState) GetProcAddress(_hMod, "InternetGetConnectedState");
	}	
}

FileTransferManager::~FileTransferManager()
{
	// free wininet
	if (_hMod)
		FreeLibrary(_hMod);
}


bool FileTransferManager::addTransfer(FileTransfer& transfer, bool blocking, int numRedirects)
{	
	bool succeeded = true;
	QNetworkRequest request = QNetworkRequest(QUrl::fromEncoded(transfer.getUrl().toAscii()));

	// uploads using qt for this function is not supported as no one seems to use it here.  See addPostTransfer
	assert(transfer.getType() == FileTransfer::Download);
	
	// set the user agent
	QString userAgent = QString("BumpTop/r%1/%2").arg(SVN_VERSION_NUMBER).arg(winOS->GetGUID());
	QByteArray tmp = userAgent.toAscii();
	request.setRawHeader("User-Agent", tmp);
	
	// create a new transfer
	FileTransferWrapper * ftw = new FileTransferWrapper(transfer, &transferList, blocking, numRedirects);
	transferList.append(ftw);

	// set the proxy settings if necessary
	if (transfer.getUrl().startsWith("http://", Qt::CaseInsensitive))
	{
		if (!GLOBAL(settings).httpProxyUrl.isEmpty())
		{
			QUrl proxyUrl = QUrl(GLOBAL(settings).httpProxyUrl);
			QNetworkProxy proxy = QNetworkProxy(QNetworkProxy::Socks5Proxy, proxyUrl.host(), proxyUrl.port(8080), proxyUrl.userName(), proxyUrl.password());
			ftw->_manager->setProxy(proxy);			
		}
	}

	QNetworkReply * reply = ftw->_manager->get(request);
	ftw->_reply = reply;
	assert(QObject::connect(ftw->_manager, SIGNAL(authenticationRequired(QNetworkReply *, QAuthenticator  *)), ftw, SLOT(authenticate(QNetworkReply *, QAuthenticator  *))));

	if (blocking)
	{
		QEventLoop loop;
		assert(QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit())));

		// set the timeout
		if (transfer.getTimeout())	
			QTimer::singleShot(transfer.getTimeout() * 1000, &loop, SLOT(quit()));	

		// loop until we finish downloading
		loop.exec();

		// handle the reply after we finish
		succeeded = ftw->finishedReply(reply);
	}
	else
	{
		// let the wrapper handle the file transfer and any redirects
		assert(QObject::connect(ftw->_manager, SIGNAL(finished(QNetworkReply*)), ftw, SLOT(finished(QNetworkReply*))));
		assert(QObject::connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), ftw, SLOT(error(QNetworkReply::NetworkError))));
	}
	
	return succeeded;
}

bool FileTransferManager::addPostTransfer(FileTransfer& transfer, bool blocking, QObject* progressListener)
{	
	bool err = false;
	QByteArray tmp;		
	QNetworkRequest *request = new QNetworkRequest(QUrl::fromEncoded(transfer.getUrl().toAscii()));

	assert(!transfer.shouldSaveToFile());
	
	// set the user agent
	QString userAgent = QString("BumpTop/r%1/%2").arg(SVN_VERSION_NUMBER).arg(winOS->GetGUID());
	tmp = userAgent.toAscii();
	request->setRawHeader("User-Agent", tmp);
	
	// XXX: doesn't really make sense to redirect on uploads
	FileTransferWrapper *ftw = new FileTransferWrapper(transfer, &transferList, blocking, 0);
	transferList.append(ftw);

	// set the proxy settings if necessary
	if (transfer.getUrl().startsWith("http://", Qt::CaseInsensitive))
	{
		if (!GLOBAL(settings).httpProxyUrl.isEmpty())
		{
			QUrl proxyUrl = QUrl(GLOBAL(settings).httpProxyUrl);
			QNetworkProxy proxy = QNetworkProxy(QNetworkProxy::Socks5Proxy, proxyUrl.host(), proxyUrl.port(8080), proxyUrl.userName(), proxyUrl.password());
			ftw->_manager->setProxy(proxy);			
		}
	}

	ftw->_postData = new QByteArray();
	QByteArray tmpAlt;
	
	QString boundary = QString("--------BUMPTOPBOUNDKf3Ie32MbvD2J78");
	QString crlf = QString("\r\n");
	// set the transfer data functions
	
	switch (transfer.getType())
	{
	case FileTransfer::Upload:
		{	
	
			// Fill in the HTTP request parameters, which are sent in the body.
			// In each param, the key maps to the name of a part in the request body
			// (e.g. Content-Disposition: form-data; name="<the_key>")
			// and the value maps to the data for that part. If the key is also
			// in fileParamKeys, then the value is a filename to read the data from.
			// Otherwise, the value itself is the data for that part.

			assert(transfer.getFileParamKeys().size() <= transfer.getParams().size());
			QSet<QString> fileParamKeys = transfer.getFileParamKeys();
			QHash<QString, QString> params = transfer.getParams();
			QHashIterator<QString, QString> iter(params);
			request->setRawHeader("Content-Type", ("multipart/form-data; boundary=" + boundary).toAscii());
			
			while (iter.hasNext())
			{
				*ftw->_postData += "--" + boundary + crlf;
				iter.next();

				if (fileParamKeys.contains(iter.key()))
				{
					// this is a file 
					tmp = iter.key().toAscii();
					tmpAlt = iter.value().toAscii();


					QFileInfo upInfo = QFileInfo(tmpAlt);
					if (!upInfo.exists()) {
						err = true;
						break;
					}
					QFile upFile(tmpAlt);
					if (!upFile.open(QIODevice::ReadWrite)) err = true;
					
					*ftw->_postData += "Content-Disposition: form-data; name=\"" + tmp + "\"; filename=\"" + upInfo.fileName() + "\"" + crlf;		
					
					// store the filename in a custom attribute so it can be retrived when signals are called (ie. uploadProgress)
					request->setAttribute(QNetworkRequest::User, QVariant(upInfo.fileName()));

					QString upContentType = "application/octet-stream";
										
					*ftw->_postData += "Content-Type: " + upContentType + crlf + crlf;
					
					//Data
					*ftw->_postData += upFile.readAll();				
					*ftw->_postData += crlf;
				}
				else
				{
					// this is a param 
					tmp = iter.key().toAscii();
					tmpAlt = iter.value().toAscii();
					*ftw->_postData += "Content-Disposition: form-data; name=\"" + tmp + "\"" + crlf + crlf;
					*ftw->_postData += tmpAlt + crlf;
					
				}
			}
			
			//Last boundary
			*ftw->_postData += "--" + boundary + "--" + crlf;
			
			break;
		}	

	case FileTransfer::Download:
		{
			*ftw->_postData += encodeParams(transfer.getParams()).toAscii();
		}
	}
	
	QEventLoop loop;	

	if (blocking)
		assert(QObject::connect(ftw->_manager, SIGNAL(finished(QNetworkReply*)), &loop, SLOT(quit())));
	else
		assert(QObject::connect(ftw->_manager, SIGNAL(finished(QNetworkReply*)), ftw, SLOT(finished(QNetworkReply*))));
	assert(QObject::connect(ftw->_manager, SIGNAL(authenticationRequired(QNetworkReply *, QAuthenticator  *)), ftw, SLOT(authenticate(QNetworkReply *, QAuthenticator  *))));
	ftw->_postBuffer = new QBuffer(ftw->_postData);
	assert(ftw->_postBuffer->open(QBuffer::ReadWrite));
	QNetworkReply *reply = ftw->_manager->post(*request ,ftw->_postBuffer);
	ftw->_reply = reply;
	assert(QObject::connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), ftw, SLOT(error(QNetworkReply::NetworkError))));
	if (progressListener)
		assert(QObject::connect(reply, SIGNAL(uploadProgress(qint64, qint64)), progressListener, SLOT(uploadProgress(qint64, qint64))));
	
	
	// set the timeout
	if (transfer.getTimeout())	
		QTimer::singleShot(transfer.getTimeout() * 1000, &loop, SLOT(quit()));
	

	if (blocking)
	{		
		loop.exec();
		if (blocking)
			ftw->finishedReply(reply);
	}

	if (err)
		transfer.getHandler()->onTransferError(transfer);

	return !err;	
}

bool FileTransferManager::hasInternetConnection(QString url)
{
	if (_pfnICC && _pfnIGCC)
	{
		// take a look at: IEnumNetworkConnections
		// for vista/win7

		// otherwise, we are relying on http://support.microsoft.com/kb/242558

		DWORD flags = 0;
	    if (_pfnIGCC(&flags, 0))
		{
			// #define FLAG_ICC_FORCE_CONNECTION 0x00000001	
			if (_pfnICC((LPCTSTR)url.utf16(), 0x00000001, 0))
				return true;
			else
			{
				// #define INTERNET_ERROR_BASE 12000
				// #define ERROR_INTERNET_TIMEOUT (INTERNET_ERROR_BASE+2)
				// #define ERROR_INTERNET_NAME_NOT_RESOLVED (INTERNET_ERROR_BASE+7)
				// #define ERROR_INTERNET_CANNOT_CONNECT (INTERNET_ERROR_BASE+29)
				DWORD err = GetLastError();
				if (err == 12002 ||
					err == 12007 ||
					err == 12029)
				{
					return false;
				}
				else
					return true;
			}
		}
	}
	return false;
} 

void FileTransferManager::removeIncompleteFile( const FileTransfer& transfer )
{
	if (transfer.shouldSaveToFile())
	{
		QFile localPath(transfer.getLocalPath());
		if (exists(native(localPath)))
		{
			localPath.remove();
		}
	}	
}

QString FileTransferManager::encodeParams( QHash<QString, QString> params )
{
	QString paramsStr;
	QHashIterator<QString, QString> iter(params);
	while (iter.hasNext())
	{
		iter.next();
		if (!paramsStr.isEmpty())
			paramsStr.append("&");
		paramsStr.append(
			QString("%1=")
			.arg(iter.key())
			.append(QUrl::toPercentEncoding(iter.value())));
	}
	return paramsStr;
}

bool FileTransferManager::hasTransfersToHandler(FileTransferEventHandler * handler)
{
	for (int i = 0; i < transferList.size(); ++i) 
	{
		FileTransferWrapper * ftw = transferList.at(i);
		if (ftw->getFileTransfer().getHandler() == handler) 
			return true;
	}
	return false;
}

void FileTransferManager::removeTransfersToHandler(FileTransferEventHandler * handler) 
{
	int i = 0;
	while (i < transferList.size())
	{
		FileTransferWrapper* ftw = transferList.at(i);
		if (ftw->getFileTransfer().getHandler() == handler) {
			ftw->abort();
			transferList.removeOne(ftw);
			if (verbose) consoleWrite("ABORTED AN OPERATION !!\n");
		}
		else {
			i++;
		}
	}
}
void FileTransferManager::removeAllTransfers() 
{
	while (!transferList.isEmpty()) {
		FileTransferWrapper* ftw = transferList.takeFirst();
		ftw->abort();
	}
}

